// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/core/logger.h>

using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(28);
    log_init("./unit.conf");

    JsonSchema s;
    Json j;

    // test disallow
    
    s.parse("{\"disallow\":\"string\"}");
    NOTHROW( j.parse("90").validate(&s) );
    NOTHROW( j.parse("123.456").validate(&s) );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    NOTHROW( j.parse("true").validate(&s) );
    NOTHROW( j.parse("null").validate(&s) );
    NOTHROW( j.parse("[]").validate(&s) );
    NOTHROW( j.parse("{}").validate(&s) );

    s.parse("{\"disallow\":\"any\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"disallow\":[\"string\",\"number\",\"integer\",\"boolean\",\"object\",\"array\",\"null\"]}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: schema does not allow for type \"string\"" );
    THROWS( j.parse("true").validate(&s), "Json Exception: schema does not allow for type \"boolean\"" );
    THROWS( j.parse("null").validate(&s), "Json Exception: schema does not allow for type \"null\"" );
    THROWS( j.parse("[]").validate(&s), "Json Exception: schema does not allow for type \"array\"" );
    THROWS( j.parse("{}").validate(&s), "Json Exception: schema does not allow for type \"object\"" );

    s.parse("{\"disallow\":\"number\"}");

    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );
    THROWS( j.parse("123.456").validate(&s), "Json Exception: schema does not allow for type \"number\"" );
    NOTHROW( j.parse("\"string\"").validate(&s) );
    NOTHROW( j.parse("true").validate(&s) );
    NOTHROW( j.parse("null").validate(&s) );
    NOTHROW( j.parse("[]").validate(&s) );
    NOTHROW( j.parse("{}").validate(&s) );

    TEST_END;
}
