#pragma once

#include "../ISpi.hpp"

extern "C"
{
    #include "stm32l4xx_hal.h"
}

namespace Peripherals
{
    namespace HAL
    {
        class Spi : public ISpi
        {
        private:
            SPI_HandleTypeDef& spiHandle;
            uint32_t timeout{ HAL_MAX_DELAY };
        public:
            SpiResult Transmit(std::span<uint8_t> data) override;
            SpiResult Receive(std::span<uint8_t> buffer) override;
        
            Spi() = delete;
            Spi(const Spi&) = delete;
            Spi(Spi&&) = delete;
            Spi& operator=(const Spi&) = delete;
            Spi& operator=(Spi&&) = delete;
            ~Spi() = default;

            explicit Spi(SPI_HandleTypeDef& spiHandle_, const uint32_t timeout_ = HAL_MAX_DELAY);
        };
    };
};