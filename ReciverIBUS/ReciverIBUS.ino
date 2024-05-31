#include <esp_now.h>
#include <WiFi.h>
#include <SoftwareSerial.h>
#include <RecurringTask.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    float Rx;
    float Ry;
    float Lx;
    float Ly;
    float arm;
    float mode;
    float commFlag;
} struct_message;

SoftwareSerial SerialIBUS(44, 43); // RX, TX

typedef struct
{
    uint8_t header1;
    uint8_t header2;
    uint16_t channels[14];
    uint16_t checksum;
} IBUS_Packet;
IBUS_Packet packet = {};

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));  

  if (myData.commFlag==1234){

    myData.Rx = map(myData.Rx,0,4095,1000,2000);
    myData.Ry = map(myData.Ry,0,4095,1000,2000);
    myData.Lx = map(myData.Lx,0,4095,1000,2000);
    myData.Ly = map(myData.Ly,0,4095,1000,2000);

    myData.Rx = constrain(myData.Rx,1000,2000);
    myData.Ry = constrain(myData.Ry,1000,2000);
    myData.Lx = constrain(myData.Lx,1000,2000);
    myData.Ly = constrain(myData.Ly,1000,2000);

    packet.channels[0] = myData.Rx;
    packet.channels[1] = myData.Ry;
    packet.channels[2] = myData.Lx;
    packet.channels[3] = myData.Ly;
    packet.channels[4] = myData.commFlag;
    packet.channels[5] = myData.arm;
    packet.channels[6] = myData.mode;

    packet.checksum = calc_checksum(packet);
    send_packet(packet);
  }
 
}

uint16_t calc_checksum(IBUS_Packet &p){
    uint16_t checksum = 0xFFFF;
    checksum -= p.header1;
    checksum -= p.header2;

    for (int i = 0; i < 14; i++)
    {
        checksum -= p.channels[i] & 0xFF;
        checksum -= (p.channels[i] >> 8) & 0xFF;
    }

    return checksum;
}

void send_packet(IBUS_Packet &p){
    SerialIBUS.write((uint8_t *)&p, sizeof(IBUS_Packet));
}


void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  SerialIBUS.begin(115200);

  packet.header1 = 0x20;
  packet.header2 = 0x40;

  for (int i = 0; i < 14; i++){
      packet.channels[i] = 0x5dc;
  }

  packet.checksum = calc_checksum(packet);
   
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {



  
  // delay(10); It worked with this
}