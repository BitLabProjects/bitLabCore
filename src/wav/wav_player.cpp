#include "wav_player.h"
#include "../utils.h"

WavPlayer::WavPlayer(): led(LED2)
{
  wavFile = NULL;
  sampleBuffersCount = 2;
  sampleBuffers = new WavSampleBuffer[sampleBuffersCount];
  for(int i=0; i<sampleBuffersCount; i++)
  {
    sampleBuffers[i].allocate(1024 * 10);
  }
  stop();

  led.period_ms(1);
}

bool WavPlayer::play()
{
  if (playStatus != Stopped)
    return false;

  wavFile = fopen("/sd/imperial_march.wav","r");

  if (!wavDecoder.load(wavFile)) {
    fclose(wavFile);
    return false;
  }

  sampleBuffersMaxTime = wavDecoder.wavHeader.getDuration();
  printf("Wav loaded:\nChannels: %li\nSample rate: %li\nBits per sample: %li\nData length: %li\nDuration: %li ms\n", 
            wavDecoder.wavHeader.numChannels, 
            wavDecoder.wavHeader.sampleRate,
            wavDecoder.wavHeader.bitsPerSample,
            wavDecoder.wavHeader.dataLength,
            sampleBuffersMaxTime);

  printf("%i ms -> %li samples\n", 
         500, wavDecoder.wavHeader.convertTimeInSamples(500));

  if (!fillSampleBuffer())
  {
    fclose(wavFile);
    return false;
  }

  playStatus = Playing;
  return true;
}

void WavPlayer::stop()
{
  //Reset all the buffers, a successive play starts from the beginning
  sampleBuffersHead = 0;
  sampleBuffersTail = 0;
  sampleBuffersHeadTimeInSamples = 0;
  sampleBuffersTailTimeInSamples = 0;
  playerTime = 0;
  if (wavFile)
  {
    fclose(wavFile);
    wavFile = NULL;
  }
  playStatus = Stopped;
}

void WavPlayer::mainLoop()
{
  if (playStatus == Playing)
  {
    fillSampleBuffer();
  }
}

bool WavPlayer::fillSampleBuffer()
{
  // Fill the samples buffer, return false if no more samples available
  int sampleBuffersLast = (sampleBuffersTail + (sampleBuffersCount - 1)) % sampleBuffersCount;
  if (sampleBuffersHead == sampleBuffersLast)
  {
    return true; //Buffer already full
  }

  printf("Filling...\n");
  while (sampleBuffersHead != sampleBuffersLast)
  {
    if (!wavDecoder.readSamples(wavFile, &sampleBuffers[sampleBuffersHead]))
    {
      printf("No more samples\n");
      stop();
      return false;
    }
    printf("Samples read: %i\n", sampleBuffers[sampleBuffersHead].count);

    sampleBuffersHeadTimeInSamples += sampleBuffers[sampleBuffersHead].count;

    sampleBuffersHead = (sampleBuffersHead + 1) % sampleBuffersCount;
  }
  printf("Filling complete\n");

  return true;
}

void WavPlayer::tick(millisec64 timeDelta)
{
  if (playStatus != Playing)
  {
    return;
  }

  // If the buffer contains something
  if (sampleBuffersTail != sampleBuffersHead)
  {
    millisec64 newPlayerTime = (playerTime + timeDelta) % sampleBuffersMaxTime;
    int32_t newPlayerTimeInSamples = wavDecoder.wavHeader.convertTimeInSamples(newPlayerTime);
    if (newPlayerTimeInSamples < sampleBuffersTailTimeInSamples)
    {
      // TODO loop
      stop();
      return;
    }
    
    playerTime = newPlayerTime;

    WavSampleBuffer *sbEntry = &sampleBuffers[sampleBuffersTail];
    int32_t currSampleOffset = (newPlayerTimeInSamples - sampleBuffersTailTimeInSamples);
    currSampleOffset = Utils::min(currSampleOffset, sbEntry->count - 1);

    // Play just the sample at the current time
    int8_t sample = sbEntry->samples[currSampleOffset];
    led.write(sample / 128.0f);

    if (currSampleOffset >= sbEntry->count - 1)
    {
      // Buffer completed, move to next one
      sampleBuffersTailTimeInSamples += sbEntry->count;
      sampleBuffersTail = (sampleBuffersTail + 1) % sampleBuffersCount;
    }
  }
}
