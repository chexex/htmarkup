#!/usr/bin/perl -w
#
# streets_conv.pl 
# convert street names to regular expressions (PCRE style)
# applicable for qclassify
#
# ---------------------------------------------------
# Usage: get streets file and simply use it's output
# as input file to idx_phrases (qclassify)
# ---------------------------------------------------
#
# Copyright (C) 2009 Kisel Jan
#
use locale;
use strict;

use constant {
    HOME_EXPR   => "(д\\.?|дом|[-\\.])?\\s*\\d+[аб]?(\\s*\\/\\s+\\d+)?\\s*[\\.,]?(\\s*(к\\.|корп\\.?|корпус|с\\.|стр\\.|строение|[,-\\.])\\s*\\d+)?",
    CITY_EXPR   => "(г\\.|город)?\\s*(москва|петербург|санкт-петербург)",
};

my %transforms = (
    "аллея"      => "ал\\.?|аллея",
    "бульвар"    => "бул\\.?|бульвар[ауе]?",
    "вал"        => "вал",
    "линия"      => "л\\.|лин\\.?|линия",
    "набережная" => "наб\\.?|набережная",
    "переулок"   => "пер\\.?|переулок",
    "площадь"    => "пл\\.?|площадь",
    "проезд"     => "пр\\.?|проезд[ауе]?",
    "просек"     => "пр\\.?|просек",
    "просека"    => "пр\\.?|просека",
    "проспект"   => "пр\\.?|просп\\.?|проспект",
    "тупик"      => "туп\\.?|тупик",
    "улица"      => "ул\\.?|улица",
    "шоссе"      => "ш\\.?|шоссе",
);


my $SPL = "[,\\.]?";
my $prefix = join('|', keys(%transforms));
my ($nIn, $nskip, $nOut);

while (<>) 
{
    chomp;
    $nIn++;

    my @v;
    if (m/^($prefix)\s+(.+)$/) {
        my ($type, $name) = ($transforms{$1}, $2);
        push @v, sprintf("(%s$SPL\\s+)?(%s)\\s*%s$SPL(\\s*%s)?", CITY_EXPR, $type, $name, HOME_EXPR),
                 sprintf("(%s$SPL\\s+)?%s\\s+(%s)$SPL(\\s*%s)?", CITY_EXPR, $name, $type, HOME_EXPR),
                 sprintf("(%s$SPL\\s+)?%s$SPL\\s*%s\\s*(%s)", CITY_EXPR, HOME_EXPR, $name, $type),
                 sprintf("(%s$SPL\\s+)?(%s)?\\s*%s$SPL\\s*%s", CITY_EXPR, $type, $name, HOME_EXPR),
                 sprintf("(%s$SPL\\s+)?%s(\\s+(%s))?$SPL\\s*%s", CITY_EXPR, $name, $type, HOME_EXPR),
                 sprintf("(%s$SPL\\s+)?%s$SPL\\s*%s(\\s*%s)?", CITY_EXPR, HOME_EXPR, $name, $type);
    } 
    
    foreach (@v) {
        print "/^$_\$/i\n";
    }
    
    $nskip++ unless (scalar(@v));
    $nOut += scalar(@v);
}

printf STDERR ("\n" . '='x30 . "\nReadden: %d, %d skipped (%.1f %%)\n", $nIn, $nskip, ($nskip / $nIn) * 100.0);
printf STDERR ("Written: %d\n", $nOut);
