#include "Arduino.h"
#include "code.h"
#include "remote.h"
#include "comms.h"

#define MAX_REMOTES 4

static Remote* remotes[MAX_REMOTES] = { NULL, NULL, NULL, NULL };

int Remote::FindSlot()
{
  for(int i = 0 ; i < MAX_REMOTES ; i++)
  {
    if(remotes[i] == NULL)
      return i;
  }
  return 0xFF;
}

int Remote::NewRemote(RemoteInfo info)
{
  int slot = FindSlot();
  if(slot == 0xFF)
    return 0xFF;

  remotes[slot] = new Remote(info);
  if(remotes[slot] == NULL)
    return 0xFE;

  if(slot == 0xFF)
    Comms::prints("No free slots");
  else if(slot == 0xFE)
    Comms::prints("Not enough memory");
  else
    Comms::prints("Remote allocated to slot %u", slot);
  return slot;
}

Remote* Remote::FindRemote(int id)
{
  for(int i = 0; i < MAX_REMOTES ; i++)
  {
    if(remotes[i]->info.id == id)
      return remotes[i];
  }
  return NULL;
}

Remote::Remote(RemoteInfo init_info)
{
  info = init_info;  

  if(info.predata_size > 0)
  {
    predata = (uint8_t *)malloc(info.predata_size);
  }
  
  code = (uint8_t *)malloc(info.number_of_codes * info.code_size);

  if (code == NULL)
  {
    if(predata != NULL)
      free(predata);
  }

  PulseTrainLength = pulse_train_length();
}

uint16_t Remote::size_of_transmission(void)
{
  uint16_t result = info.predata_size + info.code_size;
}

uint16_t Remote::get_code(int index, uint8_t *result)
{
  if(result == NULL)
    return 0;

  if(index > info.number_of_codes)
    return 0;

  if(info.predata_size > 0)
    memcpy(result, predata, info.predata_size);

  memcpy(&result[info.predata_size], &code[index * info.code_size], info.code_size);

  return size_of_transmission();
}

void Remote::set_code(int index, uint8_t* data)
{
  if(index > info.number_of_codes)
    return;
  memcpy(&code[index * info.code_size], data, info.code_size);
}

void Remote::One(pulse *p)
{
  p->mark = info.one[0];
  p->space = info.one[1];
}

void Remote::Zero(pulse *p)
{
  p->mark = info.zero[0];
  p->space = info.zero[1];
}

int Remote::pulse_train_length()
{
  int length = 0;

  // Header
  length++;

  // Predata
  length += (8 * info.predata_size);

  //code
  length += (8 * info.code_size);

  //trail
  length++;

  return length;
}

uint32_t Remote::max_number_pulses(void)
{
  uint32_t duration = info.gap;
  uint32_t count = 0;
  duration -= header_duration();
  count++;

  // Predata
  for (int i = 0; i < info.predata_size ; i++)
  {
    for(int j = 0 ; j < 8 ; j++)
    {
      if((predata[i] & (0x80 >> j)) > 0)
      {
        duration -= one_duration();
      }
      else
      {
        duration -= zero_duration();
      }
      count++;
    }
  }

  for (int i = 0; i < info.code_size ; i++)
  {
    for(int j = 0 ; j < 8 ; j++)
    {
      duration -= zero_duration();
      count++;
    }
  }

  duration -= info.ptrail;
  count++;

  while((info.gap - duration) > (0xFFFF))
  {
    duration -= 0xFFFF;
    count++;
  }

  return count;
}

int Remote::one_duration(void)
{
  return info.one[0] + info.one[1];
}

int Remote::zero_duration(void)
{
  return info.zero[0] + info.zero[1];
}

int Remote::header_duration(void)
{
  return info.header[0] + info.header[1];
}

int Remote::load_code(int index, pulse* pulses)
{
  uint32_t duration = info.gap;
  int pulse_count = 0;

  pulses[pulse_count].mark = info.header[0];
  pulses[pulse_count].space = info.header[1];
  pulse_count++;
  duration -= header_duration();

  // Predata
  for (int i = 0; i < info.predata_size ; i++)
  {
    Comms::prints("Predata[%u] - %2X", i, predata[i]);
    for(int j = 0 ; j < 8 ; j++)
    {
      if((predata[i] & (0x80 >> j)) > 0)
      {
        One(&pulses[pulse_count]);
        duration -= one_duration();
      }
      else
      {
        Zero(&pulses[pulse_count]);
        duration -= zero_duration();
      }
      pulse_count++;
    }
  }

  for (int i = 0; i < info.code_size ; i++)
  {
    Comms::prints("Code[%u] - %2X", (index * info.code_size) + i, code[(index * info.code_size) + i]);
    for(int j = 0 ; j < 8 ; j++)
    {
      if((code[(index * info.code_size) + i] & (0x80 >> j)) > 0)
      {
        One(&pulses[pulse_count]);
        duration -= one_duration();
      }
      else
      {
        Zero(&pulses[pulse_count]);
        duration -= zero_duration();
      }
      pulse_count++;
    }
  }

  pulses[pulse_count].mark = info.ptrail;
  duration -= info.ptrail;
  Comms::prints("Pulses - %u", pulse_count);
  Comms::prints("Duration - %u", duration);
  Comms::prints("Gap - %u", info.gap - duration);

  while((info.gap - duration) > (0xFFFF))
  {
    pulses[pulse_count++].space = 0xFFFF;
    pulses[pulse_count].mark = 0;
    duration -= 0xFFFF;
  }

  pulses[pulse_count++].space = info.gap - duration;

  Comms::prints("Loaded:");

  Comms::prints("One: %u, %u", info.one[0], info.one[1]);
  Comms::prints("Zero: %u, %u", info.zero[0], info.zero[1]);

  for(int i = 0  ; i < pulse_count ; i++)
  {
    int mark = pulses[i].mark;
    int space = pulses[i].space;
    Comms::prints("%u - Mark: %u, Space: %u", i, mark, space);
  }

  return pulse_count;
}
