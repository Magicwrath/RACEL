#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double temperatura(void) {
  FILE* ft;
  char tekst[100];
  ft = fopen("/sys/bus/w1/devices/28-00000???????/w1_slave", "r");

  if(ft == NULL)
    return 0;

  int i = 0;
  for(i = 0; i < 22; i++)
    fscanf(ft, "%s", tekst);

  fclose(ft);

  //obrisati t
  for(i = 0; i < 10; i++)
    tekst[i] = tekst[i + 2];

  int temp = atoi(tekst);
  double tem = (double)temp / 1000;

  return tem;
};

void zapis(double temp) {
  FILE* log;

  log = fopen("log", "a+");
  if (log == NULL)
    return;

  fprintf(log, "\n%.3f", temp);
  fclose(log);
};

int main(void) {
  FILE* log;
  int cz = 0, j = 0;
  struct timespec ts1, ts2;
  long czas;

  log = fopen("log", "w");
  if(log == NULL)
    return 0;

  fprintf(log, "\n\t\t***Temperatura***\n Izmerena temperatura = %.3f \xC2 \xB0 C\t C\n", temperatura());
  fclose(log);

  printf("\n\nPocetna temp = %.3f \xC2 \xB0 C", temperatura());

  while(1) {
    j++;

    clock_gettime(CLOCK_REALTIME, &ts1);

    printf("\nMerenje br. %d.\tTemp = %.3f \xC2 \xB0 C", j, temperatura());

    clock_gettime(CLOCK_REALTIME, &ts2);

    czas = (ts2.tv_nsec - ts1.tv_nsec) / 1000000;

    if(czas <= 0)
      czas = czas + 999;

    printf("\tTrajanje merenja: %ld ms", czas);

    zapis(temperatura());
  }

  return 0;
}
