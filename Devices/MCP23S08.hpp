#pragma once

#include <cstdint>

namespace Device
{
    enum class MCP23S08Register : uint8_t
    {
        IODIR = 0x00,
        IPOL = 0x01,
        GPINTEN = 0x02,
        DEFVAL = 0x03,
        INTCON = 0x04,
        IOCON = 0x05,
        GPPU = 0x06,
        INTF = 0x07,
        INTCAP = 0x08,
        GPIO = 0x09,
        OLAT = 0x0A
    };
};