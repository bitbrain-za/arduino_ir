#ifndef COMMS_H_
#define COMMS_H_

/*

  CMD_ADD_REMOTE 
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

  CMD_ADD_PREDATA
  [0]   id
  [1..N]  predata

  CMD_ADD_CODE,
  [0]   id
  [1]   code id
  [2..N]  code

  CMD_SEND
  [0] id
  [1] code_id
  [2] repeats

  CMD_SEND_CONTINOUS
  [0] id
  [1] code_id
  [2] time ms

  CMD_STOP
  none

*/

typedef enum Commands
{
  CMD_SEND = 0x04,
  CMD_STOP = 0x06,
  CMD_SEND_CODE = 0x07
};

class Comms
{
public:
  static void parse_serial();
  static void setCallback(void (*fp)(int, int));
  static void prints(const char *puc_fmt, ...);
  
private:
  static void (*callback_send_code)(int, int);

  static int ParseU8();
  static int ParseU16();
  static long ParseU32();
  static void ParseNewRemote();
  static void ParseNewCode();
  static void ParsePreData();
  static void ParseSend();
  static void ParseSendCode();

};

#endif
