# Modbus Master for 28335 and possibly 28379d

This project enables your TI F28335 DSP to be a Modbus Master client. It uses FIFO enhancements and don't use any ISR.

----Hoping to adapt this code to use with the TMS320F28379D DSP 

# Using

* Create a new project at CCS
* Insert all files from this repository on your project
* Import the CCS configs (at the "ccs_configs" folder)
* Compile and run

----I will leave a example code in the example folder which expect to read the temperature of a XY-MD02 humidity and temperature sensor conected in SCI-B trough a ttl-rs485(max485) adapter connected in gpios 19(RX) and 18(TX) which are the pins 3 and 4 on the launchxl board
