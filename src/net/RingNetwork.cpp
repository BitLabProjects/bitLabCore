#include "RingNetwork.h"

RingNetwork::RingNetwork(PinName TxPin, PinName RxPin) : serial(TxPin, RxPin),
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
  serial.attach(callback(this, &RingNetwork::txIrq), Serial::TxIrq);
}

void RingNetwork::mainLoop()
{
  if (packetReceived()) {
    auto x = getPacket();
    
    //TODO Validate hash
    
    //For now, send every packet back to the requester
    auto tmp = x->header.src_address;
    x->header.src_address = x->header.dst_address;
    x->header.dst_address = tmp;
    x->header.ttl = ttl_max;
    sendPacket(x);

    receiveNextPacket();
  }
}

void RingNetwork::sendPacket(const RingPacket* packet) {
  if (tx_state != TxState::TxIdle) {
    //TODO Signal error
    return;
  }

  //Note that escaping is handled by txIrq
  uint8_t* dst_tx = tx_packet;
  *dst_tx++ = packet->header.data_size;
  *dst_tx++ = packet->header.control;
  *dst_tx++ = packet->header.src_address;
  *dst_tx++ = packet->header.dst_address;
  *dst_tx++ = packet->header.ttl;
  for(uint8_t i=0; i<packet->header.data_size; i++) {
    *dst_tx++ = packet->data[i];
  }
  //TODO Calculate hash
  *dst_tx++ = 0;
  *dst_tx++ = 0;
  *dst_tx++ = 0;
  *dst_tx++ = 0;

  tx_packet_size = 5 + packet->header.data_size + 4;
  tx_state = TxState::SendStartByte;
}

void RingNetwork::tick(millisec64 timeDelta)
{
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
    return;

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