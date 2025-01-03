# esp32-freeRTOS Firmware

## Versions

### Deploy procedure
In development stage firmware has always the latest version launched.
Only after a version is finished FW_VERSION macro in package.h file is changed to version in development.
Then, compiled file is uploaded to mgmt-iot-devices platform as a nightmare version.
After being tested with some devices, version can be changed to stable a version


## TODO
  - Add uid to the header of fota request
  - Fota md5 is being well calculated but the comparison is not being well done

### 1.1.2 - In dev

### 1.1.1
	Creates:
	- Doc folder
	Moves: 
	- macros from package.h to app_package.h
	Fixes:
	- write settings fw version
	- WIFI fota
	- fota mqtt msg
	Adds:
	- DEBUG_MQTT_PAYLOAD
	- delay to tasks
	- mRTOS task for wifi devices
	- var in workflow
	- Macros for tasks
	- enum for logs
	MQTT:
	- Sends reboot cause, rssi and tech
	Uses:
	- modem-freeRTOSv1.0.5 version on build

### 1.1.0 - Big refactor - Not fully tested
	- supports modem-freeRTOS v1.0.4
	- compiled with 3.0.7 esp32 by Espressif Systems
	- changes on wifi and AP state machine
	- move RS485 inside core
	- enables uart2
	- removes runner task

### 1.0.3
	- stable version
	- supports LTE and WiFi
	- WiFi AP configuration

	Problems:
		"uid:ac67b2e9d11cesp32" - check for a leak in message topic

### 1.0.2
	- no history

### 1.0.1
	- no history

### 1.0.0
	- first release
