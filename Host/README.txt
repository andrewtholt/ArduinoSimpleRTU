
To run this, first:

Program the Arduino with simpleRTU.ino

Then

./run.sh

This setups the IPC and then runs listener and sender.

Then:

ficl -f tst2.fth

At the prompt type

main

and then (e.g.)

13 OUTPUT set-mode
13 1 set-pin
13 0 set-pin

And you should observe the on board LED light & extinguish.

