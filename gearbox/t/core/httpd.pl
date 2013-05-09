#!/usr/bin/env perl
######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################

use warnings;
use strict;
use POSIX;
use File::Basename;
use File::Path;

use HTTP::Daemon;
use HTTP::Status;
use HTTP::Response;
                 
my $d = HTTP::Daemon->new(
    LocalAddr => "127.0.0.1",
    ReuseAddr => 1,
) || die "Daemon: $!";

my $url = $d->url;
$url =~ s{//[^:]+:(\d+)/$}{//127.0.0.1:$1};
print $url;

my $child_ok = 0;
$SIG{HUP} = sub { $child_ok++ };

my $ppid = $$;
if( my $pid = fork() ) { 
    # parent writes out child pid and waits for child to signal us
    # that is is up and running
    open PID, ">./httpd.pid" or die "Failed to open ./httpd.pid: $!";
    print PID $pid;
    close PID;
    while( !$child_ok ) {
        sleep 1;
    }
    exit 0;
}
else {
    # child daemonizes now
    mkpath("./http/logs", 0, 0755) unless -d "./http/logs";
    close STDOUT;
    open STDOUT, ">>./http/logs/output.$$.log";
    $|++; # autoflush
    close STDIN;
    close STDERR;
    open STDERR, ">>./http/logs/output.$$.log";
    POSIX::setsid();
    # HUP parent to let them know we are done
    kill 'HUP', $ppid;
}


$SIG{ALRM} = sub { die "httpd lived to long" };
# only live for 15m
alarm(60 * 15);

my $counter = 0;
while (my $c = $d->accept) {
    my $r = $c->get_request();
    my $path = $r->uri->path;
    my $meth = lc $r->method; 
    
    open REQ, ">>./http/requests/$counter" or die "Failed to open ./http/requests/$counter: $!";
    my $req = $r->as_string();
    my $host = URI->new($url)->host_port;
    $req =~ s/$host/%{HOSTNAME}/g;
    print REQ $req;
    close REQ;
    
    if( $meth eq 'get' && $path =~ m{^/error/(\d+)} ) {
        my $resp = HTTP::Response->new($1);
        $resp->header("Connection", "close");
        my $file = "./http/$meth/$path";
        if( -f $file ) {
            my $content = do {
                local undef $/;
                open my $file, "<$file" or die "$file: $!";
                <$file>;
            };
            $resp->content($content);
        }
        else {
            $resp->content("ERROR");
        }
        $c->send_response( $resp );
    }
    elsif ( -f "./http/$meth/$path" ) {
        $c->send_file_response("./http/$meth/$path");
    }
    elsif( $meth eq 'post' || $meth eq 'put' ) {
        my $resp = HTTP::Response->new(200);
        $resp->header("Connection", "close");
        $resp->content( "OK");
        $c->send_response( $resp );
    }
    elsif ($meth eq 'get' and $path =~ m{^/curse} ) {
        my $resp = HTTP::Response->new(200);
        $resp->header("Content-Type", "text/x-clamation");
        $resp->header("Content-Language", "en_US");
        $resp->header("Connection", "close");
        $resp->content( "Dammit, I'm mad!");
        $c->send_response( $resp );
    }
    elsif ($meth eq 'get' and $path =~ m{^/sleep/(\d+)} ) {
        my $sleep = $1;
        $c->send_status_line;
        $c->send_basic_header;
        $c->send_header("Connection", "close");
        $c->send_crlf;
        select $c;
        $|++;
        select STDOUT;
        print $c "content\n";
        sleep $sleep;
        print $c "content\n";
    }
    elsif ($meth eq 'get' and $path =~ m{^/cookies} ) {
        my $sleep = $1;
        $c->send_status_line;
        $c->send_basic_header;
        $c->send_header("Set-Cookie" => "cook1=val1");
        $c->send_header("Set-Cookie" => "cook2=val2; Domain=127.0.0.1; Path=/");
        $c->send_header("Set-Cookie" => "cook3=val3; Expires=Mon, 18-Jan-2037 00:00:01 GMT");
        $c->send_crlf;
        select $c;
        $|++;
        select STDOUT;
        print $c "content\n";
        sleep $sleep;
        print $c "content\n";
    }
    else {
        $c->send_error(RC_FORBIDDEN)
    }
    $c->close;
    undef($c);
    $counter++;
}
