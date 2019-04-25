/*============================================================================
 * Licencia:
 * Autor:
 * Fecha:
 *===========================================================================*/

#ifndef _MEFBUTTON_REFRESH_H_
#define _MEFBUTTON_REFRESH_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "sapi.h"              // <= sAPI header
#include "FreeRTOS.h"
#include "task.h"

/*==================[typedef]================================================*/

typedef enum{
   B_arriba,
   B_abajo,
   B_desc,
   B_asc
} MEFbutton_t;

void MEFbutton_refresh( gpioMap_t tecla , MEFbutton_t *estado );
 

#endif /* MEFBUTTON_REFRESH_H */