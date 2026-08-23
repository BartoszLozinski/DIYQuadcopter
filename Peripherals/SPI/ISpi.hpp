#pragma once

#include <cstdint>
#include <span>

namespace Peripherals
{
    enum class SpiResult
    {
        Success,
        Busy,
        Error,
    };

    class ISpi
    {
    public:
        virtual ~ISpi() = default;
        virtual SpiResult Transmit(std::span<uint8_t> data) = 0;
        virtual SpiResult Receive(std::span<uint8_t> buffer) = 0;
    };
};