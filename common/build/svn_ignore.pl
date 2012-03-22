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
use File::Temp qw(tempfile);
use Cwd;
use Data::Dumper;

my @dirs = tick('find . -type d \! -path \*/.svn\*');
chomp(@dirs);
@dirs = grep { -d "$_/.svn" } @dirs;
my $top = cwd();

DIR:
for my $dir ( @dirs ) {
    chdir($top);
    chdir($dir);
    my @files = tick("svn status . --depth immediates");
    chomp(@files);
    next unless $? == 0;
    my @ignores = sort grep { s/^[?]\s+// } @files;
    next unless @ignores;
    my ($fh, $filename) = tempfile(CLEANUP => 1);
    my @bad = grep { /[.](c|h|cc|hh|pl|yicf|yidf|state)$/i } @ignores;
    if( @bad ) {
        warn "Cowardly refusing to ignore these files in $dir:\n    ",
            join("\n    ", @bad), "\n";
        warn "Either svn add this file, or manually edit svn:ignore property\n",
            "on $dir to ignore this file.\n";
        next;
    }
    s/^.*?([.][^.]+)$/*$1/ for @ignores;
    my %uniq;
    # make @ignores unique
    @ignores = grep { !$uniq{$_}++ } @ignores;
    while( 1 ) {
        print $fh join("\n", @ignores), "\n";
        print "Adding these ignores to $dir:\n";
        print join("\n", @ignores), "\n";
        print "OK? [Y\\n]:";
        my $ans = <>;
        if( $ans =~ /^(y(es)?|)$/i ) {
            last;
        }
        elsif( $ans =~ /^n(o)?$/i ) {
            next DIR;
        }
    }
    close($fh);
    system("svn pe svn:ignore . --editor-cmd='cat $filename >>'");
}

sub tick {
    my $cmd = join(" ", @_);
    #warn "tick: $cmd\n";
    return qx{$cmd};
}
