#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <string.h>
#include <unistd.h>

#define LCDAddr 0x27
#define BLEN 1

void write_word(int fd, int data){
    int temp = data;
    if ( BLEN == 1 )
        temp |= 0x08;
    else
        temp &= 0xF7;
    wiringPiI2CWrite(fd, temp);
}

void send_command(int fd, int comm){
    int buf;
    // Send bit7-4 firstly
    buf = comm & 0xF0;
    buf |= 0x04;            // RS = 0, RW = 0, EN = 1
    write_word(fd, buf);
    delay(2);
    buf &= 0xFB;            // Make EN = 0
    write_word(fd, buf);

    // Send bit3-0 secondly
    buf = (comm & 0x0F) << 4;
    buf |= 0x04;            // RS = 0, RW = 0, EN = 1
    write_word(fd, buf);
    delay(2);
    buf &= 0xFB;            // Make EN = 0
    write_word(fd, buf);
}

void send_data(int fd, int data){
    int buf;
    // Send bit7-4 firstly
    buf = data & 0xF0;
    buf |= 0x05;            // RS = 1, RW = 0, EN = 1
    write_word(fd, buf);
    delay(2);
    buf &= 0xFB;            // Make EN = 0
    write_word(fd, buf);

    // Send bit3-0 secondly
    buf = (data & 0x0F) << 4;
    buf |= 0x05;            // RS = 1, RW = 0, EN = 1
    write_word(fd, buf);
    delay(2);
    buf &= 0xFB;            // Make EN = 0
    write_word(fd, buf);
}

void init(int fd){
    send_command(fd, 0x33);    // Must initialize to 8-line mode at first
    delay(5);
    send_command(fd, 0x32);    // Then initialize to 4-line mode
    delay(5);
    send_command(fd, 0x28);    // 2 Lines & 5*7 dots
    delay(5);
    send_command(fd, 0x0C);    // Enable display without cursor
    delay(5);
    send_command(fd, 0x01);    // Clear Screen
    wiringPiI2CWrite(fd, 0x08);
}

void clear(int fd){
    send_command(fd, 0x01);    //clear Screen
}

void writelcd(int fd, int x, int y, char data[]){
    int addr, i;
    int tmp;
    if (x < 0)  x = 0;
    if (x > 15) x = 15;
    if (y < 0)  y = 0;
    if (y > 2)  y = 2;

    // Move cursor
    addr = 0x80 + 0x40 * y + x;
    send_command(fd, addr);
    
    tmp = strlen(data);
    for (i = 0; i < tmp; i++){
        send_data(fd, data[i]);
    }
}

/*
void main(){
    fd = wiringPiI2CSetup(LCDAddr);
    init(fd);
    char buffer[10] = {0};
    for (int i = 0; i<10000; i++)
    {
        usleep(10000);
        sprintf(buffer, "%d", i);
        writelcd(fd, 0, 0, buffer);
    }
}
*/
