#pragma once

#include <gtest/gtest.h>
#include "Peripherals/UART/Utils/RingBuffer.hpp"

template<typename T>
class RingBuffer_Fixture : public ::testing::Test
{
protected:
    T buffer;
};

using RingBufferTypes = ::testing::Types<
    RingBuffer<uint8_t, 8>,
    RingBuffer<uint8_t, 16>,
    RingBuffer<uint8_t, 32>
>;

TYPED_TEST_SUITE(RingBuffer_Fixture, RingBufferTypes);

TYPED_TEST(RingBuffer_Fixture, InitiallyEmpty)
{
    ASSERT_TRUE(this->buffer.IsEmpty());
    ASSERT_FALSE(this->buffer.IsFull());
    ASSERT_EQ(this->buffer.Size(), 0);
}

TYPED_TEST(RingBuffer_Fixture, PushSingleElement)
{
    bool result = this->buffer.Push(42);
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(this->buffer.IsEmpty());
    ASSERT_EQ(this->buffer.Size(), 1);
}

TYPED_TEST(RingBuffer_Fixture, PushAndPopSingleElement)
{
    ASSERT_TRUE(this->buffer.Push(42));
    auto result = this->buffer.Pop();
    
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), 42);
    ASSERT_TRUE(this->buffer.IsEmpty());
    ASSERT_EQ(this->buffer.Size(), 0);
}

TYPED_TEST(RingBuffer_Fixture, PopFromEmptyBuffer)
{
    auto result = this->buffer.Pop();
    
    ASSERT_FALSE(result.has_value());
}

TYPED_TEST(RingBuffer_Fixture, PushMultipleElements)
{
    for (std::size_t i = 0; i < 5; i++)
    {
        bool result = this->buffer.Push(i);
        ASSERT_TRUE(result);
    }
    
    ASSERT_EQ(this->buffer.Size(), 5);
    ASSERT_FALSE(this->buffer.IsEmpty());
}

TYPED_TEST(RingBuffer_Fixture, PushAndPopMultipleElements)
{
    for (std::size_t i = 0; i < 5; i++)
        ASSERT_TRUE(this->buffer.Push(i));
    
    for (std::size_t i = 0; i < 5; i++)
    {
        auto result = this->buffer.Pop();
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result.value(), i);
    }
    
    ASSERT_TRUE(this->buffer.IsEmpty());
}

TYPED_TEST(RingBuffer_Fixture, FillBufferCompletely)
{
    // Fill buffer to capacity - 1 and verify it's full
    // We fill with values to track content
    for (std::size_t i = 0; i < 7; i++)  // Assuming buffer size >= 8
    {
        bool result = this->buffer.Push(i);
        ASSERT_TRUE(result);
    }
    
    ASSERT_TRUE(this->buffer.IsFull());
    ASSERT_EQ(this->buffer.Size(), 7);
}

TYPED_TEST(RingBuffer_Fixture, PushWhenFull)
{
    constexpr std::size_t capacity = 8;
    // Fill buffer to capacity - 1 (since we can't actually fill completely due to ring buffer design)
    for (std::size_t i = 0; i < capacity - 1; i++)
    {
        ASSERT_TRUE(this->buffer.Push(i));
    }
    
    ASSERT_TRUE(this->buffer.IsFull());
    
    bool result = this->buffer.Push(99);
    ASSERT_FALSE(result);
}

TYPED_TEST(RingBuffer_Fixture, WrapAround)
{
    constexpr std::size_t size = 8;
    
    // Fill buffer
    for (std::size_t i = 0; i < size - 1; i++)
        ASSERT_TRUE(this->buffer.Push(i));
    
    // Remove some elements
    for (std::size_t i = 0; i < 3; i++)
        this->buffer.Pop();
    
    ASSERT_EQ(this->buffer.Size(), size - 4);
    
    // Add new elements (this should wrap around)
    for (std::size_t i = 0; i < 3; i++)
    {
        bool result = this->buffer.Push(100 + i);
        ASSERT_TRUE(result);
    }
    
    ASSERT_EQ(this->buffer.Size(), size - 1);
}

TYPED_TEST(RingBuffer_Fixture, FifoOrder)
{
    const uint8_t values[] = {10, 20, 30, 40, 50};
    
    for (uint8_t val : values)
        ASSERT_TRUE(this->buffer.Push(val));
    
    for (uint8_t val : values)
    {
        auto result = this->buffer.Pop();
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result.value(), val);
    }
}

TYPED_TEST(RingBuffer_Fixture, InterleavedPushPop)
{
    ASSERT_TRUE(this->buffer.Push(1));
    ASSERT_TRUE(this->buffer.Push(2));
    
    auto r1 = this->buffer.Pop();
    ASSERT_EQ(r1.value(), 1);
    
    ASSERT_TRUE(this->buffer.Push(3));
    ASSERT_TRUE(this->buffer.Push(4));
    
    auto r2 = this->buffer.Pop();
    ASSERT_EQ(r2.value(), 2);
    
    auto r3 = this->buffer.Pop();
    ASSERT_EQ(r3.value(), 3);
    
    auto r4 = this->buffer.Pop();
    ASSERT_EQ(r4.value(), 4);
    
    ASSERT_TRUE(this->buffer.IsEmpty());
}

TYPED_TEST(RingBuffer_Fixture, SizeTracking)
{
    ASSERT_EQ(this->buffer.Size(), 0);
    
    ASSERT_TRUE(this->buffer.Push(1));
    ASSERT_EQ(this->buffer.Size(), 1);
    
    ASSERT_TRUE(this->buffer.Push(2));
    ASSERT_EQ(this->buffer.Size(), 2);
    
    this->buffer.Pop();
    ASSERT_EQ(this->buffer.Size(), 1);
    
    this->buffer.Pop();
    ASSERT_EQ(this->buffer.Size(), 0);
}

TYPED_TEST(RingBuffer_Fixture, AllZeros)
{
    for (std::size_t i = 0; i < 3; i++)
        ASSERT_TRUE(this->buffer.Push(0));
    
    for (int i = 0; i < 3; i++)
    {
        auto result = this->buffer.Pop();
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result.value(), 0);
    }
}

TYPED_TEST(RingBuffer_Fixture, AllMaxValues)
{
    constexpr uint8_t maxVal = 0xFF;
    
    for (std::size_t i = 0; i < 3; i++)
        ASSERT_TRUE(this->buffer.Push(maxVal));
    
    for (int i = 0; i < 3; i++)
    {
        auto result = this->buffer.Pop();
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(result.value(), maxVal);
    }
}

TYPED_TEST(RingBuffer_Fixture, CyclicalFilling)
{
    // Multiple cycles of filling and emptying
    for (std::size_t cycle = 0; cycle < 3; cycle++)
    {
        const uint8_t startVal = cycle * 10;
        
        for (std::size_t i = 0; i < 5; i++)
            ASSERT_TRUE(this->buffer.Push(startVal + i));
        
        ASSERT_EQ(this->buffer.Size(), 5);
        
        for (std::size_t i = 0; i < 5; i++)
        {
            auto result = this->buffer.Pop();
            ASSERT_TRUE(result.has_value());
            ASSERT_EQ(result.value(), startVal + i);
        }
        
        ASSERT_TRUE(this->buffer.IsEmpty());
    }
}

TYPED_TEST(RingBuffer_Fixture, PatternPushPop)
{
    // Pattern: push 2, pop 1, push 2, pop 1, etc.
    ASSERT_TRUE(this->buffer.Push(1));
    ASSERT_TRUE(this->buffer.Push(2));
    
    auto r1 = this->buffer.Pop();
    ASSERT_EQ(r1.value(), 1);
    
    ASSERT_TRUE(this->buffer.Push(3));
    ASSERT_TRUE(this->buffer.Push(4));
    
    auto r2 = this->buffer.Pop();
    ASSERT_EQ(r2.value(), 2);
    
    auto r3 = this->buffer.Pop();
    ASSERT_EQ(r3.value(), 3);
    
    auto r4 = this->buffer.Pop();
    ASSERT_EQ(r4.value(), 4);
    
    ASSERT_TRUE(this->buffer.IsEmpty());
}
