#ifndef _STORYBOARDLOADER_H_
#define _STORYBOARDLOADER_H_

#include "Storyboard.h"
#include "..\json\Json.h"

class StoryboardLoader {
public:
  StoryboardLoader(Storyboard *storyboard, const char *jsonContent);

  bool load();

private:
  Storyboard* storyboard;
  Json json;
  bool readTimelines(int timelinesArray_ti, int timelinesCount);
  bool readEntries(Timeline* timeline, int entriesArray_ti, int entriesCount);
  bool accept(const jsmnaccept_t* acceptArg);
};

#endif