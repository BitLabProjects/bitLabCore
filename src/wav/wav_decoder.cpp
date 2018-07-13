#include "wav_decoder.h"
#include "../utils.h"

WavDecoder::WavDecoder()
{
}

bool WavDecoder::load(FILE *fp)
{
  char c[5];
  int nbRead;
  int chunkSize;
  int subChunk1Size;
  int subChunk2Size;
  short int audFormat;
  short int nbChannels;
  int sampleRate;
  int byteRate;
  short int blockAlign;
  short int bitsPerSample;

  c[4] = 0;

  nbRead = fread(c, sizeof(char), 4, fp);

  // EOF ?
  if (nbRead < 4)
    return false;

  // Not a RIFF ?
  if (strcmp(c, "RIFF") != 0)
  {
    printf("Not a RIFF: %s\n", c);
    return false;
  }

  nbRead = fread(&chunkSize, sizeof(int), 1, fp);

  // EOF ?
  if (nbRead < 1)
    return false;

  nbRead = fread(c, sizeof(char), 4, fp);

  // EOF ?
  if (nbRead < 4)
    return false;

  // Not a WAVE riff ?
  if (strcmp(c, "WAVE") != 0)
  {
    printf("Not a WAVE: %s\n", c);
    return false;
  }

  nbRead = fread(c, sizeof(char), 4, fp);

  // EOF ?
  if (nbRead < 4)
    return false;

  // Not a "fmt " subchunk ?
  if (strcmp(c, "fmt ") != 0)
  {
    printf("No fmt subchunk: %s\n", c);
    return false;
  }

  // read size of chunk.
  nbRead = fread(&subChunk1Size, sizeof(int), 1, fp);
  if (nbRead < 1)
    return false;

  // is it a PCM ?
  if (subChunk1Size != 16)
  {
    printf("Not PCM fmt chunk size: %x\n", subChunk1Size);
    return false;
  }

  nbRead = fread(&audFormat, sizeof(short int), 1, fp);
  if (nbRead < 1)
    return false;

  // is it PCM ?
  if (audFormat != 1)
  {
    printf("No PCM format (1): %x\n", audFormat);
    return false;
  }

  nbRead = fread(&nbChannels, sizeof(short int), 1, fp);
  if (nbRead < 1)
    return false;

  // is it mono or stereo ?
  if (nbChannels > 2 || nbChannels < 1)
  {
    printf("Number of channels invalid: %x\n", nbChannels);
    return false;
  }

  nbRead = fread(&sampleRate, sizeof(int), 1, fp);
  if (nbRead < 1)
    return false;

  nbRead = fread(&byteRate, sizeof(int), 1, fp);
  if (nbRead < 1)
    return false;

  nbRead = fread(&blockAlign, sizeof(short int), 1, fp);
  if (nbRead < 1)
    return false;

  nbRead = fread(&bitsPerSample, sizeof(short int), 1, fp);
  if (nbRead < 1)
    return false;

  nbRead = fread(c, sizeof(char), 4, fp);

  // EOF ?
  if (nbRead < 4)
    return false;

  // Not a data section ?
  if (strcmp(c, "data") != 0)
  {
    printf("Not a data subchunk: %s\n", c);
    return false;
  }

  nbRead = fread(&subChunk2Size, sizeof(int), 1, fp);
  if (nbRead < 1)
    return false;

  // Now we can generate the structure...
  wavHeader.sampleRate = sampleRate;
  wavHeader.numChannels = nbChannels;
  wavHeader.bitsPerSample = bitsPerSample;
  wavHeader.dataLength = subChunk2Size;

  samplesAlreadyRead = 0;

  return true;
}

const int32_t fileReadBufferSize = 10240;
uint8_t fileReadBuffer[fileReadBufferSize];

bool WavDecoder::readSamples(FILE *fp, WavSampleBuffer* sampleBuffer)
{
  /* Calculate the number of samples that fit into the buffer
   * considering also how many samples are left from the file
   */
  int32_t remainingSamples = wavHeader.getSamplesPerChannelCount() - samplesAlreadyRead;
  int32_t samplesToRead = Utils::min(remainingSamples, sampleBuffer->size);

  if (samplesToRead <= 0)
  {
    return false;
  }

  samplesAlreadyRead += samplesToRead;
  sampleBuffer->count = samplesToRead;
  
  int8_t *samples = sampleBuffer->samples;

  int32_t bytesPerSamplexChans = wavHeader.numChannels * (wavHeader.bitsPerSample/8);
  int32_t samplesPerFill = 5000;
  int32_t fillCount = samplesToRead / samplesPerFill;
  // Add one fill if not an exact multiple of 500
  if (fillCount * samplesPerFill != samplesToRead)
  {
    fillCount++;
  }

  //printf("Reading %i samples, %i fills, alreadyRead: %i\n", samplesToRead, fillCount, samplesAlreadyRead);

  while (samplesToRead > 0) {
    int samplesToReadThisTime = Utils::min(samplesToRead, samplesPerFill);
    samplesToRead -= samplesToReadThisTime;

    fread(fileReadBuffer, bytesPerSamplexChans, samplesToReadThisTime, fp);

    uint8_t* fileReadBufferPtr = fileReadBuffer;

    unsigned char stuff8;
    /* Is it a stereo file ? */
    if (wavHeader.numChannels == 2)
    {
      /* 8-bit ? convert 0-255 to -128-127 */
      if (wavHeader.bitsPerSample == 8)
      {
        for (int i = 0; i < samplesToReadThisTime; i++)
        {
          stuff8 = *fileReadBufferPtr++;
          int8_t sample_L = -128 + stuff8;
          stuff8 = *fileReadBufferPtr++;
          int8_t sample_R = -128 + stuff8;
          *samples++ = sample_L;
        }
      }
      /* 16-bit ? convert signed 16 to signed 8 */
      else
      {
        for (int i = 0; i < samplesToReadThisTime; i++)
        {
          // TODO Use all 16 bits
          // We take only MSB of wave data...
          stuff8 = *fileReadBufferPtr++;
          stuff8 = *fileReadBufferPtr++;
          int8_t sample_L = (signed char)stuff8;
          // read right output and forget about it
          stuff8 = *fileReadBufferPtr++;
          stuff8 = *fileReadBufferPtr++;
          int8_t sample_R = (signed char)stuff8;
          *samples++ = sample_L;
        }
      }
    }
    /* Monaural file */
    /** PATCHED FOR ARDUINO **/
    else
    {
      if (wavHeader.bitsPerSample == 8)
      {
        for (int i = 0; i < samplesToReadThisTime; i++)
        {
          fread(&stuff8, sizeof(unsigned char), 1, fp);
          int8_t sample = stuff8;
          *samples++ = sample;
        }
      }
      else
      {
        for (int i = 0; i < samplesToReadThisTime; i++)
        {
          // TODO Use all 16 bits
          // We take only MSB of wave data...
          stuff8 = *fileReadBufferPtr++;
          stuff8 = *fileReadBufferPtr++;
          int8_t sample = *((int8_t*)(&stuff8));
          *samples++ = sample;
        }
      }
    }
  }


  return true;
}
