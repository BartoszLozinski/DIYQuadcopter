#pragma once
//TO connect with you linux terminal (for example at ubuntu)
//
// check tty com ports with
// dmesg | grep tty 		(or sudo if not allow due to reading the kernel buffer failed: Operation not permitted)
// screen /dev/tty*ACMx* [baud rate] (i.e. screen /dev/ttyACM1 115200
//
// to kill screen process: ctrl + A --> K to kill process
// to detach session:      ctrl + A --> D
// available sockets:      screen -ls
// reconnect:              screen -r [ID]
//
// if you cannot reconnect, try: lsof /dev/ttyACM1
// kill [PID]
//
// try once more

#include <optional>

#include "IUart.hpp"
#include "UartBase.hpp"
#include "Utils/RingBuffer.hpp"

namespace Peripherals
{
    namespace RegisterLevel
    {
        // TODO: move to cpp and export quantizied buffer sizes?

        template< std::size_t bufferSize = 64>
        class Uart : public IUart
            , public UartBase<USART_TypeDef, bufferSize>
        {
        protected:
            UartRingBuffer<bufferSize> rxBuffer{};
            UartRingBuffer<bufferSize> txBuffer{};

            uint32_t overflowCount = 0;
            volatile bool txBusy{ false };

            void EnableRxInterrupt()
            {
                this->usart->CR1 |= USART_CR1_RXNEIE; //Enable RX interrupt
            };

            void EnableTxInterrupt()
            {
                this->usart->CR1 |= USART_CR1_TXEIE; // Enable TX interrupt
            }

        public:
            Uart(const USART_TypeDef& source) = delete;
            Uart(USART_TypeDef&& source) = delete;
            Uart& operator=(const USART_TypeDef& source) = delete;
            Uart& operator=(USART_TypeDef&& source) = delete;
            Uart(USART_TypeDef* const usart_) : UartBase<USART_TypeDef, bufferSize>(usart_) {};

            /*Get appropriate Tx and Rx pins from datasheet*/
            template<typename TxPin, typename RxPin>
            void Init(TxPin& txPin, RxPin& rxPin, const uint32_t baudRate = 115200)
            {
                this->UartBase<USART_TypeDef, bufferSize>::Init(txPin, rxPin, baudRate);
                EnableRxInterrupt();
            }

            std::optional<uint8_t> Read() override
            {
                return rxBuffer.Pop();
            }

            void Transmit(const uint8_t* data, std::size_t size) override
            {
                for (std::size_t i = 0; i < size; ++i)
                {
                    if (!txBuffer.Push(data[i]))
                    {
                        ++overflowCount; // or handle drop differently
                        break;
                    }
                }

                ProcessTx();
            }

            void ProcessTx()
            {
                if (txBusy)
                    return;

                if (const auto byte = txBuffer.Pop())
                {
                    txBusy = true;
                    this->usart->TDR = byte.value();
                    EnableTxInterrupt();
                }
            }

            void HandleTxInterrupt()
            {
                // ISR_TXEIE - transmit register empty
                // TCIE - transmission comlpete
                if ((this->usart->ISR & USART_ISR_TXE) && (this->usart->CR1 & USART_CR1_TXEIE))
                {
                    const auto byte = txBuffer.Pop();

                    if (byte)
                    {
                        this->usart->TDR = byte.value();
                    }
                    else
                    {
                        this->usart->CR1 &= ~USART_CR1_TXEIE;
                        txBusy = false;
                    }
                }
            }

            void HandleRxInterrupt()
            {
                if (this->usart->ISR & USART_ISR_RXNE)
                {
                    const auto byte = static_cast<uint8_t>(this->usart->RDR);
                
                    if (!rxBuffer.Push(byte))
                        ++overflowCount;
                }
            }

            void IRQHandler()
            {
                HandleRxInterrupt();        
                HandleTxInterrupt();
            }
        };
    }
}
