all: mcode.c
	gcc mcode.c -o mcode -lwiringPi
clean:
	rm mcode
