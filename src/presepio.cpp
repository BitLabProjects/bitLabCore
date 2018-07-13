#include "presepio.h"

#include "storyboard/storyboard_player.h"

Presepio::Presepio() : relay_board(),
                       triac_board()
{
  addModule(new StoryboardPlayer(&relay_board, &triac_board));
}


