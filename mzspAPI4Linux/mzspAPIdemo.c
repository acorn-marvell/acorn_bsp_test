#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
     
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
     
#include <sys/stat.h>
#include <memory.h>
     
#include <sys/time.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "inc/mzspAPI.h"
#include "inc/cli.h"
#include "inc/zigbee_cli.h"
#include "inc/zigbee.h"


int main(int argc, unsigned char * argv[])
{
    uint32_t result;
    uint8_t uartPort;
    debug_printf("[mzspDemo]: ./mzspAPIdemo uartPort\n");

    if(argc < 2){
        debug_printf("invalid parameter \n");
        return 0; 
    }
    uartPort = atoi((unsigned char *)argv[1]);
    
    result = mzgeneral_init(uartPort, MZ_NORMAL_URAT);
    if(result != 0){
        printf("[mzspDemo]:init comport error\n");
        return;
    }

    mzgeneral_setJoinIndFunc(nodeJoinHook);
    mzgeneral_setLeaveIndFunc(nodeLeaveHook);

    //mznmc_setSelfPermitJoin(1);

    cli_init();

    zigbee_cli_init();

    /* hang on and input "CLI_HALT" to exit */
    cli_main_loop();

#if 0
    while(1){
        if(testTargetAddress != 0){
            result = mzzcl_SetAPSHeaderParameters(0x0104,3,3,2,testTargetAddress,2);
            if(result != 0){
                debug_printf("[mzspDemo]:mzzcl_SetAPSHeaderParameters error\n");
            }
            result = mzzcl_switchControl(2);
            if(result != 0){
                debug_printf("[mzspDemo]:mzzcl_switchOnOff failed\n");
            }
            result = mzzcl_levelControl(1,0,1);
            if(result != 0){
                debug_printf("[mzspDemo]:mzzcl_levelControl failed\n");
            }
            result = mzzcl_levelControl(4,100,1);
            if(result != 0){
                debug_printf("[mzspDemo]:mzzcl_levelControl failed\n");
            }
        }
        sleep(5);
    }
#endif
}
