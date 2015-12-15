#!/bin/sh

set -x

LIST=$(ps -ef | egrep "avrdude" | grep -v grep | awk '{ print $2 }')

for P in $LIST; do
    kill $P
done
