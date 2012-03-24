#!/usr/bin/env perl

use Data::Dumper;

use Ham::APRS::FAP qw(parseaprs);

my $aprspacket = $ARGV[0] || 'OH2RDP>BEACON,OH2RDG*,WIDE:!6028.51N/02505.68E#PHG7220/RELAY,WIDE, OH2AP Jarvenpaa';

my %packetdata;
my $retval = parseaprs($aprspacket, \%packetdata);

if ($retval == 1) {
    # decoding ok, do something with the data

    my @q = sort keys(%packetdata);
    

    print Dumper(%packetdata). "\n";

    for my $key (@q) {
	my $value = $packetdata{$key};
#	print "$key: $value\n";
	if ("wx" == $key) {
	    
	}
    }
} else {
    warn "Parsing failed: $packetdata{resultmsg} ($packetdata{resultcode})\n";
}
