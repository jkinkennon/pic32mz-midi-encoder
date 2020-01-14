#include <stdint.h>

#ifndef _CONSOLE_H    /* Guard against multiple inclusion */
#define _CONSOLE_H

/* User Configurable Definitions */
//#define USE_CRESC_LEDS        
//#define USE_TRANSPOSE
#define USE_MATRIX          
#define NUM_COLUMNS         11  // Columns per matrix
#define NUM_ROWS            6   // Rows per matrix
#define LED_ON              1   // 1 if status LEDs go to ground, 0 if sinking
#define LED_OFF             0   //   current where LEDs all wire to +3.3v

/* Fixed Definitions not part of user configuration */
#define NUM_KEYS            64  // Keys per matrix - normally 64
#define NUM_CHANNELS        12  // Always 12 possible MIDI channels
#define NUM_SWITCHES        (NUM_CHANNELS * NUM_KEYS) // default total keys and pistons
#define NUM_SPI_BANKS       6   // Number of SPI banks used (128 bits) (0 to 6)
#define DINS_PER_BANK       4   // Number of DINs on each SPI bank
#define DINS_PER_CHANNEL    2   // Number of 32-bit DINs per MIDI channel
#define NUM_POTS            8   // Number of potentiometers
//#define STOPS_PER_CHANNEL_32    // Comment out for 64 stops per channel
#define NUM_STOPS           64

typedef enum {
    DIV_UNUSED = 0,
    DIV_PEDAL,
    DIV_SOLO,
    DIV_SWELL,  
    DIV_GREAT,        
    DIV_CHOIR,
    DIV_COUPLER
} DIVISION;

typedef struct {
    DIVISION division;  // For Allen consoles this is the power source
    uint8_t channel;    // 4 possible channels
    uint8_t port;       // 64 stops per channel
    bool    state;      // True = stop is selected, False = stop is off
} stop_t;

typedef enum {
    CH_UNUSED = 0,
    CH_MANUAL,
    CH_PEDAL,
    CH_STOPS,
    CH_PISTONS
} CHANNEL_USE;

typedef struct {
    uint8_t channel;    // 16 possible channels, see NUM_CHANNELS
    uint8_t port;       // SPI port, 0 to 5
    bool    invertData; // true to invert incoming SPI data
    CHANNEL_USE channel_use;
} channel_t;

typedef union {
    uint8_t transTable[16][NUM_KEYS][8];
    uint8_t transOffset[4][2048];
} transTable_t;

void initTranslateTable(void);

#endif /* _CONSOLE_H */

/* *****************************************************************************
 End of File
 */
