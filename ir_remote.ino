#include "Arduino.h"
#include "code.h"
#include "remote.h"
#include "comms.h"
#include "pwm.h"

Remote* remote_tv;
Remote* remote_amp;

pulse* tx_wave;


void send_code_single(int id, int code_id, int resends);

void setup() 
{
  Serial.begin(9600);
  delay(5000);
  Comms::prints("Processor Started");
  Serial.println("Starting PWM");
  PWM::init();
  Serial.println("PWM started");

  Comms::setCallback(send_code_single);
}

void loop()
{
  if(Serial.available() > 0)
  {
    Comms::parse_serial();
  }
}

void send_code_single(int id, int resends)
{
  Comms::prints("Sending code %u, %u times", id, resends);

  for (int i = 0; i < 68; i++)
  {
    switch(id)
    {
      case AMP_POWER: PWM::wave[i] = pgm_read_word_near(KEY_AMP_POWER + i); break;
      case AMP_VOL_UP: PWM::wave[i] = pgm_read_word_near(KEY_AMP_VOL_UP + i); break;
      case AMP_MUTE: PWM::wave[i] = pgm_read_word_near(KEY_AMP_MUTE + i); break;
      case AMP_VOL_DOWN: PWM::wave[i] = pgm_read_word_near(KEY_AMP_VOL_DOWN + i); break;
      case AMP_DVD: PWM::wave[i] = pgm_read_word_near(KEY_AMP_DVD + i); break;
      case AMP_AUX: PWM::wave[i] = pgm_read_word_near(KEY_AMP_AUX + i); break;
      case AMP_CD: PWM::wave[i] = pgm_read_word_near(KEY_AMP_CD + i); break;
      case AMP_TAPE: PWM::wave[i] = pgm_read_word_near(KEY_AMP_TAPE + i); break;
      case AMP_TUNER: PWM::wave[i] = pgm_read_word_near(KEY_AMP_TUNER + i); break;
      case AMP_VIDEO1: PWM::wave[i] = pgm_read_word_near(KEY_AMP_VIDEO1 + i); break;
      case AMP_VIDEO2: PWM::wave[i] = pgm_read_word_near(KEY_AMP_VIDEO2 + i); break;
      case AMP_BASS: PWM::wave[i] = pgm_read_word_near(KEY_AMP_BASS + i); break;
      case TV_POWER: PWM::wave[i] = pgm_read_word_near(KEY_TV_POWER + i); break;
      case TV_VOL_UP: PWM::wave[i] = pgm_read_word_near(KEY_TV_VOL_UP + i); break;
      case TV_MUTE: PWM::wave[i] = pgm_read_word_near(KEY_TV_MUTE + i); break;
      case TV_VOL_DOWN: PWM::wave[i] = pgm_read_word_near(KEY_TV_VOL_DOWN + i); break;
      case TV_PLAY: PWM::wave[i] = pgm_read_word_near(KEY_TV_PLAY + i); break;
      case TV_PAUSE: PWM::wave[i] = pgm_read_word_near(KEY_TV_PAUSE + i); break;
      case TV_STOP: PWM::wave[i] = pgm_read_word_near(KEY_TV_STOP + i); break;
      case TV_UP: PWM::wave[i] = pgm_read_word_near(KEY_TV_UP + i); break;
      case TV_DOWN: PWM::wave[i] = pgm_read_word_near(KEY_TV_DOWN + i); break;
      case TV_LEFT: PWM::wave[i] = pgm_read_word_near(KEY_TV_LEFT + i); break;
      case TV_RIGHT: PWM::wave[i] = pgm_read_word_near(KEY_TV_RIGHT + i); break;
      case TV_ENTER: PWM::wave[i] = pgm_read_word_near(KEY_TV_ENTER + i); break;
    }
  }
  PWM::tx_code(resends, 68);
}