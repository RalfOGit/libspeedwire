#include <gtest/gtest.h>
#include <SpeedwireTime.hpp>

using namespace libspeedwire;

// timers must not return 0
TEST(SpeedwireTimeTest, TimerNotNull) {
    ASSERT_NE(SpeedwireTime::getEmeterTimeNow(), 0);
    ASSERT_NE(SpeedwireTime::getInverterTimeNow(), 0);
    ASSERT_NE(LocalHost::getUnixEpochTimeInMs(), 0);
}

// timer conversions back and forth must return the exact same values
TEST(SpeedwireTimeTest, TimerConversionExact) {
    uint64_t epoch = LocalHost::getUnixEpochTimeInMs();
    uint32_t emeter = SpeedwireTime::convertUnixEpochTimeToEmeterTimer(epoch);
    uint32_t inverter = SpeedwireTime::convertUnixEpochTimeToInverterTimer(epoch);
    uint64_t epoch2 = SpeedwireTime::convertEmeterTimeToUnixEpochTime(emeter, epoch);
    uint64_t epoch3 = SpeedwireTime::convertInverterTimeToUnixEpochTime(inverter, epoch);
    ASSERT_EQ(epoch, epoch2);
    ASSERT_EQ(epoch / (uint64_t)1000, epoch3 / (uint64_t)1000);
}

// timer conversions back and forth with a positive and negative shift of the reference epoch time
TEST(SpeedwireTimeTest, TimerConversionDelayedSuccess) {
    const uint64_t ms_per_day = 24 * 60 * 60 * 1000;
    uint64_t epoch = LocalHost::getUnixEpochTimeInMs();

    // for emeter time, positive and negative shift of the reference epoch time by up to +/-24 days must work
    uint32_t emeter = SpeedwireTime::convertUnixEpochTimeToEmeterTimer(epoch);
    for (uint64_t offset = 1; offset <= 24; ++offset) {
        uint64_t epoch_with_offset = epoch - offset * ms_per_day;
        uint64_t epoch2 = SpeedwireTime::convertEmeterTimeToUnixEpochTime(emeter, epoch_with_offset);
        ASSERT_EQ(epoch, epoch2);
    }
    for (uint64_t offset = 1; offset <= 24; ++offset) {
        uint64_t epoch_with_offset = epoch + offset * ms_per_day;
        uint64_t epoch2 = SpeedwireTime::convertEmeterTimeToUnixEpochTime(emeter, epoch_with_offset);
        ASSERT_EQ(epoch, epoch2);
    }

    // for inverter time, positive and negative shift of the reference epoch time by up to +/-24000 days must work
    uint32_t inverter = SpeedwireTime::convertUnixEpochTimeToInverterTimer(epoch);
    for (uint64_t offset = 1; offset <= 24000; ++offset) {
        uint64_t epoch_with_offset = epoch - offset * ms_per_day;
        if ((int64_t)epoch_with_offset < 0) break;
        uint64_t epoch2 = SpeedwireTime::convertInverterTimeToUnixEpochTime(inverter, epoch_with_offset);
        ASSERT_EQ(epoch / (uint64_t)1000, epoch2 / (uint64_t)1000);
    }
    for (uint64_t offset = 1; offset <= 24000; ++offset) {
        uint64_t epoch_with_offset = epoch + offset * ms_per_day;
        uint64_t epoch2 = SpeedwireTime::convertInverterTimeToUnixEpochTime(inverter, epoch_with_offset);
        ASSERT_EQ(epoch / (uint64_t)1000, epoch2 / (uint64_t)1000);
    }
}

// timer conversions back and forth with a positive and negative shift of the reference epoch time
TEST(SpeedwireTimeTest, TimerConversionDelayedFailure) {
    const uint64_t ms_per_day = 24 * 60 * 60 * 1000;
    uint64_t epoch = LocalHost::getUnixEpochTimeInMs();
    uint32_t emeter = SpeedwireTime::convertUnixEpochTimeToEmeterTimer(epoch);
    uint32_t inverter = SpeedwireTime::convertUnixEpochTimeToInverterTimer(epoch);

    // for emeter time, positive and negative shift of the reference epoch time by more than +/-24 days must fail
    uint64_t epoch_with_offset = epoch - 25 * ms_per_day;
    uint64_t epoch2 = SpeedwireTime::convertEmeterTimeToUnixEpochTime(emeter, epoch_with_offset);
    ASSERT_NE(epoch, epoch2);
    epoch_with_offset = epoch + 25 * ms_per_day;
    epoch2 = SpeedwireTime::convertEmeterTimeToUnixEpochTime(emeter, epoch_with_offset);
    ASSERT_NE(epoch, epoch2);

    // for inverter time, positive and negative shift of the reference epoch time by more than +/-24000 days must fail
    epoch_with_offset = epoch - 24001 * ms_per_day;
    epoch2 = SpeedwireTime::convertInverterTimeToUnixEpochTime(emeter, epoch_with_offset);
    ASSERT_NE(epoch, epoch2);
    epoch_with_offset = epoch + 24001 * ms_per_day;
    epoch2 = SpeedwireTime::convertInverterTimeToUnixEpochTime(emeter, epoch_with_offset);
    ASSERT_NE(epoch, epoch2);
}


