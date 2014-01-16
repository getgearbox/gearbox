#!/usr/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

TestDelayWorker->new($ARGV[0])->run();

package TestDelayWorker;
use strict;
use warnings;
use Gearbox::Worker;
use JSON;
use base qw(Gearbox::Worker);

use constant DBDIR => "/var/gearbox/db/test-delay-perl/";

sub new {
    my $self = shift->SUPER::new(@_);

    $self->register_handler( "do_get_testdelayperl_counter_v1" );
    $self->register_handler( "do_post_testdelayperl_counter_v1" );
    $self->register_handler( "do_delete_testdelayperl_counter_v1" );
    $self->register_handler( "do_increment_testdelayperl_counter_v1" );
    
    return $self;
}

my $json;
sub json {
    return $json || ($json = JSON->new->allow_nonref);
}

sub do_get_testdelayperl_counter_v1 {
    my ( $self, $job, $resp )  = @_;
    $resp->content( slurp( DBDIR . $job->resource_name() ) );
    return $WORKER_SUCCESS;
}

sub do_post_testdelayperl_counter_v1 {
    my ( $self, $job, $resp ) = @_;
    my $matrix = $job->matrix_arguments();
    my $start = 0;
    if( exists $matrix->{"start"} ) {
        $start = $matrix->{"start"};
    }
    write_file( DBDIR . $job->resource_name(), $start);

    $self->afterwards($job, "do_increment_testdelayperl_counter_v1", int $job->matrix_arguments()->{"delay"} || 1);
    return $WORKER_CONTINUE;
}

sub do_delete_testdelayperl_counter_v1 {
    my ( $self, $job, $resp ) = @_;
    unlink(DBDIR . $self->arguments()->[0]);
    return $WORKER_SUCCESS;
}
  
sub do_increment_testdelayperl_counter_v1 {
    my ($self,$job,$resp) = @_;
    my $newval = 1 + slurp(DBDIR . $job->resource_name());
    write_file( DBDIR . $job->resource_name(), $newval );
    my $matrix = $job->matrix_arguments();
    my $start = 0;
    if( exists $matrix->{"start"} ) {
        $start = $matrix->{"start"};
    }
    my $end = 10;
    if( exists $matrix->{"end"} ) {
        $end = $matrix->{"end"};
    }
    $resp->status()->add_message("set to " . $newval);
    if( $newval == $end ) {
        return $WORKER_SUCCESS;
    }
    else {
        $resp->status()->progress( $resp->status()->progress() + ($end - $start) );
    }

    if ( $job->matrix_arguments()->{"retry"} ) {
        $resp->status()->add_message( "retry attempt number " . ($resp->status()->failures()+1) );
        return $WORKER_RETRY;
    } else {
        $self->afterwards($job, int $job->matrix_arguments()->{"delay"} || 1);
    }
    return $WORKER_CONTINUE;
}

#
# Helper Routines
#

sub slurp {
    my $file = shift;
    return do {
        local $/ = undef;
        open my $fh, '<', $file or die "can't read contents of $file: $!\n";
        <$fh>;
    };
}

sub write_file {
    my $file = shift;
    my $contents = shift;
    open my $fh, ">", $file or die "Couldn't open $file to write: $!\n";
    print $fh $contents;
    close $fh;
    return;
}

1;
