#ifndef __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__

#include <cstdint>
#include <string>
#include <vector>

namespace libspeedwire {

    /**
     *  Enumeration describing well-known device classes.
     */
    enum class DeviceClass : uint16_t {
        UNKNOWN          = 0x0000,  //!< Unknown device class
        PV_INVERTER      = 0x8001,  //!< Photovoltaic inverter device class
        BATTERY_INVERTER = 0x8007,  //!< Battery inverter device class
        HYBRID_INVERTER  = 0x8009,  //!< Hybrid inverter device class
        EMETER           = 0x8065   //!< Electrical energy meter device class
    };

    //! Convert DeviceClass to a string
    std::string toString(const DeviceClass deviceclass) {
        switch (deviceclass) {
        case DeviceClass::PV_INVERTER:       return "PV-Inverter";
        case DeviceClass::BATTERY_INVERTER:  return "Battery-Inverter";
        case DeviceClass::HYBRID_INVERTER:   return "Hybrid-Inverter";
        case DeviceClass::EMETER:            return "Emeter";
        default:                             return "Unknown";
        }
    }

    /**
     *  Class describing well-known device types.
     */
    class DeviceType {
    public:
        uint16_t    susyID;         //!< SusyID
        std::string name;           //!< Brief technical short name
        std::string longName;       //!< Longer marketing name
        DeviceClass deviceClass;    //!< Device class

        //! Constructor
        DeviceType(const uint16_t susy_id, const std::string& name_, const std::string& long_name, const DeviceClass& device_class) :
            susyID(susy_id),
            name(name_),
            longName(long_name),
            deviceClass(device_class)
        {}

        // pre-defined emeter instances
        static const DeviceType &Emeter10(void)      { static DeviceType devicetype = DeviceType(270, "EMETER-10", "Energy-Meter-1.0", DeviceClass::EMETER); return devicetype; }
        static const DeviceType &Emeter20(void)      { static DeviceType devicetype = DeviceType(349, "EMETER-20", "Energy-Meter-2.0", DeviceClass::EMETER); return devicetype; }
        static const DeviceType &HomeManager20(void) { static DeviceType devicetype = DeviceType(372, "HM-20", "Sunny-Home-Manager-2.0", DeviceClass::EMETER); return devicetype; }

        // pre-defined pv inverter instances
        static const DeviceType &Tripower5000(void)  { static DeviceType devicetype = DeviceType(378, "STP-5.0-3AV-40", "Sunny-Tripower-5.0", DeviceClass::PV_INVERTER); return devicetype; }

        // unknown device type
        static const DeviceType &Unknown(void)       { static DeviceType devicetype = DeviceType(0, "UNKNOWN", "Unknown Device", DeviceClass::UNKNOWN); return devicetype; }

        static DeviceType fromSusyID(const uint16_t susy_id) {
            if (susy_id == Emeter10().susyID)       { return Emeter10(); }
            if (susy_id == Emeter20().susyID)       { return Emeter20(); }
            if (susy_id == HomeManager20().susyID)  { return HomeManager20(); }
            if (susy_id == Tripower5000().susyID)   { return Tripower5000(); }
            return Unknown();
        }
    };

}   // namespace libspeedwire

#endif
