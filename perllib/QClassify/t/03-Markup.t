use Test::More qw(no_plan);
use strict;
BEGIN { use_ok('QClassify') };

my $n;
my $out;

my $qc2 = new QClassify("t/config3.xml");
$qc2->index2file(); # save index
$qc2->initMarkup(); # initialize markup: read config and load index file

# default (config) marker will be used
($n, $out) = $qc2->markup("FreeBSD may be received via svn");
is($n, 1, "only 'svn' should be marked");
is($out, "FreeBSD may be received via <b>svn</b>");

# instead older of %U (udata) we use marker itself
($n, $out) = $qc2->markup("Linux may be received via git");
is($n, 1, "only 'git' should be marked");
is($out, "Linux may be received via <a href=\"http://git-scm.com/\">git</a>&nbsp;<img src=\"http://git-scm.com/%E3%E8%F2/favicon.png\">");

