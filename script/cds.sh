#! /bin/bash -e

PATH=/bin:/usr/bin:/sbin:/usr/sbin

OPTIONS=""

if [ -n "REDIS" ]; then
    OPTIONS="$OPTIONS --redis $REDIS"
fi

if [ -n "$AMAZON_ACCESS_KEY" ]; then
    OPTIONS="$OPTIONS --amazon-access-key $AMAZON_ACCESS_KEY"
fi

if [ -n "$AMAZON_KEY" ]; then
    OPTIONS="$OPTIONS --amazon-key $AMAZON_KEY"
fi

if [ -n "$TMDB_KEY" ]; then
    OPTIONS="$OPTIONS --tmdb-key $TMDB_KEY"
fi

if [ -n "$DIRECTORY" ]; then
    OPTIONS="$OPTIONS --directory $DIRECTORY"
fi

echo "start /usr/local/bin/cds $OPTIONS"
#/usr/local/bin/cds $OPTIONS
