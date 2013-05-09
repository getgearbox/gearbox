// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/core/logger.h>

using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(144);
    log_init("./unit.conf");

    JsonSchema s;
    Json j;

    // test string minLength, maxLength
    
    s.parse("{\"type\":\"string\",\"minLength\":5,\"maxLength\":10}");
    THROWS( j.parse("90").validate(&s), "Json Exception: schema does not allow for type \"integer\"" );

    NOTHROW( j.parse("\"string\"").validate(&s) );
    NOTHROW( j.parse("\"12345\"").validate(&s) );
    NOTHROW( j.parse("\"1234567890\"").validate(&s) );
    THROWS( j.parse("\"1234\"").validate(&s), "Json Exception: string length 4 is less than schema minimum length 5" );
    THROWS( j.parse("\"12345678901\"").validate(&s), "Json Exception: string length 11 is larger than schema maximum length 10" );

    // test string enum
    s.parse("{\"type\":\"string\",\"enum\": [\"foo\", \"bar\", \"baz\"]}");
    NOTHROW( j.parse("\"foo\"").validate(&s) );
    NOTHROW( j.parse("\"bar\"").validate(&s) );
    NOTHROW( j.parse("\"baz\"").validate(&s) );
    THROWS( j.parse("\"fo\"").validate(&s), "Json Exception: value is not valid in schema enum" );
    THROWS( j.parse("\"fooo\"").validate(&s), "Json Exception: value is not valid in schema enum" );
    THROWS( j.parse("\"string\"").validate(&s), "Json Exception: value is not valid in schema enum" );
    THROWS( j.parse("\"\"").validate(&s), "Json Exception: value is not valid in schema enum" );

    // test mixed enum
    s.parse("{\"type\":\"any\",\"enum\": [\"string\", 90, 123.456, true]}");
    NOTHROW( j.parse("\"string\"").validate(&s) );
    IS( j.typeName(), "string" );
    NOTHROW( j.parse("90").validate(&s) );
    IS( j.typeName(), "integer" );
    NOTHROW( j.parse("123.456").validate(&s) );
    IS( j.typeName(), "double" );
    NOTHROW( j.parse("true").validate(&s) );
    IS( j.typeName(), "boolean" );
    THROWS( j.parse("\"foo\"").validate(&s), "Json Exception: value is not valid in schema enum" );
    THROWS( j.parse("91").validate(&s), "Json Exception: value is not valid in schema enum" );
    THROWS( j.parse("false").validate(&s), "Json Exception: value is not valid in schema enum" );
    THROWS( j.parse("123.457").validate(&s), "Json Exception: value is not valid in schema enum" );
        
    // test date-time format for string
    s.parse("{\"type\":\"string\",\"format\":\"date-time\"}");
    NOTHROW( j.parse("\"2009-10-30T14:58:34Z\"").validate(&s) );
    NOTHROW( j.parse("\"2009-10-30T14:58:34+08:00\"").validate(&s) );
    NOTHROW( j.parse("\"2009-10-30T14:58:34-05:00\"").validate(&s) );
    // does not allow TZ
    THROWS( j.parse("\"2009-10-30T14:58:34UTC\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // requires 4 digit years
    THROWS( j.parse("\"09-13-30T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus months
    THROWS( j.parse("\"2009-13-30T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus days
    THROWS( j.parse("\"2009-12-32T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus days (Nov only 31 days)
    THROWS( j.parse("\"2009-11-31T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    
    // test date-time format for string

    s.parse("{\"type\":\"string\",\"format\":\"date-time\"}");
    NOTHROW( j.parse("\"2009-10-30T14:58:34Z\"").validate(&s) );
    NOTHROW( j.parse("\"2009-10-30T14:58:34+08:00\"").validate(&s) );
    NOTHROW( j.parse("\"2009-10-30T14:58:34-05:00\"").validate(&s) );
    // does not allow TZ
    THROWS( j.parse("\"2009-10-30T14:58:34UTC\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // requires 4 digit years
    THROWS( j.parse("\"09-13-30T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus months
    THROWS( j.parse("\"2009-13-30T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus days
    THROWS( j.parse("\"2009-12-32T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus days (Nov only 31 days)
    THROWS( j.parse("\"2009-11-31T14:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus hours
    THROWS( j.parse("\"2009-12-31T24:58:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus minutes
    THROWS( j.parse("\"2009-12-31T14:60:34Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus seconds
    THROWS( j.parse("\"2009-12-31T14:59:60Z\"").validate(&s), "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus offset hour
    THROWS( j.parse("\"2009-12-31T14:59:59+24:00\"").validate(&s), 
            "Json Exception: value does not match required schema format date-time" );
    // does not allow bogus offset min
    THROWS( j.parse("\"2009-12-31T14:59:59+24:60\"").validate(&s), 
            "Json Exception: value does not match required schema format date-time" );

    // test date format for string

    s.parse("{\"type\":\"string\",\"format\":\"date\"}");

    NOTHROW( j.parse("\"2009-10-30\"").validate(&s) );
    // requires 4 digit years
    THROWS( j.parse("\"09-13-30\"").validate(&s), "Json Exception: value does not match required schema format date" );
    // does not allow bogus months
    THROWS( j.parse("\"2009-13-30\"").validate(&s), "Json Exception: value does not match required schema format date" );
    // does not allow bogus days
    THROWS( j.parse("\"2009-12-32\"").validate(&s), "Json Exception: value does not match required schema format date" );
    // does not allow bogus days (Nov only 31 days)
    THROWS( j.parse("\"2009-11-31\"").validate(&s), "Json Exception: value does not match required schema format date" );

    // test uri format for string

    s.parse("{\"type\":\"string\",\"format\":\"uri\"}");
    
    NOTHROW( j.parse("\"http://www.example.com\"").validate(&s) );
    NOTHROW( j.parse("\"https://www.example.com\"").validate(&s) );
    NOTHROW( j.parse("\"http://www.example.com:80\"").validate(&s) );
    NOTHROW( j.parse("\"http://localhost:80/cc/v1/hypervisor\"").validate(&s) );
    
    THROWS( j.parse("\"http:/www.example.com\"").validate(&s), "Json Exception: value does not match required schema format uri" );
    THROWS( j.parse("\"httpq://www.example.com\"").validate(&s), "Json Exception: value does not match required schema format uri" );
    THROWS( j.parse("\"foo://www.example.com\"").validate(&s), "Json Exception: value does not match required schema format uri" );

    // test ip-address format for string
    
    s.parse("{\"type\":\"string\",\"format\":\"ip-address\"}");
    
    NOTHROW( j.parse("\"123.123.123.123\"").validate(&s) );
    NOTHROW( j.parse("\"1.1.1.1\"").validate(&s) );
    NOTHROW( j.parse("\"0.0.0.0\"").validate(&s) );
    THROWS( j.parse("\"256.0.0.0\"").validate(&s), "Json Exception: value does not match required schema format ip-address" );
    THROWS( j.parse("\"0.256.0.0\"").validate(&s), "Json Exception: value does not match required schema format ip-address" );
    THROWS( j.parse("\"0.0.256.0\"").validate(&s), "Json Exception: value does not match required schema format ip-address" );
    THROWS( j.parse("\"0.0.0.256\"").validate(&s), "Json Exception: value does not match required schema format ip-address" );
    THROWS( j.parse("\"0.0.0\"").validate(&s), "Json Exception: value does not match required schema format ip-address" );

    s.parse("{\"type\":\"string\",\"format\":\"ipv6\"}");

    NOTHROW( j.parse("\"fe80:0000:0000:0000:0204:61ff:fe9d:f156\"").validate(&s) );
    NOTHROW( j.parse("\"fe80:0:0:0:204:61ff:fe9d:f156\"").validate(&s) );
    NOTHROW( j.parse("\"fe80::204:61ff:fe9d:f156\"").validate(&s) );
    NOTHROW( j.parse("\"fe80:0000:0000:0000:0204:61ff:254.157.241.86\"").validate(&s) );
    NOTHROW( j.parse("\"fe80:0:0:0:0204:61ff:254.157.241.86\"").validate(&s) );
    NOTHROW( j.parse("\"fe80::204:61ff:254.157.241.86\"").validate(&s) );
    NOTHROW( j.parse("\"::1\"").validate(&s) );
    
    THROWS( j.parse("\"1111:2222:3333:4444:5555:\"").validate(&s), "Json Exception: value does not match required schema format ipv6" );
    THROWS( j.parse("\"1111:2222:3333::5555:\"").validate(&s), "Json Exception: value does not match required schema format ipv6" );
    THROWS( j.parse("\"1111:2222::5555:\"").validate(&s), "Json Exception: value does not match required schema format ipv6" );
    THROWS( j.parse("\"1111::5555:\"").validate(&s), "Json Exception: value does not match required schema format ipv6" );
    THROWS( j.parse("\"::5555:\"").validate(&s), "Json Exception: value does not match required schema format ipv6" );
    THROWS( j.parse("\"123.123.123.123\"").validate(&s), "Json Exception: value does not match required schema format ipv6" );

    // test mac-address format for string

    s.parse("{\"type\":\"string\",\"format\":\"mac-address\"}");

    NOTHROW( j.parse("\"a:b:c:d:e:f\"").validate(&s) );
    NOTHROW( j.parse("\"1:2:3:4:5:6\"").validate(&s) );
    NOTHROW( j.parse("\"A:B:C:D:E:F\"").validate(&s) );
    NOTHROW( j.parse("\"aa:bb:00:11:AA:BB\"").validate(&s) );
    
    THROWS( j.parse("\"a:b:c:d:e:\"").validate(&s), "Json Exception: value does not match required schema format mac-address" );
    THROWS( j.parse("\"a:b:c:d:e:fff\"").validate(&s), "Json Exception: value does not match required schema format mac-address" );
    THROWS( j.parse("\"a:b:c:d:e:g\"").validate(&s), "Json Exception: value does not match required schema format mac-address" );
    THROWS( j.parse("\"a:b:c:d:e:-1\"").validate(&s), "Json Exception: value does not match required schema format mac-address" );

    // test pattern for string
    
    s.parse("{\"type\":\"string\",\"pattern\":\"a+b+z\"}");
    
    NOTHROW( j.parse("\"abz\"").validate(&s) );
    NOTHROW( j.parse("\"aaaaabz\"").validate(&s) );
    NOTHROW( j.parse("\"aaaaabbbbbbbz\"").validate(&s) );
    THROWS( j.parse("\"abzz\"").validate(&s), "Json Exception: value does not match schema pattern \"a+b+z\"" );

    // test pattern with back-reference \1
    s.parse("{\"type\":\"string\",\"pattern\":\"(...).*\\\\1\"}");

    NOTHROW( j.parse("\"foofoo\"").validate(&s) );
    NOTHROW( j.parse("\"foofoofoo\"").validate(&s) );
    NOTHROW( j.parse("\"aaafooaaa\"").validate(&s) );
    NOTHROW( j.parse("\"_.--.__.-\"").validate(&s) );
    THROWS( j.parse("\"a\"").validate(&s), "Json Exception: value does not match schema pattern \"(...).*\\1\"" );
    THROWS( j.parse("\"foobar\"").validate(&s), "Json Exception: value does not match schema pattern \"(...).*\\1\"" );

    // test min/max for integer
    
    s.parse("{\"type\":\"integer\",\"minimum\":10,\"maximum\":15}");
    
    NOTHROW( j.parse("10").validate(&s) );
    NOTHROW( j.parse("11").validate(&s) );
    NOTHROW( j.parse("12").validate(&s) );
    NOTHROW( j.parse("13").validate(&s) );
    NOTHROW( j.parse("14").validate(&s) );
    NOTHROW( j.parse("15").validate(&s) );
    THROWS( j.parse("16").validate(&s), "Json Exception: value 16 is larger than schema maximum value 15" );
    THROWS( j.parse("9").validate(&s), "Json Exception: value 9 is less than schema minimum value 10" );

    // test min/max for double (number)

    s.parse("{\"type\":\"number\",\"minimum\":10.1,\"maximum\":14.9}");
 
    NOTHROW( j.parse("10.100000000001").validate(&s) );
    NOTHROW( j.parse("10.1").validate(&s) );
    NOTHROW( j.parse("11").validate(&s) );
    NOTHROW( j.parse("12.4").validate(&s) );
    NOTHROW( j.parse("13").validate(&s) );
    NOTHROW( j.parse("14.5").validate(&s) );
    NOTHROW( j.parse("14.899999999999").validate(&s) );
    THROWS( j.parse("9.9999999").validate(&s), "Json Exception: value 9.9999999 is less than schema minimum value 10.1" );
    THROWS( j.parse("15.0000001").validate(&s), "Json Exception: value 15.0000001 is larger than schema maximum value 14.9" );
    THROWS( j.parse("10").validate(&s), "Json Exception: value 10 is less than schema minimum value 10.1" );
    THROWS( j.parse("14.91").validate(&s), "Json Exception: value 14.91 is larger than schema maximum value 14.9" );

    // test maxDecimal for double

    s.parse("{\"type\":\"number\",\"maxDecimal\":5}");

    NOTHROW( j.parse("10").validate(&s) );
    NOTHROW( j.parse("10.1").validate(&s) );
    NOTHROW( j.parse("10.12").validate(&s) );
    NOTHROW( j.parse("10.123").validate(&s) );
    NOTHROW( j.parse("10.1234").validate(&s) );
    NOTHROW( j.parse("10.12345").validate(&s) );
    THROWS( j.parse("10.123456").validate(&s), 
            "Json Exception: value 10.123456 has 6 decimal places, but the schema has specified maximum of 5 decimal places");
    NOTHROW( j.parse("10.000000000000").validate(&s) );
    THROWS( j.parse("10.000000000001").validate(&s), 
            "Json Exception: value 10.000000000001 has 12 decimal places, but the schema has specified maximum of 5 decimal places");

    // test enum for integer

    s.parse("{\"type\":\"integer\",\"enum\": [1,2,3,4]}");
    NOTHROW( j.parse("1").validate(&s) );
    NOTHROW( j.parse("2").validate(&s) );
    NOTHROW( j.parse("3").validate(&s) );
    NOTHROW( j.parse("4").validate(&s) );
    THROWS( j.parse("5").validate(&s), "Json Exception: value is not valid in schema enum");
    THROWS( j.parse("0").validate(&s), "Json Exception: value is not valid in schema enum");

    // test enum for double

    s.parse("{\"type\":\"number\",\"enum\": [1.1,1.123,3.4567,1.12312]}");

    NOTHROW( j.parse("1.1").validate(&s) );
    NOTHROW( j.parse("1.123").validate(&s) );
    NOTHROW( j.parse("3.4567").validate(&s) );
    NOTHROW( j.parse("1.12312").validate(&s) );
    NOTHROW( j.parse("1.10").validate(&s) );
    THROWS( j.parse("5").validate(&s), "Json Exception: value is not valid in schema enum");
    THROWS( j.parse("0").validate(&s), "Json Exception: value is not valid in schema enum");
    THROWS( j.parse("1.11").validate(&s), "Json Exception: value is not valid in schema enum");
    THROWS( j.parse("0.123").validate(&s), "Json Exception: value is not valid in schema enum");
    THROWS( j.parse("3.45678").validate(&s), "Json Exception: value is not valid in schema enum");
    THROWS( j.parse("1.123312").validate(&s), "Json Exception: value is not valid in schema enum");

    // test enum for bool

    s.parse("{\"type\":\"boolean\",\"enum\": [false]}");
    NOTHROW( j.parse("false").validate(&s) );
    THROWS( j.parse("true").validate(&s), "Json Exception: value is not valid in schema enum" );
    
    TEST_END;
}
