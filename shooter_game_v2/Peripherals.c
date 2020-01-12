
#include <stdint.h>
#include "../../tm4c123gh6pm.h"


#include "Peripherals.h"


//sensors pins 
// Head Enable PB2 , ST Enable PB3 , Leg Enable PB4
void Sensors_Init(void){ volatile unsigned long delay;
	  SYSCTL_RCGC2_R |= 0x00000002;     //  B clock
  delay = SYSCTL_RCGC2_R;           //  delay   
 // GPIO_PORTB_CR_R |= 0x1C;           //  allow changes to PB2,PB3,PB4       
  GPIO_PORTB_AMSEL_R = 0x00;        //  disable analog function
  GPIO_PORTB_PCTL_R = 0x00000000;   //  GPIO clear bit PCTL  
  GPIO_PORTB_AFSEL_R = 0x00;        //  no alternate function
	GPIO_PORTB_DIR_R &= ~0x1C;          //  PB2,PB3,PB4  input  
  GPIO_PORTB_PDR_R |= 0x1C;          //  enable pullup resistors on  PB2,PB3,PB4    
  GPIO_PORTB_DEN_R |= 0x1C;          //  enable digital pins PB2,PB3,PB4 
	
}
//this Function Checks Head-ST-Leg LDR sensors and
// Returns  with (uint32)     0 => No Hit
//														1 => Head-Shot
//														2 => ST-Shot
//														3 => Leg-Shot
// Sensors  Head PB2 , ST PB3 , Leg PB4
uint32_t Sensor_Read(void){
	static uint32_t New1=0,New2=0,New3=0,Last1=0,Last2=0,Last3=0;
	New1= (GPIO_PORTB_DATA_R & 0x04)>>2;
	New2=(GPIO_PORTB_DATA_R & 0x08) >>3;
	New3=  (GPIO_PORTB_DATA_R & 0x10) >>4;
	if( (Last1==0) && (New1==1)){ 
	  Last1=1;
		return 1;
	}
	
	if((Last2==0) && (New2==1)) {
		Last2=1;
		return 2 ;
		
	}
	
	if((Last3==0) && (New3==1)){
     Last3=1;
		return 3 ;
	}
	if( New1==0)Last1=0;
	if( New2==0)Last2=0;
	if( New3==0)Last3=0;
	return 0;
}


//GUN Enable PF2
void Gun_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     //  F clock
  delay = SYSCTL_RCGC2_R;           //  delay   
  //GPIO_PORTF_CR_R |= 0x04;           //  allow changes to PF2       
  GPIO_PORTF_AMSEL_R |= 0x00;        //  disable analog function
  GPIO_PORTF_PCTL_R |= 0x00000000;   //  GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R |= 0x04;          //  PF2 input  
  GPIO_PORTF_AFSEL_R |= 0x00;        //  no alternate function
 // GPIO_PORTF_PUR_R |= 0x04;          //  enable pullup resistors on  PF2   
  GPIO_PORTF_DEN_R |= 0x04;          //  enable digital pins PF2
	GPIO_PORTF_DR8R_R|=0x04;
	
}
//This function Enables the Gun
//                       
//                       
void  Gun_Enable(void){
 GPIO_PORTF_DATA_R |= 0x04;

}

	//This function Disbles the Gun

void  Gun_Disable(void){
	GPIO_PORTF_DATA_R &= ~0x04;
}

