######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::Service::Entry;

use strict;
use warnings;

use Carp;
BEGIN { $ENV{PERL_JSON_BACKEND} = 'JSON::PP' }
use JSON; # always uses JSON::PP
use Gearbox::Utils qw(debug info read_file write_file System);
use Data::Dumper;

my $CMDRUN = <<EOT;
#!/bin/sh

exec 2>&1
exec softlimit -c = setuidgid ::USER:: ::CMD::
EOT

sub new {
    my $class = shift;

    my %params = @_;
    croak "Need name, command, and user"
        unless ( $params{command}
        && $params{name}
        && $params{user} );

    bless {%params}, $class;
}

sub replace_var {
    my ($name, $spec, $cfg) = @_;
    return $spec->{$name} if exists $spec->{$name};
    return $cfg->{$name}  if exists $cfg->{$name};
    
    # name like key.subkey
    if( $name  =~ /[.]/ ) {
        my @parts = split /[.]/, $name;
        for my $data ( $spec, $cfg ) {
            my $cur = $data;
            my $found = 1;
            for my $part ( @parts ) {
                if( exists $cur->{$part} ) {
                    $cur = $cur->{$part};
                } else {
                    $found=0;
                    last;
                }
            }
            return $cur if $found;
        }
    }
    warn Data::Dumper->Dump([\$name, $spec, $cfg],['*name', 'spec', 'cfg']);
    croak("Failed to fined replacement token for %\{$name} in spec");
}

# returns a list of Gearbox::Service::Entry objects;
sub new_from_spec {
    my $class  = shift;
    my %params = @_;

    my $name    = $params{name}    or croak("Need a name");
    my $command = $params{command} or croak("Need a command");
    my $cfg     = $params{cfg}
        or croak("Need a cfg: the complete config for replacing tokens");

    my $prefix = $params{prefix} || "";
    my $count = defined( $params{count} ) ? $params{count} : 1;
    my $user = $params{user} || "root";
    my $logcmd = $params{logcmd} || $cfg->{logcmd} || $cfg->{log}->{cmd} || "";

    for my $var ( $prefix, $count, $user, $command, $logcmd ) {
        my $prev = $var;
        while( $var =~ s/%{([^}]+)}/replace_var($1,\%params,$cfg)/ge ) {
            # in case we replace %{foo} with %{foo} where "foo" is not
            # a variable in the config
            last if $prev eq $var;
            $prev = $var; 
        }
    }

    if ( $count == 1 ) {
        return $class->new(
            name    => $prefix . $name,
            command => $command,
            user    => $user,
            logcmd  => $logcmd
        );
    }
    my @results;
    for my $i ( 1 .. $count ) {
        my $real_name = sprintf( "$prefix$name-%02d", $i );
        push @results,
            $class->new(
            name    => $real_name,
            command => $command,
            user    => $user,
            logcmd  => $logcmd
            );
    }
    return @results;
}

sub create {
    my $self       = shift;
    my $servicedir = shift;

    my $name = $self->{name};

    info("Adding managed service $name");

    # create the current running service entry
    debug("Creating directory structure under $servicedir");

    my $dir = "$servicedir/$name";
    mkdir $dir;
    mkdir "$dir/log";

    # use multilog if no logger cmd line specified
    unless ($self->{logcmd}) {
        mkdir "$dir/log/main";
        my ( $uid, $gid ) = ( getpwnam("nobody") )[ 2, 3 ];
        chown $uid, $gid, "$dir/log/main";
    } else {
        System("rm -rf $dir/log/main");
    }
    write_file( "$dir/log/run", $self->generate_logger );
    chmod 0755, "$dir/log/run";

    my $cmd = $self->generate_cmd;
    write_file( "$dir/run", $cmd );
    chmod 0755, "$dir/run";
}

sub ensure {
    my ( $self, $servicedir ) = @_;

    my $name = $self->{'name'};
    my $dir  = "$servicedir/$name";
    my $ok   = 1;

    if ( read_file("$dir/run") eq $self->generate_cmd ) {
        debug("existing service $name run script matches our definition");
    } else {
        info("Existing service $name run script does not match. Fixing");
        undef $ok;
    }

    if ( read_file("$dir/log/run") eq $self->generate_logger ) {
        debug("existing service $name logger run script matches our definition");
    } else {
        info("Existing service $name logger run script does not match. Fixing");
        undef $ok;
    }

    return if $ok;

    $self->create($servicedir);
    info("Restarting $dir");
    System("svc -t $dir");
}

# We want to make this not depend on the object so it can be called
# by other classes
sub generate_cmd {
    my $self = shift;

    my $cmd      = $CMDRUN;
    my %replaces = (
        USER => $self->{user},
        CMD  => $self->{command},
    );

    $cmd =~ s/::(\w+)::/exists $replaces{$1} ? $replaces{$1} : $1/ge;
    return $cmd;
}

sub generate_logger {
    my $self = shift;

    my $logcmd = $self->{logcmd} || "multilog t ./main";

    return <<EOF;
#! /bin/sh

exec 2>&1
exec setuidgid nobody $logcmd
EOF
}

1;
