#!/usr/bin/perl

use FindBin qw($Bin);

use lib "$Bin/../lib";
use Test::More qw(no_plan);

BEGIN {
    use_ok("Gearbox::Service::Builder");
}

my $bldr = Gearbox::Service::Builder->new(
    file           => "files/1.json",
    key            => "daemons",
    service_dir    => "/tmp/svc/1",
    supervised_dir => "/tmp/service",
    prefix         => "gearbox-1-",
);

$bldr->generate;

$bldr = Gearbox::Service::Builder->new(
    file           => "files/2.json",
    key            => "daemons",
    service_dir    => "/tmp/svc/2",
    supervised_dir => "/tmp/service",
    prefix         => "gearbox-2-",
);

$bldr->generate;

