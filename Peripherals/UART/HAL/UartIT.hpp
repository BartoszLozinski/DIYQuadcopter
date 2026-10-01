#pragma once

#include "IUart.hpp"
#include "Utils/RingBuffer.hpp"

extern "C"
{
#include "stm32l4xx_hal.h"
}

namespace Peripherals
{
    namespace HAL
    {
        template<std::size_t BufferSize = 64>
        class UartIT : public IUart
        {
        private:
            UART_HandleTypeDef& huart;
            std::size_t overflowCount{ 0 };
            UartRingBuffer<BufferSize> rxBuffer{};
            UartRingBuffer<BufferSize> txBuffer{};
            uint8_t rxByte{};
            uint8_t txByte{};
            volatile bool txBusy{ false };
            volatile bool rxBusy{ false };

            void StartTransmitIT();
            void StartReceiveIT();

        public:
            UartIT() = delete;
            UartIT(const UartIT&) = delete;
            UartIT(UartIT&&) = delete;
            UartIT& operator=(const UartIT&) = delete;
            UartIT& operator=(UartIT&&) = delete;
            ~UartIT() = default;
            explicit UartIT(UART_HandleTypeDef& huart_)
                : huart(huart_)
            {};

            void RxCpltCallback();
            void TxCpltCallback();
            [[nodiscard]] std::optional<uint8_t> Read() override;
            void Transmit(const uint8_t* data, size_t size) override;
            void ProcessTx();
            void ProcessRx();
        };
    }
}