#ifndef __LIBSPEEDWIRE_SPEEDWIRETIME_H__
#define __LIBSPEEDWIRE_SPEEDWIRETIME_H__

#include <LocalHost.hpp>

namespace libspeedwire {

    /**
     *  Class implementing speedwire timer related accessors and conversions.
     */
    class SpeedwireTime {
    public:

        /**
         * Get the current time in a representation used by SMA emeters.
         * I.e. the 32 least significant bits of the unix epoch time in milliseconds.
         * @return the emeter time
         */
        static uint32_t getEmeterTimeNow(void) {
            return convertUnixEpochTimeToEmeterTimer(LocalHost::getUnixEpochTimeInMs());
        }

        /**
         * Get the current time in a representation used by SMA inverters.
         * I.e. the 32 least significant bits of the unix epoch time in seconds.
         * @return the inverter time
         */
        static uint32_t getInverterTimeNow(void) {
            return convertUnixEpochTimeToInverterTimer(LocalHost::getUnixEpochTimeInMs());
        }

        /**
         * Convert the given epoch time in ms to a representation used by SMA emeters.
         * I.e. the 32 least significant bits of the unix epoch time in milliseconds.
         * @param epoch_time_in_ms the given epoch time in ms
         * @return the emeter time
         */
        static uint32_t convertUnixEpochTimeToEmeterTimer(const uint64_t epoch_time_in_ms) {
            return  (uint32_t)epoch_time_in_ms;
        }

        /**
         * Convert the given unix epoch time in ms to a representation used by SMA inverters.
         * I.e. the 32 least significant bits of the unix epoch time in seconds.
         * @param epoch_time_in_ms the given unix epoch time in ms
         * @return the inverter time
         */
        static uint32_t convertUnixEpochTimeToInverterTimer(const uint64_t epoch_time_in_ms) {
            return  (uint32_t)(epoch_time_in_ms / 1000);
        }

        /**
         * Convert the given emeter time to an unix epoch time in ms.
         * This uses the current 64-bit epoch time to fill the missing msb bits of the 32-bit timestamp.
         * This only works if the given 32-bit inverter time is no older than 24 days.
         * @param emeter_time the given emeter time
         * @param unix_epoch_time_in_ms the current 64-bit unix epoch time in milliseconds
         * @return the unix epoch time in ms
         */
        static uint64_t convertEmeterTimeToUnixEpochTime(const uint32_t emeter_time, const uint64_t unix_epoch_time_in_ms = LocalHost::getUnixEpochTimeInMs()) {
            return expandTimeTo64(emeter_time, unix_epoch_time_in_ms);
        }

        /**
         * Convert the given inverter time to an unix epoch time in ms.
         * This uses the current 64-bit epoch time to fill the missing msb bits of the 32-bit timestamp.
         * This only works if the given 32-bit inverter time is no older than 24000 days.
         * @param inverter_time the given inverter time
         * @param unix_epoch_time_in_ms the current 64-bit unix epoch time in milliseconds
         * @return the unix epoch time in ms
         */
        static uint64_t convertInverterTimeToUnixEpochTime(const uint32_t inverter_time, const uint64_t unix_epoch_time_in_ms = LocalHost::getUnixEpochTimeInMs()) {
            const uint64_t current_time_in_sec = unix_epoch_time_in_ms / (uint64_t)1000;
            const uint64_t expanded_inverter_time = expandTimeTo64(inverter_time, current_time_in_sec);
            return expanded_inverter_time * (uint64_t)1000;    // convert to ms
        }

        /**
         * Convert the given emeter time to an inverter time.
         * This uses the current 64-bit epoch time to fill the missing msb bits of the 32-bit timestamp.
         * This only works if the given 32-bit inverter time is no older than 24 days.
         * @param emeter_time the given emeter time
         * @param unix_epoch_time_in_ms the current 64-bit unix epoch time in milliseconds
         * @return the inverter time
         */
        static uint32_t convertEmeterToInverterTime(const uint32_t emeter_time, const uint64_t unix_epoch_time_in_ms = LocalHost::getUnixEpochTimeInMs()) {
            const uint64_t emeter_time64 = convertEmeterTimeToUnixEpochTime(emeter_time, unix_epoch_time_in_ms);
            return convertUnixEpochTimeToInverterTimer(emeter_time64);
        }

        /**
         * Convert the given inverter time to an emeter time.
         * This uses the current 64-bit epoch time to fill the missing msb bits of the 32-bit timestamp.
         * This only works if the given 32-bit inverter time is no older than 24000 days.
         * @param inverter_time the given inverter time
         * @param unix_epoch_time_in_ms the current 64-bit unix epoch time in milliseconds
         * @return the emeter time
         */
        static uint32_t convertInverterToEmeterTime(const uint32_t inverter_time, const uint64_t unix_epoch_time_in_ms = LocalHost::getUnixEpochTimeInMs()) {
            const uint64_t inverter_time64 = convertInverterTimeToUnixEpochTime(inverter_time, unix_epoch_time_in_ms);
            return convertUnixEpochTimeToEmeterTimer(inverter_time64);
        }

        /**
         * Expand the given 32-bit timestamp to a 64-bit unix epoch time.
         * This uses the given 64-bit epoch time to fill the missing msb bits of the 32-bit timestamp.
         * @param truncated_time32 a 32-bit timestamp, like an emeter or inverter timestamp
         * @param current_time64 the 64-bit unix epoch time, typically the current time
         * @return the unix epoch time in ms
         */
        static uint64_t expandTimeTo64(const uint32_t truncated_time32, const uint64_t current_time64 = LocalHost::getUnixEpochTimeInMs()) {
            // split the 64-bit current time into 32-bit msb and lsb parts
            uint64_t current_time_msbs = (current_time64 >> 32u) & 0xffffffff;
            uint64_t current_time_lsbs = (current_time64) & 0xffffffff;

            // join the upper 32 bits of the unix epoch time with the lower 32 bits from the truncated timestamp
            uint64_t expanded_time64 = (current_time_msbs << 32u) | truncated_time32;
            uint64_t delta = calculateAbsTimeDifference(current_time64, expanded_time64);

            // check if the lsbs are in the upper half of the value range
            if ((current_time_lsbs & 0x80000000) != 0) {
                // if so, calculate an expanded time with the mbs incremented by 1 and compare the absolute difference
                // choose the expanded time closest to the current time
                uint64_t expanded_time64_p1 = ((current_time_msbs + 1) << 32u) | truncated_time32;
                uint64_t delta_p1 = calculateAbsTimeDifference(current_time64, expanded_time64_p1);
                if (delta_p1 <= delta) {
                    expanded_time64 = expanded_time64_p1;
                }
            }
            else {
                // if so, calculate an expanded time with the msbs decremented by 1 and compare the absolute difference
                // choose the expanded time closest to the current time
                uint64_t expanded_time64_m1 = ((current_time_msbs - 1) << 32u) | truncated_time32;
                uint64_t delta_m1 = calculateAbsTimeDifference(current_time64, expanded_time64_m1);
                if (delta_m1 <= delta) {
                    expanded_time64 = expanded_time64_m1;
                }
            }
            return expanded_time64;
        }

        /**
         *  Calculate the signed time difference between time1 and time2
         */
        static int32_t calculateTimeDifference(uint32_t time1, uint32_t time2) {
            return (int32_t)(time1 - time2);  // rely on an inherent 2's complement modulo arithmetic property
        }

        /**
         *  Calculate the signed time difference between time1 and time2
         */
        static int64_t calculateTimeDifference(uint64_t time1, uint64_t time2) {
            return (int64_t)(time1 - time2);  // rely on an inherent 2's complement modulo arithmetic property 
        }

        /**
         *  Calculate the absolute time difference between time1 and time2
         */
        static uint32_t calculateAbsTimeDifference(uint32_t time1, uint32_t time2) {
            int32_t signed_diff = calculateTimeDifference(time1, time2);
            return (signed_diff >= 0 ? signed_diff : -signed_diff);
        }

        /**
         *  Calculate the absolute time difference between time1 and time2
         */
        static uint64_t calculateAbsTimeDifference(uint64_t time1, uint64_t time2) {
            int64_t signed_diff = calculateTimeDifference(time1, time2);
            return (signed_diff >= 0 ? signed_diff : -signed_diff);
        }
    };

}   // namespace libspeedwire

#endif
