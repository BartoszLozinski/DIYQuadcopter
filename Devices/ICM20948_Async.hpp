#pragma once

#include "ICM20948Base.hpp"
#include "Peripherals/I2C/I2CBase.hpp"

namespace Device
{
    class ICM20948_Async : public ICM20948Base
    {
    private:
        enum class State
        {
            Idle,
            TransferScheduled,
            WakeUpScheduled,
            RegisterReadyToRead,
            WakeUpCompleted,
            Error,
        };

        Peripherals::I2CBase_IT& i2c;
        State state = State::Idle;
        bool isAwake = false;

        uint8_t currentPwrMgmt1Reg = 0x41; //default value after reset, device is in sleep mode (bit 7 = 1), auto selects best available clock source (bit 2:0 = 1-5)
        
        uint8_t readRegisterBuffer{};

        void StartRead(const uint16_t reg, std::span<uint8_t>(buffer), const State succesfulState);
        [[nodiscard]] std::optional<uint8_t> ReadRegister(uint8_t reg);

    public:
        ICM20948_Async(Peripherals::I2CBase_IT& i2c_);
        void WakeUp() override;
        void OnTxComplete();
        void OnRxComplete();
    };
};