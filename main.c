/** ###################################################################
**     Filename  : Project.c
**     Project   : Project
**     Processor : MC9S12C32MFA25
**     Version   : Driver 01.14
**     Compiler  : CodeWarrior HC12 C Compiler
**     Date/Time : 3/15/2017, 6:50 PM
**     Abstract  :
**         Main module.
**         This module contains user's application code.
**     Settings  :
**     Contents  :
**         No public methods
**
** ###################################################################*/
/* MODULE Project */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "MotorL_EN.h"
#include "MotorDriveL_0.h"
#include "MotorDriveL_1.h"
#include "MotorDriveL_2.h"
#include "MotorDriveL_3.h"
#include "MotorR_EN.h"
#include "MotorDriveR.h"
#include "MotorL_Timer2.h"
#include "MotorR_Timer2.h"
#include "MotorL_DriveTimer.h"
#include "MotorR_DriveTimer.h"
#include "Sensors.h"
#include "PIDTimer.h"
#include "PE7.h"
#include "PA0.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include <stdio.h>
#include <stdlib.h>


/* User includes (#include below this line is not maintained by Processor Expert) */


byte motorSeq[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
byte sensors[3];
int motorL = 0;
int motorR = 7;
int stepsR = 0;
int stepsL = 0;
const int max_number = 160;
const int finalX1 = 2;
const int finalX2 = 3;
const int finalY = 6;
int numStep = 400;
int distance[6][13];
int walls[6][13];
int direction;
int x;
int y;
int xMax = 6;
int yMax = 13;
int stack[100] = {0};
int stackcounter = 0;


/* Prototypes */
void power_motors(int power);
void steps(int motor);
void moveForward(void);
void turnLeft(void);
void turnRight(void);
void turnAround(void);
void pollSensors(void);
void initializeDistance(void);
void initializeWalls(void);
void updateWalls(void);
void updateDistance(const int, const int);
void makeMove(void);
void writeCom(byte command);
void InitializeLCD(void);
void writeData(byte data);
void clearLCD(void);
void displaySensors(void);
int getminDistance(int, int);
void displayStats();


void main(void)
{


  /* Write your local variable definition here */


  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/


  /* Write your code here */
  direction = 0;
  x = 0;
  y = 0;
  initializeDistance();
  initializeWalls();
  while(!((x == finalX1 || x == finalX2) && (y == finalY))){
    // disables motors
    MotorL_Timer2_Disable();
    MotorR_Timer2_Disable();
    MotorL_DriveTimer_Disable();
    MotorR_DriveTimer_Disable();
    // initializes then clears LCD
    InitializeLCD();
    clearLCD();
    // pollsSensors, update walls, and displays readings on LCD
    pollSensors();
    updateWalls();
    displayStats();
    // delay
    Cpu_Delay100US(5000);
    // enables the motors
    MotorL_Timer2_Enable();
    MotorR_Timer2_Enable();
    MotorL_DriveTimer_Enable();
    MotorR_DriveTimer_Enable();
    // updates the distances then moves accordingly
    updateDistance(x, y);
    makeMove();
    // delay
    Cpu_Delay100US(5000);
  } /* end while */


  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/


void makeMove(){
  int minNeighbor = getminDistance(x, y);
  // if micromouse is facing North
  if(direction == 0){
    //if no north walls
    if(!(walls[x][y] & 8)){
      // if distance in cell to north is = minNeighbor
      if(distance[x+1][y] == minNeighbor){
        // move to north cell then increment x value
        moveForward();
        direction = 0;
        x = x + 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    //if no west walls
    if(!(walls[x][y] & 1)){
      // if distance in cell to west is = minNeighbor
      if(distance[x][y - 1] == minNeighbor){
        // move to west cell and decrement y value
        turnLeft();
        moveForward();
        direction = 3;
        y = y - 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no East wall
    if(!(walls[x][y] & 4)){
      // if distance in cell to east is = minNeighbor
      if(distance[x][y+1] == minNeighbor){
        // move to east cell and increment y value
        turnRight();
        moveForward();
        direction = 1;
        y = y + 1;
        Cpu_Delay100US(40);
        return;
            }
    }
    // if no south wall
    if(!(walls[x][y] & 2)){
      // if distance in cell to south is = minNeighbor
      if(distance[x - 1][y]  == minNeighbor){
        // move to south cell and decrement x value
        turnAround();
        moveForward();
        direction = 2;
        x = x - 1;
        Cpu_Delay100US(40);
        return;
            }
    }
  } // end outer if
  //Micromouse is facing East
  if(direction == 1){
    // if no east wall
    if(!(walls[x][y] & 4)){
      // if distance in cell to east is = minNeighbor
      if(distance[x][y+1] == minNeighbor){
        // move to east cell and increment y value
        moveForward();
        direction = 1;
        y = y + 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no north wall
    if(!(walls[x][y] & 8)){
      // if distance in cell to north is = minNeighbor
      if(distance[x+1][y] == minNeighbor){
        // move to north cell then increment x value
        turnLeft();
        moveForward();
        direction = 0;
        x = x + 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no south wall
    if(!(walls[x][y] & 2)){
      // if distance in cell to south is = minNeighbor
      if(distance[x-1][y] == minNeighbor){
        // move to south cell and decrement x value
        turnRight();
        moveForward();
        direction = 2;
        x = x - 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no west wall
    if(!(walls[x][y] & 1)){
      // if distance in cell to west is = minNeighbor
      if(distance[x][y-1] == minNeighbor){
        // move to west cell and decrement y value
        turnAround();
        moveForward();
        direction = 3;
        y = y - 1;
        Cpu_Delay100US(40);
        return;
      }
    }
  }
  //Micromouse is facing South
  if(direction == 2){
    // if no south wall
    if(!(walls[x][y] & 2)){
      // if distance in cell to south is = minNeighbor
      if(distance[x-1][y] == minNeighbor){
        // move to south cell and decrement x value
        moveForward();
        direction = 2;
        x = x - 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no west wall
    if(!(walls[x][y] & 1)){
      // if distance in cell to west is = minNeighbor
      if(distance[x][y-1] == minNeighbor){
        // move to west cell and decrement y value
        turnRight();
        moveForward();
        direction = 3;
        y = y - 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no east wall
    if(!(walls[x][y] & 4)){
      // if distance in cell to east is = minNeighbor
      if(distance[x][y+1] == minNeighbor){
        // move to east cell and increment y value
        turnLeft();
        moveForward();
        direction = 1;
        y = y + 1;
        Cpu_Delay100US(40);
        return;
      }
    }
    // if no north wall
    if(!(walls[x][y] & 8)){
      // if distance in cell to north is = minNeighbor
      if(distance[x+1][y] == minNeighbor){
        // move to north cell then increment x value
        turnAround();
        moveForward();
        direction = 0;
        x = x + 1;
        Cpu_Delay100US(40);
        return;
      }
    }
  }
  //Micromouse is facing West
  if(direction == 3){
    // if no west wall
    if(!(walls[x][y] & 1)){
      // if distance in cell to west is = minNeighbor
      if(distance[x][y-1] == minNeighbor){
        // move to west cell and decrement y value
        moveForward();
        direction = 3;
        y = y - 1;
        Cpu_Delay100US(40);
        return;
      }
    } // end west if
    // if no north wall
    if(!(walls[x][y] & 8)){
      // if distance in cell to north is = minNeighbor
      if(distance[x+1][y] == minNeighbor){
        // move to north cell then increment x value
        turnRight();
        moveForward();
        direction = 0;
        x = x + 1;
        Cpu_Delay100US(40);
        return;
      }
    } // end north if
    // if no south wall
    if(!(walls[x][y] & 2)){
      // if distance in cell to south is = minNeighbor
      if(distance[x-1][y] == minNeighbor){
        // move to south cell and decrement x value
        turnLeft();
        moveForward();
        direction = 2;
        x = x - 1;
        Cpu_Delay100US(40);
        return;
      }
    } // end south if
    // if no east wall
    if(!(walls[x][y] & 4)){
      // if distance in cell to east is = minNeighbor
      if(distance[x][y+1] == minNeighbor){
        // move to east cell and increment y value
        turnAround();
        moveForward();
        direction = 1;
        y = y + 1;
        Cpu_Delay100US(40);
        return;
      }
    } // end east if
  }
} // end makeMove




void updateWalls(){
  int north, east, south, west;
  int tempwalls = 0;
  north = 8;
  east = 4;
  south = 2;
  west = 1;
  /* if facing NORTH */
  if (direction == 0){
    // if left sensor senses a wall to the west
    if (sensors[0] > 70) {
      tempwalls |= west;
    }
    // if front sensor senses a wall to the north
    if (sensors[1] > 70 ){
      tempwalls |= north;
    }
    // if right sensor senses a wall to the east
    if (sensors[2] > 70){
      tempwalls |= east;
    }
  }
  /* if facing EAST */
  if (direction == 1){
    // if left sensor senses a wall to the north
    if (sensors[0] > 70){
      tempwalls |= north;
    }
    // if front sensor senses a wall to the east
    if (sensors[1] > 70){
      tempwalls |= east;
    }
    // if right sensor senses a wall to the south
    if (sensors[2] > 70){
      tempwalls |= south;
    }
  } /* end east if */
  /* if facing SOUTH */
  if (direction == 2){
    // if left sensor senses a wall to the east
    if (sensors[0] > 70){
      tempwalls |= east;
    }
    // if front sensor senses a wall to the south
    if (sensors[1] > 70){
      tempwalls |= south;
    }
    // if right sensor senses a wall to the west
    if (sensors[2] > 70){
      tempwalls |= west;
    }
  } /* end south if */
  /* if facing WEST */
  if (direction == 3){
    // if left sensor senses a wall to the south
    if (sensors[0] > 70){
      tempwalls |= south;
    }
    // if front sensor senses a wall to the west
    if (sensors[1] > 70){
      tempwalls |= west;
    }
    // if right sensor senses a wall to the north
    if (sensors[2] > 70){
      tempwalls |= north;
    }
  } /* end west if */
  walls[x][y] = tempwalls;
} // end updateWalls


// gets the minimum distance to the center of the maze
int getminDistance(int row, int col){
  int minNeighbor = 100;
  // if there's no wall to the west
  if(!(walls[row][col] & 1)){
    // if value in west cell < minNeighbor
    if(distance[row][col - 1] < minNeighbor){
      // then change minNeighbor to that value in west cell
      minNeighbor = distance[row][col - 1];
    } // end inner if
  } // end outer if
  // if there's no wall to the north
  if(!(walls[row][col] & 8)){
    // if value in north cell is < minNeighbor
          if(distance[row + 1][col] < minNeighbor){
      // then change minNeighbor to that of the north cell
      minNeighbor = distance[row + 1][col];
          } // end inner if
  } // end outer if
  // if there's no wall to the south
  if(!(walls[row][col] & 2)){
    // if value of south cell is < minNeighbor
          if(distance[row - 1][col] < minNeighbor){
      // then change minNeighbor to that of south cell
      minNeighbor = distance[row - 1][col];
          } // end inner if
  } // end outer if
  // if there's no wall to the east
  if(!(walls[row][col] & 4)){
    // if value in east cell is < minNeighbor
          if(distance[row][col+1] < minNeighbor){
      // then chnage minNeighbor to that of east cell
      minNeighbor = distance[row][col + 1];
          } // end inner if
  } // end outer if
  //
  return minNeighbor;
} // end getminDistance




void updateDistance(row,col){
  int tempRow, tempCol;
  int stackcounter = 0;
  int minNeighbor;
  stack[stackcounter] = (col*10)+row;
  //Stackcounter always points to the top of the stack
  stackcounter = stackcounter + 1;
  while(stackcounter > 0){
    tempRow = stack[stackcounter-1]%10;
    tempCol = stack[stackcounter-1]/10;
    minNeighbor = getminDistance(tempRow,tempCol);
    stackcounter = stackcounter - 1;
    if(distance[tempRow][tempCol] != minNeighbor + 1){
      distance[tempRow][tempCol] = minNeighbor + 1;
      if(!(walls[tempRow][tempCol] & 1)){
        stack[stackcounter] = ((tempCol-1)*10)+tempRow;
        stackcounter = stackcounter + 1;
      } // end if
      if(!(walls[tempRow][tempCol] & 8)){
        stack[stackcounter] = (tempCol*10)+(tempRow+1);
        stackcounter = stackcounter + 1;
      } // end if
      if(!(walls[tempRow][tempCol] & 2)){;
              stack[stackcounter] = (tempCol*10)+(tempRow-1);
        stackcounter = stackcounter + 1;
      } // end if
      if(!(walls[tempRow][tempCol] & 4)){
              stack[stackcounter] = ((tempCol+1)*10)+tempRow;
        stackcounter = stackcounter + 1;
      } // end inner if statement
    } // end outer if statement
  } // end while
} // end updateDistance




void writeCom(byte command){
  PE7_PutVal(0);
  PA0_PutVal(0);
  MotorDriveR_PutVal(command>>4);
  PE7_PutVal(1);
  PE7_PutVal(0);
  MotorDriveR_PutVal(command);
  PE7_PutVal(1);
  PE7_PutVal(0);
  Cpu_Delay100US(1);
} // end writeCom


void InitializeLCD(void){
  Cpu_Delay100US(160);
  // Function Set S bit
  writeCom(0x30);
  // Wait 4.1 ms
  Cpu_Delay100US(50);
  // Function Set 8 bit
  writeCom(0x30);
  //Wait 100 microseconds
  Cpu_Delay100US(2);
  //Function set 4 bit
  writeCom(0x30);
  // Function Set
  writeCom(0x20);
  //Display On
  writeCom(0x28);
  writeCom(0x0C);
  //Display Clear
  writeCom(0x01);
  //Entry Mode Set
  writeCom(0x06);
  //Day to ensure that device was initialized properly
  Cpu_Delay100US(1);
} // end InitializeLCD


void writeData(byte data){
  //disabling LCD
  PE7_PutVal(0);
  //Selecting the Data Register
  PA0_PutVal(1);
  // puts upper nibble in the data bus
  MotorDriveR_PutVal((data>>4) & 0x0f);
  //strobes the Enable Line
  PE7_PutVal(1);
  PE7_PutVal(0);
  //Put lower nibble in the data bus
  MotorDriveR_PutVal(data & 0x0f);
  //strobes the Enable Line
  PE7_PutVal(1);
  PE7_PutVal(0);
  //Delay for observation
  Cpu_Delay100US(1);
} // end writeData


// displays the direction and position of the micromouse
// this function is mostly used for error checking when mouse is in maze
void displayStats(){
  // if mouse is facing north
  if(direction == 0){
    writeData('N');
    writeData(0x20);
    writeData(0x20);
    writeData('X');
    writeData(0x20);
    writeData(x+48);//(char)x + 48);
    writeCom(0xC0);
    writeData('Y');
    writeData(y + 48);
    writeData(0x20);
    writeData(0x20);
    writeData(walls[x][y] + 48);
    return;
  } // end north if
  // if mouse is facing east
  if(direction == 1){
    writeData('E');
    writeData(0x20);
    writeData(0x20);
    writeData('X');
    writeData(x + 48);
    writeCom(0xC0);
    writeData('Y');
    writeData(y + 48);
    writeData(0x20);
    writeData(0x20);
    writeData(walls[x][y] + 48);
    return;
  } // end east if
  // if mouse is facing south
  if(direction == 2){
    writeData('S');
    writeData(0x20);
    writeData(0x20);
    writeData('X');
    writeData(x + 48);
    writeCom(0xC0);
    writeData('Y');
    writeData(y + 48);
    writeData(0x20);
    writeData(0x20);
    writeData(walls[x][y] + 48);
    return;
  } // end south if
  // if mouse is facing west
  if(direction == 3){
    writeData('W');
    writeData(0x20);
    writeData(0x20);
    writeData('X');
    writeData(x + 48);
    writeCom(0xC0);
    writeData('Y');
    writeData(y + 48);
    writeData(0x20);
    writeData(0x20);
    writeData(walls[x][y] + 48);
    return;
  } // end west if
} // end displayStats


// clears the LCD screen
void clearLCD(){
  // Sends the Control Register to clear the LCD screen
  writeCom(0x01);
  Cpu_Delay100US(160);
} // end clearLCD


void displaySensors(){
  //Turns the left sensor into ASCII and writes it to the top left row on LCD
  writeData((sensors[0] /100)+48);
  writeData((sensors[0] /10)+48);
  writeData((sensors[0] %10)+48);
  // Two Spaces
  writeData(0x20);
  writeData(0x20);
  //Turns the right sensor into ASCII, writes it to the top right row on the LCD
  writeData((sensors[2] /100)+48);
  writeData((sensors[2] /10)+48);
  writeData((sensors[2] %10)+48);
  //Moves to the second row and prints two spaces
  writeCom(0xC0);
  writeData(0x20);
  writeData(0x20);
  //Turns the center sensor into ASCII and writes to the terminal
  writeData((sensors[1] /100) +48);
  writeData((sensors[1] /10)+48);
  writeData((sensors[1] %10)+48);
} // end displaySensors


// polls the sensors for their measurements
void pollSensors(){
  Sensors_Measure(1);
  Sensors_GetValue(sensors);
} // end pollSensors


void turnAround(){
  turnLeft();
  turnLeft();
} // end turnAround


// makes micromouse turn 90 degrees to the right
void turnRight(){
  power_motors(1);
  numStep = 195;
  while(numStep != 0){;
    motorR = motorL;
  }sw
  power_motors(0);
  /*direction = direction + 1;
  if(direction > 3){
   direction = 0;
  } */
} // end turnRight


// makes micromouse move forward into the next cell
void moveForward(){
  power_motors(1);
  numStep = 367;
  while(numStep != 0){
    pollSensors();
  }
  power_motors(0);
} // end moveForward


// makes micromouse turn 90 degrees to the left
void turnLeft(){
  numStep = 200;
  power_motors(1);
  while(numStep != 0){
    motorL = motorR;
  }
  power_motors(0);
  /*direction = direction - 1;
  if(direction < 0){
   direction = 3;
  } */
}


// this will turn motors on when parameter is 1 and off when parameter is 0
void power_motors(int power) {
 if(power == 1) {
  MotorR_EN_PutVal(1);
  MotorL_EN_PutVal(0x03);
 } else {
  MotorR_EN_PutVal(0);
  MotorL_EN_PutVal(0x00);
 }
} // end power_motors


// this function initializes the distances to the center of the maze
void initializeDistance(){
  distance[0][0] = 8;
  distance[0][1] = 7;
  distance[0][2] = 6;
  distance[0][3] = 5;
  distance[0][4] = 4;
  distance[0][5] = 3;
  distance[0][6] = 2;
  distance[0][7] = 3;
  distance[0][8] = 4;
  distance[0][9] = 5;
  distance[0][10] = 6;
  distance[0][11] = 7;
  distance[0][12] = 8;
  distance[1][0] = 7;
  distance[1][1] = 6;
  distance[1][2] = 5;
  distance[1][3] = 4;
  distance[1][4] = 3;
  distance[1][5] = 2;
  distance[1][6] = 1;
  distance[1][7] = 2;
  distance[1][8] = 3;
  distance[1][9] = 4;
  distance[1][10] = 5;
  distance[1][11] = 6;
  distance[1][12] = 7;
  distance[2][0] = 6;
  distance[2][1] = 5;
  distance[2][2] = 4;
  distance[2][3] = 3;
  distance[2][4] = 2;
  distance[2][5] = 1;
  distance[2][6] = 0;
  distance[2][7] = 1;
  distance[2][8] = 2;
  distance[2][9] = 3;
  distance[2][10] = 4;
  distance[2][11] = 5;
  distance[2][12] = 6;
  distance[3][0] = 6;
  distance[3][1] = 5;
  distance[3][2] = 4;
  distance[3][3] = 3;
  distance[3][4] = 2;
  distance[3][5] = 1;
  distance[3][6] = 0;
  distance[3][7] = 1;
  distance[3][8] = 2;
  distance[3][9] = 3;
  distance[3][10] = 4;
  distance[3][11] = 5;
  distance[3][12] = 6;
  distance[4][0] = 7;
  distance[4][1] = 6;
  distance[4][2] = 5;
  distance[4][3] = 4;
  distance[4][4] = 3;
  distance[4][5] = 2;
  distance[4][6] = 1;
  distance[4][7] = 2;
  distance[4][8] = 3;
  distance[4][9] = 4;
  distance[4][10] = 5;
  distance[4][11] = 6;
  distance[4][12] = 7;
  distance[5][0] = 8;
  distance[5][1] = 7;
  distance[5][2] = 6;
  distance[5][3] = 5;
  distance[5][4] = 4;
  distance[5][5] = 3;
  distance[5][6] = 2;
  distance[5][7] = 3;
  distance[5][8] = 4;
  distance[5][9] = 5;
  distance[5][10] = 6;
  distance[5][11] = 7;
  distance[5][12] = 8;
} // end initializeDistance


// initializes walls for the beginning of the maze
void initializeWalls(){
  walls[0][0] = 3;
  walls[0][1] = 2;
  walls[0][2] = 2;
  walls[0][3] = 2;
  walls[0][4] = 2;
  walls[0][5] = 2;
  walls[0][6] = 2;
  walls[0][7] = 2;
  walls[0][8] = 2;
  walls[0][9] = 2;
  walls[0][10] = 2;
  walls[0][11] = 2;
  walls[0][12] = 6;
  walls[1][0] = 1;
  walls[1][1] = 0;
  walls[1][2] = 0;
  walls[1][3] = 0;
  walls[1][4] = 0;
  walls[1][5] = 0;
  walls[1][6] = 0;
  walls[1][7] = 0;
  walls[1][8] = 0;
  walls[1][9] = 0;
  walls[1][10] = 0;
  walls[1][11] = 0;
  walls[1][12] = 4;
  walls[2][0] = 1;
  walls[2][1] = 0;
  walls[2][2] = 0;
  walls[2][3] = 0;
  walls[2][4] = 0;
  walls[2][5] = 0;
  walls[2][6] = 0;
  walls[2][7] = 0;
  walls[2][8] = 0;
  walls[2][9] = 0;
  walls[2][10] = 0;
  walls[2][11] = 0;
  walls[2][12] = 4;
  walls[3][0] = 1;
  walls[3][1] = 0;
  walls[3][2] = 0;
  walls[3][3] = 0;
  walls[3][4] = 0;
  walls[3][5] = 0;
  walls[3][6] = 0;
  walls[3][7] = 0;
  walls[3][8] = 0;
  walls[3][9] = 0;
  walls[3][10] = 0;
  walls[3][11] = 0;
  walls[3][12] = 4;
  walls[4][0] = 1;
  walls[4][1] = 0;
  walls[4][2] = 0;
  walls[4][3] = 0;
  walls[4][4] = 0;
  walls[4][5] = 0;
  walls[4][6] = 0;
  walls[4][7] = 0;
  walls[4][8] = 0;
  walls[4][9] = 0;
  walls[4][10] = 0;
  walls[4][11] = 0;
  walls[4][12] = 4;
  walls[5][0] = 9;
  walls[5][1] = 8;
  walls[5][2] = 8;
  walls[5][3] = 8;
  walls[5][4] = 8;
  walls[5][5] = 8;
  walls[5][6] = 8;
  walls[5][7] = 8;
  walls[5][8] = 8;
  walls[5][9] = 8;
  walls[5][10] = 8;
  walls[5][11] = 8;
  walls[5][12] = 12;
}


/* END Project */
/*
** ###################################################################
**
**     This file was created by Processor Expert 3.02 [04.44]
**     for the Freescale HCS12 series of microcontrollers.
**
** ###################################################################
*/
