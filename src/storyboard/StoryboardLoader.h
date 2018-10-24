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

  typedef enum
  {
      SLS_Begin = 0,
      SLS_TimelinesArray = 1,
      SLS_Timeline = 2,
      SLS_EntriesArray = 3,
      SLS_Entry = 4,
      SLS_End = 9999,
  } StoryboardLoaderState_t;

  StoryboardLoaderState_t state;
  static const int bufferSize = 32;
  char key[bufferSize];
  char value[bufferSize];
  Timeline tempTimeline;
  uint32_t temp_outputHardwareId;
  uint8_t temp_outputId;
  TimelineEntry tempTimelineEntry;

  bool readTimelines(int timelinesArray_ti, int timelinesCount);
  bool readEntries(Timeline* timeline, int entriesArray_ti, int entriesCount);
  bool accept(const JsonAccept_t* acceptArg);

  bool tryMatchInt32(const JsonAccept_t *acceptArg,
                     const char *expectedKey,
                     int32_t &result);
  bool tryMatchUInt32(const JsonAccept_t *acceptArg,
                      const char *expectedKey,
                      uint32_t &result);
};

#endif