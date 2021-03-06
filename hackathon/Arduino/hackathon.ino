#include <SPI.h>
#include "SX1272.h"

// Define section

#define BAND868 //900, 433
#define MAX_NB_CHANNEL 9
#define STARTING_CHANNEL 10
#define ENDING_CHANNEL 18
uint8_t loraChannelIndex=0;
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_10_868,CH_11_868,CH_12_868,CH_13_868,CH_14_868,CH_15_868,CH_16_868,CH_17_868,CH_18_868};
#define LORAMODE  1 //Mode
#define LORA_ADDR 6 //Self address
#define DEFAULT_DEST_ADDR 1 //Gateway address

#define PRINTLN                   Serial.println("")              
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define FLUSHOUTPUT               Serial.flush();



//Variales
int dest_addr=DEFAULT_DEST_ADDR;
char cmd[260]="****************";
char sprintf_buf[100];
int msg_sn=0;
bool radioON=false;
uint8_t loraMode=LORAMODE;
uint32_t loraChannel=loraChannelArray[loraChannelIndex];
char loraPower='x'; //innitial poser level, M (maximum), H (high), L (low)
uint8_t loraAddr=LORA_ADDR;
unsigned int inter_pkt_time=10000; //Time between sending
unsigned int random_inter_pkt_time=0;
long last_periodic_sendtime=0;
// packet size for periodic sending
uint8_t MSS=40;


//Configure LoRa tranciever
void startConfig() {

  int e;
    
  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  // Select frequency channel
  if (loraMode==11) {
    e = sx1272.setChannel(CH_18_868);
  }
  else {
    e = sx1272.setChannel(loraChannel);
  }  
  // Select output power (Max, High or Low)
  e = sx1272.setPower(loraPower);
  // get preamble length
  e = sx1272.getPreambleLength();
  // Set the node address and print the result
  //e = sx1272.setNodeAddress(loraAddr);
  sx1272._nodeAddress=loraAddr;
  e=0;
}

void setup() {
  int e;
  
  //Add our code here
  Serial.begin(38400);
  // Power ON the module
  e = sx1272.ON();
  
  PRINT_CSTSTR("%s","^$**********Power ON: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

  e = sx1272.getSyncWord();

  if (!e) {
    PRINT_CSTSTR("%s","^$Default sync word: 0x");
    PRINT_HEX("%X", sx1272._syncWord);
    PRINTLN;
  }    
  
  if (!e) {
    radioON=true;
    startConfig();
  }
  
  FLUSHOUTPUT;
  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:

  int i=0, e;
  int cmdValue;
// START OF PERIODIC TASKS

  if (radioON) {


      // periodic message sending
      if (inter_pkt_time)
      
        if (millis()-last_periodic_sendtime > (random_inter_pkt_time?random_inter_pkt_time:inter_pkt_time)) {
          PRINT_CSTSTR("%s","Start to send data: ");
          PRINT_VALUE("%ld",millis());  
          PRINTLN;
          
          sx1272.CarrierSense();
          long startSend=millis();
          e = sx1272.sendPacketTimeout(dest_addr, (uint8_t*)cmd, strlen(cmd), 10000);
          PRINT_CSTSTR("%s","LoRa Sent in ");
          PRINT_VALUE("%ld",millis()-startSend);  
          PRINTLN;
          PRINT_CSTSTR("%s","Packet sent, state ");
          PRINT_VALUE("%d",e);
          PRINTLN;
          if (random_inter_pkt_time) {                
            random_inter_pkt_time=random(1000,inter_pkt_time);
            PRINT_CSTSTR("%s","next in ");
            PRINT_VALUE("%ld",random_inter_pkt_time);
            PRINTLN;
          }
          last_periodic_sendtime=millis();       
        }  
        
      // the end-device should also open a receiving window to receive 
      // INIT & UPDT messages
      e=1;
      // open a receive window
      uint16_t w_timer=1000;
      if (loraMode==1)
        w_timer=5000;
      e = sx1272.receivePacketTimeout(w_timer);
      // IF WE RECEIVE A RADIO PACKET

      if (!e) {
         int a=0, b=0;
         uint8_t tmp_length;

         sx1272.getSNR();
         sx1272.getRSSIpacket();

         tmp_length=sx1272._payloadlength;
         
         sprintf(sprintf_buf,"--- rxlora. dst=%d type=0x%.2X src=%d seq=%d len=%d SNR=%d RSSIpkt=%d BW=%d CR=4/%d SF=%d\n", 
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type, 
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum,
                   tmp_length, 
                   sx1272._SNR,
                   sx1272._RSSIpacket,
                   (sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
                   sx1272._codingRate+4,
                   sx1272._spreadingFactor);
                   
         PRINT_STR("%s",sprintf_buf);
         // provide a short output for external program to have information about the received packet
         // ^psrc_id,seq,len,SNR,RSSI
         sprintf(sprintf_buf,"^p%d,%d,%d,%d,%d,%d,%d\n",
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type,                   
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum, 
                   tmp_length,
                   sx1272._SNR,
                   sx1272._RSSIpacket);
                   
         PRINT_STR("%s",sprintf_buf);          

         // ^rbw,cr,sf
         sprintf(sprintf_buf,"^r%d,%d,%d\n", 
                   (sx1272._bandwidth==BW_125)?125:((sx1272._bandwidth==BW_250)?250:500),
                   sx1272._codingRate+4,
                   sx1272._spreadingFactor);
                   
         PRINT_STR("%s",sprintf_buf);  

      }

  }
}
