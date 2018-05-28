#ifndef __JSON_H_
#define __JSON_H_

#include <stdlib.h>
#include <string.h>

#include "platform/Callback.h"

typedef enum
{
  Json_Accept_KeyValue = 0,
  Json_Accept_ObjectBegin = 1,
  Json_Accept_ObjectEnd = 2,
  Json_Accept_ArrayBegin = 3,
  Json_Accept_ArrayEnd = 4
} JsonAcceptType_t;

typedef struct
{
  JsonAcceptType_t type;
  int tokenIdx;
  const char *key;
  int keyLength;
  bool valueIsString;
  const char *value;
  int valueLength;
} JsonAccept_t;


typedef enum 
{
  Json_Parsed = 0,
  /* Not enough tokens were provided */
  Json_Error_NoMemory = -1,
  /* Invalid character inside JSON string */
  Json_Error_Invalid = -2,
} JSonParseResult_t;

class Json
{
public:
  Json(const char *jsonString, 
       size_t length, 
       mbed::Callback<bool(const JsonAccept_t *)> accept);

  JSonParseResult_t parse();

private:
  const char *source;
  const size_t sourceLength;
  const mbed::Callback<bool(const JsonAccept_t *)> accept;
  
  size_t pos;

  bool parsePrimitive(int& start, int& length);
  bool parseString(int& start, int& length);

  typedef enum
  {
    PS_START = 0,
    PS_OBJECT = 1,
    PS_ARRAY = 2,
    PS_OBJECT_KEY = 3,
    PS_OBJECT_KEY_VALUE_END = 4,
    PS_ARRAY_VALUE_END = 5,
    PS_END = 99,
  } ParserStateState_t;

  typedef struct
  {
    ParserStateState_t state;
    int keyStart;
    int keyLength;
  } ParserState_t;

  void fillAccept(JsonAccept_t *acceptArg, JsonAcceptType_t type, const char *source, ParserState_t *parentState, int valueStart, int valueLength, bool valueIsString = false);
  bool tryPushStack(ParserState_t* stack, int& size, int maxSize,
                    ParserStateState_t state, int keyStart, int keyLength);
  bool tryCallCallbackAndPopStack(ParserState_t* stateStack, int& stateStackSize, 
                                  JsonAcceptType_t type,
                                  int valueStart, int valueLength, bool valueIsString);
};
#endif
