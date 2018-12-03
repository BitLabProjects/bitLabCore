#include "RingNetwork.h"
#include "..\os\os.h"
#include "..\os\bitLabCore.h"

RingNetwork::RingNetwork(PinName TxPin,
                         PinName RxPin,
                         bool watchForSilence) : serial(TxPin, RxPin),
                                                 hardware_id(0),
                                                 mac_state(MacState::AddressNotAssigned),
                                                 mac_address(0),
                                                 watchForSilence(watchForSilence),
                                                 mac_watcher_state(MacWatcherState::Start),
                                                 mac_watcher_timeout(0),
                                                 tx_state(TxState::TxIdle),
                                                 tx_packet_size(0),
                                                 tx_packet_idx(0),
                                                 rx_state(RxState::RxIdle),
                                                 rx_state_return(RxState::RxIdle),
                                                 rx_bytes_to_read(0),
                                                 rx_bytes_dst(NULL),
                                                 rx_packet_ready(false)
{
  mac_delay_timeout = 0;
}

void RingNetwork::init(const bitLabCore *core)
{
  hardware_id = core->getHardwareId();
  serial.baud(115200);
  serial.attach(callback(this, &RingNetwork::rxIrq), Serial::RxIrq);
  //Do not attach TxIrq here: it will be done when needed and the txIrq method detaches itself when there's nothing to transmit
  //serial.attach(callback(this, &RingNetwork::txIrq), Serial::TxIrq);
}

void RingNetwork::mainLoop()
{
  bool packetReceived = this->packetReceived();
  //Always update watcher: it has the timeot to detect too much silence
  if (watchForSilence) {
    mainLoop_UpdateWatcher(packetReceived);
  }

  if (packetReceived)
  {
    if (mac_delay_timeout == 0)
    {
      mainLoop_UpdateMac(getPacket());
      receiveNextPacket();
    }
  }
}

void RingNetwork::mainLoop_UpdateMac(RingPacket *p)
{
  auto protocol_msgid = p->header.data_size > 0 ? p->data[0] : RingNetworkProtocol::protocol_msgid_free;

  PTxAction pTxAction = PTxAction::SendFreePacket;

  switch (mac_state)
  {
  case MacState::AddressNotAssigned:
    if (p->isFreePacket())
    {
      mac_address = ((hardware_id >>  0) & 255) ^
                    ((hardware_id >>  8) & 255) ^
                    ((hardware_id >> 16) & 255) ^ 
                    ((hardware_id >> 24) & 255);
      if (mac_address == RingNetworkProtocol::broadcast_address) {
        mac_address = 254;
      }
      mac_device_name[0] = 'D';
      mac_device_name[1] = 'e';
      mac_device_name[2] = 'v';
      mac_device_name[3] = 'i';
      mac_device_name[4] = 'c';
      mac_device_name[5] = 'e';
      mac_device_name[6] = '-';
      mac_device_name[7] = '0' + (mac_address / 100) % 10;
      mac_device_name[8] = '0' + (mac_address / 10) % 10;
      mac_device_name[9] = '0' + (mac_address / 1) % 10;
      for (uint32_t i = 10; i < RingNetworkProtocol::device_name_maxsize; i++)
      {
        mac_device_name[i] = '\0';
      }
      p->header.control = 0;
      p->header.data_size = 1 + RingNetworkProtocol::device_name_maxsize;
      p->header.dst_address = mac_address;
      p->header.src_address = mac_address;
      p->header.ttl = RingNetworkProtocol::ttl_max;
      p->data[0] = RingNetworkProtocol::protocol_msgid_addressclaim;
      for (uint32_t i = 0; i < RingNetworkProtocol::device_name_maxsize; i++)
      {
        p->data[1 + i] = (uint8_t)mac_device_name[i];
      }
      //printf("[%5i] %i) Sent address claim packet\r\n", Os::currTime(), hardware_id);
      pTxAction = PTxAction::Send;
      mac_state = MacState::AddressClaiming;
    }
    else
    {
      //printf("[%5i] %i) Pass\r\n", Os::currTime(), hardware_id);
      pTxAction = PTxAction::PassAlongDecreasingTTL;
    }
    break;

  case MacState::AddressClaiming:
    if (p->isProtocolPacket())
    {
      switch (protocol_msgid)
      {
      case RingNetworkProtocol::protocol_msgid_addressclaim:
        //printf("[%5i] %i) Address claim received, addr:%i, name:%16s\r\n", Os::currTime(), hardware_id, p->header.dst_address, (char *)(&p->data[1]));
        //We are claiming an address and received an addressclaim packet, there are three cases
        if (p->isForDstAddress(mac_address))
        {
          auto name_cmp_result = strncmp(mac_device_name, (char *)(&p->data[1]), RingNetworkProtocol::device_name_maxsize);
          if (name_cmp_result == 0)
          {
            //case 1: It's our packet that round-tripped the ring, the address is claimed
            //printf("[%5i] %i) Address claimed\r\n", Os::currTime(), hardware_id);
            pTxAction = PTxAction::SendFreePacket;
            mac_state = MacState::Idle;
          }
          else
          {
            //case 2: It's another device claiming the same address, the one with the 'lower' name wins
            if (name_cmp_result < 0)
            {
              //printf("[%5i] %i) Discarded conflicting claim\r\n", Os::currTime(), hardware_id);
              //We have a lower name, discard the other device packet name and keep waiting
              pTxAction = PTxAction::SendFreePacket;
            }
            else
            {
              //printf("[%5i] %i) Conflicting claim, going back to initial state\r\n", Os::currTime(), hardware_id);
              //The other device has a lower name, go back to initial state
              pTxAction = PTxAction::PassAlongDecreasingTTL;
              mac_state = MacState::AddressNotAssigned;
            }
          }
        }
        else
        {
          //case 3: It's another device claiming another address, pass the message along
          //printf("[%5i] %i) Address claim of another device\r\n", Os::currTime(), hardware_id);
          pTxAction = PTxAction::PassAlongDecreasingTTL;
        }
        break;

      default:
        //Unknown protocol msgid, TODO signal
        //Pass along, even if the dst address is the one we're trying to claim.
        pTxAction = PTxAction::PassAlongDecreasingTTL;
        break;
      }
    }
    else
    {
      //It's a data packet, we don't have an address yet so we can only pass along
      //Note that if the address we are trying to claim is of someone else, we don't want to disturb his traffic here
      pTxAction = PTxAction::PassAlongDecreasingTTL;
    }
    break;

  case MacState::Idle:
    if (p->isBroadcast()) {
      // If it's a broadcast, override the pTxAction: pass along if not from us, send free if we generated it
      if (p->header.src_address == mac_address) {
        pTxAction = PTxAction::SendFreePacket;
      } else {
        if (onPacketReceived)
        {
          // Note that the callback can't modify the packet
          onPacketReceived.call(p, &pTxAction);
        }
        pTxAction = PTxAction::PassAlongDecreasingTTL;
      }
    }
    else if (p->isForDstAddress(mac_address))
    {
      if (p->isProtocolPacket())
      {
        switch (protocol_msgid)
        {
        case RingNetworkProtocol::protocol_msgid_addressclaim:
          //We are not claiming an address and received an addressclaim packet: refuse the claimer
          //since we are the one owning the address being claimed
          pTxAction = PTxAction::SendFreePacket;
          break;

        case RingNetworkProtocol::protocol_msgid_hello:
          if (onPacketReceived)
          {
            onPacketReceived.call(p, &pTxAction);
          }
          break;

        default:
          //Unknown protocol msgid, TODO signal
          pTxAction = PTxAction::SendFreePacket;
          break;
        }
      }
      else
      {
        if (onPacketReceived)
        {
          onPacketReceived.call(p, &pTxAction);
        }
      }
    }
    else
    {
      if (protocol_msgid == RingNetworkProtocol::protocol_msgid_free)
      {
        if (onPacketReceived)
        {
          onPacketReceived.call(p, &pTxAction);
        }
        else
        {
          pTxAction = PTxAction::PassAlongDecreasingTTL;
        }
      }
      else if (p->isProtocolPacket() && protocol_msgid == RingNetworkProtocol::protocol_msgid_whoareyou)
      {
        if (p->header.ttl <= 1)
        {
          //this whoareyou is for us, answer
          p->setHelloUsingSrcAsDst(mac_address, hardware_id);
          pTxAction = PTxAction::Send;
        }
        else
        {
          //this whoareyou is not for us, pass along
          pTxAction = PTxAction::PassAlongDecreasingTTL;
        }
      }
      else
      {
        //Not for us, pass along
        pTxAction = PTxAction::PassAlongDecreasingTTL;
      }
    }
    break;

  default:
    //printf("[%5i] %i) Pass\r\n", Os::currTime(), hardware_id);
    pTxAction = PTxAction::PassAlongDecreasingTTL;
    break;
  }

  switch (pTxAction)
  {
  case PTxAction::PassAlongDecreasingTTL:
    p->header.ttl -= 1;
    //If the ttl reaches zero, transform the packet to a free packet:
    //This is the place where we prevent infinite packet looping
    //Es: a packet destinated to a non-existing dst_address
    if (p->header.ttl == 0)
    {
      p->setFreePacket();
    }
    break;

  case PTxAction::SendFreePacket:
    p->setFreePacket();
    break;

  case PTxAction::Send:
    break;
  }
  sendPacket(p);
}

void RingNetwork::mainLoop_UpdateWatcher(bool packetReceived)
{
  //Every time a packet is received, the watcher is resetted
  if (packetReceived)
  {
    mac_watcher_state = MacWatcherState::Start;
    is_connected = true;
  }

  // TODO atomic access of mac_watcher_timeout
  switch (mac_watcher_state)
  {
  case MacWatcherState::Start:
    mac_watcher_timeout = 500;
    mac_watcher_state = MacWatcherState::WaitingSilence;
    break;

  case MacWatcherState::WaitingSilence:
    if (mac_watcher_timeout == 0)
    {
      mac_watcher_timeout = hardware_id % 500;
      mac_watcher_state = MacWatcherState::WaitingAfterSilence;
      //printf("[%5i] %i) 500 ms of silence detected, waiting %i ms\r\n", Os::currTime(), hardware_id, mac_watcher_timeout);
    }
    break;

  case MacWatcherState::WaitingAfterSilence:
    if (mac_watcher_timeout == 0)
    {
      is_connected = false;
      RingPacket p;
      p.setFreePacket();
      sendPacket(&p);
      mac_watcher_state = MacWatcherState::Start;
      //printf("[%5i] %i) injecting free packet after silence\r\n", Os::currTime(), hardware_id);
    }
    break;
  }
}

void RingNetwork::sendPacket(const RingPacket *packet)
{
  // TODO Exclude interrupts

  if (!txIsIdle())
  {
    //TODO Signal error
    return;
  }

  //Note that escaping is handled by txIrq
  uint8_t *dst_tx = tx_packet;
  *dst_tx++ = packet->header.data_size;
  *dst_tx++ = packet->header.control;
  *dst_tx++ = packet->header.src_address;
  *dst_tx++ = packet->header.dst_address;
  *dst_tx++ = packet->header.ttl;
  for (uint8_t i = 0; i < packet->header.data_size; i++)
  {
    *dst_tx++ = packet->data[i];
  }
  //TODO Calculate hash
  *dst_tx++ = 0;
  *dst_tx++ = 0;
  *dst_tx++ = 0;
  *dst_tx++ = 0;

  tx_packet_size = 5 + packet->header.data_size + 4;
  tx_state = TxState::SendStartByte;

  if (serial.writeable())
  {
    serial.attach(callback(this, &RingNetwork::txIrq), Serial::TxIrq);
  }
}

void RingNetwork::tick(millisec timeDelta)
{
  mac_watcher_timeout -= timeDelta;
  if (mac_watcher_timeout < 0)
  {
    mac_watcher_timeout = 0;
  }

  mac_delay_timeout -= timeDelta;
  if (mac_delay_timeout < 0)
  {
    mac_delay_timeout = 0;
  }
}

const uint8_t StartByte = 85;
const uint8_t EndByte = 170;
const uint8_t EscapeByte = 27;
inline bool byteNeedsEscaping(uint8_t value)
{
  return value == StartByte || value == EndByte || value == EscapeByte;
}

void RingNetwork::txIrq()
{
  if (tx_state == TxState::TxIdle)
  {
    serial.abort_write();
    return;
  }

  while (serial.writeable())
  {
    switch (tx_state)
    {
    case TxState::TxIdle:
      return;

    case TxState::SendStartByte:
      serial.putc(StartByte);
      tx_packet_idx = 0;
      tx_state = TxState::SendPacket;
      break;

    case TxState::SendPacket:
      if (tx_packet_idx >= tx_packet_size)
      {
        tx_state = TxState::SendEndByte;
      }
      else
      {
        uint8_t value = tx_packet[tx_packet_idx];
        if (byteNeedsEscaping(value))
        {
          value = EscapeByte;
          tx_state = TxState::TxEscape;
        }
        else
        {
          tx_packet_idx += 1;
        }
        serial.putc(value);
      }
      break;

    case TxState::TxEscape:
      serial.putc(tx_packet[tx_packet_idx]);
      tx_packet_idx += 1;
      tx_state = TxState::SendPacket;
      break;

    case TxState::SendEndByte:
      serial.putc(EndByte);
      tx_state = TxState::TxIdle;
      break;
    }
  }
}

void RingNetwork::rxIrq()
{
  while (serial.readable())
  {
    uint8_t value = serial.getc();

    // When a packet is already received, discard all incoming data
    if (rx_packet_ready)
    {
      // TODO signal buffer underflow
      continue;
    }

    bool goToNextBlock = false;
    switch (rx_state)
    {
    case RxState::RxIdle:
      if (value == StartByte)
      {
        rx_bytes_dst = (volatile uint8_t *)&rx_packet.header;
        rx_bytes_to_read = 5;
        rx_state = RxState::ReceivePacketHeader;
      }
      break;

    case RxState::ReceivePacketHeader:
    case RxState::ReceivePacketData:
    case RxState::ReceivePacketFooter:
      if (value == EscapeByte)
      {
        rx_state_return = rx_state;
        rx_state = RxState::RxEscape;
      }
      else if (value == StartByte)
      {
        //TODO signal exception
        rx_bytes_dst = (volatile uint8_t *)&rx_packet.header;
        rx_bytes_to_read = 5;
        rx_state = RxState::ReceivePacketHeader;
      }
      else if (value == EndByte)
      {
        //TODO signal exception
        rx_state = RxState::RxIdle;
      }
      else
      {
        *rx_bytes_dst = value;
        rx_bytes_dst += 1;
        rx_bytes_to_read -= 1;
        if (rx_bytes_to_read == 0)
        {
          goToNextBlock = true;
        }
      }
      break;

    case RxState::RxEscape:
      rx_state = rx_state_return;
      *rx_bytes_dst = value;
      rx_bytes_dst += 1;
      rx_bytes_to_read -= 1;
      if (rx_bytes_to_read == 0)
      {
        goToNextBlock = true;
      }
      break;

    case RxState::ReceiveEndByte:
      if (value != EndByte)
      {
        //TODO mark malformed packet
      }
      rx_state = RxState::RxIdle;
      rx_packet_ready = true;
      //mac_delay_timeout = 100;
      break;
    }

    if (goToNextBlock)
    {
      switch (rx_state)
      {
      case RxState::ReceivePacketHeader:
        //First byte is data length. Move on with footer if no data is present
        rx_bytes_to_read = rx_packet.header.data_size;
        if (rx_bytes_to_read > 0)
        {
          rx_state = RxState::ReceivePacketData;
          rx_bytes_dst = rx_packet.data;
        }
        else
        {
          rx_bytes_to_read = 4;
          rx_bytes_dst = (volatile uint8_t *)&rx_packet.footer;
          rx_state = RxState::ReceivePacketFooter;
        }
        break;
      case RxState::ReceivePacketData:
        rx_bytes_to_read = 4;
        rx_bytes_dst = (volatile uint8_t *)&rx_packet.footer;
        rx_state = RxState::ReceivePacketFooter;
        break;
      case RxState::ReceivePacketFooter:
        //rx_bytes_to_read = 0; Unused for end byte
        rx_state = RxState::ReceiveEndByte;
        break;
      default:
        //Must not happen
        rx_state = RxState::RxIdle;
        break;
      }
    }
  }
}