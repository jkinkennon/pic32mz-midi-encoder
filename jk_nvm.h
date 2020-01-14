/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Kinkennon Services

  @File Name
    jk_nvm.h

 */
/* ************************************************************************** */

#ifndef _JK_NVM_H    /* Guard against multiple inclusion */
#define _JK_NVM_H

#include "app.h"

#define KEEP    __attribute__ ((keep)) __attribute__((address(DRV_NVM_MEDIA_START_ADDRESS)))
#define JK_NVM_MEMORY_AREA_SIZE (DRV_NVM_MEDIA_SIZE * 1024)
#define MAX_CHANNELS    16

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

/* Enumeration of the Read, write and erase region geometry indices */
typedef enum {

    /* Read region index */
    JK_NVM_READ_REGION_INDEX = 0,

    /* Write region index */
    JK_NVM_WRITE_REGION_INDEX,

    /* Erase region index */
    JK_NVM_ERASE_REGION_INDEX

} JK_NVM_REGION_INDEX;

/* Enumeration listing the Erase operation sub-states */
typedef enum {

    /* Initialize variables for Erase operation */
    JK_NVM_ERASE_STATE_INIT = 0,

    /* Issue Erase command */
    JK_NVM_ERASE_STATE_ERASE_CMD,

    /* Check if the erase command operation is complete */
    JK_NVM_ERASE_STATE_ERASE_CMD_STATUS,

    /* Issue Read command */
    JK_NVM_ERASE_STATE_READ_CMD,

    /* Check if the read command operation is complete */
    JK_NVM_ERASE_STATE_READ_CMD_STATUS,

    /* Verify that the data is indeed erased */
    JK_NVM_ERASE_STATE_VERIFY_DATA,

    /* Erase and verification of data is successful */
    JK_NVM_ERASE_STATE_IDLE,

    /* Erase or verification of data is unsuccessful */
    JK_NVM_ERASE_STATE_ERROR

} JK_NVM_ERASE_STATES;

/* Enumeration listing the EraseWrite operation sub-states */
typedef enum {

    /* Initialize the variables for the EraseWrite operation */
    JK_NVM_ERASEWRITE_INIT = 0,

    /* Issue EraseWrite Command */
    JK_NVM_ERASEWRITE_ERASEWRITE,

    /* Check if the EraseWrite command is complete */
    JK_NVM_ERASEWRITE_ERASEWRITE_STATUS,

    /* Issue command to read page one data */
    JK_NVM_ERASEWRITE_READ_PAGE_ONE,

    /* Check if the read command is complete */
    JK_NVM_ERASEWRITE_READ_PAGE_ONE_STATUS,

    /* Verify page one data */
    JK_NVM_ERASEWRITE_VERIFY_PAGE_ONE_DATA,

    /* Issue command to read page two data */
    JK_NVM_ERASEWRITE_READ_PAGE_TWO,

    /* Check if the read command is complete */
    JK_NVM_ERASEWRITE_READ_PAGE_TWO_STATUS,

    /* Verify page two data */
    JK_NVM_ERASEWRITE_VERIFY_PAGE_TWO_DATA,

    /* EraseWrite operation is successful */
    JK_NVM_ERASEWRITE_IDLE,

    /* EraseWrite operation is unsuccessful */
    JK_NVM_ERASEWRITE_ERROR

} JK_NVM_ERASEWRITE_STATES;

typedef enum
{
    /* Open the NVM driver, read the media layout and register for
    the NVM driver events */
    JK_NVM_STATE_INIT,

    /* Erase the memory */
    JK_NVM_STATE_ERASE_ALL,

    /* Perform sequential read write operations */
    JK_NVM_STATE_SEQ_RW,

    /* Perform the erasewrite operations */
    JK_NVM_STATE_ERASEWRITE_RW,

    /* Close the NVM driver */
    JK_NVM_STATE_CLOSE,

    /* App demonstration is successful */
    JK_NVM_STATE_IDLE,

    /* An app error has occurred. App demonstration is unsuccessful */
    JK_NVM_STATE_ERROR

} JK_NVM_STATES;

typedef struct
{
    /* NVM Driver Handle */
    DRV_HANDLE                  nvmHandle;

    /* NVM Command Handles */
    DRV_NVM_COMMAND_HANDLE      nvmCommandHandle[8];

    /* Erase operation's current state */
    JK_NVM_ERASE_STATES             eraseState;

    /* EraseWrite operation's current state */
    JK_NVM_ERASEWRITE_STATES        eraseWriteState;

    /* Application's current state */
    JK_NVM_STATES               state;

    /* Some of the states have to be repeated as the erase
    operation is done prior to each data read/write operation.
    nextState is used to track the next main state for such
    cases. */
    JK_NVM_STATES               nextState;

    /* Counter to track the number of successful command
    complete events */
    uint8_t                     eventCount;

    /* Counter to track the number of unsuccessful command
    complete events */
    uint8_t                     errorEventCount;

    /* Counter to track the number of reads to be done for the
    random read write operation */
    uint8_t                     randomRWCount;

    /* Read block address */
    uint32_t                    readBlockAddr;

    /* Number of blocks to be read */
    uint32_t                    numReadBlocks;

} JK_NVM_DATA;

typedef struct {
    // First page of NVM (16k))
    uint8_t factoryTranlateTable[MAX_CHANNELS][NUM_KEYS][8];    // 8k
    uint8_t userTranslateTable[MAX_CHANNELS][NUM_KEYS][8];      // 8k
    // Second page of NVM
    stop_t factorySamsTable[NUM_STOPS];
    stop_t userSamsTable[NUM_STOPS];
    bool useCrescendoLEDs;
    bool useTranspose;
} jk_nvm_config_t;

void JK_NVM_Initialize(void);
void JK_NVM_Tasks(void);
void JK_NVM_FillWriteBuffer(void);
    
#endif /* _JK_NVM_H */

/* *****************************************************************************
 End of File
 */
