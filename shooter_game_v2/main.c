#include <stdint.h>
#include "PLL.h"
#include "UART.h"
#include "../kit/os.h"
#include "../kit/LCD.h"
#include <string.h>
#include "Peripherals.h"

#define UART0_RIS_R             (*((volatile uint32_t *)0x4000C03C))
#define UART0_ICR_R             (*((volatile uint32_t *)0x4000C044))
	
#define UART1_RIS_R             (*((volatile uint32_t *)0x4000D03C))
#define UART1_ICR_R             (*((volatile uint32_t *)0x4000D044))

/******************* Functions prototyping ***********************/

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void Sensors_Init(void);
void Gun_Init(void);
/**************** Semaphores Declearation ************************/
int32_t Mux_LCD;
int32_t Mux_FIS;
int32_t Mux_Hlth;

int32_t UART_Input;
int32_t New_FIS;
int32_t New_String;
int32_t New_hlt;


int32_t Per_SendHlth;  			//Binary semaphore 
int32_t Per_RSensor;
/***************** Global Variables ******************************/
volatile uint32_t Health=200;
int32_t GameRun=1;
#define H1(h)		(char)((h)/100) + '0'
#define H2(h)		(char)(((uint32_t)((h)%100)-(uint32_t)( (h)%10))/10) +'0'
#define H3(h)	  (char)((uint32_t) (h)%10) + '0'
char UART_Data[14];



int count0=0,count1=0,count2=0,count3=0,count4=0,count5=0,count6=0,count0_IN0=0,count1_IN=0,count2_IN=0,count0_IN1=0,count0_IN2=0,count0_IN3=0;
#define THREADFREQ 1000   // frequency in Hz of round robin scheduler
//This thread is called after the UART RX interrupt to FIll the Software FIFO
//From the Hardware UART FIFO
//Critical Section: IF Fill_Software_FIFO() suspended during its executions and
//					the UART RX interrupt occur it may change the content of the Hardware FIFO
//					caused different Software 
//					FIFO if Fill_Software_FIFO() is resumed
void Task0(){
	
	while(1){
		if(!GameRun){
			OS_Wait(&GameRun);
		}
		count0++;
		//OS_Wait(&Mux_FIS);
		//OS_Wait(&GameRun);
		OS_Wait(&UART_Input);
		//DisableInterrupts();		//Critical Section for the Hardware FIFO
		count0_IN0++;
		//Fill_Software_FIFO();
		UART_InString(UART_Data,14);
		count0_IN1++;
		OS_FIFO_Init();
		count0_IN2++;
		//EnableInterrupts();
		OS_Signal(&New_String);
		count0_IN3++;
		//EnableInterrupts();

	}
}
//This thread Reads from the Software FIFO and generate the String -Global varibale- then
//signal the LCD

void Task1(){
	/*
	while(1){
		if(!GameRun){
			OS_Wait(&GameRun);
		}
		count1++;
		//OS_Wait(&Mux_FIS);
		//OS_FIFO_Get();
	  //OS_Wait(&New_FIS);
	
		//DisableInterrupts();
		//count1_IN++;
		//UART_InString(UART_Data,15);
		//EnableInterrupts();
	//	OS_Signal(&Mux_FIS);
		//OS_Signal(&New_String);
		
	  OS_Suspend();
		
	}*/while(1){
	OS_Suspend();
	}
}

//this thread print the string-global varibale- on the LCD

void Task2(){
		
	while(1){
		if(!GameRun){
			OS_Wait(&GameRun);
		}
			count2++;
	   
		  OS_Wait(&New_String);
			OS_Wait(&Mux_LCD);
			count2_IN++;
		  // KIT_LCD_Clear_Screen();
			KIT_LCD_goto(1,1);
		  // DisableInterrupts();
		  //KIT_LCD_print("Hello");
			KIT_LCD_print(UART_Data);
	  //	EnableInterrupts();
			OS_Signal(&Mux_LCD);
			
	}
}









//Periodic main Task Runs Every 1200 ms UART TX Send my health value 
// to the oppenent's MC through Bluetooth Tx module

  
void Task4(){
	
	while(1){
		if(!GameRun){
			OS_Wait(&GameRun);
		}
		count4++;
    //OS_Wait(&GameRun);			//GameRun => 0 
														//GameRun => 1
		OS_Wait(&Per_SendHlth);
		//OS_Wait(&Mux_Hlth);
		char Hlth[]={'E','N','M','Y',' ','H','L','T','H',' ',H1(Health),H2(Health),H3(Health),'*'};
		count4++;
		UART_OutString(Hlth);
		//Health --;
		//OS_Signal(&Mux_Hlth);
		
	}
}
//Periodic main Task Runs Every 250 ms to check the sensors LDR (HEAD-ST-LEG)
// This task waits for Binary Semaphore 4 perodic event(Per_RSensor) 
// Update : (Health) -  Semaphore (GameRun) 

void Task5(){
	static uint32_t Read;
	while(1){
		if(!GameRun){
			OS_Wait(&GameRun);
		}
		count5++;
		OS_Wait(&Per_RSensor);
		Read=Sensor_Read();
		//OS_Wait(&Mux_Hlth);
		if (Read==1){ //Head shoot -> minimize Health per 90 
		Health-=90;
			OS_Signal(&New_hlt);
		//OS_Suspend();
		}     
		else if (Read==2){//ST shoot -> minimize Health per 30
			Health-=30;
			OS_Signal(&New_hlt);
			//OS_Suspend();
		}			
		else if (Read==3){//Leg shoot -> minimize Health per 10

			Health-=10;
			OS_Signal(&New_hlt);

			//OS_Suspend();			
		}	
		//else{
	//	}

		if (((Health ==0)||(Health>200))){
			Health=0;
			 Gun_Disable();
		   			GameRun=0;
					OS_Wait(&Mux_LCD);
			KIT_LCD_Clear_Screen();
			KIT_LCD_goto(1,7);
			KIT_LCD_print("Game");
			KIT_LCD_goto(2,7);
			KIT_LCD_print("Over");
			
		}
		
		//OS_Signal(&Mux_Hlth);
		//OS_Sleep(500);
	}
	
}


//
void Task6(void){
	
	while(1){
		if(!GameRun){
			OS_Wait(&GameRun);
		}
		count6++;
		OS_Wait(&New_hlt);
		OS_Wait(&Mux_LCD);
		
		
		KIT_LCD_goto(2,1);
		char Hlth[]={'M','Y',' ','H','L','T','H',' ',H1(Health),H2(Health),H3(Health),0};
		KIT_LCD_print(Hlth);
		
		OS_Signal(&Mux_LCD);
	}
	
}
//Idle Task


void Task3(){
	while(1){
		//WaitForInterrupt();
		count3++;
	}
}
int main(void){
//	DisableInterrupts();
	PLL_Init(Bus80MHz);
	OS_FIFO_Init();
	/*******************Hardware Initialzation**********************/
	Sensors_Init();
	Gun_Init();
	KIT_LCD_init();
	UART1_Init();
	KIT_LCD_Clear_Screen();
	Gun_Enable();
	/*******************Semaphores Initialzation********************/
  //OS_InitSemaphore(&GameRun,1);
	OS_InitSemaphore(&Mux_LCD,1);
	OS_InitSemaphore(&Mux_FIS,1);
	OS_InitSemaphore(&Mux_Hlth,1);
	OS_InitSemaphore(&UART_Input,0);
	OS_InitSemaphore(&New_FIS,0);
	OS_InitSemaphore(&New_String,0);
	OS_InitSemaphore(&Per_SendHlth,0);     
	OS_InitSemaphore(&Per_RSensor,0);
	OS_InitSemaphore(&New_hlt,1);
	OS_AddThreads(&Task0,0,&Task1,0,&Task2,0,&Task3,0,&Task4,0,&Task5,0,&Task6,0);
	OS_PeriodTrigger0_Init(&Per_SendHlth,3000);  // every 1200 ms
	OS_PeriodTrigger1_Init(&Per_RSensor,250) ;     //every 250ms 
	
	OS_Launch(OS_Clock_GetFreq()/THREADFREQ); // doesn't return, interrupts enabled in here
	
  while(1){}
}







// at least one of three things has happened:
// hardware TX FIFO goes from 3 to 2 or less items
// hardware RX FIFO goes from 1 to 2 or more items
// UART receiver has timed out
void UART1_Handler(void){
 /* if(UART1_RIS_R&UART_RIS_TXRIS){       // hardware TX FIFO <= 2 items
    UART1_ICR_R = UART_ICR_TXIC;        // acknowledge TX FIFO
    // copy from software TX FIFO to hardware TX FIFO
    copySoftwareToHardware();
    if(TxFifo_Size() == 0){             // software TX FIFO is empty
      UART1_IM_R &= ~UART_IM_TXIM;      // disable TX FIFO interrupt
    }
	
  }*/
	 static int readback;
	 static int count=0;
	readback=UART1_RIS_R;
		
  if(UART1_RIS_R&UART_RIS_RXRIS){       // hardware RX FIFO >= 2 items
   
    // copy from hardware RX FIFO to software RX FIFO
		
    Fill_Software_FIFO();
		count++;
		if(count==7){
		OS_Signal(&UART_Input);count=0;}
		 UART1_ICR_R = UART_ICR_RXIC;        // acknowledge RX FIFO
	
  }
 /* if(UART1_RIS_R&UART_RIS_RTRIS){       // receiver timed out
   
    // copy from hardware RX FIFO to software RX FIFO
    //copyHardwareToSoftware();
		OS_Signal(&UART_Input);
		UART1_ICR_R = UART_ICR_RTIC;        // acknowledge receiver time out
		
  }*/
	UART1_ICR_R|=UART1_RIS_R;
	readback=UART1_RIS_R;
}





