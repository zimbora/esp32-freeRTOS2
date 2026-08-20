
#include "fota.h"
#include "../settings/settings.h"

bool FOTA::running(){

  return undergoing;
}

void FOTA::in_progress(bool state){
  undergoing = state;
}

bool FOTA::start(uint32_t size){
  if (!Update.begin(size)){
    LOG_ERROR("fota: cannot begin, not enough space");
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
    LOG_ERROR("fota: write has failed\n");
    Update.printError(Serial);
    return false;
  }
  return true;
}

bool FOTA::has_finished(){

  
  if (!Update.end(true)) {
  	LOG_ERROR("fota: cannot end");
  	Update.printError(Serial);
    in_progress(false);

    return false;
  }

  if (!Update.isFinished()) {
  	LOG_ERROR("fota: did not finish");
  	Update.printError(Serial);
  	in_progress(false);

    return false;
  }

  return true;
}
