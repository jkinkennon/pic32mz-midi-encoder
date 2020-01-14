
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app.h"
#include "console.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

transTable_t translateTable;

/* MIDI channel utilization */
// channel_t is {MIDI Channel, SPI Port, invertData?, Channel Use}

channel_t channelTable[NUM_CHANNELS] = {
    {0, 0, true, CH_MANUAL},
    {1, 0, true, CH_MANUAL},
    {2, 1, true, CH_MANUAL},
    {3, 1, true, CH_MANUAL},
    {4, 2, true, CH_PEDAL},
    {5, 2, true, CH_MANUAL},
    {6, 3, true, CH_PISTONS},
    {7, 3, true, CH_UNUSED},
    {8, 4, true, CH_STOPS},
    {9, 4, true, CH_UNUSED},
    {10, 5, true, CH_UNUSED},
    {11, 5, true, CH_UNUSED}
};

/* Stops by division, channel, (0 - 63) or (0 - 31), state */
stop_t samsTable[NUM_STOPS] = {         // 256 stops
    // Pedal division SAMs (12 + 2)
    {DIV_PEDAL, 8, 0, false},       // Contra Bass 32       1-1-A0
    {DIV_PEDAL, 8, 1, false},       // Contre Boudon 32     1-1-A1
    {DIV_PEDAL, 8, 2, false},       // Prinzipal 16         1-1-A2
    {DIV_PEDAL, 8, 3, false},       // Bourdon 16           1-1-A3
    {DIV_PEDAL, 8, 4, false},       // Lieblich Gedeckt 16  1-1-A4
    {DIV_PEDAL, 8, 5, false},       // Octave 8             1-1-A5
    {DIV_PEDAL, 8, 6, false},       // Gedeckt Flute 8      1-1-A6
    {DIV_PEDAL, 8, 7, false},       // Choral Bass 4        1-1-A7
    {DIV_PEDAL, 8, 8, false},       // Flute Ouverte 4      1-2-A0
    {DIV_PEDAL, 8, 9, false},       // Mixtur II            1-2-A1
    {DIV_PEDAL, 8, 10, false},      // Pousaune 16          1-2-A2
    {DIV_PEDAL, 8, 11, false},      // Trumpete 8           1-2-A3
    {DIV_COUPLER, 8, 12, false},    // Great to Pedal       1-2-A4
    {DIV_COUPLER, 8, 13, false},    // Swell to Pedal       1-2-A5
    // Swell division SAMs (19))
    {DIV_SWELL, 8, 14, false},      // Salizional 8         1-2-A6
    {DIV_SWELL, 8, 15, false},      // Voix Celeste 8       1-2-A7
    {DIV_SWELL, 8, 16, false},      // Gemshorn 8           1-3-A0
    {DIV_SWELL, 8, 17, false},      // Gedeckt 8            1-3-A1
    {DIV_SWELL, 8, 18, false},      // Spitz Prinzipal 4    1-3-A2
    {DIV_SWELL, 8, 19, false},      // Koppel Flute 4       1-3-A3
    {DIV_SWELL, 8, 20, false},      // Nasat 2 2/3          1-3-A4
    {DIV_SWELL, 8, 21, false},      // Blockflute 2         1-3-A5
    {DIV_SWELL, 8, 22, false},      // Terz 1 3/5           1-3-A6
    {DIV_SWELL, 8, 23, false},      // Sifflute 1           1-3-A7
    {DIV_SWELL, 8, 24, false},      // MixturIII            1-4-A0
    {DIV_SWELL, 8, 25, false},      // Contra Fagotto 16    1-4-A1
    {DIV_SWELL, 8, 26, false},      // Hautbois 8           1-4-A2
    {DIV_SWELL, 8, 27, false},      // Trumpette 8          1-4-A3
    {DIV_SWELL, 8, 28, false},      // Clarion 4            1-4-A4
    {DIV_SWELL, 8, 29, false},      // Alterable 1          1-4-A5
    {DIV_SWELL, 8, 30, false},      // Alterable 2          1-4-A6
    {DIV_SWELL, 8, 31, false},      // Chiff                1-4-A7
    {DIV_SWELL, 8, 32, false},      // Tremulant            2-1-A0
    // Great division SAMs (18)
    {DIV_GREAT, 8, 33, false},      // Quintaden 16         2-1-A1
    {DIV_GREAT, 8, 34, false},      // Prinzipal 8          2-1-A2
    {DIV_GREAT, 8, 35, false},      // Dulciana 8           2-1-A3
    {DIV_GREAT, 8, 36, false},      // Dulciana Celeste     2-1-A4
    {DIV_GREAT, 8, 37, false},      // Hohlflute 8          2-1-A5
    {DIV_SWELL, 8, 37, false},      // Oktav 4              2-1-A6
    {DIV_SWELL, 8, 39, false},      // Spitzflute 4         2-1-A7
    {DIV_SWELL, 8, 40, false},      // Quinte 2 2/3         2-2-A0
    {DIV_SWELL, 8, 41, false},      // Doublette 2          2-2-A1
    {DIV_SWELL, 8, 42, false},      // Waldflute 2          2-2-A2
    {DIV_SWELL, 8, 43, false},      // Mixtur IV            2-2-A3
    {DIV_SWELL, 8, 44, false},      // Schmalmei 8          2-2-A4
    {DIV_SWELL, 8, 45, false},      // Krummhorn 8          2-2-A5
    {DIV_SWELL, 8, 46, false},      // Alterable 3          2-2-A6
    {DIV_SWELL, 8, 47, false},      // Alterable 4          2-2-A7
    {DIV_SWELL, 8, 48, false},      // Percussion           2-3-A0
    {DIV_SWELL, 8, 49, false},      // Swell to Great       2-3-A1
    {DIV_SWELL, 8, 50, false},      // Tremulant            2-3-A2
    
    {DIV_COUPLER, 8, 51, false},    // Sharp Tuning         2-3-A3
    {DIV_COUPLER, 8, 52, false},    // Sharp Attack Swell   2-3-A4
    {DIV_COUPLER, 8, 53, false},    // Random Motion Off    2-3-A5
    {DIV_COUPLER, 8, 54, false},    // Speech Artic. Off    2-3-A6
    {DIV_COUPLER, 8, 55, false},    // Flute Vibrato        2-3-A7
    {DIV_COUPLER, 8, 56, false},    // Reverb Off           2-4-A0
    {DIV_UNUSED, 8, 57, false},     // Spare                2-4-A1
    {DIV_UNUSED, 8, 58, false},     // Spare                2-4-A2
    {DIV_UNUSED, 8, 59, false},     // Spare                2-4-A3
    {DIV_UNUSED, 8, 60, false},     // Spare                2-4-A4
    {DIV_UNUSED, 8, 61, false},     // Spare                2-4-A5
    {DIV_UNUSED, 8, 62, false},     // Spare                2-4-A6
    {DIV_UNUSED, 8, 63, false}      // Spare                2-4-A7
};

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

void initTranslateTable(void) {
    int ch, key;

    // initialize the twelve channels (or less) with normal defaults
    for (ch = 0; ch < 16; ch++) {
        for (key = 0; key < NUM_KEYS; key++) {
            translateTable.transTable[ch][key][0] = CIN_NOTE_ON;
            translateTable.transTable[ch][key][1] = M_NOTE_ON | ch;
            translateTable.transTable[ch][key][2] = M_KBD_FIRST_KEY + key;
            translateTable.transTable[ch][key][3] = M_VELOCITY_ON;
            translateTable.transTable[ch][key][4] = CIN_NOTE_OFF;
            translateTable.transTable[ch][key][5] = M_NOTE_OFF | ch;
            translateTable.transTable[ch][key][6] = M_KBD_FIRST_KEY + key;
            translateTable.transTable[ch][key][7] = M_VELOCITY_OFF;
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */


/* *****************************************************************************
 End of File
 */
