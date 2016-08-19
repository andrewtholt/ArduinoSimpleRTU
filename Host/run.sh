#!/bin/sh

set -x
PATH=$PATH:/home/andrewh/Arduino/sketchbook/simpleRTU/Host

starter

if [ $? -ne 0 ]; then
    echo "Starter failed."
    exit 1
fi


listener -p /dev/ttyUSB0 &
sleep 1

sender -p /dev/ttyUSB0 &



