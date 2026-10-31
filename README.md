# ATmega2560 Bare-Metal I2C Implementation for LSM6DSOX
This is a basic implementation of the I2C standard using ATmega2560 micro-controller registers for communication with the LSM6DSOX sensor. The program realizes data transmission for Arduino -> PC and Arduino <-> LSM6DSOX communication.

## Description
The implementation allows for:
* Initialization of any LSM6DSOX registers.
* Reading data from LSM6DSOX registers.
* Handling the built-in FIFO memory.
* Triggering data send operations via INT1 from the LSM6DSOX sensor.
