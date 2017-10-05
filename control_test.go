package main
import (
	. "github.com/cyoung/rpi"
	"fmt"
	"time"
)
const pwm = PIN_GPIO_0
const rev = PIN_GPIO_3
const put = PIN_GPIO_2
var a, b int
func main(){
	a=0
	b=100
	WiringPiSetup()
	PinMode(pwm, OUTPUT)
	PinMode(put, OUTPUT)
	PinMode(rev, OUTPUT)
	DigitalWrite(put, 1)
	fmt.Printf("Started")
	go run()
	for a>=0{
		fmt.Scanf("%d %d",&a,&b)
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
