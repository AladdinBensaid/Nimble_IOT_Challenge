#Ala ben said
#Nimble embedded challenge
#python application (motor simulation)

import time
import threading, queue
from paho.mqtt import client as mqtt_client
import keyboard
import serial
 
ser = serial.Serial()
ser.baudrate = 115200
ser.port = 'COM7'
ser.open()
c = 0
l = []
n = 0
test = False
MIN = 0
MAX = 256
a = 0
q = 0
stop = 0
global FeedbackStruct
global CommandStruct
FeedbackStruct = {'current_position':5, 'target_position':6}
CommandStruct = {'target_position':0}
broker = 'broker.emqx.io'
port = 1883
topic = "command/target/mqtt"
topic2 = "feedback/mqtt"
client_id = "the_python_application"


print(" * Set number for target position between 0 and 256 ")
print("  ")
print(" * press Q to exit program ")
print(" ")

#MQTT Connection
def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client




#MQTT publish function : publish feedback struct
def publish(client):
    
    while True:
        msg = FeedbackStruct['current_position']
        msg2 = FeedbackStruct['target_position']
        client.publish(topic2, msg)
        time.sleep(1)
        client.publish(topic2, msg2)




#MQTT subscribe function : listen for new target position & update
def subscribe(client: mqtt_client):
    time.sleep(0.01)
    def on_message(client, userdata, msg):
        #print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        #update 
        FeedbackStruct['current_position']= int(msg.payload.decode())
        #print("FeedbackStruct['current_position']:")
        #print(int(FeedbackStruct['current_position']))
    client.subscribe(topic)
    client.on_message = on_message
    






#input thread task
def input_thread () :
    global condition
    global a
    global n
    global q
    while(True) :
            if (keyboard.is_pressed('q')) :
                        print("exit program")
                        a = 0
                        q = 1
            elif (keyboard.is_pressed('enter')) :
                        time.sleep(0.75)
                        n= 0
                        for i in range(0,len(l),2):
                              n = n+ (l[i]*pow(10,((len(l)/2)-1)-(i/2))) 
                        l.clear()
                        if (n > MAX):
                            print("write number between (0..256) ")
                        else :
                            a = n
            else:
                    try :
                        c = int(keyboard.read_key())
                        l.append(c)
                    except :
                        print(" ")




                        

#Simulation thread
def simulation_thread ():
    
    while(True):
        client = connect_mqtt()
        client.loop_start()
        subscribe(client)
        publish(client)
        client.loop_forever()



        
t1 = threading.Thread(target=input_thread)
t2 = threading.Thread(target=simulation_thread)
t1.start()
t2.start()



#Main thread
while(True):
            CommandStruct['target_position'] = int(a)
            value = bytearray(int(a))
            ser.write('c'.encode())  #send c char + target position value
            time.sleep(1)
            ser.write(value)          

            
            if(q == 1):         
                ser.write('q'.encode()) #send q char to exit program
                q = 0
            start =time.time()
            if ((start-stop)>2) :
                    ser.write('f'.encode()) # send f char every 2 seconds for feedback from esp32 
                    time.sleep(0.1)
                    s = ser.read()
                    if (not(test)) :
                                FeedbackStruct['current_position'] = (ord(s))
                                test = True
                    else :
                                FeedbackStruct['target_position'] = (ord(s))
                                test = False
                    stop = time.time()
                    #print("   ")
                    print("feedback :")
                    print("current_position: " ,FeedbackStruct['current_position'] ,"target_position :" , FeedbackStruct['target_position'])



    
                      

        

