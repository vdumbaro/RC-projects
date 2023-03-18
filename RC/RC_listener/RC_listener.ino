
#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
 RH_NRF24 nrf24(7, 8); // For RFM73 on Anarduino Mini

void setup() 
{
  Serial.begin(9600);
  if (!nrf24.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("setRF failed");    
  else
    Serial.println("NRF setup OK");
}

void loop()
{
  if (nrf24.available())
  {
    // Should be a message for us now   
    byte buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (nrf24.recv(buf, &len))
    {
      Serial.print("got message [");
      Serial.print(len);
      Serial.print("]: " );
      for (int i=0;i<len;i++){
        Serial.print(buf[i]);
        Serial.print(" ");
      }
      Serial.println();
      /*
      // Send a reply
      uint8_t data[] = "OK";
      nrf24.send(data, sizeof(data));
      nrf24.waitPacketSent();
      Serial.println("Sent a reply");
      */
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}
