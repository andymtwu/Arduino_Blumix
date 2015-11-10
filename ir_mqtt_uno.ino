#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#define MS_PROXY "nrzsro.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_MAX_PACKET_SIZE 100
byte mac[]    = {  0x00, 0x21, 0xCC, 0x5E, 0x29, 0x28 };
#define MQTT_CLIENTID "d:nrzsro:ArduinoUnoType:0021cc5e2928"
#define PUBLISH_TOPIC "iot-2/evt/status/fmt/json"
#define SUBSCRIBE_TOPIC "iot-2/cmd/+/fmt/json"
#define AUTHTOKEN "6&yLO!Hdb_?oA(NcoP"
#define AUTHMETHOD "use-token-auth"

EthernetClient c;
IPStack ipstack(c);
MQTT::Client<IPStack, Countdown, 100, 1> client = MQTT::Client<IPStack, Countdown, 100, 1>(ipstack);
int incomingByte = 11;
void messageArrived(MQTT::MessageData& md);
String deviceEvent;

// 紅外線
const int irReceiver = 2;                // 紅外線接收器
const int irLed  = 3;                    // 紅外線發射器
const int ledPin = 13;                   // 紅外線指示燈
const unsigned int frequency = 35000;    // 發射頻率(單位: Hz)

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  
  pinMode(irReceiver, INPUT);           // 把 irReceiver 接腳設置為 INPUT
  pinMode(irLed, OUTPUT);               // 把 irLed 接腳設置為 INPUT
  pinMode(ledPin, OUTPUT);              // 把 ledPin 設置為 OUTPUT
  tone(irLed, frequency);               // 產生指定頻率的脈波 (Pulses)  
  delay(1000);
}

// 讓指示燈閃爍幾下 
void turn_on_LED() {
  for (byte i=0;i<5;i++){
    digitalWrite(ledPin, HIGH);          // 打開指示燈
    delay(100);
    digitalWrite(ledPin, LOW);          // 打開指示燈
    delay(100);
  }
}

void loop() {
  int rc = -1;
  if (!client.isConnected()) {
    Serial.println("Connecting to IoT Foundation");
    while (rc != 0) {
      rc = ipstack.connect(MS_PROXY, MQTT_PORT);
      Serial.println("trying..");
    }
     
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = MQTT_CLIENTID;
    data.username.cstring = AUTHMETHOD;
    data.password.cstring = AUTHTOKEN;
    rc = -1;
    while ((rc = client.connect(data)) != 0)
    ;

    client.unsubscribe(SUBSCRIBE_TOPIC);
    //Try to subscribe for commands
    if ((rc = client.subscribe(SUBSCRIBE_TOPIC, MQTT::QOS0, messageArrived)) != 0) {
            Serial.print("Subscribe failed with return code : ");
            Serial.println(rc);
    } else {
          Serial.println("Subscribed\n");
    }
    Serial.println("Subscription tried......");

    Serial.println("Connected successfully\n");
    Serial.println("Value\Device Event (JSON)");
    Serial.println("____________________________________________________________________________");
  }
 
  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  deviceEvent = String("{\"d\":{\"myName\":\"Arduino Uno\",\"value\":");
  char buffer[60];
  //   if(Serial.available()>0) {
    //incomingByte = Serial.read();
 // }
 
//  dtostrf(getValue(),1,2, buffer);
  int gv = getValue();
  if ( gv == 0){
    deviceEvent += "0";
    deviceEvent += "}}";
    Serial.print("\t");
    Serial.print("0");
    Serial.print("\t\t");
    deviceEvent.toCharArray(buffer, 60);
    Serial.println(buffer);
    message.payload = buffer;
    message.payloadlen = strlen(buffer);
    rc = client.publish(PUBLISH_TOPIC, message);
  
    if (rc != 0) {
      Serial.print("return code from publish was ");
      Serial.println(rc);
    }
    client.yield(1000);
  }
}

void messageArrived(MQTT::MessageData& md) {
  Serial.print("\nMessage Received\t");
    MQTT::Message &message = md.message;
    int topicLen = strlen(md.topicName.lenstring.data) + 1;

    char * topic = md.topicName.lenstring.data;
    topic[topicLen] = '\0';  
    int payloadLen = message.payloadlen + 1;
    
    char * payload = (char*)message.payload;
    payload[payloadLen] = '\0';  
    
    //Command topic: iot-2/cmd/alerm/fmt/json

    if(strstr(topic, "/cmd/alerm") != NULL) {
      Serial.print("Command IS Supported : ");
      Serial.print(payload);
      Serial.println("\t.....\n");

      turn_on_LED();                             // 讓指示燈 on
      
    } else {
      Serial.println("Command Not Supported:");            
    }
}


int getValue(void) {
  
// 紅外線
  int ir_status = digitalRead(irReceiver);   // 讀取 irReceiver 的狀態
  
  // 檢查 irReceiver 是否有收到紅外線訊號
  // 有的話，ir_status 會是 0 (因為 Receiver 會把訊號反向, 所以 0 代表有收到訊號)

  return (ir_status);
}
