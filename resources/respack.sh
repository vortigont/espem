#!/bin/sh

USAGE="Usage: `basename $0` [-h] [-t embuitag] [-f]"

# embui branch/tag name to fetch
embuirepo='https://github.com/vortigont/EmbUI'
embuitag="v2.6"

#####
# no changes below this point!

# etag file
tags=etags.txt

refresh_data=0
refresh_js=0

embuijs_files='lib.js maker.js dyncss.js'

# parse cmd options
while getopts hft: OPT; do
    case "$OPT" in
        h)
            echo $USAGE
            exit 0
            ;;
        f)
            echo "Force refresh"
            rm -f $tags
            refresh_data=1
            refresh_js=1
            ;;
        t)
            echo "EmbUI tag is set to: $OPTARG"
            embuitag=$OPTARG
            ;;
        \?)
            # getopts issues an error message
            echo $USAGE >&2
            exit 1
            ;;
    esac
done


[ -f $tags ] || touch $tags

# check github file for a new hash
freshtag(){
    local url="$1"
    etag=$(curl -sL -I $url | grep etag | awk '{print $2}')
    echo "$url $etag" >> newetags.txt
    if [ $(grep -cs $etag $tags) -eq 0 ] ; then
        #echo "new tag found for $url"
        return 0
    fi
    #echo "old tag for $url"
    return 1
}

# get resource file from EmbUI repo
getResgz(){
    local res=$1
    local url="${embuirepo}/raw/$embuitag/resources/html/${res}"
    if freshtag ${url} ; then
        curl -sL $url | gzip -9 > ../data/${res}.gz
    fi
}

# update local file
updlocalgz(){
    local res=$1
    [ ! -f ../data/${res} ] || [ html/${res} -nt ../data/${res}.gz ] &&  gzip -9kf html/${res} && mv -f html/${res}.gz ../data/${res}.gz
}

echo "Preparing resources for ESPEM FS image into ../data/ dir" 

mkdir -p ../data/css ../data/js

# собираем скрипты и стили из репозитория EmbUI
if freshtag ${embuirepo}/raw/$embuitag/resources/data.zip ; then
    refresh_data=1
    echo "EmbUI resources requires updating"
else
    echo "EmbUI is up to date"
fi

# check for newer js files
for f in ${embuijs_files}
do
    if freshtag ${embuirepo}/raw/$embuitag/resources/html/js/${f} ; then
        refresh_js=1
        break
    fi
done


# check if there are any fresh styles for the project to merge with EmbUI
if [ -f html/css/style_*.css ] ; then
    for f in "html/css/style_*.css"
    do
        [ ! -f ../data/css/$( basename $f).gz ] || [ $f -nt ../data/css/$( basename $f).gz ] && refresh_data=1
    done
fi

# if any of the styles needs updating, than we need to repack both embui and local files
if [ $refresh_data -eq 1 ] ; then

    echo "Refreshing EmbUI css files/pics..."

    curl -sL ${embuirepo}/raw/$embuitag/resources/data.zip > embui.zip
    unzip -o -d ../data/ embui.zip "css/*" "js/tz*"

    # append our local styles to the embui
    if [ -f html/css/style_*.css ] ; then
        for f in "css/style_*.css"
        do
            gzip -d ../data/css/$( basename $f).gz
            cat $f >> ../data/css/$( basename $f)
            gzip -9 ../data/css/$( basename $f)
            touch -r $f ../data/css/$( basename $f).gz
        done
    fi

    rm -f embui.zip
fi

if [ $refresh_js -eq 1 ] ; then
    echo "refreshing embui js files..."

    rm -f embui.js
    for f in ${embuijs_files}
    do
        curl -sL ${embuirepo}/raw/$embuitag/resources/html/js/${f} >> embui.js
    done
    gzip -9 embui.js && mv -f embui.js.gz ../data/js/
fi


# update index file if missing or newer
if [ ! -f html/index.html ] ; then
    getResgz index.html
else
    updlocalgz 'index.html'
fi

if [ ! -f html/favicon.ico ] ; then
    getResgz favicon.ico
else
    updlocalgz 'favicon.ico'
fi

mv -f newetags.txt $tags

## update local js/css if newer
updlocalgz 'js/espem.js'
updlocalgz 'css/espem.css'
