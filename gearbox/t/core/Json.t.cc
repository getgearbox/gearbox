// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/Json.h>
#include <gearbox/core/logger.h>
using namespace Gearbox;

#include <string>
using std::string;

int main() {
    chdir(TESTDIR);
    TEST_START(138);
    log_init("./unit.conf");

    Json j;
    j.parse("{\"key1\": {\"key2\": [\"idx1\", [\"idx2\"]]}, \"int\": 1234567890, \"double\": 123.456, \"bool\": true, \"string\": \"abc\"}");

    // test equals
    Json k(j);
    OK( j == k );
    k["key1"].deleteKey("key2");
    OK( j != k );

    k = j;
    k["key1"]["key2"][0] = "foobar";
    OK( j != k );

    k = j;
    k["key1"]["int"] = 31337;
    OK( j != k );

    k = j;
    k["key1"]["bool"] = false;
    OK( j != k );

    k = j;
    k["key1"]["double"] = 3.14;
    OK( j != k );

    k.parse("{}");
    OK( j != k );

    IS( j["key1"]["key2"][0].as<string>(), "idx1" );
    THROWS( j["key1"]["key2"]["bar"], "Json Exception: cannot access object key \"bar\" on type array at: [\"key1\"][\"key2\"]" );

    const Json cj(j);
    THROWS( cj["key1"]["key2"][4], "Json Exception: index out of range at: [\"key1\"][\"key2\"]" );

    // change copied object (j) and make sure the copy(cj) is not modified
    NOTHROW( j["key1"]["key2"][4][1] = 0 );
    THROWS( cj["key1"]["key2"][4], "Json Exception: index out of range at: [\"key1\"][\"key2\"]" );


    bool b1 = j["bool"].as<bool>();
    IS( b1, true );
    bool & b2 = j["bool"].as<bool>();
    IS( b2, true );
    bool b3 = cj["bool"].as<bool>();
    IS( b3, true );
    const bool & b4 = cj["bool"].as<bool>();
    IS( b4, true );
    bool b5, b6;
    j["bool"].to(b5);
    cj["bool"].to(b6);
    IS( b5, true );
    IS( b6, true );
    
    THROWS( j["bool"].as<int>(), "Json Exception: cannot convert boolean to integer at: [\"bool\"]" );

    int i1 = j["int"].as<int>();
    IS( i1, 1234567890 );
    int i2 = cj["int"].as<int>();
    IS( i2, 1234567890 );
    const int & i3 = cj["int"].as<int>();
    IS( i3, 1234567890 );
    int i4, i5;
    j["int"].to(i4);
    cj["int"].to(i5);
    IS( i4, 1234567890 );
    IS( i5, 1234567890 );
    THROWS( j["int"].as<string>(), "Json Exception: cannot convert integer to string at: [\"int\"]" );

    int64_t l1 = j["int"].as<int64_t>();
    IS( l1, 1234567890 );
    int64_t & l2 = j["int"].as<int64_t>();
    IS( l2, 1234567890 );
    int64_t l3 = cj["int"].as<int64_t>();
    IS( l3, 1234567890 );
    const int64_t & l4 = cj["int"].as<int64_t>();
    IS( l4, 1234567890 );
    int64_t l5, l6;
    j["int"].to(l5);
    cj["int"].to(l6);
    IS( l5, 1234567890 );
    IS( l6, 1234567890 );
    THROWS( j["int"].as<string>(), "Json Exception: cannot convert integer to string at: [\"int\"]" );

    string s1 = j["string"].as<string>();
    IS( s1, "abc" );
    string & s2 = j["string"].as<string>();
    IS( s2, "abc");
    string s3 = cj["string"].as<string>();
    IS( s3, "abc" );
    const string & s4 = cj["string"].as<string>();
    IS( s4, "abc" );
    string s5, s6;
    j["string"].to(s5);
    cj["string"].to(s6);
    IS( s5, "abc" );
    IS( s6, "abc" );
    THROWS( j["string"].as<int>(), "Json Exception: cannot convert string to integer at: [\"string\"]" );

    double d1 = j["double"].as<double>();
    IS( d1, 123.456 );
    double & d2 = j["double"].as<double>();
    IS( d2, 123.456 );
    double d3 = cj["double"].as<double>();
    IS( d3, 123.456 );
    const double & d4 = cj["double"].as<double>();
    IS( d4, 123.456 );
    double d5, d6;
    j["double"].to(d5);
    cj["double"].to(d6);
    IS( d5, 123.456 );
    IS( d6, 123.456 );
    THROWS( j["double"].as<int>(), "Json Exception: cannot convert double to integer at: [\"double\"]" );
    
    float f1 = j["double"].as<float>();
    IS( f1, 123.456 );
    float f2 = cj["double"].as<float>();
    IS( f2, 123.456 );
    const float & f3 = cj["double"].as<float>();
    IS( f3, 123.456 );
    float f4, f5;
    j["double"].to(f4);
    cj["double"].to(f5);
    IS( f4, 123.456 );
    IS( f5, 123.456 );
    THROWS( j["double"].as<int>(), "Json Exception: cannot convert double to integer at: [\"double\"]" );

    IS( j["key1"]["key2"][1][0].as<string>(), "idx2" );
    IS( j["int"].as<int64_t>(), 1234567890 );
    IS( j["double"].as<double>(), 123.456 );
    IS( j["bool"].as<bool>(), true );
    IS( j["string"].as<string>(), "abc" );
    OK( j.hasKey("key1") );
    OK( ! j.hasKey("foo") );
    OK( j["key1"].hasKey("key2") );
    NOTHROW( j["new"] = "def" );
    IS( j["new"].as<string>(), "def" );
    Json x;
    x = "def";
    IS( x.as<string>(), "def" );
    THROWS( j[0], "Json Exception: cannot access array index on type object" );
    NOTHROW( j["key1"]["key2"][4] = 123 );
    NOTHROW( j["key1"]["key2"][3] = 123.456 );
    NOTHROW( j["key1"]["key2"][2] = string("string") );
    NOTHROW( j["key1"]["key2"][6] = "boo" );

    Json sameKey;
    THROWS( sameKey.parse("{ \"foo\": 1, \"foo\": 2 }"), "duplicate key name found: foo" );

    Json i(10);
    IS(i.as<int>(), 10);
    i =  11;
    IS(i.as<int>(), 11);
    IS( i.typeName(), "integer" );
    Json js("abcdef");

#define JSON_STRING(init, initexp, newval, newexp, name) {     \
        Json j(init);                                          \
        IS( j.typeName(), #name );                             \
        IS( j.serialize(), initexp );                          \
        string & ref = j.as<string>();                         \
        ref = newval;                                          \
        IS( j.serialize(), newexp );                           \
        j.clear();                                             \
        IS( j.length(), -1 );                                  \
        IS( j.serialize(), "null" );                           \
        j = init;                                              \
        IS( j.serialize(), initexp );                          \
    }

    JSON_STRING("abc", "\"abc\"", "foo", "\"foo\"", string);
    JSON_STRING(string("abc"), "\"abc\"", string("foo"), "\"foo\"", string);

#define JSON_TEST(init, newval, type, name) {                   \
        Json j(init);                                           \
        IS( j.typeName(), #name );                              \
        IS( j.serialize(), #init );                             \
        type & ref = j.as<type>();                              \
        ref = newval;                                           \
        IS( j.serialize(), #newval );                           \
        j.clear();                                              \
        IS( j.length(), -1 );                                   \
        IS( j.serialize(), "null" );                            \
        j = init;                                               \
        IS( j.serialize(), #init );                             \
    }

    JSON_TEST(90, 54, int64_t, integer);
    JSON_TEST(true, false, bool, boolean);
    JSON_TEST(90.11, 12.34, double, double);

    // default then assign
    Json o;
    o = Json::Object();
    IS( o.empty(), true );
    IS( o.length(), -1 );
    IS( o.typeName(), "object" );
    IS( o.serialize(), "{}" );
    IS( o.serialize(true), "{\n\n}\n" );
    IS( o.hasKey("foo"), false );
    IS( o.type(), Json::OBJECT );
    o["key"] = "value";
    IS( o.serialize(), "{\"key\":\"value\"}" );
    THROWS( o[0] = 1, "Json Exception: cannot access array index on type object" );
    o["arr"] = Json::Array();
    IS( o.serialize(), "{\"arr\":[],\"key\":\"value\"}" );
    o["arr2"][0] = 1;
    IS( o.serialize(), "{\"arr\":[],\"arr2\":[1],\"key\":\"value\"}" );

    // use Json(const Json::Object &) constructor
    Json o2 = Json::Object();
    IS( o2.empty(), true );
    IS( o2.length(), -1 );
    IS( o2.typeName(), "object" );
    IS( o2.serialize(), "{}" );
    IS( o2.serialize(true), "{\n\n}\n" );
    IS( o2.type(), Json::OBJECT );
    
    Json a = Json::Array();
    IS( a.empty(), true );
    IS( a.length(), 0 );
    IS( a.typeName(), "array" );
    IS( a.serialize(), "[]" );
    IS( a.type(), Json::ARRAY );
    // auto grow an array
    a[3] = 32.122;
    IS( a.serialize(), "[null,null,null,32.122]" );
    IS( a[3].typeName(), "double" );
    THROWS( a.deleteKey("xyzzy"), "Json Exception: delKey() called on type array" );

    Json o3 = Json::Object();
    o3["grue"] = "lurking";
    IS( o3.hasKey("grue"), true );
    IS( o3.deleteKey("grue"), true );
    IS( o3.hasKey("grue"), false );
    IS( o3.deleteKey("ruby"), false );

    Json j4;
    IS( j4["bool"].init<bool>() = true, true );
    IS( j4["int"].init<int>() = 4, 4 );
    IS( j4["dbl"].init<double>() = 1.234, 1.234 );
    IS( j4["str"].init<string>() = "string", "string");
    NOTHROW( j4["obj"].init<Json::Object>() );
    NOTHROW( j4["arr"].init<Json::Array>() );
    IS(j4.serialize(), "{\"arr\":[],\"bool\":true,\"dbl\":1.234,\"int\":4,\"obj\":{},\"str\":\"string\"}");
        
    

    TEST_END;
}
