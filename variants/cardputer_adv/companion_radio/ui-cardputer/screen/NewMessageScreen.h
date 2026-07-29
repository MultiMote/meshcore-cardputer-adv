#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "globals.h"

class NewMessageScreen : public CardputerScreen {
  CardputerUITask *_task;
  char origin[62];
  char message[MAX_MESSAGE_LENGTH];
  unsigned long dismiss_after = 0;

public:
  NewMessageScreen(CardputerUITask *task) : _task(task) {}
  void newMessage(const char *from, const char *msg);
  int render(CardputerDisplay &lcd) override;
  bool handleInput(Keyboard::Event &e) override;
  void poll() override;
};
