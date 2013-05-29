use Test::More qw(no_plan);
use strict;
BEGIN { use_ok('QClassify') };

# [re]initMarkup should init markup even being called twice

my $n;
my $out;

my $qc = new QClassify("t/config3.xml");
$qc->index2file(); # save index
$qc->initMarkup(); # initialize markup: read config and load index file

# default (config) marker will be used
($n, $out) = $qc->markup("FreeBSD may be received via svn");
diag("Marked text (2): \"$out\"\n");
is($n, 1, "only 'svn' should be marked");
is($out, "FreeBSD may be received via <b>svn</b>");

#####################################################################################
# now we modify dictionary (+FreeBSD) in external-like mode
# (note aht `qc' object knows nothing about our indexing, so it's just external file
#
my $qc_tmp = new QClassify("t/config5.xml");
$qc_tmp->index2file(); # save index
#####################################################################################


$qc->initMarkup(); # initialize markup: config and index are rereadden

# default (config) marker will be used
($n, $out) = $qc->markup("FreeBSD may be received via svn");
diag("Marked text (2): \"$out\"\n");
is($n, 2, "'FreeBSD' and 'svn' should be marked");
is($out, "<b>FreeBSD</b> may be received via <b>svn</b>");

