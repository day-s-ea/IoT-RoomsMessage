import uuid
import paho.mqtt.client as paho
import json
import serial
import time

broker="broker.hivemq.com"                              
port=1883

def on_message(client, userdata, msg):  # The callback for when a PUBLISH message is received from the server.
    msgPayload = str(msg.payload).replace("b'","").replace("'","") # message received
    print("-------------------------------------------------")
    print("Message received \nTopic: " + msg.topic + "\nMessage: " + msgPayload)  # Print the topic from which the text message came and the message itself
    # Check if it is a json format
    try:
        jsonMsg = json.loads(str(msgPayload))
        pass
    except:
        print("Error with message convertion into json")
        return False
    
    # id Gateway string
    idGateway = str(jsonMsg['idGateway'])

    # route the message to the right gateway
    if(idGateway == "1"):
        #message string

        idRoom = str(jsonMsg['idRoom'])
        time = str(jsonMsg['time'])
        msgPic = str(jsonMsg['message'])

        #connection Port
        ser = serial.Serial()

        ser.baudrate = 9600
        ser.port = 'COM5'
        #ser.timeout =0
        ser.stopbits = serial.STOPBITS_ONE
        ser.bytesize = 8
        ser.parity = serial.PARITY_NONE
        ser.rtscts = 0

        if(ser.isOpen() == False):
            try:
                ser.open()
            except:
                print("Error with Serial Port Open")
                return False
        
        #encoded = strToSendForPic.encode('utf-8')

        # I process the data into a string first 
        # Add final char
        msgToSand = idRoom + time + msgPic + "\n"
        encoded = msgToSand.encode('utf-8')
        ser.write(encoded)
        print("Data Send to Pic")

client1= paho.Client("msgWelcome" + uuid.uuid4().hex[:6].upper())        #create client object
#client1.username_pw_set("", "") for the password
client1.on_message = on_message                     #assign function to callback
client1.connect(broker,port)                        #establish connection
client1.subscribe("bedroom/msgWelcome")                  #sub
client1.loop_forever()