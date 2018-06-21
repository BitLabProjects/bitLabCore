#include "Json.h"

#include "..\os\os.h"

Json::Json(const char *source, 
           size_t length, 
           mbed::Callback<bool(const JsonAccept_t *)> accept)
    : source(source), sourceLength(length), accept(accept)
{
  
}

void Json::fillAccept(JsonAccept_t *acceptArg, JsonAcceptType_t type, ParserState_t *parentState, int valueStart, int valueLength, bool valueIsString)
{
  acceptArg->type = type;
  acceptArg->tokenIdx = 0;
  //Os::debug("tokenKey->start: %i, tokenKey->end: %i\n", tokenKey->start, tokenKey->end);
  if (parentState->keyLength <= 0)
  {
    // It's an object or array not yet completed, pass a null name
    acceptArg->key = NULL;
    acceptArg->keyLength = 0;
  }
  else
  {
    acceptArg->key = &source[parentState->keyStart];
    acceptArg->keyLength = parentState->keyLength;
  }

  acceptArg->valueIsString = valueIsString;
  if (valueLength <= 0)
  {
    acceptArg->value = NULL;
    acceptArg->valueLength = 0;
  }
  else
  {
    acceptArg->value = &source[valueStart];
    acceptArg->valueLength = valueLength;
  }
}

inline bool isWhitespace(char c)
{
  switch (c)
  {
    case '\t':
    case '\r':
    case '\n':
    case ' ':
      return true;
  }
  return false;
}

bool isStartOfPrimitiveValue(char c)
{
  switch (c)
  {
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 't':
    case 'f':
    case 'n':
      return true;
  }
  return false;
}

bool Json::tryPushStack(ParserState_t* stack, int& size, int maxSize,
                        ParserStateState_t state, int keyStart, int keyLength)
{
  if (size == maxSize)
  {
    return false;
  }

  stack[size].state = state;
  stack[size].keyStart = keyStart;
  stack[size].keyLength = keyLength;
  size += 1;

  return true;
}

/**
 * Fills next available token with JSON primitive.
 */
bool Json::parsePrimitive(int& start, int& length)
{
  start = pos;

  for (; pos < sourceLength && source[pos] != '\0'; pos++)
  {
    switch (source[pos])
    {
      case '\t':
      case '\r':
      case '\n':
      case ' ':
      case ',':
      case ']':
      case '}':
      {
        length = pos - start;
        // pos is pointing to the char that terminated the primitive value, 
        // go back one char as it does not belong to the primitive value
        pos--; 
        return true;
      }
    }
    if (source[pos] < 32 || source[pos] >= 127)
    {
      return false;
    }
  }
  /* primitive must be followed by a comma/object/array */
  return false;
}

/**
 * Fills next token with JSON string.
 */
bool Json::parseString(int& start, int& length)
{
  start = pos;
  pos++;

  /* Skip starting quote */
  for (; pos < sourceLength && source[pos] != '\0'; pos++)
  {
    char c = source[pos];

    /* Quote: end of string */
    if (c == '\"')
    {
      start = start + 1;
      length = pos - start;
      // Do not increment pos, the caller will do it in the next iteration cycle
      return true;
    }

    /* Backslash: Quoted symbol expected */
    if (c == '\\' && pos + 1 < sourceLength)
    {
      int i;
      pos++;
      switch (source[pos])
      {
      /* Allowed escaped symbols */
      case '\"':
      case '/':
      case '\\':
      case 'b':
      case 'f':
      case 'r':
      case 'n':
      case 't':
        break;
        /* Allows escaped symbol \uXXXX */
      case 'u':
        pos++;
        for (i = 0; i < 4 && pos < sourceLength && source[pos] != '\0'; i++)
        {
          /* If it isn't a hex character we have an error */
          if (!((source[pos] >= 48 && source[pos] <= 57) || /* 0-9 */
                (source[pos] >= 65 && source[pos] <= 70) || /* A-F */
                (source[pos] >= 97 && source[pos] <= 102)))
          { /* a-f */
            return false;
          }
          pos++;
        }
        pos--;
        break;
        /* Unexpected symbol */
      default:
        return false;
      }
    }
  }
  return false;
}

bool Json::tryCallCallbackAndPopStack(ParserState_t* stateStack, int& stateStackSize, 
                                      JsonAcceptType_t type,
                                      int valueStart, int valueLength, bool valueIsString)
{
  JsonAccept_t acceptArg;

  if (stateStackSize > 1)
  {
    ParserState_t* parentState = &stateStack[stateStackSize - 2];
    // We have parsed a primitive value at the beginning state, signal it
    // The parent can be either in Key state or in Array state
    fillAccept(&acceptArg, type, parentState, valueStart, valueLength, valueIsString);
    if (!accept.call(&acceptArg))
    {
      // Nothing to do, a primitive has no children to skip
    }
    // Now pop the state, putting it into the next state based on its current state
    if (parentState->state == PS_OBJECT_KEY)
    {
      parentState->state = PS_OBJECT_KEY_VALUE_END;
    } else if (parentState->state == PS_ARRAY) {
      parentState->state = PS_ARRAY_VALUE_END;
    } else {
      Os::assertFalse("Parent state is unexpected");
      return false;
    }
    stateStackSize -= 1;
  } else {
    // It's a top level object, nothing else must follow
    // TODO call accept callback to signal parse end
    stateStack[stateStackSize - 1].state = PS_END;
  }
  return true;
}

JSonParseResult_t Json::parse()
{
  pos = 0;

  JsonAccept_t acceptArg;

  const int MaxStack = 10;
  int stateStackSize = 0;
  ParserState_t stateStack[MaxStack];
  tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0);

  //TODO
  int skipDepth = 0; //If >0 we need to skip subtrees until we decrease it to 0

  for (; pos < sourceLength && source[pos] != '\0'; pos++)
  {
    // If we don't have a current state, it was popped because the value was parsed completely
    if (stateStackSize == 0)
    {
      return Json_Error_Invalid;
    }

    char c = source[pos];

    // Ignore all whitespace
    if (isWhitespace(c))
    {
      continue;
    }

    ParserState_t* currState = &stateStack[stateStackSize - 1];
    switch (currState->state)
    {
      case PS_START:
        //When starting we can have an object, an array
        if (c == '{' || c == '[')
        {
          bool isObj = (c == '{');
          if (stateStackSize > 1)
          {
            ParserState_t* parentState = &stateStack[stateStackSize - 2];
            // We have parsed a primitive value at the beginning state, signal it
            // The parent can be either in Key state or in Array state
            fillAccept(&acceptArg, isObj ? Json_Accept_ObjectBegin : Json_Accept_ArrayBegin, parentState, 0, 0, false);
            if (!accept.call(&acceptArg))
            {
              // TODO skip state
            }
          } else {
            // TODO It's a top level object, don't signal for now
          }
          // TODO Signal object begin
          currState->state = isObj ? PS_OBJECT : PS_ARRAY;
          // When entering PS_ARRAY, push a new PS_START state that will parse the first value
          if (!isObj)
          {
            if (!tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0))
              return Json_Error_NoMemory;
          }
        } else if (isStartOfPrimitiveValue(c) || c == '\"')
        {
          bool isString = (c == '\"');
          int valueStart;
          int valueLength;
          bool parsed;
          if (isString)
            parsed = parseString(valueStart, valueLength);
          else 
            parsed = parsePrimitive(valueStart, valueLength);

          if (!parsed)
            return Json_Error_Invalid;

          if (!tryCallCallbackAndPopStack(stateStack, stateStackSize, Json_Accept_KeyValue, valueStart, valueLength, isString))
            return Json_Error_Invalid;

        } else 
        {
          // Unexpected char
          return Json_Error_Invalid;
        }
        break;

      case PS_OBJECT:
        //When in object state, we can only have a key
        if (c == '\"')
        {
          if (!parseString(currState->keyStart, currState->keyLength))
            return Json_Error_Invalid;
          
          currState->state = PS_OBJECT_KEY;
        } else 
        {
          // Unexpected char
          return Json_Error_Invalid;
        }
        break;

      case PS_OBJECT_KEY:
        if (c == ':')
        {
          // We now need a value, to do this push a new PS_START state in the stack
          // This is equivalent to a function call
          // The next state will be set when popping the stack and it will be PS_OBJECT_KEY_VALUE_END
          if (!tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0))
            return Json_Error_NoMemory;
          
        } else 
        {
          // Unexpected char
          return Json_Error_Invalid;
        }
        break;

      case PS_OBJECT_KEY_VALUE_END:
        if (c == ',')
        {
          currState->state = PS_OBJECT;
        } else if (c == '}')
        {
          if (!tryCallCallbackAndPopStack(stateStack, stateStackSize, Json_Accept_ObjectEnd, 0, 0, false))
            return Json_Error_Invalid;
          
        } else 
        {
          // Unexpected char
          return Json_Error_Invalid;
        }
        break;

      case PS_ARRAY:
        // We never handle PS_ARRAY directly: when this state is set, a new PS_START is pushed.
        // When the state gets popped it goes to PS_ARRAY_VALUE_END which either terminates the array or pushes a new start
        return Json_Error_Invalid;

      case PS_ARRAY_VALUE_END:
        if (c == ',')
        {
          currState->state = PS_ARRAY;
          if (!tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0))
            return Json_Error_NoMemory;
          
        } else if (c == ']')
        {
          if (!tryCallCallbackAndPopStack(stateStack, stateStackSize, Json_Accept_ArrayEnd, 0, 0, false))
            return Json_Error_Invalid;
          
        } else 
        {
          // Unexpected char
          return Json_Error_Invalid;
        }
        break;

        // Unknown state
      default:
        return Json_Error_Invalid;
    }
  }

  return Json_Parsed;
}
