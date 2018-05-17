#include "storyboard_loader.h"

#include "..\json\Json.h"
#include "..\os\os.h"

StoryboardLoader::StoryboardLoader(Storyboard *storyboard, const char *jsonContent) : storyboard(storyboard), json(jsonContent, strlen(jsonContent), 100)
{
}

bool StoryboardLoader::load()
{
  if (!json.isValidJson())
  {
    Os::debug("Invalid JSON\n");
    return false;
  }
  if (json.type(0) != JSMN_OBJECT)
  {
    Os::debug("Invalid JSON. ROOT element is not Object\n");
    return false;
  }

  int timelinesArray_ti = json.findValueIndexByKeyName("timelines", JSMN_ARRAY);

  int timelinesCount = (timelinesArray_ti == -1) ? 0 : json.childCount(timelinesArray_ti);
  int totalDuration = json.getIntegerValue("totalDuration", 0);
  storyboard->create(timelinesCount, totalDuration);
  Os::debug("Storyboard has %i timelines and duration %i ms\n", timelinesCount, totalDuration);

  return readTimelines(timelinesArray_ti, timelinesCount);
}

bool StoryboardLoader::readTimelines(int timelinesArray_ti, int timelinesCount)
{
  int timelineObject_ti = -1;
  for (int idxTimeline = 0; idxTimeline < timelinesCount; idxTimeline++)
  {
    timelineObject_ti = json.findChildIndexOf(timelinesArray_ti, timelineObject_ti);
    if (timelineObject_ti == -1)
    {
      Os::assertFalse("timelinesCount");
      return false;
    }

    int outputId = json.getIntegerValue("outputId", 1, timelineObject_ti);
    if (outputId != idxTimeline + 1)
    {
      Os::debug("timeline with output %i not sequential\n", outputId);
      return false;
    }

    int entriesArray_ti = json.findValueIndexByKeyName("entries", JSMN_ARRAY, timelineObject_ti);
    int entriesCount = (entriesArray_ti == -1) ? 0 : json.childCount(entriesArray_ti);
    Timeline* timeline = storyboard->addTimeline(outputId, entriesCount);
    Os::debug("Timeline #%i on output #%i has %i entries\n", idxTimeline, outputId, entriesCount);

    if (!readEntries(timeline, entriesArray_ti, entriesCount))
    {
      return false;
    }
  }

  return true;
}
bool StoryboardLoader::readEntries(Timeline* timeline, int entriesArray_ti, int entriesCount)
{
  int entryObject_ti = -1;
  for (int idxEntry = 0; idxEntry < entriesCount; idxEntry++)
  {
    entryObject_ti = json.findChildIndexOf(entriesArray_ti, entryObject_ti);
    if (entryObject_ti == -1)
    {
      Os::assertFalse("entriesCount");
      return false;
    }

    int time = json.getIntegerValue("time", 0, entryObject_ti);
    int value = json.getIntegerValue("value", 0, entryObject_ti);
    int duration = json.getIntegerValue("duration", 0, entryObject_ti);
    Os::debug(" %5i ms) %i in %i ms\n", time, value, duration);

    timeline->add(time, value, duration);
  }

  return true;
}
