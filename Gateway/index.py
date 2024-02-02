import uuid
import paho.mqtt.client as paho
import json
import serial
import time

# Definizione del broker MQTT e della porta
broker = "broker.hivemq.com"
port = 1883

# Callback per la gestione dei messaggi MQTT ricevuti
def on_message(client, userdata, msg):
    # Estrai il payload dal messaggio ricevuto
    msgPayload = str(msg.payload).replace("b'", "").replace("'", "")

    # Stampa il messaggio ricevuto e il topic associato
    print("-------------------------------------------------")
    print("Message received \nTopic: " + msg.topic + "\nMessage: " + msgPayload)

    # Verifica se il messaggio è in formato JSON
    try:
        jsonMsg = json.loads(str(msgPayload))
    except:
        print("Error with message conversion into JSON")
        return False

    # Estrai ID del Gateway
    idGateway = str(jsonMsg['idGateway'])

    # Instrada il messaggio verso il giusto gateway 
    if idGateway == "1":
        # Estrai i dettagli del messaggio
        idRoom = str(jsonMsg['idRoom'])
        time = str(jsonMsg['time'])
        msgPic = str(jsonMsg['message'])

        # Apri la porta seriale per la comunicazione
        ser = serial.Serial()
        ser.baudrate = 9600
        ser.port = 'COM5'
        ser.stopbits = serial.STOPBITS_ONE
        ser.bytesize = 8
        ser.parity = serial.PARITY_NONE
        ser.rtscts = 0

        if not ser.isOpen():
            try:
                ser.open()
            except:
                print("Error opening serial port")
                return False

        # Prepara il messaggio da inviare al dispositivo PIC
        msgToSend = idRoom + time + msgPic + "\n"
        encodedMsg = msgToSend.encode('utf-8')

        # Invia il messaggio codificato tramite comunicazione seriale
        ser.write(encodedMsg)
        print("Data sent to PIC")

# Creazione del client MQTT con un ID univoco
client1 = paho.Client("msgWelcome" + uuid.uuid4().hex[:6].upper())
#client1.username_pw_set("", "") per la password

# Imposta la callback per la gestione dei messaggi
client1.on_message = on_message

# Connessione al broker MQTT
client1.connect(broker,port)

# Iscrizione al topic dove sono attesi i messaggi
client1.subscribe("bedroom/msgWelcome")

# Avvia il loop del client MQTT per mantenere la connessione e processare i messaggi in arrivo
client1.loop_forever()
