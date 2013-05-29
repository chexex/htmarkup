use Test::More qw(no_plan);
use strict;
BEGIN { use_ok('QClassify') };

my $qc = new QClassify("t/config_bug1.xml");
diag("trying to load absent index, should not cause SIGSEGV");
fail("initMarkup should fail") if $qc->initMarkup();
isnt($qc->error(), 0, "index file not been found: error must not give 0");

is($qc->getIndexFileName(), "t/phrases_not_found.idx", "getIndexFileName");

fail("loadConfig(good config) should be ok") unless $qc->loadConfig("t/config3.xml");

diag("trying to open non-existant config");
my $qc2 = new QClassify("t/config_unexistant.xml");
isnt($qc2->error(), 0, "config file not been found: error must not give 0");

