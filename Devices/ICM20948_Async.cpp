#include "ICM20948_Async.hpp"

namespace Device
{
    ICM20948_Async::ICM20948_Async(Peripherals::I2CBase_IT& i2c_)
        : i2c(i2c_) {}

    void ICM20948_Async::OnRxComplete()
    {
        i2c.OnRxComplete();
        // update states which are required
    }

    void ICM20948_Async::OnTxComplete()
    {
        i2c.OnTxComplete();
        if (state == State::WakeUpScheduled)
            state = State::WakeUpCompleted;
    }

    std::optional<uint8_t> ICM20948_Async::ReadRegister(const uint8_t reg)
    {
        if (state == State::Idle)
        {
            state = i2c.Read(static_cast<uint16_t>(RegisterAddresses::ADDR)
                            , reg
                            , std::span<uint8_t>(&readRegisterBuffer, sizeof(readRegisterBuffer))) == Peripherals::I2CResult::Success ? State::TransferScheduled : State::Error;
        }
        else if (state == State::RegisterReadyToRead)
        {
            i2c.NotifyDataIsRead();
            return readRegisterBuffer;
        }

        return std::nullopt;
    }

    void ICM20948_Async::StartRead(const uint16_t reg, std::span<uint8_t>(buffer), const State successfulState)
    {
        state = i2c.Read(static_cast<uint16_t>(RegisterAddresses::ADDR)
                        , reg
                        , buffer) == Peripherals::I2CResult::Success ? successfulState : State::Error;
    }

    void ICM20948_Async::WakeUp()
    {
        if (isAwake)
            return;

        switch (state)
        {
        case State::Idle:
            StartRead( static_cast<uint16_t>(RegisterAddresses::PWR_MGMT_1)
                     , std::span<uint8_t>(&readRegisterBuffer, sizeof(readRegisterBuffer))
                     , State::TransferScheduled);
            break;
        case State::RegisterReadyToRead:
            currentPwrMgmt1Reg = readRegisterBuffer;
            i2c.NotifyDataIsRead();
            state = State::WakeUpScheduled;
            static constexpr uint8_t SLEEP_MODE_BIT_MASK = 0x1 << 6; // 1 - sleep, 0 - wake up
            currentPwrMgmt1Reg &= ~SLEEP_MODE_BIT_MASK;
            i2c.Write(static_cast<uint16_t>(RegisterAddresses::ADDR)
                    , static_cast<uint16_t>(RegisterAddresses::PWR_MGMT_1)
                    , std::span<uint8_t>(&currentPwrMgmt1Reg, sizeof(currentPwrMgmt1Reg)));
            break;
        case State::WakeUpCompleted:
            // check if something needs to be adjusted
            i2c.NotifyDataIsRead();
            isAwake = true;
            state = State::Idle;
            break;
        default:
            break;
        };
    }
    
};