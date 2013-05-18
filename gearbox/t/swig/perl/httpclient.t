#!/usr/bin/env perl
use lib $ENV{PWD}."/".($0 =~ m{(.*)/[^/]+$})[0];
use strict;
use warnings;
use setup;
use Test::Trivial (tests => 6);

# not much here, we really only import the consts
# for use with the $JobManager->job(HttpClient::Method, Uri) api
use Gearbox;

IS $Gearbox::HttpClient::METHOD_GET => 0;
IS $Gearbox::HttpClient::METHOD_DELETE => 1;
IS $Gearbox::HttpClient::METHOD_POST => 2;
IS $Gearbox::HttpClient::METHOD_PUT => 3;
IS $Gearbox::HttpClient::METHOD_HEAD => 4;
IS $Gearbox::HttpClient::METHOD_UNKNOWN => 5;

