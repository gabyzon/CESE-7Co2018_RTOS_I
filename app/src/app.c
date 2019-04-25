#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "app.h"
#include "MEFbutton_refresh.h"

// Defino las maquinas de estados para los botones
MEFbutton_t MEFbutton1, MEFbutton2, MEFbutton3, MEFbutton4; 
// Creo las estructuras para las colas
QueueHandle_t cola_1, cola_2;
// Creo las estructuras para los mutex
SemaphoreHandle_t mutex1, mutex2;

// Defino variables globales
uint8_t N = 10, n=0; // cantidad de estímulos por defecto que se realizarán

// Defino los stacks
StackType_t buttonTaskStack[configMINIMAL_STACK_SIZE];
StackType_t mainTaskStack[configMINIMAL_STACK_SIZE];
StackType_t chronoTaskStack[configMINIMAL_STACK_SIZE];
StackType_t uartTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t buttonTaskTCB;
StaticTask_t mainTaskTCB;
StaticTask_t chronoTaskTCB;
StaticTask_t uartTaskTCB;

// Declaro los prototipos de tareas
void taskButton( void* taskParmPtr );
void taskMain( void* taskParmPtr );
void taskChrono( void* taskParmPtr );
void taskUART( void* taskParmPtr );

int main(void)
{
    // Configuro la CIAA
    boardConfig();
    
    // Creo las tareas a utilizar
    xTaskCreateStatic( taskButton, "taskButton", configMINIMAL_STACK_SIZE, NULL,
                        tskIDLE_PRIORITY+3, buttonTaskStack, &buttonTaskTCB);

    xTaskCreateStatic( taskMain, "taskMain", configMINIMAL_STACK_SIZE, NULL,
                        tskIDLE_PRIORITY+2, mainTaskStack, &mainTaskTCB);    
    
    xTaskCreateStatic( taskChrono, "taskChrono", configMINIMAL_STACK_SIZE, NULL,
                        tskIDLE_PRIORITY+2, chronoTaskStack, &chronoTaskTCB); 
    
    xTaskCreateStatic( taskUART, "taskUART", configMINIMAL_STACK_SIZE, NULL,
                        tskIDLE_PRIORITY+1, uartTaskStack, &uartTaskTCB);     

    vTaskStartScheduler();
   
    // Si el Scheduler no se inicia, entra a este while donde enciende un led rojo indicando el error.
    while(1){gpioWrite(LEDR,ON);}

    return 0;
}

/* Tarea principal, taskMain */
void taskMain( void* taskParmPtr )
{
    // Me aseguro que todos los leds esten apagados
    gpioWrite(LED1, OFF);
    gpioWrite(LED2, OFF);
    gpioWrite(LED3, OFF);
    gpioWrite(LEDR, OFF);
    gpioWrite(LEDG, OFF);
    gpioWrite(LEDB, OFF);
    
    // Defino una variable x donde se crearán los valores aleatorios necesarios.
    uint8_t x = 0;

    BaseType_t rv1;
    
    // Creo la cola de mensaje entre taskMain y taskChrono (cola_1)
    cola_1 = xQueueCreate( 1 , sizeof(uint8_t) );

    // Creo la cola de mensaje entre taskChrono y taskUART (cola_2)
    cola_2 = xQueueCreate( N , sizeof(uint32_t) );

    // Creo un mutex para la variable n
    mutex1 = xSemaphoreCreateMutex();

    // Creo un mutex para la variable N
    mutex2 = xSemaphoreCreateMutex();
    
    // Hago el control de errores
    if( mutex1==NULL || mutex2==NULL || cola_1==NULL || cola_2==NULL ){
        error_mje();
    }
    
    // Esta es una tarea periodica cada 5 ms
    portTickType xPeriodicityTmain =  25 / portTICK_RATE_MS;
    portTickType xLastWakeTimeTmain = xTaskGetTickCount();

    while(TRUE) 
    {
        // Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
        vTaskDelayUntil( &xLastWakeTimeTmain, xPeriodicityTmain );

        if(MEFbutton1 == B_asc)
        {
            while(n<=N)
            {
                x=rand()% 4 + 1; // Genera un valor pseudo aleatorio entre 1 y 4 
                rv1 = xQueueSend( cola_1, &x, 100); // Si despues de 100 ticks no se leyo la cola, envía otro numero
            }
            // Una vez que se ejecutaron los N ejercicios:
            xQueueReset(cola_1); // Elimino el datos que pueda haber quedado en la cola_1
            
            // Tomo el mutex para la variable n.
            xSemaphoreTake( mutex1 , 1000 );
                n = 0; // Reinicio el contador de estímulos realizados
            xSemaphoreGive( mutex1 );
            
            // Hago parpadear todos los led para indicar que termino el test.
            vTaskDelay( 1000 / portTICK_RATE_MS );
            gpioWrite(LEDB, ON);  
            gpioWrite(LED1, ON);  
            gpioWrite(LED2, ON);  
            gpioWrite(LED3, ON);
            vTaskDelay( 1000 / portTICK_RATE_MS ); 
            gpioWrite(LEDB, OFF);  
            gpioWrite(LED1, OFF);  
            gpioWrite(LED2, OFF);  
            gpioWrite(LED3, OFF);  
        }
    }
}

/* Tarea de cronometraje */
void taskChrono( void* taskParmPtr )
{
    uint8_t L = 0;
    uint32_t counterL1 = 0, counterL2 = 0, counterL3 = 0, counterL4 = 0;
    
    BaseType_t rv1, rv2;
    
    while(TRUE) {
        
        if( n<=N ){
            rv1 = xQueueReceive( cola_1, &L, 0xffffffff ); // Se bloquea "indefinidamente" hasta recibir un dato
            if( rv1==pdFALSE ){error_mje();} // Control de error.
            // Tomo el mutex para la variable n.
            xSemaphoreTake( mutex1 , 1000 );
                n++; // Sumo uno al contador de estímulos realizados
            xSemaphoreGive( mutex1 );
        }
        
        vTaskDelay( 1000 / portTICK_RATE_MS ); // Una vez que recibe un dato, espera 1 segundo
        
        if( L==1 )
        {
            counterL1 = 0; // Coloco a cero el contador del led 1
            gpioWrite(LEDB, ON);
            while( gpioRead(LEDB) )
            {
                counterL1++;
                if( MEFbutton1 == B_desc ){
                    // Al presionar la tecla correspondiente
                    gpioWrite(LEDB, OFF); // Apago el led
                    L=0; // Borro el numero de led a encender
                    // Cuando se apaga el led, envío el resultado (counterL1) del contador a la UART por la cola_2
                    rv2 = xQueueSend( cola_2, &counterL1, 100); // Envío el resultado a la tarea del UART
                    if( rv2==errQUEUE_FULL ){error_mje();} // Control de error.
                }
                // Envia la tarea al estado bloqueado durante 1 ms
                vTaskDelay( 1 / portTICK_RATE_MS );
            }
        }
        else if( L==2 )
        {
            counterL2 = 0; // Coloco a cero el contador del led 2
            gpioWrite(LED1, ON);
            while( gpioRead(LED1) )
            {
                counterL2++;
                if( MEFbutton2 == B_desc ){
                    // Al presionar la tecla correspondiente
                    gpioWrite(LED1, OFF); // Apago el led
                    L=0; // Borro el numero de led a encender
                    // Cuando se apaga el led, envío el resultado (counterL2) del contador a la UART por la cola_2
                    rv2 = xQueueSend( cola_2, &counterL2, 100);
                    if( rv2==errQUEUE_FULL ){error_mje();} // Control de error.
                }
                // Envia la tarea al estado bloqueado durante 1 ms
                vTaskDelay( 1 / portTICK_RATE_MS );
            }
        }
        else if( L==3 )
        {
            counterL3 = 0; // Coloco a cero el contador del led 3            
            gpioWrite(LED2, ON);
            while( gpioRead(LED2) )
            {
                counterL3++;
                if( MEFbutton3 == B_desc ){
                    // Al presionar la tecla correspondiente
                    gpioWrite(LED2, OFF); // Apago el led
                    L=0; // Borro el numero de led a encender
                    // Cuando se apaga el led, envío el resultado (counterL3) del contador a la UART por la cola_2
                    rv2 = xQueueSend( cola_2, &counterL3, 100);
                    if( rv2==errQUEUE_FULL ){error_mje();} // Control de error.
                }
                // Envia la tarea al estado bloqueado durante 1 ms
                vTaskDelay( 1 / portTICK_RATE_MS );
            }
        }
        else if( L==4 )
        {
            counterL4 = 0; // Coloco a cero el contador del led 4
            gpioWrite(LED3, ON);
            while( gpioRead(LED3) )
            {
                counterL4++;
                if( MEFbutton4 == B_desc ){
                    // Al presionar la tecla correspondiente
                    gpioWrite(LED3, OFF); // Apago el led
                    L=0; // Borro el numero de led a encender
                    // Cuando se apaga el led, envío el resultado (counterL4) del contador a la UART por la cola_2
                    rv2 = xQueueSend( cola_2, &counterL4, 100);
                    if( rv2==errQUEUE_FULL ){error_mje();} // Control de error.
                }
                // Envia la tarea al estado bloqueado durante 1 ms
                vTaskDelay( 1 / portTICK_RATE_MS );
            }
        }
    }
}

/* Tarea de comunicación */
void taskUART( void* taskParmPtr )
{
    // Configuro el puerto
    /* Inicializar UART_USB a 115200 baudios */
	uartConfig( UART_USB, 115200 );
    
    uint32_t counter = 0;
    char conta[8];
    
    BaseType_t rv2;
    
    // Esta es una tarea periodica cada 100 ms
    portTickType xPeriodicityTuart =  100 / portTICK_RATE_MS;
    portTickType xLastWakeTimeTuart = xTaskGetTickCount();
    
    while(TRUE)
    {
        vTaskDelayUntil( &xLastWakeTimeTuart, xPeriodicityTuart );
        counter = 0; // Pongo a cero el buffer de lectura de la cola_2
        rv2 = xQueueReceive( cola_2, &counter, 0xffffffff ); // Se mantiene bloqueado hasta que recibe un dato.
        if( rv2==pdFALSE ){error_mje();} // Control de error.
        if(n==1){uartWriteString( UART_USB, " Inicia el Test \r\n");}
        __utoa(counter, conta, 10); // Convierto el dato (en hex) en un string con el valor decimal correspondiente.
        uartWriteString( UART_USB, conta ); // Envío el resultado por la UART
        uartWriteString( UART_USB, " ms \r\n");
        if(n==N){uartWriteString( UART_USB, " Test finalizado \r\n");}
    }
}


/* Tarea de actualización de MEF de las teclas */
void taskButton( void* taskParmPtr )
{
    // Configuro los valores iniciales de las MEF de los botones
    MEFbutton1 = B_arriba;
    MEFbutton2 = B_arriba;
    MEFbutton3 = B_arriba;
    MEFbutton4 = B_arriba;
    
    // Esta es una tarea periodica cada 1 ms
    portTickType xPeriodicityButton =  1 / portTICK_RATE_MS;
    portTickType xLastWakeTimeButton = xTaskGetTickCount();
    
    while(TRUE)
    {
        // Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
        vTaskDelayUntil( &xLastWakeTimeButton, xPeriodicityButton );
        // Actualizo el estado de las teclas.
        MEFbutton_refresh(TEC1, &MEFbutton1);
        MEFbutton_refresh(TEC2, &MEFbutton2);
        MEFbutton_refresh(TEC3, &MEFbutton3);
        MEFbutton_refresh(TEC4, &MEFbutton4);
    }
    
    return;
}