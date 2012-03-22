#ifndef GEARBOX_JSON_SCHEMA_H
#define GEARBOX_JSON_SCHEMA_H

#include <gearbox/core/Json.h>
#include <map>

namespace Gearbox {

class JsonSchema : public Json {
    typedef Json super;
public:
    bool validMaxSize(const Json & array);
    bool validMinSize(const Json & array);
    bool validType(const Json & json);
    bool validValue(const Json & json);
    bool validRequires(const Json & object);
    bool setDefaults(Json & object);
    void typeFixup(Json & object);

    const JsonSchema & operator[](const std::string & key) const;
    const JsonSchema & operator[](int i) const;
    JsonSchema & operator[](const std::string & key);
    JsonSchema & operator[](int i);

    JsonSchema * getAdditionalProperties();
    JsonSchema * getChildSchema(int index, const Json & json);
    JsonSchema * getChildSchema(const std::string & key, const Json & json);

    // register new formats to validate against
    typedef bool (*FormatChecker)(const std::string & value);
    static void setFormat(const std::string & name, FormatChecker);

    static void defaultFormat(const std::string & name);
    
 protected:
    template<typename T>
    bool validValue_(const T & value);
    struct FormatMap : public std::map< std::string, FormatChecker> {
        FormatMap();
    };
    static FormatMap formats_;
    static std::string default_format;
};

} // namespace

#endif // GEARBOX_JSON_SCHEMA_H
