#include <stdio.h>
#include <wiringPi.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#define MAXP 5
#define MAXB 8
#define RESIST 10050
#define HP 5
#define DEBUG
#define TIMES 2000 //to fix millis issue
double speed = 0;
int notch = 0;
int location = 0;
double ampere = 0;
int kgh = 0;
int nonstop = 0;
int reverser = 0;
int sig = 0;
int flag = 0;
int limit = -1;
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
void sigSysChange(){
	if(reverser==0&&notch==-MAXB)sig++;
	if(sig>2)sig=0;
	return;
}
void sync(){
	
}
void ato(){
	if(notch==-MAXB)return;
	//decl
	if(flag&&nonstop){
		if(speed==0.0){
			notch=3;
		}
	}
	
}
void tasc(){
	if(notch==-MAXB)return;
}
void run(char * cmd){
	char* keyword = strtok(cmd," ");
	#ifdef DEBUG
	printf("%s",cmd);
	#endif
	if(!strcmp(keyword[1],"help")){
		printf("\nTrain Model for RaspberryPi help page\n\tFor futher information, visit page \'http://hyeonsoft.iptime.org/\'\n\n\thelp\tShows help\n\texit\tExit the program.\n");
	}else if(!strcmp(keyword[1],"exit")){
		printf("\nPress Esc key to exit.\n");	
	}else printf("\nCommands that you put \"%s\" in is not correct. type \"help\"to read help.",cmd);
	return;
}
void main(){
	void (*sp[2])(); //signal system pointer
	sp[0] = ato;
	sp[1] = tasc;
	char a = 0;
	char cmdline[100];
	int times = 1;
	int cmd = 0;
	while (1){
		//Put the code that finds train's location.
		if (millis() < 10 * times - 20){
			if (sig)sp[sig-1]();
			times=1;
			getpower();
		}
		if (millis() > 10 * times){
			if (sig)sp[sig-1]();
			getpower();
			times++;
			if(!cmd)printf("%d %c, %.1fkm/h, Notch: %d, DRIVING : %s @%d LIMIT: %dkm/h, %dloops\n",times, reverser == 0 ? 'N' : (reverser == 1 ? 'F' : 'R'), speed, notch, sig==0?"NONE":(sig==1?"ATO":"TASC"),flag,limit,nonstop);
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
						if(cmdline[0]!='\0')run(cmdline);
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
			case '2':
				sigSysChange();
				break;
			case ' ':
				if(flag)flag=0;else flag=1;
				break;
			case '-':
			case '_':
				if(nonstop)nonstop--;
				break;
			case '+':
			case '=':
				nonstop++;
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