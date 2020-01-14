/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "jk_usart.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

JK_USART_DATA usart1Data;
JK_USART_DATA usart2Data;
JK_USART_DATA usart3Data;
JK_USART_DATA usart4Data;
JK_USART_DATA usart5Data;

bool dataAtUsart1 = false;
bool dataAtUsart2 = false;
bool dataAtUsart3 = false;
bool dataAtUsart4 = false;
bool dataAtUsart5 = false;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

void JK_USART_BufferEventHandlerUsart1(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context );
void JK_USART_BufferEventHandlerUsart2(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context );
void JK_USART_BufferEventHandlerUsart3(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context );
void JK_USART_BufferEventHandlerUsart4(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context );
void JK_USART_BufferEventHandlerUsart5(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context );

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

void JK_USART_Initialize ( void )
{
    /* APP state task Init */
    usart1Data.state = JK_USART_STATE_INIT;
    usart2Data.state = JK_USART_STATE_INIT;
    usart3Data.state = JK_USART_STATE_INIT;
    usart4Data.state = JK_USART_STATE_INIT;
    usart5Data.state = JK_USART_STATE_INIT;
    
    usart1Data.drvUsartHandle = DRV_HANDLE_INVALID;
    usart2Data.drvUsartHandle = DRV_HANDLE_INVALID;
    usart3Data.drvUsartHandle = DRV_HANDLE_INVALID;
    usart4Data.drvUsartHandle = DRV_HANDLE_INVALID;
    usart5Data.drvUsartHandle = DRV_HANDLE_INVALID;
}

void JK_USART_Tasks1(void)
{
    
    switch(usart1Data.state)
    {
        /* Application's initial state. */
        case JK_USART_STATE_INIT:
        {
            if(usart1Data.drvUsartHandle == DRV_HANDLE_INVALID)
            {
                /* Open the USART Driver for USART1 Client  */
                usart1Data.drvUsartHandle = DRV_USART_Open( DRV_USART_INDEX_0,
                                                     DRV_IO_INTENT_READWRITE );
            }
            usart1Data.state = JK_USART_STATE_CHECK_DRVR_STATE;
            break;
        } 

        case JK_USART_STATE_CHECK_DRVR_STATE:
        {
            /* Check the USART1 driver handler */
            if (usart1Data.drvUsartHandle == DRV_HANDLE_INVALID )
            {
                /* Set the state to reinitialize */
                usart1Data.state = JK_USART_STATE_INIT;
                return;
            }
            DRV_USART_BufferEventHandlerSet(usart1Data.drvUsartHandle,
                            JK_USART_BufferEventHandlerUsart1, (uintptr_t)1);

            usart1Data.state = JK_USART_STATE_RX;
            break;
        } 
			
        case JK_USART_STATE_RX:
        {
            /* Add application Rx buffer to Usart1 Driver Rx buffer and wait */
            DRV_USART_BufferAddRead(usart1Data.drvUsartHandle,&(usart1Data.drvUsartRxBufHandle),
                        //(uint8_t*)&usart1Data.drvUsartRxBuffer[0],MAX_NUM_OF_BYTES);
                        (uint8_t*)&usart1Data.drvUsartRxBuffer[0],1);

            usart1Data.state = JK_USART_STATE_WAIT_FOR_RX;
            break;
        } 
        
        case JK_USART_STATE_TX:
        {
            /* Populate the application Tx buffer for Tx over USART1 */
            DRV_USART_BufferAddWrite(usart1Data.drvUsartHandle,&(usart1Data.drvUsartTxBufHandle),
                        (uint8_t*)&usart1Data.drvUsartTxBuffer[0],MAX_NUM_OF_BYTES);
            
            usart1Data.state = JK_USART_STATE_WAIT_FOR_TX_COMPLETION;
            break;
        } 
        
        case JK_USART_STATE_IDLE:
        {
            usart1Data.state = JK_USART_STATE_RX;
            break;
        } 
		
        case JK_USART_STATE_ERROR:
        {
            //Add the condition for Test failed
            break; 
        } 

        default:
            break;
    }
}

void JK_USART_Tasks2(void)
{
    switch(usart2Data.state)
    {
        /* Application's initial state. */
        case JK_USART_STATE_INIT:
        {
            if(usart2Data.drvUsartHandle == DRV_HANDLE_INVALID)
            {
                /* Open the USART Driver for USART1 Client  */
                usart2Data.drvUsartHandle = DRV_USART_Open( DRV_USART_INDEX_1,
                                                     DRV_IO_INTENT_READWRITE );
            }
            usart2Data.state = JK_USART_STATE_CHECK_DRVR_STATE;
        } break;

        case JK_USART_STATE_CHECK_DRVR_STATE:
        {
            /* Check the USART1 driver handler */
            if (usart2Data.drvUsartHandle == DRV_HANDLE_INVALID )
            {
                /* Set the state to reinitialize */
                usart2Data.state = JK_USART_STATE_INIT;
                return;
            }
            DRV_USART_BufferEventHandlerSet(usart2Data.drvUsartHandle,
                            JK_USART_BufferEventHandlerUsart2,(uintptr_t)2);

            usart2Data.state = JK_USART_STATE_RX;
        } break;
			
        case JK_USART_STATE_RX:
        {
            /* Add application Rx buffer to Usart2 Driver Rx buffer and wait*/
            DRV_USART_BufferAddRead(usart2Data.drvUsartHandle,&(usart2Data.drvUsartRxBufHandle),
                        (uint8_t*)&usart2Data.drvUsartRxBuffer[0],MAX_NUM_OF_BYTES);

            usart2Data.state = JK_USART_STATE_WAIT_FOR_RX;
        } break;
        
        case JK_USART_STATE_TX:
        {
            /* Populate the application Tx buffer for Tx over USART2 */
            DRV_USART_BufferAddWrite(usart2Data.drvUsartHandle,&(usart2Data.drvUsartTxBufHandle),
                        (uint8_t*)&usart2Data.drvUsartTxBuffer[0],34);
            
            usart2Data.state = JK_USART_STATE_WAIT_FOR_TX_COMPLETION;
        } break;
        
        case JK_USART_STATE_IDLE:
        {
            Nop();
        } break;
		
        case JK_USART_STATE_ERROR:
        {
            //Add the condition for Test failed
        } break;

        default:
            break;
    }
}

void JK_USART_Tasks3(void)
{
    switch(usart3Data.state)
    {
        /* Application's initial state. */
        case JK_USART_STATE_INIT:
        {
            if(usart3Data.drvUsartHandle == DRV_HANDLE_INVALID)
            {
                /* Open the USART Driver for USART1 Client  */
                usart3Data.drvUsartHandle = DRV_USART_Open( DRV_USART_INDEX_2,
                                                     DRV_IO_INTENT_READWRITE );
            }
            usart3Data.state = JK_USART_STATE_CHECK_DRVR_STATE;
        } break;

        case JK_USART_STATE_CHECK_DRVR_STATE:
        {
            /* Check the USART1 driver handler */
            if (usart3Data.drvUsartHandle == DRV_HANDLE_INVALID )
            {
                /* Set the state to reinitialize */
                usart3Data.state = JK_USART_STATE_INIT;
                return;
            }
            DRV_USART_BufferEventHandlerSet(usart3Data.drvUsartHandle,
                            JK_USART_BufferEventHandlerUsart3,(uintptr_t)3);

            usart3Data.state = JK_USART_STATE_RX;
        } break;
			
        case JK_USART_STATE_RX:
        {
            /* Add application Rx buffer to Usart3 Driver Rx buffer and wait */
            DRV_USART_BufferAddRead(usart3Data.drvUsartHandle,&(usart3Data.drvUsartRxBufHandle),
                        (uint8_t*)&usart3Data.drvUsartRxBuffer[0],MAX_NUM_OF_BYTES);

            usart3Data.state = JK_USART_STATE_WAIT_FOR_RX;
        } break;
        
        case JK_USART_STATE_TX:
        {
            /* Populate the application Tx buffer for Tx over USART3 */
            DRV_USART_BufferAddWrite(usart3Data.drvUsartHandle,&(usart3Data.drvUsartTxBufHandle),
                        (uint8_t*)&usart3Data.drvUsartTxBuffer[0],34);
            
            usart3Data.state = JK_USART_STATE_WAIT_FOR_TX_COMPLETION;
        } break;
        
        case JK_USART_STATE_IDLE:
        {
            Nop();
        } break;
		
        case JK_USART_STATE_ERROR:
        {
            //Add the condition for Test failed
        } break;

        default:
                break;
    }
}

void JK_USART_Tasks4(void)
{
    switch(usart4Data.state)
    {
        /* Application's initial state. */
        case JK_USART_STATE_INIT:
        {
            if(usart4Data.drvUsartHandle == DRV_HANDLE_INVALID)
            {
                /* Open the USART Driver for USART1 Client  */
                usart4Data.drvUsartHandle = DRV_USART_Open( DRV_USART_INDEX_3,
                                                     DRV_IO_INTENT_READWRITE );
            }
            usart4Data.state = JK_USART_STATE_CHECK_DRVR_STATE;
        } break;

        case JK_USART_STATE_CHECK_DRVR_STATE:
        {
            /* Check the USART1 driver handler */
            if (usart4Data.drvUsartHandle == DRV_HANDLE_INVALID )
            {
                /* Set the state to reinitialize */
                usart4Data.state = JK_USART_STATE_INIT;
                return;
            }
            DRV_USART_BufferEventHandlerSet(usart4Data.drvUsartHandle,
                            JK_USART_BufferEventHandlerUsart4,(uintptr_t)4);

            usart4Data.state = JK_USART_STATE_RX;
        } break;
			
        case JK_USART_STATE_RX:
        {
            /* Add application Rx buffer to Usart4 Driver Rx buffer and wait */
            DRV_USART_BufferAddRead(usart4Data.drvUsartHandle,&(usart4Data.drvUsartRxBufHandle),
                        (uint8_t*)&usart4Data.drvUsartRxBuffer[0],MAX_NUM_OF_BYTES);

            usart4Data.state = JK_USART_STATE_WAIT_FOR_RX;
        } break;
        
        case JK_USART_STATE_TX:
        {
            /* Populate the application Tx buffer for Tx over USART4 */
            DRV_USART_BufferAddWrite(usart4Data.drvUsartHandle,&(usart4Data.drvUsartTxBufHandle),
                        (uint8_t*)&usart4Data.drvUsartTxBuffer[0],34);
            
            usart4Data.state = JK_USART_STATE_WAIT_FOR_TX_COMPLETION;
        } break;
        
        case JK_USART_STATE_IDLE:
        {
            Nop();
        } break;
		
        case JK_USART_STATE_ERROR:
        {
            //Add the condition for Test failed
        } break;

        default:
                break;
    }
}

void JK_USART_Tasks5(void)
{
    switch(usart5Data.state)
    {
        /* Application's initial state. */
        case JK_USART_STATE_INIT:
        {
            if(usart5Data.drvUsartHandle == DRV_HANDLE_INVALID)
            {
                /* Open the USART Driver for USART1 Client  */
                usart5Data.drvUsartHandle = DRV_USART_Open( DRV_USART_INDEX_4,
                                                     DRV_IO_INTENT_READWRITE );
            }
            usart5Data.state = JK_USART_STATE_CHECK_DRVR_STATE;
        } break;

        case JK_USART_STATE_CHECK_DRVR_STATE:
        {
            /* Check the USART1 driver handler */
            if (usart5Data.drvUsartHandle == DRV_HANDLE_INVALID )
            {
                /* Set the state to reinitialize */
                usart5Data.state = JK_USART_STATE_INIT;
                return;
            }
            DRV_USART_BufferEventHandlerSet(usart5Data.drvUsartHandle,
                            JK_USART_BufferEventHandlerUsart5,(uintptr_t)5);

            usart5Data.state = JK_USART_STATE_RX;
        } break;
			
        case JK_USART_STATE_RX:
        {
            /* Add application Rx buffer to Usart5 Driver Rx buffer and wait */
            DRV_USART_BufferAddRead(usart5Data.drvUsartHandle,&(usart5Data.drvUsartRxBufHandle),
                        (uint8_t*)&usart5Data.drvUsartRxBuffer[0],MAX_NUM_OF_BYTES);

            usart5Data.state = JK_USART_STATE_WAIT_FOR_RX;
        } break;
        
        case JK_USART_STATE_TX:
        {
            /* Populate the application Tx buffer for Tx over USART5 */
            DRV_USART_BufferAddWrite(usart5Data.drvUsartHandle,&(usart5Data.drvUsartTxBufHandle),
                        (uint8_t*)&usart5Data.drvUsartTxBuffer[0],34);
            
            usart5Data.state = JK_USART_STATE_WAIT_FOR_TX_COMPLETION;
        } break;
        
        case JK_USART_STATE_IDLE:
        {
            Nop();
        } break;
		
        case JK_USART_STATE_ERROR:
        {
            //Add the condition for Test failed
        } break;

        default:
                break;
    }
}

void JK_USART_BufferEventHandlerUsart1(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context)
{
    switch(buffEvent)
    {	
        /* Buffer event is completed successfully */
        case DRV_USART_BUFFER_EVENT_COMPLETE:
        {
            if(usart1Data.state == JK_USART_STATE_WAIT_FOR_TX_COMPLETION)
                usart1Data.state = JK_USART_STATE_RX;
            else if(usart1Data.state == JK_USART_STATE_WAIT_FOR_RX) {
                if(context == 1)
                    dataAtUsart1 = true;
                usart1Data.state = JK_USART_STATE_IDLE;
            }
        }
        break;

        /* Buffer event has some error */
        case DRV_USART_BUFFER_EVENT_ERROR:
        {
            break;
        }

       default:
            break;
    }
}

void JK_USART_BufferEventHandlerUsart2(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context)
{
    switch(buffEvent)
    {	
        /* Buffer event is completed successfully */
        case DRV_USART_BUFFER_EVENT_COMPLETE:
        {
            break;
        }
        /* Buffer event has some error */
        case DRV_USART_BUFFER_EVENT_ERROR:
        {
            break;
        }

       default:
            break;
    }
}

void JK_USART_BufferEventHandlerUsart3(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context)
{
    switch(buffEvent)
    {	
        /* Buffer event is completed successfully */
        case DRV_USART_BUFFER_EVENT_COMPLETE:
        {
            break;
        }
        /* Buffer event has some error */
        case DRV_USART_BUFFER_EVENT_ERROR:
        {
            break;
        }

       default:
            break;
    }
}

void JK_USART_BufferEventHandlerUsart4(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context)
{
    switch(buffEvent)
    {	
        /* Buffer event is completed successfully */
        case DRV_USART_BUFFER_EVENT_COMPLETE:
        {
            break;
        }
        /* Buffer event has some error */
        case DRV_USART_BUFFER_EVENT_ERROR:
        {
            break;
        }

       default:
            break;
    }
}

void JK_USART_BufferEventHandlerUsart5(DRV_USART_BUFFER_EVENT buffEvent,
                            DRV_USART_BUFFER_HANDLE hBufferEvent,
                            uintptr_t context)
{
    switch(buffEvent)
    {	
        /* Buffer event is completed successfully */
        case DRV_USART_BUFFER_EVENT_COMPLETE:
        {
            break;
        }
        /* Buffer event has some error */
        case DRV_USART_BUFFER_EVENT_ERROR:
        {
            break;
        }

       default:
            break;
    }
}

/* *****************************************************************************
 End of File
 */
