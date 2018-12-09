#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <softPwm.h>
#define PWM 0
#define REV 3
#define PUT 2
#define CHOP 150
void wr(int pin,int spd){
	spd+=13;
	softPwmWrite(pin,spd);
	return;
}
int main (void)
{
	int i=0;
	char a[50];
	int pwm=PWM;
	int chop=CHOP;
//	scanf("%d",&pwm);
	pinMode(pwm, OUTPUT);
	pinMode(PUT, OUTPUT);
	pinMode(REV, OUTPUT);
	softPwmCreate(pwm, 0, 150);
	if (wiringPiSetup() == -1){
		printf("initializing error\n");
		return 1;
	}

	while(1){
		scanf("%s", a);
		printf("%s\n", a);
		if(*a == 'a'){
			int p=0;
			while(p<150){
				p++;
				delay(500);
				printf("%d\n",p);
				if(p>0){
					digitalWrite(PUT,1);
					digitalWrite(REV,0);
				}else if(p<0){
					digitalWrite(PUT,0);
					digitalWrite(REV,1);
				}
				wr(pwm,abs(p)<=chop-13?abs(p):chop-13);
			}
			delay(10000);
			while(p>0){
				p--;
				delay(300);
				printf("%d\n",p);
				if(p>0){
					digitalWrite(PUT,1);
					digitalWrite(REV,0);
				}else if(p<0){
					digitalWrite(PUT,0);
					digitalWrite(REV,1);
				}
				wr(pwm,abs(p)<=chop-13?abs(p):chop-13);
			}
		}else if(*a=='p'){
			scanf("%d",&chop);
			softPwmCreate(pwm,0,chop);
		}else if(*a=='s'){
			softPwmWrite(pwm,1);
		}else if(*a=='q'){
			int p;
			scanf("%d",&p);
			if(p>0){
					digitalWrite(PUT,1);
					digitalWrite(REV,0);
				}else if(p<0){
					digitalWrite(PUT,0);
					digitalWrite(REV,1);
				}
			softPwmWrite(pwm,p);
		}else if(*a=='w'){
			softPwmWrite(pwm,0);
		}else if(*a=='e'){
			break;
			exit(0);
		}else{
			int t=atoi(a);
			if(t>0){
				digitalWrite(PUT,1);
				digitalWrite(REV,0);
			}else if(t<0){
				digitalWrite(PUT,0);
				digitalWrite(REV,1);
			}
			wr(pwm,abs(t)<=chop-13?abs(t):chop-13);
		}
	}
	digitalWrite(REV,0);
	digitalWrite(PUT,0);
	digitalWrite(pwm,0);

	return 0 ;
}
