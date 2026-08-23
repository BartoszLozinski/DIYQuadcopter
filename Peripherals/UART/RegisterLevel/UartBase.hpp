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

#include "../../Config.hpp"
#include <stm32l476xx.h>
#include <array>
#include <concepts>

#include "Peripherals/GPIO/RegisterLevel/GpioAlternate.hpp"

//concept basing on USART_TypeDef struct from CMSIS
template<typename T>
concept USARTx = requires (T uart)
{
	{ uart.CR1 } 		-> std::convertible_to<volatile uint32_t&>;		/*!< USART Control register 1,                 Address offset: 0x00 */
    { uart.CR2 } 		-> std::convertible_to<volatile uint32_t&>;		/*!< USART Control register 2,                 Address offset: 0x04 */
    { uart.CR3 } 		-> std::convertible_to<volatile uint32_t&>;		/*!< USART Control register 3,                 Address offset: 0x08 */
    { uart.BRR } 		-> std::convertible_to<volatile uint32_t&>;		/*!< USART Baud rate register,                 Address offset: 0x0C */
    { uart.GTPR}  		-> std::convertible_to<volatile uint32_t&>;		/*!< USART Guard time and prescaler register,  Address offset: 0x10 */
    { uart.RTOR } 		-> std::convertible_to<volatile uint32_t&>;     /*!< USART Receiver Time Out register,         Address offset: 0x14 */
	{ uart.RQR} 		-> std::convertible_to<volatile uint32_t&>;     /*!< USART Request register,                   Address offset: 0x18 */
	{ uart.ISR} 		-> std::convertible_to<volatile uint32_t&>;     /*!< USART Interrupt and status register,      Address offset: 0x1C */
	{ uart.ICR } 		-> std::convertible_to<volatile uint32_t&>;     /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
	{ uart.RDR }        -> std::convertible_to<volatile uint32_t&>;		/*!< USART Receive Data register,              Address offset: 0x24 */
	{ uart.TDR }        -> std::convertible_to<volatile uint32_t&>;		/*!< USART Transmit Data register,             Address offset: 0x28 */
};

namespace Peripherals
{
    namespace RegisterLevel
    {
        /*Rx and Tx pins needs to be configured/initialized before Uart*/
        template<USARTx Usart, std::size_t bufferSize = 64>
        class UartBase
        {
        protected:
            volatile Usart* const usart = nullptr;

            void EnableClock()
            {
                if (usart == USART1)
                    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
                else if (usart == USART2)
                    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
            }

            // TODO someday - add enum to select clock source or deduce it from other registers
            void ConfigureClockSource()
            {
                if (usart == USART1)
                {
                    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_USART1SEL) | RCC_USART1CLKSOURCE_PCLK2;
                }
                else if (usart == USART2)
                {
                    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_USART2SEL) | RCC_USART2CLKSOURCE_PCLK1;
                }
            }

            void UartConfig(const uint32_t baudRate)
            {
                // On STM32L476, USART1 uses PCLK2 and USART2 uses PCLK1.
                // For the default APB prescaler settings this is the same as SystemCoreClock,
                // but the explicit clock selection keeps the peripheral configuration correct.

                //USART Baud Rate Register BRR - speed of the USART
                //UARTDIV (RM 40.8 USART baud rate register) = 80 000 000 / 115200 = 34,7

                ConfigureClockSource();

                usart->BRR = SystemCoreClock / baudRate; // system_stm32l4xx.c file

                //frame 8m1 -- 0b00 - for USART_CR1 (reset value)
                //PCE parity control enable: 0 - disabled (reset value), 1 - enabled
                //USART_CR2 STOP register - 00 - 1 stop bit; (RM USART_CR2) reset value
                //USART_CR1 UE (UART enable bit) - 0 -disabled (reset value): 1 - enabled
                usart->CR1 |= USART_CR1_UE; // UART enabled
                usart->CR1 |= USART_CR1_TE; //transmitter enabled - 0 disable, 1- enabled
                usart->CR1 |= USART_CR1_RE; //receiver enabled - 0 disable, 1- enabled
            }

            volatile bool messageReady_ = false;
            std::array<char, bufferSize>::iterator index_;


        public:
            UartBase(const UartBase& source) = delete;
            UartBase(UartBase&& source) = delete;
            UartBase& operator=(const UartBase& source) = delete;
            UartBase& operator=(UartBase&& source) = delete;
            UartBase(Usart* const usart_) : usart(usart_) {};

            /*Get appropriate Tx and Rx pins from datasheet*/
            template<typename TxPin, typename RxPin>
            void Init(TxPin& txPin, RxPin& rxPin, const uint32_t baudRate = 115200)
            {
                txPin.Init();
                rxPin.Init();
                EnableClock();
                UartConfig(baudRate);
            }

            void ConfigureInterruptsPriority(const IRQn_Type selectedExti, const uint8_t priority)
            {
                NVIC_SetPriority(selectedExti, priority); //set priority (for exti ,  priotity = 1
                NVIC_EnableIRQ(selectedExti);//enable interrupt
                //enum from stm32l476xx.h (CMSIS file) - Interrupt number definition
            };
        };
    }
}
