#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "ubootenv.h"

#include "mfrAVCtrl.h"

int main(int argc, char* argv[])
{
    const char* cmd=argv[1];
    const char* prop=argv[2];
    const char* value=argv[3];

    if ((argc > 4) || (argc < 2)) {
        printf("\nUsage:%s dump|set|get [prop] [value]|enablevideo|disablevideo|enableaudio|disableaudio\n", argv[0]);
        exit(1);  
    }
    if (!strcmp("enablevideo", cmd)) {
        enableVideoOutput(true);
    } else if (!strcmp("disablevideo", cmd)) {
        enableVideoOutput(false);
    } else if (!strcmp("enableaudio", cmd)) {
        enableAudioOutput(true);
    } else if (!strcmp("disableaudio", cmd)) {
        enableAudioOutput(false);
    } else {
        bootenv_init();
        if (!strcmp("dump", cmd)) {
            bootenv_print();
        } else if (!strcmp("get", cmd)) {
            value = bootenv_get(prop);
            printf("{\"%s\":\"%s\"}\n", prop, value);
        } else if (!strcmp("set", cmd)) {
            bootenv_update(prop, value);
        } else {
            printf("\nWrong command: %s\n", cmd);
            printf("\nUsage:%s dump|set|get [prop] [value]|enablevideo|disablevideo|enableaudio|disableaudio\n", argv[0]);
        }
    }

    return 0;     
}                 

