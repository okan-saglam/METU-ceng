// ============================ //
// Do not edit this part!!!!    //
// ============================ //
// 0x300001 - CONFIG1H
#pragma config OSC = HSPLL      // Oscillator Selection bits (HS oscillator,
                               // PLL enabled (Clock Frequency = 4 x FOSC1))
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit
                               // (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit
                               // (Oscillator Switchover mode disabled)
// 0x300002 - CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out
                               // Reset disabled in hardware and software)
// 0x300003 - CONFIG1H
#pragma config WDT = OFF        // Watchdog Timer Enable bit
                               // (WDT disabled (control is placed on the SWDTEN bit))
// 0x300004 - CONFIG3L
// 0x300005 - CONFIG3H
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit
                               // (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled;
                               // RE3 input pin disabled)
// 0x300006 - CONFIG4L
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply
                               // ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit
                               // (Instruction set extension and Indexed
                               // Addressing mode disabled (Legacy mode))

#pragma config DEBUG = OFF      // Disable In-Circuit Debugger

#define KHZ 1000UL
#define MHZ (KHZ * KHZ)
#define _XTAL_FREQ (40UL * MHZ)

// ============================ //
//             End              //
// ============================ //

#include <xc.h>
#include <stdint.h>
#define T_PRELOAD_LOW 0x69
#define T_PRELOAD_HIGH 0x67

uint8_t prevB;
// ============================ //
//        DEFINITIONS           //
// ============================ //

// You can write struct definitions here...

// ============================ //
//          GLOBALS             //
// ============================ //

int counter = 0;

int leftState = 0;

int rightState = 0;
int downState = 0;

int upState = 0;

int rotateState=0;

int submitState = 0;
int newInput = 1;

int ledCount = 0;
int inputCount = 0;
int rotateCount=0;

int blink_on = 1;

int submittedGameTable[4][8];
int aliveGameTable[4][8];

// You can write globals definitions here...

// ============================ //
//          FUNCTIONS           //
// ============================ //

void Init()
{
   ADCON1 = 0x0F;
   
   LATA = 0x00; PORTA = 0x00; TRISA = 0x00;
   
   LATB = 0x00; PORTB = 0x00; TRISB = 0x30;
   LATG = 0x00; PORTG = 0x00; TRISG = 0x1D;

   LATC = 0x00; PORTC = 0x00; TRISC = 0x00;
   LATD = 0x00; PORTD = 0x00; TRISD = 0x00;
   LATE = 0x00; PORTE = 0x00; TRISE = 0x00;
   LATF = 0x00; PORTF = 0x00; TRISF = 0x00;
   
   LATH = 0x0F; PORTH = 0x0F; TRISH = 0x00;
   LATJ = 0x3F; PORTJ = 0x3F; TRISJ = 0x00; // 0: 00111111
}

void InitializeTimerAndInterrupts()
{
   
   // Enable pre-scalar
   // Full pre-scale
   // we also need to do in-code scaling
  
   
   T0CON = 0x00; 
   T0CONbits.TMR0ON = 1;
   T0CONbits.T0PS2 = 1;
   T0CONbits.T0PS1 = 0;
   T0CONbits.T0PS0 = 1;
   // Pre-load the value
   TMR0H = T_PRELOAD_HIGH;
   TMR0L = T_PRELOAD_LOW;

   RCONbits.IPEN = 0;
   INTCON = 0x00;
   INTCONbits.GIE = 1;
   INTCONbits.TMR0IE = 1;
   INTCONbits.RBIE = 1;
}


int checkStartingPoint(){
   int i,j;
   for(i=0;i<4;i++){
       for(j=0;j<8;j++){
           if(aliveGameTable[i][j]) return i;
      
       }
   }
   return 0 ;
}

int checkStartingPoint2(){
   int i,j;
   for(j=0;j<8;j++){
       for(i=0; i < 4;i++){
           if(aliveGameTable[i][j]) return j;
      
       }
   }
   return 0 ;
}

void slideLeft(){
   int i,j,temp;
   
   if(inputCount%3 == 0 ){
       for(i=1 ; i<4 ; i++){
         for(j=0 ; j<8 ; j++){
            if(aliveGameTable[i][j]){
                aliveGameTable[i-1][j] = 1;
                aliveGameTable[i][j] = 0;
                 if(submittedGameTable[i][j] == 0){
                     if(i==1){ 
                          if(PORTD & (1 << j)){
                              LATC |= (1 << (j));
                          }
                          LATD &= ~(1 << j);
                      }
                      else if(i==2){ 
                          if(PORTE & (1 << j)){
                              LATD |= (1 << (j));
                          }
                          LATE &= ~(1 << j);
                      }
                      else if(i==3){ 
                          if(PORTF & (1 << j)){
                              LATE |= (1 << (j));
                          }
                          LATF &= ~(1 << j);
                      }
                 }
                 else{
                     if(i==1){ 
                          if(PORTD & (1 << j)){
                              LATC |= (1 << (j));
                          }
                          LATD |= (1 << j);
                      }
                      else if(i==2){ 
                          if(PORTE & (1 << j)){
                              LATD |= (1 << (j));
                          }
                          LATE |= (1 << j);
                      }
                      else if(i==3){ 
                          if(PORTF & (1 << j)){
                              LATE |= (1 << (j));
                          }
                          LATF |= (1 << j);
                     }
                 }
             } 
           }  
       }
   }
   
   else{
       
       temp = checkStartingPoint();
       
       if(temp>0){
           for(i=1 ; i<4 ; i++){
               for(j=0 ; j<8 ; j++){
                   if(aliveGameTable[i][j]){
                   aliveGameTable[i-1][j] = 1;
                   aliveGameTable[i][j] = 0;
                       if(submittedGameTable[i][j] == 0){
                           if(i==1){ 
                               if(PORTD & (1 << j)){
                                  LATC |= (1 << (j));
                               }
                               LATD &= ~(1 << j);
                           }
                           else if(i==2){ 
                               if(PORTE & (1 << j)){
                                  LATD |= (1 << (j));
                               }
                               LATE &= ~(1 << j);
                           }
                           else if(i==3){ 
                               if(PORTF & (1 << j)){
                                  LATE |= (1 << (j));
                               }
                               LATF &= ~(1 << j);
                           }
                       }
                       else{
                           if(i==1){ 
                              if(PORTD & (1 << j)){
                                  LATC |= (1 << (j));
                              }
                              LATD |= (1 << j);
                          }
                          else if(i==2){ 
                              if(PORTE & (1 << j)){
                                  LATD |= (1 << (j));
                              }
                              LATE |= (1 << j);
                          }
                          else if(i==3){ 
                              if(PORTF & (1 << j)){
                                  LATE |= (1 << (j));
                              }
                              LATF |= (1 << j);
                         }
                     }
                 } 
               }  
           }
       }
       
           
   }
   
   if(!blink_on){
      for(i=0;i<4;i++){
          for(j=0;j<8;j++){
              if(aliveGameTable[i][j]){
                  if(i==0){
                      LATC &= ~(1<<j); 
                  }
                  else if(i==1){
                      LATD &= ~(1<<j); 
                  }
                  else if(i==2){
                      LATE &= ~(1<<j); 
                  }
                  else{
                      LATF &= ~(1<<j); 
                  }
              }
          }
      } 
   }
}

void isLeftPressed(){
   if(leftState){
       if(!(PORTG & 0x08)){
           leftState = 0;
       }
   }
   else{
       if(PORTG & 0x08){
           leftState = 1;
           slideLeft();
       }
   }
}

void slideRight(){
   int i,j,temp;
   
   if(inputCount%3==0){
       for(i=2 ; i>=0 ; i--){
           for(j=0 ; j<8 ; j++){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i+1][j] = 1;
                   aliveGameTable[i][j] = 0;
                   if(submittedGameTable[i][j] == 1){
                       if(i==0){ 
                           if(PORTC & (1 << j)){
                               LATD |= (1 << (j));
                           }
                           LATC |= (1 << j);
                       }
                       else if(i==1){ 
                           if(PORTD & (1 << j)){
                               LATE |= (1 << (j));
                           }
                           LATD |= (1 << j);
                       }
                       else if(i==2){ 
                           if(PORTE & (1 << j)){
                               LATF |= (1 << (j));
                           }
                           LATE |= (1 << j);
                       }
                   }
                   else{
                       if(i==0){ 
                           if(PORTC & (1 << j)){
                               LATD |= (1 << (j));
                           }
                           LATC &= ~(1 << j);
                       }
                       else if(i==1){ 
                           if(PORTD & (1 << j)){
                               LATE |= (1 << (j));
                           }
                           LATD &= ~(1 << j);
                       }
                       else if(i==2){ 
                           if(PORTE & (1 << j)){
                               LATF |= (1 << (j));
                           }
                           LATE &= ~(1 << j);
                       }
                   }
               } 
           }
       }
   }
   
   else{
       
       temp=checkStartingPoint();
       if(temp<2){
           for(i=2 ; i>=0 ; i--){
               for(j=0 ; j<8 ; j++){
                   if(aliveGameTable[i][j]){
                       aliveGameTable[i+1][j] = 1;
                       aliveGameTable[i][j] = 0;
                       if(submittedGameTable[i][j] == 1){
                           if(i==0){ 
                               if(PORTC & (1 << j)){
                                   LATD |= (1 << (j));
                               }
                               LATC |= (1 << j);
                           }
                           else if(i==1){ 
                               if(PORTD & (1 << j)){
                                   LATE |= (1 << (j));
                               }
                               LATD |= (1 << j);
                           }
                           else if(i==2){ 
                               if(PORTE & (1 << j)){
                                   LATF |= (1 << (j));
                               }
                               LATE |= (1 << j);
                           }
                       }
                       else{
                           if(i==0){ 
                               if(PORTC & (1 << j)){
                                   LATD |= (1 << (j));
                               }
                               LATC &= ~(1 << j);
                           }
                           else if(i==1){ 
                               if(PORTD & (1 << j)){
                                   LATE |= (1 << (j));
                               }
                               LATD &= ~(1 << j);
                           }
                           else if(i==2){ 
                               if(PORTE & (1 << j)){
                                   LATF |= (1 << (j));
                               }
                               LATE &= ~(1 << j);
                           }
                       }
                   } 
               }
           }    
       }
         
   }
   
   
   if(!blink_on){
      for(i=0;i<4;i++){
          for(j=0;j<8;j++){
              if(aliveGameTable[i][j]){
                  if(i==0){
                      LATC &= ~(1<<j); 
                  }
                  else if(i==1){
                      LATD &= ~(1<<j); 
                  }
                  else if(i==2){
                      LATE &= ~(1<<j); 
                  }
                  else{
                      LATF &= ~(1<<j); 
                  }
              }
          }
      } 
   }
}

void isRightPressed(){
   if(rightState){
       if(!(PORTG & 0x01)){
           rightState = 0;
       }
   }
   else{
       if(PORTG & 0x01){
           rightState = 1;
           slideRight();
       }
   }
}

void slideDown(){
   int i,j,temp;
   
   if(inputCount%3==0){
       for(i=0 ; i<4 ; i++){
           for(j=6 ; j>=0 ; j--){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i][j+1] = 1;
                   aliveGameTable[i][j] = 0;

                   if(i==0 && (PORTC & (1 << (j+1)))) LATC &= ~(1 << (j+1));
                   else if(i==1 && (PORTD & (1 << (j+1)))) LATD &= ~(1 << (j+1));
                   else if(i==2 && (PORTE & (1 << (j+1)))) LATE &= ~(1 << (j+1));
                   else if(i==3 && (PORTF & (1 << (j+1)))) LATF &= ~(1 << (j+1));


                   if(submittedGameTable[i][j] == 0){
                       if(i==0 ) LATC &= ~(1 << j);
                       else if(i==1 ) LATD &= ~(1 << j);
                       else if(i==2 ) LATE &= ~(1 << j);
                       else if(i==3 ) LATF &= ~(1 << j);
                   }
                   else {
                       if(i==0 && ((j>0 && aliveGameTable[0][j-1] == 0) || j==0) ) LATC |= (1 << j);
                       else if(i==1 && ((j>0 && aliveGameTable[1][j-1] == 0) || j==0)) LATD |= (1 << j);
                       else if(i==2 && ((j>0 && aliveGameTable[2][j-1] == 0) || j==0)) LATE |= (1 << j);
                       else if(i==3 && ((j>0 && aliveGameTable[3][j-1] == 0) || j==0)) LATF |= (1 << j);
                   }
               } 
           }
       }    
   }
   
   else{
       temp=checkStartingPoint2();
       if(temp<6){
           for(i=0 ; i<4 ; i++){
               for(j=6 ; j>=0 ; j--){
                   if(aliveGameTable[i][j]){
                       aliveGameTable[i][j+1] = 1;
                       aliveGameTable[i][j] = 0;

                       if(i==0 && (PORTC & (1 << (j+1)))) LATC &= ~(1 << (j+1));
                       else if(i==1 && (PORTD & (1 << (j+1)))) LATD &= ~(1 << (j+1));
                       else if(i==2 && (PORTE & (1 << (j+1)))) LATE &= ~(1 << (j+1));
                       else if(i==3 && (PORTF & (1 << (j+1)))) LATF &= ~(1 << (j+1));


                       if(submittedGameTable[i][j] == 0){
                           if(i==0 ) LATC &= ~(1 << j);
                           else if(i==1 ) LATD &= ~(1 << j);
                           else if(i==2 ) LATE &= ~(1 << j);
                           else if(i==3 ) LATF &= ~(1 << j);
                       }
                       else {
                           if(i==0 && ((j>0 && aliveGameTable[0][j-1] == 0) || j==0) ) LATC |= (1 << j);
                           else if(i==1 && ((j>0 && aliveGameTable[1][j-1] == 0) || j==0)) LATD |= (1 << j);
                           else if(i==2 && ((j>0 && aliveGameTable[2][j-1] == 0) || j==0)) LATE |= (1 << j);
                           else if(i==3 && ((j>0 && aliveGameTable[3][j-1] == 0) || j==0)) LATF |= (1 << j);
                       }
                   } 
               }
           }    
       }
   }
   
   
   
   
   
   
}

void slideDown2(){
   int i,j,temp;
   
   if(inputCount % 3 == 0){
       for(i=0 ; i<4 ; i++){
           for(j=6 ; j>=0 ; j--){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i][j+1] = 1;
                   aliveGameTable[i][j] = 0;
                   if(i==0 && (PORTC & (1 << j))) LATC |= (1 << (j+1));
                   else if(i==1 && (PORTD & (1 << j))) LATD |= (1 << (j+1));
                   else if(i==2 && (PORTE & (1 << j))) LATE |= (1 << (j+1));
                   else if(i==3 && (PORTF & (1 << j))) LATF |= (1 << (j+1));

                   if(submittedGameTable[i][j] == 0){
                       if(i==0) LATC &= ~(1 << j);
                       else if(i==1) LATD &= ~(1 << j);
                       else if(i==2) LATE &= ~(1 << j);
                       else if(i==3) LATF &= ~(1 << j);
                   }
                   else {
                      if(i==0 && ((j>0 && aliveGameTable[0][j-1] == 0) || j==0) ) LATC |= (1 << j);
                       else if(i==1 && ((j>0 && aliveGameTable[1][j-1] == 0) || j==0)) LATD |= (1 << j);
                       else if(i==2 && ((j>0 && aliveGameTable[2][j-1] == 0) || j==0)) LATE |= (1 << j);
                       else if(i==3 && ((j>0 && aliveGameTable[3][j-1] == 0) || j==0)) LATF |= (1 << j);
                   }
               } 
           }
       }    
   }
   
   else{
       temp=checkStartingPoint2();
       if(temp<6){
           for(i=0 ; i<4 ; i++){
               for(j=6 ; j>=0 ; j--){
                   if(aliveGameTable[i][j]){
                       aliveGameTable[i][j+1] = 1;
                       aliveGameTable[i][j] = 0;
                       if(i==0 && (PORTC & (1 << j))) LATC |= (1 << (j+1));
                       else if(i==1 && (PORTD & (1 << j))) LATD |= (1 << (j+1));
                       else if(i==2 && (PORTE & (1 << j))) LATE |= (1 << (j+1));
                       else if(i==3 && (PORTF & (1 << j))) LATF |= (1 << (j+1));

                       if(submittedGameTable[i][j] == 0){
                           if(i==0) LATC &= ~(1 << j);
                           else if(i==1) LATD &= ~(1 << j);
                           else if(i==2) LATE &= ~(1 << j);
                           else if(i==3) LATF &= ~(1 << j);
                       }
                       else {
                           if(i==0 && ((j>0 && aliveGameTable[0][j-1] == 0) || j==0) ) LATC |= (1 << j);
                           else if(i==1 && ((j>0 && aliveGameTable[1][j-1] == 0) || j==0)) LATD |= (1 << j);
                           else if(i==2 && ((j>0 && aliveGameTable[2][j-1] == 0) || j==0)) LATE |= (1 << j);
                           else if(i==3 && ((j>0 && aliveGameTable[3][j-1] == 0) || j==0)) LATF |= (1 << j);
                       }
                   } 
               }
           }    
       }
   }
   
   
   if(!blink_on){
      for(i=0;i<4;i++){
          for(j=0;j<8;j++){
              if(aliveGameTable[i][j]){
                  if(i==0){
                      LATC &= ~(1<<j); 
                  }
                  else if(i==1){
                      LATD &= ~(1<<j); 
                  }
                  else if(i==2){
                      LATE &= ~(1<<j); 
                  }
                  else{
                      LATF &= ~(1<<j); 
                  }
              }
          }
      } 
   }
}

void isDownPressed(){
   if(downState){
       if(!(PORTG & 0x04)){
           downState = 0;
       }
   }
   else{
       if(PORTG & 0x04){
           downState = 1;
           slideDown2();
       }
   }
}

void slideUp(){
   int i,j,temp;
   
   
   
   if(inputCount %3 == 0){
       for(i=0 ; i<4 ; i++){
           for(j=1 ; j<8 ; j++){
              if(aliveGameTable[i][j]){
                   aliveGameTable[i][j-1] = 1;
                   aliveGameTable[i][j] = 0;
                   if(i==0 && (PORTC & (1 << j))) LATC |= (1 << (j-1));
                   else if(i==1 && (PORTD & (1 << j))) LATD |= (1 << (j-1));
                   else if(i==2 && (PORTE & (1 << j))) LATE |= (1 << (j-1));
                   else if(i==3 && (PORTF & (1 << j))) LATF |= (1 << (j-1));

                   if(submittedGameTable[i][j] == 0){
                       if(i==0) LATC &= ~(1 << j);
                       else if(i==1) LATD &= ~(1 << j);
                       else if(i==2) LATE &= ~(1 << j);
                       else if(i==3) LATF &= ~(1 << j);
                   }
                   else {
                       if(i==0 && ((j<7 && aliveGameTable[0][j+1] == 0) || j==7) ) LATC |= (1 << j);
                       else if(i==1 && ((j<7 && aliveGameTable[1][j+1] == 0) || j==7)) LATD |= (1 << j);
                       else if(i==2 && ((j<7 && aliveGameTable[2][j+1] == 0) || j==7)) LATE |= (1 << j);
                       else if(i==3 && ((j<7 && aliveGameTable[3][j+1] == 0) || j==7)) LATF |= (1 << j);
                   }
              } 
           }
       }    
   }
   
   else{
       temp=checkStartingPoint2();
       if(temp>0){
           for(i=0 ; i<4 ; i++){
               for(j=1 ; j<8 ; j++){
                  if(aliveGameTable[i][j]){
                       aliveGameTable[i][j-1] = 1;
                       aliveGameTable[i][j] = 0;
                       if(i==0 && (PORTC & (1 << j))) LATC |= (1 << (j-1));
                       else if(i==1 && (PORTD & (1 << j))) LATD |= (1 << (j-1));
                       else if(i==2 && (PORTE & (1 << j))) LATE |= (1 << (j-1));
                       else if(i==3 && (PORTF & (1 << j))) LATF |= (1 << (j-1));

                       if(submittedGameTable[i][j] == 0){
                           if(i==0) LATC &= ~(1 << j);
                           else if(i==1) LATD &= ~(1 << j);
                           else if(i==2) LATE &= ~(1 << j);
                           else if(i==3) LATF &= ~(1 << j);
                       }
                       else {
                           if(i==0 && ((j<7 && aliveGameTable[0][j+1] == 0) || j==7) ) LATC |= (1 << j);
                           else if(i==1 && ((j<7 && aliveGameTable[1][j+1] == 0) || j==7)) LATD |= (1 << j);
                           else if(i==2 && ((j<7 && aliveGameTable[2][j+1] == 0) || j==7)) LATE |= (1 << j);
                           else if(i==3 && ((j<7 && aliveGameTable[3][j+1] == 0) || j==7)) LATF |= (1 << j);
                       }
                  } 
               }
           }    
       }
   }
   
   
   if(!blink_on){
      for(i=0;i<4;i++){
          for(j=0;j<8;j++){
              if(aliveGameTable[i][j]){
                  if(i==0){
                      LATC &= ~(1<<j); 
                  }
                  else if(i==1){
                      LATD &= ~(1<<j); 
                  }
                  else if(i==2){
                      LATE &= ~(1<<j); 
                  }
                  else{
                      LATF &= ~(1<<j); 
                  }
              }
          }
      } 
   }
}

void isUpPressed(){
   if(upState){
       if(!(PORTG & 0x10)){
           upState = 0;
       }
   }
   else{
       if(PORTG & 0x10){
           upState = 1;
           slideUp();
       }
   }
}

void input_task(){
   __delay_ms(10);
   isLeftPressed();
   isRightPressed();
   isDownPressed();
   isUpPressed();
}

void toggleAlive(){
   int i,j;
   for(i=0 ; i<4 ; i++){
       for(j=0 ; j<8 ; j++){
           if(aliveGameTable[i][j]){
               if(i==0){
                  LATC ^= (1 << j);
               }
               else if(i==1){
                  LATD ^= (1 << j);
               }
               else if(i==2){
                  LATE ^= (1 << j);
               }
               else{
                  LATF ^= (1 << j);
               }
           } 
       }
   }
}





int checkCollision(){
   int i,j;
   for(i=0 ; i<4 ; i++){
       for(j=0 ; j<8 ; j++){
           if(submittedGameTable[i][j] && aliveGameTable[i][j] ) return 1;
       }
   }
   return 0;
}

void submit(){
   int collision;
   collision = checkCollision();
   if(!collision){
       newInput = 1;
       int i,j;
       for(i=0 ; i<4 ; i++){
           for(j=0 ; j<8 ; j++){
               if(aliveGameTable[i][j]){
                   submittedGameTable[i][j] = 1;
                   if(!blink_on){
                       if(i==0){
                           LATC |= (1<<j);
                       }
                       else if(i==1){
                           LATD |= (1<<j);
                       }
                       else if(i==2){
                           LATE |= (1<<j);
                       }
                       else{
                           LATF |= (1<<j);
                       }
                   }
                   ledCount++;
               }
           }
       }
       for(i=0 ; i<4 ; i++){
           for(j=0 ; j<8 ; j++){
               aliveGameTable[i][j] = 0;
           }
       }
       
       inputCount++;
       counter=0;
   }
}

void isSubmit(){
   if(submitState){
       if(!(PORTB & 0x10)){
           submitState = 0;
       }
   }
   else{
       if(PORTB & 0x10){
           submitState = 1;
           submit();
       }
   }
}


void rotate(){
   
   int i,j;
   int flag=0;
   
   if(rotateCount%4 == 0){
       for(i=0;i<4;i++){
           for(j=0;j<8;j++){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i][j+1]=1;
                   aliveGameTable[i][j]=0 ;
                   
                   if(i==0 && (PORTC & (1 << j))) LATC |= (1 << (j+1));
                   else if(i==1 && (PORTD & (1 << j))) LATD |= (1 << (j+1));
                   else if(i==2 && (PORTE & (1 << j))) LATE |= (1 << (j+1));
                   else if(i==3 && (PORTF & (1 << j))) LATF |= (1 << (j+1));

                   if(submittedGameTable[i][j] == 0){
                       if(i==0) LATC &= ~(1 << j);
                       else if(i==1) LATD &= ~(1 << j);
                       else if(i==2) LATE &= ~(1 << j);
                       else if(i==3) LATF &= ~(1 << j);
                   }
                   
                   else{
                       if(i==0 && ((j>0 && aliveGameTable[0][j-1] == 0) || j==0) ) LATC |= (1 << j);
                       else if(i==1 && ((j>0 && aliveGameTable[1][j-1] == 0) || j==0)) LATD |= (1 << j);
                       else if(i==2 && ((j>0 && aliveGameTable[2][j-1] == 0) || j==0)) LATE |= (1 << j);
                       else if(i==3 && ((j>0 && aliveGameTable[3][j-1] == 0) || j==0)) LATF |= (1 << j);
                   }
                   
                   
                   
                   flag=1;
                   break;
               }
           }
           if(flag){
               flag=0;
               break;
           }
       }    
   }
   
   else if(rotateCount%4 == 1){
       for(j=0;j<8;j++){
           for(i=0;i<4;i++){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i-1][j]=1;
                   aliveGameTable[i][j]=0 ;
                   
                   if(submittedGameTable[i][j] == 0){
                       if(i==1){ 
                           if(PORTD & (1 << j)){
                              LATC |= (1 << (j));
                           }
                           LATD &= ~(1 << j);
                       }
                       else if(i==2){ 
                           if(PORTE & (1 << j)){
                              LATD |= (1 << (j));
                           }
                           LATE &= ~(1 << j);
                       }
                       else if(i==3){ 
                           if(PORTF & (1 << j)){
                              LATE |= (1 << (j));
                           }
                          LATF &= ~(1 << j);
                       }
                   }
                   else{
                       if(i==1){ 
                          if(PORTD & (1 << j)){
                              LATC |= (1 << (j));
                           }
                           LATD |= (1 << j);
                           }
                       else if(i==2){ 
                           if(PORTE & (1 << j)){
                              LATD |= (1 << (j));
                           }
                           LATE |= (1 << j);
                       }
                       else if(i==3){ 
                           if(PORTF & (1 << j)){
                              LATE |= (1 << (j));
                           }
                          LATF |= (1 << j);
                       }
                   }
                   
                   
                   flag=1;
                   break;
               }
           }
           if(flag){
               flag=0;
               break;
           }
       }    
   }
   else if(rotateCount%4 == 2){
       for(i=3 ; i>=0 ; i--){
           for(j=7; j>=0 ; j--){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i][j-1]=1;
                   aliveGameTable[i][j]=0 ; 
                   
                   if(i==0 && (PORTC & (1 << j))) LATC |= (1 << (j-1));
                   else if(i==1 && (PORTD & (1 << j))) LATD |= (1 << (j-1));
                   else if(i==2 && (PORTE & (1 << j))) LATE |= (1 << (j-1));
                   else if(i==3 && (PORTF & (1 << j))) LATF |= (1 << (j-1));

                   if(submittedGameTable[i][j] == 0){
                       if(i==0) LATC &= ~(1 << j);
                       else if(i==1) LATD &= ~(1 << j);
                       else if(i==2) LATE &= ~(1 << j);
                       else if(i==3) LATF &= ~(1 << j);
                   }
                   
                   else{
                       if(i==0 && ((j<7 && aliveGameTable[0][j+1] == 0) || j==7) ) LATC |= (1 << j);
                       else if(i==1 && ((j<7 && aliveGameTable[1][j+1] == 0) || j==7)) LATD |= (1 << j);
                       else if(i==2 && ((j<7 && aliveGameTable[2][j+1] == 0) || j==7)) LATE |= (1 << j);
                       else if(i==3 && ((j<7 && aliveGameTable[3][j+1] == 0) || j==7)) LATF |= (1 << j);
                   }
                   
                  
                   
                   flag=1;
                   break;
               
               }   
           
           }
           if(flag){
               flag=0;
               break;
           }
       
       }
   }
   
   
   else{
       for(i=0;i<4;i++){
           for(j=7;j>=0;j--){
               if(aliveGameTable[i][j]){
                   aliveGameTable[i+1][j]=1;
                   aliveGameTable[i][j]=0 ;
                   
                   if(submittedGameTable[i][j] == 1){
                       if(i==0){ 
                           if(PORTC & (1 << j)){
                               LATD |= (1 << (j));
                           }
                           LATC |= (1 << j);
                       }
                       else if(i==1){ 
                           if(PORTD & (1 << j)){
                               LATE |= (1 << (j));
                           }
                           LATD |= (1 << j);
                       }
                       else if(i==2){ 
                           if(PORTE & (1 << j)){
                               LATF |= (1 << (j));
                           }
                           LATE |= (1 << j);
                       }
                   }
                   else{
                       if(i==0){ 
                           if(PORTC & (1 << j)){
                               LATD |= (1 << (j));
                           }
                           LATC &= ~(1 << j);
                       }
                       else if(i==1){ 
                           if(PORTD & (1 << j)){
                               LATE |= (1 << (j));
                           }
                           LATD &= ~(1 << j);
                       }
                       else if(i==2){ 
                           if(PORTE & (1 << j)){
                               LATF |= (1 << (j));
                           }
                           LATE &= ~(1 << j);
                       }
                   }
                   
                  
                   flag=1;
                   break;
               }
           }
           if(flag){
               flag=0;
               break;
           }
       }    
   }
   
   
   if(!blink_on){
      for(i=0;i<4;i++){
          for(j=0;j<8;j++){
              if(aliveGameTable[i][j]){
                  if(i==0){
                      LATC &= ~(1<<j); // j ninci biti clearlamak
                  }
                  else if(i==1){
                      LATD &= ~(1<<j); 
                  }
                  else if(i==2){
                      LATE &= ~(1<<j); 
                  }
                  else{
                      LATF &= ~(1<<j); 
                  }
              }
          }
      } 
   }
   
}


void isRotate (){
   if(rotateState){
       if(!(PORTB & 0x20)){
           rotateState = 0;
       }
   }
   else{
       if(PORTB & 0x20){
           rotateState = 1;
           
           if(inputCount % 3 ==2){
               rotate();
               rotateCount++;
           }
           
       }
   }
}

void createInput(){
   if(inputCount % 3 == 0){
       aliveGameTable[0][0] = 1;
       
        //LATC |= 0x01;
       if(blink_on){
           LATC |= 0x01;
       }
       else{
           LATC &= 0xFE;
       } 
    }
   else if(inputCount % 3 == 1){
       aliveGameTable[0][0] = 1;
       aliveGameTable[0][1] = 1;
       aliveGameTable[1][0] = 1;
       aliveGameTable[1][1] = 1;
       
       //LATC |= 0x03;
       //LATD |= 0x03;
       
       if (blink_on){
           LATC |= 0x03;
           LATD |= 0x03;
       }
       else{
           LATC &= ~(0x03);
           LATD &= ~(0x03);
       } 
   }
   else{
       aliveGameTable[0][0] = 1;
       aliveGameTable[1][0] = 1;
       aliveGameTable[1][1] = 1;
       
       //LATC |= 0x01;
       //LATD |= 0x03;
       
       if(blink_on){
           LATC |= 0x01;
           LATD |= 0x03;
       }
       else{
           LATC &= ~(0x01);
           LATD &= ~(0x03);
       }
   }
   
}


void updateSevenSegment(){
   
   if(ledCount==1){

       LATJ = 0x06;
       LATH =0x08;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x07;
   }

   else if(ledCount==5){

       LATJ = 0x6D;
       LATH =0x08;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x07;
   }

   else if(ledCount==8){

       LATJ = 0x7F;
       LATH =0x08;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x07;
   }
   else if(ledCount==9){

       LATJ = 0x6F;
       LATH =0x08;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x07;

   }
       
   else if(ledCount==13){
       
       LATJ = 0x4F;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x06;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   
   else if(ledCount==16){
       LATJ = 0x7D;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x06;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   
   else if(ledCount==17){
       LATJ = 0x07;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x06;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   else if(ledCount==21){
       LATJ = 0x06;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x5B;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   else if(ledCount==24){
       
       LATJ = 0x66;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x5B;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   else if(ledCount==25){
       LATJ = 0x6D;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x5B;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   else if(ledCount==29){
       LATJ = 0x6F;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x5B;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
   else if(ledCount==32){
       LATJ = 0x5B;
       LATH =0x08;
       __delay_ms(1);
       LATJ = 0x4F;
       LATH =0x04;
       __delay_ms(1);
       LATJ= 0x3F;
       LATH=0x03;
   }
}



// You can write function definitions here...

// ============================ //
//   INTERRUPT SERVICE ROUTINE  //
// ============================ //
__interrupt(high_priority)
void HandleInterrupt()
{
   // ISR ...
   if(INTCONbits.TMR0IF == 1){
       INTCONbits.TMR0IF = 0;
       
       if(counter == 7){
           counter = 0;
           slideDown2();
       }
       else{
           counter++;
           
       }
       
       toggleAlive();
       
       if(blink_on){
           blink_on = 0;
       }
       else{
           blink_on=1;
       }
       
       TMR0H = T_PRELOAD_HIGH;
       TMR0L = T_PRELOAD_LOW;
   }
   if(INTCONbits.RBIF == 1){
       __delay_ms(10);
       prevB = PORTB;
       INTCONbits.RBIF = 0;
       isRotate();
       isSubmit();
   }
}

// ============================ //
//            MAIN              //
// ============================ //





void main()
{
   Init();
   __delay_ms(1000);
   InitializeTimerAndInterrupts();
   prevB=PORTB;
   
   while(1){
       if(newInput){
           createInput();
           newInput = 0;
       }
       input_task();
       updateSevenSegment();
       if(ledCount == 32){
           break;
       }
   }
   // Main ...
}


#pragma config OSC = HSPLL, FCMEN = OFF, IESO = OFF, PWRT = OFF, BOREN = OFF, WDT = OFF, MCLRE = ON, LPT1OSC = OFF, LVP = OFF, XINST = OFF, DEBUG = OFF

#include <xc.h>
//#include "breakpoints.h"

uint24_t prevB;

void __interrupt(high_priority) highPriorityISR(void) {

    if(INTCONbits.RBIF == 1){
        
        prevB = PORTB;
        INTCONbits.RBIF = 0;
        
        LATE = PORTC + PORTD;
        if(STATUSbits.C == 1){
            LATA |= 0x01;
        }
    }

}


void main(void) {
    
    ADCON1 = 0x0F;
    
    TRISA = 0x00; LATA = 0x00;
    TRISC = 0xFF; LATC = 0x00;
    TRISD = 0xFF; LATD = 0x00;
    TRISE = 0x00; LATE = 0x00;
    
    RCONbits.IPEN = 0;
    
    INTCON = 0x00;
    INTCONbits.GIE = 1;
    INTCONbits.RBIE = 1;
    
    prevB = PORTB;
    
    while(1) {}
            
}
