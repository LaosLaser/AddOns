// Write to slave display at device 0x09
#include "mbed.h"
Serial pc(USBTX, USBRX);
I2C i2c(p28, p27);
Ticker timer;

int address = 0x04;

void readI2C() {
        // Test 2: knopjes lezen via I2C (één waarde tegelijk)
        char input[2] = "";
        i2c.read(address,input, 2);
        if (input[0] != 0) {
          pc.printf("I2C read: %d\n", input[0]);
        }
}

int main() {
    int value = 0;
    char msg[33] = "LaserCutterDrivev0.1 Beta   Jaap";
    int counter = 0;
    i2c.write(address, msg, 32);
    pc.printf("\n");
    timer.attach(&readI2C, 5);
    while (1) {
        // Test 1: data schrijven naar het 16x2 scherm via I2C
        // Typ eens wat op de serial terminal, dan komt het op het schermpje!
        if (pc.readable()) {
            value = pc.getc();
            pc.printf("%c", value);
            msg[counter++] = value;
        }
        // vul uit tot 32 characters
        if (value == 13) {
            counter--;
            while (counter<32) {
                msg[counter++] = ' ';
            }
        }
        // Stuur data naar I2C als de buffer vol is
        if (counter == 32) {
            i2c.write(address, msg, 32);
            counter = 0;
            value = 0;
        }      
    }
}
