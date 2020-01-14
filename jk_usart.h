/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Kinkennon Services

  @File Name
    jk_usart.h

 */
/* ************************************************************************** */

#ifndef _JK_USART_H    /* Guard against multiple inclusion */
#define _JK_USART_H

#include "app.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

#define MAX_NUM_OF_BYTES        64

#define MAX_NUM_OF_BYTES_IN_BUF (MAX_NUM_OF_BYTES + 4)

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum
{
    /* In this state, the application opens the driver. */
    JK_USART_STATE_INIT,

    /* Check if the driver is opened and ready  */
    JK_USART_STATE_CHECK_DRVR_STATE,

    JK_USART_STATE_RX,

    JK_USART_STATE_TX,
            
    JK_USART_STATE_WAIT_FOR_RX,

    JK_USART_STATE_WAIT_FOR_TX_COMPLETION,

    JK_USART_STATE_IDLE,

    JK_USART_STATE_ERROR,

    JK_USART_STATE_RELEASE_DRVR_RESOURCES

} JK_USART_STATES;


// *****************************************************************************
typedef struct
{
    /* Application current state of USART1 */
    JK_USART_STATES  state;

    /* UART1 Driver Handle  */
    DRV_HANDLE  drvUsartHandle;

	/* Write buffer handle */
    DRV_USART_BUFFER_HANDLE   drvUsartTxBufHandle;

    /* Read buffer handle */
    DRV_USART_BUFFER_HANDLE   drvUsartRxBufHandle;

    /* UART1 TX buffer  */
    uint8_t  drvUsartTxBuffer[MAX_NUM_OF_BYTES_IN_BUF];

    /* UART1 RX buffer  */
    uint8_t  drvUsartRxBuffer[MAX_NUM_OF_BYTES_IN_BUF];
    
} JK_USART_DATA;


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

void JK_USART_Initialize(void);

void JK_USART_Tasks1(void);
void JK_USART_Tasks2(void);
void JK_USART_Tasks3(void);
void JK_USART_Tasks4(void);
void JK_USART_Tasks5(void);

#endif /* _JK_USART_H */

/* *****************************************************************************
 End of File
 */
