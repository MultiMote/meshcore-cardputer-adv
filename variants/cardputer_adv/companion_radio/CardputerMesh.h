#pragma once

#include "CardputerUITask.h"

#include <MyMesh.h>

class CardputerMesh : public MyMesh {
  CardputerUITask *_ui;

public:
  CardputerMesh(mesh::Radio &radio, mesh::RNG &rng, mesh::RTCClock &rtc, SimpleMeshTables &tables,
                DataStore &store, CardputerUITask *ui)
      : MyMesh(radio, rng, rtc, tables, store, ui), _ui(ui) {}

  void onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code, uint8_t flags,
                   const uint8_t *path_snrs, const uint8_t *path_hashes, uint8_t path_len) override;
};

extern CardputerMesh the_mesh_cp;
