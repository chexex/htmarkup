compare() {
    local orig=$1
    local new=$2

    if ! diff -U2 $orig $new; then
        exit 1
    fi
}


UNIVERSE_DIR=_UNIVERSE_
rm -rf $UNIVERSE_DIR
mkdir $UNIVERSE_DIR

cd $UNIVERSE_DIR
ROOTDIR=../../.. # from inside the $UNIVERSE_DIR
BINDIR=$ROOTDIR/qclassify
DATADIR=$ROOTDIR/data/phrases_proj

set -e
set -o pipefail
set -x
