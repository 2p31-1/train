#include <stdio.h>
#include <wiringPi.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <softPwm.h>
#define MAXP 5
#define MAXB 8
#define RESIST 10050
#define HP 5
#define DEBUG
#define TICK 150 //150 is the best.
#define TIMES 100 //to fix millis issue
int sel=0;
typedef struct obj{
	double speed ;
	int notch ;
	int location ;
	double ampere ;
	int kgh ;
	int nonstop ;
	int reverser ;
	int sig ;
	int flag ;
	int limit ;
	int fPin; //Front direction output pin
	int pPin; //Previous direcion output pin
	int pwm; //pwm pin
	int staloc; //station arriving sensor pin
	int stastp; //station stop sensor pin
}train;

typedef struct dir{
	int gPin ; //Default joint position
	int bPin ; //Customed joint position
	int status; //1:customed 0:default
}joint;

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

void power(train* a){
	if (a->notch < MAXP)a->notch++;
	return;
}
void neutr(train* a){
	if (a->notch > 0)a->notch--;
	else if (a->notch < 0)a->notch++;
	return;
}
void brake(train* a){
	if (a->notch>-MAXB + 1)a->notch--;
	return;
}
void powerbrake(train* a){
	if (a->notch > 0){
		a->notch = -1;
		return;
	}
	if (a->notch > -MAXB + 1)a->notch--;
	return;
}
void emg(train* a){
	a->notch = -MAXB;
	return;
}
void front(train* a){
	if (a->reverser != 1)a->reverser++;
	return;
}
void back(train* a){
	if (a->reverser != -1)a->reverser--;
	return;
}
double getAccel(train* a){
	double nowspeed=a->speed;
	double a0[MAXP]={0.6,1.8,2.0,2.37,2.63};
	double a1[MAXP]={0.6,1.8,2.0,2.37,2.63};
	double v1[MAXP]={4.0,19.0,40.0,42.0,38.0};
	double v2[MAXP]={4.0,19.0,40.0,60.0,80.0};
	double e[MAXP]={2.5,2.0,2.3,2.4,1.7};
	if(fabs(nowspeed)>=0&&fabs(nowspeed)<=v1[a->notch-1])return a0[a->notch-1]+(a1[a->notch-1]-a0[a->notch-1])*fabs(nowspeed)/v1[a->notch-1];
	if(fabs(nowspeed)>v1[a->notch-1]&&fabs(nowspeed)<=v2[a->notch-1])return v1[a->notch-1]*a1[a->notch-1]/fabs(nowspeed);
	if(fabs(nowspeed)>v2[a->notch-1])return v1[a->notch-1]*a1[a->notch-1]* pow(v2[a->notch-1],e[a->notch-1]-1)/ pow(fabs(nowspeed),e[a->notch-1]);
	return 0.0;
}
double getBrake(train* a){
	double decel=4.3;//Maximum deceleration = x km/h/s
	double stage=decel/(double)MAXB;
	return stage*a->notch;
}
void getpower(train* a){
	if(a->notch==0&&a->speed==0)return;
	double temp=0.0;
	double tmpspd=a->speed;
	double trackresist=(0.07+fabs(a->speed)*0.002)/TIMES;
	//check notch
	if(a->notch>0)temp=getAccel(a);
	if(a->notch<0)temp=getBrake(a);
	a->ampere=temp;
	temp /= TIMES;
	tmpspd+=temp*a->reverser;
	if(a->speed>0)tmpspd-=trackresist;
	if(a->speed<0)tmpspd+=trackresist;
	if(fabs(a->speed)<0.1&&a->notch<=0)tmpspd=0.0;
	a->speed = tmpspd;
	return;
}
void sigSysChange(train* a){
	if(a->reverser==0&&a->notch==-MAXB)a->sig++;
	if(a->sig>2)a->sig=0;
	return;
}
void sync_tr(train* a){
	if(a->fPin>0)pwmtest(a->speed);
	return;
}
void wr(int pin,int spd){
	spd+=13;
	softPwmWrite(pin,spd);
	return;
}
int pwmtest (int a)
{
	int i=0;
	pinMode(0, OUTPUT) ;
	pinMode(2, OUTPUT);
	pinMode(3,OUTPUT);
		if(a>0){
			digitalWrite(2,1);
			digitalWrite(3,0);
		}else if(a<0){
			digitalWrite(2,0);
			digitalWrite(3,1);
		}
		wr(0,abs(a)<=150-13?abs(a):150-13);
}
void ato(train* a){
	if(a->notch==-MAXB)return;
	//decl
	if(a->flag&&a->nonstop){
		if(a->speed==0.0){
			a->notch=3;
		}
	}

}
void tasc(train* a){
	if(a->notch==-MAXB)return;
}
int setup(train * a){
	if (wiringPiSetup () == -1){
		printf("initializing error\n");
		return 1;
	}
	pinMode(a->fPin,OUTPUT);
	pinMode(a->pPin,OUTPUT);
	pinMode(a->pwm,OUTPUT);

	softPwmCreate(a->pwm,0,TICK);
	return 0;
}
void run(const char * cmd,train * trainz){
	char t[100];
	strcpy(t,cmd);
	char* keyword;
	keyword = strtok(t," ");
	#ifdef DEBUG
	printf("%s",t);
	#endif
	if(!strcmp(keyword,"help")){
		printf("\nTrain Model Running Program for RaspberryPi help page\n\n\tFor futher information, visit page \'http://hyeonsoft.iptime.org/\'\n\n\thelp\tShows help\n\texit\tExit the program.\n\ttrain (number)\tChange the train that you control to (number)\n");
		return;
	}else if(!strcmp(keyword,"exit")){
		printf("\nPress Esc key to exit.\n");
		return;
	}else if(!strcmp(keyword,"train")){
		int new;
		keyword = strtok(NULL," ");
		new=atoi(keyword);
		if(new>0&&new<=10){
			printf("Changed to %d",new);
			sel=--new;
			return;
		}
		else {
			if(!strcmp(keyword,"pin")){
				keyword = strtok(NULL," ");
				if(!strcmp(keyword,"plus")){
					keyword = strtok(NULL," ");
					new=atoi(keyword);
					if(new>=0){
						trainz->fPin = new;
						printf("\nTrain No.%d's front direction GPIO pin has been set to %d.",sel+1,new);
						setup(trainz);
						return;
					}
				}else if(!strcmp(keyword,"minus")){
					keyword = strtok(NULL," ");
					new=atoi(keyword);
					if(new>=0){
						trainz->pPin = new;
						printf("\nTrain No.%d's rear direction GPIO pin has been set to %d.",sel+1,new);
						setup(trainz);
						return;
					}
				}else if(!strcmp(keyword,"pwm")){
					keyword = strtok(NULL," ");
					new=atoi(keyword);
					if(new>=0){
						trainz->pwm = new;
						printf("\nTrain No.%d's speed control(PWM) GPIO pin has been set to %d.",sel+1,new);
						setup(trainz);
						return;
					}
				}
			}
		}
	}
	printf("\nCommands that you put in is not correct. type \"help\"to read help.");
	return;
}
void main(){
	void (*sp[2])(train*); //signal system pointer
	train i[10]={0};
	//decleard



	//###################### FOR TEST
	i[0].fPin=3;
	i[0].pPin=2;
	i[0].pwm=0;
	setup(&i[0]);
	pinMode(i[0].pwm,1);
	//######################
	int il;

	sp[0] = ato;
	sp[1] = tasc;
	char a = 0;
	char cmdline[100];
	int times = 1;
	int cmd = 0;
	while (1){
		//Put the code that finds train's location.
		if (millis() < 10 * times - 20){
			il=10;
			while(il--){
				getpower(&i[il]);
				if(i[il].sig)sp[i[il].sig-1](&i[il]);
				sync_tr(&i[il]);
			}
			times=1;
		}
		if (millis() > 10 * times){
			il=10;
			while(il--){
				getpower(&i[il]);
				if(i[il].sig)sp[i[il].sig-1](&i[il]);
				sync_tr(&i[il]);
			}
			times++;
			#ifndef DEBUG2
			if(!cmd)printf("@%d %d %c, %.1fkm/h, Notch: %d, DRIVING : %s @%d LIMIT: %dkm/h, %dloops\n",
			sel+1,times, i[sel].reverser == 0 ? 'N' : (i[sel].reverser == 1 ? 'F' : 'R'), i[sel].speed,
			i[sel].notch, i[sel].sig==0?"NONE":(i[sel].sig==1?"ATO":"TASC"),i[sel].flag,i[sel].limit,i[sel].nonstop);
			#endif
		}
		if (kbhit()&&cmd){
			a=getchar();
			if(cmd==-1){
				cmd=0;
			}else{
				switch(a){
					case '\n':
						printf("\n");
						cmdline[cmd-1]='\0';
						if(cmdline[0]!='\0')run(cmdline,&i[il]);
						cmd=-1;
						break;
					case 27:
						cmd=0;
						break;
					case 8:
						printf("%c",a);
						cmd--;
						break;
					default:
						cmdline[cmd++-1]=a;
						printf("%c",a);
						break;
				}
			}
		}
		if (kbhit()&&!cmd&&cmd!=-1){
			a = getchar();
			switch (a){
			case 'Z':
			case 'z':
				power(&i[sel]);
				break;
			case 'A':
			case 'a':
			case ',':
			case '<':
				neutr(&i[sel]);
				break;
			case '.':
			case '>':
				powerbrake(&i[sel]);
				break;
			case 'Q':
			case 'q':
				brake(&i[sel]);
				break;
			case '1':
			case '/':
			case '?':
				emg(&i[sel]);
				break;
			case ';':
			case ':':
				back(&i[sel]);
				break;
			case '\'':
			case '\"':
				front(&i[sel]);
				break;
			case '2':
				sigSysChange(&i[sel]);
				break;
			case ' ':
				if(i[sel].flag)i[sel].flag=0;else i[sel].flag=1;
				break;
			case '-':
			case '_':
				if(i[sel].nonstop)i[sel].nonstop--;
				break;
			case '+':
			case '=':
				i[sel].nonstop++;
				break;
			case '\n':
				cmd=1;
				printf("\nCMD > ");
				break;
			default:
				break;
			}
		}
		if (a == 27)break;
	}
	return;
}
