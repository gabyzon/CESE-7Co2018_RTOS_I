#include "MEFbutton_refresh.h"

void MEFbutton_refresh(gpioMap_t tecla , MEFbutton_t *estado)
{
    static bool_t flagFalling = FALSE;
    static bool_t flagRising = FALSE;
    
    switch (*estado) {

        case B_arriba:{
            // si la tecla esta presionada, cambio el estado a descendente
            if (!gpioRead( tecla )) {
                *estado = B_desc;
            }
        }
        break;

        case B_abajo:{
            // si la tecla esta liberada, cambio el estado a ascendente
            if (gpioRead( tecla )) {
                *estado = B_asc;
            }
        }
        break;

        case B_desc:{
            /* ENTRADA - si no esta activo el flag, lo activo*/
            if( flagFalling == FALSE ){
                flagFalling = TRUE; // abajo de esto ejecuto la funcion del flag descendente
                /* ejecuto las tareas correspondiente al flanco descendente */
            }    
               
            /* CHECK TRANSITION CONDITIONS */
            // Agrego un delay y luego controlo el estado de la tecla
            vTaskDelay( 50 / portTICK_RATE_MS );
            
            if( gpioRead(tecla) ){
                *estado = B_arriba;
            } 
            else{
                *estado = B_abajo;
            }
           
            /*SALIDA. Si el estado ya no es B_desc, coloco en False el flag */
            if( *estado != B_desc ){
                flagFalling = FALSE;
            }
        }
        break;

        case B_asc:{
            /* ENTRY */
            if( flagRising == FALSE ){
                flagRising = TRUE; // abajo de esto ejecuto la funcion del flag ascendente
                /* ejecuto las tareas correspondiente al flanco ascendente.*/
            }
              
            /* CHECK TRANSITION CONDITIONS */
            // Agrego un delay y luego controlo el estado de la tecla
            vTaskDelay( 50 / portTICK_RATE_MS );
            
            if( gpioRead(tecla) ){
                *estado = B_arriba;
            } 
            else{
                *estado = B_abajo;
            }

            /* SALIDA */
            if( *estado != B_asc ){
                flagRising = FALSE;
            }
        }
        break;

        default:
            *estado = B_arriba;
        break;
    }
return;
}