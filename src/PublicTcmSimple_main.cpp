/*
// DESCRIPTION ********************************************************************************************************
//
// PublicSimple_main.cpp
// IDE: PIO.
// TARGET(s)	: VARIOUS
//
// WIFI : Available on ESP32 targets.
//
// UI : TCMENU BY DAVE CHERRY et.al.
//
//	SUMMARY: 
//		Simple tcMenu example available to run on  several target boards
//		e.g. esp32-wroom, esp32-s3, mega2560. Mainly Lilygo esp32 boards. Refer to the following repos:
//		https://github.com/AtmosphEng/PublicTcmSimple.git
//		https://github.com/AtmosphEng/boards.git
//
//	FEATURES:
//		tcMenu local user input via rotary encoder, or 2-buttons.
//		tcMenu local user output on LCD.
//		tcMenu remote UI via esp32 wifi
//		tcMenu remote UI via 3-wire SerialX port
//		ARDUINOOTA
//
//
// KNOWN ISSUES: 
//
//  TARGET(s): lilygo-t7-s3
//		T7-S3 ESP32-S3 USB CDC mode for Serial : -D ARDUINO_USB_MODE=1 : -D ARDUINO_USB_CDC_ON_BOOT=1 :
//		PIO Serial Terminal does not show any Serial.print but RealTerm is OK
//		(apart from T7-S3 h/w reset causing Windows temp removal of the corrsponding COMx port).
//
//	TARGET(s): lilygo-t-display-s3
//		Fix for blank TFT_eSPI display - check the following 2x custom PIO project files
//   		1) PROJDIR/.pio/libdeps/lilygo-t-display-s3/TFT_eSPI # for custom: User_Setup_Select.h
//   		2) PROJDIR/.pio/libdeps/lilygo-t-display-s3/TFT_eSPI/User_Setups for custom: Setup206_LilyGo_T_Display_S3.h
//
//		T-Display-S3 Compile Warning :
//		Compiling .pio\build\lilygo-t-display-s3\src\PublicTcmSimple_menu.cpp.o
//		In file included from .pio/libdeps/lilygo-t-display-s3/TFT_eSPI/TFT_eSPI.h:96,
//	             from src/tcMenuTfteSpi.h:17,
//               from src/PublicTcmSimple_menu.h:16,
//               from src/src_menu.h:4,
//               from src/PublicTcmSimple_main.cpp:1:
//		.pio/libdeps/lilygo-t-display-s3/TFT_eSPI/Processors/TFT_eSPI_ESP32_S3.h:110:3: warning:
//		#warning >>>>------>> DMA is not supported in parallel mode [-Wcpp]
//		#warning >>>>------>> DMA is not supported in parallel mode
//
//		Compile Warning : lilygo-t-display-s3 AND lilygo-t-embed-s3
//		In file included from .pio/libdeps/lilygo-t-display-s3/TFT_eSPI/TFT_eSPI.cpp:16:
//		.pio/libdeps/lilygo-t-display-s3/TFT_eSPI/TFT_eSPI.h:970:8: warning: #warning >>>>------>>
//		TOUCH_CS pin not defined, TFT_eSPI touch functions will not be available! [-Wcpp]
//		#warning >>>>------>> TOUCH_CS pin not defined, TFT_eSPI touch functions will not be available!
//
//	TARGET(s): lilygo-t-embed-s3
//		The t-embed serial port on GROVE conn. is NOT currently working w. embedCONTROL. However, (USB) Serial is OK.
//
//	TARGET(s): esp32dev_t_internet_com
//		tcMenu embedCONTROL: all ports working (Serial, Serial1, Serial2, Wifi) except wired Ethernet.
//
//	TARGET(s): ALL esp32-s3
//		TCMENU RC w. embedCONTROL not working on Serial with USB CDC mode e.g. 1x USB cable only for prog and debug.
//		platformio.ini: build_flags: -D ARDUINO_USB_MODE=1 
//		platformio.ini: build_flags: -D ARDUINO_USB_CDC_ON_BOOT=1 
//		WORKAROUND : 2024-04-03 - 2024-10-10
//		1) pin_config-t-display-esp32-s3.h : add : #define	MYSERIAL_BEGIN Serial.begin(BAUD_SERIAL, SERIAL_8N1, 43, 44);
//		2) PublicTcmSimple_menu.cpp:25 : add cast : SerialTagValueTransport serialTransport( (HardwareSerial*) &Serial);
//		3) Use Realterm (optional) to check TCMENU RC HMI heartbeat messages on Serial at 115200 8-n-1 baud settings.
//		4) Close Realterm (optional), reset target, run embedCONTROL and select Serial port.
//
// DESCRIPTION ********************************************************************************************************
*/

#include <Arduino.h>
#include <SPI.h>

#include "src_menu.h" // for tcMenu

#ifdef INIDEF_ETHERMEGA2560
#include "Ethernet.h"
#endif

#ifdef INIDEF_LILYGO_T_INTERNET_COM
#include <ETH.h>
#endif

#ifndef INIDEF_MEGA2560
#include "..\..\Credentials\Credentials.h"
#endif

#ifdef INIDEF_MEGA2560
#include "pin_config-avr-mega2560.h" // SPI data LCD interface
#endif

#ifdef INIDEF_ETHERMEGA2560
#include "pin_config-avr-ethermega2560.h" // SPI data LCD interface
#endif

#if(0) // AAAFIXME
#ifdef INIDEF_KEYESTUDIO_KS0413
#include "pin_config-keyestudio-ESP32.h" // SPI data LCD interface
#endif
#else
// #define PIN_DEBUG_LED 19
#endif

#ifdef INIDEF_LILYGO_T_INTERNET_COM
#include "pin_config-t-internet-com-ESP32.h"

#include <Adafruit_NeoPixel.h>
#define LED_COUNT			 1
#define LED_BRIGHTNESS 1 // about 1/5 brightness (max = 255)
Adafruit_NeoPixel pixels(LED_COUNT, PIN_LED_NEOPIXEL);
uint32_t colors[] = {0x000000, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF}; // Off, Red, Green, Blue, white
#define NUM_COLORS (sizeof colors / sizeof colors[0])
#endif

#ifdef INIDEF_LILYGO_T_DISPLAY_S3
#include "pin_config-t-display-esp32-s3.h" // 8-bit parallel data LCD interface
#endif

#ifdef INIDEF_LILYGO_T_EMBED_S3
#include "pin_config-t-embed-esp32-s3.h" // SPI data LCD interface

#include "APA102.h"
APA102<PIN_APA102_DI, PIN_APA102_CLK> ledStrip; // t-embed builtin RGB_LED
#endif

#ifdef INIDEF_LILYGO_T7_S3
#include "pin_config-t7-s3-esp32-s3.h"
#endif

#define MYSERIALX Serial1 // myDebug

#ifdef INIDEF_ARDUINOOTA
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#endif

#ifdef ARDUINO_ARCH_AVR
#define PIN_DUMMY_GROUND_D20	20 // For Mega wiring convenience of Serial1 cable.
#define PIN_DUMMY_GROUND_D21	21 // For Mega wiring convenience of Serial2 cable.

// encoder2 - for direct control of BaseTCW
#define PIN_ENCODER2_CLK_D22	22
#define PIN_ENCODER2_DAT_D24	24
// encoder2 pushbutton on D26 is not used here.
#define PIN_DUMMY_5V0_D28			28 // For Mega wiring convenience of rotary encoder.
#define PIN_DUMMY_GROUND_D30	30 // For Mega wiring convenience of rotary encoder.

// encoder1 - for direct control of BaseCCW
#define PIN_ENCODER1_CLK_D38	38
#define PIN_ENCODER1_DAT_D40	40
// encoder1 pushbutton on D42 is not used here.
#define PIN_DUMMY_5V0_D44			44 // For Mega wiring convenience of rotary encoder.
#define PIN_DUMMY_GROUND_D46	46 // For Mega wiring convenience of rotary encoder.

// ANALOG PINS ************************************************************************************
// A0 used for LCD 16x2 shield for polling / reading all operational buttons via resistor network.
// A5 used for LCD 16x2 shield (AAA Freetronics pcb mod to relocate D9 to A5)

// encoder0 - slot 0 - for menu navigation
#define PIN_DUMMY_GROUND_A11	A11 // For Mega wiring convenience of rotary encoder.
#define PIN_DUMMY_5V0_A12			A12 // For Mega wiring convenience of rotary encoder.
//#define PIN_ENCODER0_PBSW_A13 A13 // Assigned in tcmenu Designer Code Generator.
//#define PIN_ENCODER0_CLK_A14	A14 // Assigned in tcmenu Designer Code Generator.
//#define PIN_ENCODER0_DAT_A15	A15 // Assigned in tcmenu Designer Code Generator.
#endif														// ARDUINO_ARCH_AVR

#define BAUD_SERIAL  (115200)
#define BAUD_SERIAL0 (115200)
#define BAUD_SERIAL1 (115200)
#define BAUD_SERIAL2 (115200)

#define DEF_LED_DELAY 		500
#define DEF_SERIAL_DELAY	100


int myCount1 = 0;
int myCount2 = 0;

// *** FUNCTION(S) **********************************************
void refreshMenu(void) {

	menuTcmCount1.setSendRemoteNeededAll();
	menuTcmCount2.setSendRemoteNeededAll();
	menuTcmBaseTCW.setSendRemoteNeededAll();
	menuTcmBaseCCW.setSendRemoteNeededAll();
	menuTcmDebugLED.setSendRemoteNeededAll();
}

#ifdef INIDEF_LILYGO_T_EMBED_S3
/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. */
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}
#endif


void debugLED(bool state) {

#if(0) // from github lilygo t-embed examples/led/led.ino 
  const uint8_t ledSort[7] = {2, 1, 0, 6, 5, 4, 3};
  // Set the number of LEDs to control.
  const uint16_t ledCount = 7;
  
	for(int i = 0; i < 1000; i++) {
	// Create a buffer for holding the colors (3 bytes per color).
  rgb_color colors[ledCount];
  // Set the brightness to use (the maximum is 31).
  uint8_t brightness = 1;
  static uint64_t time;
  time++;
  for (uint16_t i = 0; i < ledCount; i++) {
    colors[i] = hsvToRgb((uint32_t)time * 359 / 256, 255, 255);
  }
  ledStrip.write(colors, ledCount, brightness);
  delay(10);
	}
#endif

#ifdef INIDEF_LILYGO_T_EMBED_S3 // APA102 RGBLEDs
  const uint16_t ledCount = 1;
	// Create a buffer for holding the colors (3 bytes per color).
  rgb_color colors[ledCount];
	if(state){
  	colors[0] = {64, 64, 64};
	}
	else {
	  colors[0] = {0, 0 ,0};
	}
  // Set the brightness to use (the maximum is 31).
  uint8_t brightness = 1;
	ledStrip.write(colors, ledCount, brightness);

#else

#ifdef INIDEF_LILYGO_T_INTERNET_COM // NeoPixel RGBLED

	if(state) {
		pixels.setPixelColor(0, colors[3]); // blue (on).
	}
	else {
		pixels.setPixelColor(0, colors[0]); // black (off).
	}
	pixels.show();

#else
	digitalWrite(PIN_DEBUG_LED, state); // Menu selection default toggles this boolean value.
#endif

#endif

}


#ifdef INIDEF_LILYGO_T_INTERNET_COM
#define TCP_SERVER_PORT     3333 // For tcmenu embedControl.

//WiFiServer myServer(TCP_SERVER_PORT);
static bool eth_connected = false;


void WiFiEvent(WiFiEvent_t event)
{
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        MYSERIALX.println("ETH Started");
        //set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        MYSERIALX.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        MYSERIALX.print("ETH MAC: ");
        MYSERIALX.print(ETH.macAddress());
        MYSERIALX.print(", IPv4: ");
        MYSERIALX.print(ETH.localIP());
        if (ETH.fullDuplex()) {
            MYSERIALX.print(", FULL_DUPLEX");
        }
        MYSERIALX.print(", ");
        MYSERIALX.print(ETH.linkSpeed());
        MYSERIALX.println("Mbps");
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        MYSERIALX.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        MYSERIALX.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

#endif


//
void setup() {

	Serial.begin(BAUD_SERIAL1);
	delay(DEF_SERIAL_DELAY); // Need time here?

#if(0)
	int incomingByte = 0; // For dumping unwanted initial ESP serial boot message.
	Serial0.begin(BAUD_SERIAL0);
	delay(100); // Need time here?
	if (Serial0.available() > 0) {
  // read and DUMP the incoming byte:
  incomingByte = Serial.read();
	}
#endif

//	Serial.println("Serial begin ok. line 1");
//	Serial.println("Serial begin ok. line 2");

#ifdef MYSERIAL0_BEGIN
	MYSERIAL0_BEGIN;
	delay(DEF_SERIAL_DELAY); // Need time here?
#endif

#ifdef MYSERIAL1_BEGIN
	MYSERIAL1_BEGIN;
	delay(DEF_SERIAL_DELAY); // Need time here?
#endif

#ifdef MYSERIAL2_BEGIN
	MYSERIAL2_BEGIN;
	delay(DEF_SERIAL_DELAY); // Need time here?
#endif

#ifdef MYSERIAL3_BEGIN
	MYSERIAL3_BEGIN;
	delay(DEF_SERIAL_DELAY); // Need time here?
#endif


#ifdef INIDEF_LILYGO_T_EMBED_S3
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);
#endif

#define ALPHALIMA_DELAY_MS 2000

#ifdef INIDEF_WIFI_STA
	WiFi.mode(WIFI_STA);
	WiFi.config(ip_static, ip_gway, netmask);
	WiFi.begin(ssid, pw);
	delay(ALPHALIMA_DELAY_MS); // THIS DELAY IS VERY IMPORTANT : comment from AlphaLima www.LK8000.com ?

#if(0)
	Serial.print("ESP Board MAC Address:  "); // 2024-04-05 testing for wifi on t-display-s3
  Serial.println(WiFi.macAddress());
#endif

#endif

#ifdef INIDEF_WIFI_AP // ESP32 WIFI ACCESS POINT
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, pw); // configure ssid and password for softAP
	delay(ALPHALIMA_DELAY_MS); // THIS DELAY IS VERY IMPORTANT : comment from AlphaLima www.LK8000.com ?
	WiFi.softAPConfig(ip_static, ip_gway, netmask); // IP_AP, IP_GATEWAY, MASK. configure ip address for softAP
#endif

#ifdef INIDEF_LILYGO_T_INTERNET_COM
																		// Neopixel WS2812 RGB LED init.
	pixels.begin();										// INITIALIZE NeoPixel strip object (REQUIRED)
	pixels.show();										// Turn OFF all pixels ASAP
	pixels.setBrightness(LED_BRIGHTNESS);
#else
	pinMode(PIN_DEBUG_LED, OUTPUT); // Common LED name for all targets / controllers. Assign I/O mode to pin.
#endif

	debugLED(true); // simple setup routine to provide a double flash of debugLED type installed on target board.
	delay(DEF_LED_DELAY);	
	debugLED(false);
	delay(DEF_LED_DELAY);	
	debugLED(true);
	delay(DEF_LED_DELAY);	
	debugLED(false);
	delay(DEF_LED_DELAY);	

#ifdef INIDEF_LILYGO_T_DISPLAY_S3
	pinMode(PIN_POWER_ON, OUTPUT);
	digitalWrite(PIN_POWER_ON, HIGH);

	pinMode(PIN_LCD_BL, OUTPUT);
	digitalWrite(PIN_LCD_BL, HIGH);
#endif

	setupMenu(); // for tcMenu

#ifdef INIDEF_LILYGO_T_EMBED_S3
	renderer.turnOffResetLogic(); // Turn off tcmenu cursor reset interval to prevent reset to root position.
#endif

#ifdef ARDUINO_ARCH_AVR
	//digitalWrite(IO_PIN_53, OUTPUT); // Mega Slave Select (SS)

	digitalWrite(PIN_DUMMY_GROUND_D20, LOW); // For wiring convenience on Mega for Serial1 cable with adjacent ground
	pinMode(PIN_DUMMY_GROUND_D20, OUTPUT);
	digitalWrite(PIN_DUMMY_GROUND_D20, LOW); // Belt and braces.

	digitalWrite(PIN_DUMMY_GROUND_D21, LOW); // For wiring convenience on Mega for Serial2 cable with adjacent ground
	pinMode(PIN_DUMMY_GROUND_D21, OUTPUT);
	digitalWrite(PIN_DUMMY_GROUND_D21, LOW); // Belt and braces.

		// first encoder - enc1 - encoder0 - slot 0 - for menu navigation
	digitalWrite(PIN_DUMMY_GROUND_A11, LOW); // For wiring convenience on Mega for rotary enc power 0V on 5-pin header
	pinMode(PIN_DUMMY_GROUND_A11, OUTPUT);
	digitalWrite(PIN_DUMMY_GROUND_A11, LOW); // Belt and braces.

	digitalWrite(PIN_DUMMY_5V0_A12, HIGH); // For wiring convenience on Mega for rotary enc power 5V on 5-pin header
	pinMode(PIN_DUMMY_5V0_A12, OUTPUT);
	digitalWrite(PIN_DUMMY_5V0_A12, HIGH); // Belt and braces.
	
#if (0)
	// for mega2560 hardware. ease of use via consecutive pin assignments on the connector(s).

	// first encoder - enc1 - encoder0 - slot 0 - for menu navigation
	digitalWrite(PIN_DUMMY_5V0_A12, HIGH); // For wiring convenience on Mega for rotary encoder VCC 5V0 on 5-pin header
	pinMode(PIN_DUMMY_5V0_A12, OUTPUT);
	digitalWrite(PIN_DUMMY_5V0_A12, HIGH); // Belt and braces.

	digitalWrite(PIN_DUMMY_GROUND_A11, LOW); // For wiring convenience on Mega for rotary encoder ground on 5-pin header
	pinMode(PIN_DUMMY_GROUND_A11, OUTPUT);
	digitalWrite(PIN_DUMMY_GROUND_A11, LOW); // Belt and braces.

	// second encoder - enc2 - encoder1
	digitalWrite(PIN_DUMMY_5V0_D44, HIGH); // For wiring convenience on Mega for rotary encoder VCC 5V0 on 5-pin header
	pinMode(PIN_DUMMY_5V0_D44, OUTPUT);
	digitalWrite(PIN_DUMMY_5V0_D44, HIGH); // Belt and braces.

	digitalWrite(PIN_DUMMY_GROUND_D46, LOW); // For wiring convenience on Mega for rotary encoder ground on 5-pin header
	pinMode(PIN_DUMMY_GROUND_D46, OUTPUT);
	digitalWrite(PIN_DUMMY_GROUND_D46, LOW); // Belt and braces.

	// third encoder - enc3 - encoder2
	digitalWrite(PIN_DUMMY_5V0_D28, HIGH); // For wiring convenience on Mega for rotary encoder VCC 5V0 on 5-pin header
	pinMode(PIN_DUMMY_5V0_D28, OUTPUT);
	digitalWrite(PIN_DUMMY_5V0_D28, HIGH); // Belt and braces.

	digitalWrite(PIN_DUMMY_GROUND_D30, LOW); // For wiring convenience on Mega for rotary encoder ground on 5-pin header
	pinMode(PIN_DUMMY_GROUND_D30, OUTPUT);
	digitalWrite(PIN_DUMMY_GROUND_D30, LOW); // Belt and braces.
#endif

#ifdef UI_ROTARY_ENC
	// enc1 is index 0. enc2 is index 1, etc.
	rotaryEncoder1 = new HardwareRotaryEncoder(PIN_ENCODER2_DAT_D40, PIN_ENCODER2_CLK_D38, [](int encoderValue) {
		menuTcmBaseCCW.setCurrentValue(encoderValue);
	});

	rotaryEncoder1->changePrecision(menuTcmBaseCCW.getMaximumValue(), menuTcmBaseCCW.getCurrentValue()); // max, curr
	switches.setEncoder(1, rotaryEncoder1);

	rotaryEncoder2 = new HardwareRotaryEncoder(PIN_ENCODER3_DAT_D24, PIN_ENCODER3_CLK_D22, [](int encoderValue) {
		menuTcmBaseTCW.setCurrentValue(encoderValue);
	});

	rotaryEncoder2->changePrecision(menuTcmBaseTCW.getMaximumValue(),
																	menuTcmBaseTCW.getCurrentValue()); // max value, current value
	switches.setEncoder(2, rotaryEncoder2);
#endif

	myCount1 = 1;
	menuTcmCount1.setCurrentValue(myCount1);

	taskManager.scheduleFixedRate(1000, [] {						// ms
		myCount2 = (menuTcmCount1.getCurrentValue()) * 2; // Get a menu value and transform it.
		menuTcmCount2.setCurrentValue(myCount2);
	});

#endif // ARDUINO_ARCH_AVR

#if(0)
#define DEF_TCM_SERIAL_XOVER_SYNC_ONCE
// #define DEF_TCM_SERIAL_XOVER_SYNC_REP

#ifdef DEF_TCM_SERIAL_XOVER_SYNC_ONCE
	refreshMenu();
#endif
#endif


	taskManager.scheduleFixedRate(1000, [] { // ms. Simple way to keep XoverEmbedControl synced after schedule delay.


#ifdef DEF_TCM_SERIAL_XOVER_SYNC_REP			 // For simple sync, these need to to be scheduled repeatedly.
		menuTcmCount1.setSendRemoteNeededAll();
		menuTcmCount2.setSendRemoteNeededAll();
		menuTcmBaseTCW.setSendRemoteNeededAll();
		menuTcmBaseCCW.setSendRemoteNeededAll();
		menuTcmDebugLED.setSendRemoteNeededAll();
#endif
		menuTcmTimeSec.setCurrentValue(menuTcmTimeSec.getCurrentValue() + 1); // Long way to avoid another variable.
	});

#if defined(INIDEF_ETHERMEGA2560)
  // You can use Ethernet.init(pin) to configure the CS pin

  Ethernet.init(PIN_ETHERNET_CS);  // Most Arduino shields

  //Ethernet.init(5);   // MKR ETH shield

  //Ethernet.init(0);   // Teensy 2.0

  //Ethernet.init(20);  // Teensy++ 2.0

  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet

  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // initialize the ethernet device

  //Ethernet.begin(myMAC, myETHWiredStaticIP, myDNS, myGateway, mySubnet);
  Ethernet.begin(myMAC, myETHWiredStaticIP);


  // Check for Ethernet hardware present

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {

    MYSERIALX.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");

    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }

  }

  if (Ethernet.linkStatus() == LinkOFF) {
    MYSERIALX.println("Ethernet cable is not connected.");
  }
  // start listening for clients

  MYSERIALX.println(Ethernet.localIP());
#endif

#ifdef INIDEF_LILYGO_T_INTERNET_COM
  WiFi.onEvent(WiFiEvent);

  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN,
            ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

  if (ETH.config(myETHWiredStaticIP, myGateway, mySubnet, myDNS, myDNS) == false) {
    MYSERIALX.println("Configuration FAILED.");
  }
	else {
	  MYSERIALX.println("Configuration passed.");
	}
#endif
  
  //myServer.begin();

  //MYSERIALX.print("Chat myServer address:");

#ifdef INIDEF_ETHERMEGA2560
  MYSERIALX.println(Ethernet.localIP());
#endif


#ifdef INIDEF_ARDUINOOTA
	// Port defaults to 3232
	// ArduinoOTA.setPort(3232);

	// Hostname defaults to esp3232-[MAC]
	// ArduinoOTA.setHostname("myesp32");

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA
			.onStart([]() {
				String type;
				if (ArduinoOTA.getCommand() == U_FLASH)
					type = "sketch";
				else // U_SPIFFS
					type = "filesystem";

				// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
				MYSERIALX.println("Start updating " + type);
			})
			.onEnd([]() { MYSERIALX.println("\nEnd"); })
			.onProgress([](unsigned int progress, unsigned int total) {
				MYSERIALX.printf("Progress: %u%%\r", (progress / (total / 100)));
			})
			.onError([](ota_error_t error) {
				MYSERIALX.printf("Error[%u]: ", error);
				if (error == OTA_AUTH_ERROR)
					MYSERIALX.println("Auth Failed");
				else if (error == OTA_BEGIN_ERROR)
					MYSERIALX.println("Begin Failed");
				else if (error == OTA_CONNECT_ERROR)
					MYSERIALX.println("Connect Failed");
				else if (error == OTA_RECEIVE_ERROR)
					MYSERIALX.println("Receive Failed");
				else if (error == OTA_END_ERROR)
					MYSERIALX.println("End Failed");
			});

	ArduinoOTA.begin();
#endif

} // setup)(


//
void loop() {
	taskManager.runLoop();

#ifdef INIDEF_ARDUINOOTA
	ArduinoOTA.handle();
#endif
} // loop
//

void CALLBACK_FUNCTION onChangeTcmCount1(int id) {
	myCount1++;
}

void CALLBACK_FUNCTION onChangeTcmCount2(int id) {
	// myCount2++;
}

void CALLBACK_FUNCTION onChangeTcmBaseCCW(int id) {
}

void CALLBACK_FUNCTION onChangeTcmBaseTCW(int id) {
}

void CALLBACK_FUNCTION onChangeTcmDebugLED(int id) {
	debugLED(menuTcmDebugLED.getBoolean());
}


void CALLBACK_FUNCTION onChangeTcmTimeSec(int id) {
}

void CALLBACK_FUNCTION onChangeTcmRefreshMenu(int id) { refreshMenu(); }


void (* resetFunc) (void) = 0; // Arduino Forum alto777

void CALLBACK_FUNCTION onChangeTcmRestart(int id) {

//#ifndef INIDEF_ETHERMEGA2560
//	ESP.restart();
//#else
	resetFunc();
//#endif

}


#ifdef INIDEF_LILYGO_T_INTERNET_COM

void CALLBACK_FUNCTION onChangeTcmNeoPixelLedOff(int id) {
	// pixels.setPixelColor(0,0); // Equivalent to the following line.
	pixels.setPixelColor(0, colors[0]); // black (off).
	pixels.show();
}

void CALLBACK_FUNCTION onChangeTcmNeoPixelLedRed(int id) {
	pixels.setPixelColor(0, colors[1]); // red.
	pixels.show();
}

void CALLBACK_FUNCTION onChangeTcmNeoPixelLedGreen(int id) {
	pixels.setPixelColor(0, colors[2]); // green.
	pixels.show();
}

void CALLBACK_FUNCTION onChangeTcmNeoPixelLedBlue(int id) {
	pixels.setPixelColor(0, colors[3]); // blue.
	pixels.show();
}

void CALLBACK_FUNCTION onChangeTcmNeoPixelLedWhite(int id) {
	pixels.setPixelColor(0, colors[4]); // white.
	pixels.show();
}

#endif

//
// END_OF_FILE
//



void CALLBACK_FUNCTION onChangeTcmNPixLedWhite(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onChangeTcmNPixLedRed(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onChangeTcmNPixLedOff(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onChangeTcmNPixLedGreen(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onChangeTcmNPixLedBlue(int id) {
    // TODO - your menu change code
}
