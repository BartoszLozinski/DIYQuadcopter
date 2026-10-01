#pragma once

#include <gtest/gtest.h>
#include <queue>
#include "Peripherals/UART/LineParser.hpp"

struct FakeUart
{
    std::queue<uint8_t> dataQueue;
    
    void AddData(const std::string& str)
    {
        for (uint8_t byte : str)
        {
            dataQueue.push(byte);
        }
    }
    
    void AddByte(uint8_t byte)
    {
        dataQueue.push(byte);
    }
    
    std::optional<uint8_t> Read()
    {
        if (dataQueue.empty())
            return std::nullopt;
        
        uint8_t byte = dataQueue.front();
        dataQueue.pop();
        return byte;
    }
    
    void Clear()
    {
        while (!dataQueue.empty())
            dataQueue.pop();
    }
};

class LineParser_Fixture : public ::testing::Test
{
protected:
    FakeUart uart;
    UcCommunication::LineParser<FakeUart> parser{ uart };
};

TEST_F(LineParser_Fixture, ReadLineWithNewline)
{
    uart.AddData("Hello\n");
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(line.size(), 5);
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(line.data()), line.size()), "Hello");
}

TEST_F(LineParser_Fixture, ReadLineWithCarriageReturn)
{
    uart.AddData("Test\r");
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(line.size(), 4);
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(line.data()), line.size()), "Test");
}

TEST_F(LineParser_Fixture, ReadLineWithCarriageReturnAndNewline)
{
    uart.AddData("Data\r\n");
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(line.size(), 4);
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(line.data()), line.size()), "Data");
}

TEST_F(LineParser_Fixture, ReadEmptyLineWithNewline)
{
    uart.AddData("\n");
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(line.size(), 0);
}

TEST_F(LineParser_Fixture, NoDataAvailable)
{
    auto result = parser.ReadLine();
    
    ASSERT_FALSE(result.has_value());
}

TEST_F(LineParser_Fixture, ReadMultipleLines)
{
    uart.AddData("First\nSecond\nThird\n");
    
    auto result1 = parser.ReadLine();
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result1.value().data()), result1.value().size()), "First");
    
    auto result2 = parser.ReadLine();
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result2.value().data()), result2.value().size()), "Second");
    
    auto result3 = parser.ReadLine();
    ASSERT_TRUE(result3.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result3.value().data()), result3.value().size()), "Third");
}

TEST_F(LineParser_Fixture, BufferOverflow)
{
    // LineParser has default buffersize of 64 bytes
    std::string longLine(70, 'A');
    longLine += '\n';
    uart.AddData(longLine);
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    // Should only contain up to buffer size (64 bytes)
    ASSERT_LE(line.size(), 64);
}

TEST_F(LineParser_Fixture, LineWithSpecialCharacters)
{
    uart.AddData("123!@#$%\n");
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(line.data()), line.size()), "123!@#$%");
}

TEST_F(LineParser_Fixture, LineWithNumbers)
{
    uart.AddData("1234567890\n");
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(line.data()), line.size()), "1234567890");
}

TEST_F(LineParser_Fixture, NewlineAfterCarriageReturn)
{
    uart.AddData("Line1\r");
    auto result1 = parser.ReadLine();
    ASSERT_TRUE(result1.has_value());
    
    uart.AddData("\n");
    auto result2 = parser.ReadLine();
    ASSERT_FALSE(result2.has_value()); // Should skip the standalone newline after CR
}

TEST_F(LineParser_Fixture, MultipleCarriageReturnsAndNewlines)
{
    uart.AddData("ABC\r\nus\ntest\r");
    
    auto result1 = parser.ReadLine();
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result1.value().data()), result1.value().size()), "ABC");
    
    auto result2 = parser.ReadLine();
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result2.value().data()), result2.value().size()), "us");
    
    auto result3 = parser.ReadLine();
    ASSERT_TRUE(result3.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result3.value().data()), result3.value().size()), "test");
}

TEST_F(LineParser_Fixture, ByteByByteInput)
{
    uart.AddByte('H');
    uart.AddByte('i');
    uart.AddByte('\n');
    
    auto result = parser.ReadLine();
    
    ASSERT_TRUE(result.has_value());
    auto line = result.value();
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(line.data()), line.size()), "Hi");
}

TEST_F(LineParser_Fixture, ConsecutiveNewlines)
{
    uart.AddData("Line\n\nNext\n");
    
    auto result1 = parser.ReadLine();
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result1.value().data()), result1.value().size()), "Line");
    
    auto result2 = parser.ReadLine();
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(result2.value().size(), 0); // Empty line
    
    auto result3 = parser.ReadLine();
    ASSERT_TRUE(result3.has_value());
    ASSERT_EQ(std::string(reinterpret_cast<const char*>(result3.value().data()), result3.value().size()), "Next");
}
