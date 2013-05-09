// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/core/logger.h>

using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(119);
    log_init("./unit.conf");

    JsonSchema s;
    Json j;

    // checking single type

    s.parse("{\"type\":\"string\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    NOTHROW( j.parse("\"string\"").validate(&s) );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"number\"}");

    NOTHROW( j.parse("90").validate(&s) );
    NOTHROW( j.parse("123.456").validate(&s) );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"integer\"}");
    
    NOTHROW( j.parse("90").validate(&s) );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"boolean\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    NOTHROW( j.parse("true").validate(&s) );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"object\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    NOTHROW( j.parse("{}").validate(&s) );

    s.parse("{\"type\":\"array\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    NOTHROW( j.parse("[]").validate(&s) );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );
    
    s.parse("{\"type\":\"null\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    NOTHROW( j.parse("null").validate(&s) );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"any\"}");

    NOTHROW( j.parse("90").validate(&s) );
    NOTHROW( j.parse("123.456").validate(&s) );
    NOTHROW( j.parse("\"string\"").validate(&s) );
    NOTHROW( j.parse("true").validate(&s) );
    NOTHROW( j.parse("null").validate(&s) );
    NOTHROW( j.parse("[]").validate(&s) );
    NOTHROW( j.parse("{}").validate(&s) );

    // checking type as an array 

    s.parse("{\"type\":[\"string\"]}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    NOTHROW( j.parse("\"string\"").validate(&s) );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"number\"]}");

    NOTHROW( j.parse("90").validate(&s) );
    NOTHROW( j.parse("123.456").validate(&s) );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"integer\"]}");
    
    NOTHROW( j.parse("90").validate(&s) );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"boolean\"]}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    NOTHROW( j.parse("true").validate(&s) );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"object\"]}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    NOTHROW( j.parse("{}").validate(&s) );

    s.parse("{\"type\":[\"array\"]}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    NOTHROW( j.parse("[]").validate(&s) );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );
    
    s.parse("{\"type\":[\"null\"]}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    NOTHROW( j.parse("null").validate(&s) );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"any\"]}");

    NOTHROW( j.parse("90").validate(&s) );
    NOTHROW( j.parse("123.456").validate(&s) );
    NOTHROW( j.parse("\"string\"").validate(&s) );
    NOTHROW( j.parse("true").validate(&s) );
    NOTHROW( j.parse("null").validate(&s) );
    NOTHROW( j.parse("[]").validate(&s) );
    NOTHROW( j.parse("{}").validate(&s) );

    s.parse("{\"type\":[\"string\",\"number\",\"integer\",\"boolean\",\"object\",\"array\",\"null\"]}");

    NOTHROW( j.parse("90").validate(&s) );
    NOTHROW( j.parse("123.456").validate(&s) );
    NOTHROW( j.parse("\"string\"").validate(&s) );
    NOTHROW( j.parse("true").validate(&s) );
    NOTHROW( j.parse("null").validate(&s) );
    NOTHROW( j.parse("[]").validate(&s) );
    NOTHROW( j.parse("{}").validate(&s) );

    TEST_END;
}
