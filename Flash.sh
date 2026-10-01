#!/bin/bash

CONFIG=${1:-Debug}
if [[ "$CONFIG" != "Debug" && "$CONFIG" != "Release" ]]; then
    echo "Usage: $0 [Debug|Release]"
    exit 1
fi

openocd -f /usr/share/openocd/scripts/interface/stlink.cfg \
        -f /usr/share/openocd/scripts/target/stm32l4x.cfg \
        -c "program build/$CONFIG/Core/DrivingRobot verify reset exit"
