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

// time differences tests 32-bits
TEST(SpeedwireTimeTest, TimerDifferencesUint32) {
    const uint64_t ms_per_day = 24 * 60 * 60 * 1000;
    uint32_t emeter = SpeedwireTime::getEmeterTimeNow();

    int32_t diff = SpeedwireTime::calculateTimeDifference(emeter, emeter - 1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateTimeDifference(emeter, emeter + 1);
    ASSERT_EQ(diff, -1);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint32_t)-1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateTimeDifference((uint32_t)-1, 0);
    ASSERT_EQ(diff, -1);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint32_t)10);
    ASSERT_EQ(diff, -10);
    diff = SpeedwireTime::calculateTimeDifference((uint32_t)10, 0);
    ASSERT_EQ(diff, 10);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint32_t)-10);
    ASSERT_EQ(diff, 10);
    diff = SpeedwireTime::calculateTimeDifference((uint32_t)-10, 0);
    ASSERT_EQ(diff, -10);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint32_t)0x80000000);
    ASSERT_EQ(diff, 0x80000000);
    diff = SpeedwireTime::calculateTimeDifference((uint32_t)0x80000000, 0);
    ASSERT_EQ(diff, 0x80000000); // overflow condition

    diff = SpeedwireTime::calculateTimeDifference(0, (uint32_t)0x7FFFFFFF);
    ASSERT_EQ(diff, 0x80000001); // overflow condition
    diff = SpeedwireTime::calculateTimeDifference((uint32_t)0x7FFFFFFF, 0);
    ASSERT_EQ(diff, 0x7FFFFFFF);
}

// time differences tests 64-bits
TEST(SpeedwireTimeTest, TimerDifferencesUint64) {
    uint64_t epoch = LocalHost::getUnixEpochTimeInMs();

    int64_t diff = SpeedwireTime::calculateTimeDifference(epoch, epoch - 1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateTimeDifference(epoch, epoch + 1);
    ASSERT_EQ(diff, -1);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint64_t)-1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateTimeDifference((uint64_t)-1, 0);
    ASSERT_EQ(diff, -1);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint64_t)10);
    ASSERT_EQ(diff, -10);
    diff = SpeedwireTime::calculateTimeDifference((uint64_t)10, 0);
    ASSERT_EQ(diff, 10);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint64_t)-10);
    ASSERT_EQ(diff, 10);
    diff = SpeedwireTime::calculateTimeDifference((uint64_t)-10, 0);
    ASSERT_EQ(diff, -10);

    diff = SpeedwireTime::calculateTimeDifference(0, (uint64_t)0x8000000000000000);
    ASSERT_EQ(diff, 0x8000000000000000);
    diff = SpeedwireTime::calculateTimeDifference((uint64_t)0x8000000000000000, 0);
    ASSERT_EQ(diff, 0x8000000000000000); // overflow condition

    diff = SpeedwireTime::calculateTimeDifference(0, (uint64_t)0x7FFFFFFFFFFFFFFF);
    ASSERT_EQ(diff, 0x8000000000000001); // overflow condition
    diff = SpeedwireTime::calculateTimeDifference((uint64_t)0x7FFFFFFFFFFFFFFF, 0);
    ASSERT_EQ(diff, 0x7FFFFFFFFFFFFFFF);
}

// absolute time differences tests 32-bits
TEST(SpeedwireTimeTest, AbsTimerDifferencesUint32) {
    const uint64_t ms_per_day = 24 * 60 * 60 * 1000;
    uint32_t emeter = SpeedwireTime::getEmeterTimeNow();

    int32_t diff = SpeedwireTime::calculateAbsTimeDifference(emeter, emeter - 1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateAbsTimeDifference(emeter, emeter + 1);
    ASSERT_EQ(diff, 1);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint32_t)-1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint32_t)-1, 0);
    ASSERT_EQ(diff, 1);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint32_t)10);
    ASSERT_EQ(diff, 10);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint32_t)10, 0);
    ASSERT_EQ(diff, 10);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint32_t)-10);
    ASSERT_EQ(diff, 10);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint32_t)-10, 0);
    ASSERT_EQ(diff, 10);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint32_t)0x80000000);
    ASSERT_EQ(diff, 0x80000000); // overflow condition
    diff = SpeedwireTime::calculateAbsTimeDifference((uint32_t)0x80000000, 0);
    ASSERT_EQ(diff, 0x80000000); // overflow condition

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint32_t)0x7FFFFFFF);
    ASSERT_EQ(diff, 0x7FFFFFFF);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint32_t)0x7FFFFFFF, 0);
    ASSERT_EQ(diff, 0x7FFFFFFF);
}

// absolute time differences tests 64-bits
TEST(SpeedwireTimeTest, AbsTimerDifferencesUint64) {
    uint64_t epoch = LocalHost::getUnixEpochTimeInMs();

    int64_t diff = SpeedwireTime::calculateAbsTimeDifference(epoch, epoch - 1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateAbsTimeDifference(epoch, epoch + 1);
    ASSERT_EQ(diff, 1);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint64_t)-1);
    ASSERT_EQ(diff, 1);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint64_t)-1, 0);
    ASSERT_EQ(diff, 1);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint64_t)10);
    ASSERT_EQ(diff, 10);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint64_t)10, 0);
    ASSERT_EQ(diff, 10);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint64_t)-10);
    ASSERT_EQ(diff, 10);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint64_t)-10, 0);
    ASSERT_EQ(diff, 10);

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint64_t)0x8000000000000000);
    ASSERT_EQ(diff, 0x8000000000000000); // overflow condition
    diff = SpeedwireTime::calculateAbsTimeDifference((uint64_t)0x8000000000000000, 0);
    ASSERT_EQ(diff, 0x8000000000000000); // overflow condition

    diff = SpeedwireTime::calculateAbsTimeDifference(0, (uint64_t)0x7FFFFFFFFFFFFFFF);
    ASSERT_EQ(diff, 0x7FFFFFFFFFFFFFFF);
    diff = SpeedwireTime::calculateAbsTimeDifference((uint64_t)0x7FFFFFFFFFFFFFFF, 0);
    ASSERT_EQ(diff, 0x7FFFFFFFFFFFFFFF);
}
