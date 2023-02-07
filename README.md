# ESP32-DMX-Control

A simple DMX controller made with an ESP32 and a MAX485 chip. The ESP32 is connected to the MAX485 via UART and the MAX485 is connected to the DMX bus. The ESP32 is connected to a WiFi network and can be controlled via a web interface.

## Hardware

-   ESP32-WROOM-32
-   MAX485 TTL to RS485 converter
-   (DMX512 3-pin XLR connector)
-   (10uF capasitor between EN and GND to boot the ESP32 without pressing the boot button)

![ESP32-DMX-Control Hardware setup](DMX-control-hardware.png)

## Software

The software is written in C++ and uses the Arduino framework. The ESP32 is programmed with PlatformIO.

## Web interface

The web interface is made with the ESPAsyncWebServer library. The web interface is served from the ESP32 and can be accessed via the IP address of the ESP32.

## DMX interface

The code is made to be used with a DMX-light with 3 channels. The first channel is used to control the red color, the second channel is used to control the green color, the third channel is used to control the blue color. The code can be easily changed to work with other DMX-lights.

## Usage

The ESP32 can be programmed with PlatformIO, and the code can be uploaded to the ESP32 via USB. Usage of the web interface requires a WiFi network. WiFi-network SSID and password have to be set in a separate file called `credentials.h`. The file should be placed in the `src` folder and should contain the following lines:

    #define WIFI_SSID "your_wifi_ssid"
    #define WIFI_PASSWORD "your_wifi_password"

The web interface's source files are located in the `data` folder. The files have to be uploaded to the ESP32's SPIFFS filesystem. This can be done with the PlatformIO "Upload Filesystem Image" command.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
