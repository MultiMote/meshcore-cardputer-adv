#pragma once

#include "CardputerDataStore.h"
#include "CardputerUITask.h"

#include <MyMesh.h>

#define CTL_TYPE_NODE_DISCOVER_REQ   0x80
#define CTL_TYPE_NODE_DISCOVER_RESP  0x90

class CardputerMesh : public MyMesh {
  CardputerUITask *_ui;
  CardputerDataStore *_store;
  uint64_t rx_packet_count = 0;
  uint32_t last_ping_tag = 0;
  uint32_t last_discover_tag = 0;
  uint8_t last_message_hash[MAX_HASH_SIZE] = {0};
  uint16_t last_message_heard_repeats = 0;

  CustomNodePrefs _custom_prefs {
    .power_save = 0,
    .battery_correction = 1.0f
  };

public:
  CardputerMesh(mesh::Radio &radio, mesh::RNG &rng, mesh::RTCClock &rtc, SimpleMeshTables &tables,
                CardputerDataStore &store, CardputerUITask *ui)
      : MyMesh(radio, rng, rtc, tables, store, ui), _ui(ui), _store(&store) {}

  void onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code, uint8_t flags,
                   const uint8_t *path_snrs, const uint8_t *path_hashes, uint8_t path_len) override;
  void onControlDataRecv(mesh::Packet* packet) override;
  mesh::DispatcherAction onRecvPacket(mesh::Packet* pkt) override;
  void onChannelMessageRecv(const mesh::GroupChannel &channel, mesh::Packet *pkt, uint32_t timestamp,
                            const char *text) override;
  void onMessageRecv(const ContactInfo &from, mesh::Packet *pkt, uint32_t sender_timestamp,
                           const char *text) override;
  void sendFloodScoped(const mesh::GroupChannel& channel, mesh::Packet* pkt, uint32_t delay_millis) override;

  // not virtual; using function hiding
  bool sendGroupMessage(uint32_t timestamp, mesh::GroupChannel& channel, const char* sender_name, const char* text, int text_len);
  // not virtual; using function hiding
  int sendMessage(const ContactInfo& recipient, uint32_t timestamp, uint8_t attempt, const char* text, uint32_t& expected_ack, uint32_t& est_timeout);

  void logRxRaw(float snr, float rssi, const uint8_t raw[], int len) override;

  ContactInfo* processAck(const uint8_t *data) override;


  inline uint64_t receivedPacketsCount() const { return rx_packet_count; }

  void begin(bool has_display);
  void savePrefs();
  bool sendPing(ContactInfo &contact);
  bool sendAdvert(bool flood);
  bool sendRepeatersDiscover();
  uint32_t getLastPingTag() const { return last_ping_tag; }
  inline CustomNodePrefs *getCustomNodePrefs() { return &_custom_prefs; }
  void loadMessageHistory(const uint8_t pkey[PUB_KEY_SIZE], bool is_channel, ChatHistory &history);

};

extern CardputerMesh the_mesh_cp;
