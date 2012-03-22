######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::Worker;

use strict;
use warnings;

use Gearbox;
use base 'Gearbox::PerlWorker';
use Gearbox::JobManager;
use Gearbox::Errors;
use Gearbox::Logger;

our $WORKER_SUCCESS  = $Gearbox::SwigWorker::WORKER_SUCCESS;
our $WORKER_ERROR    = $Gearbox::SwigWorker::WORKER_ERROR;
our $WORKER_CONTINUE = $Gearbox::SwigWorker::WORKER_CONTINUE;
our $WORKER_RETRY    = $Gearbox::SwigWorker::WORKER_RETRY;


# crude Exporter
sub import { 
    my ($pkg) = caller();
    for my $code ( qw(SUCCESS ERROR CONTINUE RETRY) ) {
        no strict 'refs';
        *{"${pkg}::WORKER_$code"} = \${"WORKER_${code}"};
    }

    # export log routines from Gearbox::Logger
    for my $code ( qw(_TRACE _DEBUG _INFO _WARN _ERROR _FATAL) ) {
        no strict 'refs';
        *{"${pkg}::$code"} = \&{"$code"};
    }
}

sub pre_request {
    my ($self,$job) = @_;
    if( my $s = $job->status() ) {
        Log::Log4perl::NDC->push($s);
    }
    if( my $r = $job->resource_name() ) {
        Log::Log4perl::NDC->push($r);
    }

    return 0;
}

sub post_request {
    Log::Log4perl::NDC->remove();
    return 0;
}

sub new {
    my ($class,$config) = @_;

    if ( !$config || !-f $config ) {
        die "missing config file\n";
    }

    my $self = $class->SUPER::new( $config );
    $self->set_self($self);
    Gearbox::Logger->init($config);
    return $self;
}

sub job_manager {
    my $jm = shift->SUPER::job_manager();
    bless $jm, "Gearbox::JobManager";
    return $jm;
}

sub DESTROY { 
    my $self = shift;

    delete $Gearbox::Worker::Autoloader::OBJ{ $self };
    return $self->SUPER::DESTROY(@_);  
}


package Gearbox::Worker::Autoloader;

use Carp;
unshift @Gearbox::ISA, __PACKAGE__;

our %OBJ;
sub AUTOLOAD {
    my $self = $_[0];

    our $AUTOLOAD;
    if ( !UNIVERSAL::isa( $self, "Gearbox::PerlWorker" ) ) {
        croak "method not found $AUTOLOAD\n";
    }

    no strict 'refs';
    if ( $AUTOLOAD =~ /swig_(.*)_set$/ ) {
        my $field = $1 or die "no field";

        *{$AUTOLOAD} = sub {
            my $self = shift;
            my $val = shift;
            $OBJ{$self}{$field} = $val;
        };
    } elsif ( $AUTOLOAD =~ /swig_(.*)_get$/ ) {
        my $field = $1 or die "no field";
        *{$AUTOLOAD} = sub {
            my $self = shift;
            return $OBJ{$self}{$field};
        }
    }
    else {
        croak "$AUTOLOAD method not found\n";
    }

    goto &$AUTOLOAD;
}

package Gearbox;
no warnings 'redefine';

sub CLEAR {
    %{$OBJ{$_[0]}} = ();
    return;
}

sub FIRSTKEY {
    my $a = keys %{$OBJ{$_[0]}};
    return each %{$OBJ{$_[0]}}
}

sub NEXTKEY {
    return each %{$OBJ{$_[0]}}
}



1;


