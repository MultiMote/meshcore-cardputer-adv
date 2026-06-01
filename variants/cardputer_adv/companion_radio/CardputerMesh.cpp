#include "CardputerMesh.h"

void CardputerMesh::onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code, uint8_t flags,
                                const uint8_t *path_snrs, const uint8_t *path_hashes, uint8_t path_len) {
  MyMesh::onTraceRecv(packet, tag, auth_code, flags, path_snrs, path_hashes, path_len);

  if (path_len > 0) {
    int8_t snr_signed = (int8_t)path_snrs[0];
    _ui->pingRecv(tag, path_len, snr_signed / 4.0f, packet->getSNR());
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
