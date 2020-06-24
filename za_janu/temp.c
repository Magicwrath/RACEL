#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/fcntl.h>

int main(int argc, char *argv[]) {
  int fd = -1, ret;

  char buffer[100];
  char *tmp1, tmp2[10], ch = 't';
  
  char dev_name[100];
  long value;
  double temperatura;
  int i,j;

  if(argc != 2) {
    perror("Greska pri pozivanju, format treba da bude: ./temp prag\n");
    exit(1);
  }

  if(wiringPiSetup() == -1)
    exit(1);

  pinMode(25, OUTPUT);

  long tmp = atoi(argv[1]);
  double threshold = (double) tmp;

  while(1) {
    fd = open("/sys/bus/w1/devices/28-00000a41dec3/w1_slave", O_RDONLY);
    ret = read(fd, buffer, sizeof(buffer));

    tmp1 = strchr(buffer, ch); //search for char 't' in buffer string
    sscanf(tmp1, "t=%s", tmp2); //parse the temperature value
    value = atoi(tmp2);
    temperatura = (double) value/1000;

  
    printf("Temperatura je %.3f\n", temperatura);

    if(temperatura > threshold)
      digitalWrite(25, HIGH);
    else
      digitalWrite(25, LOW);


    close(fd);
  }

  return 0;
}


