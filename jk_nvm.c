/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "jk_nvm.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */

/* NVM Media Layout:
   On MZ devices:
      read block size  = 1 byte
      write block size = DRV_NVM_ROW_SIZE  = 2048 bytes
      erase block size = DRV_NVM_PAGE_SIZE = 16384 bytes

  The following table illustrates the address and number of blocks for the read,
  write and erase regions for NVM media of size 32 KB on MZ devices:
  ------------------------------------------------------------------------------
  |               | Block Size  | Number of Blocks             | Address Range |
  ------------------------------------------------------------------------------
  | Read region   | 1 Byte      | 32KB/Read Block Size = 32768 | 0 - 32767     |
  ------------------------------------------------------------------------------ 
  | Write Region  | 2048 Bytes  | 32KB/Write Block Size = 16   | 0 - 15        |
  ------------------------------------------------------------------------------
  | Erase Region  | 16384 Bytes | 32KB/Erase Block Size = 2    | 0 - 1         |
  ------------------------------------------------------------------------------ 
*/

/* gAppFlashReserveArea refers to the 32KB of memory starting from address 
   DRV_NVM_MEDIA_START_ADDRESS. This memory area is reserved for NVM read/write
   operations. The attribute keep instructs the linker not to remove the section
   even if it is not refered anywhere in the code.
 */
const uint8_t gAppFlashReserveArea[JK_NVM_MEMORY_AREA_SIZE] KEEP = {0};

/* Buffer used for reading data from the NVM */
uint8_t gAppReadBuffer [DRV_NVM_PAGE_SIZE];

/* Buffer used for writing data onto the NVM */
uint8_t gAppWriteBuffer[DRV_NVM_PAGE_SIZE];

/* Pointer to the NVM Media Geometry */
SYS_FS_MEDIA_GEOMETRY *gAppNVMMediaGeometry;

JK_NVM_DATA jkNvmData;              // See JK_NVM_Initialize()

extern transTable_t translateTable;
//jk_nvm_config_t jkConfiguration;

typedef struct
{
	//SYS_MODULE_OBJ   drvObject;

} JK_NVM_DRV_OBJECTS;

JK_NVM_DRV_OBJECTS appDrvObject;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */



/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void JK_NVM_Initialize ( void )
{
    /* Initialize the state variables to default values. */
    jkNvmData.eraseState      = JK_NVM_ERASE_STATE_INIT;
    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_INIT;
    jkNvmData.state           = JK_NVM_STATE_INIT;

    jkNvmData.eventCount      = 0;
    jkNvmData.errorEventCount = 0;
    jkNvmData.randomRWCount   = 0;
    jkNvmData.readBlockAddr   = 0;
    jkNvmData.numReadBlocks   = 0;
    
    JK_NVM_FillWriteBuffer ();
}


/**********************************************************
 * This function is used to fill up the write buffer with the
 * data pattern.
 ***********************************************************/
void JK_NVM_FillWriteBuffer ()
{
    uint32_t i = 0;

    /* There are eight rows in a page. Populate each row
       with 1/4 of a translateTable.
    */
    for(i = 0; i < DRV_NVM_ROW_SIZE; i++)
    {
        gAppWriteBuffer[i] = translateTable.transOffset[0][i];
        gAppWriteBuffer[i + (1 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[1][i];
        gAppWriteBuffer[i + (2 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[2][i];
        gAppWriteBuffer[i + (3 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[3][i];

        gAppWriteBuffer[i + (4 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[0][i];
        gAppWriteBuffer[i + (5 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[1][i];
        gAppWriteBuffer[i + (6 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[2][i];
        gAppWriteBuffer[i + (7 * DRV_NVM_ROW_SIZE)] = translateTable.transOffset[3][i];
    }
}



/* This function verifies that the data in the first page is as 
 * expected.
 */
bool JK_NVM_VerifyPageOneData ()
{
    uint32_t row;
    uint32_t offset;

    for(row = 0; row < 8; row ++)
    {
        for (offset = 0; offset < DRV_NVM_ROW_SIZE; offset++)
        {
            if(gAppReadBuffer[offset + (row * DRV_NVM_ROW_SIZE)] != translateTable.transOffset[row % 4][offset])
                return false;
        }
    }

    return true;
}

/* This function verifies that the data in the second page is as 
 * expected.
 */
bool JK_NVM_VerifyPageTwoData ()
{
    uint32_t i = 0;

    for(i = 0; i < DRV_NVM_ROW_SIZE; i ++)
    {
//        /* 8th block should have gAppDataPattern[2] */
//        if(gAppReadBuffer[i] != gAppDataPattern[2])
//            return false;
//        /* 9th block should have gAppDataPattern[3] */
//        if(gAppReadBuffer[i + DRV_NVM_ROW_SIZE] != gAppDataPattern[3])
//            return false;
    }

    return true;
}

/**********************************************************
 * This function issues command to erase the entire memory 
 * area and then reads back the memory to ensure that the 
 * erase operation has set the bits to '1'
 ***********************************************************/
void JK_NVM_EraseMemoryAndVerify ( void )
{
    uint32_t i = 0;
    DRV_NVM_COMMAND_STATUS commandStatus;
    
    switch (jkNvmData.eraseState)
    {
        case JK_NVM_ERASE_STATE_INIT:
            {
                jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERASE_CMD;
                jkNvmData.readBlockAddr    = 0;
                jkNvmData.numReadBlocks    = DRV_NVM_PAGE_SIZE;

                /* Intentional fall through */
            }

        case JK_NVM_ERASE_STATE_ERASE_CMD:
            {
                /* Erase the entire 32 KB of memory area */
                DRV_NVM_Erase(jkNvmData.nvmHandle, 
                        &jkNvmData.nvmCommandHandle[0],
                        0, 
                        gAppNVMMediaGeometry->geometryTable[JK_NVM_ERASE_REGION_INDEX].numBlocks);

                if(jkNvmData.nvmCommandHandle[0] != DRV_NVM_COMMAND_HANDLE_INVALID)
                {
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERASE_CMD_STATUS;
                }
                else
                {
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERROR;
                }
                break;
            }

        case JK_NVM_ERASE_STATE_ERASE_CMD_STATUS:
            {
                /* Check if the Erase operation has been completed successfully. */
                commandStatus = DRV_NVM_CommandStatus(jkNvmData.nvmHandle, jkNvmData.nvmCommandHandle[0]);
                if(DRV_NVM_COMMAND_COMPLETED == commandStatus)
                {
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_READ_CMD;
                }
                else if (DRV_NVM_COMMAND_ERROR_UNKNOWN == commandStatus)
                {
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERROR;
                }
                break;
            }

        case JK_NVM_ERASE_STATE_READ_CMD:
            {
                DRV_NVM_Read(jkNvmData.nvmHandle, &jkNvmData.nvmCommandHandle[0], 
                        gAppReadBuffer, jkNvmData.readBlockAddr, jkNvmData.numReadBlocks);

                if (jkNvmData.nvmCommandHandle[0] == DRV_NVM_COMMAND_HANDLE_INVALID)
                {
                    /* Failed to read data from the NVM */
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERROR; 
                }
                else
                {
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_READ_CMD_STATUS; 
                }

                break;
            }

        case JK_NVM_ERASE_STATE_READ_CMD_STATUS:
            {
                commandStatus = DRV_NVM_CommandStatus(jkNvmData.nvmHandle, jkNvmData.nvmCommandHandle[0]);
                if(DRV_NVM_COMMAND_COMPLETED == commandStatus)
                {
                    /* Update the read block address */
                    jkNvmData.readBlockAddr += jkNvmData.numReadBlocks;

                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_VERIFY_DATA;
                }
                else if (DRV_NVM_COMMAND_ERROR_UNKNOWN == commandStatus)
                {
                    jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERROR;
                }

                break;
            }

        case JK_NVM_ERASE_STATE_VERIFY_DATA:
            {
                for (i = 0; i < jkNvmData.numReadBlocks; i++)
                {
                    if (gAppReadBuffer[i] != 0xFF)
                    {
                        jkNvmData.eraseState = JK_NVM_ERASE_STATE_ERROR;
                        break;
                    }
                }

                if (jkNvmData.eraseState == JK_NVM_ERASE_STATE_VERIFY_DATA)
                {
                    if (jkNvmData.readBlockAddr == gAppNVMMediaGeometry->geometryTable[JK_NVM_READ_REGION_INDEX].numBlocks)
                    {
                        /* Completed verifying the erased memory. */
                        jkNvmData.eraseState = JK_NVM_ERASE_STATE_IDLE;
                    }
                    else
                    {
                        jkNvmData.eraseState = JK_NVM_ERASE_STATE_READ_CMD;
                    }
                }
                break;
            }

        case JK_NVM_ERASE_STATE_IDLE:
            {
                jkNvmData.eraseState = JK_NVM_ERASE_STATE_INIT;
                break;
            }

        case JK_NVM_ERASE_STATE_ERROR:
        default:
            {
                break;
            }
    }
}

/**********************************************************
 * This function makes use of the EraseWrite feature. This
 * feature combines the erase step along with that of the
 * write operation. The function writes different data pattern
 * to pages 1 and 2 and verifies the EraseWrite functionality.
 ***********************************************************/
void JK_NVM_EraseWriteOperations ( void )
{
    DRV_NVM_COMMAND_STATUS commandStatus;

    switch (jkNvmData.eraseWriteState)
    {
        case JK_NVM_ERASEWRITE_INIT:
            {
                /* Clear the event counts */
                jkNvmData.eventCount = 0;
                jkNvmData.errorEventCount = 0;
                jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERASEWRITE;
                /* Intentional Fallthrough */
            }

        case JK_NVM_ERASEWRITE_ERASEWRITE:
            {
                /* Erase write combines the erase and write operation. Even if 
                   write is spilled over to the next page, the EraseWrite 
                   operation handles it. 
                   */
                DRV_NVM_EraseWrite(jkNvmData.nvmHandle, &jkNvmData.nvmCommandHandle[0],
                        gAppWriteBuffer, 0, 8);
                
                if (jkNvmData.nvmCommandHandle[0] == DRV_NVM_COMMAND_HANDLE_INVALID)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                    break;
                }

                jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERASEWRITE_STATUS;
                //jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_IDLE;
                break;
            }

        case JK_NVM_ERASEWRITE_ERASEWRITE_STATUS:
            {
                /* Wait for the EraseWrite complete events */
                if (jkNvmData.eventCount == 1)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_READ_PAGE_ONE;
                    //jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_IDLE;
                }

                if (jkNvmData.errorEventCount > 0)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }

                break;
            }

        case JK_NVM_ERASEWRITE_READ_PAGE_ONE:
            {
                /* Read page one data and verify the data */
                DRV_NVM_Read(jkNvmData.nvmHandle, &jkNvmData.nvmCommandHandle[0], 
                        gAppReadBuffer, 0, DRV_NVM_PAGE_SIZE);
                if (jkNvmData.nvmCommandHandle[0] == DRV_NVM_COMMAND_HANDLE_INVALID)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }
                else
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_READ_PAGE_ONE_STATUS;
                }
                break;
            }

        case JK_NVM_ERASEWRITE_READ_PAGE_ONE_STATUS:
            {
                commandStatus = DRV_NVM_CommandStatus(jkNvmData.nvmHandle, jkNvmData.nvmCommandHandle[0]);
                if(DRV_NVM_COMMAND_COMPLETED == commandStatus)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_VERIFY_PAGE_ONE_DATA;
                }
                else if (DRV_NVM_COMMAND_ERROR_UNKNOWN == commandStatus)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }
                break;
            }

        case JK_NVM_ERASEWRITE_VERIFY_PAGE_ONE_DATA:
            {
                /* Verify the page 1 data */
                if (JK_NVM_VerifyPageOneData())
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_READ_PAGE_TWO;
                }
                else
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }
                break;
            }

        case JK_NVM_ERASEWRITE_READ_PAGE_TWO:
            {
                /* Read the page 2 data */
                DRV_NVM_Read(jkNvmData.nvmHandle, &jkNvmData.nvmCommandHandle[0], 
                        gAppReadBuffer, DRV_NVM_PAGE_SIZE, DRV_NVM_PAGE_SIZE);
                if (jkNvmData.nvmCommandHandle[0] == DRV_NVM_COMMAND_HANDLE_INVALID)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }
                else
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_READ_PAGE_TWO_STATUS;
                }
                break;
            }

        case JK_NVM_ERASEWRITE_READ_PAGE_TWO_STATUS:
            {
                commandStatus = DRV_NVM_CommandStatus(jkNvmData.nvmHandle, jkNvmData.nvmCommandHandle[0]);
                if(DRV_NVM_COMMAND_COMPLETED == commandStatus)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_VERIFY_PAGE_TWO_DATA;
                }
                else if (DRV_NVM_COMMAND_ERROR_UNKNOWN == commandStatus)
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }
                break;
            }

        case JK_NVM_ERASEWRITE_VERIFY_PAGE_TWO_DATA:
            {
                /* Verify the page 2 data */
                if (JK_NVM_VerifyPageTwoData())
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_IDLE;
                }
                else
                {
                    jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_ERROR;
                }
                break;
            }

        case JK_NVM_ERASEWRITE_IDLE:
            {
                /* EraseWrite completed successfully */
                //jkNvmData.eraseWriteState = JK_NVM_ERASEWRITE_INIT;
                break;
            }

        case JK_NVM_ERASEWRITE_ERROR:
        default:
            {
                /* EraseWrite failed */
                break;
            }
    }
}

/********************************************************
 * NVM Driver Events handler
 ********************************************************/

void JK_NVM_EventHandler
(
    DRV_NVM_EVENT event,
    DRV_NVM_COMMAND_HANDLE commandHandle,
    uintptr_t context
)
{
    switch (event)
    {
        case DRV_NVM_EVENT_COMMAND_COMPLETE:
            {
                jkNvmData.eventCount ++;
                break;
            }
        case DRV_NVM_EVENT_COMMAND_ERROR:
            {
                jkNvmData.errorEventCount ++;
                break;
            }
        default:
            {
                break;
            }
    }
}

/**********************************************************
 * Application tasks routine. This function implements the
 * application state machine.
 ***********************************************************/
void JK_NVM_Tasks ( void )
{
    switch(jkNvmData.state)
    {
        case JK_NVM_STATE_INIT:
            jkNvmData.nvmHandle = DRV_NVM_Open(0, DRV_IO_INTENT_READWRITE);
            if(DRV_HANDLE_INVALID == jkNvmData.nvmHandle)
            {
                jkNvmData.state = JK_NVM_STATE_ERROR;
                break;
            }

            /* Register for NVM driver events */
            DRV_NVM_EventHandlerSet (jkNvmData.nvmHandle, JK_NVM_EventHandler, 1);

            /* Read the NVM Media Geometry. */
            gAppNVMMediaGeometry = DRV_NVM_GeometryGet(jkNvmData.nvmHandle);
            if(NULL == gAppNVMMediaGeometry)
            {
                jkNvmData.state = JK_NVM_STATE_ERROR;
                break;
            }

            /* After the erase operation is completed start the sequential 
               read write operation.
               */
            jkNvmData.nextState = JK_NVM_STATE_ERASEWRITE_RW;
            jkNvmData.state = JK_NVM_STATE_ERASE_ALL;
            break;

        case JK_NVM_STATE_ERASE_ALL:
            /* The erase operation sets all bits to 1.  Erase the memory area 
               and verify that it has indeed been erased by reading back the
               memory and checking that all bits are set to 1.
               */
            JK_NVM_EraseMemoryAndVerify ();
            if (jkNvmData.eraseState == JK_NVM_ERASE_STATE_IDLE)
            {
                /* Erase and verify operation is complete. Move on
                to the next state. */
                jkNvmData.state = jkNvmData.nextState;
            }
            else if (jkNvmData.eraseState == JK_NVM_ERASE_STATE_ERROR)
            {
                /* Erase and verify operation failed. */
                jkNvmData.state = JK_NVM_STATE_ERROR;
            }
            break;

        case JK_NVM_STATE_ERASEWRITE_RW:
            /* Perform EraseWrite operations */
            JK_NVM_EraseWriteOperations ();
            if (jkNvmData.eraseWriteState == JK_NVM_ERASEWRITE_IDLE)
            {
                /* Erase write Operation is complete. Move on
                to the next state. */
                jkNvmData.state = JK_NVM_STATE_CLOSE;
            }
            else if (jkNvmData.eraseWriteState == JK_NVM_ERASEWRITE_ERROR)
            {
                /* Erase Write Operation failed. */
                jkNvmData.state = JK_NVM_STATE_ERROR;
            }
            break;

        case JK_NVM_STATE_CLOSE:
        {
            /* Close the driver */
            DRV_NVM_Close (jkNvmData.nvmHandle);
            jkNvmData.state = JK_NVM_STATE_IDLE;

            /* Intentional fall through */
        }
        case JK_NVM_STATE_IDLE:
        {
            /* App demo completed successfully. */
            //LATGbits.LATG15 = LED_ON;
            break;
        }

        case JK_NVM_STATE_ERROR:
        default:
        {
            /* App demo failed. */
            //LATGbits.LATG14 = LED_ON;
            break;
        }
    }
}

/* *****************************************************************************
 End of File
 */
