#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <softPwm.h>
#include <iostream>
#include <thread>
#define PWM 0
#define REV 3
#define PUT 2
using namespace std;

void func1(){
	for(int x=1;x<100;x++){
		cout<<"hi"<<endl;
		delay(100);
	}
}
void func2(){
	getchar();
}

/*void barus1(int bandwidth,double percent){
	digitalWrite(PWM,1);
	int on=(int)(((double)bandwidth/100)*percent);
	usleep(on);
	cout<<"barus 1\ts\t"<<on<<endl;
	unsigned a=millis();
	digitalWrite(PWM,0);
	usleep(bandwidth-on);
	a=millis();
	cout<<"barus 1\tg\t"<<bandwidth-on<<endl;
}*/

int main(){
	pinMode(PWM,OUTPUT);
	pinMode(PUT,OUTPUT);
	pinMode(REV,OUTPUT);
	if (wiringPiSetup () == -1){
		printf("initializing error\n");
		return 1;
	}
	digitalWrite(PUT,1);
	digitalWrite(REV,0);
	digitalWrite(PWM,0);
	int t,p;
	cin>>t;
	cin>>p;
	unsigned int a=millis();
	unsigned int start=millis();
/*	while(p>percent){
		
	}*/
	while(1){
		if(p>2000)p--;
		digitalWrite(PWM,1);
		usleep(t);
		cout<<a<<"\ts\t"<<t<<endl;
		a=millis();
		digitalWrite(PWM,0);
		usleep(p);
		a=millis();
		cout<<millis()-start<<"\tg\t"<<p<<endl;
	}
	pinMode(PWM,1);
	pinMode(PUT,1);
	digitalWrite(PWM,1);
	digitalWrite(PUT,1);
	delay(10000);
	return 0;
}
