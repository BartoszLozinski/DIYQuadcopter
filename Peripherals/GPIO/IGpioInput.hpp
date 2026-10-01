#pragma once

namespace Peripherals
{
    class IGpioInput
    {
    public:
        virtual uint32_t Read() const = 0;
        virtual ~IGpioInput() = default;
    };
}