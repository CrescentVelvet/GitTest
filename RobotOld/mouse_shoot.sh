#!/bin/bash
echo "Input 0 to stop the mouse"
while [[ "$#" -ne 0 ]]
do
    echo "$1"
    # xdotool mousemove 2200 550 click 1
    # sleep 1
    # xdotool mousemove 1200 350 click 1
    # sleep 1
    shift
done