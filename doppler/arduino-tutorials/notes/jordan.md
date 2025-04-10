# Terminology

## Personal Shorthand
- ASW: Arduino software

## serial connection

[Reference](https://learn.sparkfun.com/tutorials/serial-communication)
 - rx: receiver
 - tx: transmitter

## serial terminal

[Reference](https://learn.sparkfun.com/tutorials/terminal-basics/all)

"you cannot issue command line statements within a terminal window"

- baud rate: how fast your data is being transmitted and received (9600 bits per second is standard)
- com/serial port: "Your device will show up on your computer as either COM# (if you’re on a Windows machine) or /dev/tty.usbserial-######## (if you’re on a Mac/Linux computer), where the #’s are unique numbers or alphabetic characters."

"within the Arduino environment, [the serial terminal] is called the Serial Monitor"

The Serial Monitor is opened by clicking the little magnifying glass-looking thing in the upper right of the Arduino software UI OR pressing [Ctrl]+[Shift]+[M].
If nothing is connected, clicking that icon or using the keyboard shortcut yields:
`Board at null is not available`
![image](https://github.com/user-attachments/assets/3b640847-ec86-4dc2-af22-8a5fd5133cab)

# Methodology

## Making a connection

Plugging the Arduino into a USB port makes it to where, after a few seconds, another port option that ends in (Arduino Uno) will pop up in the Tools -> Port dropdown menu in the Arduino Software (ASW).
