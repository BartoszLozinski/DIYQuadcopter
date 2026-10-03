#include "LPS25HB_Async.hpp"

namespace Device
{
    LPS25HB_Async::LPS25HB_Async(Peripherals::I2CBase_IT& i2c_)
        : i2c(i2c_) {}

    void LPS25HB_Async::OnRxComplete()
    {
        i2c.OnRxComplete();

        using enum State;
        switch(state)
        {
        case RxPressureReading:
            state = PressureReadyToRead;
            break;
        case RxTempReading:
            state = TempReadyToRead;
            break;
        case MeasurementFrequencyCtrlReg1Reading:
            state = MeasurementFrequencyCtrlReg1ReadyToRead;
            break;
        default:
            state = RegisterReadyToRead;
        }
    }

    void LPS25HB_Async::OnTxComplete()
    {
        i2c.OnTxComplete();
        if (state == State::WakeUpScheduled)
            state = State::WakeUpCompleted;
        else if (state == State::SetupMeasurementFrequencyScheduled)
            state = State::SetupMeasurementFrequencyCompleted;
    }

    std::optional<uint8_t> LPS25HB_Async::ReadRegister(const uint8_t reg)
    {
        if (state == State::Idle)
        {
            state = i2c.Read(Registers::ADDR << addressOffset
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

    std::optional<uint8_t> LPS25HB_Async::ReadWhoAmI()
    {
        const auto result = ReadRegister(Registers::WHO_AM_I);
        if (result && state == State::RegisterReadyToRead)
        {
            state = State::Idle;
        }
        
        return result;
    }

    [[nodiscard]] uint32_t LPS25HB_Async::ParsePressureBuffer() const
    {
        return (static_cast<uint32_t>(rawPressureBuffer[2]) << 16) | 
                static_cast<uint32_t>(rawPressureBuffer[1] << 8) |
                static_cast<uint32_t>(rawPressureBuffer[0]);
    }

    [[nodiscard]] int16_t LPS25HB_Async::ParseTemperatureBuffer() const
    {
        return (static_cast<int16_t>(rawTempBuffer[1]) << 8) | static_cast<int16_t>(rawTempBuffer[0]);
    }

    std::optional<float> LPS25HB_Async::ReadTemperature()
    {
        if (state == State::Idle)
        {
            StartRead(Registers::TEMP_OUT_L, rawTempBuffer, State::RxTempReading);
        }
        else if (state == State::TempReadyToRead)
        {
            const int16_t tempBuffer = CompleteAsyncRead(&LPS25HB_Async::ParseTemperatureBuffer, State::Idle);
            return RecalculateRawTemperature(tempBuffer);
        }

        return std::nullopt;
    }
    
    std::optional<int32_t> LPS25HB_Async::ReadPressure()
    {
        if (state == State::Idle)
        {
            StartRead(Registers::PRESS_OUT_XL, rawPressureBuffer, State::RxPressureReading);
        }
        else if (state == State::PressureReadyToRead)
        {
            const uint32_t pressureBuffer = CompleteAsyncRead(&LPS25HB_Async::ParsePressureBuffer, State::Idle);
            return RecalculateRawPressure(pressureBuffer);
        }

        return std::nullopt;
    }

    void LPS25HB_Async::ReadCurrentMeasurementFrequency()
    {
        StartRead(Registers::CTRL_REG1, std::span<uint8_t>(&measurementFrequencyCtrlReg1, sizeof(measurementFrequencyCtrlReg1)), State::MeasurementFrequencyCtrlReg1Reading);
    }
    
    void LPS25HB_Async::SetMeasurementFrequency(const MeasurementFrequency freq)
    {
        static constexpr uint8_t ODR_MASK = Registers::CTRL_REG1_ODR2 | Registers::CTRL_REG1_ODR1 | Registers::CTRL_REG1_ODR0;

        //Sync version copied
        uint8_t regValue = 0;
        switch (freq)
        {
        case MeasurementFrequency::OneShot:
            regValue = 0x00;
            break;
        case MeasurementFrequency::Hz1:
            regValue = Registers::CTRL_REG1_ODR0;
            break;
        case MeasurementFrequency::Hz7:
            regValue = Registers::CTRL_REG1_ODR1;
            break;
        case MeasurementFrequency::Hz12_5:
            regValue = Registers::CTRL_REG1_ODR1 | Registers::CTRL_REG1_ODR0;
            break;
        case MeasurementFrequency::Hz25:
            regValue = Registers::CTRL_REG1_ODR2;
            break;
        }

        measurementFrequencyCtrlReg1 = (measurementFrequencyCtrlReg1 & ~ODR_MASK) | (regValue & ODR_MASK);
        i2c.Write(Registers::ADDR << addressOffset, Registers::CTRL_REG1, std::span<uint8_t>(&measurementFrequencyCtrlReg1, sizeof(measurementFrequencyCtrlReg1)));
        state = State::SetupMeasurementFrequencyScheduled;        
    }

    void LPS25HB_Async::StartRead(const int16_t reg, std::span<uint8_t>(buffer), const State successfulState)
    {
        state = i2c.Read(Registers::ADDR << addressOffset
                        , reg | AUTO_INCREMENT
                        , buffer) == Peripherals::I2CResult::Success ? successfulState : State::Error;
    }

    void LPS25HB_Async::WakeUp()
    {
        if (isAwake)
            return;

        switch (state)
        {
        case State::Idle:
            wakeUpCurrentCtrlReg1 = 0;
            StartRead(Registers::CTRL_REG1, std::span<uint8_t>(&readRegisterBuffer, sizeof(readRegisterBuffer)), State::TransferScheduled);
            break;
        case State::RegisterReadyToRead:
            wakeUpCurrentCtrlReg1 = readRegisterBuffer;
            i2c.NotifyDataIsRead();
            state = State::WakeUpScheduled;
            wakeUpCurrentCtrlReg1 = static_cast<uint8_t>(wakeUpCurrentCtrlReg1 | Registers::CTRL_REG1_PD);
            i2c.Write(Registers::ADDR << addressOffset, Registers::CTRL_REG1, std::span<uint8_t>(&wakeUpCurrentCtrlReg1, sizeof(wakeUpCurrentCtrlReg1)));
            break;
        case State::WakeUpCompleted:
            i2c.NotifyDataIsRead();
            ReadCurrentMeasurementFrequency();
            break;
        case State::MeasurementFrequencyCtrlReg1ReadyToRead:
            i2c.NotifyDataIsRead();
            SetMeasurementFrequency(MeasurementFrequency::Hz25);
            break;
        case State::SetupMeasurementFrequencyCompleted:
            i2c.NotifyDataIsRead();
            isAwake = true;
            state = State::Idle;
            break;
        default:
            break;
        }
    }
}
