#ifndef __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__

#include <cstdint>
#include <string>

namespace libspeedwire {

    /**
     *  Enumeration describing known device classes.
     */
    enum class DeviceClass : uint16_t {
        UNKNOWN          = 0000,  //!< Unknown device class
        PV_INVERTER      = 8001,  //!< Photovoltaic inverter device class
        BATTERY_INVERTER = 8007,  //!< Battery inverter device class
        HYBRID_INVERTER  = 8009,  //!< Hybrid inverter device class
        EMETER           = 8065   //!< Electrical energy meter device class
    };

    //! Convert DeviceClass to a string
    inline std::string toString(const DeviceClass deviceclass) {
        switch (deviceclass) {
        case DeviceClass::PV_INVERTER:       return "PV-Inverter";
        case DeviceClass::BATTERY_INVERTER:  return "Battery-Inverter";
        case DeviceClass::HYBRID_INVERTER:   return "Hybrid-Inverter";
        case DeviceClass::EMETER:            return "Emeter";
        default:                             return "Unknown";
        }
    }

    /**
     *  Enumeration describing known device types.
     */
    enum class DeviceType : uint16_t {
        UNKNOWN        = 0000,      //!< Unknown device class
        Emeter10       = 0x245b,    //!< Emeter-1.0
        Emeter20       = 0x246f,    //!< Emeter-2.0
        HomeManager20  = 0x247F,    //!< Sunny-Home-Manager-2.0
        STP_4_0_3AV_40 = 0x2480,    //!< Tripower-4.0 v4.0
        STP_5_0_3AV_40 = 0x2481,    //!< Tripower-5.0 v4.0
        STP_6_0_3AV_40 = 0x2482     //!< Tripower-6.0 v4.0
    };


    /**
     *  Class describing known speedwire device.
     */
    class SpeedwireDevice {
    public:
        uint16_t    susyID;         //!< SusyID
        std::string name;           //!< Brief technical short name
        std::string longName;       //!< Longer marketing name
        DeviceClass deviceClass;    //!< Device class
        DeviceType  deviceType;     //!< Device type

        //! Constructor
        SpeedwireDevice(const uint16_t susy_id, const std::string& name_, const std::string& long_name, const DeviceClass& device_class, const DeviceType& device_type) :
            susyID(susy_id),
            name(name_),
            longName(long_name),
            deviceClass(device_class),
            deviceType(device_type)
        {}

        // pre-defined emeter instances
        static const SpeedwireDevice&Emeter10(void)      { static const SpeedwireDevice device(270, "EMETER-10", "Energy-Meter-1.0",   DeviceClass::EMETER, DeviceType::Emeter10); return device; }
        static const SpeedwireDevice&Emeter20(void)      { static const SpeedwireDevice device(349, "EMETER-20", "Energy-Meter-2.0",   DeviceClass::EMETER, DeviceType::Emeter20); return device; }
        static const SpeedwireDevice&HomeManager20(void) { static const SpeedwireDevice device(372, "HM-20", "Sunny-Home-Manager-2.0", DeviceClass::EMETER, DeviceType::HomeManager20); return device; }

        // pre-defined pv inverter instances
        static const SpeedwireDevice& Tripower4000_3AV40(void) { static const SpeedwireDevice device(378, "STP-4.0-3AV-40", "Sunny-Tripower-4.0-3AV-40", DeviceClass::PV_INVERTER, DeviceType::STP_4_0_3AV_40); return device; }
        static const SpeedwireDevice& Tripower5000_3AV40(void) { static const SpeedwireDevice device(378, "STP-5.0-3AV-40", "Sunny-Tripower-5.0-3AV-40", DeviceClass::PV_INVERTER, DeviceType::STP_5_0_3AV_40); return device; }
        static const SpeedwireDevice& Tripower6000_3AV40(void) { static const SpeedwireDevice device(378, "STP-6.0-3AV-40", "Sunny-Tripower-6.0-3AV-40", DeviceClass::PV_INVERTER, DeviceType::STP_6_0_3AV_40); return device; }

        // unknown device type
        static const SpeedwireDevice&Unknown(void)       { static const SpeedwireDevice device(0, "UNKNOWN", "Unknown Device", DeviceClass::UNKNOWN, DeviceType::UNKNOWN); return device; }

        /**
         * Return the SpeedwireDevice given the susy_id. This works only for emeter device types.
         * For other device types there is usually no unique relationship between susyid and device type.
         */
        static const SpeedwireDevice&fromSusyID(const uint16_t susy_id) {
            if (susy_id == Emeter10().susyID)       { return Emeter10(); }
            if (susy_id == Emeter20().susyID)       { return Emeter20(); }
            if (susy_id == HomeManager20().susyID)  { return HomeManager20(); }
            return Unknown();
        }

        /**
         * Return the SpeedwireDevice given the device type.
         */
        static const SpeedwireDevice& fromDeviceType(const DeviceType device_type) {
            if (device_type == Emeter10().deviceType) { return Emeter10(); }
            if (device_type == Emeter20().deviceType) { return Emeter20(); }
            if (device_type == HomeManager20().deviceType) { return HomeManager20(); }
            if (device_type == Tripower4000_3AV40().deviceType) { return Tripower4000_3AV40(); }
            if (device_type == Tripower5000_3AV40().deviceType) { return Tripower5000_3AV40(); }
            if (device_type == Tripower6000_3AV40().deviceType) { return Tripower6000_3AV40(); }
            return Unknown();
        }

    };

}   // namespace libspeedwire

#endif
