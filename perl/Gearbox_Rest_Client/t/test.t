#!/usr/bin/perl
use strict;
use warnings;
use lib qw(..);

use Test::More tests => 19;

BEGIN { use_ok( 'Gearbox::Rest::Client' ); }

my $c;
ok( $c = Gearbox::Rest::Client->new(sleep => 0), "test constructor" );

my $data;
ok( $data = $c->req("GET", "http://localhost:4080/test/thing/x-12345"), "test GET" );
is( $data->{"key"}, "value", "test response");

ok( $data = $c->req("DELETE", "http://localhost:4080/test/thing/x-12345"), "test DELETE" );
is( $data, 1, "test response");

ok( $data = $c->req("PUT", "http://localhost:4080/test/thing/x-12345", "content"), "test PUT" );
is( $data->{"key"}, "value", "test response");

ok( $data = $c->req("POST", "http://localhost:4080/test/thing/x-12345", "content"), "test POST" );
is( $data->{"key"}, "value", "test response");

use Log::Log4perl qw(:easy);
Log::Log4perl->easy_init({
    level    => $DEBUG,
    file     => "STDOUT",
    layout   => '# %p: %m%n'
});

ok( $c = Gearbox::Rest::Client->new(trace => \&Test::More::diag, sleep => 0), "test constructor" );
ok( $data = $c->req("GET", "http://localhost:4080/test/thing/x-12345"), "test GET" );
is( $data->{"key"}, "value", "test response");

ok( $data = $c->req("DELETE", "http://localhost:4080/test/thing/x-12345"), "test DELETE" );
is( $data, 1, "test response");

ok( $data = $c->req("PUT", "http://localhost:4080/test/thing/x-12345", "content"), "test PUT" );
is( $data->{"key"}, "value", "test response");

ok( $data = $c->req("POST", "http://localhost:4080/test/thing/x-12345", "content"), "test POST" );
is( $data->{"key"}, "value", "test response");

package LWP::UserAgent;
use HTTP::Response;
our %STATUS = ();
our $LAST_STATUS=0;
sub get_status {
    my $name = shift;
    unless ( $name ) {
        $name = "s-" . ++$LAST_STATUS;
    }
    my $progress;
    if( exists $STATUS{$name} ) {
        $STATUS{$name} += 10;
        $progress = $STATUS{$name};
    }
    else {
        $progress = $STATUS{$name} = 0;
    }
    
    my $code = "";
    if( $progress == 100 ) {
        $code = ", \"code\": 0";
    }

    return <<EOM;
    {
        "status_uri": "http://localhost:4080/test/status/$name",
        "uri": "http://localhost:4080/test/thing/x-12345",
        "progress": $progress,
        "status": 0
        $code
    }
EOM
}

no warnings 'redefine';
sub request {
    my ($self, $req) = @_;
    use Carp;
    if( $req->method eq "GET" ) {
        my $res = HTTP::Response->new(200);
        if( $req->uri->path() =~ /status/ ) {
            my $status = ($req->uri->path_segments())[-1];
            $res->content( get_status($status) );
        }
        else {
            $res->content('{"key":"value"}');
        }
        return $res;
    }
    else {
        my $res = HTTP::Response->new(202);
        $res->content( get_status() );
        return $res;
    }
}
    
        
