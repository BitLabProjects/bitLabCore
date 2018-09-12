#ifndef _WAV_PLAYER_H_
#define _WAV_PLAYER_H_

#include "mbed.h"
#include "../os/types.h"
#include "wav_decoder.h"
#include "FastPWM.h"

class WavPlayer
{
public:
  WavPlayer();

  bool play();
  void stop();
  void mainLoop();
  void tick(millisec64 timeDelta);

private:
  enum PlayStatus
  {
    Stopped,
    Paused,
    Playing
  };
  PlayStatus playStatus;
  WavDecoder wavDecoder;
  FILE *wavFile;
  bool wavFileLoaded;

  // Circular buffer for samples buffers
  int sampleBuffersCount;
  WavSampleBuffer* sampleBuffers;
  volatile int sampleBuffersHead;
  volatile int sampleBuffersTail;
  millisec sampleBuffersHeadTimeInSamples;
  millisec sampleBuffersTailTimeInSamples;
  millisec sampleBuffersMaxTime;
  volatile millisec64 playerTime;

  int counter;

  volatile bool underflow;

  FastPWM led;

  bool fillSampleBuffer();
};

#endif