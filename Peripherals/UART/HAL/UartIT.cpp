#if defined(HAL_UART_MODULE_ENABLED)

#include "UartIT.hpp"

namespace Peripherals
{
    namespace HAL
    {
        template<std::size_t BufferSize>
        void UartIT<BufferSize>::StartReceiveIT()
        {
            HAL_UART_Receive_IT(&huart, &rxByte, 1);
        }


        template<std::size_t BufferSize>
        void UartIT<BufferSize>::StartTransmitIT()
        {
            if (txBusy || txBuffer.IsEmpty())
                return;

            txBusy = true;
            
            if (const auto byte = txBuffer.Pop())
            {
                txByte = byte.value();
                HAL_UART_Transmit_IT(&huart, &txByte, 1);
            }
            else
            {
                txBusy = false;
            }
        }

        template<std::size_t BufferSize>
        void UartIT<BufferSize>::RxCpltCallback()
        {
            if (!rxBuffer.Push(rxByte))
                ++overflowCount;

            rxBusy = false;
        }

        template<std::size_t BufferSize>
        void UartIT<BufferSize>::TxCpltCallback()
        {
            txBusy = false;
        }

        template<std::size_t BufferSize>
        std::optional<uint8_t> UartIT<BufferSize>::Read()
        {
            return rxBuffer.Pop();
        }

        template<std::size_t BufferSize>
        void UartIT<BufferSize>::Transmit(const uint8_t* data, size_t size)
        {
            for (size_t i = 0; i < size; ++i)
            {
                if (!txBuffer.Push(data[i]))
                {
                    ++overflowCount; // or handle drop differently
                    break;
                }
            }
        }

        template<std::size_t BufferSize>
        void UartIT<BufferSize>::ProcessTx()
        {
            if (!txBusy && !txBuffer.IsEmpty())
                StartTransmitIT();
        }

        template<std::size_t BufferSize>
        void UartIT<BufferSize>::ProcessRx()
        {
            if (!rxBusy)
            {
                rxBusy = true;
                StartReceiveIT();
            }
        }
    }
}

template class Peripherals::HAL::UartIT<64>;

#endif