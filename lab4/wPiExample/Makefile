result=pwm
.PHONY=all
all: $(result)
%: %.o
	gcc -o $@ $< -lwiringPi -lpthread
%.o: %.c
	gcc -c $<
.PHONY=clean
clean:
	rm $(result) *.o
