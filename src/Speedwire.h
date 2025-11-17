/**
 * Speedwire Library for ESP32/Arduino
 *
 * Main include file for the Speedwire library.
 * Include this file in your Arduino sketch to use the library.
 *
 * Example:
 *   #include <Speedwire.h>
 *   using namespace libspeedwire;
 */

#ifndef __SPEEDWIRE_H__
#define __SPEEDWIRE_H__

// Core functionality
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>

// Discovery
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireDiscoveryProtocol.hpp>

// Protocol parsing
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <SpeedwireInverterProtocol.hpp>
#include <SpeedwireData.hpp>

// Receive dispatcher
#include <SpeedwireReceiveDispatcher.hpp>

// OBIS data handling
#include <ObisData.hpp>
#include <ObisFilter.hpp>

// Utilities
#include <AddressConversion.hpp>
#include <Logger.hpp>
#include <SpeedwireByteEncoding.hpp>

// Device management
#include <SpeedwireDevice.hpp>

#endif // __SPEEDWIRE_H__
