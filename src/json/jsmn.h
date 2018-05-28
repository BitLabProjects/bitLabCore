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

#ifndef __JSMN_H_
#define __JSMN_H_

#include <stddef.h>

#include "platform/Callback.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*
  Modified version of original jsmn lib ... added a type "JSMN_KEY" and enabled
  parent-pointers and strict JSON check.
*/
    /**
     * JSON type identifier. Basic types are:
     *  o Object
     *  o Array
     *  o String
     *  o Other primitive: number, boolean (true/false) or null
     */
    typedef enum
    {
        JSMN_UNDEFINED = 0,
        JSMN_OBJECT = 1,
        JSMN_ARRAY = 2,
        JSMN_STRING = 3,
        JSMN_PRIMITIVE = 4,
        JSMN_KEY = 5
    } jsmntype_t;

    enum jsmnerr
    {
        /* Not enough tokens were provided */
        JSMN_ERROR_NOMEM = -1,
        /* Invalid character inside JSON string */
        JSMN_ERROR_INVAL = -2,
        /* The string is not a full JSON packet, more bytes expected */
        JSMN_ERROR_PART = -3
    };

    /**
     * JSON token description.
     * @param       type    type (object, array, string etc.)
     * @param       start   start position in JSON data string
     * @param       end     end position in JSON data string
     */
    typedef struct
    {
            jsmntype_t type;
            int start;
            int end;
            int parent;
            int childCount;
    } jsmntok_t;


    typedef enum
    {
        JSMN_Accept_KeyValue = 0,
        JSMN_Accept_ObjectBegin = 1,
        JSMN_Accept_ObjectEnd = 2,
        JSMN_Accept_ArrayBegin = 3,
        JSMN_Accept_ArrayEnd = 4
    } jsmnaccepttype_t;

    typedef struct
    {
      jsmnaccepttype_t type;
      int tokenIdx;
      const char* key;
      int keyLength;
      bool valueIsString;
      const char* value;
      int valueLength;
    } jsmnaccept_t;

    /**
     * JSON parser. Contains an array of token blocks available. Also stores
     * the string being parsed now and current position in that string
     */
    typedef struct
    {
            unsigned int pos; /* offset in the JSON string */
            unsigned int toknext; /* next token to allocate */
            int toksuper; /* superior token node, e.g parent object or array */
    } jsmn_parser;

    /**
     * Create JSON parser over an array of tokens
     */
    void jsmn_init ( jsmn_parser *parser );

    /**
     * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
     * a single JSON object.
     */
    int jsmn_parse ( jsmn_parser *parser, 
                     const char *js, 
                     size_t len,
                     mbed::Callback<bool(const jsmnaccept_t*)> accept);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */

