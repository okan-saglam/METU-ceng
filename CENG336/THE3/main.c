#include "pragmas.h"



#define BUFSIZE 512
#define SPBRG_VAL 21

#define PKT_MAX_SIZE 128 // Maximum packet size
#define PKT_HEADER 0x24  // Marker for start-of-packet
#define PKT_END 0x23     // Marker for end-of-packet

typedef enum
{
    INBUF = 0,
    OUTBUF = 1
} buf_t;

typedef enum
{
    PKT_WAIT_HDR,
    PKT_GET_BODY,
    PKT_WAIT_ACK
} pkt_state_t;



// ============================ //
//          GLOBALS             //
// ============================ //

uint8_t inbuf[BUFSIZE];   /* Preallocated buffer for incoming data */
uint8_t outbuf[BUFSIZE];  /* Preallocated buffer for outgoing data  */
uint8_t head[2] = {0, 0}; /* head for pushing, tail for popping */
uint8_t tail[2] = {0, 0};

pkt_state_t pkt_state = PKT_WAIT_HDR;
uint8_t pkt_body[PKT_MAX_SIZE]; // Packet data
uint8_t pkt_bodysize; // Size of the current packet
uint8_t pkt_valid = 0; // Set to 1 when packet has been received. Must be set to 0 once packet is processed


//char arrays for receiving commands
char dist[5]; 
char speed[5];
char altitude[5];

int period; //it is determined by alt command
int counter=0; //to check if we should send alt response
int adcreadflag=0; //to check if we are going to have adc readings
int manualcontrolflag=0; //to check if we are going to get led commands and give prs responses

uint8_t prevB;
//to implement state machines for portb
int rb4_state=0;
int rb5_state=0;
int rb6_state=0;
int rb7_state=0;
int sendPRS4=0;
int sendPRS5=0;
int sendPRS6=0;
int sendPRS7=0;

int endFlag = 0; //to check if end command is received

//these are the values for decoding commands
uint32_t spd;
uint32_t distance;
uint32_t alt;

uint32_t tempAltitude; //this holds the adc result, "X" given in pdf

// ============================ //
//          FUNCTIONS           //
// ============================ //
void disable_rxtx() { 
    PIE1bits.RC1IE = 0;
    PIE1bits.TX1IE = 0;
}

void enable_rxtx()  { 
    PIE1bits.RC1IE = 1;
    PIE1bits.TX1IE = 1;  
}
void init_timer()
{
    
    // Enable pre-scalar
    // Setting pre-scalar value
    
    T0CON = 0x00; 
    T0CONbits.TMR0ON = 1;
    T0CONbits.T0PS2 = 0;
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS0 = 1;
    // Pre-load the value

    TMR0H = 0x0B;
    TMR0L = 0xDC;
    
    
    
}

uint8_t buf_isempty(buf_t buf) 
{ 
  return (head[buf] == tail[buf]) ? 1 : 0; 
}

void buf_push(uint8_t v, buf_t buf)
{
    if (buf == INBUF){
        inbuf[head[buf]] = v;
    }
        
    else{
        outbuf[head[buf]] = v;
    }
        
    head[buf]++;
    
    if (head[buf] == BUFSIZE){
        head[buf] = 0;
    }
        
}

uint8_t buf_pop(buf_t buf)
{
    uint8_t v;
    if (buf_isempty(buf))
    {
        return 0;
    }
    else
    {
        if (buf == INBUF)
            v = inbuf[tail[buf]];
        else
            v = outbuf[tail[buf]];
        tail[buf]++;
        if (tail[buf] == BUFSIZE)
            tail[buf] = 0;
        return v;
    }
}

void receive_task(){
    disable_rxtx();
    // Wait until new bytes arrive
    if (!buf_isempty(INBUF)) {
        uint8_t v;
        switch(pkt_state) {
        case PKT_WAIT_HDR:
            v = buf_pop(INBUF);
            if (v == PKT_HEADER) {
                // Packet header is encountered, retrieve the rest of the packet
                pkt_state = PKT_GET_BODY;
                pkt_bodysize = 0;
            }
            break;
        case PKT_GET_BODY:
            v = buf_pop(INBUF);
            if (v == PKT_END) {
                // End of packet is encountered, signal process_task())
                pkt_state = PKT_WAIT_ACK;
                pkt_valid = 1;
            } else if (v == PKT_HEADER) {
                // Unexpected packet start. Abort current packet and restart
                pkt_bodysize = 0;
            } else 
                pkt_body[pkt_bodysize++] = v;
            break;
        case PKT_WAIT_ACK:
            if (pkt_valid == 0) {
                // Packet processing seems to be finished, continue monitoring
                pkt_state = PKT_WAIT_HDR;
            }
            break;
        }
    }
    enable_rxtx();     
}



void process_task(){
    //decoding commands which we received at receive_task()
    if(pkt_valid){
        if(pkt_body[0]=='G' && pkt_body[1]=='O'&& pkt_body[2]=='O'){
            if(pkt_bodysize==7){
                    int i;
                    for(i=3;i<7;i++){
                        dist[i-3]=pkt_body[i];
                        
                    }
                    sscanf(dist,  "%04x", &distance);
                    
                }
            init_timer();
        }
        
        else if(pkt_body[0]=='E' && pkt_body[1]=='N'&& pkt_body[2]=='D'){
            
            if(pkt_bodysize==3){
                disable_rxtx();    
                endFlag=1;
            }
            
                
        }
        
        else if (pkt_body[0]=='S' && pkt_body[1]=='P'&& pkt_body[2]=='D'){
            if(pkt_bodysize==7){
                    
                    
                    int i;
                    for(i=3;i<7;i++){
                        speed[i-3]=pkt_body[i];
                    }
                    
                    sscanf(speed,  "%04x", &spd);
            }
             
        }
        
        else if (pkt_body[0]=='A' && pkt_body[1]=='L'&& pkt_body[2]=='T'){
            
            adcreadflag = 1;
            counter = 0;
            if(pkt_bodysize==7){

                
                int i;
                for(i=3;i<7;i++){
                    altitude[i-3]=pkt_body[i];
                }

                sscanf(altitude,  "%04x", &alt);

                if(alt == 0){
                    adcreadflag=0;
                }

                else if (alt == 200){
                    period = 2; 
                }
                else if (alt == 400){
                    period = 4;

                }
                else if (alt== 600){
                    period = 6;
                }
            }
        }
        
        else if (pkt_body[0]=='M' && pkt_body[1]=='A'&& pkt_body[2]=='N'){
            if(pkt_bodysize == 5 && pkt_body[3]=='0'){
                if(pkt_body[4]=='1'){
                    
                    manualcontrolflag =1;
                    INTCONbits.RBIE = 1;
                }

                else{
                    manualcontrolflag=0;
                    INTCONbits.RBIE = 0;
                }
                
            }
        }
        
        
        else if (pkt_body[0]=='L' && pkt_body[1]=='E'&& pkt_body[2]=='D'){
            if(pkt_bodysize == 5 && manualcontrolflag && pkt_body[3]=='0'){
                switch(pkt_body[4]){
                    case '0':
                        LATAbits.LA0 = 0;
                        LATBbits.LB0 = 0;
                        LATCbits.LC0 = 0;
                        LATDbits.LD0 = 0;         

                        break;

                    case '1':
                       LATDbits.LD0 = 1; 

                       break;

                    case '2':
                       LATCbits.LC0 = 1; 

                       break;

                    case '3':
                       LATBbits.LB0 = 1; 

                       break;

                    case '4':
                       LATAbits.LA0 = 1; 

                       break;

                    default:
                        break;

                }       
            }
        }
    
        pkt_valid = 0;
        pkt_bodysize = 0;
        
    }
}
typedef enum {OUTPUT_INIT, OUTPUT_RUN} output_st_t;
output_st_t output_st = OUTPUT_INIT;
/* Output task function */
void transmit_task() {
    
    switch (output_st) {
    case OUTPUT_INIT:
        output_st = OUTPUT_RUN;
        break;
    case OUTPUT_RUN:
        disable_rxtx();
        // Check if there is any buffered output or ongoing transmission
        if (!buf_isempty(OUTBUF)&& TXSTA1bits.TXEN == 0) { 
            // If transmission is already ongoing, do nothing, 
            // the ISR will send the next char. Otherwise, send the 
            // first char and enable transmission
            TXSTA1bits.TXEN = 1;
            TXREG1 = buf_pop(OUTBUF);
        }
        enable_rxtx();
        break;
    }
    
}
//state machines for portb[4-7]
void isPRS04Created(){
    if(rb4_state){
        if(PORTB & 0x10){
            rb4_state = 0;
            sendPRS4=1;     
        }
    }
    else{
        if(!(PORTB & 0x10)){
            rb4_state = 1;    
        }
    }
}

void isPRS05Created(){
    if(rb5_state){
        if((PORTB & 0x20)){
            rb5_state = 0;
            sendPRS5=1;
        }
    }
    else{
        if(!(PORTB & 0x20)){
            rb5_state = 1;
        }
    }
}

void isPRS06Created(){
    if(rb6_state){
        if((PORTB & 0x40)){
            rb6_state = 0;
            sendPRS6=1;
        }
    }
    else{
        if(!(PORTB & 0x40)){
            rb6_state = 1;
        }
    }
}

void isPRS07Created(){
    if(rb7_state){
        if((PORTB & 0x80)){
            rb7_state = 0;
            sendPRS7=1;
        }
    }
    else{
        if(!(PORTB & 0x80)){
            rb7_state = 1;
            
        }
    }
}





void __interrupt(high_priority) highPriorityISR() {
    
     if(INTCONbits.TMR0IF == 1){
        INTCONbits.TMR0IF = 0;
        
        TMR0H = 0x0B;
        TMR0L = 0xDC;
        
        counter++;
        if(distance-spd>0){
            distance -=  spd;
        }
        else{
            distance = 0;
        }
        //we should check if we need to send prs response and send that response if needed
        if(sendPRS4 && INTCONbits.RBIE){
            sendPRS4 = 0;
            buf_push('$',OUTBUF);
            buf_push('P',OUTBUF);
            buf_push('R',OUTBUF);
            buf_push('S',OUTBUF);
            buf_push('0',OUTBUF);
            buf_push('4',OUTBUF);
            buf_push('#',OUTBUF);
           
        }
        else if(sendPRS5 && INTCONbits.RBIE){
            sendPRS5 = 0;
            buf_push('$',OUTBUF);
            buf_push('P',OUTBUF);
            buf_push('R',OUTBUF);
            buf_push('S',OUTBUF);
            buf_push('0',OUTBUF);
            buf_push('5',OUTBUF);
            buf_push('#',OUTBUF);
            
        }
        else if(sendPRS6 && INTCONbits.RBIE){
            sendPRS6 = 0;
            buf_push('$',OUTBUF);
            buf_push('P',OUTBUF);
            buf_push('R',OUTBUF);
            buf_push('S',OUTBUF);
            buf_push('0',OUTBUF);
            buf_push('6',OUTBUF);
            buf_push('#',OUTBUF);
            
        }
        else if(sendPRS7 && INTCONbits.RBIE){
            sendPRS7 = 0;
            buf_push('$',OUTBUF);
            buf_push('P',OUTBUF);
            buf_push('R',OUTBUF);
            buf_push('S',OUTBUF);
            buf_push('0',OUTBUF);
            buf_push('7',OUTBUF);
            buf_push('#',OUTBUF);
        }
        else{
            
            if(adcreadflag == 0 || counter!=period){
                //we should send dst response 
                buf_push('$',OUTBUF);
                buf_push('D',OUTBUF);
                buf_push('S',OUTBUF);
                buf_push('T',OUTBUF);
                char temp[5];
                sprintf(temp, "%04x", distance);

                buf_push( temp[0],OUTBUF);
                buf_push( temp[1],OUTBUF);
                buf_push( temp[2],OUTBUF);
                buf_push( temp[3],OUTBUF);
                buf_push('#',OUTBUF);
            }
            else{
                //we should send alt response to do this we need to enable adcon interrupts
                ADCON0bits.ADON = 1;
                ADCON0bits.GO = 1;
                ADCON0bits.GODONE = 1;
                PIE1bits.ADIE = 1;
                
                counter=0;
                
            }
            
        }
    }
     
    if(PIR1bits.ADIF == 1){
        //we should send alt response
        LATBbits.LB3 = 1;
        PIR1bits.ADIF = 0;
        
        unsigned int resultOfADC;
        resultOfADC = (ADRESH<<8)+ ADRESL; // We are combining the result of ADC module into one integer
        
        
        // These if and else statements are written according to the pdf. 
        if(0 <= resultOfADC && resultOfADC < 256){
            tempAltitude = 9000;
        }
        else if(256 <= resultOfADC && resultOfADC < 512){
            tempAltitude = 10000;
        }
        else if(512 <= resultOfADC && resultOfADC < 768){
            tempAltitude = 11000;
        }
        else if(768 <= resultOfADC && resultOfADC <= 1023){
            tempAltitude = 12000;
        }
        
        
        buf_push('$',OUTBUF);
        buf_push('A',OUTBUF);
        buf_push('L',OUTBUF);
        buf_push('T',OUTBUF);
        
        char temp[5];
        sprintf(temp, "%04x", tempAltitude); // Here we are transforming the single integer value into char array with their hexadecimal value
        
        buf_push( temp[0],OUTBUF);
        buf_push( temp[1],OUTBUF);
        buf_push( temp[2],OUTBUF);
        buf_push( temp[3],OUTBUF);
        buf_push('#',OUTBUF);
        
        
    }
    
    if(INTCONbits.RBIF == 1){
        // find out where that rb interrupt came from
        prevB = PORTB;
        INTCONbits.RBIF = 0;
        isPRS04Created();
        isPRS05Created();
        isPRS06Created();
        isPRS07Created();
    }
    
    
    
    if (PIR1bits.RC1IF == 1){
        PIR1bits.RC1IF = 0;      // Acknowledge interrupt
        buf_push(RCREG1, INBUF); // Buffer incoming byte
    } 
       
    if (PIR1bits.TX1IF ==1){
        PIR1bits.TX1IF = 0;    // Acknowledge interrupt
        // If all bytes are transmitted, turn off transmission
        if (buf_isempty(OUTBUF)){
            while(!TXSTA1bits.TRMT);
            TXSTA1bits.TXEN = 0;
        }
        // Otherwise, send next byte
        else{ 
            TXREG1 = buf_pop(OUTBUF);
        }
    }
}




void init_ports(){
    //initialize port types as input&output and port values as zero 
    TRISB = 0xF0; LATB= 0x00;
    TRISH = 0x10; LATH= 0x00;
    TRISA = 0x00; LATA= 0x00;
    TRISC = 0x80; LATC= 0x00;
    TRISD = 0x00; LATD= 0x00;   
}






void init_interrupts(){
    //initialize interrupts
    enable_rxtx();
    INTCON = 0x00;
    INTCONbits.PEIE =1 ; // Peripheral interrupt enable flag
    INTCONbits.T0IE = 1; // Timer zero interrupt enable flag
    INTCON2bits.RBPU = 0; // We are clearing RB Pull-Up 
    
}

void init_serial() {
    
    TXSTA1bits.TX9 = 0;  // No 9th bit
    TXSTA1bits.TXEN = 0; // Transmission is disabled for the time being
    TXSTA1bits.SYNC = 0;
    TXSTA1bits.BRGH = 0;
    RCSTA1bits.SPEN = 1; // Enable serial port
    RCSTA1bits.RX9 = 0;  // No 9th bit
    RCSTA1bits.CREN = 1; // Continuous reception
    BAUDCON1bits.BRG16 = 1;

    SPBRGH1 = (SPBRG_VAL >> 8) & 0xFF;
    SPBRG1 = SPBRG_VAL & 0xFF;
}

void init_adcon() {
    //initialize adcon
    ADCON0 = 0x30; // Initializes the bits that control the operation of the A/D module
    ADCON1 = 0x00; // Initializes the bits that configure the functions of the port pins
    ADCON2 = 0xAA; // Initializes the bits that configure the A/D clock source, programmed acquisition time and justification
    ADRESH = 0x00; // These two registers are holding the result of the ADC.
    ADRESL = 0x00;
    
}


void main() {
    
    init_ports();
    init_serial();
    init_adcon();
    init_interrupts();
    INTCONbits.GIE =1;
    
    prevB = PORTB;
    while(1)
    {
        receive_task();
        process_task();
        transmit_task();
        if(endFlag){ //check if end command is received and break if received
            break;
        }
        
    }
    return;
}