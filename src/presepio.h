#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "os\bitLabCore.h"
#include "boards\triac_board.h"
#include "boards\relay_board.h"

class Presepio: public bitLabCore
{
public:
  Presepio();

private:
  RelayBoard relay_board;
  TriacBoard triac_board;
};

#endif