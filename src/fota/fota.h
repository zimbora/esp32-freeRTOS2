#ifndef FOTA_H
#define FOTA_H

#include "Arduino.h"
#include "Update.h"

class FOTA {
public:
            FOTA(){};
  bool      running();
  bool      start(uint32_t size);
  void      in_progress(bool undergoing);
  bool      write_block(uint8_t* frame, uint16_t length);
  bool      has_finished();

private:

  bool      undergoing    = false;
};

#endif
