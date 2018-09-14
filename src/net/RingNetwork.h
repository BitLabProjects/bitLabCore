#ifndef _RINGNETWORK_H_
#define _RINGNETWORK_H_

#include "mbed.h"
#include "..\os\CoreModule.h"

class RingNetwork: public CoreModule {
public:
  RingNetwork(PinName TxPin, PinName RxPin);

  // --- CoreModule ---
  void init();
  void mainLoop();
  void tick(millisec64 timeDelta);
  // ------------------

private:
  Serial serial;
  static const uint32_t packet_maxsize = 264;
  
  //data for tx
  enum TxState {
    Idle,
    SendStartByte,
    SendPacket,
    Escape,
    SendEndByte
  };
  volatile TxState  tx_state;
  uint8_t tx_packet[packet_maxsize];
  volatile uint32_t tx_packet_size;
  volatile uint32_t tx_packet_idx;

  void rxIrq();
  void txIrq();
};

#endif