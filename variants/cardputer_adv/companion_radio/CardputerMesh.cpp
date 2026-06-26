#include "CardputerMesh.h"

void CardputerMesh::onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code, uint8_t flags,
                                const uint8_t *path_snrs, const uint8_t *path_hashes, uint8_t path_len) {
  MyMesh::onTraceRecv(packet, tag, auth_code, flags, path_snrs, path_hashes, path_len);

  if (path_len == getNodePrefs()->path_hash_mode + 1 && tag == last_ping_tag) {
    int8_t snr_signed = (int8_t)path_snrs[0];
    _ui->pingRecv(snr_signed / 4.0f, packet->getSNR());
  }
}

void CardputerMesh::onControlDataRecv(mesh::Packet *packet) {
  MyMesh::onControlDataRecv(packet);
  uint8_t type = packet->payload[0] & 0xF0;
  uint8_t node_type = packet->payload[0] & 0x0F;

  if (!(type == CTL_TYPE_NODE_DISCOVER_RESP && packet->payload_len >= 6)) {
    return;
  }

  if (node_type != ADV_TYPE_REPEATER) {
    return;
  }

  if (packet->payload_len < 6 + PUB_KEY_SIZE) {
    return;
  }

  uint32_t tag;
  memcpy(&tag, &packet->payload[2], 4);

  if (tag != last_discover_tag) {
    return;
  }

  mesh::Identity id(&packet->payload[6]);

  if (id.matches(self_id)) {
    return;
  }

  _ui->discoverRecv(id, packet->getSNR());
}

mesh::DispatcherAction CardputerMesh::onRecvPacket(mesh::Packet *pkt) {
  mesh::DispatcherAction action = MyMesh::onRecvPacket(pkt);

  if (pkt->getPayloadType() == PAYLOAD_TYPE_GRP_TXT) {
    uint8_t hash[MAX_HASH_SIZE];
    pkt->calculatePacketHash(hash);

    if (memcmp(hash, last_message_hash, MAX_HASH_SIZE) == 0) {
      last_message_heard_repeats++;
      _ui->messageRepeatsRecv(last_message_heard_repeats);
    }
  }

  return action;
}

void CardputerMesh::onChannelMessageRecv(const mesh::GroupChannel &channel, mesh::Packet *pkt,
                                         uint32_t timestamp, const char *text) {
  MyMesh::onChannelMessageRecv(channel, pkt, timestamp, text);
  _store->storeMessage(channel.secret, text, false, true);
  _ui->onChannelMessageRecv(channel, text);
}

void CardputerMesh::onMessageRecv(const ContactInfo &from, mesh::Packet *pkt, uint32_t sender_timestamp,
                                  const char *text) {
  MyMesh::onMessageRecv(from, pkt, sender_timestamp, text);

  _store->storeMessage(from.id.pub_key, text, false, false);
  _ui->onContactMessageRecv(from, text);
}

void CardputerMesh::sendFloodScoped(const mesh::GroupChannel &channel, mesh::Packet *pkt,
                                    uint32_t delay_millis) {
  MyMesh::sendFloodScoped(channel, pkt, delay_millis);

  if (pkt->getPayloadType() == PAYLOAD_TYPE_GRP_TXT) {
    pkt->calculatePacketHash(last_message_hash);
    last_message_heard_repeats = 0;
  }
}

bool CardputerMesh::sendGroupMessage(uint32_t timestamp, mesh::GroupChannel &channel, const char *sender_name,
                                     const char *text, int text_len) {
  bool ok = MyMesh::sendGroupMessage(timestamp, channel, sender_name, text, text_len);
  _store->storeMessage(channel.secret, text, true, true);
  return ok;
}

int CardputerMesh::sendMessage(const ContactInfo &recipient, uint32_t timestamp, uint8_t attempt,
                               const char *text, uint32_t &expected_ack, uint32_t &est_timeout) {
  int rc = MyMesh::sendMessage(recipient, timestamp, attempt, text, expected_ack, est_timeout);
  _store->storeMessage(recipient.id.pub_key, text, true, false);
  return rc;
}

void CardputerMesh::logRxRaw(float snr, float rssi, const uint8_t raw[], int len) {
  MyMesh::logRxRaw(snr, rssi, raw, len);
  rx_packet_count++;
}

ContactInfo *CardputerMesh::processAck(const uint8_t *data) {
  uint32_t hash;
  memcpy(&hash, data, 4);
  _ui->onAckRecv(hash);

  return MyMesh::processAck(data);
}

void CardputerMesh::begin(bool has_display) {
  MyMesh::begin(has_display);
  _store->loadCustomPrefs(_custom_prefs);
}

void CardputerMesh::savePrefs() {
  MyMesh::savePrefs();
  _store->saveCustomPrefs(_custom_prefs);
}

bool CardputerMesh::sendPing(ContactInfo &contact) {
  NodePrefs *prefs = getNodePrefs();
  uint8_t flags = prefs->path_hash_mode & 0x03;

  getRNG()->random((uint8_t *)&last_ping_tag, sizeof(last_ping_tag));

  mesh::Packet *pkt = the_mesh_cp.createTrace(last_ping_tag, 0, flags);

  if (!pkt) {
    return false;
  }

  sendDirect(pkt, contact.id.pub_key, prefs->path_hash_mode + 1);
  return true;
}

bool CardputerMesh::sendAdvert(bool flood) {
  mesh::Packet *pkt;
  NodePrefs *prefs = getNodePrefs();

  if (prefs->advert_loc_policy == ADVERT_LOC_NONE) {
    pkt = createSelfAdvert(prefs->node_name);
  } else {
    pkt = createSelfAdvert(prefs->node_name, sensors.node_lat, sensors.node_lon);
  }

  if (!pkt) {
    return false;
  }

  if (flood) {
    TransportKey default_scope;
    memcpy(&default_scope.key, prefs->default_scope_key, sizeof(default_scope.key));
    MyMesh::sendFloodScoped(default_scope, pkt, 0);
  } else {
    sendZeroHop(pkt);
  }

  return true;
}

struct __attribute__((packed)) discover_data_t {
  uint8_t ctl_type;
  uint8_t discover_type;
  uint32_t tag;
  uint32_t since;
};

bool CardputerMesh::sendRepeatersDiscover() {
  discover_data_t data;
  data.ctl_type = CTL_TYPE_NODE_DISCOVER_REQ;
  data.discover_type = (1 << ADV_TYPE_REPEATER);
  data.since = 0;
  getRNG()->random((uint8_t *)&data.tag, sizeof(data.tag));

  auto pkt = createControlData((uint8_t *)&data, sizeof(data));
  if (!pkt) {
    return false;
  }

  last_discover_tag = data.tag;
  sendZeroHop(pkt);

  return true;
}

void CardputerMesh::loadMessageHistory(const uint8_t pkey[PUB_KEY_SIZE], bool is_channel,
                                       ChatHistory &history) {
  _store->loadMessages(pkey, is_channel, history);
}
