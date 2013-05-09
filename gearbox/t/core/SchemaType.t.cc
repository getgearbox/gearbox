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
    j.setSchema(&s);

    // checking single type

    s.parse("{\"type\":\"string\"}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    NOTHROW( j.parse("\"string\"") );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"number\"}");

    NOTHROW( j.parse("90") );
    NOTHROW( j.parse("123.456") );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"integer\"}");
    
    NOTHROW( j.parse("90") );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"boolean\"}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    NOTHROW( j.parse("true") );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"object\"}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    NOTHROW( j.parse("{}") );

    s.parse("{\"type\":\"array\"}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    NOTHROW( j.parse("[]") );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );
    
    s.parse("{\"type\":\"null\"}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    NOTHROW( j.parse("null") );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":\"any\"}");

    NOTHROW( j.parse("90") );
    NOTHROW( j.parse("123.456") );
    NOTHROW( j.parse("\"string\"") );
    NOTHROW( j.parse("true") );
    NOTHROW( j.parse("null") );
    NOTHROW( j.parse("[]") );
    NOTHROW( j.parse("{}") );

    // checking type as an array 

    s.parse("{\"type\":[\"string\"]}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    NOTHROW( j.parse("\"string\"") );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"number\"]}");

    NOTHROW( j.parse("90") );
    NOTHROW( j.parse("123.456") );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"integer\"]}");
    
    NOTHROW( j.parse("90") );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"boolean\"]}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    NOTHROW( j.parse("true") );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"object\"]}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    NOTHROW( j.parse("{}") );

    s.parse("{\"type\":[\"array\"]}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null"), "Json Exception: schema does not allow for type \"null\"" );
    NOTHROW( j.parse("[]") );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );
    
    s.parse("{\"type\":[\"null\"]}");

    THROWS( j.parse("90"), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456"), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\""), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true"), "Json Exception: schema does not allow for type \"boolean\"" );
    NOTHROW( j.parse("null") );
    THROWS( j.parse("[]"), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}"), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"type\":[\"any\"]}");

    NOTHROW( j.parse("90") );
    NOTHROW( j.parse("123.456") );
    NOTHROW( j.parse("\"string\"") );
    NOTHROW( j.parse("true") );
    NOTHROW( j.parse("null") );
    NOTHROW( j.parse("[]") );
    NOTHROW( j.parse("{}") );

    s.parse("{\"type\":[\"string\",\"number\",\"integer\",\"boolean\",\"object\",\"array\",\"null\"]}");

    NOTHROW( j.parse("90") );
    NOTHROW( j.parse("123.456") );
    NOTHROW( j.parse("\"string\"") );
    NOTHROW( j.parse("true") );
    NOTHROW( j.parse("null") );
    NOTHROW( j.parse("[]") );
    NOTHROW( j.parse("{}") );

    TEST_END;
}
