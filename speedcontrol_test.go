package main
import (
	. "github.com/cyoung/rpi"
	"fmt"
	"time"
	"os/exec"
)
const pwm = PIN_GPIO_0
const rev = PIN_GPIO_3
const put = PIN_GPIO_2
var a, b, spd int
func main(){
	a=0
	b=10000
	spd = 1
	WiringPiSetup()
	exec.Command("gpio","mode","0","OUT").Run()
	exec.Command("gpio","mode","2","OUT").Run()
	exec.Command("gpio","mode","3","OUT").Run()
	PinMode(pwm, OUTPUT)
	PinMode(put, OUTPUT)
	PinMode(rev, OUTPUT)
	DigitalWrite(put, 1)
	DigitalWrite(rev, 0)
	fmt.Printf("Started")
	go run()
	for spd!=0{

//		fmt.Scanf("%d",&spd)
		Delay(500)
		spd++
		fmt.Printf("%d\n",spd)

		spdtopwm()
		fmt.Printf("ok\n")
	}
	DigitalWrite(pwm,0)
}

func run(){
		for true{
		DigitalWrite(pwm,1)
		time.Sleep(time.Duration(a) * time.Nanosecond)
		DigitalWrite(pwm,0)
		time.Sleep(time.Duration(b) * time.Nanosecond)
	}
}

func spdtopwm(){
	var t int
	if(spd>0){
		DigitalWrite(put, 1)
		DigitalWrite(rev, 0)
	}else{
		DigitalWrite(put, 0)
		DigitalWrite(rev, 1)
		spd = -spd
	}
	if(spd<10){
		a=1000
		b=300000-20000*spd
	}else if(spd>=10 && spd <40){
		t=spd-10
		a=1000
		b=100000-3000*t
	}else if(spd>=40 && spd <70){
		t=spd-40
		a=1000
		b=10000-100*t
	}else if(spd>=70 && spd <80){
		t=spd-70
		a=1000+500*t
		b=7000
	}else if(spd>=80 && spd <100){
		t=spd-80
		a=6100+90*t
		b=8000-90*t
	}else if(spd>=100 && spd <150){
		t=spd-100
		a=7000+60*t
		b=7100-2*t
	}else if(spd>150){
		t=spd-150
		a=10000
		b=7000-20*t
		if(b<0){
			b=0
		}
	}
}
