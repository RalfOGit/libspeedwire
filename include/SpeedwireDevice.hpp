#ifndef __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__

#include <cstdint>
#include <string>

namespace libspeedwire {

    /**
     *  Enumeration describing known device classes. Definitions from: SMA_Modbus-TB-de-13 Version 1.3
     */
    enum class SpeedwireDeviceClass : uint16_t {
        UNKNOWN          = 0000,  //!< Unknown device class
        PV_INVERTER      = 8001,  //!< Photovoltaic inverter device class
        WIND_INVERTER    = 8002,  //!< Wind turbine inverter device class
        BATTERY_INVERTER = 8007,  //!< Battery inverter device class
        HYBRID_INVERTER  = 8009,  //!< Hybrid inverter device class
        LOAD             = 8033,  //!< Energy consumer device class
        SENSOR           = 8064,  //!< General sensor device class
        EMETER           = 8065,  //!< Electrical energy meter device class
        COMMUNICATION    = 8128   //!< Communication product device class
    };

    //! Convert DeviceClass to a string
    inline std::string toString(const SpeedwireDeviceClass deviceclass) {
        switch (deviceclass) {
        case SpeedwireDeviceClass::PV_INVERTER:       return "PV-Inverter";
        case SpeedwireDeviceClass::WIND_INVERTER:     return "Wind-Inverter";
        case SpeedwireDeviceClass::BATTERY_INVERTER:  return "Battery-Inverter";
        case SpeedwireDeviceClass::HYBRID_INVERTER:   return "Hybrid-Inverter";
        case SpeedwireDeviceClass::LOAD:              return "Load";
        case SpeedwireDeviceClass::SENSOR:            return "Sensor";
        case SpeedwireDeviceClass::EMETER:            return "Emeter";
        case SpeedwireDeviceClass::COMMUNICATION:     return "Communication";
        default:                                      return "Unknown";
        }
    }

    enum class SpeedwireDeviceType : uint16_t {
        UNKNOWN = 0000  //!< Unknown device type
    };

    /**
     *  Class describing known speedwire device.
     */
    class SpeedwireDevice {
    public:
        SpeedwireDeviceClass deviceClass;    //!< Device class
        SpeedwireDeviceType  deviceType;     //!< Device type
        uint16_t             susyID;         //!< SusyID
        std::string          name;           //!< Brief technical short name
        std::string          longName;       //!< Longer marketing name

        //! Constructor
        SpeedwireDevice(const SpeedwireDeviceClass& device_class, const SpeedwireDeviceType& device_type, const uint16_t susy_id, const std::string& name_, const std::string& long_name) :
            deviceClass(device_class),
            deviceType(device_type),
            susyID(susy_id),
            name(name_),
            longName(long_name)
        {}

        // pre-defined emeter instances
        static const SpeedwireDevice& Emeter10(void)      { static const SpeedwireDevice device(SpeedwireDeviceClass::EMETER, (SpeedwireDeviceType)9307, 270, "EMETER-10", "Energy-Meter-1.0"); return device; }
        static const SpeedwireDevice& Emeter20(void)      { static const SpeedwireDevice device(SpeedwireDeviceClass::EMETER, (SpeedwireDeviceType)9327, 349, "EMETER-20", "Energy-Meter-2.0"); return device; }
        static const SpeedwireDevice& HomeManager20(void) { static const SpeedwireDevice device(SpeedwireDeviceClass::EMETER, (SpeedwireDeviceType)9343, 372, "HM-20", "Sunny-Home-Manager-2.0"); return device; }

        // pre-defined pv inverter instances
        static const SpeedwireDevice& Tripower4000_3AV40(void) { static const SpeedwireDevice device(SpeedwireDeviceClass::PV_INVERTER, (SpeedwireDeviceType)9344, 378, "STP-4.0-3AV-40", "Sunny-Tripower-4.0-3AV-40"); return device; }
        static const SpeedwireDevice& Tripower5000_3AV40(void) { static const SpeedwireDevice device(SpeedwireDeviceClass::PV_INVERTER, (SpeedwireDeviceType)9345, 378, "STP-5.0-3AV-40", "Sunny-Tripower-5.0-3AV-40"); return device; }
        static const SpeedwireDevice& Tripower6000_3AV40(void) { static const SpeedwireDevice device(SpeedwireDeviceClass::PV_INVERTER, (SpeedwireDeviceType)9346, 378, "STP-6.0-3AV-40", "Sunny-Tripower-6.0-3AV-40"); return device; }

        // unknown device type
        static const SpeedwireDevice& Unknown(void)       { static const SpeedwireDevice device(SpeedwireDeviceClass::UNKNOWN, SpeedwireDeviceType::UNKNOWN, 0, "UNKNOWN", "Unknown Device"); return device; }

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
        static const SpeedwireDevice& fromDeviceType(const SpeedwireDeviceType device_type) {
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
