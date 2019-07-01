#include "Arduino.h"
#include "code.h"
#include "remote.h"
#include "comms.h"
#include "pwm.h"

void (*Comms::callback_send_code)(int, int) = NULL;

void Comms::parse_serial()
{
  int i = 0;
  int rx_byte = 0;
  char cmd[5] = "0";

  if('S' != Serial.read())
    return;

  while(rx_byte != ',')
  {
    if(i >= 2)
    {
      Comms::prints("LEN error");
      return;
    }

    rx_byte = Serial.read();
    if(rx_byte > 0)
    {
      cmd[i++] = (char)(rx_byte);
    }
  }

  switch(atoi(cmd))
  {
    case CMD_SEND:
      ParseSend();
      break;
    case CMD_STOP:
      PWM::stop();
      break;
    case CMD_SEND_CODE:
      ParseSendCode();
      break;

    default:
      Comms::prints("Unknown command");
    return;
  }
}

int Comms::ParseU8()
{
  int rx_byte = 0;
  char data[3] = "00";
  int i = 0;

  while(rx_byte != ',')
  {
    if(i > 2)
    {
      return -1;
    }

    rx_byte = Serial.read();
    if(rx_byte >= 0)
      data[i++] = (char)(rx_byte);
  }

  return int(strtol(data, NULL, 16));
}

int Comms::ParseU16()
{
  int rx_byte = 0;
  char data[5] = "00";
  int i = 0;

  while(rx_byte != ',')
  {
    if(i > 4)
    {
      Comms::prints("failed to parse U16");
      return -1;
    }

    rx_byte = Serial.read();
    if(rx_byte >= 0)
      data[i++] = (char)(rx_byte);
  }

  return int(strtol(data, NULL, 16));
}

static long Comms::ParseU32()
{
  int rx_byte = 0;
  char data[9] = "00000000";
  int i = 0;

  while(rx_byte != ',')
  {
    if(i > 8)
      return -1;

    rx_byte = Serial.read();
    if(rx_byte >= 0)
      data[i++] = (char)(rx_byte);
  }

  return long(strtol(data, NULL, 16));
}

void Comms::ParseNewRemote()
{
  /*
  [0]     id 
  [1]     predata_size;
  [2]     code_size;
  [3]     number_of_codes;
  [4..5]    header;
  [6..7]    one;
  [8..9]    zero;
  [10..11]  ptrail;
  [12..13]  gap;
  [14]    use_repeat;
  [15..16]  repeat;
  */

  RemoteInfo newInfo;

  int val = 0;

  val = ParseU8();
  if(val < 0)
    return;
  newInfo.id = (uint8_t)val;

  val = ParseU8();
  if(val < 0)
    return;
  newInfo.code_size = (uint8_t)val;

  val = ParseU8();
  if(val < 0)
    return;
  newInfo.number_of_codes = (uint8_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.header[0] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.header[1] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.one[0] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.one[1] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.zero[0] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.zero[1] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.ptrail = (uint16_t)val;

  val = ParseU32();
  if(val < 0)
    return;
  newInfo.gap = (uint32_t)val;

  val = ParseU8();
  if(val < 0)
    return;
  newInfo.use_repeat = (val > 0);

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.repeat[0] = (uint16_t)val;

  val = ParseU16();
  if(val < 0)
    return;
  newInfo.repeat[1] = (uint16_t)val;

  Remote::NewRemote(newInfo);
}

void Comms::ParseNewCode()
{
  int id = 0;
  int code_id = 0;
  int val = 0;

  id = ParseU8();
  if(id < 0)
    return;

  code_id = ParseU8();
  if(code_id < 0)
    return;

  Remote* remote = Remote::FindRemote(id);

  uint8_t code[remote->info.code_size];

  for(int i = 0 ; i < remote->info.code_size ; i++)
  {
    val = ParseU8();
    if(val < 0)
      return;
    code[i] = (uint8_t)val;
  }
  remote->set_code(code_id, code);
}

void Comms::ParsePreData()
{
  int id = 0;
  int code_id = 0;
  int val = 0;

  id = ParseU8();
  if(id < 0)
    return;

  Remote* remote = Remote::FindRemote(id);

  uint8_t code[remote->info.predata_size];

  for(int i = 0 ; i < remote->info.predata_size ; i++)
  {
    val = ParseU8();
    if(val < 0)
      return;
    code[i] = (uint8_t)val;
  }
  memcpy(remote->predata, code, remote->info.predata_size);
}

void Comms::ParseSend()
{
  int id = 0;
  int val = 0;
  int resends = 0;

  id = ParseU8();
  if(id < 0)
    return;

  resends = ParseU8();
  if(resends < 0)
    return;  

  PWM::stop();
  callback_send_code(id, resends);
}

void Comms::ParseSendCode()
{

  int length = 0;
  int val = 0;
  int resends = 0;

  length = ParseU8();
  if((length < 0) || (length > MAX_PULSES))
    return;

  resends = ParseU8();
  if(resends < 0)
    return;

  PWM::stop();

  for(int i = 0 ; i < length ; i++)
  {
    val = ParseU16();
    if(val < 0)
      return;

    PWM::wave[i] = (uint16_t)val;
  }

  PWM::tx_code(resends, length);
}

void Comms::setCallback(void (*fp)(int, int))
{
  callback_send_code = fp;
}


void Comms::prints(const char *puc_fmt, ...)
{
  char print_buffer[0xFF];
  va_list argptr;

  va_start(argptr, puc_fmt);
  vsprintf(print_buffer, puc_fmt, argptr);
  va_end(argptr);

  Serial.println(print_buffer);
}