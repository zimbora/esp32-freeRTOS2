#!/bin/bash

home_dir="$HOME"
build="dev"
board="esp32"
project="esp32-freeRTOS2"
app="demo"
fw_version="1.0.0"
app_version="1.0.0"

# Set the path to your header file
FILE="package.h"
FILEAPP="./src/app/user/app_package.h"

# Use grep to find the line, then sed to extract the version number
fw_version=$(grep "#define FW_VERSION" "$FILE" | sed -E 's/.*#define FW_VERSION[[:space:]]+"([^"]+)".*/\1/')
app_version=$(grep "#define APP_VERSION" "$FILEAPP" | sed -E 's/.*#define APP_VERSION[[:space:]]+"([^"]+)".*/\1/')
echo "fw version: $fw_version"
echo "app version: $app_version"

docker=false
force_lib_install=false
libs="ESP32httpUpdate@2.1.145 ArduinoJson@6.19.4 
      ESP32Logger2@1.0.3 EspMQTTClientFork@1.13.4
			Time@1.6.1 esp32-BG95@1.0.6 modem-freeRTOS@1.0.10
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
    -b|--build)
      build="$2"
      echo "build set: ${build}"
      shift 2
      ;;
    -B|--board)
      board="$2"
      echo "board set: ${board}"
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
    -f|--force)
      force_lib_install=true
      echo "Force install on version mismatch: enabled"
      shift
      ;;
    *)
      echo "Unknown parameter: $1"
      exit 1
      ;;
  esac
done

# Expand a leading '~' if provided via CLI args
home_dir="${home_dir/#\~/$HOME}"

case "$board" in
  esp32|esp32c5)
    ;;
  *)
    echo "Unsupported board: $board"
    echo "Available boards: esp32, esp32c5"
    exit 1
    ;;
esac

board_fqbn="esp32:esp32:${board}"

# Modify MQTT_HOST_1 based on build type
echo "Build type: ${build}"
CREDENTIALS_FILE="./src/app/user/credentials.h"

if [ "$build" == "staging" ]; then
    echo "Setting MQTT_HOST_1 for stage environment..."
    sed -i.bak 's/#define MQTT_HOST_1 "[^"]*"/#define MQTT_HOST_1 "devices.staging.inloc.cloud"/' "$CREDENTIALS_FILE"
    sed -i.bak 's/#define WIFI_SSID "[^"]*"/#define WIFI_SSID "Inloc"/' "$CREDENTIALS_FILE"
    sed -i.bak 's/#define WIFI_PASSWORD "[^"]*"/#define WIFI_PASSWORD "inlocAPpwd"/' "$CREDENTIALS_FILE"
    echo "MQTT_HOST_1 set to devices.staging.inloc.cloud"
    echo "WIFI_SSID set to inlocAP"
    echo "WIFI_PASSWORD set to inlocAPpwd"
elif [ "$build" == "prod" ]; then
    echo "Setting MQTT_HOST_1 for production environment..."
    sed -i.bak 's/#define MQTT_HOST_1 "[^"]*"/#define MQTT_HOST_1 "devices.inloc.cloud"/' "$CREDENTIALS_FILE"
    echo "WIFI_SSID set to Inloc"
    echo "WIFI_PASSWORD set to inlocAPpwd"
    echo "MQTT_HOST_1 set to devices.inloc.cloud"
else
    echo "Using default MQTT_HOST_1 for dev environment"
fi

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
elif [ "$docker" == "false" ]; then
  if [ "$running_os" == "linux" ]; then
    arduino_lib_path="${home_dir}/Arduino/libraries/"
  elif [ "$running_os" == "macos" ]; then
    arduino_lib_path="$HOME/Documents/Arduino/libraries/"
  else
    echo "Unsupported OS: $running_os"
    exit 1
  fi
fi

arduino-cli config dump

# Install third-party libraries (replace 'LibraryName' with actual library names)

find_installed_lib_dir() {
  local target_name="$1"
  local dir
  
  for dir in "$arduino_lib_path"*/; do
    [ -d "$dir" ] || continue

    if [ -f "${dir}library.properties" ]; then
      local prop_name
      prop_name=$(grep -E '^name=' "${dir}library.properties" | head -n 1 | cut -d'=' -f2- | tr -d '\r')
      if [ "$prop_name" = "$target_name" ]; then
        echo "${dir%/}"
        return 0
      fi
    fi

    if [ "$(basename "${dir%/}")" = "$target_name" ]; then
      echo "${dir%/}"
      return 0
    fi
  done

  return 1
}

YELLOW='\033[1;33m'
RED='\033[1;31m'
NC='\033[0m'

for lib in $libs; do
	echo "$lib"

  lib_name="${lib%@*}"
  requested_version="${lib#*@}"
  if [ "$lib_name" = "$requested_version" ]; then
    requested_version=""
  fi

  installed_dir=$(find_installed_lib_dir "$lib_name")
  if [ -n "$installed_dir" ]; then
    echo "Library '$lib_name' already exists at: $installed_dir"

    installed_version=""
    if [ -f "$installed_dir/library.properties" ]; then
      installed_version=$(grep -E '^version=' "$installed_dir/library.properties" | head -n 1 | cut -d'=' -f2- | tr -d '\r')
      echo "Installed version: $installed_version"
    fi

    if [ -n "$requested_version" ] && [ -n "$installed_version" ] && [ "$installed_version" != "$requested_version" ]; then
      echo -e "${YELLOW}Version mismatch for $lib_name:${NC} requested=${RED}${requested_version}${NC}, installed=${RED}${installed_version}${NC}"
      if [ "$force_lib_install" == "true" ]; then
        echo "-f provided: forcing install of $lib"
        arduino-cli lib install "$lib"
        if [ $? -ne 0 ]; then
          echo "Error: Failed to install $lib"
          exit 1
        fi
      else
        echo "Keeping installed version unchanged."
      fi
    fi

    continue
  fi

	arduino-cli lib install "$lib"
  # Check if the last command was successful
  if [ $? -ne 0 ]; then
      echo "Error: Failed to install $lib"
      exit 1  # Exit with an error status
  fi
done

if [ "$docker" == "true" ]; then
	ls "$arduino_lib_path"
fi

echo "Installation complete!"
echo "project: ${project}"
echo "app: ${app}"

arduino-cli cache clean

# Compile the project and capture output
echo "Compiling with board: ${board} (${board_fqbn})"
output=$(arduino-cli compile -b "${board_fqbn}" \
--build-property build.partitions=min_spiffs \
--build-property upload.maximum_size=1966080 \
--build-path ./build/${app} . 2>&1)

# Check if the compilation was successful
if [ $? -eq 0 ]; then
  echo "Compilation successful!"
else
  # Log output to a file
  echo "$output"
  echo "Error: Compilation failed."
  exit 1  # Exit with an error status
fi

echo "$output" | tee compile_logs.txt

if [ -d "images" ]; then
  rm -r "images"
fi

mkdir -p "images"

filenames=$( find build/${app}/${project}* )
cp compile_logs.txt images/
cp ${filenames} images/
mv images/esp32-freeRTOS2.ino.bin images/${project}-${app}-${fw_version}-${app_version}-${build}-${board}.bin
mv images/esp32-freeRTOS2.ino.merged.bin images/${project}-${app}-${fw_version}-${app_version}-${build}-${board}.merged.bin
cp build/${app}/build.options.json images/
