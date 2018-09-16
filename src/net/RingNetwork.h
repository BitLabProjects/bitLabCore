#ifndef _RINGNETWORK_H_
#define _RINGNETWORK_H_

#include "mbed.h"
#include "..\os\CoreModule.h"

struct __packed RingPacketHeader {
  uint8_t data_size;
  uint8_t control;
  uint8_t src_address;
  uint8_t dst_address;
  uint8_t ttl;
};

struct __packed RingPacketFooter {
  uint32_t hash;
};

struct __packed RingPacket {
  RingPacketHeader header;
  uint8_t data[256];
  RingPacketFooter footer;
};

class RingNetwork : public CoreModule
{
public:
  RingNetwork(PinName TxPin, PinName RxPin);

  // --- CoreModule ---
  void init();
  void mainLoop();
  void tick(millisec64 timeDelta);
  // ------------------

  bool packetReceived() { return rx_packet_ready; }
  RingPacket* getPacket() { return &rx_packet; }
  void receiveNextPacket() { rx_packet_ready = false; }

  void sendPacket(const RingPacket* packet);

private:
  Serial serial;
  static const uint32_t packet_maxsize = 265;
  static const uint8_t ttl_max = 10;

  //data for tx
  enum TxState
  {
    TxIdle,
    SendStartByte,
    SendPacket,
    TxEscape,
    SendEndByte
  };
  volatile TxState tx_state;
  uint8_t tx_packet[packet_maxsize];
  volatile uint32_t tx_packet_size;
  volatile uint32_t tx_packet_idx;

  //data for rx
  enum RxState
  {
    RxIdle,
    ReceivePacketHeader,
    ReceivePacketData,
    ReceivePacketFooter,
    RxEscape,
    ReceiveEndByte
  };
  volatile RxState rx_state;
  volatile RxState rx_state_return;
  volatile uint32_t rx_bytes_to_read;
  volatile uint8_t* rx_bytes_dst;
  //TODO Understand if volatile here is needed, it spreads to read functions and i don't want it
  RingPacket rx_packet;
  volatile bool rx_packet_ready;

  void rxIrq();
  void txIrq();
};

#endif