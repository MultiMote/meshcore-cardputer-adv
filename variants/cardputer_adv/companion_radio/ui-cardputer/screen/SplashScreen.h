#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"
#include "globals.h"

class SplashScreen : public UIScreen {
  CardputerUITask *_task;
  unsigned long dismiss_after;
  char version_info[12];

public:
  SplashScreen(CardputerUITask *task);
  int render(DisplayDriver &display) override;
  void poll() override;
};
