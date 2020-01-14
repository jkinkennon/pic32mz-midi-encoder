/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

#ifndef _APP_H
#define _APP_H

/* Specify an extension for GCC based compilers */
#if defined(__GNUC__)
#define __EXTENSION __extension__
#else
#define __EXTENSION
#endif

#if !defined(__PACKED)
    #define __PACKED
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include "mididef.h"
#include "sysex.h"
#include "console.h"
#include "jk_nvm.h"
#include "jk_usart.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define SIZEOF_MSG_BUF      4

typedef union {
    uint8_t channel[NUM_CHANNELS][NUM_KEYS];    // key access
    uint8_t offset[NUM_SWITCHES];               // offset access
} keytable8_t;

typedef union {
    bool channel[NUM_CHANNELS][NUM_KEYS];       // key access
    bool offset[NUM_SWITCHES];                  // offset access
} keytableB_t;

typedef union 
{
    uint16_t Val;
    uint8_t v[2] __PACKED;
    struct __PACKED
    {
        uint8_t out;
        uint8_t in;    // High byte, sent first
    } byte;
    struct __PACKED
    {
        __EXTENSION uint8_t A0:1;
        __EXTENSION uint8_t A1:1;
        __EXTENSION uint8_t A2:1;
        __EXTENSION uint8_t A3:1;
        __EXTENSION uint8_t A4:1;
        __EXTENSION uint8_t A5:1;
        __EXTENSION uint8_t A6:1;
        __EXTENSION uint8_t A7:1;
        __EXTENSION uint8_t B0:1;
        __EXTENSION uint8_t B1:1;
        __EXTENSION uint8_t B2:1;
        __EXTENSION uint8_t B3:1;
        __EXTENSION uint8_t B4:1;
        __EXTENSION uint8_t B5:1;
        __EXTENSION uint8_t B6:1;
        __EXTENSION uint8_t B7:1;   // High bit, sent first
    } bits;
} uint16_spi_t;

typedef union 
{
    uint32_t Val;
    uint16_t v16[2] __PACKED;
    uint8_t v8[4] __PACKED;
    struct __PACKED
    {
        uint8_t data1;
        uint8_t data2;
        uint8_t data3;
        uint8_t data4;
    } byte;
} uint32_spi_t;

typedef union {                                             // current input from DINs
    uint32_t bank[NUM_SPI_BANKS][DINS_PER_BANK];            // 1 32-bit words for each DIN
    uint32_t channel[NUM_CHANNELS][DINS_PER_CHANNEL];       // spiIn or spiOut by channel
    uint32_t offset[(NUM_CHANNELS * DINS_PER_CHANNEL)];
} spi_t;

struct midiQueueMsg {
    uint8_t midiMessage[SIZEOF_MSG_BUF];
    uint8_t msgType;
    uint8_t msgChannel;
    uint8_t msgFinishedLen;
    struct midiQueueMsg *nextPtr;
};

typedef struct midiQueueMsg MIDIQUEUEMSG;
typedef MIDIQUEUEMSG *MIDIQUEUEMSGPTR;

typedef union 
{
    uint16_t Val;
    uint8_t v[2] __PACKED;
    struct __PACKED
    {
        uint8_t LB;
        uint8_t HB;
    } byte;
    struct __PACKED
    {
        __EXTENSION uint8_t b0:1;
        __EXTENSION uint8_t b1:1;
        __EXTENSION uint8_t b2:1;
        __EXTENSION uint8_t b3:1;
        __EXTENSION uint8_t b4:1;
        __EXTENSION uint8_t b5:1;
        __EXTENSION uint8_t b6:1;
        __EXTENSION uint8_t b7:1;
        __EXTENSION uint8_t b8:1;
        __EXTENSION uint8_t b9:1;
        __EXTENSION uint8_t b10:1;
        __EXTENSION uint8_t b11:1;
        __EXTENSION uint8_t b12:1;
        __EXTENSION uint8_t b13:1;
        __EXTENSION uint8_t b14:1;
        __EXTENSION uint8_t b15:1;
    } bits;
} UINT16_VAL, UINT16_BITS;


// *****************************************************************************
/* Application States

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_STATE_INIT=0,

    /* Application waits for device configuration*/
    APP_STATE_WAIT_FOR_CONFIGURATION,

    /* Wait for receive */
    APP_STATE_READ,
            
    /* Wait for the TX to get completed */
    APP_STATE_WRITE,
            
    /* Scan SPI inputs */
    APP_STATE_SCAN,
            
//    /* Configure encoder via USB UART A */
//    APP_STATE_CONFIGURE_ENCODER,

    /* Application Error state*/
    APP_STATE_ERROR

} APP_STATES;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* Device layer handle returned by device layer open function */
    USB_DEVICE_HANDLE usbDevHandle;

    /* Application's current state*/
    APP_STATES state;

   /* Track device configuration */
    bool deviceIsConfigured;

    /* Configuration value */
    uint8_t configurationValue;

    /* speed */
    USB_SPEED speed;

    /* ep data sent */
    bool epDataWritePending;

    /* ep data received */
    bool epDataReadPending;

    /* Transfer handle */
    USB_DEVICE_TRANSFER_HANDLE writeTransferHandle;
    
    /* Transfer handle */
    USB_DEVICE_TRANSFER_HANDLE readTransferHandle;

    /* The transmit endpoint address */
    USB_ENDPOINT_ADDRESS endpointTx;

    /* The receive endpoint address */
    USB_ENDPOINT_ADDRESS endpointRx;

    /* Tracks the alternate setting */
    uint8_t altSetting;
    
} APP_DATA;



// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the 
    application in its initial state and prepares it to run so that its 
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks ( void );

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/
void delayTimer1(unsigned short int);
void resetTimer2(void);
void resetTimer3(void);
void resetTimer4(void);
void decodeMidi(void);
void updateKeyTable(int channel);
void clearBuffers(void);
int isEmpty(MIDIQUEUEMSGPTR);
void dequeue(MIDIQUEUEMSGPTR *, MIDIQUEUEMSGPTR *);
void enqueue(MIDIQUEUEMSGPTR *, MIDIQUEUEMSGPTR *);
void setMatrixColumn(int);
void getTwelveBits(int);
void getSixteenBits(int);

#endif /* _APP_H */
/*******************************************************************************
 End of File
 */

