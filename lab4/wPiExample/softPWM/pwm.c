#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
char i;

int main() {
  wiringPiSetup();

  if(softPwmCreate(28, 0, 100) != 0)
    printf("Doslo je do greske!\n");

  while(1) {
    for (i = 0; i < 101; i++) {
      softPwmWrite(28, i);
      delay(10);
    }
  }
}
