#!/usr/bin/env bash

# bugfix: idx_phrases fail to index empty database

. common.incl.sh

touch emptyfile.txt
$BINDIR/idx_phrases -c ../T-04-empty.config.xml
$BINDIR/cphrase -c ../T-04-empty.config.xml -- 'проверка корректности' | grep results >T-04-empty.out 
compare T-04-empty.out ../T-04-empty.out.dump
