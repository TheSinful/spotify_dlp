#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include "../src/utils/logger.h"
#include <stdexcept>

// custom sink to capture log messages
template <typename Mutex>
class test_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    std::vector<std::string> messages;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        messages.push_back(std::string(msg.payload.data(), msg.payload.size()));
    }

    void flush_() override {}
};

using test_sink_mt = test_sink<std::mutex>;

class LogTest : public ::testing::Test
{
protected:
    std::shared_ptr<test_sink_mt> test_sink_;
    std::shared_ptr<spdlog::logger> original_logger_;

    void SetUp() override
    {
        LoggerUtils::setVerbose(false);
        original_logger_ = spdlog::default_logger();
        test_sink_ = std::make_shared<test_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("test_logger", test_sink_);
        spdlog::set_default_logger(logger);
    }

    void TearDown() override
    {
        spdlog::set_default_logger(original_logger_);
    }

    std::string getLastLogMessage()
    {
        if (test_sink_->messages.empty())
        {
            return "";
        }
        return test_sink_->messages.back();
    }
};

TEST_F(LogTest, TestThrowLog)
{
    EXPECT_THROW(
        THROW_AND_LOG(std::runtime_error,
                       "Error occurred",
                       "Error occurred: detailed info - operation failed at step 3"),
        std::runtime_error);
}

TEST_F(LogTest, TestNormalLogging)
{
    LOG_INFO("Normal message", "Verbose message with details");
    EXPECT_EQ(getLastLogMessage(), "Normal message");
}

TEST_F(LogTest, TestVerboseLogging)
{
    LoggerUtils::setVerbose(true);
    LOG_INFO("Normal message", "Verbose message with details");
    EXPECT_EQ(getLastLogMessage(), "Verbose message with details");
}

TEST_F(LogTest, DebugLogging_WhenDebugDisabled_LogsNothing)
{
    LoggerUtils::setDebug(false);
    LoggerUtils::setVerbose(false);

    LOG_DEBUG("Debug message", "Verbose debug message");
    EXPECT_EQ(getLastLogMessage(), "");
}

TEST_F(LogTest, DebugLogging_WhenDebugEnabled_LogsBasicMessage)
{
    LoggerUtils::setDebug(true);
    LoggerUtils::setVerbose(false);

    LOG_DEBUG("Debug message", "Verbose debug message");
    EXPECT_EQ(getLastLogMessage(), "Debug message");
}

TEST_F(LogTest, DebugLogging_WhenDebugAndVerboseEnabled_LogsVerboseMessage)
{
    LoggerUtils::setDebug(true);
    LoggerUtils::setVerbose(true);

    LOG_DEBUG("Debug message", "Verbose debug message");
    EXPECT_EQ(getLastLogMessage(), "Verbose debug message");
}

TEST_F(LogTest, DebugLogging_WhenDebugDisabledButVerboseEnabled_LogsNothing)
{
    LoggerUtils::setDebug(false);
    LoggerUtils::setVerbose(true);

    LOG_DEBUG("Debug message", "Verbose debug message");
    EXPECT_EQ(getLastLogMessage(), "");
}

TEST_F(LogTest, DebugLogging_StateTransition_PreservesLastMessage)
{
    // Setup initial state
    LoggerUtils::setDebug(true);
    LoggerUtils::setVerbose(true);
    LOG_DEBUG("Debug message", "Verbose debug message");
    std::string lastMessage = getLastLogMessage();

    // Transition state
    LoggerUtils::setDebug(false);
    LOG_DEBUG("New debug message", "New verbose message");

    // Should preserve previous message
    EXPECT_EQ(getLastLogMessage(), lastMessage);
}