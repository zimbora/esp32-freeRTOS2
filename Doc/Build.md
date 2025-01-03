# Build

## Setup

### arduino-cli
```
brew update
brew install arduino-cli
```

### setup configuration file
arduino-cli config init
Config file written: /home/username/.arduino15/arduino-cli.yaml

### Documentation for file: https://arduino.github.io/arduino-cli/0.19/configuration/

### if using a 3rd party board, add to yaml file
board_manager:
  additional_urls:
    - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli board listall
arduino-cli core install esp32:esp32
Platform esp32:esp32@2.0.2 already installed

## compile

project=esp32-freeRTOS2
app=DEMO

### with destination
arduino-cli compile -b esp32:esp32:esp32 \
--build-property build.partitions=min_spiffs \
--build-property upload.maximum_size=1966080  \
--build-path ./build/${app} . 2>&1 | tee compile_logs.txt

### without destination
arduino-cli compile -b esp32:esp32:esp32 \
--build-property build.partitions=min_spiffs \
--build-property upload.maximum_size=1966080  \
. 2>&1 | tee compile_logs.txt


filenames=$( find build/${app}/${project}* )
mkdir -p images/${app}
cp ${filenames} images/${app}/

## upload

app=test
port=cu.usbserial-1140

### arduino cli
arduino-cli upload -p /dev/${port} --fqbn esp32:esp32:esp32 -b115200 esp32-freeRTOS2.ino

### esptool
esptool.py --port /dev/${port} write_flash --flash_mode dio --flash_size 4MB 0x0000 ./build/${app}/esp32-freeRTOS2.ino.bootloader.bin

esptool.py --port /dev/${port} write_flash --flash_mode dio --flash_size 4MB 0x10000 ./build/${app}/esp32-freeRTOS2.ino.bin

## Report

Used platform Version Path
esp32:esp32   2.0.11  ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.11


## libraries

Used library      Version Path
WiFi              2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/WiFi
Update            2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/Update
ArduinoOTA        2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/ArduinoOTA
WebServer         2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/WebServer
ESPmDNS           2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/ESPmDNS
HTTPClient        2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/HTTPClient
WiFiClientSecure  2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/WiFiClientSecure
FS                2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/FS
ESP32 BLE Arduino 2.0.0   ~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries/BLE
ArduinoJson       6.19.4  ~/Documents/Arduino/libraries/ArduinoJson
ESP32Logger       1.0.2   ~/Documents/Arduino/libraries/ESP32Logger
EspMQTTClient     1.13.3  ~/Documents/Arduino/libraries/EspMQTTClient
PubSubClient      2.8     ~/Documents/Arduino/libraries/PubSubClient
LittleFS_esp32    1.0.5   ~/Documents/Arduino/libraries/LittleFS_esp32
TaskScheduler     3.6.0   ~/Documents/Arduino/libraries/TaskScheduler
Time              1.6.1   ~/Documents/Arduino/libraries/Time
modem-freeRTOS    1.0.0   ~/Documents/Arduino/libraries/modem-freeRTOS
sysfile           1.0.0   ~/Documents/Arduino/libraries/sysfile
autorequest       1.0.0   ~/Documents/Arduino/libraries/autorequest
alarm             1.0.0   ~/Documents/Arduino/libraries/alarm
modbusrtu         1.0.0   ~/Documents/Arduino/libraries/ModbusRTU
