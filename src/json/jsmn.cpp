/* Author: Faheem Inayat
 * Created by "Night Crue" Team @ TechShop San Jose, CA
 *
 * --- DISCLAIMER ---
 * This code is a modified version of original JSMN lirary, written by
 *    *** Serge A. Zaitsev ***
 * and hosted at https://github.com/zserge/jsmn
 * Any modification to the original source is not guaranteed to be included
 * in this version.  As of writing of this file, the original source is 
 * licensed under MIT License
 * (http://www.opensource.org/licenses/mit-license.php).
 */

#include "jsmn.h"

#include "..\os\os.h"

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

void fillAccept(jsmnaccept_t *acceptArg, jsmnaccepttype_t type, const char *js, ParserState_t *parentState, int valueStart, int valueLength, bool valueIsString = false)
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
    acceptArg->key = &js[parentState->keyStart];
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
    acceptArg->value = &js[valueStart];
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

bool tryPushStack(ParserState_t* stack, int& size, int maxSize,
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
static bool jsmn_parse_primitive(jsmn_parser *parser, const char *js, size_t len, int& start, int& length)
{
  jsmntok_t *token;
  start = parser->pos;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    switch (js[parser->pos])
    {
      case '\t':
      case '\r':
      case '\n':
      case ' ':
      case ',':
      case ']':
      case '}':
      {
        length = parser->pos - start;
        // pos is pointing to the char that terminated the primitive value, 
        // go back one char as it does not belong to the primitive value
        parser->pos--; 
        return true;
      }
    }
    if (js[parser->pos] < 32 || js[parser->pos] >= 127)
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
static bool jsmn_parse_string(jsmn_parser *parser, const char *js, size_t len, int& start, int& length)
{
  jsmntok_t *token;
  start = parser->pos;
  parser->pos++;

  /* Skip starting quote */
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    char c = js[parser->pos];

    /* Quote: end of string */
    if (c == '\"')
    {
      start = start + 1;
      length = parser->pos - start;
      // Do not increment parser->pos, the caller will do it in the next iteration cycle
      return true;
    }

    /* Backslash: Quoted symbol expected */
    if (c == '\\' && parser->pos + 1 < len)
    {
      int i;
      parser->pos++;
      switch (js[parser->pos])
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
        parser->pos++;
        for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++)
        {
          /* If it isn't a hex character we have an error */
          if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
                (js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
                (js[parser->pos] >= 97 && js[parser->pos] <= 102)))
          { /* a-f */
            return false;
          }
          parser->pos++;
        }
        parser->pos--;
        break;
        /* Unexpected symbol */
      default:
        return false;
      }
    }
  }
  return false;
}

bool tryCallCallbackAndPopStack(const char *js, 
                                ParserState_t* stateStack, int& stateStackSize, 
                                jsmnaccepttype_t type,
                                int valueStart, int valueLength, bool valueIsString,
                                mbed::Callback<bool(const jsmnaccept_t *)> accept)
{
  jsmnaccept_t acceptArg;

  if (stateStackSize > 1)
  {
    ParserState_t* parentState = &stateStack[stateStackSize - 2];
    // We have parsed a primitive value at the beginning state, signal it
    // The parent can be either in Key state or in Array state
    fillAccept(&acceptArg, type, js, parentState, valueStart, valueLength, valueIsString);
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

int jsmn_parse(jsmn_parser *parser, const char *js, size_t len, mbed::Callback<bool(const jsmnaccept_t *)> accept)
{
  jsmnaccept_t acceptArg;

  const int MaxStack = 10;
  int stateStackSize = 0;
  ParserState_t stateStack[MaxStack];
  tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0);

  //TODO
  int skipDepth = 0; //If >0 we need to skip subtrees until we decrease it to 0

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    // If we don't have a current state, it was popped because the value was parsed completely
    if (stateStackSize == 0)
    {
      return JSMN_ERROR_INVAL;
    }

    char c = js[parser->pos];

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
            fillAccept(&acceptArg, isObj ? JSMN_Accept_ObjectBegin : JSMN_Accept_ArrayBegin, js, parentState, 0, 0, false);
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
              return JSMN_ERROR_NOMEM;
          }
        } else if (isStartOfPrimitiveValue(c) || c == '\"')
        {
          bool isString = (c == '\"');
          int valueStart;
          int valueLength;
          bool parsed;
          if (isString)
            parsed = jsmn_parse_string(parser, js, len, valueStart, valueLength);
          else 
            parsed = jsmn_parse_primitive(parser, js, len, valueStart, valueLength);

          if (!parsed)
            return JSMN_ERROR_INVAL;

          if (!tryCallCallbackAndPopStack(js, stateStack, stateStackSize, JSMN_Accept_KeyValue, valueStart, valueLength, isString, accept))
            return JSMN_ERROR_INVAL;

        } else 
        {
          // Unexpected char
          return JSMN_ERROR_INVAL;
        }
        break;

      case PS_OBJECT:
        //When in object state, we can only have a key
        if (c == '\"')
        {
          if (!jsmn_parse_string(parser, js, len, currState->keyStart, currState->keyLength))
            return JSMN_ERROR_INVAL;
          
          currState->state = PS_OBJECT_KEY;
        } else 
        {
          // Unexpected char
          return JSMN_ERROR_INVAL;
        }
        break;

      case PS_OBJECT_KEY:
        if (c == ':')
        {
          // We now need a value, to do this push a new PS_START state in the stack
          // This is equivalent to a function call
          // The next state will be set when popping the stack and it will be PS_OBJECT_KEY_VALUE_END
          if (!tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0))
            return JSMN_ERROR_NOMEM;
          
        } else 
        {
          // Unexpected char
          return JSMN_ERROR_INVAL;
        }
        break;

      case PS_OBJECT_KEY_VALUE_END:
        if (c == ',')
        {
          currState->state = PS_OBJECT;
        } else if (c == '}')
        {
          if (!tryCallCallbackAndPopStack(js, stateStack, stateStackSize, JSMN_Accept_ObjectEnd, 0, 0, false, accept))
            return JSMN_ERROR_INVAL;
          
        } else 
        {
          // Unexpected char
          return JSMN_ERROR_INVAL;
        }
        break;

      case PS_ARRAY:
        // We never handle PS_ARRAY directly: when this state is set, a new PS_START is pushed.
        // When the state gets popped it goes to PS_ARRAY_VALUE_END which either terminates the array or pushes a new start
        return JSMN_ERROR_INVAL;

      case PS_ARRAY_VALUE_END:
        if (c == ',')
        {
          currState->state = PS_ARRAY;
          if (!tryPushStack(stateStack, stateStackSize, MaxStack, PS_START, 0, 0))
            return JSMN_ERROR_NOMEM;
          
        } else if (c == ']')
        {
          if (!tryCallCallbackAndPopStack(js, stateStack, stateStackSize, JSMN_Accept_ArrayEnd, 0, 0, false, accept))
            return JSMN_ERROR_INVAL;
          
        } else 
        {
          // Unexpected char
          return JSMN_ERROR_INVAL;
        }
        break;

        // Unknown state
      default:
        return JSMN_ERROR_INVAL;
    }
  }

  return 1;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser)
{
  parser->pos = 0;
  parser->toknext = 0;
  parser->toksuper = -1;
}
