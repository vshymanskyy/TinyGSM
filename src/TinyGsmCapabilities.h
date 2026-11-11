/**
 * @file       TinyGsmCapabilities.h
 * @brief      Compile-time capability detection traits for TinyGSM modems.
 * @author     Sara Damiano
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2026
 * @date       2026
 */

#ifndef SRC_TINYGSMCAPABILITIES_H_
#define SRC_TINYGSMCAPABILITIES_H_

// Forward declarations of capability template classes
template <class modemType, class sslConfig>
class TinyGsmSSL;
template <class modemType>
class TinyGsmGPS;
template <class modemType>
class TinyGsmCellularData;
template <class modemType>
class TinyGsmWifi;
template <class modemType>
class TinyGsmSMS;
template <class modemType>
class TinyGsmCalling;
template <class modemType>
class TinyGsmBattery;
template <class modemType>
class TinyGsmTemperature;
template <class modemType>
class TinyGsmNTP;
template <class modemType>
class TinyGsmTime;
template <class modemType>
class TinyGsmGSMLocation;
template <class modemType>
class TinyGsmBluetooth;
template <class modemType, class tcpConfig>
class TinyGsmTCP;

// Forward declarations of modem types
class TinyGsmSim800;
class TinyGsmSim808;
class TinyGsmUBLOX;
class TinyGsmSaraR4;
class TinyGsmSaraR5;
class TinyGsmSequansMonarch;
class TinyGsmESP8266;
class TinyGsmESP8266NonOS;
class TinyGsmXBee;

/**
 * @brief Namespace for TinyGSM capability detection traits.
 *
 * These traits check if a modem type inherits from specific capability
 * templates to determine what features are available.
 *
 * Custom implementations of type traits for AVR compatibility (no
 * type_traits). Compatible with C++11 and later.
 *
 * Usage:
 * @code
 * // Query capability at compile time
 * if (TinyGsmCapabilities::has_ssl<TinyGsm>::value) {
 *   Serial.println("SSL is supported");
 * }
 *
 * // For conditional compilation, use C++17 if constexpr:
 * if constexpr (TinyGsmCapabilities::has_ssl<TinyGsm>::value) {
 *   TinyGsmClientSecure client(modem);  // Only compiles if SSL available
 * }
 * @endcode
 */
namespace TinyGsmCapabilities {

/**
 * @brief Custom integral_constant (like std::integral_constant)
 */
template <typename T, T v>
struct integral_constant {
  static constexpr T        value = v;
  typedef T                 value_type;
  typedef integral_constant type;
};

typedef integral_constant<bool, true>  true_type;
typedef integral_constant<bool, false> false_type;

/**
 * @brief C++11-compatible helper for detecting nested types.
 */
template <typename T>
struct make_void {
  typedef void type;
};

/**
 * @brief Helper for is_base_of - computes inheritance relationship
 */
template <typename Base, typename Derived>
struct is_base_of_impl {
 private:
  // Use sizeof trick to determine inheritance at compile time
  // If Derived* can be converted to Base*, inheritance exists
  typedef char yes[1];
  typedef char no[2];

  static yes& test(Base*);
  static no&  test(...);

  // Create a pointer to Derived for the test
  static Derived* getDerived();

 public:
  static const bool value = sizeof(test(getDerived())) == sizeof(yes);
};

/**
 * @brief Custom is_base_of implementation (simplified)
 * Tests if Base is a base class of Derived
 */
template <typename Base, typename Derived>
struct is_base_of
    : integral_constant<bool, is_base_of_impl<Base, Derived>::value> {};

/**
 * @brief Detect if a modem type has SSL/TLS support
 * @note Default implementation checks for TinyGsmSSL base class
 */
template <typename T, typename = void>
struct has_ssl : false_type {};

/// @copydoc has_ssl
template <typename T>
struct has_ssl<T, typename make_void<typename T::SslConfig>::type>
    : is_base_of<TinyGsmSSL<T, typename T::SslConfig>, T> {};

/**
 * @brief Detect if a modem type can specify certificates
 * @note Default implementation checks for TinyGsmSSL base class
 */
template <typename T, typename = void>
struct can_specify_certs : false_type {};

/// @copydoc can_specify_certs
template <typename T>
struct can_specify_certs<T, typename make_void<typename T::SslConfig>::type>
    : is_base_of<TinyGsmSSL<T, typename T::SslConfig>, T> {};

/**
 * @brief Detect if a modem type can load certificates
 * @note Default implementation checks for TinyGsmSSL base class
 */
template <typename T, typename = void>
struct can_load_certs : false_type {};

/// @copydoc can_load_certs
template <typename T>
struct can_load_certs<T, typename make_void<typename T::SslConfig>::type>
    : is_base_of<TinyGsmSSL<T, typename T::SslConfig>, T> {};

/**
 * @brief Detect if a modem type has GPS/GNSS support
 */
template <typename T>
struct has_gps : is_base_of<TinyGsmGPS<T>, T> {};

/**
 * @brief Detect if a modem type has cellular data support
 */
template <typename T>
struct has_cellular_data : is_base_of<TinyGsmCellularData<T>, T> {};

/**
 * @brief Detect if a modem type has WiFi support
 */
template <typename T>
struct has_wifi : is_base_of<TinyGsmWifi<T>, T> {};

/**
 * @brief Detect if a modem type has SMS messaging support
 */
template <typename T>
struct has_sms : is_base_of<TinyGsmSMS<T>, T> {};

/**
 * @brief Detect if a modem type has voice calling support
 */
template <typename T>
struct has_calling : is_base_of<TinyGsmCalling<T>, T> {};

/**
 * @brief Detect if a modem type has battery status support
 */
template <typename T>
struct has_battery : is_base_of<TinyGsmBattery<T>, T> {};

/**
 * @brief Detect if a modem type has temperature sensor support
 */
template <typename T>
struct has_temperature : is_base_of<TinyGsmTemperature<T>, T> {};

/**
 * @brief Detect if a modem type has NTP support
 */
template <typename T>
struct has_ntp : is_base_of<TinyGsmNTP<T>, T> {};

/**
 * @brief Detect if a modem type has time/clock functions
 */
template <typename T>
struct has_time : is_base_of<TinyGsmTime<T>, T> {};

/**
 * @brief Detect if a modem type has GSM location support
 */
template <typename T>
struct has_gsm_location : is_base_of<TinyGsmGSMLocation<T>, T> {};

/**
 * @brief Detect if a modem type has Bluetooth support
 */
template <typename T>
struct has_bluetooth : is_base_of<TinyGsmBluetooth<T>, T> {};

/**
 * @brief Detect if a modem type has TCP socket support
 * @note Always true since all modems must support TCP
 */
template <typename T>
struct has_tcp : true_type {};

// ============================================================================
// Specializations for modems with non-standard SSL support
// ============================================================================

// Modems that have SSL but don't inherit from TinyGsmSSL
// (They use TINY_GSM_MODEM_HAS_SSL but don't have certificate management)


#if !defined(TINY_GSM_MODEM_SIM900)
// The SIM900 doesn't have SSL
template <>
struct has_ssl<TinyGsmSim800> : true_type {};
#endif
template <>
struct can_specify_certs<TinyGsmSim800> : false_type {};
template <>
struct can_load_certs<TinyGsmSim800> : false_type {};

template <>
struct has_ssl<TinyGsmSim808> : true_type {};
template <>
struct can_specify_certs<TinyGsmSim808> : false_type {};
template <>
struct can_load_certs<TinyGsmSim808> : false_type {};

template <>
struct has_ssl<TinyGsmUBLOX> : true_type {};
template <>
struct can_specify_certs<TinyGsmUBLOX> : false_type {};
template <>
struct can_load_certs<TinyGsmUBLOX> : false_type {};

template <>
struct has_ssl<TinyGsmSaraR4> : true_type {};
template <>
struct can_specify_certs<TinyGsmSaraR4> : false_type {};
template <>
struct can_load_certs<TinyGsmSaraR4> : false_type {};

template <>
struct has_ssl<TinyGsmSaraR5> : true_type {};
template <>
struct can_specify_certs<TinyGsmSaraR5> : false_type {};
template <>
struct can_load_certs<TinyGsmSaraR5> : false_type {};

template <>
struct has_ssl<TinyGsmSequansMonarch> : true_type {};
template <>
struct can_specify_certs<TinyGsmSequansMonarch> : false_type {};
template <>
struct can_load_certs<TinyGsmSequansMonarch> : false_type {};

template <>
struct has_ssl<TinyGsmESP8266NonOS> : true_type {};
template <>
struct can_specify_certs<TinyGsmESP8266NonOS> : false_type {};
template <>
struct can_load_certs<TinyGsmESP8266NonOS> : false_type {};

// Modems that inherit from TinyGsmSSL but can't load certificates
// (They have certificate functions but uploading is done externally)

template <>
struct can_load_certs<TinyGsmESP8266> : false_type {};

template <>
struct can_load_certs<TinyGsmXBee> : false_type {};


// customize the 800, which would otherwise report most traits as false because
// they're not directly inherited

template <>
struct has_cellular_data<TinyGsmSim808> : true_type {};
template <>
struct has_sms<TinyGsmSim808> : true_type {};
template <>
struct has_calling<TinyGsmSim808> : true_type {};
template <>
struct has_battery<TinyGsmSim808> : true_type {};
template <>
struct has_time<TinyGsmSim808> : true_type {};
template <>
struct has_ntp<TinyGsmSim808> : true_type {};
template <>
struct has_gsm_location<TinyGsmSim808> : true_type {};

}  // namespace TinyGsmCapabilities

#endif  // SRC_TINYGSMCAPABILITIES_H_
