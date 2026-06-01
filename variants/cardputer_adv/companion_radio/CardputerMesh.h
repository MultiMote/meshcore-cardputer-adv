#pragma once

#include "CardputerDataStore.h"
#include "CardputerUITask.h"

#include <MyMesh.h>


class CardputerMesh : public MyMesh {
  CardputerUITask *_ui;
  CardputerDataStore *_store;
  uint64_t rx_packet_count = 0;

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
  void logRxRaw(float snr, float rssi, const uint8_t raw[], int len) override;

  inline uint64_t receivedPacketsCount() const { return rx_packet_count; }

  void begin(bool has_display);
  void savePrefs();
  inline CustomNodePrefs *getCustomNodePrefs() { return &_custom_prefs; }
};

extern CardputerMesh the_mesh_cp;
