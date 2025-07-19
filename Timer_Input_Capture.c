// configure PC6 as output of TIM3 CH1
//PC6=Pin 37 , PB6=Pin 58
//Connect a wire between these two pins
// configure PB6 as input of TIM4 CH1
//Timer3 in output toggle mode which toggles at ever 1s and that we capture using TIM4
//With the captured input calculating the frequency of the signal
//1s delay-period=100,frequency=1.000000
//2s delay-period=500,frequency=2.000000
//Need change in ARR value of TIm3 for different delays
//In debug mode goto expressions and copy the frequency variable
//Press reset button of the board
#include<stdint.h>


uint32_t current;
uint32_t last=0;
uint32_t period;
uint32_t frequency;
uint32_t RPM;
float speed;
int wheel_circumference = 14;

int main(void)

{


//Step 1: GPIOC, GPIOB Clock Enable
// Clock Registers from RCC
uint32_t *pClkCtrlRegAHB1ENR =   (uint32_t*)0x40023830;//AHB1ENR for GPIO C, GPIO B.
uint32_t *pClkCtrlRegAPB1ENR =   (uint32_t*)0X40023840;//APB1ENR for TIM3 and TIM4


// PORTC MODER
uint32_t *pPortCModeReg = (uint32_t*)0x40020800;
//PORTB MODER
uint32_t *pPortBModeReg = (uint32_t*)0x40020400;

//PORTC->AFRL register.For pin 6 we need to use AFRL
uint32_t *pPortCAFRLReg = (uint32_t*) 0X40020820;
//PORTB Base address 0x4002 0400 ,AFRL Offset=0x20
//PORTB->AFRL Reg -PB6
uint32_t *pPortBAFRLReg = (uint32_t*)0x40020420;




// TIM3 Register values

uint32_t *pTIM3PSCReg =(uint32_t*)0X40000428;// PSC Register Address TIM3->PSC
uint32_t *pTIM3ARRReg =(uint32_t*)0X4000042C;// TIM3->ARR
uint32_t *pTIM3CCMR1Reg =(uint32_t*)0X40000418;// TIM3->CCMR1
uint32_t *pTIM3CCERReg =(uint32_t*)0X40000420;// TIM3->CCER
uint32_t *pTIM3CNTReg =(uint32_t*)0X40000424;// TIM3->CNT
uint32_t *pTIM3CR1Reg =(uint32_t*)0X40000400; //TIM3->CR1
//uint32_t *pTIM3CCR1Reg =(uint32_t*)0X40000434; //TIM3->CR1

//TIM4 Registers

uint32_t *pTIM4PSCReg =(uint32_t*)0X40000828;// PSC Register Address TIM4->PSC
uint32_t *pTIM4CCMR1Reg =(uint32_t*)0X40000818;// TIM4->CCMR1
uint32_t *pTIM4CCERReg =(uint32_t*)0X40000820;// TIM4->CCER
uint32_t *pTIM4CR1Reg =(uint32_t*)0X40000800; //TIM4->CR1
uint32_t *pTIM4SRReg =  (uint32_t*)0X40000810; // Timer 4 Status Register
uint32_t *pTIM4CCR1Reg =(uint32_t*)0X40000834;// TIM4->CCR1


*pClkCtrlRegAHB1ENR|= ( 3 << 1);// Enable GPIOB for Timer4 in input mode -PB6
//*pClkCtrlRegAHB1ENR|= ( 1 << 2);// Enable GPIOC for Timer3 in output mode -PC6
//TIM4 Channel 1 is the alternate function for PB6 pin
//TIM3 Channel 1 is the alternate function for PC6 pin


//In APB1 both Timer 3& 4 Connected
*pClkCtrlRegAPB1ENR|= ( 1 << 2);// Set bit 2 for TIM4 Enable
*pClkCtrlRegAPB1ENR|= ( 1 << 1);// Set bit 1 for TIM3 Enable


    // PORT C MODER setting for Alternate mode
 *pPortCModeReg &= ~( 3 <<12);//clearing bit 12 & 13 –PC6
 *pPortCModeReg |= ( 1 <<13);// set bit 13

 // PORT B MODER setting for Alternate mode
 *pPortBModeReg &= ~(3 <<12);//clearing bit 13 & 12 –PB6
 *pPortBModeReg |= (1 <<13);// set bit 13 for alternate function mode

//TIM3 is AF2 so we need to set the corresponding 4 bits in AFRL as : 0010

 *pPortCAFRLReg&= ~(0XF<<24);// clearing bit 24 to 27 –corresponding to pin 6
 *pPortCAFRLReg|=(1<<25);// setting bit 25 - 0010 for AF2

//TIM4_CH1 is AF2 so we need to set this 4 bits as : 0010
 *pPortBAFRLReg&= ~(0XF<<24);// clearing bit 27 to 24
 *pPortBAFRLReg|=(1<<25);// setting bit 25 -> 0010



//TIM3 CH1 in OUTPUT Toggle Mode
//Toggle CH1 when the counter value is 0
// system clock is 16 MHZ
    *pTIM3PSCReg = 1600-1; // 16 000 000/1600= 10 000
    *pTIM3ARRReg = 10000000-1; //10 000/10 000= 1 HZ


//capture/compare mode ,OC1M - Channel 1 bit 6 5 4 dedicated for channel 1
//In output compare mode we are setting channel 1 of TIM3in toggle mode
    *pTIM3CCMR1Reg &=~(7<<4);// Clearing bit 4,5,6
    *pTIM3CCMR1Reg |=(3<<4);//setting bit 4 & 5 -011 for toggle mode

    *pTIM4CCR1Reg=0; //Cleared
//For one channel 4 bits are dedicated. Here we are using channel 1 so set bit 0
//Capture compare channel 1 output enable by setting bit 0
    *pTIM3CCERReg |=(1<<0);


    *pTIM3CNTReg=0; //Clearing the CNT register
    *pTIM3CR1Reg|=(1<<0);// set bit 0 to enable TIM3

//******************************************************************************************************************
    //CR1 is the counter value transferred by the last input capture 1 event (IC1). The
    // TIMx_CCR1 register is read-only and cannot be programmed when it is in Input capture mode.
    //configure TIM4 Channel 1 in input mode

*pTIM4PSCReg = 16000-1;
//In input capture mode we are setting channel 1 of TIM4 in toggle mode
    *pTIM4CCMR1Reg &=~(7<<4);// Clearing bit 4,5,6
    *pTIM4CCMR1Reg =0X41;//bit 7:4 ->0100 ,bit 0=1 set CH1 to capture at every edge,input capture mode

    *pTIM4CCERReg=0X0B;//enable CH1 capture both edges
    *pTIM4CR1Reg|=(1<<0);// set bit 0 to enable TIM4

while(1)
{
	while(!(*pTIM4SRReg & (1<<1))){} //Wait until input edge captured,bit 1
	current = *pTIM4CCR1Reg;
	//printf("Edge capture: %ld\n",current);
	period = current - last;
	last = current;
	frequency =(100000/period);
	RPM = frequency*60/period;
	speed = (wheel_circumference*frequency*3.6)/period;
	//printf("Period: %ld\n",period);
	//printf("Frequency: %ld\n",frequency);
	last = current;
}
}



