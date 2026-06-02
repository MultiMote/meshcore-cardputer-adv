#include "CardputerMesh.h"

void CardputerMesh::onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code, uint8_t flags,
                                const uint8_t *path_snrs, const uint8_t *path_hashes, uint8_t path_len) {
  MyMesh::onTraceRecv(packet, tag, auth_code, flags, path_snrs, path_hashes, path_len);

  if (path_len == getNodePrefs()->path_hash_mode + 1 && tag == last_ping_tag) {
    int8_t snr_signed = (int8_t)path_snrs[0];
    _ui->pingRecv(snr_signed / 4.0f, packet->getSNR());
  }
}

void CardputerMesh::logRxRaw(float snr, float rssi, const uint8_t raw[], int len) {
  MyMesh::logRxRaw(snr, rssi, raw, len);
  rx_packet_count++;
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
  uint32_t auth = 0;
  uint8_t flags = prefs->path_hash_mode & 0x03;

  getRNG()->random((uint8_t *)&last_ping_tag, sizeof(last_ping_tag));

  mesh::Packet *pkt = the_mesh_cp.createTrace(last_ping_tag, auth, flags);

  if (!pkt) {
    return false;
  }

  sendDirect(pkt, contact.id.pub_key, prefs->path_hash_mode + 1);
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
    unsigned long delay_millis = 0;
    TransportKey default_scope;
    memcpy(&default_scope.key, prefs->default_scope_key, sizeof(default_scope.key));
    sendFloodScoped(default_scope, pkt, delay_millis);
  } else {
    sendZeroHop(pkt);
  }

  return true;
}
