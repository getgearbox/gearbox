// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/JsonSchema.h>
#include "JsonPrivate.h"

#include <string>
#include <sstream>

#include <boost/regex.hpp>

#include <gearbox/core/logger.h>

#include <sys/types.h>   // for inet_ptoa
#include <sys/socket.h>  // for inet_ptoa
#include <arpa/inet.h>   // for inet_ptoa

using std::string;

namespace Gearbox {

static bool DateTimeFormatChecker(const std::string & value ) {
    // regex infered from: http://www.w3.org/TR/NOTE-datetime
    // (?:0[13578]|1[02])-(?:[0-2]\\d|3[01]) // 31 day months
    // (?:0[469]|11)-(?:[0-2]\\d|30)         // 30 day months
    // .. this does not bother to validate if year is leapyear
    // 02-(?:[0-1]\\d|2[0-9])                // 29 day months
    // allow for midnight to be 23 or 00 otherwise only 23 hours 59 min and 59 secs
    // (?:24:00:00|(?:[01]\\d|2[0-3]):[0-5]\\d:[0-5]\\d) 
    // tz offsets can be any up to (not including) 1 day
    // (?:[0-1]\\d|2[0-3]):[0-5]\\d

    static boost::regex rx(
        "^\\d{4}-(?:(?:0[13578]|1[02])-(?:[0-2]\\d|3[01])|(?:0[469]|11)-(?:[0-2]\\d|30)|02-(?:[0-1]\\d|2[0-9]))T(?:24:00:00|(?:[01]\\d|2[0-3]):[0-5]\\d:[0-5]\\d)(?:Z|[-+](?:[0-1]\\d|2[0-3]):[0-5]\\d)$",
        boost::regex::perl
    );

    return regex_match(value, rx);
}

static bool DateFormatChecker(const std::string & value) {
    // regex infered from: http://www.w3.org/TR/NOTE-datetime
    // (?:0[13578]|1[02])-(?:[0-2]\\d|3[01]) // 31 day months
    // (?:0[469]|11)-(?:[0-2]\\d|30)         // 30 day months
    // .. this does not bother to validate if year is leapyear
    // 02-(?:[0-1]\\d|2[0-9])                // 29 day months 
    
    static boost::regex rx(
        "^\\d{4}-(?:(?:0[13578]|1[02])-(?:[0-2]\\d|3[01])|(?:0[469]|11)-(?:[0-2]\\d|30)|02-(?:[0-1]\\d|2[0-9]))$",
        boost::regex::perl
    );

    return regex_match(value, rx);
}

static bool UriFormatChecker(const std::string & value) {
    // regex taken from: perl -MRegexp::Common=URI -e 'print $RE{URI}' | perl -pi -e 's/http/https?/; s/\\/\\\\/g'
    static boost::regex rx(
        "(?:(?:(?:https?)://(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z])[.]?)|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]*)))?(?:/(?:(?:(?:(?:(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*)(?:;(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*))*)(?:/(?:(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*)(?:;(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*))*))*))(?:[?](?:(?:(?:[;/?:@&=+$,a-zA-Z0-9\\-_.!~*'()]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*)))?))?)|(?:(?:nntp)://(?:(?:(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]+)))?)/(?:(?:[a-zA-Z][-A-Za-z0-9.+_]*))(?:/(?:[0-9]+))?))|(?:(?:file)://(?:(?:(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+))|localhost)?)(?:/(?:(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),:@&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*)(?:/(?:(?:[-a-zA-Z0-9$_.+!*'(),:@&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))*)))))|(?:(?:ftp)://(?:(?:(?:(?:[a-zA-Z0-9\\-_.!~*'();:&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*))(?:)@)?(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z])[.]?)|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]*)))?(?:/(?:(?:(?:(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*)(?:/(?:(?:[a-zA-Z0-9\\-_.!~*'():@&=+$,]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*))*))(?:;type=(?:[AIai]))?))?)|(?:(?:tel):(?:(?:(?:[+](?:[0-9\\-.()]+)(?:;isub=[0-9\\-.()]+)?(?:;postd=[0-9\\-.()*#ABCDwp]+)?(?:(?:;(?:phone-context)=(?:(?:(?:[+][0-9\\-.()]+)|(?:[0-9\\-.()*#ABCDwp]+))|(?:(?:[!'E-OQ-VX-Z_e-oq-vx-z~]|(?:%(?:2[124-7CFcf]|3[AC-Fac-f]|4[05-9A-Fa-f]|5[1-689A-Fa-f]|6[05-9A-Fa-f]|7[1-689A-Ea-e])))(?:[!'()*\\-.0-9A-Z_a-z~]+|(?:%(?:2[1-9A-Fa-f]|3[AC-Fac-f]|[4-6][0-9A-Fa-f]|7[0-9A-Ea-e])))*)))|(?:;(?:tsp)=(?: |(?:(?:(?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?)(?:[.](?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?))*))))|(?:;(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:=(?:(?:(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:[?](?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*))?)|(?:%22(?:(?:%5C(?:[a-zA-Z0-9\\-_.!~*'()]|(?:%[a-fA-F0-9][a-fA-F0-9])))|[a-zA-Z0-9\\-_.!~*'()]+|(?:%(?:[01][a-fA-F0-9])|2[013-9A-Fa-f]|[3-9A-Fa-f][a-fA-F0-9]))*%22)))?))*)|(?:[0-9\\-.()*#ABCDwp]+(?:;isub=[0-9\\-.()]+)?(?:;postd=[0-9\\-.()*#ABCDwp]+)?(?:;(?:phone-context)=(?:(?:(?:[+][0-9\\-.()]+)|(?:[0-9\\-.()*#ABCDwp]+))|(?:(?:[!'E-OQ-VX-Z_e-oq-vx-z~]|(?:%(?:2[124-7CFcf]|3[AC-Fac-f]|4[05-9A-Fa-f]|5[1-689A-Fa-f]|6[05-9A-Fa-f]|7[1-689A-Ea-e])))(?:[!'()*\\-.0-9A-Z_a-z~]+|(?:%(?:2[1-9A-Fa-f]|3[AC-Fac-f]|[4-6][0-9A-Fa-f]|7[0-9A-Ea-e])))*)))(?:(?:;(?:phone-context)=(?:(?:(?:[+][0-9\\-.()]+)|(?:[0-9\\-.()*#ABCDwp]+))|(?:(?:[!'E-OQ-VX-Z_e-oq-vx-z~]|(?:%(?:2[124-7CFcf]|3[AC-Fac-f]|4[05-9A-Fa-f]|5[1-689A-Fa-f]|6[05-9A-Fa-f]|7[1-689A-Ea-e])))(?:[!'()*\\-.0-9A-Z_a-z~]+|(?:%(?:2[1-9A-Fa-f]|3[AC-Fac-f]|[4-6][0-9A-Fa-f]|7[0-9A-Ea-e])))*)))|(?:;(?:tsp)=(?: |(?:(?:(?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?)(?:[.](?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?))*))))|(?:;(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:=(?:(?:(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:[?](?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*))?)|(?:%22(?:(?:%5C(?:[a-zA-Z0-9\\-_.!~*'()]|(?:%[a-fA-F0-9][a-fA-F0-9])))|[a-zA-Z0-9\\-_.!~*'()]+|(?:%(?:[01][a-fA-F0-9])|2[013-9A-Fa-f]|[3-9A-Fa-f][a-fA-F0-9]))*%22)))?))*))))|(?:(?:fax):(?:(?:(?:[+](?:[0-9\\-.()]+)(?:;isub=[0-9\\-.()]+)?(?:;tsub=[0-9\\-.()]+)?(?:;postd=[0-9\\-.()*#ABCDwp]+)?(?:(?:;(?:phone-context)=(?:(?:(?:[+][0-9\\-.()]+)|(?:[0-9\\-.()*#ABCDwp]+))|(?:(?:[!'E-OQ-VX-Z_e-oq-vx-z~]|(?:%(?:2[124-7CFcf]|3[AC-Fac-f]|4[05-9A-Fa-f]|5[1-689A-Fa-f]|6[05-9A-Fa-f]|7[1-689A-Ea-e])))(?:[!'()*\\-.0-9A-Z_a-z~]+|(?:%(?:2[1-9A-Fa-f]|3[AC-Fac-f]|[4-6][0-9A-Fa-f]|7[0-9A-Ea-e])))*)))|(?:;(?:tsp)=(?: |(?:(?:(?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?)(?:[.](?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?))*))))|(?:;(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:=(?:(?:(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:[?](?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*))?)|(?:%22(?:(?:%5C(?:[a-zA-Z0-9\\-_.!~*'()]|(?:%[a-fA-F0-9][a-fA-F0-9])))|[a-zA-Z0-9\\-_.!~*'()]+|(?:%(?:[01][a-fA-F0-9])|2[013-9A-Fa-f]|[3-9A-Fa-f][a-fA-F0-9]))*%22)))?))*)|(?:[0-9\\-.()*#ABCDwp]+(?:;isub=[0-9\\-.()]+)?(?:;tsub=[0-9\\-.()]+)?(?:;postd=[0-9\\-.()*#ABCDwp]+)?(?:;(?:phone-context)=(?:(?:(?:[+][0-9\\-.()]+)|(?:[0-9\\-.()*#ABCDwp]+))|(?:(?:[!'E-OQ-VX-Z_e-oq-vx-z~]|(?:%(?:2[124-7CFcf]|3[AC-Fac-f]|4[05-9A-Fa-f]|5[1-689A-Fa-f]|6[05-9A-Fa-f]|7[1-689A-Ea-e])))(?:[!'()*\\-.0-9A-Z_a-z~]+|(?:%(?:2[1-9A-Fa-f]|3[AC-Fac-f]|[4-6][0-9A-Fa-f]|7[0-9A-Ea-e])))*)))(?:(?:;(?:phone-context)=(?:(?:(?:[+][0-9\\-.()]+)|(?:[0-9\\-.()*#ABCDwp]+))|(?:(?:[!'E-OQ-VX-Z_e-oq-vx-z~]|(?:%(?:2[124-7CFcf]|3[AC-Fac-f]|4[05-9A-Fa-f]|5[1-689A-Fa-f]|6[05-9A-Fa-f]|7[1-689A-Ea-e])))(?:[!'()*\\-.0-9A-Z_a-z~]+|(?:%(?:2[1-9A-Fa-f]|3[AC-Fac-f]|[4-6][0-9A-Fa-f]|7[0-9A-Ea-e])))*)))|(?:;(?:tsp)=(?: |(?:(?:(?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?)(?:[.](?:[A-Za-z](?:(?:(?:[-A-Za-z0-9]+)){0,61}[A-Za-z0-9])?))*))))|(?:;(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:=(?:(?:(?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*)(?:[?](?:(?:[!'*\\-.0-9A-Z_a-z~]+|%(?:2[13-7ABDEabde]|3[0-9]|4[1-9A-Fa-f]|5[AEFaef]|6[0-9A-Fa-f]|7[0-9ACEace]))*))?)|(?:%22(?:(?:%5C(?:[a-zA-Z0-9\\-_.!~*'()]|(?:%[a-fA-F0-9][a-fA-F0-9])))|[a-zA-Z0-9\\-_.!~*'()]+|(?:%(?:[01][a-fA-F0-9])|2[013-9A-Fa-f]|[3-9A-Fa-f][a-fA-F0-9]))*%22)))?))*))))|(?:(?:prospero)://(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]+)))?/(?:(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),?:@&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*)(?:/(?:(?:[-a-zA-Z0-9$_.+!*'(),?:@&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))*))(?:(?:;(?:(?:[-a-zA-Z0-9$_.+!*'(),?:@&]|(?:%[a-fA-F0-9][a-fA-F0-9]))*)=(?:(?:[-a-zA-Z0-9$_.+!*'(),?:@&]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))*))|(?:(?:tv):(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z])[.]?))?)|(?:(?:telnet)://(?:(?:(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),;?&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))(?::(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),;?&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*)))?)@)?(?:(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]+)))?)(?:/)?)|(?:(?:news):(?:(?:[*]|(?:(?:[-a-zA-Z0-9$_.+!*'(),;/?:&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))+@(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))|(?:[a-zA-Z][-A-Za-z0-9.+_]*))))|(?:(?:wais)://(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]+)))?/(?:(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))(?:[?](?:(?:(?:[-a-zA-Z0-9$_.+!*'(),;:@&=]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))|/(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),]|(?:%[a-fA-F0-9][a-fA-F0-9]))*))/(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),]|(?:%[a-fA-F0-9][a-fA-F0-9]))*)))?))|(?:(?:gopher)://(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]+)))?/(?:(?:(?:[0-9+IgT]))(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),:@&=]+|(?:%[a-fA-F0-9][a-fA-F0-9]))*))))|(?:(?:pop)://(?:(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),&=~]+|(?:%[a-fA-F0-9][a-fA-F0-9]))+))(?:;AUTH=(?:[*]|(?:(?:(?:[-a-zA-Z0-9$_.+!*'(),&=~]+|(?:%[a-fA-F0-9][a-fA-F0-9]))+)|(?:[+](?:APOP|(?:(?:[-a-zA-Z0-9$_.+!*'(),&=~]+|(?:%[a-fA-F0-9][a-fA-F0-9]))+))))))?@)?(?:(?:(?:(?:(?:(?:[a-zA-Z0-9][-a-zA-Z0-9]*)?[a-zA-Z0-9])[.])*(?:[a-zA-Z][-a-zA-Z0-9]*[a-zA-Z0-9]|[a-zA-Z]))|(?:[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))(?::(?:(?:[0-9]+)))?))",
        boost::regex::perl
    );
    return regex_match(value, rx);
}

static bool Ipv4FormatChecker( const std::string & value ) {
    struct sockaddr_in ip4addr;
    int rc = inet_pton(AF_INET, value.c_str(), &ip4addr.sin_addr);
    return rc > 0;
}

static bool Ipv6FormatChecker( const std::string & value ) {
    struct sockaddr_in6 ip6addr;
    int rc = inet_pton(AF_INET6, value.c_str(), &ip6addr.sin6_addr); 
    return rc > 0;
}

static bool MacFormatChecker( const std::string & value ) {
    // regex infered from: 
    // http://cpansearch.perl.org/src/ABIGAIL/Regexp-Common-2.122/lib/Regexp/Common/net.pm
    static boost::regex rx(
        "^(?:[0-9a-fA-F]{1,2}):(?:[0-9a-fA-F]{1,2}):(?:[0-9a-fA-F]{1,2}):(?:[0-9a-fA-F]{1,2}):(?:[0-9a-fA-F]{1,2}):(?:[0-9a-fA-F]{1,2})$",
        boost::regex::perl
    );

    return regex_match(value, rx);
}


JsonSchema::FormatMap JsonSchema::formats_;
std::string JsonSchema::default_format;

JsonSchema::FormatMap::FormatMap() {
    // for some format requirements check out this forum:
    // http://groups.google.com/group/json-schema/web/json-schema-possible-formats

    (*this)["date-time"] = DateTimeFormatChecker;
    (*this)["date"] = DateFormatChecker;
    (*this)["uri"] = UriFormatChecker;
    (*this)["ip-address"] = Ipv4FormatChecker;
    (*this)["ipv4"] = Ipv4FormatChecker;
    (*this)["ipv6"] = Ipv6FormatChecker;
    (*this)["mac-address"] = MacFormatChecker;
}

void JsonSchema::setFormat(const std::string & name, JsonSchema::FormatChecker checker) {
    JsonSchema::formats_[name] = checker;
}

void JsonSchema::defaultFormat( const std::string & name ) {
    default_format = name;
}

bool JsonSchema::validMaxSize(const Json & array) {
    if( this->hasKey("maxItems") ) {
        unsigned int count = (*this)["maxItems"].as<int>();
        if( array.as<Json::Array>().size() > count ) {
            std::ostringstream oss;
            oss << "array size is larger than schema maximum size " << count;
            bool cont = this->handleError(oss.str(), array.context());
            if( !cont ) return false;
        }
    }
    return true;
}

bool JsonSchema::validMinSize(const Json & array) {
    if( this->hasKey("minItems") ) {
        unsigned int count = (*this)["minItems"].as<int>();
        if( array.as<Json::Array>().size() < count ) {
            std::ostringstream oss;
            oss << "array size " << array.as<Json::Array>().size() << " "
                << "is less than schema minimum size " << count;
            bool cont = this->handleError(oss.str(), array.context());
            if( !cont ) return false;
        }
    }
    if( this->hasKey("items") && (*this)["items"].type() == Json::ARRAY ) {
        // this is tuple type, so make sure that there are exactly
        // the number of elements as many elements

        // if additionalProperties, then this is an unbounded array
        JsonSchema * addProp = this->getAdditionalProperties();
        if(addProp) return true;

        if( (*this)["items"].as<Json::Array>().size() != array.as<Json::Array>().size() ) {
            std::ostringstream oss;
            oss << "array size " << array.as<Json::Array>().size() << " does not match schema required size of tuple type "
                << (*this)["items"].as<Json::Array>().size();
            bool cont = this->handleError(oss.str(), array.context());
            if( !cont ) return false;
        }
    }
    return true;
}

static string typeToSchemaType(Json::Type type) {
    switch(type) {
    case Json::UNDEF:  return "null";
    case Json::STRING: return "string";
    case Json::INT:    return "integer";
    case Json::DOUBLE: return "number";
    case Json::BOOL:   return "boolean";
    case Json::OBJECT: return "object";
    case Json::ARRAY:  return "array";
    }
    return "any";
}

static bool typeListContains(
    JsonSchema & schema, 
    const std::string & typeListName, 
    const std::string & docType) {
    if( schema[typeListName].type() == Json::ARRAY ) {
        Json::Array & a = schema[typeListName].as<Json::Array>();
        for( unsigned int i=0; i<a.size(); ++i ) {
            string & type = a[i]->as<string>();
            if( type == "any" )   return true;
            if( type == docType ) return true;
            if( type == "number" && docType == "integer") return true;
        }
    }
    else {
        string & type = schema[typeListName].as<string>();
        if( type == "any" )   return true;
            if( type == docType ) return true;
            if( type == "number" && docType == "integer") return true;
    }
    return false;
}

bool JsonSchema::validType(const Json & json) {
    string docType = typeToSchemaType(json.type());

    if( !this->hasKey("type") && !this->hasKey("disallow") )
        return true;
        
    if( this->hasKey("type")
        && typeListContains(*this, "type", docType) )
        return true;
        
    if( this->hasKey("disallow")
        && !typeListContains(*this, "disallow", docType) )
        return true;
    
    std::string tmp;
    tmp += "schema does not allow for type \"";
    tmp += docType;
    tmp += "\"";
    bool cont = this->handleError(tmp, json.context());
    if( !cont ) return false;
    return true;
}

void JsonSchema::typeFixup(Json & json) {
    if ( !this->hasKey("type") ) return;
 
    if( (*this)["type"].type() == Json::ARRAY ) {
        Json::Array & a = (*this)["type"].as<Json::Array>();
        for( unsigned int i=0; i<a.size(); ++i ) {
            string & type = a[i]->as<string>();
            if( type == "number" &&  typeToSchemaType(json.type()) == "integer") {
                json = static_cast<double>(json.as<int>());
            }
        }
    }
    else {
        string & type = (*this)["type"].as<string>();
        if( type == "number" &&  typeToSchemaType(json.type()) == "integer") {
            json = static_cast<double>(json.as<int>());
        }
    }
}

template<typename T>
bool validMin(const Json & json, const T & minVal, string & err) {
    if( (json.type() == Json::INT && json.as<int64_t>() >= minVal)
        || (json.type() == Json::DOUBLE && json.as<double>() >= minVal) ) {
        return true;
    }

    std::stringstream oss;
    oss.precision(15);
    oss << "value " << json << " is less than schema minimum value " << minVal;
    err = oss.str();
    return false;
}

template<typename T>
bool validMax(const Json & json, const T & maxVal, string & err) {
    if( (json.type() == Json::INT && json.as<int64_t>() <= maxVal)
        || (json.type() == Json::DOUBLE && json.as<double>() <= maxVal) ) {
        return true;
    }

    std::stringstream oss;
    // 15 is highest precision for a double
    oss.precision(15);
    oss << "value " << json << " is larger than schema maximum value " << maxVal;
    err = oss.str();
    return false;
}

bool JsonSchema::validValue(const Json & json) {
    string err;
    if( this->hasKey("minimum") ) {
        Json & min = (*this)["minimum"];
        Json::Type t = min.type();
        if( t == Json::INT 
            && !validMin(json, min.as<int64_t>(), err) ) {
            bool cont = handleError(err, json.context());
            if( !cont ) return false;
        }
        if( t == Json::DOUBLE
            && !validMin(json, min.as<double>(), err) ) {
            bool cont = handleError(err, json.context());
            if( !cont ) return false;
        }
    }

    if( this->hasKey("maximum") ) {
        Json & max = (*this)["maximum"];
        Json::Type t = max.type();
        if( t == Json::INT
            && !validMax(json, max.as<int64_t>(), err) ) {
            bool cont = handleError(err, json.context());
            if( !cont ) return false;
        }
        if( t == Json::DOUBLE
            && !validMax(json, max.as<double>(), err) ) {
            bool cont = handleError(err, json.context());
            if( !cont ) return false;
        }
    }

    if( json.type() == Json::DOUBLE ) {
        if( this->hasKey("maxDecimal") ) {
            // 15 is highest precision for a double
            std::ostringstream oss;
            oss.precision(15);
            oss << json.as<double>();
            unsigned int decimals = oss.str().length() - oss.str().find('.') - 1;
            uint64_t max = (*this)["maxDecimal"].as<uint64_t>();
            if( decimals > max ) {
                std::ostringstream oss;
                oss.precision(15);
                oss << "value " << json << " has " << decimals 
                    << " decimal places, but the schema has specified maximum of " 
                    << max << " decimal places";
                bool cont = handleError(oss.str(), json.context());
                if( !cont ) return false;
            }
        }
    }

    if( json.type() == Json::STRING && this->hasKey("minLength") ) {
        uint64_t minLen = (*this)["minLength"].as<uint64_t>();
        if( json.as<string>().size() < minLen ) {
            std::stringstream oss;
            oss << "string length " << json.as<string>().size() << " is less than schema minimum length " << minLen;
            bool cont = this->handleError(oss.str(), json.context());
            if( !cont ) return false;
        }
    }
    
    if( json.type() == Json::STRING && this->hasKey("maxLength") ) {
        uint64_t maxLen = (*this)["maxLength"].as<uint64_t>();
        if( json.as<string>().size() > maxLen ) {
            std::stringstream oss;
            oss << "string length " << json.as<string>().size() << " is larger than schema maximum length " << maxLen;
            bool cont = this->handleError(oss.str(), json.context());
            if( !cont ) return false;
        }
    }

    if( json.type() == Json::STRING ) {
        std::string format;
        if( this->hasKey("format") ) {
            format = (*this)["format"].as<string>();
        }
        else if ( ! JsonSchema::default_format.empty() ) {
            format = JsonSchema::default_format;
        }
        if( ! format.empty() ) {
            JsonSchema::FormatMap::iterator it = formats_.find( format );
            if( it != formats_.end() ) {
                if( ! it->second(json.as<string>()) ) {
                    _WARN("value \"" << json.as<string>() << "\" does not match required schema format " << format);

                    // dont return the value in the error message in case it is from malicious
                    // content and our error message is printed back to user.
                    bool cont = this->handleError(
                        "value does not match required schema format " + format,
                        json.context()
                    );
                    if( !cont ) return false;
                }
            }
        }
    }

    if( json.type() == Json::STRING && this->hasKey("pattern") ) {
        // TODO check regex pattern against string
        boost::regex rx( (*this)["pattern"].as<string>(), boost::regex::perl );
        if( ! boost::regex_match( json.as<string>(), rx ) ) {
            _WARN("value \"" << json.as<string>() << "\" does not match schema pattern \"" + (*this)["pattern"].as<string>() + "\"");
            bool cont = this->handleError(
                "value does not match schema pattern \"" + (*this)["pattern"].as<string>() + "\"",
                json.context()
            );
            if( !cont ) return false;
        }
    }

    if( this->hasKey("enum") ) {
        JsonSchema & e = (*this)["enum"];
        for(int i=0; i<e.length(); ++i) {
            Json::Type t = e[i].type();
            if( json.type() != t )
                continue;
            if( t == Json::INT && e[i].as<int64_t>() == json.as<int64_t>() )
                return true;
            if( t == Json::DOUBLE && e[i].as<double>() == json.as<double>() )
                return true;
            if( t == Json::BOOL && e[i].as<bool>() == json.as<bool>() )
                return true;
            if( t == Json::STRING && e[i].as<string>() == json.as<string>() )
                return true;
        }
        std::stringstream oss;
        oss.precision(15);
        oss << "value is not valid in schema enum";
        bool cont = this->handleError(oss.str(), json.context());
        if( !cont ) return false;
    }

    return true;
}

bool JsonSchema::validRequires(const Json & object) {
    if( this->hasKey("properties") ) {
        Json::Object & o = (*this)["properties"].as<Json::Object>();
        Json::Object::iterator it = o.begin();
        for( ; it != o.end(); ++it ) {
            JsonSchema & s = (*this)["properties"][it->first];
            if( s.hasKey("requires") && s.hasKey("optional") && s["optional"].as<bool>() && object.hasKey(it->first) ) {
                if( s["requires"].type() == Json::STRING ) {
                    string & req = s["requires"].as<string>();
                    if( ! object.hasKey( req ) ) {
                        bool cont = this->handleError(
                            "property \"" + it->first + "\" requires additional property \"" + req + "\" to be set according to schema",
                            object.context()
                        );
                        if( !cont ) return false;
                    }
                }
                else if( s["requires"].type() == Json::ARRAY ) {
                    for( int i=0; i < s["requires"].length(); ++i ) {
                        string & req = s["requires"][i].as<string>();
                        if( ! object.hasKey( req ) ) {
                            bool cont = this->handleError(
                                "property \"" + it->first + "\" requires additional property \"" + req + "\" to be set according to schema",
                                object.context()
                            );
                            if( !cont ) return false;
                        }
                    }
                }
            }
            if( s.hasKey("optional") && s["optional"].as<bool>() ) {
                continue;
            }
            // it is not optional, so make sure it is set
            if( ! object.hasKey( it->first ) ) {
                bool cont = this->handleError(
                    "non-optional property \"" + it->first + "\" is missing",
                    object.context()
                );
                if( !cont ) return false;
            }
        }
    }
    return true;
}

bool JsonSchema::setDefaults(Json & object) {
    if( this->hasKey("properties") ) {
        Json::Object & o = (*this)["properties"].as<Json::Object>();
        Json::Object::iterator it = o.begin();
        for( ; it != o.end(); ++it ) {
            JsonSchema & s = (*this)["properties"][it->first];
            if( s.hasKey("default") && !object.hasKey(it->first) ) {

                s.typeFixup( s["default"] );

                // found property with default value, and object does
                // not have property present ... so set it to the
                // default value
                object[it->first] = static_cast<Json &>(s["default"]);
            }
        }
    }
    return true;
}

const JsonSchema & JsonSchema::operator[](const std::string & key) const {
    return static_cast<const JsonSchema&>( this->super::operator[](key) );
}

const JsonSchema & JsonSchema::operator[](int i) const {
    return static_cast<const JsonSchema&>( this->super::operator[](i) );
}

JsonSchema & JsonSchema::operator[](const std::string & key) {
    return static_cast<JsonSchema&>( this->super::operator[](key) );
}

JsonSchema & JsonSchema::operator[](int i) {
    return static_cast<JsonSchema&>( this->super::operator[](i) );
}

JsonSchema * JsonSchema::getAdditionalProperties() {
    if( (*this).hasKey("additionalProperties") ) {
        if( (*this)["additionalProperties"].type() == Json::BOOL ) {
            if( (*this)["additionalProperties"].as<bool>() ) {
                // reset the property to be an object so we can use it for
                // validation.  (replace the bool with an object)
                (*this)["additionalProperties"].parse("{\"type\":\"any\"}");
                return &((*this)["additionalProperties"]);
            }
        } 
        else if( (*this)["additionalProperties"].type() == Json::OBJECT ) {
            return &((*this)["additionalProperties"]);
        }
    }
    else {
        // default additionalProperty is to allow "anything"
        // this will autovivify additionalProperties then set the 
        // value to the generic "any" object type
        (*this)["additionalProperties"].parse("{\"type\":\"any\"}");
        return &((*this)["additionalProperties"]);
    }
    return NULL;
}

JsonSchema * JsonSchema::getChildSchema(int index, const Json & json) {
    JsonSchema * addProp = this->getAdditionalProperties();
    if( this->hasKey("items") ) {
        int length = (*this)["items"].length();
        if( length >= 0 ) {
            // items is an array
            if( index >= length ) {
                // ... if index out of bounds look for additionalProperties
                if( addProp ) {
                    return addProp;
                }
                else {
                    std::ostringstream oss;
                    oss << "out of bounds: schema does not allow for elements at index " << index;
                    this->handleError(oss.str(), json.context());
                    return NULL;
                }
            }
            else {
                // reset the parent to be *this* instead of the ["items"]
                // array so when the setSchemaUp function is called we get
                // reset to *this* and can process next item in array
                (*this)["items"][index].impl_->parent  = this;
                return &((*this)["items"][index]);
            }
        }
        else {
            // items is an object
            return &((*this)["items"]);
        }
    }
    else if( addProp ) {
        return addProp;
    }
    else {
        this->handleError(
            "out of bounds: schema does not allow for any elements in array",
            json.context()
        );
        return NULL;
    }
    this->handleError(
        "Unknow error trying to determine child schema for array",
        this->context()
    );
    return NULL;
}

JsonSchema * JsonSchema::getChildSchema( const string & key, const Json & json ) {
    
    JsonSchema * addProp = this->getAdditionalProperties();
    
    if( this->hasKey("properties") ) {
        if( (*this)["properties"].hasKey(key) ) {
            (*this)["properties"][key].impl_->parent = this;
            return &((*this)["properties"][key]);
        }
        if(addProp) return addProp;
        this->handleError(
            "invalid property \"" + key + "\": schema does not allow for this property",
            json.context()
        );
        return NULL;
    }
    else if( addProp ) {
        return addProp;
    }
    this->handleError(
        "invalid property \"" + key + "\": schema allows for no properties",
        json.context()
    );
    return NULL;
}

} // namespace
