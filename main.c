#include <avr/io.h>
#include <avr/interrupt.h>
#include "defines.h"
volatile uint16_t PPM_buffer [Channels*2-2] = {0};
volatile uint8_t  i = 0;
volatile uint8_t  j = 0;
volatile uint8_t PPM_start = 0;
uint16_t Serwo_average[5] = {0};
int16_t Motor_average[5] = {0};
uint8_t Serwo_counter_sum = 0;
uint8_t Motor_counter_sum = 0;
volatile int16_t MotorSpeed = 0;
volatile int16_t MotorSpeedTarget = 0;
volatile uint8_t direction = 0;

void Serwo_move();
void Motor_move();

int main(void)
{
	PLLCSR |= (1<< PINDIV) | (1<< PLLE);						//PLL input configurator dla 16Mhz HSE (1) oraz  PLL Enable
	PLLFRQ |= (1<< PLLTM1) | (1<<PDIV2);						//divider PLL na 1,5 oraz PLL Lock Frequency na 48Mhz co daje output na 32Mhz
	
	DDRD |= (1<<PORTD5) | (1<<PORTD7) | (1<<Motor_forward1) | (1<<Motor_forward2) | (1<<Motor_backward1) | (1<<Motor_backward2);
	PORTC &= ~(1<<PORTC6) & (1<<PORTC7);
	PCICR |= (1<<PCIE0);
	PCMSK0 |= (1<<PCINT4);
	//PORTD |= (1<<PORTD5);
	
	DDRB |= (1<<PORTB5);
	
	TCCR3B |= (1<<WGM32) | (1<<CS31);				//Timer3 16 bit na presclaer /64, tryb CTC
	TIMSK3 |= (1<<OCIE3A);										//Wlaczenie przerwania na compare A
	OCR3A = 500;
	
	TCCR1B |= (1<<CS11) | (1<<CS10);				//Timer1 16 bit na presclaer /64
	TCNT1 = 0;										//Zerowanie (just in case)
	
	TCCR4B |= (1<<CS40) | (1<<CS41) | (1<<CS43);	//timer4 prescaler na /1024
	TC4H = 2;										//10 bit resolution (wlasciwie 9bit) dla OCR4C
	OCR4C = 112;									//Timer 4 zlicza do OCR4C
	OCR4D = 45;										// +- srodek serwa
	TCCR4C |= (1<<COM4D1) |  (1<< PWM4D);			//pwm D enable and compare outpou type
	
	sei();
	while (1)
	{
		if(PPM_buffer[ChannelArm*2]>300){
			Serwo_move();
			Motor_move();
			PORTD |= (1<<PORTD5);
		}
		else{MotorSpeed=0;PORTD &= ~(1<<PORTD5);}
	}
}
void Serwo_move()
{
	Serwo_average[Serwo_counter_sum] = PPM_buffer[ChannelSerwo*2]*a + b;
	TC4H = 0;
	OCR4D = (Serwo_average[0]+Serwo_average[1]+Serwo_average[2]+Serwo_average[3]+Serwo_average[4])/5;		//kiedy LOW dla PWM 50hz range 0-313
	Serwo_counter_sum++;
	if(Serwo_counter_sum==5){Serwo_counter_sum=0;}
}

void Motor_move()
{
	Motor_average[Motor_counter_sum] = 275-PPM_buffer[ChannelMotor*2];
	MotorSpeed = (Motor_average[0] + Motor_average[1] +Motor_average[2] + Motor_average[3] + Motor_average[4])/5;
	if(MotorSpeed>10){direction=1;if(MotorSpeed>100){MotorSpeed=100;};}
	else if(MotorSpeed<-10){direction=2;if(MotorSpeed<-100){MotorSpeed=-100;};MotorSpeed*=-1;}
	else{direction=0;}
	Motor_counter_sum++;
	if(Motor_counter_sum==5){Motor_counter_sum=0;}
	
}

ISR(PCINT0_vect)
{
	if(!(PINB & (1<<PINB4))){PPM_start=1;}
	if(PPM_start){
		if(TCNT1>500){i=0;}
		PPM_buffer [i] = TCNT1;
		TCNT1 = 0;
		i++;
		if(i==Channels*2-2){i=0;}
	}
}

ISR(TIMER3_COMPA_vect)
{
	if(j==0){
		if(direction==1){PORTD &= ~(1<<Motor_backward1) & ~(1<<Motor_backward2);PORTD |= (1<<Motor_forward1) | (1<<Motor_forward2);}
		else if(direction==2){PORTD &= ~(1<<Motor_forward1) & ~(1<<Motor_forward2);PORTD |= (1<<Motor_backward1) | (1<<Motor_backward2);}
		else{PORTD &= ~(1<<Motor_forward1) & ~(1<<Motor_forward2) & ~(1<<Motor_backward1) & ~(1<<Motor_backward2);}
		}
	if(j==MotorSpeed){PORTD &= ~(1<<Motor_forward1) & ~(1<<Motor_backward1);}
	j++;
	if(j==101){j=0;}
}


