#pragma once

#include "IUart.hpp"
#include "Utils/RingBuffer.hpp"

extern "C"
{
#include "stm32l4xx_hal.h"
}

#if defined(HAL_UART_MODULE_ENABLED)

template<std::size_t BufferSize>
using UartRingBuffer = RingBuffer<uint8_t, BufferSize>;

namespace Peripherals
{
    namespace HAL
    {
        template<std::size_t BufferSize = 64>
        class Uart : public IUart
        {
            private:
            UART_HandleTypeDef& huart;
            std::size_t overflowCount{ 0 };
            UartRingBuffer<64> rxBuffer{};

        public:
            Uart() = delete;
            Uart(const Uart&) = delete;
            Uart(Uart&&) = delete;
            Uart& operator=(const Uart&) = delete;
            Uart& operator=(Uart&&) = delete;
            ~Uart() = default;
            explicit Uart(UART_HandleTypeDef& huart_)
                : huart(huart_)
            {};

            void Poll()
            {
                uint8_t byte;

                while (HAL_UART_Receive(&huart, &byte, 1, 0) == HAL_OK)
                {
                    if (!this->rxBuffer.Push(byte))
                        ++overflowCount;
                }
            }

            [[nodiscard]] std::optional<uint8_t> Read() override
            {
                return rxBuffer.Pop();
            }

            void Transmit(const uint8_t* data, size_t size) override
            {
                HAL_UART_Transmit(&huart, const_cast<uint8_t*>(data), size, HAL_MAX_DELAY);
            }

        };
    }
}

#endif

/*

USAGE EXAMPLE


#include "../../Peripherals/UART/HAL/Uart.hpp"
UART_HandleTypeDef huart2;
Peripherals::HAL::Uart uart2{ huart2 };

//All necessary HAL generated config

int main()
{
    while(true)
    {    
        if (uart2PollTimer.IsExpired())
        {
            uart2PollTimer.Reset();
            uart2.Poll();
        }
        if (auto lineOpt = lineParser.ReadLine())
        {
            const auto line = *lineOpt;
            const char* prefix = "Received line: ";
            uart2.Transmit(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
            uart2.Transmit(reinterpret_cast<const uint8_t*>(line.data()), line.size());
            uart2.Transmit(reinterpret_cast<const uint8_t*>("\r\n"), 2);
            if (std::string_view{ reinterpret_cast<const char*>(line.data()), line.size() } == "test")
            {
                const char* msg = "MATCH!\r\n";
                uart2.Transmit(reinterpret_cast<const uint8_t*>(msg), strlen(msg));
            }
        }
        if (uartResetTimer.IsExpired())
        {
            uartResetTimer.Reset();
            std::string_view resetMsg = "UART Reset\r\n";
            uart2.Transmit(reinterpret_cast<const uint8_t*>(resetMsg.data()), resetMsg.size());
        }
    }
}
*/
