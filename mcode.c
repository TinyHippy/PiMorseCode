#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPiI2C.h>
#include <stdbool.h>
#include <time.h>
#define MAX_LENGTH 100
#define LEDPIN 0
#define BTTNPIN 1
#define DIT 250

//lcd variables
int LCDAddr = 0x27;
int BLEN = 1;
int fd;

//hard coded morse code
int morseCode[36][5] = {
	{0,1,2,2,2}, //a
	{1,0,0,0,2}, //b
	{1,0,1,0,2}, //c
	{1,0,0,2,2}, //d
	{0,2,2,2,2}, //e
	{0,0,1,0,2}, //f
	{1,1,0,2,2}, //g
	{0,0,0,0,2}, //h
	{0,0,2,2,2}, //i
	{0,1,1,1,2}, //j
	{1,0,1,2,2}, //k
	{0,1,0,0,2}, //l
	{1,1,2,2,2}, //m
	{1,0,2,2,2}, //n
	{1,1,1,2,2}, //o
	{0,1,1,0,2}, //p
	{1,1,0,1,2}, //q
	{0,1,0,2,2}, //r
	{0,0,0,2,2}, //s
	{1,2,2,2,2}, //t
	{0,0,1,2,2}, //u
	{0,0,0,1,2}, //v
	{0,1,1,2,2}, //w
	{1,0,0,1,2}, //x
	{1,0,1,1,2}, //y
	{1,1,0,0,2}, //z
	{1,1,1,1,1}, //zero
	{0,1,1,1,1}, //one
	{0,0,1,1,1}, //two
	{0,0,0,1,1}, //three
	{0,0,0,0,1}, //four
	{0,0,0,0,0}, //five
	{1,0,0,0,0}, //six
	{1,1,0,0,0}, //seven
	{1,1,1,0,0}, //eight
	{1,1,1,1,0} //nine
};




void writeWord(int in){
	
	int temp = in;
	if(BLEN == 1){
		temp |= 0x08;
	}
	else{
		temp &= 0xF7;
	}
	wiringPiI2CWrite(fd, temp);
}

void sendCommand(int command){

	int buf;
	buf = command & 0xF0;
	buf |= 0x04;
	writeWord(buf);
	delay(2);
	buf &= 0xFB;
	writeWord(buf);

	buf = (command & 0x0f) << 4;
	buf |= 0x04;
	writeWord(buf);
	delay(2);
	buf &= 0xFB; 
	writeWord(buf);

}


void sendData(int data){
	

	int buf;
	buf = data & 0xF0;
	buf |= 0x05;
	writeWord(buf);
	delay(2);
	buf &= 0xFB;
	writeWord(buf);

	buf = (data & 0x0f) << 4;
	buf |= 0x05;
	writeWord(buf);
	delay(2);
	buf &= 0xFB; 
	writeWord(buf);

}

void init(){

	sendCommand(0x33);
	delay(5);

	sendCommand(0x32);
	delay(5);


	sendCommand(0x28);
	delay(5);


	sendCommand(0x0C);
	delay(5);


	sendCommand(0x01);
	wiringPiI2CWrite(fd, 0x08);
}


void clear(){
	sendCommand(0x01);
}


void write(int x, int y, char data[]){
	
	int addr, i; 
	int tmp;
	if (x < 0){
		x = 0;
	}
	if(x > 15) {
		x = 15; 
	}
	if( y < 0){
		y = 0;
	}
	if( y > 1){
		y = 1; 	
	}

	addr = 0x80  + 0x040 * y + x; 
	sendCommand(addr);

	tmp = strlen(data);
	for(i = 0; i < tmp; i ++){
		sendData(data[i]);
	}


}

void shortBlink() {	
	digitalWrite(LEDPIN, LOW);
//	printf("LED is on\n");
	delay(DIT);
	digitalWrite(LEDPIN, HIGH);
//	printf("LED is off\n");
	delay(DIT); 

}




void longBlink(){	
	digitalWrite(LEDPIN, LOW);
//	printf("LED is on\n");
	delay(DIT*3);
	digitalWrite(LEDPIN, HIGH);
//	printf("LED is off\n");
	delay(DIT); 

}



void blink(int in[]){
	for(int i = 0; i < 5; i ++){
		if(in[i] == 0){
			shortBlink();
		}
		else if(in[i] == 1){
			longBlink();
		}
	}

}


void displayCurrLetter(char in[]){
	clear();
	write(3,0, "Character:");
	write(7,1, in);

}

void blinkWord(char in[]){

	char tmp[3] = {' ', ' ', ' '};
	for(int i = 0; i < strlen(in); i++){
		
		tmp[0] = in[i];
		displayCurrLetter(tmp);
		//if capital blink it
		if(in[i] >=0x41 && in[i] <= 0x5A){

			blink(morseCode[in[i]-0x41]);
		}
		//else if lower case
		else if(in[i] > 0x5A){
			if((in[i] - 0x20) <= 0x5A){
				blink(morseCode[in[i]-(0x41 + 0x20)]);
			}
		}
		//else if number
		else if(in[i] >=0x30 && in[i] <= 0x39){
			printf("%d\n", in[i]-(0x30));
			blink(morseCode[in[i]-(0x30)+26]);
		}

		delay(DIT*3);
	}
	delay(DIT*7);
}

void greetings(){


	write(0,0, "Type a word or");
	write(1,1, "tap one.");

}

void farewell(){

	clear();
	write(0,0, "Program");
	write(1,1, "Completed");
	delay(9000);
	clear();
}

int detectInput(){
	int out = 0;
	bool hold = false;
	bool loop = true;
	clock_t start;
	clock_t end; 
	double elapsed;
	
	while(loop){

		if(digitalRead(BTTNPIN) == 0){
			digitalWrite(LEDPIN, HIGH);

			if(hold){

				hold = false;
				end = clock();
				elapsed = ((double)(end-start)) / CLOCKS_PER_SEC;
				printf("Button Held for: %f sec\n", elapsed);
				loop = false;
				if(elapsed > DIT){
					out = 1;
				}//if
			}//if

		}//if
		else{

			if(!hold){
				start = clock();
				hold = true; 
			}//if

			digitalWrite(LEDPIN, LOW);
		}//else

	}//while

	return out; 


}

char detectChar(){

	int tmp[5] = {2,2,2,2,2};
	char out = ' ';
	for(int i = 0; i < 5; i ++){

	}
	return out; 	

}



int main(void){
	
	char word[MAX_LENGTH];
	fd = wiringPiI2CSetup(LCDAddr);
	init();
	greetings();
	printf("Enter string to be converted to mores code\n");
	scanf("%99s", word);
	printf("Flashing mores code for %s\n", word);
	if(wiringPiSetup() == -1){
		printf("wiringPi Setup failed\n");
	}
	else {
		pinMode(LEDPIN, OUTPUT); //set LEDPIN to output to write value to it
		pinMode(BTTNPIN, INPUT);
		blinkWord(word);	
	}

	detectInput();

	farewell();
	return 0; 
}
