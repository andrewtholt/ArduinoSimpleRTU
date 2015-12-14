#!/bin/sh

set -x

LIST=$(ps -ef | egrep "sender|listener" | grep -v grep | awk '{ print $2 }')

for P in $LIST; do
    kill $P
done
