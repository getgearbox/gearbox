######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
BEGIN {
    use File::Basename;
    use Cwd;
    my $path = File::Basename::dirname(Cwd::realpath($0));
    my $stubpath = Cwd::realpath("$path/../../../../common/stub");

    my $PATH= Cwd::realpath("$stubpath/.libs") . ":" . join ":", map { Cwd::realpath("$path/../../../$_/.libs") } qw(core job worker store);
    
    # $ENV{DYLD_LIBRARY_PATH} = "$path/.libs:$PATH";
    # $ENV{LD_LIBRARY_PATH} = "$path/.libs:$PATH";
    # print "DYLD_LIBRARY_PATH: $ENV{DYLD_LIBRARY_PATH}\n";
    # require DynaLoader;
    # @ISA = qw(DynaLoader);
    # bootstrap setup;
    
    
    unless( $ENV{LD_LIBRARY_PATH} eq $PATH ) {
        $ENV{LD_LIBRARY_PATH} = $PATH;
        my $stub = "$stubpath/.libs/libgearman_stub.so";
        $ENV{LD_PRELOAD} = $stub;
        # for OSX
        $ENV{DYLD_LIBRARY_PATH} = $PATH;
        $ENV{DYLD_INSERT_LIBRARIES} = "$stubpath/.libs/libgearman_stub.dylib";
        $ENV{DYLD_FORCE_FLAT_NAMESPACE} = 1;
        exec($0,@ARGV);
    }
    push @INC, Cwd::realpath("$path/../../../swig/perl");
    push @INC, Cwd::realpath("$path/../../../swig/perl/.libs");
    push @INC, Cwd::realpath("$path/../../../swig/perl/lib");
    $0 = Cwd::realpath($0);
    chdir($path);
}

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
