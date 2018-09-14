#include "RingNetwork.h"

RingNetwork::RingNetwork(PinName TxPin, PinName RxPin):
  serial(TxPin, RxPin), 
  tx_state(TxState::Idle),
  tx_packet_size(0), 
  tx_packet_idx(0)
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
}

void RingNetwork::tick(millisec64 timeDelta)
{
}

const uint8_t StartByte = 85;
const uint8_t EndByte = 170;
const uint8_t EscapeByte = 27;
inline bool byteNeedsEscaping(uint8_t value) {
  return value == StartByte || value == EndByte || value == EscapeByte;
}

void RingNetwork::txIrq()
{
  if (tx_state == TxState::Idle)
    return;

  while (serial.writeable()) {
    switch (tx_state) {
      case TxState::Idle:
        return;

      case TxState::SendStartByte:
        serial.putc(StartByte);
        tx_packet_idx = 0;
        tx_state = TxState::SendPacket;
      break;

      case TxState::SendPacket:
        if (tx_packet_idx >= tx_packet_size) {
          tx_state = TxState::SendEndByte;
        } else {
          uint8_t value = tx_packet[tx_packet_idx];
          if (byteNeedsEscaping(value)) {
            value = EscapeByte;
            tx_state = TxState::Escape;
          } else {
            tx_packet_idx += 1;
          }
          serial.putc(value);
        }
      break;

      case TxState::Escape:
        serial.putc(tx_packet[tx_packet_idx]);
        tx_packet_idx += 1;
        tx_state = TxState::SendPacket;
      break;

      case TxState::SendEndByte:
        serial.putc(EndByte);
        tx_state = TxState::Idle;
      break;
    }
  }
}

void RingNetwork::rxIrq()
{
}