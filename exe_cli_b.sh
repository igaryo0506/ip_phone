#!/bin/bash
./3p_client "$1" "49990" | play -t raw -b 16 -c 1 -e s -r 44100 -