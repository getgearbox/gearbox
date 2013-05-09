// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/core/logger.h>

using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(59);
    log_init("./unit.conf");

    JsonSchema s;
    Json j;

    // empty object, any  properties
    s.parse("{\"type\":\"object\"}");

    NOTHROW( j.parse("{}").validate(&s) );
    NOTHROW( j.parse("{\"key\":1}").validate(&s) );
    
    // requires 1 key of type number
    s.parse("{\"type\":\"object\",\"properties\":{\"key\":{\"type\":\"number\"}}}");

    NOTHROW( j.parse("{\"key\":1}").validate(&s) );
    THROWS( j.parse("{\"key\":\"string\"}").validate(&s), "Json Exception: schema does not allow for type \"string\" at: [\"key\"]" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: non-optional property \"key\" is missing" );

    s.parse("{\"type\":\"object\",\"properties\":{\"key\":{\"type\":\"number\",\"optional\":true}}}");

    NOTHROW( j.parse("{}").validate(&s) );
    OK( ! j.hasKey("key") );
    NOTHROW( j.parse("{\"key\":1}").validate(&s) );
    OK( j.hasKey("key") );
    THROWS( j.parse("{\"key\":\"string\"}").validate(&s), "Json Exception: schema does not allow for type \"string\" at: [\"key\"]" );

    s.parse("{\"type\":\"object\",\"properties\":{\"key\":{\"type\":\"number\",\"optional\":false}}}");
    
    NOTHROW( j.parse("{\"key\":1}").validate(&s) );
    THROWS( j.parse("{\"key\":\"string\"}").validate(&s), "Json Exception: schema does not allow for type \"string\" at: [\"key\"]" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: non-optional property \"key\" is missing" );
    
    s.parse("{\"type\":\"object\",\"properties\":{\"key\":{\"type\":\"number\",\"default\":123}}}");

    NOTHROW( j.parse("{}").validate(&s) );
    IS( j["key"].as<double>(), 123 );
    NOTHROW( j.parse("{\"key\":1}").validate(&s) );
    IS( j["key"].as<double>(), 1 );
    NOTHROW( j.parse("{\"key\":1.1234}").validate(&s) );
    IS( j["key"].as<double>(), 1.1234 );
    THROWS( j.parse("{\"key\":\"string\"}").validate(&s), "Json Exception: schema does not allow for type \"string\" at: [\"key\"]" );

    // set up object to have 2 keys, and "key" requires "otherkey"
    s.parse("{\"type\":\"object\",\"properties\":{\"key\":{\"type\":\"number\",\"optional\":true,\"requires\":\"otherkey\"},\"otherkey\":{\"type\":\"string\",\"optional\":true}}}");
    
    NOTHROW( j.parse("{}").validate(&s) );
    // key not required
    NOTHROW( j.parse("{\"otherkey\":\"string\"}" ) );
    // err, otherkey is required if key is set
    THROWS( j.parse("{\"key\":42}").validate(&s), 
            "Json Exception: property \"key\" requires additional property \"otherkey\" to be set according to schema" );
    // ok if both keys set
    NOTHROW( j.parse("{\"otherkey\":\"string\",\"key\":24}" ) );
    
    
    // now otherkey has default, so you can use "key" by itself, even those key requires otherkey
    s.parse("{\"type\":\"object\",\"properties\":{\"key\":{\"type\":\"number\",\"optional\":true,\"requires\":\"otherkey\"},\"otherkey\":{\"type\":\"string\",\"optional\":true,\"default\":\"abc\"}}}");
    
    NOTHROW( j.parse("{\"otherkey\":\"string\"}" ) );
    NOTHROW( j.parse("{\"key\":42}").validate(&s) );
    // verify the default value
    IS( j["otherkey"].as<std::string>(), "abc" );

    // test additionalProperties (as bool, allow any)
    s.parse("{\"type\":\"object\",\"additionalProperties\":true}");
    NOTHROW( j.parse("{}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":true}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":1}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":1.1}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":\"string\"}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":[]}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":{}}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":1,\"bar\":2}").validate(&s) );

    // test additionalProperties (as bool false, allow none)
    s.parse("{\"type\":\"object\",\"additionalProperties\":false}");
    NOTHROW( j.parse("{}").validate(&s) );
    THROWS( j.parse("{\"foo\":true}").validate(&s), "Json Exception: invalid property \"foo\": schema allows for no properties at: [\"foo\"]" );
    THROWS( j.parse("{\"foo\":1}").validate(&s), "Json Exception: invalid property \"foo\": schema allows for no properties at: [\"foo\"]" );
    THROWS( j.parse("{\"foo\":1.1}").validate(&s), "Json Exception: invalid property \"foo\": schema allows for no properties at: [\"foo\"]" );
    THROWS( j.parse("{\"foo\":\"string\"}").validate(&s), "Json Exception: invalid property \"foo\": schema allows for no properties at: [\"foo\"]" );
    THROWS( j.parse("{\"foo\":[]}").validate(&s), "Json Exception: invalid property \"foo\": schema allows for no properties at: [\"foo\"]" );
    THROWS( j.parse("{\"foo\":{}}").validate(&s), "Json Exception: invalid property \"foo\": schema allows for no properties at: [\"foo\"]" );
    THROWS( j.parse("{\"foo\":1,\"bar\":2}").validate(&s), "Json Exception: invalid property \"bar\": schema allows for no properties at: [\"bar\"]" );
    
    // test additionalProperties (as object, allow only ints)
    s.parse("{\"type\":\"object\",\"additionalProperties\":{\"type\":\"integer\"}}");

    NOTHROW( j.parse("{}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":1}").validate(&s) );
    NOTHROW( j.parse("{\"foo\":1,\"bar\":2}").validate(&s) );
    THROWS( j.parse("{\"foo\":1.1}").validate(&s), "Json Exception: schema does not allow for type \"number\" at: [\"foo\"]" );

    // object of objects
    s.parse(
        "{"
        "\"type\":\"object\","
        "\"properties\":{"
        "    \"A\":{"
        "        \"type\":\"object\","
        "        \"optional\":true,"
        "        \"requires\":\"B\","
        "        \"properties\":{"
        "            \"C\":{"
        "                \"type\":\"number\","
        "                \"optional\":true"
        "            },"
        "            \"D\":{"
        "                \"type\":\"any\","
        "                \"requires\":\"C\","
        "                \"optional\":true"
        "            },"
        "            \"E\":{"
        "                \"type\":\"object\","
        "                \"optional\":true,"
        "                \"additionalProperties\":{"
        "                    \"type\":\"integer\""
        "                }"
        "            }"
        "        }"
        "    },"
        "    \"B\":{"
        "        \"type\":\"string\","
        "        \"optional\":true"
        "    }"
        "}"
        "}"
    );

    NOTHROW( j.parse("{}").validate(&s) );
    NOTHROW( j.parse("{\"B\":\"value\"}").validate(&s) );
    NOTHROW( j.parse("{\"A\":{\"C\":123},\"B\":\"value\"}").validate(&s) );
    NOTHROW( j.parse("{\"A\":{\"C\":123,\"D\":true},\"B\":\"value\"}").validate(&s) );
    NOTHROW( j.parse("{\"A\":{\"C\":123,\"D\":true,\"E\":{\"a\":1,\"b\":2,\"c\":3}},\"B\":\"value\"}").validate(&s) );
    NOTHROW( j.parse("{\"A\":{\"C\":123,\"D\":true,\"E\":{}},\"B\":\"value\"}").validate(&s) );
    THROWS( j.parse("{\"A\":{\"C\":123,\"D\":true,\"E\":{\"a\":1.1}},\"B\":\"value\"}").validate(&s), 
            "Json Exception: schema does not allow for type \"number\" at: [\"A\"][\"E\"][\"a\"]" );
    THROWS( j.parse("{\"A\":{\"C\":false,\"D\":true},\"B\":\"value\"}").validate(&s), 
            "Json Exception: schema does not allow for type \"boolean\" at: [\"A\"][\"C\"]" );
    THROWS( j.parse("{\"A\":{\"D\":true},\"B\":\"value\"}").validate(&s), 
            "Json Exception: property \"D\" requires additional property \"C\" to be set according to schema at: [\"A\"]" );
    THROWS( j.parse("{\"A\":{\"C\":123,\"D\":true}}").validate(&s), 
            "Json Exception: property \"A\" requires additional property \"B\" to be set according to schema" );

    s.parse("{\"type\":\"object\",\"properties\":{\"array\":{\"type\":\"array\",\"default\":[]}}}");
    NOTHROW( j.parse("{}").validate(&s) );
    IS( j.serialize(), "{\"array\":[]}" );
             
    TEST_END;
}
