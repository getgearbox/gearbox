#!/bin/zsh -f
#
#
# script is used to setup initial target language by using cpp as a template

SCRIPT=$0

usage() {
    echo "Usage: $SCRIPT <--language python>"
    echo "       [--dir test-agents] [--dir test-basic]"
    exit 1
}

while [[ $# -ge 1 ]]; do
    case $1 in
        --language) shift; LANGUAGE=$1; shift ;;
        --dir)      shift; TEST_DIRS=( $TEST_DIRS $1 ); shift ;;
        *) echo "Unknown option \"$1\""; usage ;;
    esac
done

if [[ -z $LANGUAGE ]]; then
    echo "--language is required."
    usage
fi

case $LANGUAGE in
    python) EXT=".py" ;;
    *) echo "unknown language $LANGUAGE for correct extention. exiting..."
        exit 1
        ;;
esac

if [[ -z $TEST_DIRS ]]; then
    TEST_DIRS=( test-* )
fi

build_schemas_symlinks() {
    cd $LANGUAGE/schemas
    rm -f *

    set -x
    for file in ../../schemas/*; do
        testname=${${${file:t}#*-}/-*}
        set -x
        ln -s $file ${${file:t}/$testname/${testname}$LANGUAGE}
        set +x
    done
    set -x
    cd ../..
}

build_config() {
    MODULE=$1

    sed -e "s|Location |Location /$LANGUAGE|" \
        -e "s|/gearbox/$MODULE.conf|/gearbox/$MODULE-$LANGUAGE.conf|" \
        conf/httpd-$MODULE.conf > \
            $LANGUAGE/conf/httpd-$MODULE-$LANGUAGE.conf

    #test-cancel -> testcancel
    ALTNAME=${MODULE/-/}
    #test-cancel - TestCancel
    NAMEONE=${MODULE/-*}
    NAMETWO=${MODULE/*-}
    CAMELNAME=${(C)NAMEONE[1]}$NAMEONE[2,-1]${(C)NAMETWO[1]}$NAMETWO[2,-1]

    sed -e "s|$ALTNAME|${ALTNAME}$LANGUAGE|" \
        -e "s|worker$CAMELNAME|worker$CAMELNAME$EXT|" \
        -e "s|$MODULE.conf|$MODULE-$LANGUAGE.conf|" \
        conf/$MODULE.conf > \
            $LANGUAGE/conf/$MODULE-$LANGUAGE.conf

    #only used for test-agents
    if [[ $MODULE == "test-agents" ]]; then
        sed -e "s|_testagents_|_testagents${LANGUAGE}_|" \
            conf/$MODULE-agents.conf > \
                $LANGUAGE/conf/$MODULE-$LANGUAGE-agents.conf
    fi
}

for directory in $TEST_DIRS; do
    set -x
    cd $directory
    mkdir -p $LANGUAGE/{conf,schemas}

    build_config $directory
    build_schemas_symlinks

    cd ..
done
