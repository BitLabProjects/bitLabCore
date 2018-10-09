#include "RingNetwork.h"
#include "..\os\os.h"

RingNetwork::RingNetwork(PinName TxPin, PinName RxPin, uint32_t hardwareId) : serial(TxPin, RxPin), 
                                                                              hardware_id(hardwareId),
                                                                              mac_state(MacState::AddressNotAssigned),
                                                                              mac_address(0),
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
}

void RingNetwork::init()
{
  serial.baud(115200);
  serial.attach(callback(this, &RingNetwork::rxIrq), Serial::RxIrq);
  //Do not attach TxIrq here: it will be done when needed and the txIrq method detaches itself when there's nothing to transmit
  //serial.attach(callback(this, &RingNetwork::txIrq), Serial::TxIrq);
}

enum PTxAction
{
  PassAlongDecreasingTTL,
  SendFreePacket,
  Send
};

void RingNetwork::mainLoop()
{
  bool packetReceived = this->packetReceived();
  //Always update watcher: it has the timeot to detect too much silence
  mainLoop_UpdateWatcher(packetReceived);

  if (packetReceived)
  {
    mainLoop_UpdateMac(getPacket());
    receiveNextPacket();
  }
}

void RingNetwork::mainLoop_UpdateMac(RingPacket *p)
{
  PTxAction pTxAction = PTxAction::SendFreePacket;

  switch (mac_state)
  {
  case MacState::AddressNotAssigned:
      if (p->isFreePacket())
      {
        mac_address = hardware_id & 255;
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
        for (int i = 10; i < RingNetworkProtocol::device_name_maxsize; i++)
        {
          mac_device_name[i] = 0;
        }
        p->header.control = 0;
        p->header.data_size = 1 + RingNetworkProtocol::device_name_maxsize;
        p->header.dst_address = mac_address;
        p->header.src_address = mac_address;
        p->header.ttl = RingNetworkProtocol::ttl_max;
        p->data[0] = RingNetworkProtocol::protocol_msgid_addressclaim;
        for (int i = 0; i < RingNetworkProtocol::device_name_maxsize; i++)
        {
          p->data[1 + i] = mac_device_name[i];
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
        auto protocol_msgid = p->data[0];
        switch (protocol_msgid)
        {
        case RingNetworkProtocol::protocol_msgid_addressclaim:
          //printf("[%5i] %i) Address claim received, addr:%i, name:%16s\r\n", Os::currTime(), hardware_id, p->header.dst_address, (char *)(&p->data[1]));
          //We are claiming an address and received an addressclaim packet, there are three cases
          if (p->header.dst_address == mac_address)
          {
            auto name_cmp_result = strncmp(mac_device_name, (char *)(&p->data[1]), RingNetworkProtocol::device_name_maxsize);
            if (name_cmp_result == 0)
            {
              //printf("[%5i] %i) Address claimed\r\n", Os::currTime(), hardware_id);
              //It's our packet that round-tripped the ring, the address is claimed
              pTxAction = PTxAction::SendFreePacket;
              mac_state = MacState::Idle;
            }
            else
            {
              //It's another device claiming the same address, the one with the 'lower' name wins
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
            //printf("[%5i] %i) Address claim of another device\r\n", Os::currTime(), hardware_id);
            //3. It's another device claiming another address, pass the message along
            pTxAction = PTxAction::PassAlongDecreasingTTL;
          }
          break;

        default:
          //Unknown protocol msgid, TODO signal
          if (p->header.dst_address == mac_address)
          {
            pTxAction = PTxAction::SendFreePacket;
          }
          else
          {
            pTxAction = PTxAction::PassAlongDecreasingTTL;
          }
          break;
        }
      }
      else
      {
        //It's a data packet, we don't have an address yet so we can only pass along if it's not for us
        //Note that if the address we are trying to claim is of someone else, we are disturbing his traffic here
        if (p->header.dst_address == mac_address)
        {
          pTxAction = PTxAction::SendFreePacket;
        }
        else
        {
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

void RingNetwork::tick(millisec64 timeDelta)
{
  mac_watcher_timeout -= timeDelta;
  if (mac_watcher_timeout < 0)
  {
    mac_watcher_timeout = 0;
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
      return;

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