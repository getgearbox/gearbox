#!/usr/bin/perl
use strict;
use warnings;
use lib qw(../../../perl/Gearbox_Rest_Client);
use Gearbox::Rest::Client;
use Test::Trivial tests => 64;

use Log::Log4perl qw(:easy);
Log::Log4perl->easy_init({
    level    => $DEBUG,
    file     => "STDOUT",
    layout   => '# %m%n'
});

use Getopt::Long;
my $TRACE = 0;
GetOptions(
    'trace' => \$TRACE,
);

my $c = Gearbox::Rest::Client->new( trace => $TRACE );
my $PREFIX = $ENV{PREFIX} ? "/$ENV{PREFIX}" : "";
my $URI = "http://" . ($ENV{SMOKE_HOST} || "localhost") . "$PREFIX/test-sync/v1";

my $data;
my $err;

######################################################################
#
# create thing with id "myname"
#
######################################################################
OK $data = $c->req("PUT", "$URI/thing/myname", '{"id":"myname","stuff":"value"}');

my $expected = {
    "id" => "myname",
    "stuff" => "value"
};
# verify the reponse object looks like we expect
IS $data => $expected;

# no status for a syncronous PUT
IS $c->status() => undef;

######################################################################
#
# do an INDEX GET
#
######################################################################
OK $data = $c->req("GET", "$URI/thing/");
ISA $data->{things} => 'ARRAY';
OK @{$data->{things}} > 0;
OK grep { $_ eq 'myname' } @{$data->{things}};

# check the the SetEnv apache config setting got pushed out
# via our header ... note that headers are always
# lower-cased in and out of gearbox
IS $c->res()->header("testsync") => "TestValue";

######################################################################
#
# do an INDEX get but fetch fully expanded objects
# with the matrix argument
#
######################################################################
OK $data = $c->req("GET", "$URI/thing/;_expand=1");
ISA $data->{things} => 'ARRAY';
OK @{$data->{things}} > 0;
OK my ($thing) = grep { $_->{id} eq 'myname' } @{$data->{things}};
IS $thing => $expected;

######################################################################
#
# do an INDEX get but only fetch last 1 item via
# cgi query param
#
######################################################################
OK $data = $c->req("GET", "$URI/thing/?_count=1");
ISA $data->{things} => 'ARRAY';
IS @{$data->{things}} =>  1;

######################################################################
#
# do a fake-out DELETE, this should be a no-op
# with the fake-out header
#
######################################################################
OK $c->req("DELETE", "$URI/thing/myname", undef, { "fake-out" => 1 });

# no status for a syncronous DELETE
IS $c->status() => undef;

# the thing should still be there after our fake-out delete
OK $c->req("GET", "$URI/thing/myname");

######################################################################
#
# delete the thing we just created
#
######################################################################
OK $c->req("DELETE", "$URI/thing/myname");

# no status for a syncronous DELETE
IS $c->status() => undef;

######################################################################
#
# we should get an error now when we try to get the
# thing we just deleted
#
######################################################################
ISA $err = ERR { $c->req("GET", "$URI/thing/myname") }
    => "Gearbox::Rest::Client::Error";

# check the error status message
IS $err->{code} => 404;
IS $err->{progress} => 100;
IS $err->{uri} => "$URI/thing/myname";
IS $err->{state} => "COMPLETED";
LIKE $err->{messages}->[0] => qr/thing "myname" not found/;

######################################################################
#
# create a new thing, but let the system auto generate
# the name
#
######################################################################
OK $data = $c->req("POST", "$URI/thing", '{"stuff":"newval"}');

IS $data->{stuff} => "newval";

my $id  = $data->{id};
# should be something like: t-fkrtn27ebh744xjsy92ersaheb
LIKE $data->{id} => qr/t-\S+/;

# no status for a syncronous POST
IS $c->status() => undef;

######################################################################
# 
# update the thing to have a new "stuff" value
# 
######################################################################
OK $data = $c->req("POST", "$URI/thing/$id", '{"stuff":"foobar"}');

$expected = {
    id => $id,
    stuff => "foobar"
};

IS $data, $expected;

# no status for a syncronous POST
IS $c->status() => undef;

######################################################################
#
# expecting a Json schema error with the bogus key:
#
######################################################################
ISA $err = ERR { 
    $c->req("POST", "$URI/thing", '{"stuff":"value","bogus":"key"}')
} => "Gearbox::Rest::Client::Error";

# check the error status message
IS $err->{code} => 400;
IS $err->{progress} => 100;
IS $err->{uri} => "$URI/thing";
IS $err->{state} => "COMPLETED";
LIKE $err->{messages}->[0] => qr/invalid property/;

######################################################################
#
# expecting a Json schema error with the missing required key:
#
######################################################################
ISA $err = ERR { 
    $c->req("POST", "$URI/thing", '{}')
} => "Gearbox::Rest::Client::Error";

# check the error status message
IS $err->{code} => 400;
IS $err->{progress} => 100;
IS $err->{uri} => "$URI/thing";
IS $err->{state} => "COMPLETED";
LIKE $err->{messages}->[0] => qr/non-optional property "stuff" is missing/;

######################################################################
#
# doing a GET with content is not allowed per get schema
#
######################################################################
ISA $err = ERR { 
    $c->req("GET", "$URI/thing/$id", '{}')
} => "Gearbox::Rest::Client::Error";

# check the error status message
IS $err->{code} => 400;
IS $err->{progress} => 100;
IS $err->{uri} => "$URI/thing/$id";
IS $err->{state} => "COMPLETED";
LIKE $err->{messages}->[0] => qr/schema does not allow for type "object"/;

######################################################################
#
# doing a DELETE with content is not allowed per delete schema
#
######################################################################
ISA $err = ERR { 
    $c->req("DELETE", "$URI/thing/$id", '{}')
} => "Gearbox::Rest::Client::Error";

# check the error status message
IS $err->{code} => 400;
IS $err->{progress} => 100;
IS $err->{uri} => "$URI/thing/$id";
IS $err->{state} => "COMPLETED";
LIKE $err->{messages}->[0] => qr/schema does not allow for type "object"/;

######################################################################
#
# doing a PUT with empty content is not allowed per delete schema
#
######################################################################
ISA $err = ERR { 
    $c->req("PUT", "$URI/bogus/myname", '{}')
} => "Gearbox::Rest::Client::Error";

# check the error status message
IS $err->{code} => 400;
IS $err->{progress} => 100;
IS $err->{uri} => "$URI/bogus/myname";
IS $err->{state} => "COMPLETED";
LIKE $err->{messages}->[0] => qr/No valid handler found!/;

