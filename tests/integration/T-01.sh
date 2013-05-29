#!/usr/bin/env bash

. common.incl.sh

$BINDIR/idx_phrases -c ../T-01.config.xml

# i suppose the last phrase absent :-)
$BINDIR/cphrase -c ../T-01.config.xml -- "Петропавловск-Камчатский" "алгоритмы диагностических обследований" "алгоритмы диагностических обследований в России" "Шэрон Стоун фильмы" "Кисель Ян" | grep results >T-01.out 
compare T-01.out ../T-01.dump
