#ifndef __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__
#define __LIBSPEEDWIRE_SPEEDWIREDEVICE_HPP__

#include <cstdint>
#include <string>
#include <AddressConversion.hpp>
#include <LocalHost.hpp>

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
    class SpeedwireAddress {
    public:
        uint16_t    susyID;                //!< Susy ID of the speedwire device.
        uint32_t    serialNumber;          //!< Serial number of the speedwire device.

        SpeedwireAddress(void) : susyID(0), serialNumber(0) {}
        SpeedwireAddress(const uint16_t susyid, const uint32_t serial) : susyID(susyid), serialNumber(serial) {}

        /** Compare two instances. */
        bool operator==(const SpeedwireAddress& rhs) const { return (susyID == rhs.susyID && serialNumber == rhs.serialNumber); }

        bool isComplete(void) const { return (susyID != 0 && serialNumber != 0); }

        bool isBroadcast(void) const { return (susyID == 0xffff && serialNumber == 0xffffffff); }

        /** Convert SpeedwireAddress to a string */
        std::string toString(void) const {
            char buffer[256] = { 0 };
            snprintf(buffer, sizeof(buffer), "%u:%u", susyID, serialNumber);
            return std::string(buffer);
        }

        /** Get a reference to a local device address. This can be used as a source device for commands. */
        static const SpeedwireAddress& getLocalAddress(void) {
            static SpeedwireAddress local(0x0078, 0x3a28be52);
            // assign different serial numbers for libspeedwire instances running on different nodes
            // based on the least-significant byte of the local interfaces ip address
            static bool initialized = false;
            if (!initialized) {
                initialized = true;
                for (const auto& if_addr : LocalHost::getInstance().getLocalIPv4Addresses()) {
                    if (if_addr.substr(0, 7) == "192.168") {
                        uint8_t byte0 = (uint8_t)((AddressConversion::toInAddress(if_addr).s_addr >> 24) & 0xff);
                        local.serialNumber = ((local.serialNumber / 1000) * 1000) + byte0;
                    }
                }
            }
            return local;
        }

        /** Get a reference to a broadcast device address. This can be used as a broadcast destination device for commands. */
        static const SpeedwireAddress& getBroadcastAddress(void) {
            static SpeedwireAddress broadcast(0xffff, 0xffffffff);
            return broadcast;
        }
    };


    /**
     *  Class encapsulating information about a speedwire device instance.
     */
    class SpeedwireDevice {
    public:
        SpeedwireAddress deviceAddress;         //!< Speedwire device address, i.e. susy ID and serial number
        std::string      deviceClass;           //!< Device class of the speedwire device, i.e. emeter or inverter.
        std::string      deviceModel;           //!< Device model of the speedwire device, i.e. emeter or inverter.
        std::string      deviceIpAddress;       //!< IP address of the device, either on the local subnet or somewhere else.
        std::string      interfaceIpAddress;    //!< IP address of the local interface through which the device is reachable.

        /** Default constructor.
         *  Just initialize all member variables to a defined state; set susyId and serialNumber to 0. */
        SpeedwireDevice(void) : deviceAddress(), deviceClass(), deviceModel(), deviceIpAddress(), interfaceIpAddress() {}

        /** Convert speedwire information to a single line string. */
        std::string toString(void) const {
            char buffer[256] = { 0 };
            snprintf(buffer, sizeof(buffer), "SusyID %3u  Serial %10u  Class %-16s  Model %-14s  IP %s  IF %s",
                deviceAddress.susyID, deviceAddress.serialNumber, deviceClass.c_str(), deviceModel.c_str(), deviceIpAddress.c_str(), interfaceIpAddress.c_str());
            return std::string(buffer);
        }

        /** Compare two instances; assume that if SusyID, Serial and IP is the same, it is the same device. */
        bool operator==(const SpeedwireDevice& rhs) const { return (deviceAddress == rhs.deviceAddress && deviceIpAddress == rhs.deviceIpAddress); }

        /** Check if this instance is just pre-registered with a given IP, i.e the device ip address is given. */
        bool hasIPAddressOnly(void) const { return (deviceIpAddress.length() > 0 && deviceAddress.isComplete() == false); }

        /** Check if this instance is just pre-registered with a given serial number, i.e the device serial number is given. */
        bool hasSerialNumberOnly(void) const { return (deviceIpAddress.length() == 0 && deviceAddress.susyID == 0 && deviceAddress.serialNumber != 0); }

        /** Check if this instance is fully registered, i.e all device information is given. */
        bool isComplete(void) const {
            return (deviceAddress.isComplete() && deviceClass.length() > 0 && deviceModel.length() > 0 && deviceIpAddress.length() > 0 && interfaceIpAddress.length() > 0);
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
