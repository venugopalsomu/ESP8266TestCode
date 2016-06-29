
//wifi switch with ESP8266 using MQTT.

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
//#include <Bounce2.h>
#include <EEPROM.h>
#include <string>
 

//global variables:
#define RELAY_PIN_1  12  // Arduino Digital I/O pin number for relay (ESP8266)
#define BUTTON_PIN_1  0  // Arduino Digital I/O pin number for button 


//Eeeprom Address allocation
#define EEPROM_RELAY_ADDRESS_0 0
#define EEPROM_SWITCH_ADDRESS_0 1
#define EEPROM_MQTT_ADDRESS_0 2

//#define CHILD_ID 1   // Id of the sensor child // for now using mac address
#define RELAY_ON 1
#define RELAY_OFF 0

#define BUFFER_SIZE 20

struct wifiSwitchStatus
{
   int swithcPinNum;
   int RelayPinNum;
   boolean relayOnOffStatus;
   boolean switchOnOffStatus;
   boolean eepromRelayOnOffStatus;
   boolean eepromSwitchOnOffStatus;
   boolean eepromMqttOnOffStatus;
   boolean eepromRelayAddress;
   boolean eepromSwitchAddress;
   boolean eepromMqttAddress;
};

static struct wifiSwitchStatus SwitchStatus[1];

//int oldValue=0;
byte state;
boolean Switchstatusupdate = false;

//EEPROM configurations
// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
//int addr = 0;
#define PREV_RELAYSTATUS_ADR 0
#define PREV_SWITCH_STATUS_ADR 1
#define PREV_MQTT_STATUS_ADR 2

// Replace with your Wifi SSID and passphrase
const char* ssid = "JOSHITHA";
const char* pass = "saradadevi";

const char *server_ip = "m11.cloudmqtt.com";
const int mqtt_port = 18500;
const char *mqtt_user = "venugopal";
const char *mqtt_pass = "432112";

const char* topic[]={"test/relayStatus0", "test/relayStatus1", "test/relayStatus2"};

// Replace with the IP address of your MQTT server
//IPAddress server_ip(192,168,0,109);

//
// toString function header
//
String toString(byte* payload, unsigned int length);

void initHwPins()
{
  SwitchStatus[0].swithcPinNum = BUTTON_PIN_1;
  SwitchStatus[0].RelayPinNum = RELAY_PIN_1;


  SwitchStatus[0].eepromRelayAddress = EEPROM_RELAY_ADDRESS_0;
  SwitchStatus[0].eepromSwitchAddress = EEPROM_SWITCH_ADDRESS_0;
  SwitchStatus[0].eepromMqttAddress = EEPROM_MQTT_ADDRESS_0;

}

void initPinconfiguration()
{
    int i=0;
    // Setup the button
    for ( i=0; i<1; i++)
    {
        // Set internal PULLUP
        pinMode(SwitchStatus[i].swithcPinNum, INPUT_PULLUP);
        //pinMode(SwitchStatus[i].swithcPinNum, INPUT);
        // Activate internal pull-up
        digitalWrite(SwitchStatus[i].swithcPinNum, HIGH);
    }
	
    // Set the default status for the RELAY
    for ( i=0; i<1; i++)
    {
        // Make sure relays are off when starting up
        digitalWrite(SwitchStatus[i].RelayPinNum, RELAY_OFF);
        // Then set relay pins in output mode
        pinMode(SwitchStatus[i].RelayPinNum, OUTPUT);
    }

    // Set relays to last known state (using eeprom storage) 
    EEPROM.begin(512);
    //Read all relay status
    for(i=0; i<1; i++)
    {
      state = EEPROM.read(SwitchStatus[i].eepromRelayAddress);
      Serial.print("EEPROM status");
      //Venu update relay number
      Serial.println(state);
      digitalWrite(SwitchStatus[i].RelayPinNum, state);
    }
    //read ans store the Switch Status from EEPROM
    for(i=0; i<1; i++)
    {
      SwitchStatus[i].switchOnOffStatus = EEPROM.read(SwitchStatus[i].eepromSwitchAddress);
    }
    
}



void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.print("ESP8266 Wifi Switch ");

  initHwPins();
  initPinconfiguration();  
}

WiFiClient wclient;
PubSubClient client(wclient, server_ip, mqtt_port);

#define STATUS_ON 49
#define STATUS_OFF 48

void receive_data(const MQTT::Publish& pub) {
  int topicnum =4;  //4 is invald topic, use macro
  Serial.print(pub.topic());
  Serial.print(" => ");

  //find which topic has data
  for(int i=0; i<1; i++)
  {
      if(pub.topic()==topic[i])
      {
        topicnum =i;
        break;
      }
  }
  if(topicnum >1)
    return;
  
  if (pub.has_stream()) {
    return;
  } 
  else
  {
      uint8_t buf[BUFFER_SIZE];
      String msg = pub.payload_string();
      boolean OnOff;
      state = (int)msg[0];
      if(state == STATUS_ON) // 49 -> ascii 1
      {
          OnOff = RELAY_ON;
      }
      if(state == STATUS_OFF)  //48 -> ascii 0
      {
          OnOff = RELAY_OFF;
      }
      Serial.println(OnOff);
      Serial.print("Topic no:");
      Serial.println(topicnum);
      digitalWrite(SwitchStatus[topicnum].RelayPinNum, OnOff);
      EEPROM.write(SwitchStatus[topicnum].eepromRelayAddress, OnOff); // try not to use every time insted just write if there is a change in status from previous
      EEPROM.commit();
  }
}


//Update it for PUSH button, it was written for physical on off sitch.. plz do that ............................
int j=0;
int val=0;
void SwitchUpdate1()
{
   // Check every relay and update accordingly
   
    for (j=0; j<1; j++)
    {
       val = digitalRead(SwitchStatus[j].swithcPinNum);
       //Serial.print("Switch updated..");
       //Serial.print(j);
       //Serial.print(":");
       //Serial.println(val);
       if(SwitchStatus[j].switchOnOffStatus != val)  // SwitchStatus[i].switchOnOffStatus will contain previous Switch data
       {
          Serial.print("Switch updated..");
          Serial.println(val);
          digitalWrite(SwitchStatus[j].RelayPinNum, val); //Inverse the state
     
          //update it to cloud aswell
         if (client.connected())
         {
           //  client.publish(topic[j], String(val));
         }
             
          //write to EEPROM
          EEPROM.write(SwitchStatus[j].eepromSwitchAddress, (byte)val);
          EEPROM.commit();

          SwitchStatus[j].switchOnOffStatus = val;
      }
    }
}

void loop() {
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      // Give ourselves a unique client name
      Serial.println("Connecting to client ");
      if (client.connect(MQTT::Connect(WiFi.macAddress()).set_auth(mqtt_user, mqtt_pass))) {
        
        client.set_callback(receive_data);   // Register our callback for receiving DATA
        for(int i=0;i<1; i++)
        {
           Serial.print("Subscribing to topic: ");
           Serial.println(topic[i]);
           client.subscribe(topic[i]);
        }
      }
    }
  }
    
    SwitchUpdate1();
    if (client.connected())
    {
      client.loop();
    }
}



//
// toString function
//
String toString(byte* payload, unsigned int length) {
  int i = 0;
  char buff[length + 1];
  for (i = 0; i < length; i++) {
    buff[i] = payload[i];
  }
  buff[i] = '\0';
  String msg = String(buff);
  return msg;
}


