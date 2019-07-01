
#include "Arduino.h"
#include "remote.h"
#include "pwm.h"
#include "comms.h"

// fClock = 16MHz
// fIR = 38kHz
// fTimer = 16MHz
// Ideal OCRA timer = 421,05263157894736842105263157895 - 1
// OCR1 = 0x1A4 (420)
// fPWM = 38,0048kHz
// Measured: 38.0039kHz

volatile int PWM::index;
int PWM::wave_pulses;
uint16_t PWM::wave[MAX_PULSES];
volatile bool PWM::done;
volatile int PWM::count;
volatile int PWM::resends;

void PWM::init(void)
{
  // 16MHz / 16 = 1MHz
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS21);
  OCR2A = 15;

  cli();
  TIMSK2 |= (1<<OCIE2A);
  sei();

  pinMode(10, OUTPUT);   // PB2  - OC1B

  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);     // FAST PWM - Top = OCRA
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);     // /1 Prescaler
  OCR1AH = 0x01;
  OCR1AL = 0xA4;

  done = true;
  index = 0;
  wave_pulses = 0;
  Off();
}

void PWM::On(void)
{
  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);     // FAST PWM - Top = OCRA
  OCR1BH = 0x00;
  OCR1BL = 0xD2;
}

void PWM::Off(void)
{
  TCCR1A = _BV(WGM11) | _BV(WGM10);     // FAST PWM - Top = OCRA
  OCR1BH = 0x00;
  OCR1BL = 0x00;
}

void PWM::tx_code(int repeats, int length)
{
  index = 0;
  count = 1;
  done = false;
  wave_pulses = length;
  resends = repeats;
}

void PWM::stop(void)
{
  cli();
  resends = 0;
  sei();
}

ISR(TIMER2_COMPA_vect)
{
  static bool mark = true;
  char string[0xff];

  if(!PWM::done)
  {
    if(--PWM::count == 0x00)
    {
      PWM::count = PWM::wave[PWM::index++] / 8;
      if(PWM::count > 0)
      {
        if(mark)
        {
          // Comms::prints("Mark: %u", PWM::count * 8);
          PWM::On();
        }
        else
        {
          // Comms::prints("Space: %u", PWM::count * 8);
          PWM::Off();
        }
      }
      mark = !mark;

      if(PWM::index == PWM::wave_pulses)
      {
        mark = true;
        PWM::index = 0;
        if(PWM::resends-- == 0)
          PWM::done = true;
      }
    }
  }
}
