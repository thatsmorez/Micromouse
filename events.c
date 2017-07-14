/** ##########################################################
**     Filename  : Events.c
**     Project   : Project_5
**     Processor : MC9S12C32MFA25
**     Component : Events
**     Version   : Driver 01.04
**     Compiler  : CodeWarrior HC12 C Compiler
**     Date/Time : 2/15/2017, 6:23 PM
**     Abstract  :
**         This is user's event module.
**         Put your event handler code here.
**     Settings  :
**     Contents  :
**         No public methods
**
** ############################################################*/
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"

/* User includes (#include below this line is not maintained by Processor Expert) */

#pragma CODE_SEG DEFAULT

//Declaration of Global Variables
int toggle1, toggle2;  
extern byte motorSeq[8];  
extern int motorL;
extern int motorR;	
extern int stepsL;	
extern int stepsR;	
extern int numStep;	
extern byte sensors[3];	
int error;	
float k = 200;	

//sends data to the right motor
void motorR_WriteBits(byte value){  
  MotorDriveR_PutVal(value);	

}

//sends data to the left motor
void motorL_WriteBits(byte value){  
  MotorDriveL_0_PutVal((value & 0x01) && 0x01);  
  MotorDriveL_1_PutVal((value & 0x02) && 0x02);  
  MotorDriveL_2_PutVal((value & 0x04) && 0x04);  
  MotorDriveL_3_PutVal((value & 0x08) && 0x08);  
}

/* When this interrupt is called by the timer, the right motor moves to fit with its location in the motor array */
void MotorR_Timer2_OnInterrupt(void)
{
  if(toggle2){  
   motorR_WriteBits(0); 
   toggle2 = 0; 
  } else {
   motorR_WriteBits(motorSeq[motorR]);
   toggle2 = 1; 
  }
}

/*When this interrupt is called by the timer, the leftmotor moves to fit with its location in the motor array */
void MotorL_Timer2_OnInterrupt(void)
{
  if(toggle1){
   motorL_WriteBits(0);
   toggle1 = 0; 
  } else {
   motorL_WriteBits(motorSeq[motorL]);
   toggle1 = 1; 
  }
}

//Moves the left motor’s position in the motor sequence array
void MotorL_DriveTimer_OnInterrupt(void)
{
    if(motorL == 7){
      motorL = 0;
    } else{ 
        motorL++;
    }
  }

//Moves the right motor’s position in the motor sequence array
void MotorR_DriveTimer_OnInterrupt(void)
{
    if(motorR == 0){
      motorR = 7;
    } else{ 
        motorR--;
    } 
    numStep--;
  }
 
void Sensors_OnEnd(void)
{
  
}
/*
** ===================================================================
**     Event       :  PIDTimer_OnInterrupt (module Events)
**
**     Component   :  PIDTimer [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

//method for the PID Controller
void PIDTimer_OnInterrupt(void)
{   
     //if there is a right and left wall
     if(sensors[0] > 40 && sensors[2] > 40){
       error = (sensors[0]-sensors[2])/2;  
       error *= k;
     } else if(sensors[0] < 40){
       //If there is only a right wall
       //Pretend the left wall is there and calculate the error
       sensors[2] = 80;
      error = (sensors[0]-sensors[2])/2;  
       error *= k;
     } else{
      //If there is only a left wall
     //pretend the right wall is there and calculate the error
      sensors[0] = 80;
      error = (sensors[0]-sensors[2])/2;  
       error *= k;
     }
     
     if(error!=0) { 
      //if there is an error, both the right and left motors will speed up/slow down accordingly 
      MotorL_DriveTimer_SetPeriodUS(4000-error);  
      MotorR_DriveTimer_SetPeriodUS(4000+error);  
     } else{  
    //If there is not an error, both motors move at the same speed
      MotorR_DriveTimer_SetPeriodUS(4000);  
      MotorL_DriveTimer_SetPeriodUS(4000);  
     }
}

/* END Events */

/*
** ########################################################
**
**     This file was created by Processor Expert 3.02 [04.44]
**     for the Freescale HCS12 series of microcontrollers.
**
** #########################################################
*/
