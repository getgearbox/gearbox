#ifndef GEARBOX_JSON_PRIVATE_H
#define GEARBOX_JSON_PRIVATE_H
#include <gearbox/core/Json.h>
#include <gearbox/core/JsonSchema.h>
#include <string>
namespace Gearbox {
    struct Json::Private {
        Json * parent;
        Type type;
        JsonSchema * schema;
        mutable std::string error;
        ErrorHandler errHandler;
        Private(Type t = Json::UNDEF)
            : parent(NULL),
              type(t),
              schema(NULL),
              errHandler(Json::onErrorThrow) {}
    };
}
#endif
