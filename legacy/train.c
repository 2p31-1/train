#include <stdio.h>
#include <wiringPi.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#define MAXP 5
#define MAXB 8
#define RESIST 10050
#define HP 5
#define DEBUG
#define TIMES 2000 //to fix millis issue
double speed = 0;
int notch = 0;
double ampere = 0;
int kgh = 0;
int reverser = 0;

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

void power(){
	if (notch < MAXP)notch++;
	return;
}
void neutr(){
	if (notch > 0)notch--;
	else if (notch < 0)notch++;
	return;
}
void brake(){
	if (notch>-MAXB + 1)notch--;
	return;
}
void powerbrake(){
	if (notch > 0){
		notch = -1;
		return;
	}
	if (notch > -MAXB + 1)notch--;
	return;
}
void emg(){
	notch = -MAXB;
	return;
}
void front(){
	if (reverser != 1)reverser++;
	return;
}
void back(){
	if (reverser != -1)reverser--;
	return;
}
double getAccel(double nowspeed){
	double a0[MAXP]={0.6,1.8,2.0,2.37,2.63};
	double a1[MAXP]={0.6,1.8,2.0,2.37,2.63};
	double v1[MAXP]={4.0,19.0,40.0,42.0,38.0};
	double v2[MAXP]={4.0,19.0,40.0,60.0,80.0};
	double e[MAXP]={2.5,2.0,2.3,2.4,1.7};
	if(fabs(nowspeed)>=0&&fabs(nowspeed)<=v1[notch-1])return a0[notch-1]+(a1[notch-1]-a0[notch-1])*fabs(nowspeed)/v1[notch-1];
	if(fabs(nowspeed)>v1[notch-1]&&fabs(nowspeed)<=v2[notch-1])return v1[notch-1]*a1[notch-1]/fabs(nowspeed);
	if(fabs(nowspeed)>v2[notch-1])return v1[notch-1]*a1[notch-1]* pow(v2[notch-1],e[notch-1]-1)/ pow(fabs(nowspeed),e[notch-1]);
	return 0.0;
}
double getBrake(){
	double decel=4.3;//Maximum deceleration = x km/h/s
	double stage=decel/(double)MAXB;
	return stage*notch;
}
void getpower(){
	double temp=0.0;
	double tmpspd=speed;
	double trackresist=(0.07+fabs(speed)*0.002)/TIMES;
	//check notch
	if(notch>0)temp=getAccel(tmpspd);
	if(notch<0)temp=getBrake();
	ampere=temp;
	temp /= TIMES;
	tmpspd+=temp*reverser;
	if(speed>0)tmpspd-=trackresist;
	if(speed<0)tmpspd+=trackresist;
	if(fabs(speed)<0.1&&notch<=0)tmpspd=0.0;
	speed = tmpspd;
	return;
}
void main(){
	
	char a = 0;
	int times = 1;
	while (1){
		if (millis() < 10 * times - 20){
			times=1;
			getpower();
		}
		if (millis() > 10 * times){
			getpower();
			times++;
		}
#ifdef DEBUG
		printf("%d %d %c, %.1fkm/h, notch : %d, Accel : %.4fkm/h/s, brake : %dkgh\n",millis(), times, reverser == 0 ? 'N' : (reverser == 1 ? 'F' : 'R'), speed, notch, ampere, kgh);
#endif
		if (kbhit()){
			a = getchar();
			switch (a){
			case 'Z':
			case 'z':
				power();
				break;
			case 'A':
			case 'a':
			case ',':
			case '<':
				neutr();
				break;
			case '.':
			case '>':
				powerbrake();
				break;
			case 'Q':
			case 'q':
				brake();
				break;
			case '1':
			case '/':
			case '?':
				emg();
				break;
			case ';':
			case ':':
				back();
				break;
			case '\'':
			case '\"':
				front();
				break;
			default:
				break;
			}
		}
		if (a == 27)break;
	}
	return;
}