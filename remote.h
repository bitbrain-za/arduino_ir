#ifndef REMOTE_H_
#define REMOTE_H_

typedef struct RemoteInfo
{
  uint8_t id;
  uint16_t header[2];
  uint16_t one[2];
  uint16_t zero[2];
  uint16_t ptrail;
  bool use_repeat;
  uint16_t repeat[2];
  uint32_t gap;
  uint8_t predata_size;
  uint8_t number_of_codes;
  uint8_t code_size;
};

typedef struct pulse
{
  uint16_t mark;
  uint16_t space;
};

class Remote
{
public:

  static Remote* FindRemote(int id);
  static int NewRemote(RemoteInfo info);
  static int FindSlot();

  Remote(RemoteInfo info);
  ~Remote();

  uint16_t size_of_transmission(void);
  uint16_t get_code(int index, uint8_t *result);
  void set_code(int index, uint8_t* data);

  int load_code(int index, pulse* pulses);
  uint32_t max_number_pulses(void);

  RemoteInfo info;
  uint8_t* predata;
  uint8_t* code;

  int PulseTrainLength;

private:
  void One(pulse *p);
  void Zero(pulse *p);

  int header_duration(void);
  int zero_duration(void);
  int one_duration(void);
  int pulse_train_length();
};

#endif
