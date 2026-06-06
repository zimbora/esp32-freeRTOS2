# Script to flash esp32 sniffer
# Command: 
# >> sudo ./flash.sh -p /dev/ttyUSB0 -f esp32-freeRTOS2.ino.merged.bin

filename=esp32-freeRTOS2.ino.merged.bin
port=/dev/ttyUSB0

while [ "$#" -gt 0 ]; do
  case "$1" in
  	-h|--help)
      echo "args:
      	-p/--port		output port indetification
      	-f/--filename		name of file to be flashed inside current dir"
      exit 0
      ;;
    -p|--port)
      port="$2"
      echo "home dir set: ${port}"
      shift 2
      ;;
    -f|--filename)
      filename="$2"
      echo "filename: $filename"
      shift 2
      ;;
    *)
      echo "Unknown parameter: $1"
      exit 1
      ;;
  esac
done

#sudo esptool.py --port ${port} read_mac
sudo esptool --port ${port} erase_flash 
sudo esptool --port ${port} --baud 460800 write-flash 0x0 ${filename}
