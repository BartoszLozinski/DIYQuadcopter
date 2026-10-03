#pragma once

#include "Peripherals/I2C/I2CBase.hpp"
#include "LPS25HBBase.hpp"
#include <utility>
#include <functional>

namespace Device
{
    class LPS25HB_Async : public LPS25HBBase
    {
    private:
        enum class State
        {
            Idle,
            TransferScheduled,
            WakeUpScheduled,
            SetupMeasurementFrequencyScheduled,
            MeasurementFrequencyCtrlReg1Reading,
            MeasurementFrequencyCtrlReg1ReadyToRead,
            RxTempReading,
            RxPressureReading,
            TempReadyToRead,
            PressureReadyToRead,
            RegisterReadyToRead,
            WakeUpCompleted,
            SetupMeasurementFrequencyCompleted,
            Error,
        };
        
        Peripherals::I2CBase_IT& i2c;
        void WriteRegister(const uint8_t reg, uint8_t value);
        [[nodiscard]] std::optional<uint8_t> ReadRegister(uint8_t reg);
        void StartRead(const int16_t reg, std::span<uint8_t>(buffer), const State successfulState);
        [[nodiscard]] uint32_t ParsePressureBuffer() const;
        [[nodiscard]] int16_t ParseTemperatureBuffer() const;
        [[nodiscard]] uint8_t ParseWakeUpCurrentCtrlReg1() const;
        State state = State::Idle;
        uint8_t readRegisterBuffer{}; 
        std::array<uint8_t, 2> rawTempBuffer{};
        std::array<uint8_t, 3> rawPressureBuffer{};
        uint8_t measurementFrequencyCtrlReg1 = 0;
        uint8_t wakeUpCurrentCtrlReg1 = 0;
        bool isAwake = false;

        template<typename F>
        [[nodiscard]] decltype(auto)CompleteAsyncRead(F&& ParseFunction, const State nextState)
        {
            i2c.NotifyDataIsRead();
            state = nextState;
            return std::invoke(std::forward<F>(ParseFunction), *this);
        }

    public:

        LPS25HB_Async(Peripherals::I2CBase_IT& i2c_);
        [[nodiscard]] std::optional<uint8_t> ReadWhoAmI() override;
        [[nodiscard]] std::optional<float> ReadTemperature() override;
        [[nodiscard]] std::optional<int32_t> ReadPressure() override;
        void ReadCurrentMeasurementFrequency();
        void SetMeasurementFrequency(const MeasurementFrequency freq);
        [[nodiscard]] bool IsAwake() const { return isAwake; }
        void WakeUp() override;

        void OnRxComplete();
        void OnTxComplete();
    };
}
