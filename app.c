/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include <proc/p32mz2048efh144.h>

#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */

APP_DATA appData;

/* Read Data Buffer */
uint8_t receivedDataBuffer[512] __attribute__((coherent, aligned(4)));

/* Transmit data buffer */
uint8_t transmitDataBuffer[512] __attribute__((coherent, aligned(4)));

/* The endpoint size is 64 for FS and 512 for HS */
uint16_t endpointSize = 512;

/*
 * These types are defined in sysex.h and store System Exclusive MIDI messages.
 * The framework exists in parseMidiMsg() to put these messages to further use.
 */
HW_Sysex_LCD_Msg_t LCD_Msg;
HW_Sysex_Status_String_t HW_Status_String;
HW_Sysex_Status_Float_t HW_Status_Float;
HW_Sysex_Status_Byte_t HW_Status_Byte;

/*
 * keyTable[key#] is a byte representing recent scans on that key (or switch).
 * The top bit == 1 (0b10000000) indicates that the key is considered to be down
 * (closed or ON). The lower three bits indicate status for recent scans.
 * A key that is ON (0b10000111) might transition to OFF as follows:
 * (0b10000110) on this scan key was no longer closed (key up)
 * (0b10000100) 1 mSec later it is again open
 * (0b00000000) 2 mSec and the third scan shows open as well (no key bounce) and
 *              the program considers the key open (up).  Opposite on key down.
 */
keytable8_t keyTable;

/*
 * The inverted input value at each of (64 x N channels) input points. The
 * normal key up voltage is 3.3v giving a value of 1 at the port. This value is
 * inverted as it's easier to think of ON == 1 and OFF == 0.
 * Press key #23 and keyBit[23] = 1.
 */
keytableB_t keyBit;

spi_t spiIn; // declare a union of type spiIn_t
spi_t spiOut; // declare a union of type spiOut_t


/*
 * The translation table handles up to 512 keys or switches.
 * There are normally 8 channels, 64 keys, noteOn (4 bytes) + noteOff (4 bytes)
 */
extern transTable_t translateTable;

MIDIQUEUEMSGPTR headRxPtr = NULL, tailRxPtr = NULL;
MIDIQUEUEMSGPTR headTxPtr = NULL, tailTxPtr = NULL;
MIDIQUEUEMSG midiMsg;

extern stop_t samsTable[NUM_STOPS];
extern channel_t channelTable[NUM_CHANNELS];

uint8_t sysexMsg[64]; // single sysex msg
int sx = 0; // index into midiSysexMsg
int rx = 0; // index into midiRxMsg
int tx = 0; // index into midiTxMsg

// Not all consoles put pedal on sp1. Typically pedal is first and we move 
// couplers to sp4 along with any choir stops.
bool sp1_SAMsPending = false; // a stop has changed in the first division
bool sp2_SAMsPending = false; // a stop has changed in the second division
bool sp3_SAMsPending = false; // a stop has changed in the third division
bool sp4_SAMsPending = false; // a stop has changed in the fourth division
bool setSAMsTime = false;
bool SAMsPowerOn = false; // flag set when any SAMs power is active

int keyScanCount = 0; // count reset every 16 kybd scans
bool keyScanTime = false;

uint32_t stalePot[NUM_POTS] = {0, 0, 0, 0,
    0, 0, 0, 0}; // previous potentiometer values
uint32_t freshPot[NUM_POTS] = {0, 0, 0, 0,
    0, 0, 0, 0}; // current potentiometer values

int transposeHW;
int transposeSW;

extern JK_USART_DATA usart1Data;
extern JK_USART_DATA usart2Data;
extern JK_USART_DATA usart3Data;
extern JK_USART_DATA usart4Data;
extern JK_USART_DATA usart5Data;

extern bool dataAtUsart1;
extern bool dataAtUsart2;
extern bool dataAtUsart3;
extern bool dataAtUsart4;
extern bool dataAtUsart5;

/* Timer Driver Handles */
DRV_HANDLE drvTIMER_1_Handle;
DRV_HANDLE drvTIMER_2_Handle;
DRV_HANDLE drvTIMER_3_Handle;
DRV_HANDLE drvTIMER_4_Handle;

/*************************************************
 * Application Device Layer Event Handler
 *************************************************/

void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * eventData, uintptr_t context) {
    USB_SETUP_PACKET * setupPacket;

    switch (event) {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:
            
            /* Device was either de-configured or reset */
            appData.deviceIsConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* pData will point to the configuration. Check the configuration */
            appData.configurationValue = ((USB_DEVICE_EVENT_DATA_CONFIGURED *) eventData)->configurationValue;
            if (appData.configurationValue == 1) {
                appData.deviceIsConfigured = true;
            }
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS has been detected. We can attach the device */
            USB_DEVICE_Attach(appData.usbDevHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is removed. We can detach the device */
            USB_DEVICE_Detach(appData.usbDevHandle);
            break;

        case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
            /* This means we have received a setup packet */
            setupPacket = (USB_SETUP_PACKET *) eventData;
            if (setupPacket->bRequest == USB_REQUEST_SET_INTERFACE) {
                /* If we have got the SET_INTERFACE request, we just acknowledge
                 for now. This demo has only one alternate setting which is already
                 active. */
                USB_DEVICE_ControlStatus(appData.usbDevHandle, USB_DEVICE_CONTROL_STATUS_OK);
            } else if (setupPacket->bRequest == USB_REQUEST_GET_INTERFACE) {
                /* We have only one alternate setting and this setting 0. So
                 * we send this information to the host. */

                USB_DEVICE_ControlSend(appData.usbDevHandle, &appData.altSetting, 1);
            } else {
                /* We have received a request that we cannot handle. Stall it*/
                USB_DEVICE_ControlStatus(appData.usbDevHandle, USB_DEVICE_CONTROL_STATUS_ERROR);

            }
            break;

        case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
            /* Endpoint read is complete */
            appData.epDataReadPending = false;
            break;

        case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
            /* Endpoint write is complete */
            appData.epDataWritePending = false;
            break;

            /* These events are not used in this demo */
        case USB_DEVICE_EVENT_SYNCH_FRAME:
        case USB_DEVICE_EVENT_SET_DESCRIPTOR:
        case USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_SENT:
        case USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
        case USB_DEVICE_EVENT_CONTROL_TRANSFER_ABORTED:
        case USB_DEVICE_EVENT_SOF:
        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void decodeMidi(void) {
    rx = 0; // receive buffer index
    int i, j, k;
    int stop = 0;
    int piston = 0;
    int offset = 0;
    bool stopStateOld;
    
    // Handle received MIDI packets
    while (!isEmpty(headRxPtr)) {
        dequeue(&headRxPtr, &tailRxPtr);
        switch (midiMsg.msgType) {
            case M_NOTE_ON:
            case M_NOTE_OFF:
                // Implement handling stop decoding, etc.
                piston = midiMsg.midiMessage[2] - 0x24;
#ifdef STOPS_PER_CHANNEL_32                
                stop = piston / 2;
#else
                stop = offset = piston;
#endif     
                switch (midiMsg.msgChannel) {
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                        i = 0;
                        if (stop > NUM_STOPS) break; // sanity check                         
                        if (stop > 31) {
                            i = 1;
                            offset -= 32;
                        }        
                        // Retrieve old stop state
                        stopStateOld = samsTable[stop].state;
                        // Set new stop state
                        if (midiMsg.msgType == M_NOTE_OFF) {
                            samsTable[stop].state = false;
                            spiOut.channel[midiMsg.msgChannel][i] |= 1 << offset; // set bit
                        }
                        if (midiMsg.msgType == M_NOTE_ON) {
                            samsTable[stop].state = true;
                            spiOut.channel[midiMsg.msgChannel][i] &= ~(1 << offset); // clear bit
                        }
                        if (stopStateOld != samsTable[stop].state) {
                            switch (samsTable[stop].division) {
                                case DIV_UNUSED:
                                    break;
                                case DIV_PEDAL:
                                    sp1_SAMsPending = true;
                                    break;
                                case DIV_SWELL:
                                    sp2_SAMsPending = true;
                                    break;
                                case DIV_GREAT:
                                    sp3_SAMsPending = true;
                                    break;
                                case DIV_COUPLER:
                                    sp4_SAMsPending = true;
                                    break;
                                default:
                                    break;
                            }
                            resetTimer3(); // Timer 3 will make setSAMsTime = true
                        }
                        break;
                    case 6:
                    case 7:
                        i = 0;
                        if (piston > 31) {
                            i = 1;
                            piston -= 32;
                        }
                        if (midiMsg.msgType == M_NOTE_OFF)
                            spiOut.channel[midiMsg.msgChannel][i] &= ~(1 << piston); // clear bit
                        if (midiMsg.msgType == M_NOTE_ON)
                            spiOut.channel[midiMsg.msgChannel][i] |= 1 << piston; // set bit
                        break;
                    default:
                        break;
                }
                
            case M_SYSTEM_EX:
                sx = midiMsg.msgFinishedLen;
                switch (sx) {
                    case 39: // handle Hauptwerk LCD message
                        for (i = 0; i < 39; i++)
                            LCD_Msg.v[i] = sysexMsg[i];
                        switch (LCD_Msg.s2.panel_ID_LSB) {
                            case 0:
                                usart1Data.drvUsartTxBuffer[0] = '\r';
                                usart1Data.drvUsartTxBuffer[1] = '\n';
                                for (i = 0; i < 32; i++) {
                                    usart1Data.drvUsartTxBuffer[i+2] = LCD_Msg.s2.ascii_bytes[i];
                                }
                                usart1Data.state = JK_USART_STATE_TX;
                                break;
                            case 1:
                                //LATGbits.LATG12 = LED_ON;
                                usart2Data.drvUsartTxBuffer[0] = 0x7c;
                                usart2Data.drvUsartTxBuffer[1] = 0x2d;
                                for (i = 0; i < 32; i++) {
                                    usart2Data.drvUsartTxBuffer[i+2] = LCD_Msg.s2.ascii_bytes[i];
                                }
                                usart2Data.state = JK_USART_STATE_TX;
                                break; 
                            case 2:
                                usart3Data.drvUsartTxBuffer[0] = 0x7c;
                                usart3Data.drvUsartTxBuffer[1] = 0x2d;
                                for (i = 0; i < 32; i++) {
                                    usart3Data.drvUsartTxBuffer[i+2] = LCD_Msg.s2.ascii_bytes[i];
                                }
                                usart3Data.state = JK_USART_STATE_TX;
                                break;
                            case 3:
                                usart4Data.drvUsartTxBuffer[0] = 0x7c;
                                usart4Data.drvUsartTxBuffer[1] = 0x2d;
                                for (i = 0; i < 32; i++) {
                                    usart4Data.drvUsartTxBuffer[i+2] = LCD_Msg.s2.ascii_bytes[i];
                                }
                                usart4Data.state = JK_USART_STATE_TX;
                                break;
                            case 4:
                                usart5Data.drvUsartTxBuffer[0] = 0x7c;
                                usart5Data.drvUsartTxBuffer[1] = 0x2d;
                                for (i = 0; i < 32; i++) {
                                    usart5Data.drvUsartTxBuffer[i+2] = LCD_Msg.s2.ascii_bytes[i];
                                }
                                usart5Data.state = JK_USART_STATE_TX;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 22: // change to translation for one key
                        if (sysexMsg[2] != 0x70) break; // msg type/length mismatch
                        j = sysexMsg[3]; // kybd
                        k = sysexMsg[4]; // key
                        if (k < NUM_KEYS) {
                            translateTable.transTable[j][k][0] = (sysexMsg[5] << 4) | sysexMsg[6];
                            translateTable.transTable[j][k][1] = (sysexMsg[7] << 4) | sysexMsg[8];
                            translateTable.transTable[j][k][2] = (sysexMsg[9] << 4) | sysexMsg[10];
                            translateTable.transTable[j][k][3] = (sysexMsg[11] << 4) | sysexMsg[12];
                            translateTable.transTable[j][k][4] = (sysexMsg[13] << 4) | sysexMsg[14];
                            translateTable.transTable[j][k][5] = (sysexMsg[15] << 4) | sysexMsg[16];
                            translateTable.transTable[j][k][6] = (sysexMsg[17] << 4) | sysexMsg[18];
                            translateTable.transTable[j][k][7] = (sysexMsg[19] << 4) | sysexMsg[20];
                        }
                        break;
                    case 21: // handle Hauptwerk Status, string variable
                        for (i = 0; i < 21; i++)
                            HW_Status_String.v[i] = sysexMsg[i]; // copy msg to structure
                        if (HW_Status_String.s1.message_type != MSG_TYPE_STATUS_STRING) break;
                        // TODO: handle message
                        break;
                    case 9: // handle Hauptwerk Status, float variable
        //                for (i = 0; i < 9; i++)
        //                    HW_Status_Float.v[i] = sysexMsg[i]; // copy msg to structure
        //                if (HW_Status_Float.s1.message_type != MSG_TYPE_STATUS_FLOAT) break;
        //                switch (HW_Status_Float.s1.variable_ID) {
        //                        uint8_t temp;
        //                    case TRANSPOSER_SEMITONES:
        //                        temp = HW_Status_Float.s1.float_4;
        //                        if (temp & 0x40)
        //                            temp |= 0x80; // if 0bx1xxxxxx, 0b11xxxxxx
        //                        transposeHW = (int) temp;
        //
        //                        if (transposeSW > transposeHW) {
        //                            midiTxMsg[0] = translateTable[7][0x3d][0];
        //                            midiTxMsg[1] = translateTable[7][0x3d][1];
        //                            midiTxMsg[2] = translateTable[7][0x3d][2];
        //                            sendMidiMsg();
        //                        } else if (transposeHW > transposeSW) {
        //                            midiTxMsg[0] = translateTable[7][0x3e][0];
        //                            midiTxMsg[1] = translateTable[7][0x3e][1];
        //                            midiTxMsg[2] = translateTable[7][0x3e][2];
        //                            sendMidiMsg();
        //                        }
        //                        if (transposeSW == 0) {
        //                            LATCbits.LATC1 = LED_OFF; // RC1 - First LED - Red (transpose)
        //                        } else {
        //                            LATCbits.LATC1 = LED_ON;
        //                        }
        //                        break;
        //                }
                        break;
                    case 6: // handle Hauptwerk Status, boolean variables
                        for (i = 0; i < 6; i++) {
                            HW_Status_Byte.v[i] = sysexMsg[i]; // copy msg to structure
                        }
                        if (HW_Status_Byte.s.message_type != MSG_TYPE_STATUS_BOOLEAN) {
                            break;
                        }
                        switch (HW_Status_Byte.s.variable_ID) {
        #ifndef USE_CRESC_LEDS
                            case IS_SETTER_MODE_ON:
                                if (HW_Status_Byte.s.byte_value)
                                    LATDbits.LATD6 = LED_ON; // Setter Mode On
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATDbits.LATD6 = LED_OFF; // turn off Indicator
                                break;
                            case IS_SCOPE_MODE_ON:
                                if (HW_Status_Byte.s.byte_value)
                                    LATDbits.LATD13 = LED_ON; // Scope Mode On
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATDbits.LATD13 = LED_OFF; // turn off Indicator
                                break;
                            case IS_RECORDING_AUDIO:
                                if (HW_Status_Byte.s.byte_value)
                                    LATDbits.LATD14 = LED_ON; // Audio Recording
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATDbits.LATD14 = LED_OFF; // turn off Recording Indicator
                                break;
                            case IS_RECORDING_MIDI:
                                if (HW_Status_Byte.s.byte_value)
                                    LATGbits.LATG9 = LED_ON; // MIDI Recording
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATGbits.LATG9 = LED_OFF; // turn off Recording Indicator
                                break;
                            case IS_PLAYING_MIDI:
                                if (HW_Status_Byte.s.byte_value)
                                    LATGbits.LATG12 = LED_ON; // MIDI Playing
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATGbits.LATG12 = LED_OFF; // turn off MIDI Indicator
                                break;
                            case IS_IN_ERROR_STATE:
                                if (HW_Status_Byte.s.byte_value)
                                    LATGbits.LATG13 = LED_ON; // Error
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATGbits.LATG13 = LED_OFF; // turn off Error Indicator
                                break;
                            case IS_ORGAN_LOADING:
                                if (HW_Status_Byte.s.byte_value)
                                    LATGbits.LATG14 = LED_ON; // Organ Loading
                                else if (HW_Status_Byte.s.byte_value == 0)
                                    LATGbits.LATG14 = LED_OFF; // turn off Loading Indicator
                                break;
        #endif
                            case IS_ORGAN_READY:
                                if (HW_Status_Byte.s.byte_value) {
                                    LATGbits.LATG15 = LED_ON; // Audio Active
                                    for (j = 0; j < 8; j++) {
                                        stalePot[j] = 0;
                                    }
                                } else if (HW_Status_Byte.s.byte_value == 0) {
                                    //T2CONbits.ON = 0;   // stop scanning
                                    LATGbits.LATG15 = LED_OFF; // turn off Audio Active
                                }
                                break;
        #ifdef USE_CRESC_LEDS
                            case MASTER_CRESC_PED:
                                LATDbits.LATD6 = LED_OFF;
                                LATDbits.LATD13 = LED_OFF;
                                LATDbits.LATD14 = LED_OFF;
                                LATGbits.LATG9 = LED_OFF;
                                if (HW_Status_Byte.s.byte_value > 0) {
                                    LATDbits.LATD6 = LED_ON;
                                }
                                if (HW_Status_Byte.s.byte_value > 9) {
                                    LATDbits.LATD13 = LED_ON;
                                }
                                if (HW_Status_Byte.s.byte_value > 18) {
                                    LATDbits.LATD14 = LED_ON;
                                }
                                if (HW_Status_Byte.s.byte_value > 27) {
                                    LATGbits.LATG9 = LED_ON;
                                }
                                break;
        #endif
                            default:
                                break;
                        }   // End of switch on status byte
                    default:
                        break;
                }   // End of switch on SysEx length
                sx = 0;                
            default:
                break;
        }   // End of switch on message type
    }
}

void setMatrixColumn(int column) {
    uint32_t result = 0x00000001;
    result <<= column;
    spiOut.channel[0][0] = ~result;
}

void getTwelveBits(int column) {
    int bitNum;
    int key = 0;
    int matrix = 0;
    if (column > 10) {
        matrix = 2;
    }
    uint32_t bitsRead = ~spiIn.channel[0][0];
    uint32_t mask = 0;
    
    switch (column) {
        case 0:
            keyBit.channel[0][0] = (bool)(bitsRead & 0x00000020);
            keyBit.channel[1][0] = (bool)(bitsRead & 0x00002000);
            break;
        case 11:
            keyBit.channel[2][0] = (bool)(bitsRead & 0x00000020);
            keyBit.channel[3][0] = (bool)(bitsRead & 0x00002000);
            break;
        default:
            key = ((column % 11) * 6) - 5;
            for (bitNum = 0; bitNum < 6; bitNum++) {
                mask = 0x00000001 << bitNum;
                keyBit.channel[matrix][key + bitNum] = (bool)(bitsRead & mask);
            }
            matrix++;
            for (bitNum = 0; bitNum < 6; bitNum++) {
                mask = 0x00000100 << bitNum;
                keyBit.channel[matrix][key + bitNum] = (bool)(bitsRead & mask);
            }
            break;
    }
}

void getSixteenBits(int column) {
    int bitNum;
    int key = (column % 16) * 8;
    int matrix = 0;
    if (column > 7) {
        matrix = 2;
    }
    uint32_t bitsRead = ~spiIn.channel[0][0];
    
    uint32_t mask = 0x00000001 << column;
    for (bitNum = 0; bitNum < 8; bitNum++) {
        keyBit.channel[matrix][key + bitNum] = (bool)(bitsRead & mask);
        mask <<= 1;
    }
    matrix += 1;
    mask = 0x00010000 << column;
    for (bitNum = 0; bitNum < 8; bitNum++) {
        keyBit.channel[matrix][key + bitNum] = (bool)(bitsRead & mask);
        mask <<= 1;
    }
}

void updateKeyTable(int channel) {
    uint8_t key;
    uint8_t prevData;

    for (key = 0; key < 64; key++) { // process one set of 64 keys
        prevData = keyTable.channel[channel][key];
        bool newKey = keyBit.channel[channel][key];
        bool keyDown = (prevData & 0x80);
        prevData <<= 1; // shift the data left one bit
        if (newKey) prevData++; // set newest bit
        prevData &= 0x07; // strip any high bits

        if (!keyDown) { // if note is off in keyTable see if it should be on
            //if (prevData == 0b00000111) { // if 3 consecutive keyDown
            if (prevData > 0) {  // this is a fast noteOn for testing
                keyDown = true; // turn key on in keyTable

                midiMsg.midiMessage[0] = translateTable.transTable[channel][key][0];
                midiMsg.midiMessage[1] = translateTable.transTable[channel][key][1];
                midiMsg.midiMessage[2] = translateTable.transTable[channel][key][2];
                midiMsg.midiMessage[3] = translateTable.transTable[channel][key][3];
                midiMsg.msgType = M_NOTE_ON;
                midiMsg.msgChannel = channel;
                midiMsg.msgFinishedLen = 3;
                enqueue(&headTxPtr, &tailTxPtr);
                /* If stops then store state and adjust SPI output */
                // need to convert channel and key to stop #
                if (channelTable[channel].channel_use == CH_STOPS) {
                    uint8_t word = 0;       // 0 is the least significant word
                    int stop = 0;
                    switch (midiMsg.msgChannel) {
                        case 8:
                            word = 0;
                            stop = midiMsg.midiMessage[2] - 0x24;
                            break;
//                        case 9:
//                            word = 1;
//                            stop = midiMsg.midiMessage[2];
//                            break;
//                        case 10:
//                            word = 0;
//                            stop = midiMsg.midiMessage[2] + 0x1C;
//                            break;
//                        case 11:
//                            word = 1;
//                            stop = midiMsg.midiMessage[2] + 0x3C;
//                            break;
                        default:
                            break;
                    }
                    uint8_t stopOffset = samsTable[stop].port;   // 0 to 63
                    uint8_t bitOffset = stopOffset;
                    if (stopOffset > 31) {
                        word = 1;
                        bitOffset -= 32;
                    }
                    samsTable[key].state = true;
                    spiOut.channel[channel][word] &= (~(1 << bitOffset)); // clear bit
                    //spiOut.channel[channel][word] |= 1 << bitOffset; // set bit
                }
            }
        } else if (keyDown) { // if note is on in keyTable...
            if (prevData == 0b00000000) { // if 3 consecutive keyUp
                keyDown = false; // turn key off in keyTable

                midiMsg.midiMessage[0] = translateTable.transTable[channel][key][4];
                midiMsg.midiMessage[1] = translateTable.transTable[channel][key][5];
                midiMsg.midiMessage[2] = translateTable.transTable[channel][key][6];
                midiMsg.midiMessage[3] = translateTable.transTable[channel][key][7];
                midiMsg.msgType = M_NOTE_OFF;
                midiMsg.msgChannel = channel;
                midiMsg.msgFinishedLen = 3;
                enqueue(&headTxPtr, &tailTxPtr);
                /* If stops then store state and adjust SPI output */
                if (channelTable[channel].channel_use == CH_STOPS) {
                    uint8_t word = 0;       // 0 is the least significant word
                    int stop = 0;
                    switch (midiMsg.msgChannel) {
                        case 8:
                            word = 0;
                            stop = midiMsg.midiMessage[2] - 0x24;
                            break;
//                        case 9:
//                            word = 1;
//                            stop = midiMsg.midiMessage[2];
//                            break;
//                        case 10:
//                            word = 0;
//                            stop = midiMsg.midiMessage[2] + 0x1C;
//                            break;
//                        case 11:
//                            word = 1;
//                            stop = midiMsg.midiMessage[2] + 0x3C;
//                            break;
                        default:
                            break;
                    }
                    uint8_t stopOffset = samsTable[stop].port;   // 0 to 63
                    uint8_t bitOffset = stopOffset;
                    if (stopOffset > 31) {
                        word = 1;
                        bitOffset -= 32;
                    }
                    samsTable[key].state = false;
                    spiOut.channel[channel][word] |= 1 << bitOffset; // set bit
                    //spiOut.channel[channel][word] &= (~(1 << bitOffset)); // clear bit
                }
            }
        }
        if (keyDown) prevData |= 0x80; // store the data
        keyTable.channel[channel][key] = prevData;
    }
}

void enqueue(MIDIQUEUEMSGPTR *headPtr, MIDIQUEUEMSGPTR *tailPtr) {
    MIDIQUEUEMSGPTR newPtr;
    
    newPtr = malloc(sizeof(MIDIQUEUEMSG));
    
    if (newPtr != NULL) {
        newPtr->msgType = midiMsg.msgType;
        newPtr->msgChannel = midiMsg.msgChannel;
        newPtr->msgFinishedLen = midiMsg.msgFinishedLen;
        newPtr->nextPtr = NULL;
        int i;
        for (i = 0; i < 4; i++) {
            newPtr->midiMessage[i] = midiMsg.midiMessage[i];
        }
        
        if (isEmpty(*headPtr))
            *headPtr = newPtr;
        else
            (*tailPtr)->nextPtr = newPtr;
        
        *tailPtr = newPtr;
    }
}

void dequeue(MIDIQUEUEMSGPTR *headPtr, MIDIQUEUEMSGPTR *tailPtr) {
    MIDIQUEUEMSGPTR tempPtr;
    
    midiMsg.msgType = (*headPtr)->msgType;
    midiMsg.msgChannel = (*headPtr)->msgChannel;
    midiMsg.msgFinishedLen = (*headPtr)->msgFinishedLen;
    int i;
    for (i = 0; i < 4; i++) {
        midiMsg.midiMessage[i] = (*headPtr)->midiMessage[i];
    }
    tempPtr = *headPtr;
    *headPtr = (*headPtr)->nextPtr;
    
    if (*headPtr == NULL)
        *tailPtr = NULL;
    
    free(tempPtr);
}

int isEmpty(MIDIQUEUEMSGPTR headPtr) {
    return headPtr == NULL;
}

void delayTimer1(unsigned short int preset) {
    T1CON = 0x8020;         // start Timer1, prescaler 1:64, int clk
    TMR1 = 0;               // zero the timer
    while (TMR1 < preset);
}

void resetTimer2(void) {
    DRV_TMR_Stop(drvTIMER_2_Handle);
    DRV_TMR_Start(drvTIMER_2_Handle);
}

void resetTimer3(void) {
    DRV_TMR_Stop(drvTIMER_3_Handle);
    DRV_TMR_Start(drvTIMER_3_Handle);
}

void resetTimer4(void) {
    DRV_TMR_Stop(drvTIMER_4_Handle);
    DRV_TMR_Start(drvTIMER_4_Handle);
}

void CallBackT2(uintptr_t context, uint32_t alarmCount) {
    if (!keyScanTime) {
        if (++keyScanCount > 15) {
            keyScanCount = 0;        // reset samScanTime counter
        }
        keyScanTime = true;
    }
}

void CallBackT3(uintptr_t context, uint32_t alarmCount) {
    /* Timer 3 does not run unless started by parseMidiMsg()
     * If Timer 4 has not run yet then try to setSAMsTime in
     * another 50ms. */
    if (!setSAMsTime) {
        setSAMsTime = true; // must be SAMs that need set or cleared
        DRV_TMR_Stop(drvTIMER_3_Handle);   // Turn Timer 3 off
    }
}

void CallBackT4(uintptr_t context, uint32_t alarmCount) {
    /* Timer 4 does not run unless started by program */
    LATASET = 0x005C;   // 1's turns off SAMs power for RA2, RA3, RA4, RA6
    if (SAMsPowerOn) {
        SAMsPowerOn = false;
        DRV_TMR_Start(drvTIMER_4_Handle);   // turn on
    }
    else if (!SAMsPowerOn) {
        DRV_TMR_Stop(drvTIMER_4_Handle);    // turn off
    }
}

/************************************************
 * Application State Reset Function
 ************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )
 */

void APP_Initialize(void) {
    /* Initialize the application object */
    appData.state = APP_STATE_INIT;
    appData.usbDevHandle = USB_DEVICE_HANDLE_INVALID;
    appData.deviceIsConfigured = false;
    appData.readTransferHandle = USB_DEVICE_TRANSFER_HANDLE_INVALID;
    appData.writeTransferHandle = USB_DEVICE_TRANSFER_HANDLE_INVALID;
    appData.epDataReadPending = false;
    appData.epDataWritePending = false;
    appData.speed = USB_SPEED_HIGH;
    appData.altSetting = 0;
    //    appData.configurationValue = 0;
    appData.endpointRx = 0x01;
    appData.endpointTx = 0x81;
    
    int i, j;
    for (i = 0; i < NUM_CHANNELS; i++) {
        for (j = 0; j < NUM_KEYS; j++) {
            keyTable.channel[i][j] = 0;
            keyBit.channel[i][j] = false;
        }
    }
    for (i = 0; i < NUM_CHANNELS; i++) {
        for (j = 0; j < DINS_PER_CHANNEL; j++) {
            spiIn.channel[i][j] = 0xffffffff;
            spiOut.channel[i][j] = 0xffffffff;
        }
    }
    
    initTranslateTable();
    
    /* Open Timers */
    uint32_t myFreq;
    uint32_t clkFreq;
    uint32_t divider;
    
    drvTIMER_2_Handle = DRV_TMR_Open (DRV_TMR_INDEX_1, DRV_IO_INTENT_EXCLUSIVE);
    myFreq = 2000; // 0.5 ms
    clkFreq = DRV_TMR_CounterFrequencyGet(drvTIMER_2_Handle); // timer running frequency
    divider = clkFreq / myFreq;
    DRV_TMR_AlarmRegister(drvTIMER_2_Handle, divider, true, 0, CallBackT2);
    DRV_TMR_Start(drvTIMER_2_Handle);
    
    drvTIMER_3_Handle = DRV_TMR_Open (DRV_TMR_INDEX_2, DRV_IO_INTENT_EXCLUSIVE);
    myFreq = 20; // 50 ms
    clkFreq = DRV_TMR_CounterFrequencyGet(drvTIMER_3_Handle); // timer running frequency
    divider = clkFreq / myFreq;
    DRV_TMR_AlarmRegister(drvTIMER_3_Handle, divider, true, 0, CallBackT3);
    
    drvTIMER_4_Handle = DRV_TMR_Open (DRV_TMR_INDEX_3, DRV_IO_INTENT_EXCLUSIVE);
    myFreq = 20; // 50 ms
    clkFreq = DRV_TMR_CounterFrequencyGet(drvTIMER_4_Handle); // timer running frequency
    divider = clkFreq / myFreq;
    DRV_TMR_AlarmRegister(drvTIMER_4_Handle, divider, true, 0, CallBackT4);
        
    /* Open ADC */
    DRV_ADC0_Open();
    
    /* Open NVM */
    JK_NVM_Initialize();
    
    /* Open USART */
    JK_USART_Initialize();
        
    /* Open SPI */
    SPI1STATCLR = 0x40; // clear the Overflow
    SPI1CONbits.ON = 1; // turn it on
    SPI2STATCLR = 0x40; // clear the Overflow
    SPI2CONbits.ON = 1; // turn it on
    SPI3STATCLR = 0x40; // clear the Overflow
    SPI3CONbits.ON = 1; // turn it on
    SPI4STATCLR = 0x40; // clear the Overflow
    SPI4CONbits.ON = 1; // turn it on
    SPI5STATCLR = 0x40; // clear the Overflow
    SPI5CONbits.ON = 1; // turn it on
    SPI6STATCLR = 0x40; // clear the Overflow
    SPI6CONbits.ON = 1; // turn it on
    
    for (i = 0; i < 64; i++) {
        sysexMsg[i] = 0;
    }

    //WDTCONbits.ON = 1;  // Turn on the watchdog timer
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {
    /* Update the application state machine based
     * on the current state */

    int i, j, k;
    
    switch (appData.state) {
        case APP_STATE_INIT:
            /* Open the device layer */
            appData.usbDevHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
            if (appData.usbDevHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appData.usbDevHandle, APP_USBDeviceEventHandler, (uintptr_t) & appData);

                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            }
            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device is configured */
            if (appData.deviceIsConfigured == true) {
                if (USB_DEVICE_ActiveSpeedGet(appData.usbDevHandle) == USB_SPEED_FULL) {
                    endpointSize = 64;
                } else if (USB_DEVICE_ActiveSpeedGet(appData.usbDevHandle) == USB_SPEED_HIGH) {
                    endpointSize = 512;
                }
                
                /* Enable endpoints. */
                if (USB_DEVICE_EndpointIsEnabled(appData.usbDevHandle, appData.endpointRx) == false) {
                    /* Enable Read Endpoint */
                    USB_DEVICE_EndpointEnable(appData.usbDevHandle, 0, appData.endpointRx,
                            USB_TRANSFER_TYPE_BULK, endpointSize);
                }
                if (USB_DEVICE_EndpointIsEnabled(appData.usbDevHandle, appData.endpointTx) == false) {
                    /* Enable Write Endpoint */
                    USB_DEVICE_EndpointEnable(appData.usbDevHandle, 0, appData.endpointTx,
                            USB_TRANSFER_TYPE_BULK, endpointSize);
                }
                /* Indicate that we are waiting for read */
                appData.epDataReadPending = true;

                /* Place a new read request. */
                USB_DEVICE_EndpointRead(appData.usbDevHandle, &appData.readTransferHandle,
                        appData.endpointRx, &receivedDataBuffer[0], sizeof (receivedDataBuffer));

                /* Device is ready to run the main task */
                appData.state = APP_STATE_READ;
            }
            break;

        case APP_STATE_READ:
            
            /* Service the watchdog timer */
            //WDTCONbits.WDTCLRKEY = 0x5743;
            
            if (!appData.deviceIsConfigured) {
                /* This means the device got deconfigured. Change the
                 * application state back to waiting for configuration. */
                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;

                /* Disable the endpoint*/
                USB_DEVICE_EndpointDisable(appData.usbDevHandle, appData.endpointRx);
                USB_DEVICE_EndpointDisable(appData.usbDevHandle, appData.endpointTx);
                appData.epDataReadPending = false;
                appData.epDataWritePending = false;
            }

            /* Parse legit MIDI Packets */
            
            rx = 0;
            sx = 0;
            
            if (appData.epDataReadPending == false) {
                while (rx < 512) {
                    for (i = 0; i < 4; i++) {
                        midiMsg.midiMessage[i] = receivedDataBuffer[rx++];
                        midiMsg.msgType = midiMsg.midiMessage[1] & 0xf0;
                        midiMsg.msgChannel = midiMsg.midiMessage[1] & 0x0f;
                    }
                    switch (midiMsg.midiMessage[0]) {
                        case CIN_NOTE_OFF:
                        case CIN_NOTE_ON:
                        case CIN_POLY_KEYPRESS:
                        case CIN_PITCHBEND_CHANGE:
                        case CIN_3_BYTE_SYS_COM:
                        case CIN_CONTROL_CHANGE:
                            midiMsg.msgFinishedLen = 3;
                            enqueue(&headRxPtr, &tailRxPtr);
                            break;
                        case CIN_MISC_RESERVED:
                        case CIN_CABLE_EVENTS:
                            /* not a valid packet or not handled */
                            break;
                        case CIN_PROGRAM_CHANGE:
                        case CIN_CHANNEL_PRESSURE:
                        case CIN_2_BYTE_SYS_COM:
                            midiMsg.msgFinishedLen = 2;
                            enqueue(&headRxPtr, &tailRxPtr);
                            break;
                        case CIN_SINGLE_BYTE:
                            rx += 4; // Advance past real time msg.                        
                            break;
                        case CIN_SYSEX_STRT_CONT:
                            for (i = 1; i < 4; i++) {
                                sysexMsg[sx++] = midiMsg.midiMessage[i];
                            }
                            break;
                        case CIN_SYSEX_ENDS_3:
                            for (i = 1; i < 4; i++) {
                                sysexMsg[sx++] = midiMsg.midiMessage[i];
                            }
                            if (sysexMsg[sx - 1] == M_END_EXCLUSIVE) {
                                midiMsg.msgFinishedLen = sx;
                                midiMsg.msgType = M_SYSTEM_EX;
                                enqueue(&headRxPtr, &tailRxPtr);
                                sx = 0;
                            }
                            break;
                        case CIN_SYSEX_ENDS_2:
                            for (i = 1; i < 3; i++) {
                                sysexMsg[sx++] = midiMsg.midiMessage[i];
                            }
                            if (sysexMsg[sx - 1] == M_END_EXCLUSIVE) {
                                midiMsg.msgFinishedLen = sx;
                                midiMsg.msgType = M_SYSTEM_EX;
                                enqueue(&headRxPtr, &tailRxPtr);
                                sx = 0;
                            }
                            break;
                        case CIN_SYSEX_ENDS_1:
                            sysexMsg[sx++] = midiMsg.midiMessage[1];

                            if (sysexMsg[sx - 1] == M_END_EXCLUSIVE) {
                                midiMsg.msgFinishedLen = sx;
                                midiMsg.msgType = M_SYSTEM_EX;
                                enqueue(&headRxPtr, &tailRxPtr);
                                sx = 0;
                            }
                            break;
                        default:
                            break;
                    }
                }
                
                decodeMidi();

                for (i = 0; i < 512; i++) {
                    receivedDataBuffer[i] = 0;
                }
                for (i = 0; i < 64; i++) {
                    sysexMsg[i] = 0;
                }
            }

            appData.epDataReadPending = true;

            /* Place a new read request. */
            USB_DEVICE_EndpointRead(appData.usbDevHandle, &appData.readTransferHandle,
                    appData.endpointRx, &receivedDataBuffer[0], sizeof (receivedDataBuffer));

            
            if (keyScanTime) {
                appData.state = APP_STATE_SCAN;
            } else {
                appData.state = APP_STATE_WRITE;
            }

            break;

        case APP_STATE_SCAN:
            
            /* Scan SPI inputs */
            keyScanTime = false;
            
            // Registers to Load mode
            LATCCLR = 0x0004;
            LATDCLR = 0x1031;
            LATFCLR = 0x1000;

#ifndef USE_MATRIX
            if (!SPI1STATbits.SPIRBE)
                spiIn.channel[0][1] = SPI1BUF; // read a 32-bit integer 2 times
            if (!SPI1STATbits.SPIRBE)
                spiIn.channel[0][0] = SPI1BUF;
            if (!SPI1STATbits.SPIRBE)
                spiIn.channel[1][1] = SPI1BUF;
            if (!SPI1STATbits.SPIRBE)
                spiIn.channel[1][0] = SPI1BUF;

            if (!SPI2STATbits.SPIRBE)
                spiIn.channel[2][1] = SPI2BUF;
            if (!SPI2STATbits.SPIRBE)
                spiIn.channel[2][0] = SPI2BUF;
            if (!SPI2STATbits.SPIRBE)
                spiIn.channel[3][1] = SPI2BUF;
            if (!SPI2STATbits.SPIRBE)
                spiIn.channel[3][0] = SPI2BUF;
#else            
            for (i = 0; i < 22; i++) {
                
                LATDCLR = 0x0010;           // load, don't shift
                while (!SPI1STATbits.SPIRBE)
                    spiIn.channel[0][0] = SPI1BUF; // read previous column
                
                getTwelveBits(i);
                
                LATDSET = 0x0010;           // shift, don't load
                if (i == 18)
                    setMatrixColumn(0);
                else if (i == 19)
                    setMatrixColumn(1);
                else if (i == 20)
                    setMatrixColumn(2);
                else if (i == 21)
                    setMatrixColumn(3);
                else
                    setMatrixColumn(i + 4);
                SPI1BUF = spiOut.channel[0][0]; // set next column
                delayTimer1(5);
            }
            
            for (i = 0; i < 4; i++) {
                updateKeyTable(i);
            }
            
#endif     
            if (!SPI3STATbits.SPIRBE)
                spiIn.channel[4][1] = SPI3BUF;
            if (!SPI3STATbits.SPIRBE)
                spiIn.channel[4][0] = SPI3BUF;
            if (!SPI3STATbits.SPIRBE)
                spiIn.channel[5][1] = SPI3BUF;
            if (!SPI3STATbits.SPIRBE)
                spiIn.channel[5][0] = SPI3BUF;
            
            if (!SPI4STATbits.SPIRBE)
                spiIn.channel[6][1] = SPI4BUF;
            if (!SPI4STATbits.SPIRBE)
                spiIn.channel[6][0] = SPI4BUF;
            if (!SPI4STATbits.SPIRBE)
                spiIn.channel[7][1] = SPI4BUF;
            if (!SPI4STATbits.SPIRBE)
                spiIn.channel[7][0] = SPI4BUF;
            
            if (channelTable[8].channel_use == CH_STOPS) {
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[8][0] = SPI5BUF;
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[8][1] = SPI5BUF;
            } else {
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[8][1] = SPI5BUF;
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[8][0] = SPI5BUF;
            }
            
            if (channelTable[9].channel_use == CH_STOPS) {
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[9][0] = SPI5BUF;
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[9][1] = SPI5BUF;
            } else {
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[9][1] = SPI5BUF;
                if (!SPI5STATbits.SPIRBE)
                    spiIn.channel[9][0] = SPI5BUF;
            }
            
            if (channelTable[10].channel_use == CH_STOPS) {
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[10][0] = SPI6BUF;
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[10][1] = SPI6BUF;
            } else {
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[10][1] = SPI6BUF;
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[10][0] = SPI6BUF;
            }
            
            if (channelTable[11].channel_use == CH_STOPS) {
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[11][0] = SPI6BUF;
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[11][1] = SPI6BUF;
            } else {
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[11][1] = SPI6BUF;
                if (!SPI6STATbits.SPIRBE)
                    spiIn.channel[11][0] = SPI6BUF;
            }
            
            LATCSET = 0x0004;
            LATDSET = 0x1031;
            LATFSET = 0x1000;

            for (i = 3; i >= 0; i--) {
#ifndef USE_MATRIX
                SPI1BUF = spiOut.bank[0][i]; // write a 32-bit integer to each DOUT
                SPI2BUF = spiOut.bank[1][i];
#endif
                SPI3BUF = spiOut.bank[2][i];
                SPI4BUF = spiOut.bank[3][i];
                SPI5BUF = spiOut.bank[4][i];
                SPI6BUF = spiOut.bank[5][i];
            }
            
            // Clear transmit buffer
            for (i = 0; i < 512; i++) {
                transmitDataBuffer[i] = 0;
            }
            tx = 0;

            // Update keyBit table
#ifdef USE_MATRIX
            for (i = 4; i < NUM_CHANNELS; i++) { // for channels 5 thru 12
#else            
            for (i = 0; i < NUM_CHANNELS; i++) { // for channels 1 thru 12
#endif
                for (j = 0; j < DINS_PER_CHANNEL; j++) { // for 2 32-bit words
                    uint32_t tempData = spiIn.channel[i][j]; // normally high
                    if (channelTable[i].invertData) {
                        tempData ^= 0xffffffff; // normally low
                    }
                    int index = j * 32;
                    for (k = 0; k < 32; k++) {
                        keyBit.channel[i][index++] = tempData & 0x00000001;
                        tempData >>= 1;
                    }
                }
                updateKeyTable(i);
            }
            DRV_ADC_Start();
            
            /* Scan analog inputs */
            uint32_t diff;
            
            switch (keyScanCount) {
                case 0:
                    if (DRV_ADC_SamplesAvailable(12)) {
                        freshPot[0] = DRV_ADC_SamplesRead(12);
                    }
                    break;
                case 1:
                    if (DRV_ADC_SamplesAvailable(13)) {
                        freshPot[1] = DRV_ADC_SamplesRead(13);
                    }
                    break;
                case 2:
                    if (DRV_ADC_SamplesAvailable(19)) {
                        freshPot[2] = DRV_ADC_SamplesRead(19);
                    }
                    break;
                case 3:
                    if (DRV_ADC_SamplesAvailable(20)) {
                        freshPot[3] = DRV_ADC_SamplesRead(20);
                    }
                    break;
                case 4:
                    if (DRV_ADC_SamplesAvailable(27)) {
                        freshPot[4] = DRV_ADC_SamplesRead(27);
                    }
                    break;
                case 5:
                    if (DRV_ADC_SamplesAvailable(28)) {
                        freshPot[5] = DRV_ADC_SamplesRead(28);
                    }
                    break;
                case 6:
                    if (DRV_ADC_SamplesAvailable(24)) {
                        freshPot[6] = DRV_ADC_SamplesRead(24);
                    }
                    break;
                case 7:
                    if (DRV_ADC_SamplesAvailable(34)) {
                        freshPot[7] = DRV_ADC_SamplesRead(34);
                    }
                    break;
                default:
                    break;
            }
            
            if (keyScanCount < 8) {
                freshPot[keyScanCount] = freshPot[keyScanCount] >> 5;
                freshPot[keyScanCount] = (freshPot[keyScanCount] + stalePot[keyScanCount]) >> 1; // average
                diff = freshPot[keyScanCount] - stalePot[keyScanCount]; // find difference
                if (diff > 1) {
                    midiMsg.msgType = M_CTRL_CHANGE;
                    midiMsg.msgChannel = keyScanCount;
                    midiMsg.msgFinishedLen = 3;
                    midiMsg.midiMessage[0] = CIN_CONTROL_CHANGE;
                    midiMsg.midiMessage[1] = M_CTRL_CHANGE | keyScanCount;
                    midiMsg.midiMessage[2] = M_CC_VOLUME;
                    midiMsg.midiMessage[3] = (uint8_t) freshPot[keyScanCount];
                    stalePot[keyScanCount] = freshPot[keyScanCount]; // save previous value

                    enqueue(&headTxPtr, &tailTxPtr);
                }
            }
            
            if ((setSAMsTime) && ( !SAMsPowerOn)) {
                if (sp1_SAMsPending) {
                    SAMsPowerOn = true; // Turn on power flag
                    LATAbits.LATA2 = 0; // Turn on power #1 (red)
                    resetTimer4(); // Start timer4
                    sp1_SAMsPending = false;
                } else if (sp2_SAMsPending) {
                    SAMsPowerOn = true;
                    LATAbits.LATA3 = 0; // Turn on power #2 (green)
                    resetTimer4();
                    sp2_SAMsPending = false;
                } else if (sp3_SAMsPending) {
                    SAMsPowerOn = true;
                    LATAbits.LATA4 = 0; // Turn on power #3 (blue)
                    resetTimer4();
                    sp3_SAMsPending = false;
                } else if (sp4_SAMsPending) {
                    SAMsPowerOn = true;
                    LATAbits.LATA6 = 0; // Turn on power #4 (white)
                    resetTimer4();
                    sp4_SAMsPending = false;
                } else {
                    setSAMsTime = false; // When nothing more is pending
                }
            }
            
            appData.state = APP_STATE_WRITE;

            break;

        case APP_STATE_WRITE:
            
            JK_NVM_Tasks();
            
            JK_USART_Tasks1();
            JK_USART_Tasks2();
            JK_USART_Tasks3();
            JK_USART_Tasks4();
            JK_USART_Tasks5();
                       
//            if (dataAtUsart1) {
//                usart1Data.drvUsartTxBuffer[0] = usart1Data.drvUsartRxBuffer[0];
//                usart1Data.state = JK_USART_STATE_TX;
//                dataAtUsart1 = false;
//            } 
            
            int txIndex = 0;

            while (!isEmpty(headTxPtr)) {
                dequeue(&headTxPtr, &tailTxPtr);
                
                for (i = 0; i < 4; i++) {
                    transmitDataBuffer[txIndex++] = midiMsg.midiMessage[i];
                }
                if (txIndex >= 64) {
                    LATGbits.LATG12 = LED_ON;
                    break;
                }
            }

            if (txIndex > 0) {
                appData.epDataWritePending = true;

                USB_DEVICE_EndpointWrite(appData.usbDevHandle, &appData.writeTransferHandle,
                        appData.endpointTx, &transmitDataBuffer[0],
                        txIndex,
                        USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
            }

            appData.state = APP_STATE_READ;

            break;
            
//        case APP_STATE_CONFIGURE_ENCODER:
//            
//            appData.state = APP_STATE_INIT;
//            break;

        case APP_STATE_ERROR:
            break;

        default:
            break;
    }
}

/*******************************************************************************
 End of File
 */

