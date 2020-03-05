#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>

int main() {
  int i, taster_pritisnut = 0;
  int r_value;

  if(wiringPiSetuP() == -1)
    exit(1);

  pinMode(21, INPUT); //KEY0 na ploci -> ulaz
  pinMode(25, OUTPUT); //LED3 na ploci -> izlaz

  digitalWrite(25, HIGH); //LED3 ukljucen u startu

  if(softPwmCreate(28, 0, 100) != 0)
    exit(2);

  i = 0;
  taster_pritisnut = 0;

  while(1) {
    r_value = digitalRead(21);
    digitalWrite(25, r_value);

    if(!r_value && !taster_pritisnut) {
      taster_pritisnut = 1;

      i += 20;
      if(i > 100)
        i = 0;

      printf("i = %d\n", 1);
      fflush(stdout);

      softPwmWrite(28, i);
    }

    if(r_value)
      taster_pritisnut = 0;
  }

  return 0;
}
