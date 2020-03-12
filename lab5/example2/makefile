.PHONY:all
all: ds18b20

%: %.o
	gcc -o $@ $<

%.o: %.c
	gcc -c $<

.PHONY: clean
clean:
	rm ds18b20.o ds18b20
