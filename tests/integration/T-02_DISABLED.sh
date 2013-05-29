#!/usr/bin/env bash

. common.incl.sh

$BINDIR/idx_phrases -c ../T-02.config.xml
$BINDIR/cphrase -c ../T-02.config.xml -- "Ломоносова 10б" | grep results >T-02.out
compare T-02.out ../T-02.dump
