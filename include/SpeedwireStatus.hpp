#ifndef __LIBSPEEDWIRE_SPEEDWIRESTATUS_HPP__
#define __LIBSPEEDWIRE_SPEEDWIRESTATUS_HPP__


namespace libspeedwire {

    /**
     *  Enumeration of speedwire status values. This is a weakly ordered set of arbitrarily assigned values.
     *  From EDMx-Modbus-TI-en-16.pdf and from SBFspot
     */
    class SpeedwireStatus {
    public:
        uint32_t            value;
        std::string         name;           //!< Brief technical short name
        std::string         longName;       //!< Longer descriptive name

        //! Constructor
        SpeedwireStatus(const uint32_t value_, const std::string& name_, const std::string& long_name) :
            value(value_), name(name_), longName(long_name) {}

        //! Default constructor; needed for std::map.
        SpeedwireStatus(void) : value(0), name(), longName() {}

        // Operator overloads to simplify comparison and assignment operations
        operator uint32_t() const { return value; }
        operator std::string() const { return name; }
        bool operator==(const SpeedwireStatus& rhs) const { return value == rhs.value; }
        bool operator!=(const SpeedwireStatus& rhs) const { return value != rhs.value; }

        // unknown instance
        static const SpeedwireStatus& NOTFOUND(void) { static const SpeedwireStatus status(0xffffffff, "NOTFOUND", "NOTFOUND"); return status; }

        // pre-defined status instances
        static const SpeedwireStatus& Error(void) { static const SpeedwireStatus status(35, "Error", "Error"); return status; }
        static const SpeedwireStatus& Closed(void) { static const SpeedwireStatus status(51, "Closed", "Closed"); return status; }
        static const SpeedwireStatus& None(void) { static const SpeedwireStatus status(302, "None", "None"); return status; }
        static const SpeedwireStatus& Off(void) { static const SpeedwireStatus status(303, "Off", "Off"); return status; }
        static const SpeedwireStatus& Ok(void) { static const SpeedwireStatus status(307, "OK", "OK"); return status; }
        static const SpeedwireStatus& On(void) { static const SpeedwireStatus status(308, "On", "On"); return status; }
        static const SpeedwireStatus& Operation(void) { static const SpeedwireStatus status(309, "Operation", "Operation"); return status; }
        static const SpeedwireStatus& Open(void) { static const SpeedwireStatus status(311, "Open", "Open"); return status; }
        static const SpeedwireStatus& ContactSMA(void) { static const SpeedwireStatus status(336, "ContactSMA", "Contact SMA"); return status; }
        static const SpeedwireStatus& ContactInstaller(void) { static const SpeedwireStatus status(337, "ContactInst", "Contact Installer"); return status; }
        static const SpeedwireStatus& Invalid(void) { static const SpeedwireStatus status(338, "Invalid", "Invalid"); return status; }
        static const SpeedwireStatus& Stop(void) { static const SpeedwireStatus status(381, "Stop", "Stop"); return status; }
        static const SpeedwireStatus& Warning(void) { static const SpeedwireStatus status(455, "Warning", "Warning"); return status; }
        static const SpeedwireStatus& Activated(void) { static const SpeedwireStatus status(569, "Activated", "Activated"); return status; }
        static const SpeedwireStatus& Active(void) { static const SpeedwireStatus status(802, "Active", "Active"); return status; }
        static const SpeedwireStatus& Inactive(void) { static const SpeedwireStatus status(803, "Inactive", "Inactive"); return status; }
        static const SpeedwireStatus& NoDescription(void) { static const SpeedwireStatus status(885, "NoDescr", "No description available"); return status; }
        static const SpeedwireStatus& NoMessage(void) { static const SpeedwireStatus status(886, "NoMessage", "No message available"); return status; }
        static const SpeedwireStatus& NoAction(void) { static const SpeedwireStatus status(887, "NoAction", "No suggested action"); return status; }
        static const SpeedwireStatus& Yes(void) { static const SpeedwireStatus status(1129, "Yes", "Yes"); return status; }
        static const SpeedwireStatus& No(void) { static const SpeedwireStatus status(1130, "No", "No"); return status; }
        static const SpeedwireStatus& GridSwOpen(void) { static const SpeedwireStatus status(1131, "GrSwOpen", "Grid switch open"); return status; }
        static const SpeedwireStatus& Standby(void) { static const SpeedwireStatus status(1295, "Standby", "Standby"); return status; }
        static const SpeedwireStatus& Locked(void) { static const SpeedwireStatus status(1795, "Locked", "Locked"); return status; }
        static const SpeedwireStatus& UpdateRecv(void) { static const SpeedwireStatus status(3179, "UpdateRecv", "Update receiving"); return status; }
        static const SpeedwireStatus& UpdateExec(void) { static const SpeedwireStatus status(3180, "UpdateExec", "Update executing"); return status; }
        static const SpeedwireStatus& UpdateOK(void) { static const SpeedwireStatus status(3181, "UpdateOK", "Update installed OK"); return status; }
        static const SpeedwireStatus& UpdateFailed(void) { static const SpeedwireStatus status(3182, "UpdateFail", "Update failed"); return status; }
        static const SpeedwireStatus& UpdateNone(void) { static const SpeedwireStatus status(3584, "UpdateNone", "Update none"); return status; }
        static const SpeedwireStatus& NaN(void) { static const SpeedwireStatus status(0x00fffffd, "NaN", "Nan"); return status; }
        static const SpeedwireStatus& EoD(void) { static const SpeedwireStatus status(0x00fffffe, "EoD", "EoD"); return status; }

        static std::vector<SpeedwireStatus> getAllPredefined(void) {
            std::vector<SpeedwireStatus> predefined;
            predefined.push_back(Error());
            predefined.push_back(Closed());
            predefined.push_back(None());
            predefined.push_back(Off());
            predefined.push_back(Ok());
            predefined.push_back(On());
            predefined.push_back(Operation());
            predefined.push_back(Open());
            predefined.push_back(ContactSMA());
            predefined.push_back(ContactInstaller());
            predefined.push_back(Invalid());
            predefined.push_back(Stop());
            predefined.push_back(Warning());
            predefined.push_back(Activated());
            predefined.push_back(Active());
            predefined.push_back(Inactive());
            predefined.push_back(NoDescription());
            predefined.push_back(NoMessage());
            predefined.push_back(NoAction());
            predefined.push_back(Yes());
            predefined.push_back(No());
            predefined.push_back(GridSwOpen());
            predefined.push_back(Standby());
            predefined.push_back(Locked());
            predefined.push_back(UpdateRecv());
            predefined.push_back(UpdateExec());
            predefined.push_back(UpdateOK());
            predefined.push_back(UpdateFailed());
            predefined.push_back(UpdateNone());
            predefined.push_back(NaN());
            predefined.push_back(EoD());
            return predefined;
        }
    };


    /**
     *  Class implementing a map for emeter obis data.
     *  The class extends std::map<uint32_t, ObisData>.
     */
    class SpeedwireStatusMap : public std::map<uint32_t, SpeedwireStatus> {
    public:
        /**
         *  Default constructor.
         */
        SpeedwireStatusMap(void) : std::map<uint32_t, SpeedwireStatus>() {}

        /**
         *  Construct a new map from the given vector of SpeedwireStatus elements.
         *  @param elements the vector of SpeedwireStatus elements
         */
        SpeedwireStatusMap(const std::vector<SpeedwireStatus>& elements) {
            this->add(elements);
        }

        /**
         *  Add a new element to the map of speedwire status elements.
         *  @param element The SpeedwireStatus element to be added to the map
         */
        void add(const SpeedwireStatus& element) { operator[](element.value) = element; }

        /**
         *  Add a vector of elements to the map of speedwire status elements.
         *  @param elements The vector of SpeedwireStatus element to be added to the map
         */
        void add(const std::vector<SpeedwireStatus>& elements) {
            for (auto& e : elements) {
                add(e);
            }
        }

        /**
         *  Remove the given element from the map of speedwire status elements.
         *  @param element The SpeedwireStatus element to be removed from the map
         */
        void remove(const SpeedwireStatus& entry) {
            erase(entry.value);
        }
        
        /**
         *  Get a reference to the SpeedwireStatusMap containing all predefined elements
         *  @return the map
         */
        static const SpeedwireStatusMap& getAllPredefined(void) {
            static SpeedwireStatusMap allPredefined;
            if (allPredefined.size() == 0) {
                allPredefined = SpeedwireStatusMap(SpeedwireStatus::getAllPredefined());
            }
            return allPredefined;
        }

        /**
         *  Find the given status value in the map map of predefined speedwire status elements.
         *  @param value status value
         *  @return a const iterator pointing to the element, or to cend()
         */
        static const SpeedwireStatusMap::const_iterator findPredefined(uint32_t value) {
            const SpeedwireStatusMap& map = getAllPredefined();
            return map.find(value & 0x00ffffff);
        }

        /**
         *  Check if the given status value is contained in the map map of predefined speedwire status elements.
         *  @param value status value
         *  @return true or false
         */
        static bool isPredefined(uint32_t value) {
            const SpeedwireStatusMap& map = getAllPredefined();
            const auto& it = map.find(value & 0x00ffffff);
            return (it == map.cend());
        }

        /**
         *  Get the given status value from the map map of predefined speedwire status elements.
         *  @param value status value
         *  @return a const reference to the predefined speedwire status element, or a const reference to the NOTFOUND status element
         */
        static const SpeedwireStatus& getPredefined(uint32_t value) {
            const SpeedwireStatusMap& map = getAllPredefined();
            const auto& it = map.find(value & 0x00ffffff);
            if (it != map.cend()) {
                return it->second;
            }
            return SpeedwireStatus::NOTFOUND();
        }
    };

}   // namespace libspeedwire

#endif
