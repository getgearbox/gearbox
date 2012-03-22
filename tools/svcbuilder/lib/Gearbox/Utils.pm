######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::Utils;

use strict;
use warnings;

use Carp;
use Fcntl;
use Exporter;

our @ISA = qw(Exporter);
our @EXPORT_OK
    = qw(read_file write_file debug set_verbose set_quiet info System);

sub read_file {
    my $file = shift;

    my $size = -s $file;
    croak("Invalid file $file\n") unless $size;

    sysopen my $fh, $file, O_RDONLY or croak("Can't open $file: $!");
    my $buf;
    my $read = sysread $fh, $buf, $size;
    croak("Can't read $size bytes from $file ($read bytes read)")
        unless $read == $size;
    close $fh;

    return $buf;
}

sub write_file {
    my ( $file, $contents ) = @_;

    open my $fh, ">", "$file.tmp"
        or croak("Can't open $file.tmp for writing: $!");
    print $fh $contents;
    close $fh;

    rename "$file.tmp" => $file;
}

sub System {
    my @cmd = @_;
    debug(@cmd);
    system(@cmd);
    if ($?) {
        info("ERROR: @cmd");
    }
    return $?;
}

# poor man's logging functions
# this is here so we don't have to depend on the
# many Log::Log4perl dependencies for this simple
# service
{
    my $verbose;

    sub set_verbose {
        $verbose = 1;
    }

    sub set_quiet {
        $verbose = 0;
    }

    sub debug {
        return unless $verbose;
        print "@_\n";
    }

    sub info {
        print "@_\n";
    }
}

1;
