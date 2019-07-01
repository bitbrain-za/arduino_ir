
#ifndef PWM_H_
#define PWM_H_

#define MAX_PULSES 0xFF

class PWM
{
public:
  static void init(void);
  static void On(void);
  static void Off(void);
  static void tx_code(int repeats, int length);
  static void stop(void);
  volatile static int index;
  static int wave_pulses;
  static uint16_t wave[MAX_PULSES];
  static volatile bool done;
  static volatile int count;
  static volatile int resends;

};

#endif