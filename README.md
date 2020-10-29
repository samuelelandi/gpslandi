GPSLANDI  is a TCP gateway for a GPS module with RS-232 interface.
The program works on any flavour of Linux, anyway it has been built for
Raspberry PI (2,3,4).
The GPS module tested is a modern Ultimate GPS breakout 66 channels 10HZ
updates: https://www.adafruit.com/product/746
The gateway decodes standard NMEA-00183 message, so it should work with any
compatible GPS module.
This module is part of project for a drone boat,anyway it's usable for any
different purpose where a GPS location is required.

Authors: Achille and Samuele Landi

Configuration:
Edit the file gps-landi-server.c and change the following line, replacing
/dev/ttyS0 with the serial port where the GPS is wired:
#define RS232 "/dev/ttyS0"

Requirements:
gcc compiler only, for Raspberry PI you could install the following package:
apt-get install build-essential

Build:
build the program executing from the folder of download
./compile.sh

Run:
./gps-landi-server


The gateway will listen on port 8000/tcp for a connection. As soon a
connection arrives,it send the last know position and disconnect.

You can try to connect to the gateway from the same device with:
telnet 127.0.0.1 8000

Additional information are shown on the screen and written to the syslog file.

For any help, please drop an email to: samuele@landi.ae or achille@landi.ae












