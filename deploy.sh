#!/bin/bash

home_dir="~"
project="esp32-freeRTOS2"
app="demo"
fw_version="1.0.0"
app_version="1.0.0"

# Set the path to your header file
FILE="package.h"

# Use grep to find the line, then sed to extract the version number
fw_version=$(grep "#define FW_VERSION" "$FILE" | sed -E 's/#define FW_VERSION\s+"([^"]+)"/\1/')
echo "fw version: $fw_version"

docker=false
libs="ESP32httpUpdate@2.1.145 ArduinoJson@6.19.4 
      ESP32Logger2@1.0.3 EspMQTTClientFork@1.13.4
			Time@1.6.1 esp32-BG95@1.0.6 modem-freeRTOS@1.0.5
			sysfile@1.0.3 autorequest@1.0.1 alarm@1.0.1 modbusrtu@1.0.1
			"

if [ -f /.dockerenv ]; then
    echo "You are inside a Docker container!"
    docker="true"
    home_dir="/root"
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
      echo "App set: $app"
      shift 2
      ;;
    -v|--fw_version)
      fw_version="$2"
      echo "fw version set: $fw_version"
      shift 2
      ;;
    -va|--app_version)
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

# Check if arduino-cli is installed
if command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli is installed."

    current_dir=$(pwd)

    # add to PATH
    export PATH="$PATH:$current_dir/bin"
else
    # Install arduino-cli
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

  current_dir=$(pwd)

  # add to PATH
  export PATH="$PATH:$current_dir/bin"

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

arduino-cli cache clean

arduino-cli compile -b esp32:esp32:esp32 \
--build-property build.partitions=min_spiffs \
--build-property upload.maximum_size=1966080  \
--build-path ./build/${app} . 2>&1 | tee compile_logs.txt

if [ ! -d "images" ]; then
  rm -r "images"
fi

mkdir -p "images"

filenames=$( find build/${app}/${project}* )
cp ${filenames} images/
cp build/${app}/build.options.json images/
cp build/${app}/build.options.json images/

mv images/${project}.ino.bin images/${project}-${fw_version}-${app}-${app_version}.ino.bin