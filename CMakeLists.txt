project("speedwire")

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

set(COMMON_SOURCES
    src/DataProcessor.cpp
    src/AddressConversion.cpp
    src/LocalHost.cpp
    src/Logger.cpp
    src/Measurement.cpp
    src/ObisData.cpp
    src/ObisFilter.cpp
    src/SpeedwireByteEncoding.cpp
    src/SpeedwireCommand.cpp
    src/SpeedwireData.cpp
    src/SpeedwireDiscovery.cpp
    src/SpeedwireEmeterProtocol.cpp
    src/SpeedwireHeader.cpp
    src/SpeedwireInverterProtocol.cpp
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
