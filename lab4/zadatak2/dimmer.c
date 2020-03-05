#include <wiringPi.h>
#include <stdio.h>
#include <softPwm.h>

#define DIMMER_STEP 20
#define PWM_RANGE 100

int main() {
  int i = 0, taster_pritisnut = 0;
  int key0, key1;

  if(wiringPiSetup() == -1)
    exit(1);

  pinMode(21, INPUT); //KEY0 ulaz
  pinMode(22, INPUT); //proveri da li je ovo KEY1 ulaz
  pinMode(25, OUTPUT); //LED3 izlaz
  pinMode(26, OUTPUT); //LED2 izlaz

  digitalWrite(25, HIGH);

  if(softPwmCreate(28, 0, PWM_RANGE) != 0)
    exit(2);

  i = 0;
  taster_pritisnut = 0;

  while(1) {
    key0 = digitalRead(21);
    key1 = digitalRead(22);
    digitalWrite(25, key0);
    digitalWrite(26, key1);

    if(!key0 && !key1 && !taster_pritisnut) {
      taster_pritisnut = 1;

      if(!key0) {
        if(i < 100) i += DIMMER_STEP;
      }

      if(!key1) {
        if(i > 0) i -= DIMMER_STEP;
      }

      printf("i = %d\n", i);
      fflush(stdout);

      softPwmWrite(28, i);
    }

    if(key0 && key1)
      taster_pritisnut = 0;
  }

  return 0;
}
