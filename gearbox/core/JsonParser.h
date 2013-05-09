#ifndef GEARBOX_JSON_PARSER_H
#define GEARBOX_JSON_PARSER_H

#include <yajl/yajl_parse.h>
#include <memory>
#include <gearbox/core/JsonSchema.h>

namespace Gearbox {

typedef bool (*SchemaErrorHandler)(const std::string & error, const std::string & context);

class Json::Parser {
public:
    static void parse(
        const std::string & content,
        Json & json,
        JsonSchema * schema = NULL
    );

private:

    struct Context {
        Json * cursor;
        JsonSchema * schema;
        SchemaErrorHandler errFunc;
        std::string exception;
    };

    static Json * getObj(void * ctx);
    static Json * getJson(void * ctx);
    static void   setJson(void * ctx, Json * j);
    static void   setJsonUp(void * ctx);
    static JsonSchema * getSchema(void * ctx);
    static void   setSchema(void * ctx, JsonSchema * j);
    static void   setSchemaUp(void * ctx);
    
    static int parse_null(void * ctx);
    static int parse_boolean(void * ctx, int boolVal);
    static int parse_number(void *ctx, const char *numberVal, size_t numberLen);
    static int parse_string(
        void * ctx, 
        const unsigned char * stringVal,
        size_t stringLen
    );
    static int parse_start_map(void * ctx);
    static int parse_map_key(
        void * ctx,
        const unsigned char * key,
        size_t stringLen
    );
    static int parse_end_map(void * ctx);
    static int parse_start_array(void * ctx);
    static int parse_end_array(void * ctx);

    /* static yajl_parser_config cfg; */
    static yajl_callbacks callbacks;
};

} // namespace
#endif // GEARBOX_JSON_PARSER_H
