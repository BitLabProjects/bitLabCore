#ifndef _WAV_DECODER_H_
#define _WAV_DECODER_H_

#include "mbed.h"
#include "../config.h"

struct WavHeader
{
  int32_t sampleRate;
  int32_t numChannels;
  int32_t bitsPerSample;
  int32_t dataLength;

  int32_t getSamplesPerChannelCount()
  {
    return dataLength / numChannels / (bitsPerSample / 8);
  }

  millisec getDuration()
  {
    return getSamplesPerChannelCount() * 1000 / sampleRate;
  }

  int32_t convertTimeInSamples(millisec time)
  {
    return time * sampleRate / 1000;
  }
};

struct WavSampleBuffer
{
  int8_t *samples; //samples buffer
  int32_t size;    //samples buffer size
  int32_t count;   //current samples count in the buffer, can be lower than the actual size

  void allocate(int32_t desiredSize)
  {
    size = desiredSize;
    count = 0;
    samples = new int8_t[size];
  }
};

class WavDecoder
{
public:
  WavDecoder();

  bool load(FILE *f);
  bool readSamples(FILE *fp, WavSampleBuffer* sampleBuffer);

  WavHeader wavHeader;
  int32_t samplesAlreadyRead;
};

#endif