#ifndef _RINGNETWORK_H_
#define _RINGNETWORK_H_

#include "mbed.h"
#include "..\os\CoreModule.h"

class RingNetworkProtocol
{
public:
  static const uint32_t packet_maxsize = 265;
  static const uint8_t ttl_max = 10;
  static const uint32_t device_name_maxsize = 16;
  static const uint8_t protocol_msgid_free = 0;
  static const uint8_t protocol_msgid_addressclaim = 1;
  static const uint8_t protocol_msgid_whoareyou = 2;
  static const uint8_t protocol_msgid_hello = 3;
};

struct __packed RingPacketHeader
{
  uint8_t data_size;
  uint8_t control;
  uint8_t src_address;
  uint8_t dst_address;
  uint8_t ttl;
};

struct __packed RingPacketFooter
{
  uint32_t hash;
};

struct __packed RingPacket
{
  RingPacketHeader header;
  uint8_t data[256];
  RingPacketFooter footer;

  inline bool isProtocolPacket()
  {
    return (header.control & 1) == 0;
  }
  inline bool isFreePacket()
  {
    return isProtocolPacket() && header.data_size == 0;
  }
  inline bool isForDstAddress(uint8_t dst_address)
  {
    return header.dst_address == dst_address;
  }
  inline bool isDataPacket(uint8_t dst_address, uint8_t minDataSize, uint8_t msgId)
  {
    return (!isProtocolPacket() &&
            isForDstAddress(dst_address) &&
            header.data_size >= minDataSize &&
            data[0] == msgId);
  }

  uint32_t getDataUInt32(uint8_t offset)
  {
    uint32_t result;
    auto src = &data[offset];
    auto dst = (uint8_t*)&result;
    *(dst+0) = *(src+0);
    *(dst+1) = *(src+1);
    *(dst+2) = *(src+2);
    *(dst+3) = *(src+3);
    return result;
  }
  int32_t getDataInt32(uint8_t offset)
  {
    int32_t result;
    auto src = &data[offset];
    auto dst = (uint8_t*)&result;
    *(dst+0) = *(src+0);
    *(dst+1) = *(src+1);
    *(dst+2) = *(src+2);
    *(dst+3) = *(src+3);
    return result;
  }

  void setFreePacket()
  {
    header.control = 0;
    header.data_size = 0;
    header.src_address = 0;
    header.dst_address = 0;
    header.ttl = RingNetworkProtocol::ttl_max;
  }

  void setHelloUsingSrcAsDst(uint8_t newSrcAddress, uint32_t hardwareId)
  {
    header.control = 0;
    header.data_size = 1 + 4;
    header.dst_address = header.src_address;
    header.src_address = newSrcAddress;
    header.ttl = RingNetworkProtocol::ttl_max;
    data[0] = RingNetworkProtocol::protocol_msgid_hello;
    *((uint32_t *)&data[1]) = hardwareId;
  }
};

enum PTxAction
{
  PassAlongDecreasingTTL,
  SendFreePacket,
  Send
};

class RingNetwork : public CoreModule
{
public:
  RingNetwork(PinName TxPin, PinName RxPin, bool watchForSilence);

  // --- CoreModule ---
  const char *getName() { return "RingNetwork"; }
  void init(const bitLabCore *);
  void mainLoop();
  void tick(millisec timeDelta);
  // ------------------

  void attachOnPacketReceived(Callback<void(RingPacket *, PTxAction *)> onPacketReceived) { this->onPacketReceived = onPacketReceived; }

  bool packetReceived() { return rx_packet_ready; }
  RingPacket *getPacket() { return &rx_packet; }
  void receiveNextPacket() { rx_packet_ready = false; }

  void sendPacket(const RingPacket *packet);

  inline bool isAddressAssigned() { return mac_state == Idle; }
  inline uint8_t getAddress() { return mac_address; }
  inline bool getSilenceDetected()
  {
    if (silenceDetected)
    {
      silenceDetected = false;
      return true;
    }
    else
    {
      return false;
    }
  }

private:
  Serial serial;
  Callback<void(RingPacket *, PTxAction *)> onPacketReceived;

  enum MacState
  {
    AddressNotAssigned,
    AddressClaiming,
    Idle
  };
  uint32_t hardware_id;
  MacState mac_state;
  uint8_t mac_address;
  char mac_device_name[RingNetworkProtocol::device_name_maxsize];

  enum MacWatcherState
  {
    Start,
    WaitingSilence,
    WaitingAfterSilence
  };
  bool watchForSilence;
  MacWatcherState mac_watcher_state;
  volatile millisec mac_watcher_timeout;
  volatile millisec mac_delay_timeout;
  bool silenceDetected;

  void mainLoop_UpdateWatcher(bool packetReceived);
  void mainLoop_UpdateMac(RingPacket *p);

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
  uint8_t tx_packet[RingNetworkProtocol::packet_maxsize];
  volatile uint32_t tx_packet_size;
  volatile uint32_t tx_packet_idx;
  inline bool txIsIdle() { return tx_state == TxState::TxIdle; }

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
  volatile uint8_t *rx_bytes_dst;
  //TODO Understand if volatile here is needed, it spreads to read functions and i don't want it
  RingPacket rx_packet;
  volatile bool rx_packet_ready;

  void rxIrq();
  void txIrq();
};

#endif