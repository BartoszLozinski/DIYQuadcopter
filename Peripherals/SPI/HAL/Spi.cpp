#include "Spi.hpp"

namespace Peripherals
{
    namespace HAL
    {
        Spi::Spi(SPI_HandleTypeDef& spiHandle_, const uint32_t timeout_ /* = HAL_MAX_DELAY */)
            : spiHandle(spiHandle_)
            , timeout(timeout_)
        {}

        SpiResult Spi::Transmit(std::span<uint8_t> data)
        {
            return HAL_SPI_Transmit(&spiHandle
                                , data.data()
                                , data.size()
                                , timeout) == HAL_OK ? SpiResult::Success : SpiResult::Error;
        }

        SpiResult Spi::Receive(std::span<uint8_t> buffer)
        {
            return HAL_SPI_Receive(&spiHandle
                                , buffer.data()
                                , buffer.size()
                                , timeout) == HAL_OK ? SpiResult::Success : SpiResult::Error;
        }
    };
};