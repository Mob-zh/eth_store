#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

int main()
{

    if (ec_init("eth1"))
    {
        if (ec_config_init(FALSE) > 0)
        {
            printf("Slave found: %d\n", ec_slavecount);

            ec_slave[0].state = EC_STATE_OPERATIONAL;
            ec_writestate(0);
            ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);

            uint8 ledValue = 0x01;
            ec_slave[1].outputs[0] = ledValue;

            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            printf("LED is ON\n");

            osal_usleep(1000000);
            ec_slave[1].outputs[0] = 0x00;
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            printf("LED is ON\n");
        }
        else
        {
            printf("no slave fount\n");
        }
        ec_close();
    }
    else
    {
        printf("Failed to init EtherCAT network");
    }
    return 0;
}