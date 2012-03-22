######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::Service::Builder;

use strict;
use warnings;

use Carp;
BEGIN { $ENV{PERL_JSON_BACKEND} = 'JSON::PP' }
use JSON; # always uses JSON::PP
use Gearbox::Utils qw(System info debug);
use Gearbox::Service::Entry;
use File::Basename;

sub new {
    my $class = shift;

    my %params = @_;
    croak "Need file, key, and service_dir"
        unless ( $params{file} && $params{key} && $params{service_dir} );

    my $dir       = $params{service_dir};
    my $super_dir = $params{supervised_dir};
    $super_dir ||= "/service";

    System("mkdir -p $dir")       unless -d $dir;
    System("mkdir -p $super_dir") unless -d $super_dir;

    $params{supervised_dir} = $super_dir;
    $params{prefix} ||= "";

    bless {%params}, $class;
}

sub generate {
    my $self = shift;

    my @services = @{ $self->services };
    my @cur_svcs = @{ $self->cur_services };

    my %service_dirs = map { $_ => 1 } map { $_->{'name'} } @services;

    # currently running services that should not remain there
    my %to_remove;

    # these are services that should be running, just need to
    # check their run script
    my %to_check;

    for my $cur_svc (@cur_svcs) {
        if ( exists $service_dirs{$cur_svc} ) {
            $to_check{$cur_svc}++;
        }
        else {
            $to_remove{$cur_svc}++;
        }
    }

    my @to_check  = keys %to_check;
    my @to_remove = keys %to_remove;

    $self->ensure_services(
        services => \@services,
        to_check => \@to_check,
    );
    $self->remove_services(@to_remove);

    # refresh our view of the world after our checks
    @cur_svcs = @{ $self->cur_services };
    my %current   = map { $_ => 1 } @cur_svcs;
    my $sdir      = $self->{'service_dir'};
    my $super_dir = $self->{'supervised_dir'};

    # now we need to add the remaining services
    for my $svc (@services) {
        my $name = $svc->{'name'};
        unless ( exists $current{$name} ) {
            $svc->create($sdir);
        }

        # let's make sure all /service entries are created
        symlink( "$sdir/$name" => "$super_dir/$name" )
            unless -e "$super_dir/$name";
    }
}

sub parseJson {
    my $file = shift;
    my $json = Gearbox::Utils::read_file($file);
    my $ret = eval {
        $JSON::VERSION > 2 ? from_json($json) : jsonToObj($json)
    };
    if ($@) {
        croak("Error while parsing JSON config file $file: $@");
    }
    return $ret;
}

sub services {
    my $self = shift;

    my $file     = $self->{file};
    my $cfg      = parseJson($file);
    my $config_dir = $cfg->{config_dir} || "/etc/gearbox/config.d";
    if( -d $config_dir ) {
        for my $cf ( glob( "$config_dir/*.conf" ) ) {
            next unless -f $cf;
            my $base = File::Basename::basename($cf, ".conf");
            $cfg->{$base} = parseJson($cf);
        }
    }
    my $key        = $self->{key};
    my $daemon_cfg = $cfg->{$key};
    croak("Cannot access key $key in config file $file")
        unless $daemon_cfg;

    croak("$file :: $key should be an array ref")
        unless ref($daemon_cfg) eq "ARRAY";

    my @services;
    for my $daemon_spec (@$daemon_cfg) {
        push @services,
            Gearbox::Service::Entry->new_from_spec(
            %$daemon_spec,
            prefix => $self->{prefix},
            cfg    => $cfg
            );
    }

    return \@services;
}

sub cur_services {
    my $self = shift;

    my $dir = $self->{service_dir};
    opendir my $dh, $dir or croak("Can't opendir $dir");
    my @res = grep { $_ ne "." && $_ ne ".." } readdir($dh);
    closedir $dh;

    return \@res;
}

# make sure that the existing services are what
# we expect
sub ensure_services {
    my ( $self, %params ) = @_;
    my @services = @{ $params{services} };
    my @to_check = @{ $params{to_check} };

    my %svcs = map { $_->{'name'} => $_ } @services;
    my $sdir = $self->{'service_dir'};

    for my $name (@to_check) {
        $svcs{$name}->ensure($sdir);
    }

}

sub remove_services {
    my ( $self, @to_remove ) = @_;

    my $sdir      = $self->{'service_dir'};
    my $super_dir = $self->{'supervised_dir'};

    for my $name (@to_remove) {
        info("Removing service $name");

        if ( -e "$super_dir/$name" ) {
            unlink "$super_dir/$name"
                or croak("Could not unlink $super_dir/$name");
        }

        System("svc -dx $sdir/$name $sdir/$name/log");
        System("rm -rf $sdir/$name");
    }
}

1;
