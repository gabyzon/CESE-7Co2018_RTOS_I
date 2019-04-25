#include "error_mje.h"

void error_mje(void)
{
    while(1)
    {
        gpioWrite(LEDB, ON);
        gpioWrite(LED1, ON);
        gpioWrite(LED2, ON);
        gpioWrite(LED3, ON);
        vTaskDelay( 500 / portTICK_RATE_MS );
        gpioWrite(LEDB, OFF);
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, OFF);
        gpioWrite(LED3, OFF);        
        vTaskDelay( 500 / portTICK_RATE_MS );
    }
    return;
}