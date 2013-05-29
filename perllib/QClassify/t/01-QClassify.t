use Test::More qw(no_plan);
use strict;
BEGIN { use_ok('QClassify') };

my $qc = new QClassify("t/config.xml");
$qc->index2file(); # omit this stage if you searching by already indexed array

my %cres; # classification result: { clsname => rating } hash

diag("checking format version ... " . $qc->version());
cmp_ok($qc->version(), '>=', 10, "format version");

%cres = $qc->classifyPhrase("портфель");
is(scalar keys %cres, 1, "phrase should be only in on class");
is($cres{'school'}, 100, "phrase should be 100% here");

%cres = $qc->classifyPhrase("учебник");
is(scalar keys %cres, 2, "phrase present in both classes");
is($cres{'school'}, 75, "phrase should be 75% in 'school' class");
is($cres{'hitech'}, 10, "phrase should be 10% in 'hitech' class");

%cres = $qc->classifyPhrase("мобильный телефон");
is(scalar keys %cres, 1, "phrase should be only once");
is($cres{'hitech'}, 75, "phrase should be 75% in 'hitech' class");


# now test for penalty; as you can see from config file, `school' class has
# `partial' penalty set to 1.0

%cres = $qc->classifyPhrase("школьный портфель");
is(scalar keys %cres, 0, "phrase is partial here");

# diff form
%cres = $qc->classifyPhrase("учебники");
is(scalar keys %cres, 1, "phrase present in both classes (but penalty for hitech)");
is($cres{'school'}, 75, "phrase should be 75% in 'school' class");

