#define OFF 0
#define SERIAL_CONTROL 1
#define RC_433_CONTROL 2
#define RC_NRF_CONTROL 3
// Control the way car will receive commands
#define RC_CONTROL RC_NRF_CONTROL


#if RC_CONTROL == RC_433_CONTROL
  #include <RH_ASK.h>
  #include <SPI.h> // Not actualy used but needed to compile

  RH_ASK driver;
  #define TRANSMITTER_PIN 12
  #define MY_ID 0x73
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
  #define MY_ID 0x73
#endif
#define SEND_LED_PIN 4
#define GREEN_LED_PIN 5

byte send_msg[] = {MY_ID,0,0,0,0,0};
byte new_msg[] = {MY_ID,0,0,0,0,0};

#define SPEED_POSITION 1
#define STEAR_POSITION 2
#define Y2_POSITION 3
#define X2_POSITION 4


#define VRX_PIN  A0 // Arduino pin connected to VRX pin
#define VRY_PIN  A1 // Arduino pin connected to VRY pin
#define VR_SW_PIN  2 // Arduino Switch pin connected to VR

#define VRX_2_PIN  A2 // Arduino pin connected to VRX_2 pin
#define VRY_2_PIN  A3 // Arduino pin connected to VRY_2 pin
#define VR_SW_2_PIN  3 // Arduino Switch pin connected to VR_2

#define P1_PIN  A4 // Arduino pin connected to potentiometer 1 pin
#define P2_PIN  A5 // Arduino pin connected to potentiometer 2 pin

#define LEFT_THRESHOLD  350
#define RIGHT_THRESHOLD 650
#define UP_THRESHOLD    350
#define DOWN_THRESHOLD  650

#define COMMAND_NO     0x00
#define COMMAND_LEFT   0x01
#define COMMAND_RIGHT  0x02
#define COMMAND_UP     0x04
#define COMMAND_DOWN   0x08

int xValue = 0 ; // To store value of the X axis
int yValue = 0 ; // To store value of the Y axis
int swValue = 0 ; // To store value of swotch
int x2Value = 0 ; // To store value of the X axis
int y2Value = 0 ; // To store value of the Y axis
int sw2Value = 0 ; // To store value of swotch

int p1Value = 0 ; // To store p1 value
int p2Value = 0 ; // To store p2 value

int command = COMMAND_NO;

void fillValues(int xValue,int yValue,int x2Value,int y2Value){
  // joystic 1
  if (xValue < LEFT_THRESHOLD){
    new_msg[STEAR_POSITION] = map(xValue,0,LEFT_THRESHOLD,100,1);
  }
  else if (xValue > RIGHT_THRESHOLD)
  {
    new_msg[STEAR_POSITION] = map(xValue,RIGHT_THRESHOLD,1023,101,200);
  }
  else new_msg[STEAR_POSITION] = 0;
  
  if (yValue < UP_THRESHOLD){
    new_msg[SPEED_POSITION] = map(yValue,0,UP_THRESHOLD,100,1);
  }
  else if (yValue >DOWN_THRESHOLD)
  {
    new_msg[SPEED_POSITION] = map(yValue,DOWN_THRESHOLD,1023,101,200);
  }
  else new_msg[SPEED_POSITION] = 0;

  // joystic 2
  if (x2Value < LEFT_THRESHOLD){
    new_msg[X2_POSITION] = map(x2Value,0,LEFT_THRESHOLD,100,1);
  }
  else if (x2Value > RIGHT_THRESHOLD)
  {
    new_msg[X2_POSITION] = map(x2Value,RIGHT_THRESHOLD,1023,101,200);
  }
  else new_msg[X2_POSITION] = 0;
  
  if (y2Value < UP_THRESHOLD){
    new_msg[Y2_POSITION] = map(y2Value,0,UP_THRESHOLD,100,1);
  }
  else if (y2Value >DOWN_THRESHOLD)
  {
    new_msg[Y2_POSITION] = map(y2Value,DOWN_THRESHOLD,1023,101,200);
  }
  else new_msg[Y2_POSITION] = 0;
}

void setMax(int value1, int value2){
  if (new_msg[SPEED_POSITION] > 100)
    new_msg[SPEED_POSITION] = map(new_msg[SPEED_POSITION],100,200,100,100+value1);
   else
    new_msg[SPEED_POSITION] = map(new_msg[SPEED_POSITION],0,100,0,value1);
  
  if (new_msg[Y2_POSITION] > 100)
    new_msg[Y2_POSITION] = map(new_msg[Y2_POSITION],100,200,100,100+value2);
   else
    new_msg[Y2_POSITION] = map(new_msg[Y2_POSITION],0,100,0,value2);
}
void sendValues(){
  boolean to_send_msg = false;
  for (int n=0;n<sizeof(send_msg);n++) 
  {
    if (send_msg[n]!=new_msg[n]){
      send_msg[n]=new_msg[n];
      to_send_msg = true;
    }
  }
    
  if (to_send_msg )
  {
    digitalWrite(SEND_LED_PIN,HIGH);
    Serial.print("New message: [");
    Serial.print(send_msg[0],HEX);Serial.print(",");
    for (int n=1;n<sizeof(send_msg);n++){ 
     Serial.print(send_msg[n]);Serial.print(",");
    }
    Serial.println("]");
    // Send the new values 
    #if RC_CONTROL == RC_433_CONTROL
      driver.send(send_msg, sizeof(send_msg));
      delay(100);
      driver.waitPacketSent(); 
    #endif
    #if RC_CONTROL == RC_NRF_CONTROL
      nrf24.send(send_msg, sizeof(send_msg));
      nrf24.waitPacketSent();

      // Now wait for a reply
      uint8_t buf[5];
      uint8_t len = sizeof(buf);
  
    if (nrf24.waitAvailableTimeout(200))
    { 
      // Should be a reply message for us now   
      if (nrf24.recv(buf, &len))
      {
        Serial.print("Got a reply");
      }
      else
      {
        Serial.println("recv failed");
      }
    }
    else
    {
      Serial.println("No reply ... retry");
      nrf24.send(send_msg, sizeof(send_msg));
      nrf24.waitPacketSent(); 
    }
    #endif
  }
  digitalWrite(SEND_LED_PIN,LOW);
  
}
void setup()
{
    Serial.begin(9600);    // Debugging only
#if RC_CONTROL == RC_433_CONTROL
    pinMode(SEND_LED_PIN, OUTPUT);
    digitalWrite(SEND_LED_PIN,HIGH);
    if (!driver.init())
      Serial.println("init failed");
    else
      Serial.println("init OK");
      digitalWrite(SEND_LED_PIN,LOW);
#endif

#if RC_CONTROL == RC_NRF_CONTROL
    pinMode(SEND_LED_PIN, OUTPUT);
    digitalWrite(SEND_LED_PIN,HIGH);
    if (!nrf24.init())
      Serial.println("init failed");
    else
      digitalWrite(SEND_LED_PIN,LOW);
    // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
    if (!nrf24.setChannel(1))
      Serial.println("setChannel failed");
    if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
      Serial.println("setRF failed");
    else
      Serial.println("NRF setup OK");
#endif
    //pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(VR_SW_PIN, INPUT);
    pinMode(VR_SW_2_PIN, INPUT);
}

void loop()
{
    
    // read analog X and Y analog values
    xValue = analogRead(VRX_PIN);
    yValue = analogRead(VRY_PIN);
    swValue = digitalRead(VR_SW_PIN);
    // read analog X and Y analog values
    x2Value = analogRead(VRX_2_PIN);
    y2Value = analogRead(VRY_2_PIN);
    sw2Value = digitalRead(VR_SW_2_PIN);
    fillValues(xValue,yValue,x2Value,y2Value);
    
    p1Value = analogRead(P1_PIN);
    p2Value = analogRead(P2_PIN);
    setMax(map(p1Value,0,1023,0,100),map(p2Value,0,1023,0,100));
    
    sendValues();
    delay(100);
}
