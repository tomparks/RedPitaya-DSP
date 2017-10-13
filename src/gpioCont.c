#include "gpioCont.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

//#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
volatile uint8_t *gpio_map;

// I/O access
volatile unsigned *gpio;

int initGPIO() {
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        return 1;
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
        );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED) {
        printf("mmap error 0x%x\n", (uint32_t)GPIO_BASE);//errno also set!
        return 1;
    }

    // Always use volatile pointer!
    gpio = (volatile unsigned *)gpio_map;


    //For testing, GPIO pin number 9 is going to be used

    //Before using pin as output mode, must be config to input mode
    //Changing the mode of pin 9 is in the address +0 (the values stored is 32bits)
    //each 3 bits correspond to a pin (the last 2 bits are reseverd)

    //Init output in GPIO pin number 9
    *(gpio+GPIO_FUNCTION_SEL_0) &= ~(7<<(9*3)); // change the bits 27-29 to input mode (000)

    //Init output in GPIO pin number 9
    *(gpio+GPIO_FUNCTION_SEL_0) |= (1<<(9*3)); // change the bits 27-29 to output mode (001)

    return 0;
}

void signal9(int i) { //send signal in pin 9 if i != 0, else clear it
    if(i != 0) {
        //To set a signal to pin 9, must change the 9th bit to 1 in the address +7
        *(gpio+GPIO_OUTPUT_SET_0) = (1<<9);
    } else {
        //To clear a signal in pin 9, must change the 9th bit to 1 in the address +10
        *(gpio+GPIO_OUTPUT_CLEAR_0) = (1<<9);
    }
}

void test() {

    int g, rep;

// Switch GPIO 7..11 to output mode

/************************************************************************\
* You are about to change the GPIO settings of your computer.          *
* Mess this up and it will stop working!                               *
* It might be a good idea to 'sync' before running this program        *
* so at least you still have your code changes written to the SD-card! *
\************************************************************************/

// Set GPIO pins 7-11 to output
    for (g=7; g<=11; g++) {
        INP_GPIO(g); // must use INP_GPIO before we can use OUT_GPIO
        OUT_GPIO(g);
    }

    for (rep=0; rep<10; rep++) {
        for (g=7; g<=11; g++) {
            GPIO_SET = 1<<g;
            sleep(1);
        }
        for (g=7; g<=11; g++) {
            GPIO_CLR = 1<<g;
            sleep(1);
        }
    }

}