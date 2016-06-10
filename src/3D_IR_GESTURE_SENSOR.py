#   Arduino 3D Gesture Recognizer based on IR Photodiodes
#
#   Created by Nicolas Britos
#
#   May, 2016
#
#   V1.0
#
#   iosnicoib@hotmail.com
#

import serial
import time
import requests

#
port = "COM3"                   # Serial port.
VLCPassword = "1234"            # Type here the password you chose to control VLC over HTTP.
baud = 115200
#

vlcCommList = ["http://localhost:8080/requests/status.xml?command=pl_pause&id=<id>",    # Play/pause
               "http://localhost:8080/requests/status.xml?command=pl_next",             # Next track
               "http://localhost:8080/requests/status.xml?command=pl_previous",         # Previous track
               "http://localhost:8080/requests/status.xml?command=volume&val=%2B8",     # Volume +8
               "http://localhost:8080/requests/status.xml?command=volume&val=-8"]

# If it can't connect to the Arduino then retry every 5s.
while True:
    try:
        arduino = serial.Serial(port, baud, timeout=.1)
        break
    except:
        print "Couldn't connect to Arduino. Re-trying in 5s"
        time.sleep(5)

# When connected print the serial port and baud rate.
print "Arduino connected on port: %s with baud: %d" % (port, baud)

def readArduino():
    global ardComm

    ardComm = arduino.readline()        # Just reads the serial port and stores it in "ardComm"


def processArduino():
    if ardComm != "":                       # If ardComm is not empty then process it.
        if ardComm == "EAST\r\n":           # If ardComm is "EAST" then
            s = requests.Session()          # Open connection
            s.auth = ('', VLCPassword)      # Connect to the VLC app with the previous declared password.
            try:
                requests.get(vlcCommList[3], auth=('', VLCPassword))     # V+
            except requests.exceptions.ConnectionError:
                pass
        elif ardComm == "WEST\r\n":
            s = requests.Session()
            s.auth = ('', VLCPassword)
            try:
                requests.get(vlcCommList[4], auth=('', VLCPassword))     # V-
            except requests.exceptions.ConnectionError:
                pass
        elif ardComm == "HOLD EAST\r\n":
            s = requests.Session()
            s.auth = ('', VLCPassword)
            try:
                requests.get(vlcCommList[3], auth=('', VLCPassword))     # V+
            except requests.exceptions.ConnectionError:
                pass
        elif ardComm == "HOLD WEST\r\n":
            s = requests.Session()
            s.auth = ('', VLCPassword)
            try:
                requests.get(vlcCommList[4], auth=('', VLCPassword))     # V-
            except requests.exceptions.ConnectionError:
                pass
        elif ardComm == "SOUTH\r\n":
            s = requests.Session()
            s.auth = ('', VLCPassword)
            try:
                requests.get(vlcCommList[1], auth=('', VLCPassword))     # Next track
            except requests.exceptions.ConnectionError:
                pass
        elif ardComm == "NORTH\r\n":
            s = requests.Session()
            s.auth = ('', VLCPassword)
            try:
                requests.get(vlcCommList[2], auth=('', VLCPassword))     # Previous track
            except requests.exceptions.ConnectionError:
                pass
        elif ardComm == "DOWN\r\n":
            s = requests.Session()
            s.auth = ('', VLCPassword)
            try:
                requests.get(vlcCommList[0], auth=('', VLCPassword))     # Play/Pause
            except requests.exceptions.ConnectionError:
                pass

def main():
    # Pretty self-explanatory
    while True:
        readArduino()
        processArduino()


if __name__ == '__main__':
    main()                  # Call main()
