######################################################################
# Copyright (c) 2012, Yahoo! Inc. All rights reserved.
#
# This program is free software. You may copy or redistribute it under
# the same terms as Perl itself. Please see the LICENSE.Artistic file 
# included with this project for the terms of the Artistic License
# under which this project is licensed. 
######################################################################
package Gearbox::Rest::Client;
use strict;
use warnings;
use base qw(LWP::UserAgent);
use Regexp::Common;
use POSIX qw(strftime);
use JSON qw(from_json to_json);
use Log::Log4perl qw(:easy);

our $PKG = __PACKAGE__;

sub new {
    my $pkg = shift;
    my $self = bless $pkg->SUPER::new(@_), $pkg;
    $self->{$PKG} = {auto_poll => 1, throw => 1, sleep => 1, @_};
    return $self;
}

sub res {
    my $self = shift;
    return $self->{$PKG}->{res};
}

sub status {
    return $_[0]->{status};
}

sub req {
    my ($self, $method, $uri, $content, $headers) = @_;
    my $req = $self->{$PKG}->{req} = HTTP::Request->new($method,$uri);
    $req->header("Content-Type", "application/json" );
    if( $content ) {
        $req->content($content);
    }
    if( $headers && ref($headers) eq 'HASH' ) {
        while( my($k,$v) = each %$headers ) {
            $req->header($k,$v);
        }
    }
    
    # reset the status if this is a new (non internal) request
    delete $self->{status} unless $self->{internal_req};
    
    DEBUG($req->method() . " " . $req->uri()->canonical())
        unless $self->{internal_req};

    TRACE(
        sub {
            my $msg =
                "Request:\n".
                $self->pretty($req->as_string());
            return $msg;
        }
    );
        
        
    my $res =  $self->request($req);
    $self->{$PKG}->{res} = $res
        unless $self->{internal_req};

    local $self->{internal_req} = 1;

    TRACE(
        sub {
            my $msg =
                "Response:\n".
                $self->pretty($res->as_string());
            return $msg
        }
    );

    if ( $res->code == 202 ) {
        # accepted, so start polling
        my $json = eval { from_json($res->decoded_content(), {allow_nonref => 1}) };
        if( $@ ) {
            die "invalid JSON returned from $method $uri\n".
                "Got:\n". $res->decoded_content();
        }
        $self->{status} = $json;
        if ( ! $self->{$PKG}->{auto_poll} ) {
            DEBUG("Accepted: " . $json->{status_uri});
            return $json;
        }

        my $last_progress = -1;
        while( $json->{progress} != 100 ) {
            $json = $self->req("GET", $json->{status_uri});
            if( $json->{progress} != $last_progress ) {
                DEBUG sprintf("Progress %3i%% for %s", $json->{progress}, $json->{status_uri});
                $last_progress = $json->{progress};
            }
            last if $json->{progress} == 100;
            sleep( $self->{$PKG}->{sleep} );
        }
        if( $json->{code} != 0 ) {
            return $json unless $self->{$PKG}->{throw};
            die Gearbox::Rest::Client::Error->new($json);
        }

        return 1 if $method eq 'DELETE';
        return $self->req("GET", $json->{uri});
    }
    elsif( $res->is_success ) {
        # no content returned from sync DELETE
        return 1 if $method eq 'DELETE';
        my $json = eval { from_json($res->decoded_content(), {allow_nonref => 1}) };
        if( $@ ) {
            die "invalid JSON returned from $method $uri\n".
                "Got:\n". $res->decoded_content();
        }
        
        if ( (!$self->{status} && $uri =~ /status/) || 
                ( $self->{status} && $uri eq $self->{status}->{status_uri} ) ) {
            $self->{status} = $json;
        }
        return $json;
    }
    else {
        my $json = eval { from_json($res->decoded_content(), {allow_nonref => 1}) };
        if( $@ ) {
            # error out
            die "=====================================================\n".
                "Request:\n".
                $req->as_string().
                "=====================================================\n".
                "Response:\n".
                $res->as_string().
                "=====================================================\n";
        }
        else {
            $self->{status} = $json;
            return $json unless $self->{$PKG}->{throw};
            die Gearbox::Rest::Client::Error->new($json);
        }
    }
}

sub pretty {
    my( $self, $str ) = @_;
    $str =~ s/($Regexp::Common::RE{balanced}{-parens=>'{}[]'})/to_json(from_json($1, {allow_nonref => 1}), {pretty => 1})/eg;
    return $str;
}

package Gearbox::Rest::Client::Error;
use overload
    '""'   => \&to_string,
    '0+'   => \&to_int;

sub new {
    my($pkg,$data) = @_;
    return bless $data, $pkg;
}

sub to_string {
    JSON::to_json({%{$_[0]}}, {pretty => 1, allow_nonref => 1})
}

sub to_int {
    return $_[0]->{code};
}

1;
