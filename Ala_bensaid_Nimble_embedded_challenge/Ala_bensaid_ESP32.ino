//Ala ben said 
//Nimble embedded challenge
//FreeRTOS application using ESP32-WROOM32
#include "WiFi.h"
#include <PubSubClient.h>



const char *ssid = "GNET-84938"; // WiFi name
const char *password = "73382639";  //WiFi password
const char *topic1 = "command/target/mqtt" ;
const char *topic2 = "feedback/mqtt" ;
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *client_id = "the_client" ;
bool signal_quit ;
byte n ;
int element ;
bool p ;
int i ;
int v ;
int command_TargetPosition ;
int m ;
int w ;
WiFiClient espClient;
PubSubClient client(espClient);
struct FeedbackStruct {int TargetPosition ; int CurrentPosition ;} ;
FeedbackStruct feedback ;
QueueHandle_t Q1;   /////// command queue


void setup()
{
command_TargetPosition = 0 ;    // this one will be update by serial task
Q1 = xQueueCreate( 5, sizeof(int) ); 
Serial.begin(112500);
feedback.TargetPosition = 0 ;  //zeroed FeedbackStruct. will be update by MQTT
feedback.CurrentPosition = 0 ; //zeroed FeedbackStruct.will be update by MQTT
delay(1000);
//wifi & MQTT server initialization
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     //Serial.println("Connecting to WiFi..");
 }
 //Serial.println("Connected to the WiFi network");
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     //Serial.println("Connecting to public emqx mqtt broker.....");
     if (client.connect(client_id)) {
        // Serial.println("Public emqx mqtt broker connected");
     } else {
         //Serial.print("failed with state ");
         //Serial.print(client.state());
         delay(2000);
     }
 }

client.subscribe(topic2);

xTaskCreate(Serial_Task,"serial_task",10000,NULL,1,NULL); 
xTaskCreate(Transmitter,"TX",10000,NULL,1,NULL); ///// MQTT TX TASK

}


//////////////////////////////
void loop()
{  
client.loop();
delay(1000);
}
//////////////////////////





///////////////////////////
//MQTTT TX TASK this function will publish a command from command queue (if it's not empty)
///////////////////////////
void Transmitter(void * parameter) {
  while(true){
     delay(10); ///100 units/seconds
     if(!((feedback.CurrentPosition-command_TargetPosition)==0)){ //compare feedback and command
     v= feedback.CurrentPosition ;
     xQueueSend(Q1 ,&v,100);
                   }
    xQueueReceive(Q1, &w, portMAX_DELAY); //wait until new command target value(queue not empty)
    char message_buff[5] ;
    String pubString = String(w);
    pubString.toCharArray(message_buff, pubString.length()+1);
    client.publish(topic1,message_buff);// send command_target_position to motor simulation
           }
    /*if (signal_quit){
      vTaskDelete(NULL) ;
    }*/
}




//////////////////////////////
// MQTT RX TASK This function will receive feedback from python application and update them to feedback struct
////////////////////////////
void callback(char *topic2, byte *payload, unsigned int length) {
  char s[5];
 int Number  ;
payload[length] = '\0'; 
Number = atoi((char *)payload);
 if (p){
  feedback.CurrentPosition = Number ;  // update feedback struct
 p = false ;
 }
  else {
  feedback.TargetPosition = Number ;  //update feedback struct
 p = true ;
 }

}


/////////////////////////////
//This function will receive and send data(target position & feedback & q signal ) to python application using UART protocol
////////////////////////////
void Serial_Task (void * parameter) {

  int k  ;
  while(true){
  while(Serial.available()){
          delay(100);
           if ( k == 1){
           command_TargetPosition = Serial.read(); //get target position from python application
           k = 0;
           delay(100) ;
                        }
          switch(Serial.read()){
                 case 'f' :
                              // send feedback structure to the python application 
                              Serial.write(feedback.CurrentPosition); 
                              Serial.write(feedback.TargetPosition);
                 case 'q' : 
                                   //exit program
                                    command_TargetPosition = 0 ;
                                    feedback.TargetPosition = 0 ;
                                    feedback.CurrentPosition  = 0 ;
                                    signal_quit = true;
                 case 'c' :
                                 k = 1 ;
                  
          }
          }
  }
  delay(10);
  }
  
