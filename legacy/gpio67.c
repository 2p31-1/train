#include <stdio.h>
#include <wiringPi.h>
#define PUT 15
#define REV 18
#define PWM 14


int main (void)
{
	char in;
  if (wiringPiSetup () == -1)
  return 1 ;

	pinMode (PUT, 1) ;
	pinMode(PWM,1);
	pinMode(REV,1);
	digitalWrite(PWM,0);
  while(1){
	  printf("\nCOMMAND > ");
	  scanf("%s",&in);
	  if(in=='h'||in=='H'){
		  printf("\n\nThis is the help page of this program.\nPress z to turn the motor on.\nPress x to turn the motor off.\nPress c to close the program.");
	  }else if(in=='z'||in=='Z'){
		  printf("\nTurn the Motor on");
		  digitalWrite(REV,0);
		  digitalWrite(PUT,1);
	  }else if(in=='x'||in=='X'){
		  printf("\nTurn the Motor off");
		  digitalWrite(PUT,0);
		  digitalWrite(REV,0);
	  }else if(in=='a'||in=='A'){
		  printf("\nreverse");
		  digitalWrite(REV,1);
		  digitalWrite(PUT,0);
	  }else if(in=='c'||in=='C'){
		  printf("\nBye");
		  break;
	  }
	  fflush(stdin);
  }
  return 0;
}