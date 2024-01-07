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
        COMMUNICATION    = 8128,  //!< Communication product device class
        USER_DEFINED     = 8999   //!< User defined device class - not used by SMA
    };

    //! Convert DeviceClass to a string
    inline std::string toString(const SpeedwireDeviceClass deviceclass) {
        switch (deviceclass) {
        case SpeedwireDeviceClass::UNKNOWN:           return "Unknown";
        case SpeedwireDeviceClass::PV_INVERTER:       return "PV-Inverter";
        case SpeedwireDeviceClass::WIND_INVERTER:     return "Wind-Inverter";
        case SpeedwireDeviceClass::BATTERY_INVERTER:  return "Battery-Inverter";
        case SpeedwireDeviceClass::HYBRID_INVERTER:   return "Hybrid-Inverter";
        case SpeedwireDeviceClass::LOAD:              return "Load";
        case SpeedwireDeviceClass::SENSOR:            return "Sensor";
        case SpeedwireDeviceClass::EMETER:            return "Emeter";
        case SpeedwireDeviceClass::COMMUNICATION:     return "Communication";
        case SpeedwireDeviceClass::USER_DEFINED:      return "User-Defined";
        default:                                      return "Unknown";
        }
    }

    enum class SpeedwireDeviceModel : uint16_t {
        UNKNOWN = 0000  //!< Unknown device model
    };


    /**
     *  Class encapsulating information about a speedwire device instance.
     */
    class SpeedwireDevice {
    public:
        uint16_t    susyID;                //!< Susy ID of the speedwire device.
        uint32_t    serialNumber;          //!< Serial number of the speedwire device.
        std::string deviceClass;           //!< Device class of the speedwire device, i.e. emeter or inverter.
        std::string deviceModel;           //!< Device model of the speedwire device, i.e. emeter or inverter.
        std::string peer_ip_address;       //!< IP address of the device, either on the local subnet or somewhere else.
        std::string interface_ip_address;  //!< IP address of the local interface through which the device is reachable.

        /** Default constructor.
         *  Just initialize all member variables to a defined state; set susyId and serialNumber to 0. */
        SpeedwireDevice(void) : susyID(0), serialNumber(0), deviceClass(), deviceModel(), peer_ip_address(), interface_ip_address() {}

        /** Convert speedwire information to a single line string. */
        std::string toString(void) const {
            char buffer[256] = { 0 };
            snprintf(buffer, sizeof(buffer), "SusyID %3u  Serial %10u  Class %-16s  Model %-14s  IP %s  IF %s",
                susyID, serialNumber, deviceClass.c_str(), deviceModel.c_str(), peer_ip_address.c_str(), interface_ip_address.c_str());
            return std::string(buffer);
        }

        /** Compare two instances; assume that if SusyID, Serial and IP is the same, it is the same device. */
        bool operator==(const SpeedwireDevice& rhs) const { return (susyID == rhs.susyID && serialNumber == rhs.serialNumber && peer_ip_address == rhs.peer_ip_address); }

        /** Check if this instance is just pre-registered with a given IP, i.e the device ip address is given. */
        bool hasIPAddressOnly(void) const { return (peer_ip_address.length() > 0 && susyID == 0 && serialNumber == 0); }

        /** Check if this instance is just pre-registered with a given serial number, i.e the device serial number is given. */
        bool hasSerialNumberOnly(void) const { return (peer_ip_address.length() == 0 && susyID == 0 && serialNumber != 0); }

        /** Check if this instance is fully registered, i.e all device information is given. */
        bool isComplete(void) const {
            return (susyID != 0 && serialNumber != 0 && deviceClass.length() > 0 && deviceModel.length() > 0 && peer_ip_address.length() > 0 && interface_ip_address.length() > 0);
        }

        static const SpeedwireDevice &getLocalDevice(void) {
            static SpeedwireDevice local;
            local.susyID = 0x0078;
            local.serialNumber = 0x3a28be52;
            local.deviceClass = "Communication";
            local.deviceModel = "Local";
            local.peer_ip_address = "127.0.0.1";
            local.interface_ip_address = local.peer_ip_address;
            return local;
        }
    };


    /**
     *  Class describing known speedwire device types.
     */
    class SpeedwireDeviceType {
    public:
        SpeedwireDeviceClass deviceClass;    //!< Device class
        SpeedwireDeviceModel deviceModel;    //!< Device model
        uint16_t             susyID;         //!< SusyID
        std::string          name;           //!< Brief technical short name
        std::string          longName;       //!< Longer marketing name

        //! Constructor
        SpeedwireDeviceType(const SpeedwireDeviceClass& device_class, const SpeedwireDeviceModel& device_model, const uint16_t susy_id, const std::string& name_, const std::string& long_name) :
            deviceClass(device_class),
            deviceModel(device_model),
            susyID(susy_id),
            name(name_),
            longName(long_name)
        {}

        // pre-defined emeter types
        static const SpeedwireDeviceType& Emeter10(void)      { static const SpeedwireDeviceType device(SpeedwireDeviceClass::EMETER, (SpeedwireDeviceModel)9307, 270, "EMETER-10", "Energy-Meter-1.0"); return device; }
        static const SpeedwireDeviceType& Emeter20(void)      { static const SpeedwireDeviceType device(SpeedwireDeviceClass::EMETER, (SpeedwireDeviceModel)9327, 349, "EMETER-20", "Energy-Meter-2.0"); return device; }
        static const SpeedwireDeviceType& HomeManager20(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::EMETER, (SpeedwireDeviceModel)9343, 372, "HM-20", "Sunny-Home-Manager-2.0"); return device; }

        // pre-defined battery inverter types
        static const SpeedwireDeviceType& SBS1500_1VL10(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::BATTERY_INVERTER, (SpeedwireDeviceModel)9324, 346, "SBS1.5-1VL-10", "Sunny-Boy-Storage-1.5-1VL-10"); return device; }
        static const SpeedwireDeviceType& SBS2000_1VL10(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::BATTERY_INVERTER, (SpeedwireDeviceModel)9325, 346, "SBS2.0-1VL-10", "Sunny-Boy-Storage-2.0-1VL-10"); return device; }
        static const SpeedwireDeviceType& SBS2500_1VL10(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::BATTERY_INVERTER, (SpeedwireDeviceModel)9326, 346, "SBS2.5-1VL-10", "Sunny-Boy-Storage-2.5-1VL-10"); return device; }

        // pre-defined pv inverter types
        static const SpeedwireDeviceType& Tripower4000_3AV40(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::PV_INVERTER, (SpeedwireDeviceModel)9344, 378, "STP-4.0-3AV-40", "Sunny-Tripower-4.0-3AV-40"); return device; }
        static const SpeedwireDeviceType& Tripower5000_3AV40(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::PV_INVERTER, (SpeedwireDeviceModel)9345, 378, "STP-5.0-3AV-40", "Sunny-Tripower-5.0-3AV-40"); return device; }
        static const SpeedwireDeviceType& Tripower6000_3AV40(void) { static const SpeedwireDeviceType device(SpeedwireDeviceClass::PV_INVERTER, (SpeedwireDeviceModel)9346, 378, "STP-6.0-3AV-40", "Sunny-Tripower-6.0-3AV-40"); return device; }

        // unknown device type
        static const SpeedwireDeviceType& Unknown(void)       { static const SpeedwireDeviceType device(SpeedwireDeviceClass::UNKNOWN, SpeedwireDeviceModel::UNKNOWN, 0, "UNKNOWN", "Unknown Device"); return device; }

        /**
         * Return the SpeedwireDeviceType given the susy_id. This works only for emeter device types.
         * For other device types there is usually no unique relationship between susyid and device type.
         */
        static const SpeedwireDeviceType& fromSusyID(const uint16_t susy_id) {
            if (susy_id == Emeter10().susyID)           { return Emeter10(); }
            if (susy_id == Emeter20().susyID)           { return Emeter20(); }
            if (susy_id == HomeManager20().susyID)      { return HomeManager20(); }
            if (susy_id == SBS2500_1VL10().susyID)      { return SBS2500_1VL10(); }
            if (susy_id == Tripower5000_3AV40().susyID) { return Tripower5000_3AV40(); }
            return Unknown();
        }

        /**
         * Return the SpeedwireDeviceType given the device model.
         */
        static const SpeedwireDeviceType& fromDeviceModel(const SpeedwireDeviceModel model) {
            if (model == Emeter10().deviceModel) { return Emeter10(); }
            if (model == Emeter20().deviceModel) { return Emeter20(); }
            if (model == HomeManager20().deviceModel) { return HomeManager20(); }
            if (model == Tripower4000_3AV40().deviceModel) { return Tripower4000_3AV40(); }
            if (model == Tripower5000_3AV40().deviceModel) { return Tripower5000_3AV40(); }
            if (model == Tripower6000_3AV40().deviceModel) { return Tripower6000_3AV40(); }
            if (model == SBS1500_1VL10().deviceModel) { return SBS1500_1VL10(); }
            if (model == SBS2000_1VL10().deviceModel) { return SBS2000_1VL10(); }
            if (model == SBS2500_1VL10().deviceModel) { return SBS2500_1VL10(); }
            return Unknown();
        }

    };

}   // namespace libspeedwire

#endif
