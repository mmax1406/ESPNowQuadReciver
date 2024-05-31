#include <esp_now.h>
#include <WiFi.h>

#define R_X_AXIS_PIN 36
#define R_Y_AXIS_PIN 39
#define L_Y_AXIS_PIN 34
#define L_X_AXIS_PIN 35
#define ARM_SWITCH 32
#define MODE_SWITCH 33

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x34, 0x85, 0x18, 0x43, 0x63, 0xD0};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    float Rx;
    float Ry;
    float Lx;
    float Ly;
    float arm;
    float mode;
    float commFlag;
} struct_message;

// Create a struct_message called myData
struct_message myData;

float Rx, Ry, Lx, Ly;
float Rx_b = 0;
float Ry_b = 0;
float Lx_b = 0;
float Ly_b = 0;
int armButtonState = 0;
int armLastButtonState = 0;
int modeButtonState = 0;
int modeLastButtonState = 0;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {

  Serial.begin(115200);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
 
  // Take 1000 measurements for X-axis and Y-axis
  for (int i = 0; i < 1000; i++) {
    Rx_b += analogRead(R_X_AXIS_PIN);
    Ry_b += analogRead(R_Y_AXIS_PIN);
    Lx_b += analogRead(L_X_AXIS_PIN);
    Ly_b += analogRead(L_Y_AXIS_PIN);
    delay(1); // Wait for a short time between readings
  }

  // Calculate average bias
  Rx_b = (Rx_b/1000)-(4095/2);
  Ry_b = (Ry_b/1000)-(4095/2);
  Lx_b = (Lx_b/1000)-(4095/2);
  Ly_b = (Ly_b/1000)-(4095/2);

  myData.arm = 1500;
  myData.mode = 1500;
  pinMode(ARM_SWITCH, INPUT);
  pinMode(MODE_SWITCH, INPUT);
}

void loop() {
  // read analog X and Y analog values
  Rx = analogRead(R_X_AXIS_PIN)-Rx_b;
  Ry = analogRead(R_Y_AXIS_PIN)-Ry_b;
  Lx = analogRead(L_X_AXIS_PIN)-Lx_b;
  Ly = analogRead(L_Y_AXIS_PIN)-Ly_b;

  armButtonState = digitalRead(ARM_SWITCH); 
  if (armButtonState != armLastButtonState) {
    // Check if the button is pressed
    if (armButtonState == HIGH) {
      if (myData.arm == 1100) {
        // Toggle the mode
        myData.arm = 1500;
      }else{
        myData.arm = 1100;
      }
    }
    // Update the last button state
    armLastButtonState = armButtonState;
  }

  modeButtonState = digitalRead(MODE_SWITCH);
  if (modeButtonState != modeLastButtonState) {
    // Check if the button is pressed
    if (modeButtonState == HIGH) {
      if (myData.mode == 1900 ) {
        // Toggle the mode
        myData.mode = 1500;
      }else{
        myData.mode = 1900;
      }
    }
    // Update the last button state
    // Update the last button state
    modeLastButtonState = modeButtonState;
  }

// // For testing
//   Rx = map(random(0, 4095),0,4095,1792,192);
//   Ry = map(random(0, 4095),0,4095,1792,192);
//   Lx = map(random(0, 4095),0,4095,192,1792);
//   Ly = map(random(0, 4095),0,4095,192,1792);

  
  // Serial.print(myData.mode);
  // Serial.print(" | ");
  // Serial.print(myData.arm);
  // Serial.print(" | ");
  // Serial.print(Rx);
  // Serial.print(" | ");
  // Serial.print(Ry);
  // Serial.print(" | ");
  // Serial.print(Lx);
  // Serial.print(" | ");
  // Serial.println(Ly);

  // Send message via ESP-NOW
  myData.Rx = Rx;
  myData.Ry = Ry;
  myData.Lx = Lx;
  myData.Ly = map(Ly,0,4095,4095,0);
  myData.commFlag = 1234;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  delay(50);
}