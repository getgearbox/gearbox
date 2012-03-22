######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::JobManager;
use Gearbox;
use base qw(Gearbox::RealJobManager);

sub new {
    return shift->SUPER::new(@_);
}

sub gen_id {
    shift if (ref($_[0]) && UNIVERSAL::isa($_[0], __PACKAGE__)) || $_[0] eq __PACKAGE__;
    return Gearbox::RealJobManager::gen_id(@_);
}

sub requireSchemas {
    shift if (ref($_[0]) && UNIVERSAL::isa($_[0], __PACKAGE__)) || $_[0] eq __PACKAGE__;
    return Gearbox::RealJobManager::requireSchemas(@_);
}
    

sub job_queue_apply {
    # ignore first arg in case it is class or $self
    shift if ref($_[0]) ne 'ARRAY';
    my($queue,$sub,$data) = @_;
    for my $level ( @$queue ) {
        for my $job ( @$level ) {
            $sub->($job,$data);
        }
    }
}

sub job_queue_run {
    # ignore first arg in case it is class or $self
    shift if ref($_[0]) ne 'ARRAY';
    return Gearbox::RealJobManager::job_queue_run($_[0]);
}

1;
