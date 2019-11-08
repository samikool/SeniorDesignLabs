import serial
import time
import numpy as np
from twilio.rest import Client
import tkinter as Tk
import sys 
import time
import datetime

arduino = 0
account_sid = ''
auth_token = ''
fromPhone = ''
toPhone = ''
connected = False
wantMessage = True

def receive():
    global arduino
    global connected
    try:
        while arduino.in_waiting < 3:
            time.sleep(.001)
        b = arduino.readline()
        string_n = b.decode()
        decoded_bytes = string_n.rstrip()
        arduino.reset_input_buffer()
        return decoded_bytes
    except Exception as e:
        print(e)
        connected = False
        return np.nan
###################################################
def sendText(message):
    # Your Account Sid and Auth Token from twilio.com/console
    # DANGER! This is insecure. See http://twil.io/secure
    global account_sid
    global auth_token
    global fromPhone
    global toPhone
    client = Client(account_sid, auth_token)

    message = client.messages.create(body=message,from_=fromPhone,to=toPhone)

    print(message.sid)

##################################################################
def mainLoop():
    global connected
    global arduino
    global wantMessage
    global outputText
    global resetButton
    global resetButtonText
    global timeText

    while True:
        now = datetime.datetime.now()
        hh = str(now.hour)
        mm = str(now.minute)
        ss = str(now.second)
        ampm = 'PM'
        if(len(hh) is 1):
            hh = '0' + hh
            ampm = 'AM'
        if(len(mm) is 1):
            mm = '0' + mm
        if(len(ss) is 1):
            ss = '0' + ss
        
        timeText.set('Time: '+ hh + ':' + mm + ':' + ss +' ' + ampm)
        root.update()
        try:
            if connected == False:
                if serial.Serial('COM3', 9600, timeout=0).isOpen():
                    arduino = serial.Serial('COM3', 9600, timeout=0)
                    connected = True
                    set_number()
                    resetButtonText.set('Text Ready')
                    resetButton.config(bg = 'lightgreen')
                    time.sleep(5)
        except Exception as e:
            print(e)
            connected = False
            resetButtonText.set('error')
            resetButton.config(bg = 'pink')
            outputText.set('trying to connect to receiver')
            time.sleep(.85)
        
        #send state to arduino
        #send(str(state))

        #read temperature in C & add x value
        if(connected):
            state = str(receive())
            if state is '1': 
                outputText.set('Blocked')
                outputLabel.config(bg = "pink")
                if(wantMessage is True):
                    sendText("Critical safety event at " + hh + ":" + mm)
                    resetButton.config(bg = "grey")
                    resetButtonText.set("Reset Text")
                    wantMessage = False
            else:
                outputText.set('Receiving Signal')
                outputLabel.config(bg = "lightgreen")

def set_number():
    global account_sid
    global auth_token
    global fromPhone
    global toPhone
    global textSent

    account_sid = 'ACa77a0e8b75884a4ff3ab693eb54f1c60'
    auth_token = '60ecf338694800b9f79043e930d57ec5'
    fromPhone = '+1' + '9073181770'
    toPhone = '+1' + '3198999264'
    print("Phone set to +1 319-899-9264")

def resetText():
    global wantMessage
    wantMessage = True
    resetButton.config(bg = "lightgreen")
    resetButtonText.set("Text Ready")

root = Tk.Tk()

root.title("Receiver Controller")

canvas = Tk.Canvas(root, height = 100, width = 400)
canvas.pack()

frame = Tk.Frame(root, bg='#80d4ff', bd=5)
frame.place(relx=0.5, rely=0, relwidth=1, relheight=0.8, anchor = 'n')

outputText = Tk.StringVar()
outputText.set('trying to connect to receiver')
outputLabel = Tk.Label(frame, textvariable=outputText, font=20, bg='lightblue')
outputLabel.place(relx=0.0, rely = 0.1, relheight=1, relwidth=0.7)

resetButtonText = Tk.StringVar()
resetButtonText.set('Error')
resetButton = Tk.Button(frame, textvariable = resetButtonText, font=20, bg="pink", command=resetText)
resetButton.place(relx=0.7, rely=0.1, relheight=1, relwidth=.3)

bottomFrame = Tk.Frame(root, bg='#80d4ff', bd=5)
bottomFrame.place(relx=0, rely=0.8, relwidth=1, relheight = 0.2)

timeText = Tk.StringVar()
timeLabel = Tk.Label(bottomFrame, textvariable=timeText, font=("Ariel", 10), bg='lightblue')
timeLabel.place(relx=0.0, rely = 0.0, relheight=1, relwidth=1)

root.update()
mainLoop()