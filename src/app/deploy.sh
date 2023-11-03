#!/bin/bash

home_dir="~"
project="esp32-freeRTOS2"
app="DEMO"
app_version="0.0.0"

docker=false
libs="WiFi@2.0.0 Update@2.0.0 ArduinoOTA@2.0.0 WebServer@2.0.0
			ESPmDNS@2.0.0 WiFiClientSecure@2.0.0 FS@2.0.0 ESP32 BLE Arduino@2.0.0
			ArduinoJson@6.19.4 ESP32Logger@1.0.2 EspMQTTClient@1.13.3 PubSubClient@2.8
			LittleFS_esp32@1.0.5 TaskScheduler@3.6.0 Time@1.6.1 modem-freeRTOS@1.0.0
			sysfile@1.0.0 autorequest@1.0.0 alarm@1.0.0 modbusrtu@1.0.0
			"

if [ -f /.dockerenv ]; then
    echo "You are inside a Docker container!"
    docker="true"
fi

OS=$(uname -s)

running_os=""

case "$OS" in
  Linux)
    echo "You are on Linux!"
    running_os="linux"
    ;;
  Darwin)
    echo "You are on macOS!"
    running_os="macos"
    ;;
  *)
    echo "Unknown OS: $OS"
    ;;
esac

while [ "$#" -gt 0 ]; do
  case "$1" in
    -d|--directory)
      home_dir="$2"
      echo "home dir set: ${home_dir}"
      shift 2
      ;;
    -p|--project)
      project="$2"
      echo "Project directory set: $project"
      shift 2
      ;;
    -a|--app)
      app="$2"
      echo "Project directory set: $app"
      shift 2
      ;;
    -v|--app_version)
      app_version="$2"
      echo "app version set: $app_version"
      shift 2
      ;;
    *)
      echo "Unknown parameter: $1"
      exit 1
      ;;
  esac
done

# Update system

if [ "$docker" == "true" ]; then
	sudo apt-get update
fi

# Check if arduino-cli is installed
if command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli is installed."
else
    # Install arduino-cli
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

	# Initialize arduino-cli (this will create default directories and config file)
	arduino-cli config init

	# Update the core index (required before installing cores or libraries)
	arduino-cli core update-index

	# Install ESP32 core
	arduino-cli core install esp32:esp32
fi

arduino_config_file="${home_dir}/.arduino15/arduino-cli.yaml"
arduino_lib_path="${home_dir}/Arduino/libraries/"

if [ "$docker" == "true" ]; then
	if grep -q "directories: libraries" $arduino_config_file; then
	    echo "The 'directories: libraries' entry exists in the file!"
	else
	    echo "The 'directories: libraries' entry does not exist in the file."
		# Add 'libraries' entry under 'directories'
		echo "Trying to fix it"
		if [ "$running_os" == "linux" ]; then
			sed -i "/directories:/a \ \ libraries: ${arduino_lib_path}" "$arduino_config_file"
		else
			echo "Os not supported to set it"
			exit 1
		fi
	fi
fi

arduino-cli config dump

# Install third-party libraries (replace 'LibraryName' with actual library names)

for lib in $libs; do
	echo $lib
	arduino-cli lib install $lib
done

if [ "$docker" == "true" ]; then
	ls "$arduino_lib_path"
fi

echo "Installation complete!"
echo "project: ${project}"
echo "app: ${app}"
cd $project

arduino-cli compile -b esp32:esp32:esp32 \
--build-property APP=${app} \
--build-property APP_VERSION=$app_version  \
--build-property build.partitions=min_spiffs \
--build-property upload.maximum_size=1966080  \
--build-path ./build/${app} . 2>&1 | tee compile_logs.txt

if [ -d "images/${app}" ]; then
    rm -r "images/${app}"
fi
mkdir -p images/${app}

filenames=$( find build/${app}/${project}* )
cp ${filenames} images/${app}/
cp build/${app}/build.options.json images/${app}/

