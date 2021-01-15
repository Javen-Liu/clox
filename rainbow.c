//
// Created by Javen on 2021/1/12.
//

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "rainbow.h"

#define F 0.1
#define PI 3.14
#define RED(i)      sin(F*(float)(i)+0) * 127 + 128
#define GREEN(i)    sin(F*(float)(i)+2*PI/3) * 127 + 128
#define BLUE(i)     sin(F*(float)(i)+4*PI/3) * 127 + 128

void rainbows(const char *message){
    int len = (int) strlen(message);
    for(int i = 0; i < len; i++){
        int r = RED(i), g = GREEN(i), b = BLUE(i);
        printf("\033[38;2;%d;%d;%dm%c\033[0m", r, g, b, message[i]);
    }
}

void warnings(const char *message){
    int len = (int) strlen(message);
    for(int i = 0; i < len; i++){
        printf("\033[38;2;%d;%d;%dm%c\033[0m", 255, 215, 0, message[i]);
    }
}

//243,132,130
void errors(const char *message){
    int len = (int) strlen(message);
    for(int i = 0; i < len; i++){
        printf("\033[38;2;%d;%d;%dm%c\033[0m", 243, 132, 130, message[i]);
    }
}

void test(){
    rainbows("hello world, this is the print of rainbow!");
    warnings("hello world, this is the warning print");
    errors("hello world, this is the error print");
}