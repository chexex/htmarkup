use Test::More qw(no_plan);
use strict;
BEGIN { use_ok('QClassify') };

my $n;
my $out;

my $qc = new QClassify("t/config.xml");
$qc->initMarkup();
($n, $out) = $qc->markup("портфель");
is($n, 1, "simple markup");
($n, $out) = $qc->markup("портфель портфель и еще раз портфель");
is($n, 3, "config has no any restricts");

undef $qc; # try destructor

my $qc2 = new QClassify("t/config2.xml");
$qc2->index2file();
$qc2->initMarkup();

($n, $out) = $qc2->markup("Windows and Linux");
is($n, 1, "gap setting allows only 1 to be marked");

($n, $out) = $qc2->markup("Windows and last version of Linux");
is($n, 2, "gap is small enought, and allows 2 phrases be marked");

($n, $out) = $qc2->markup("Linux Linux and ... Linux!");
is($n, 1, "config has 'uniq' restriction");

#now test marker, it has format: %P (%U)
($n, $out) = $qc2->markup("you see Linux");
is($n, 1, "marker absent");
is($out, "you see <b>Linux (Linus Torvalds)</b>", "wrong marker");

undef $qc2; # try destructor
