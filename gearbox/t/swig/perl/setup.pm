######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
BEGIN {
    $ENV{OBJDIR} ||= "i386-rhel4-gcc3";
    $ENV{OBJDIR} = "i386-rhel4-gcc3" if $ENV{OBJDIR} = '.';
    $ENV{OBJDIR} .= ".$ENV{MODE}" if $ENV{MODE};
    my $PATH= join ";", map { "../../../$_/$ENV{OBJDIR}" } qw(core job worker scoreboard store);
    unless( $ENV{LD_LIBRARY_PATH} eq $PATH ) {
        my $stub = "$ENV{OBJDIR}/libgearman_stub.so.0";
        unless ( -f $stub ) {
            system("make quick MODE=$ENV{MODE}>/dev/null 2>&1");
        }
        $ENV{LD_LIBRARY_PATH} = $PATH;
        $ENV{LD_PRELOAD} = $stub;
        exec($0,@ARGV);
    }
}

use lib "../../../swig/perl";
use lib "../../../swig/perl/$ENV{OBJDIR}-perl58";
use lib "../../../swig/perl/lib";

use Error qw(:try);

sub THROWS(&) {
    my $code = shift;
    return try { return $code->() } otherwise {
        my $err = shift;
        if( UNIVERSAL::isa($err, "Gearbox::Error") ) {
            return "$err";
        }
        else {
            return $err->{-text} 
        }
    };
}

sub NOTHROW(&) {
    local $main::TODO = $Test::Trivial::TODO;
    my $code = shift;
    my $test = try { $code->(); "" } otherwise { return shift->{-text} };
    my $msg ||= Test::Trivial::line_to_text();
    $msg .= " does not throw an exception";
    if( $VERBOSE ) {
        require Data::Dumper;
        warn "--------------------------------------------------------\n";
        warn Data::Dumper->Dump([$test], ["NOTHROW"]);
        warn "--------------------------------------------------------\n";
    }
    my $output;
    Test::Trivial::capture_io(\$output);
    my $ok = Test::Trivial::is_deeply($test, "", $msg);
    Test::Trivial::reset_io();
    Test::Trivial::warn_line_failure() unless $ok;
    print $output;
    $ok || ($Test::Trivial::FATAL && die "All errors Fatal\n");
}

1;
