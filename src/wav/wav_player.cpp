#include "wav_player.h"
#include "../utils.h"
#include "../os/os.h"

WavPlayer::WavPlayer() : led(LED2)
{
  counter = 0;
  wavFile = NULL;
  sampleBuffersCount = 6;
  sampleBuffers = new WavSampleBuffer[sampleBuffersCount];
  for (int i = 0; i < sampleBuffersCount; i++)
  {
    sampleBuffers[i].allocate(1024 * 4);
  }
  stop();

  led.period_us(1);
}

bool WavPlayer::play()
{
  if (playStatus != Stopped)
    return false;

  wavFile = fopen("/sd/imperial_march.wav", "r");

  if (!wavDecoder.load(wavFile))
  {
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
  underflow = false;
  if (wavFile)
  {
    fclose(wavFile);
    wavFile = NULL;
  }
  playStatus = Stopped;
}

void WavPlayer::mainLoop()
{
  if (underflow)
  {
    underflow = false;
    printf("Underflow detected!\n");
  }

  if (playStatus == Playing)
  {
    //millisec64 timeBefore = Os::currTime();
    if (fillSampleBuffer())
    {
      //millisec64 fillTime = Os::currTime() - timeBefore;
      //printf("fillTime: %lli ms\n", fillTime);
    }
  }
}

bool WavPlayer::fillSampleBuffer()
{
  // Fill the samples buffer, return false if nothing was added
  int sampleBuffersLast = (sampleBuffersTail + (sampleBuffersCount - 1)) % sampleBuffersCount;
  if (sampleBuffersHead == sampleBuffersLast)
  {
    return false; //Buffer already full
  }

  //printf("Filling...\n");
  while (sampleBuffersHead != sampleBuffersLast)
  {
    if (!wavDecoder.readSamples(wavFile, &sampleBuffers[sampleBuffersHead]))
    {
      //printf("No more samples\n");
      stop();
      return false;
    }
    //printf("Samples read: %i\n", sampleBuffers[sampleBuffersHead].count);

    sampleBuffersHeadTimeInSamples += sampleBuffers[sampleBuffersHead].count;

    sampleBuffersHead = (sampleBuffersHead + 1) % sampleBuffersCount;
  }
  //printf("Filling complete\n");

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

    // Play just the sample at the current time
    // Clamp it to avoid overflowing
    int8_t sample = sbEntry->samples[Utils::min(currSampleOffset, sbEntry->count - 1)];

    // Use the second line when using the led as output
    //led.write((sample+128) / 255.0f);
    led.write(Utils::abs(sample) / 128.0f);

    if (currSampleOffset >= sbEntry->count)
    {
      // Buffer completed, move to next one
      sampleBuffersTailTimeInSamples += sbEntry->count;
      sampleBuffersTail = (sampleBuffersTail + 1) % sampleBuffersCount;
    }
  }
  else
  {
    // We are playing, but there's nothing in the buffer: it's an underflow
    underflow = true;
  }
}
