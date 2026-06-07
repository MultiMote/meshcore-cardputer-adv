#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"
#include "globals.h"

class MsgPreviewScreen : public UIScreen {
  struct MsgEntry {
    uint32_t timestamp;
    char origin[62];
    char msg[150];
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  int num_unread = 0;
  int head = MAX_UNREAD_MSGS - 1; // index of latest unread message
  MsgEntry unread[MAX_UNREAD_MSGS];

public:
  MsgPreviewScreen(CardputerUITask *task, mesh::RTCClock *rtc) : _task(task), _rtc(rtc) {}
  void addPreview(uint8_t path_len, const char *from_name, const char *msg);
  int render(DisplayDriver &display) override;
  bool handleInput(char c) override;
};
