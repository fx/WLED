#include "pin_manager.h"
#include "wled.h"

#ifdef ARDUINO_ARCH_ESP32
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 2, 0)
#include <soc/soc_caps.h> // WLEDMM
#endif
#endif

#ifdef WLED_DEBUG
static void DebugPrintOwnerTag(PinOwner tag)
{
  uint32_t q = static_cast<uint8_t>(tag);
  if (q) {
    DEBUG_PRINT(pinManager.getOwnerText(tag)); DEBUG_PRINT(F(" = ")); // WLEDMM
    DEBUG_PRINTF("0x%02x (%d)", q, q);
  } else {
    DEBUG_PRINT(F("(no owner)"));
  }
}
#endif

// WLEDMM begin

String PinManagerClass::getPinOwnerText(int gpio) {
  if ((gpio < 0) || (gpio == 0xFF)) return(F(""));
  //if (gpio >= GPIO_PIN_COUNT) return(F("n/a"));
  if (!isPinOk(gpio, false)) return(F("n/a"));
  if (!isPinAllocated(gpio)) return(F("./."));
  return(getOwnerText(getPinOwner(gpio)));
}

String PinManagerClass::getOwnerText(PinOwner tag) {
  switch(tag) {
    case PinOwner::None       : return(F("no owner")); break;       // unknown - no owner
    case PinOwner::DebugOut   : return(F("debug output")); break;   // 'Dbg'  == debug output always IO1
    case PinOwner::Ethernet   : return(F("Ethernet")); break;       // Ethernet
    case PinOwner::BusDigital : return(F("LEDs (digital)")); break; // Digital LEDs
    case PinOwner::BusPwm     : return(F("LEDs (PWM)")); break;     // PWM output using BusPwm
    case PinOwner::BusOnOff   : return(F("LEDs (on-off)")); break;  // 
    case PinOwner::Button     : return(F("Button")); break;         // 'Butn' == button from configuration
    case PinOwner::IR         : return(F("IR Receiver")); break;    // 'IR'   == IR receiver pin from configuration
    case PinOwner::Relay      : return(F("Relay")); break;          // 'Rly'  == Relay pin from configuration
    case PinOwner::SPI_RAM    : return(F("PSRAM")); break;          // 'SpiR' == SPI RAM (aka PSRAM)
    case PinOwner::DMX        : return(F("DMX out")); break;        // 'DMX'  == hard-coded to IO2
    case PinOwner::HW_I2C     : return(F("I2C (hw)")); break;            // 'I2C'  == hardware I2C pins (4&5 on ESP8266, 21&22 on ESP32)
    case PinOwner::HW_SPI     : return(F("SPI (hw)")); break;            // 'SPI'  == hardware (V)SPI pins (13,14&15 on ESP8266, 5,18&23 on ESP32)

    case PinOwner::UM_Audioreactive     : return(F("AudioReactive (UM)")); break;     // audioreative usermod - analog or digital audio input
    case PinOwner::UM_Temperature       : return(F("Temperature (UM)")); break;       // "usermod_temperature.h"
    case PinOwner::UM_PIR               : return(F("PIR (UM)")); break;               // "usermod_PIR_sensor_switch.h"
    case PinOwner::UM_FourLineDisplay   : return(F("4Line Display (UM)")); break;     // "usermod_v2_four_line_display.h -- May use "standard" HW_I2C pins
    case PinOwner::UM_RotaryEncoderUI   : return(F("Rotary Enc. (UM)")); break;       // "usermod_v2_rotary_encoder_ui.h"
    case PinOwner::UM_MultiRelay        : return(F("Multi Relay (UM)")); break;       // "usermod_multi_relay.h"
    case PinOwner::UM_AnimatedStaircase : return(F("Anim.Staircase (UM)")); break;    // "Animated_Staircase.h"
    case PinOwner::UM_RGBRotaryEncoder  : return(F("RGB Rotary Enc. (UM)")); break;   // "rgb-rotary-encoder.h"
    case PinOwner::UM_QuinLEDAnPenta    : return(F("QuinLEDAnPenta (UM)")); break;    // "quinled-an-penta.h"
    case PinOwner::UM_BME280            : return(F("BME280 (UM)")); break;            // "usermod_bme280.h" -- Uses "standard" HW_I2C pins
    case PinOwner::UM_BH1750            : return(F("BH1750 (UM)")); break;            // "usermod_bh1750.h" -- Uses "standard" HW_I2C pins
    case PinOwner::UM_SdCard            : return(F("SD-Card (UM)")); break;           // "usermod_sd_card.h" -- Uses SPI pins

    case PinOwner::UM_Example      : return(F("example (UM)")); break;            // unspecified usermod
    case PinOwner::UM_Unspecified  : return(F("usermod (UM)")); break;            // unspecified usermod
  }
  return(F("other")); // should not happen
}

String PinManagerClass::getPinSpecialText(int gpio) {  // special purpose PIN info
  if ((gpio == 0xFF) || (gpio < 0)) return(F(""));      // explicitly allow -1 as a no-op

#ifdef USERMOD_AUDIOREACTIVE
  // audioreactive settings - unfortunately, these are hiddden inside usermod now :-(
  // if((gpio == audioPin) && (dmType == 0)) return(F("analog audio in"));
  // if((gpio == i2ssdPin) && (dmType > 0)) return(F("I2S SD"));
  // if((gpio == i2swsPin) && (dmType > 0)) return(F("I2S WS"));
  // if((gpio == i2sckPin) && (dmType > 0) && (dmType != 5)) return(F("I2S SCK"));
  // if((gpio == mclkPin) && ((dmType == 2) || (dmType == 4))) return(F("I2S MCLK"));
  #ifdef I2S_SDPIN
    if (gpio == I2S_SDPIN) return(F("(default) I2S SD"));
  #endif
  #ifdef I2S_WSPIN
    if (gpio == I2S_WSPIN) return(F("(default) I2S WS"));
  #endif
  #ifdef I2S_CKPIN
    if (gpio == I2S_CKPIN) return(F("(default) I2S SCK"));
  #endif
  #ifdef MCLK_PIN
    if (gpio == MCLK_PIN) return(F("(default) I2S MCLK"));
  #endif
#endif

  // hardware special purpose PINS
  if (gpio == hardwareTX) return(F("Serial TX"));   // Serial (debug monitor) TX pin (usually GPIO1)
  if (gpio == hardwareRX) return(F("Serial RX"));   // Serial (debug monitor) RX pin (usually GPIO3)
  if ((gpio == i2c_sda)  || ((gpio == HW_PIN_SDA) && (i2c_sda < 0))) return(F("(default) I2C SDA"));
  if ((gpio == i2c_scl)  || ((gpio == HW_PIN_SCL) && (i2c_scl < 0))) return(F("(default) I2C SCL"));
  if ((gpio == spi_sclk) || ((gpio == HW_PIN_CLOCKSPI) && (spi_sclk < 0))) return(F("(default) SPI SLK  / SCK"));
  if ((gpio == spi_mosi) || ((gpio == HW_PIN_DATASPI) && (spi_mosi < 0)))  return(F("(default) SPI PICO / MOSI"));
  if ((gpio == spi_miso) || ((gpio == HW_PIN_MISOSPI) && (spi_miso < 0)))  return(F("(default) SPI POCI / MISO"));
#if defined(WLED_USE_SD_MMC) || defined(WLED_USE_SD_SPI) || defined(SD_ADAPTER)
  if ((gpio == HW_PIN_CSSPI)) return(F("(default) SPI SS"));  // no part of usermod default settings, currently only needed by SD_CARD usermod
#endif

  // MCU special PINS
  #ifdef ARDUINO_ARCH_ESP32
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
      // ESP32-S3
      if (gpio > 18 && gpio < 21) return (F("USB (CDC) / JTAG"));
      #if !defined(BOARD_HAS_PSRAM)
        if (gpio > 32 && gpio < 38)  return (F("(optional) Octal Flash or PSRAM"));
      #else
        if (gpio > 32 && gpio < 38)  return (F("(reserved) Octal PSRAM or Octal Flash"));
      #endif
      //if (gpio == 0 || gpio == 3 || gpio == 45 || gpio == 46) return (F("(strapping pin)"));

    #elif defined(CONFIG_IDF_TARGET_ESP32S2)
      // ESP32-S2
      if (gpio > 38 && gpio < 43) return (F("USB (CDC) / JTAG"));
      if (gpio == 46) return (F("pulled-down, input only"));
      //if (gpio == 0 || gpio == 45 || gpio == 46) return (F("(strapping pin)"));

    #elif defined(CONFIG_IDF_TARGET_ESP32C3)
      // ESP32-C3
      if (gpio > 17 && gpio < 20) return (F("USB (CDC) / JTAG"));
      //if (gpio == 2 || gpio == 8 || gpio == 9) return (F("(strapping pin)"));

    #else
      // "classic" ESP32, or ESP32 PICO-D4
      //if (gpio == 0 || gpio == 2 || gpio == 5) return (F("(strapping pin)"));
      //if (gpio == 12) return (F("(strapping pin - MTDI)"));
      //if (gpio == 15) return (F("(strapping pin - MTDO)"));
      //if (gpio > 11 && gpio < 16) return (F("(optional) JTAG debug probe"));
      #if defined(BOARD_HAS_PSRAM)
        if (gpio == 16 || gpio == 17) return (F("(reserved) PSRAM"));
      #endif
    #endif
  #else
    // ESP 8266
      if ((gpio == 0) || (gpio == 17)) return (F("analog-in (A0)"));  // 17 seems to be an alias for "A0" on 8266

  #endif

  // Arduino and WLED special PINS
  #if !defined(ARDUINO_ARCH_ESP32)  // these only make sense on 8266
    #if defined(LED_BUILTIN) || defined(BUILTIN_LED)
      if (gpio == LED_BUILTIN) return(F("(onboard LED)"));
    #endif
  #endif

  #ifdef LEDPIN
    if (gpio == LEDPIN) return(F("(default) LED pin"));
  #endif

  #if defined(BTNPIN)
    if (gpio == BTNPIN) return(F("(default) Button pin"));
  #endif
  #if defined(RLYPIN)
    if (gpio == RLYPIN) return(F("(default) Relay pin"));
  #endif
  #if !defined(WLED_DISABLE_INFRARED) && defined(IRPIN)
    if (gpio == IRPIN) return(F("(default) IR receiver pin"));
  #endif

  #ifdef WLED_ENABLE_DMX
    if (gpio == 2) return(F("hardcoded DMX output pin"));
  #endif
  #if defined(STATUSLED)
    if (gpio == STATUSLED) return(F("WLED Status LED"));
  #endif

  // Not-OK PINS
  if (!isPinOk(gpio, false)) return(F(""));

#if 0
  // analog pin infos - experimental !
  #ifdef ARDUINO_ARCH_ESP32
  // ADC PINs - not for 8266
  if (digitalPinToAnalogChannel(gpio) >= 0) {  // ADC pin
  #ifdef SOC_ADC_CHANNEL_NUM
    if (digitalPinToAnalogChannel(gpio) < SOC_ADC_CHANNEL_NUM(0)) return(F("ADC-1")); // for ESP32-S3, ESP32-S2, ESP32-C3 
  #else
    if (digitalPinToAnalogChannel(gpio) < 8) return(F("ADC-1"));   // for classic ESP32
  #endif
    else return(F("ADC-2"));
  } 
  #endif
#endif

  return(F("")); // default - nothing special to say
}

String PinManagerClass::getPinConflicts(int gpio) {
  if ((gpio == 0xFF) || (gpio < 0)) return(F(""));      // explicitly allow -1 as a no-op
  if (!isPinOk(gpio, false)) return(F(""));             // invalid GPIO

  if (ownerConflict[gpio] == PinOwner::None) {
    return(F(""));             // no conflict fot this GPIO
  } else {                     // found previous conflic!
    return String("!! Conflict with ") + getOwnerText(ownerConflict[gpio]) + String(" !!");
  }
}
// WLEDMM end

/// Actual allocation/deallocation routines
bool PinManagerClass::deallocatePin(byte gpio, PinOwner tag)
{
  if (gpio == 0xFF) return true;           // explicitly allow clients to free -1 as a no-op
  if (!isPinOk(gpio, false)) return false; // but return false for any other invalid pin

  // if a non-zero ownerTag, only allow de-allocation if the owner's tag is provided
  if ((ownerTag[gpio] != PinOwner::None) && (ownerTag[gpio] != tag)) {
    #ifdef WLED_DEBUG
    DEBUG_PRINT(F("PIN DEALLOC: IO "));
    DEBUG_PRINT(gpio);
    DEBUG_PRINT(F(" allocated by "));
    DebugPrintOwnerTag(ownerTag[gpio]);
    DEBUG_PRINT(F(", but attempted de-allocation by "));
    DebugPrintOwnerTag(tag);
    #endif
    return false;
  }
  
  byte by = gpio >> 3;
  byte bi = gpio - 8*by;
  bitWrite(pinAlloc[by], bi, false);
  ownerTag[gpio] = PinOwner::None;
  // ownerConflict[gpio] = PinOwner::None;  // WLEDMM clear conflict (if any)
  return true;
}

// support function for deallocating multiple pins
bool PinManagerClass::deallocateMultiplePins(const uint8_t *pinArray, byte arrayElementCount, PinOwner tag)
{
  bool shouldFail = false;
  DEBUG_PRINTLN(F("MULTIPIN DEALLOC"));
  // first verify the pins are OK and allocated by selected owner
  for (int i = 0; i < arrayElementCount; i++) {
    byte gpio = pinArray[i];
    if (gpio == 0xFF) {
      // explicit support for io -1 as a no-op (no allocation of pin),
      // as this can greatly simplify configuration arrays
      continue;
    }
    if (isPinAllocated(gpio, tag)) {
      // if the current pin is allocated by selected owner it is possible to release it
      continue;
    }
    #ifdef WLED_DEBUG
    DEBUG_PRINT(F("PIN DEALLOC: IO "));
    DEBUG_PRINT(gpio);
    DEBUG_PRINT(F(" allocated by "));
    DebugPrintOwnerTag(ownerTag[gpio]);
    DEBUG_PRINT(F(", but attempted de-allocation by "));
    DebugPrintOwnerTag(tag);
    #endif
    shouldFail = true;
  }
  if (shouldFail) {
    return false; // no pins deallocated
  }
  if (tag==PinOwner::HW_I2C) {
    if (i2cAllocCount && --i2cAllocCount>0) {
      // no deallocation done until last owner releases pins
      return true;
    }
  }
  if (tag==PinOwner::HW_SPI) {
    if (spiAllocCount && --spiAllocCount>0) {
      // no deallocation done until last owner releases pins
      return true;
    }
  }
  for (int i = 0; i < arrayElementCount; i++) {
    deallocatePin(pinArray[i], tag);
  }
  return true;
}

bool PinManagerClass::deallocateMultiplePins(const managed_pin_type * mptArray, byte arrayElementCount, PinOwner tag)
{
  uint8_t pins[arrayElementCount];
  for (int i=0; i<arrayElementCount; i++) pins[i] = mptArray[i].pin;
  return deallocateMultiplePins(pins, arrayElementCount, tag);
}

bool PinManagerClass::allocateMultiplePins(const managed_pin_type * mptArray, byte arrayElementCount, PinOwner tag)
{
  bool shouldFail = false;
  // first verify the pins are OK and not already allocated
  for (int i = 0; i < arrayElementCount; i++) {
    byte gpio = mptArray[i].pin;
    if (gpio == 0xFF) {
      // explicit support for io -1 as a no-op (no allocation of pin),
      // as this can greatly simplify configuration arrays
      continue;
    }
    if (!isPinOk(gpio, mptArray[i].isOutput)) {
      #ifdef WLED_DEBUG
      DEBUG_PRINT(F("PIN ALLOC: Invalid pin attempted to be allocated: GPIO "));
      DEBUG_PRINT(gpio);
      DEBUG_PRINT(" as "); DEBUG_PRINT(mptArray[i].isOutput ? "output": "input"); // WLEDMM
      DEBUG_PRINTLN(F(""));
      #else  // WLEDMM
      USER_PRINTF("PIN ALLOC: cannot use GPIO%d for %s.\n", gpio, mptArray[i].isOutput ? "output": "input");
      #endif
      if ((gpio < WLED_NUM_PINS) && (gpio >= 0) && (tag != PinOwner::None)) {
        ownerConflict[gpio] = tag; // WLEDMM record conflict
      }
      shouldFail = true;
    }
    if ((tag==PinOwner::HW_I2C || tag==PinOwner::HW_SPI) && isPinAllocated(gpio, tag)) {
      // allow multiple "allocations" of HW I2C & SPI bus pins
      continue;
    } else if (isPinAllocated(gpio)) {
      ownerConflict[gpio] = tag; // WLEDMM record conflict
      #ifdef WLED_DEBUG
      DEBUG_PRINT(F("PIN ALLOC: FAIL: IO ")); 
      DEBUG_PRINT(gpio);
      DEBUG_PRINT(F(" already allocated by "));
      DebugPrintOwnerTag(ownerTag[gpio]);
      DEBUG_PRINTLN(F(""));
      #else  // WLEDMM
      USER_PRINTF("PIN ALLOC: failed to assign GPIO%d to %s.\n", gpio, getOwnerText(tag).c_str());
      #endif
      shouldFail = true;
    }
  }
  if (shouldFail) {
    return false;
  }

  if (tag==PinOwner::HW_I2C) i2cAllocCount++;
  if (tag==PinOwner::HW_SPI) spiAllocCount++;

  // all pins are available .. track each one
  for (int i = 0; i < arrayElementCount; i++) {
    byte gpio = mptArray[i].pin;
    if (gpio == 0xFF) {
      // allow callers to include -1 value as non-requested pin
      // as this can greatly simplify configuration arrays
      continue;
    }
    if (gpio >= WLED_NUM_PINS) 
      continue; // WLEDMM - invalid GPIO => avoid array bounds violation

    byte by = gpio >> 3;
    byte bi = gpio - 8*by;
    bitWrite(pinAlloc[by], bi, true);
    ownerTag[gpio] = tag;
    // ownerConflict[gpio] = PinOwner::None; // WLEDMM clear conflict (if any)
    #ifdef WLED_DEBUG
    DEBUG_PRINT(F("PIN ALLOC: Pin ")); 
    DEBUG_PRINT(gpio);
    DEBUG_PRINT(F(" allocated by "));
    DebugPrintOwnerTag(tag);
    DEBUG_PRINTLN(F(""));
    #endif
  }
  return true;
}

bool PinManagerClass::allocatePin(byte gpio, bool output, PinOwner tag)
{
  // HW I2C & SPI pins have to be allocated using allocateMultiplePins variant since there is always SCL/SDA pair
  if (!isPinOk(gpio, output) || (gpio >= WLED_NUM_PINS) || tag==PinOwner::HW_I2C || tag==PinOwner::HW_SPI) { // WLEDMM bugfix - avoid array bounds violation
    #ifdef WLED_DEBUG
    if (gpio < 255) {  // 255 (-1) is the "not defined GPIO"
      if (!isPinOk(gpio, output)) {
        if ((gpio < WLED_NUM_PINS) && (gpio >= 0) && (tag != PinOwner::None)) {
          ownerConflict[gpio] = tag; // WLEDMM record conflict
        }
        DEBUG_PRINT(F("PIN ALLOC: FAIL for owner "));
        DebugPrintOwnerTag(tag);
        DEBUG_PRINT(F(": GPIO ")); DEBUG_PRINT(gpio);
        if (output) DEBUG_PRINTLN(F(" cannot be used for i/o on this MCU."));
        else DEBUG_PRINTLN(F(" cannot be used as input on this MCU."));
      } else {
        DEBUG_PRINT(F("PIN ALLOC: FAIL: GPIO ")); DEBUG_PRINT(gpio);
        DEBUG_PRINTLN(F(" - HW I2C & SPI pins have to be allocated using allocateMultiplePins()"));
      }
    }
    #else  // WLEDMM
      if (gpio < 255) { 
        USER_PRINTF("PIN ALLOC: cannot use GPIO%d for %s.\n", gpio, output ? "output": "input");
      }
    #endif
    return false;
  }
  if (isPinAllocated(gpio)) {
    ownerConflict[gpio] = tag; // WLEDMM record conflict
    #ifdef WLED_DEBUG
    DEBUG_PRINT(F("PIN ALLOC: Pin ")); 
    DEBUG_PRINT(gpio);
    DEBUG_PRINT(F(" already allocated by "));
    DebugPrintOwnerTag(ownerTag[gpio]);
    DEBUG_PRINTLN(F(""));
    #else  // WLEDMM
    USER_PRINTF("PIN ALLOC: failed to assign GPIO%d to %s.\n", gpio, getOwnerText(tag).c_str());
    #endif
    return false;
  }

  byte by = gpio >> 3;
  byte bi = gpio - 8*by;
  bitWrite(pinAlloc[by], bi, true);
  ownerTag[gpio] = tag;
  // ownerConflict[gpio] = PinOwner::None; // WLEDMM clear conflict (if any)
  #ifdef WLED_DEBUG
  DEBUG_PRINT(F("PIN ALLOC: Pin ")); 
  DEBUG_PRINT(gpio);
  DEBUG_PRINT(F(" successfully allocated by "));
  DebugPrintOwnerTag(tag);
  DEBUG_PRINTLN(F(""));
  #endif  

  return true;
}

// if tag is set to PinOwner::None, checks for ANY owner of the pin.
// if tag is set to any other value, checks if that tag is the current owner of the pin.
bool PinManagerClass::isPinAllocated(byte gpio, PinOwner tag)
{  
  if (!isPinOk(gpio, false)) return true;
  if (gpio == 0xFF) {
    DEBUG_PRINT(F(" isPinAllocated: -1 is never allocacted! ")); 
    return false; // WLEDMM bugfix - avoid invalid index to array
  }

  if ((tag != PinOwner::None) && (ownerTag[gpio] != tag)) {
    if ((ownerTag[gpio] != PinOwner::None) && (tag != PinOwner::HW_I2C) && (tag != PinOwner::HW_SPI)) ownerConflict[gpio] = tag; // WLEDMM record conflict
    return false;
  }
  byte by = gpio >> 3;
  byte bi = gpio - (by<<3);
  return bitRead(pinAlloc[by], bi);
}

/* see https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html
 * The ESP32-S3 chip features 45 physical GPIO pins (GPIO0 ~ GPIO21 and GPIO26 ~ GPIO48). Each pin can be used as a general-purpose I/O
 * Strapping pins: GPIO0, GPIO3, GPIO45 and GPIO46 are strapping pins. For more infomation, please refer to ESP32-S3 datasheet.
 * Serial TX = GPIO43, RX = GPIO44; LED BUILTIN is usually GPIO39
 * USB-JTAG: GPIO 19 and 20 are used by USB-JTAG by default. In order to use them as GPIOs, USB-JTAG will be disabled by the drivers.
 * SPI0/1: GPIO26-32 are usually used for SPI flash and PSRAM and not recommended for other uses. 
 * When using Octal Flash or Octal PSRAM or both, GPIO33~37 are connected to SPIIO4 ~ SPIIO7 and SPIDQS. Therefore, on boards embedded with ESP32-S3R8 / ESP32-S3R8V chip, GPIO33~37 are also not recommended for other uses.
 * 
 * see https://docs.espressif.com/projects/esp-idf/en/v4.4.2/esp32s3/api-reference/peripherals/adc.html
 *     https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/adc_oneshot.html
 * ADC1: GPIO1  - GPIO10 (channel 0..9)
 * ADC2: GPIO11 - GPIO20 (channel 0..9)
 * adc_power_acquire(): Please do not use the interrupt of GPIO36 and GPIO39 when using ADC or Wi-Fi and Bluetooth with sleep mode enabled. As a workaround, call adc_power_acquire() in the APP.
 * Since the ADC2 module is also used by the Wi-Fi, reading operation of adc2_get_raw() may fail between esp_wifi_start() and esp_wifi_stop(). Use the return code to see whether the reading is successful.
 */

// Check if supplied GPIO is ok to use
bool PinManagerClass::isPinOk(byte gpio, bool output)
{
#ifdef ESP32
  if (digitalPinIsValid(gpio)) {
  #if defined(CONFIG_IDF_TARGET_ESP32C3)
    // strapping pins: 2, 8, & 9
    if (gpio > 11 && gpio < 18) return false;     // 11-17 SPI FLASH
    if (gpio > 17 && gpio < 20) return false;     // 18-19 USB-JTAG
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    // 00 to 18 are for general use. Be careful about straping pins GPIO0 and GPIO3 - these may be pulled-up or pulled-down on your board.
    if (gpio > 18 && gpio < 21) return false;     // 19 + 20 = USB-JTAG. Not recommended for other uses.
    if (gpio > 21 && gpio < 33) return false;     // 22 to 32: not connected + SPI FLASH
    //if (gpio > 32 && gpio < 38) return false;     // 33 to 37: not available if using _octal_ SPI Flash or _octal_ PSRAM
    // 38 to 48 are for general use. Be careful about straping pins GPIO45 and GPIO46 - these may be pull-up or pulled-down on your board.
  #elif defined(CONFIG_IDF_TARGET_ESP32S2)
    // strapping pins: 0, 45 & 46
    if (gpio > 21 && gpio < 33) return false;     // 22 to 32: not connected + SPI FLASH
    // JTAG: GPIO39-42 are usually used for inline debugging
    // GPIO46 is input only and pulled down
  #else
    if (gpio > 5 && gpio < 12) return false;      //SPI flash pins
  #endif
    if (output) return digitalPinCanOutput(gpio);
    else        return true;
  }
#else
  if (gpio <  6) return true;
  if (gpio < 12) return false; //SPI flash pins
  if (gpio < 17) return true;
#endif
  return false;
}

PinOwner PinManagerClass::getPinOwner(byte gpio) {
  if (!isPinOk(gpio, false)) return PinOwner::None;
  return ownerTag[gpio];
}

#ifdef ARDUINO_ARCH_ESP32
#if defined(CONFIG_IDF_TARGET_ESP32C3)
  #define MAX_LED_CHANNELS 6
#else
  #if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
    #define MAX_LED_CHANNELS 8
  #else
    #define MAX_LED_CHANNELS 16
  #endif
#endif
byte PinManagerClass::allocateLedc(byte channels)
{
  if (channels > MAX_LED_CHANNELS || channels == 0) return 255;
  byte ca = 0;
  for (byte i = 0; i < MAX_LED_CHANNELS; i++) {
    byte by = i >> 3;
    byte bi = i - 8*by;
    if (bitRead(ledcAlloc[by], bi)) { //found occupied pin
      ca = 0;
    } else {
      ca++;
    }
    if (ca >= channels) { //enough free channels
      byte in = (i + 1) - ca;
      for (byte j = 0; j < ca; j++) {
        byte b = in + j;
        byte by = b >> 3;
        byte bi = b - 8*by;
        bitWrite(ledcAlloc[by], bi, true);
      }
      return in;
    }
  }
  return 255; //not enough consecutive free LEDC channels
}

void PinManagerClass::deallocateLedc(byte pos, byte channels)
{
  for (byte j = pos; j < pos + channels; j++) {
    if (j > MAX_LED_CHANNELS) return;
    byte by = j >> 3;
    byte bi = j - 8*by;
    bitWrite(ledcAlloc[by], bi, false);
  }
}
#endif

PinManagerClass pinManager = PinManagerClass();
