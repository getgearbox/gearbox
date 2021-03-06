#!/bin/zsh -f
#
#
set -e

SCRIPT=$0
LANGUAGES=(cpp perl python)

usage() {
    echo "Usage: $SCRIPT [--languages \"cpp perl python\"][--host gearbox]"
    echo "       [--test test-agents]"
    echo "    --languages defaults to \"$LANGUAGES\""
    exit 1
}

while [[ $# -ge 1 ]]; do
    case $1 in
        --languages) shift; LANGUAGES=($=1); shift ;;
        --host) shift ; export SMOKE_HOST=$1 ; shift ;;
        --test) shift ; TESTDIR=$1 ; shift ;;
        *) echo "Unknown option \"$1\""; usage ;;
    esac
done

for language in $LANGUAGES; do
    if [[ $language == "cpp" ]]; then
        unset PREFIX
    else
        export PREFIX=$language
    fi

    if [[ -z $TESTDIR ]]; then
        BASE="."
    else
        BASE=$TESTDIR
    fi

    find $BASE -name 'test.t' -print0 | \
        xargs --null -L1 zsh -c 'echo $0 ; cd $0:h && ./$0:t'
done
