project("speedwire")

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

set(COMMON_SOURCES
    src/AddressConversion.cpp
    src/AveragingProcessor.cpp
    src/CalculatedValueProcessor.cpp
    src/LocalHost.cpp
    src/Logger.cpp
    src/MeasurementType.cpp
    src/ObisData.cpp
    src/ObisFilter.cpp
    src/SpeedwireAuthentication.cpp
    src/SpeedwireByteEncoding.cpp
    src/SpeedwireCommand.cpp
    src/SpeedwireData.cpp
    src/SpeedwireDiscovery.cpp
    src/SpeedwireDiscoveryProtocol.cpp
    src/SpeedwireEmeterProtocol.cpp
    src/SpeedwireEncryptionProtocol.cpp
    src/SpeedwireHeader.cpp
    src/SpeedwireInverterProtocol.cpp
    src/SpeedwireReceiveDispatcher.cpp
    src/SpeedwireSocket.cpp
    src/SpeedwireSocketFactory.cpp
    src/SpeedwireSocketSimple.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${COMMON_SOURCES}
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    include
)

add_subdirectory  (test EXCLUDE_FROM_ALL)
add_custom_target (tests)
add_dependencies  (tests speedwire_test)
