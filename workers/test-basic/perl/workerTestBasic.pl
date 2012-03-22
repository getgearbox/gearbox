#!/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

use strict;
use warnings;

my $worker = Gearbox::WorkerTestBasicPerl->new( $ARGV[0] );
$worker->run();

package Gearbox::WorkerTestBasicPerl;

use strict;
use warnings;

use Gearbox::Worker;
use base 'Gearbox::Worker';
use JSON qw(from_json to_json);
use File::Basename qw(basename);

use constant DBDIR => "/usr/var/gearbox/db/test-basic-perl/";

sub new {
    my $class = shift;
    my $config = shift;

    my $self = $class->SUPER::new( $config );

    $self->register_handler( "do_get_testbasicperl_thing_v1" );
    $self->register_handler( "do_put_testbasicperl_thing_v1" ); 
    $self->register_handler( "do_post_testbasicperl_thing_v1" );
    $self->register_handler( "do_delete_testbasicperl_thing_v1" );

    return $self;
}

sub do_get_testbasicperl_thing_v1 {
    my $self = shift;
    my $job  = shift;
    my $resp = shift;

    my $environ = $job->environ();
    if ( exists $environ->{"TestBasic"} ) {
        $resp->add_header( "TestBasic", $environ->{"TestBasic"} );
    }

    my $args = $job->arguments();
    if( !@$args ) { # index GET
        my @files = glob(DBDIR . "*");
        my $out;

        # set things to an empty array in case our glob did not match anything
       $out->{things} = [];
       my $limit = 10; ;
       
       my $params = $job->query_params();
       if ( $params->{_count} ) {
           $limit = $params->{_count};
       }
       
       for( my $i=0; $i<@files && $i<$limit; $i++ ){ 
            my $matrix = $job->matrix_arguments();
            if ( $matrix->{_expand} && $matrix->{_expand} == 1 ) { 
                $out->{things}[$i] = from_json( slurp( $files[$i] ) );
            } else {
                $out->{things}[$i] = basename( $files[$i] );
            }
        }

        $resp->content( to_json( $out ) );

    } else {
        my $name = $args->[0];
        if ( -e DBDIR . $name ) {
            $resp->content( slurp( DBDIR . $name ) );
        } else {
            die new ERR_NOT_FOUND("thing \"$name\" not found");
        }
    }

    return $WORKER_SUCCESS;
}


sub do_put_testbasicperl_thing_v1 {
    my $self = shift;
    my $job = shift;
    my $resp = shift;

    # async message, so update status to let user know we are processing
    $resp->status()->progress(10);
    $resp->status()->add_message("processing");

    my $args = $job->arguments();
    if( !$args ) {
        die new ERR_BAD_REQUEST("missing required resource name");
    }

    my $in = from_json( $job->content() );
    if ( !$in->{id} ) { 
        die new ERR_BAD_REQUEST("missing required \"id\" field");
    }

    write_file( DBDIR . $args->[0], $job->content() );
 
    $resp->status()->add_message("done");

    return $WORKER_SUCCESS;
}

sub do_post_testbasicperl_thing_v1 {
    my $self = shift;
    my $job  = shift;
    my $resp = shift;

    $resp->status()->progress(10);
    $resp->status()->add_message("processing");
      
    if ( $job->operation() eq "create" ) {
        # post-create where the resource id is created for user
        # (instead of a PUT where the user specifies the name)

        # get the generated id
        my $in = from_json( $job->content() );
        $in->{id} = $job->resource_name();

        write_file( DBDIR . $job->resource_name(), to_json( $in ) );
    } else { 
        my $args = $job->arguments();
        # post update
        if( -e DBDIR . $args->[0] ) {
            my $in = from_json( $job->content() );
            my $out = from_json( slurp( DBDIR . $args->[0] ) );
            $out->{stuff} = $in->{stuff};
            write_file( DBDIR . $args->[0], to_json($out) );
        }
        else {
            die new ERR_NOT_FOUND("thing \"" . $args->[0] . "\" not found");
        }
    }

    $resp->status()->add_message("done");
    return $WORKER_SUCCESS;
}

sub do_delete_testbasicperl_thing_v1 {
    my $self = shift;
    my $job = shift;
    my $resp = shift;

    $resp->status()->progress(10);
    $resp->status()->add_message("processing");

    # don't actually delete if fake-out header is set
    my $headers = $job->headers();
    if( $headers->{"fake-out"} ) {
        $resp->status()->add_message("ignoring delete due to fake-out header");
        return $WORKER_SUCCESS;
    }
        
    my $args = $job->arguments();
    if( !$args ) {
        die new ERR_BAD_REQUEST("missing required resource name");
    }
      
    my $file = DBDIR . $args->[0];
    if ( -f $file ) {
      unlink($file);
    }
    else {
      die new ERR_NOT_FOUND("thing \"" . $args->[0] . "\" not found");
    }
        
    $resp->status()->add_message("done");
    return $WORKER_SUCCESS;
}

sub slurp {
    my $file = shift;
    return do {
        local $/ = undef;
        open my $fh, '<', $file or die "can't read contents of $file: $!\n";
        my $content = <$fh>;
        close $fh;
        $content;
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
