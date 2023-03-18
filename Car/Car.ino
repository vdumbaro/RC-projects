
/*
 * Phase 1:
 * Setup main car arduino board 
 * connect a relay on the D8 to control the motors. No revers.
 * Connect a servo motor D9 to control the stearing.
 */

#define OFF 0
#define SERIAL_CONTROL 1
#define RC_433_CONTROL 2
#define RC_NRF_CONTROL 3
#define RELAY 1
#define SERVO 2


// Control the way car will receive commands
#define RC_CONTROL RC_NRF_CONTROL

// Control the way car main drive
#define MAIN_MOTOR_POWER RELAY

// Control the way car stear drive
#define STEAR_MOTOR_POWER RELAY

#if RC_CONTROL == SERIAL_CONTROL
     static char message[10];
     static unsigned int message_pos = 0;
#endif

#if RC_CONTROL == RC_433_CONTROL
  #include <RH_ASK.h>
  #include <SPI.h> // Not actualy used but needed to compile
  RH_ASK driver;
  #define RECEIVER_PIN 11
  uint8_t receive_buf[3];
  uint8_t receive_buflen = sizeof(receive_buf);
  #define MY_ID 73
#endif

#if RC_CONTROL == RC_NRF_CONTROL
  #include <SPI.h>
  #include <RH_NRF24.h>
  #define CE 7 //pin D7
  #define CSN 8 //pin D8
  #define MOSI 11 //pin D11
  #define MISO 12 //pin D12
  #define SCK 13 //pin D13
  
  RH_NRF24 nrf24(CE, CSN);
  byte receive_buf[3];
  uint8_t len = sizeof(receive_buf);
  #define MY_ID 0x73
#endif

#if STEAR_MOTOR_POWER == SERVO
  #include <Servo.h>
  Servo myservo;  // create servo object to control a servo
  #endif
  #if STEAR_MOTOR_POWER == RELAY
  #define STEAR_MOTOR_PIN 2
  #define STEAR_DIRECTION_PIN 3
  #endif
  
  #if MAIN_MOTOR_POWER == RELAY
  #define MAIN_MOTOR_PIN 4
  #define MAIN_DIRECTION_PIN 5
#endif


unsigned char motor[2] = {0,0};
#define MAIN_MOTOR 0
#define STEAR_MOTOR 1

int set_drive_state(){
  #if MAIN_MOTOR_POWER == RELAY
    if (motor[MAIN_MOTOR] <= 100 ) {
      //Serial.println("Forward");
      digitalWrite(MAIN_MOTOR_PIN,HIGH);
      digitalWrite(MAIN_DIRECTION_PIN,LOW);
    }
    if (motor[MAIN_MOTOR] == 0) {
      //Serial.println("Stop");
      digitalWrite(MAIN_MOTOR_PIN,HIGH);
      digitalWrite(MAIN_DIRECTION_PIN,HIGH);
    }
    if (motor[MAIN_MOTOR] > 100) {
      //Serial.println("Back");
      digitalWrite(MAIN_MOTOR_PIN,LOW);
      digitalWrite(MAIN_DIRECTION_PIN,HIGH);
    }
    
  #else 
  #endif
  #if STEAR_MOTOR_POWER == RELAY
    if (motor[STEAR_MOTOR] <= 100){
      //Serial.println("Left");
      digitalWrite(STEAR_MOTOR_PIN,HIGH);
      digitalWrite(STEAR_DIRECTION_PIN,LOW);
    }
    if (motor[STEAR_MOTOR] == 0) {
      //Serial.println("Neutral");
      digitalWrite(STEAR_MOTOR_PIN,HIGH);
      digitalWrite(STEAR_DIRECTION_PIN,HIGH);
    }
    if (motor[STEAR_MOTOR] > 100){
      //Serial.println("Right");
      digitalWrite(STEAR_MOTOR_PIN,LOW);
      digitalWrite(STEAR_DIRECTION_PIN,HIGH);
    }
  #endif
  #if STEAR_MOTOR_POWER == SERVO
    myservo.write(motor[STEAR_MOTOR]); 
  #endif
}

void setup() {
  Serial.begin(9600);
  #if STEAR_MOTOR_POWER == RELAY
    pinMode(STEAR_MOTOR_PIN, OUTPUT);
    pinMode(STEAR_DIRECTION_PIN, OUTPUT);
    digitalWrite(STEAR_MOTOR_PIN,HIGH);
    digitalWrite(STEAR_DIRECTION_PIN,HIGH);
  #endif
  #if STEAR_MOTOR_POWER == SERVO
    //myservo.attach(STEAR_PIN);  // attaches the servo on pin 9 to the servo object
   // myservo.write(motor[STEAR_MOTOR]);  
    
  #endif
  #if MAIN_MOTOR_POWER == RELAY
    pinMode(MAIN_MOTOR_PIN, OUTPUT);
    pinMode(MAIN_DIRECTION_PIN, OUTPUT);
    digitalWrite(MAIN_MOTOR_PIN,HIGH);
    digitalWrite(MAIN_DIRECTION_PIN,HIGH);
  #endif
  #if MAIN_MOTOR_POWER == SERVO
    // add PWM for MAIN_MOTOR 
  #endif

  #if RC_CONTROL == RC_433_CONTROL
    if (!driver.init())
           Serial.println("Receiver failed to init");
      else
          Serial.println("Receiver init OK");
  #endif
  #if RC_CONTROL == RC_NRF_CONTROL
    if (!nrf24.init())
      Serial.println("init failed");
    // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
    if (!nrf24.setChannel(1))
      Serial.println("setChannel failed");
    if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
      Serial.println("setRF failed");
    else
      Serial.println("NRF setup OK");
    #endif
}

void loop() {
  #if RC_CONTROL == SERIAL_CONTROL
     //Create a place to hold the incoming message
     while (Serial.available() > 0)
     {

       //Read the next available byte in the serial receive buffer
       char inByte = Serial.read();
    
       //Message coming in (check not terminating character) and guard for over message size
       if ( inByte != '\n' && (message_pos < 10 - 1) )
       {
         //Add the incoming byte to our message
         message[message_pos] = inByte;
         message_pos++;
       }
       else
       {
         //Add null character to string
         message[message_pos] = '\0';
         Serial.println(message);
         if (strcmp(message,"F") == 0) motor[MAIN_MOTOR] = 255 ;
         if (strcmp(message,"B") == 0) motor[MAIN_MOTOR] = 0 ;
         if (strcmp(message,"N") == 0) motor[MAIN_MOTOR] = 128 ;
         if (strcmp(message,"L") == 0) motor[STEAR_MOTOR] = 0 ;
         if (strcmp(message,"R") == 0) motor[STEAR_MOTOR] = 255 ;
         if (strcmp(message,"X") == 0) motor[STEAR_MOTOR] = 127 ;
         //Reset for the next message
         message_pos = 0;
       }
     }
  #endif
  #if RC_CONTROL == RC_433_CONTROL
    if (driver.recv(receive_buf, &receive_buflen)) // Non-blocking
    {
      Serial.print("New message: ");
      Serial.print(receive_buf[0],HEX);Serial.print(" ");        
      Serial.print(receive_buf[1]); Serial.print(" ");      
      Serial.println(receive_buf[2]);
      
      if (receive_buf[0] == MY_ID) {
        if (receive_buf[1] < 120 || receive_buf[1] > 140){ 
          motor[MAIN_MOTOR]= receive_buf[1];}
        else {
          motor[MAIN_MOTOR]= 128;
        }
        Serial.print(motor[MAIN_MOTOR]);

        if (receive_buf[2] < 120 || receive_buf[2] > 140){ 
          motor[STEAR_MOTOR]= receive_buf[2];}
        else {
          motor[STEAR_MOTOR]= 128;
        }
        Serial.print(motor[STEAR_MOTOR]);
      }
      Serial.println("");
    }
  #endif

  #if RC_CONTROL == RC_NRF_CONTROL
    if (nrf24.available())
    {
      // Should be a message for us now   
      len = 3;
      if (nrf24.recv(receive_buf, &len))
      {
        //NRF24::printBuffer("request: ", buf, len);
        Serial.print("got request: ");
        Serial.print(receive_buf[0],HEX);Serial.print(" ");Serial.print(receive_buf[1],DEC);Serial.print(" ");Serial.println(receive_buf[2],DEC);
        
        // Send a reply
        uint8_t data[] = "ok";
        nrf24.send(data, sizeof(data));
        nrf24.waitPacketSent();
      }
      else
      {
        Serial.println("recv failed");
      }
      if (receive_buf[0] == MY_ID) {
        if (receive_buf[1] < 120 || receive_buf[1] > 140){ 
          motor[MAIN_MOTOR]= receive_buf[1];}
        else {
          motor[MAIN_MOTOR]= 128;
        }
        Serial.print(motor[MAIN_MOTOR]);

        if (receive_buf[2] < 120 || receive_buf[2] > 140){ 
          motor[STEAR_MOTOR]= receive_buf[2];}
        else {
          motor[STEAR_MOTOR]= 128;
        }
        Serial.print(motor[STEAR_MOTOR]);
      }
      Serial.println("");
    }
  #endif
  
  // put your main code here, to run repeatedly:
  set_drive_state();
  delay(100);

}
