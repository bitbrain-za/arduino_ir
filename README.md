# Arduino NEC IR Transmitter
Little IR transmitter for Arduino Mini Pro for use in another project

## Target
[Arduino Pro Mini ATMEGA328](https://store.arduino.cc/usa/arduino-pro-mini)

## Usage

UART comms at 5V, only receives.

###Protocol
  - Start char:'S' 
  - Command (byte)
    - Send a preprogrammed code = 0x04,
    - Stop Sending = 0x06,
    - Send a new code = 0x07
  - Parameters separated by commas
  - End with a comma


####Sending a known code
 S04,XX,YY,
 - XX: code id
 - YY: Number of repeats <FF> is continuous

####Sending an unknown code
 S07,YY,MMMM,SSSS,MMMM,SSSS...
 - YY: Number of repeats <FF> is continuous
 - MMMM is a mark duration in microseconds
 - SSSS is a space duration in microseconds

