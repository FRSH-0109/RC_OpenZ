#define F_CPU 16000000UL
#define Channels 10
#define ChannelSerwo 1
#define ChannelMotor 2
#define ChannelArm	5
#define a -0.09387755102040816										//mniejszy zakres	//wspolczynnik kierunkowy dla obliczania pozycji serwa
#define b 72.08163265306122
#define Motor_forward1 PORTD2	
#define Motor_forward2 PORTD1
#define Motor_backward1 PORTD3
#define Motor_backward2 PORTD0