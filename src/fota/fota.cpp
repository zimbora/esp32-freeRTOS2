
#include "fota.h"

bool FOTA::running(){

  return undergoing;
}

void FOTA::in_progress(bool state){
  undergoing = state;
}

bool FOTA::start(uint32_t size){
  if (!Update.begin(size)){
    Serial.print("fota: cannot begin, not enough space");
    Update.printError(Serial);
    return false;
  }
  else{
    in_progress(true);
    return true;
  }
}

bool FOTA::write_block(uint8_t* frame, uint16_t length){
  if (Update.write(frame, length) != length) {
    Serial.println("fota: write has failed");
    Update.printError(Serial);
    return false;
  }
  return true;
}

bool FOTA::has_finished(){

  
  if (!Update.end(true)) {
  	Serial.print("fota: cannot end");
  	Update.printError(Serial);
    in_progress(false);

    return false;
  }

  if (!Update.isFinished()) {
  	Serial.print("fota: did not finish");
  	Update.printError(Serial);
  	in_progress(false);

    return false;
  }

  return true;
}
