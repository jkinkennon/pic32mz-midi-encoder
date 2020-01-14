
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
    {0, 0, false, CH_MATRIX},
    {1, 0, false, CH_MATRIX},   // CH_MATRIX for 0,0 overrides this setting
    {2, 1, false, CH_MATRIX},
    {3, 1, false, CH_MATRIX},   // CH_MATRIX for 2,1 overrides this setting
    {4, 2, false, CH_UNUSED},
    {5, 2, true, CH_UNUSED},
    {6, 3, true, CH_PISTONS},
    {7, 3, true, CH_PISTONS},
    {8, 4, true, CH_STOPS},
    {9, 4, true, CH_STOPS},
    {10, 5, true, CH_STOPS},
    {11, 5, true, CH_UNUSED}
};

/* Stops by division, channel, (0 - 63) or (0 - 31), state */
stop_t samsTable[NUM_STOPS] = {         // 256 stops
    // Pedal division SAMs (33)
    {DIV_PEDAL, 8, 0, false},       // Tremulant            1-1-A0
    {DIV_PEDAL, 8, 1, false},       // Oboe 8               1-1-A1
    {DIV_PEDAL, 8, 2, false},       // Fagotto 16           1-1-A2
    {DIV_PEDAL, 8, 3, false},       // Prinzipal 4          1-1-A3
    {DIV_PEDAL, 8, 4, false},       // Spitz Prinzipal      1-1-A4
    {DIV_PEDAL, 8, 5, false},       // Celeste Effect       1-1-A5
    {DIV_PEDAL, 8, 6, false},       // Speech Artic. Off    1-1-A6
    {DIV_PEDAL, 8, 7, false},       // Schalmei 4           1-1-A7
    {DIV_PEDAL, 8, 8, false},       // Trumpet 8            1-2-A0
    {DIV_PEDAL, 8, 9, false},       // Mixtur III           1-2-A1
    {DIV_PEDAL, 8, 10, false},      // Nasat 2 2/3          1-2-A2
    {DIV_PEDAL, 8, 11, false},      // Viole 8              1-2-A3
    {DIV_PEDAL, 8, 12, false},      // Percussion           1-2-A4
    {DIV_PEDAL, 8, 13, false},      // En Chamade           1-2-A5
    {DIV_PEDAL, 8, 14, false},      // Alterable 9          1-2-A6
    {DIV_PEDAL, 8, 15, false},      // Alterable 10         1-2-A7
    {DIV_PEDAL, 8, 16, false},      // Sifflote 1           1-3-A0
    {DIV_PEDAL, 8, 17, false},      // Blockflote 2         1-3-A1
    {DIV_PEDAL, 8, 18, false},      // Koppel Flote 4       1-3-A2
    {DIV_PEDAL, 8, 19, false},      // Flute 8              1-3-A3
    {DIV_PEDAL, 8, 20, false},      // Chiff                1-3-A4
    {DIV_PEDAL, 8, 21, false},      // Schalmei 8           1-3-A5
    {DIV_PEDAL, 8, 22, false},      // Fourniture IV        1-3-A6
    {DIV_PEDAL, 8, 23, false},      // Super Oktav 2        1-3-A7
    {DIV_PEDAL, 8, 24, false},      // Oktav 4              1-4-A0
    {DIV_PEDAL, 8, 25, false},      // Prinzipal 8          1-4-A1
    {DIV_PEDAL, 8, 26, false},      // Quintaden 16         1-4-A2
    {DIV_PEDAL, 8, 27, false},      // Tremulant            1-4-A3
    {DIV_PEDAL, 8, 28, false},      // Trumpet 8            1-4-A4
    {DIV_PEDAL, 8, 29, false},      // Waldflote 2          1-4-A5
    {DIV_PEDAL, 8, 30, false},      // Quinte 2 2/3         1-4-A6
    {DIV_PEDAL, 8, 31, false},      // Spitzflote 4         1-4-A7
    {DIV_PEDAL, 8, 32, false},      // Dulciana 8           2-1-A0
    // Swell division SAMs (35)
    {DIV_SWELL, 8, 33, false},      // Celeste Tuning       2-1-A1
    {DIV_SWELL, 8, 34, false},      // Alterable 5          2-1-A2
    {DIV_SWELL, 8, 35, false},      // Alterable 6          2-1-A3
    {DIV_SWELL, 8, 36, false},      // Alterable 7          2-1-A4
    {DIV_SWELL, 8, 37, false},      // Alterable 8          2-1-A5
    {DIV_SWELL, 8, 37, false},      // Hohlflote 8          2-1-A6
    {DIV_SWELL, 8, 39, false},      // Speech Artic. Off    2-1-A7
    {DIV_SWELL, 8, 40, false},      // Chimes               2-2-A0
    {DIV_SWELL, 8, 41, false},      // Choral Bass 4        2-2-A1
    {DIV_SWELL, 8, 42, false},      // Octave 8             2-2-A2
    {DIV_SWELL, 8, 43, false},      // Prinzipal 16         2-2-A3
    {DIV_SWELL, 8, 44, false},      // Contra Bass 32       2-2-A4
    {DIV_SWELL, 8, 45, false},      // MIXTUR II            2-2-A5
    {DIV_SWELL, 8, 46, false},      // Posaune 16           2-2-A6
    {DIV_SWELL, 8, 47, false},      // Gedeckt 8            2-2-A7
    {DIV_SWELL, 8, 48, false},      // Subbass 16           2-3-A0
    {DIV_SWELL, 8, 49, false},      // Celeste Tuning       2-3-A1
    {DIV_SWELL, 8, 50, false},      // Trompete 8           2-3-A2
    {DIV_SWELL, 8, 51, false},      // Flute Ouverte        2-3-A3
    {DIV_SWELL, 8, 52, false},      // Lieblich Gedeckt 16  2-3-A4
    {DIV_SWELL, 8, 53, false},      // Untersatz 32         2-3-A5
    {DIV_SWELL, 8, 54, false},      // Sub Octaver L        2-3-A6
    {DIV_SWELL, 8, 55, false},      // Alterable 1          2-3-A7
    {DIV_SWELL, 8, 56, false},      // Alterable 2          2-4-A0
    {DIV_SWELL, 8, 57, false},      // Alterable 3          2-4-A1
    {DIV_SWELL, 8, 58, false},      // Alterable 4          2-4-A2
    {DIV_SWELL, 8, 59, false},      // Percussion           2-4-A3
    {DIV_SWELL, 8, 60, false},      // Clarion 4            2-4-A4
    {DIV_SWELL, 8, 61, false},      // Contre Trompette 16  2-4-A5
    {DIV_SWELL, 8, 62, false},      // Plein Jeu III        2-4-A6
    {DIV_SWELL, 8, 63, false},      // Spitz Prinzipal 4    2-4-A7
    {DIV_SWELL, 9, 0, false},       // Salizional 8         3-1-A0
    {DIV_SWELL, 9, 1, false},       // Sub Octaver R        3-1-A1
    {DIV_SWELL, 9, 2, false},       // Trompete 8           3-1-A2
    {DIV_SWELL, 9, 3, false},       // Terz 1 3/5           3-1-A3
    {DIV_COUPLER, 9, 4, false},     // Nazard 2 2/3         3-1-A4
    {DIV_COUPLER, 9, 5, false},     // Gemshorn 8           3-1-A5
    {DIV_COUPLER, 9, 6, false},     // Celeste Tuning       3-1-A6
    {DIV_COUPLER, 9, 7, false},     // Tremulant            3-1-A7
    {DIV_COUPLER, 9, 8, false},     // Hautbois 8           3-2-A0
    {DIV_COUPLER, 9, 9, false},     // Flachflote 2         3-2-A1
    {DIV_COUPLER, 9, 10, false},    // Rohrflote 4          3-2-A2
    {DIV_COUPLER, 9, 11, false},    // Gedeckt 8            3-2-A3
    {DIV_COUPLER, 9, 12, false},    // Blank                3-2-A4
    {DIV_COUPLER, 9, 13, false},    // Great to Pedal       3-2-A5
    {DIV_COUPLER, 9, 14, false},    // Swell to Pedal       3-2-A6
    {DIV_COUPLER, 9, 15, false},    // Choir to Pedal       3-2-A7
    // Spares (16)
    {DIV_UNUSED, 9, 16, false},     // Swell to Great       3-3-A0
    {DIV_UNUSED, 9, 17, false},     // Choir to Great       3-3-A1
    {DIV_UNUSED, 9, 18, false},     // Swell to Choir       3-3-A2
    {DIV_UNUSED, 9, 19, false},     // Monitor Speaker      3-3-A3
    {DIV_UNUSED, 9, 20, false},     // Antiphonal Organ     3-3-A4
    {DIV_UNUSED, 9, 21, false},     // Main Organ Off       3-3-A5
    {DIV_UNUSED, 9, 22, false},     // Spare                3-3-A6
    {DIV_UNUSED, 9, 23, false},     // Spare                3-3-A7
    {DIV_UNUSED, 9, 24, false},     // Spare                3-4-A0
    {DIV_UNUSED, 9, 25, false},     // Spare                3-4-A1
    {DIV_UNUSED, 9, 26, false},     // Spare                3-4-A2
    {DIV_UNUSED, 9, 27, false},     // Spare                3-4-A3
    {DIV_UNUSED, 9, 28, false},     // Spare                3-4-A4
    {DIV_UNUSED, 9, 29, false},     // Spare                3-4-A5
    {DIV_UNUSED, 9, 30, false},     // Spare                3-4-A6
    {DIV_UNUSED, 9, 31, false},     // Spare                3-4-A7
    {DIV_UNUSED, 9, 32, false},     // Swell to Great       3-3-A0
    {DIV_UNUSED, 9, 33, false},     // Choir to Great       3-3-A1
    {DIV_UNUSED, 9, 34, false},     // Swell to Choir       3-3-A2
    {DIV_UNUSED, 9, 35, false},     // Monitor Speaker      3-3-A3
    {DIV_UNUSED, 9, 36, false},     // Antiphonal Organ     3-3-A4
    {DIV_UNUSED, 9, 37, false},     // Main Organ Off       3-3-A5
    {DIV_UNUSED, 9, 38, false},     // Spare                3-3-A6
    {DIV_UNUSED, 9, 39, false},     // Spare                3-3-A7
    {DIV_UNUSED, 9, 40, false},     // Spare                3-4-A0
    {DIV_UNUSED, 9, 41, false},     // Spare                3-4-A1
    {DIV_UNUSED, 9, 42, false},     // Spare                3-4-A2
    {DIV_UNUSED, 9, 43, false},     // Spare                3-4-A3
    {DIV_UNUSED, 9, 44, false},     // Spare                3-4-A4
    {DIV_UNUSED, 9, 45, false},     // Spare                3-4-A5
    {DIV_UNUSED, 9, 46, false},     // Spare                3-4-A6
    {DIV_UNUSED, 9, 47, false},     // Spare                3-4-A7
    {DIV_UNUSED, 9, 48, false},     // Swell to Great       3-3-A0
    {DIV_UNUSED, 9, 49, false},     // Choir to Great       3-3-A1
    {DIV_UNUSED, 9, 50, false},     // Swell to Choir       3-3-A2
    {DIV_UNUSED, 9, 51, false},     // Monitor Speaker      3-3-A3
    {DIV_UNUSED, 9, 52, false},     // Antiphonal Organ     3-3-A4
    {DIV_UNUSED, 9, 53, false},     // Main Organ Off       3-3-A5
    {DIV_UNUSED, 9, 54, false},     // Spare                3-3-A6
    {DIV_UNUSED, 9, 55, false},     // Spare                3-3-A7
    {DIV_UNUSED, 9, 56, false},     // Spare                3-4-A0
    {DIV_UNUSED, 9, 57, false},     // Spare                3-4-A1
    {DIV_UNUSED, 9, 58, false},     // Spare                3-4-A2
    {DIV_UNUSED, 9, 59, false},     // Spare                3-4-A3
    {DIV_UNUSED, 9, 60, false},     // Spare                3-4-A4
    {DIV_UNUSED, 9, 61, false},     // Spare                3-4-A5
    {DIV_UNUSED, 9, 62, false},     // Spare                3-4-A6
    {DIV_UNUSED, 9, 63, false},     // Spare                3-4-A7
    // SOLO division SAMs (27)
    {DIV_SOLO, 10, 0, false},       // Tremulant            1-1-A0
    {DIV_SOLO, 10, 1, false},       // Oboe 8               1-1-A1
    {DIV_SOLO, 10, 2, false},       // Fagotto 16           1-1-A2
    {DIV_SOLO, 10, 3, false},       // Prinzipal 4          1-1-A3
    {DIV_SOLO, 10, 4, false},       // Spitz Prinzipal      1-1-A4
    {DIV_SOLO, 10, 5, false},       // Celeste Effect       1-1-A5
    {DIV_SOLO, 10, 6, false},       // Speech Artic. Off    1-1-A6
    {DIV_SOLO, 10, 7, false},       // Schalmei 4           1-1-A7
    {DIV_SOLO, 10, 8, false},       // Trumpet 8            1-2-A0
    {DIV_SOLO, 10, 9, false},       // Mixtur III           1-2-A1
    {DIV_SOLO, 10, 10, false},      // Nasat 2 2/3          1-2-A2
    {DIV_SOLO, 10, 11, false},      // Viole 8              1-2-A3
    {DIV_SOLO, 10, 12, false},      // Percussion           1-2-A4
    {DIV_SOLO, 10, 13, false},      // En Chamade           1-2-A5
    {DIV_SOLO, 10, 14, false},      // Alterable 9          1-2-A6
    {DIV_SOLO, 10, 15, false},      // Alterable 10         1-2-A7
    {DIV_SOLO, 10, 16, false},      // Sifflote 1           1-3-A0
    {DIV_SOLO, 10, 17, false},      // Blockflote 2         1-3-A1
    {DIV_SOLO, 10, 18, false},      // Koppel Flote 4       1-3-A2
    {DIV_SOLO, 10, 19, false},      // Flute 8              1-3-A3
    {DIV_SOLO, 10, 20, false},      // Chiff                1-3-A4
    {DIV_SOLO, 10, 21, false},      // Schalmei 8           1-3-A5
    {DIV_SOLO, 10, 22, false},      // Fourniture IV        1-3-A6
    {DIV_SOLO, 10, 23, false},      // Super Oktav 2        1-3-A7
    {DIV_SOLO, 10, 24, false},      // Oktav 4              1-4-A0
    {DIV_SOLO, 10, 25, false},      // Prinzipal 8          1-4-A1
    {DIV_SOLO, 10, 26, false},      // Quintaden 16         1-4-A2
    // Great division SAMs (25)
    {DIV_GREAT, 10, 27, false},     // Tremulant            1-4-A3
    {DIV_GREAT, 10, 28, false},     // Trumpet 8            1-4-A4
    {DIV_GREAT, 10, 29, false},     // Waldflote 2          1-4-A5
    {DIV_GREAT, 10, 30, false},     // Quinte 2 2/3         1-4-A6
    {DIV_GREAT, 10, 31, false},     // Spitzflote 4         1-4-A7
    {DIV_GREAT, 10, 32, false},     // Dulciana 8           2-1-A0
    {DIV_GREAT, 10, 33, false},     // Celeste Tuning       2-1-A1
    {DIV_GREAT, 10, 34, false},     // Alterable 5          2-1-A2
    {DIV_GREAT, 10, 35, false},     // Alterable 6          2-1-A3
    {DIV_GREAT, 10, 36, false},     // Alterable 7          2-1-A4
    {DIV_GREAT, 10, 37, false},     // Alterable 8          2-1-A5
    {DIV_GREAT, 10, 37, false},     // Hohlflote 8          2-1-A6
    {DIV_GREAT, 10, 39, false},     // Speech Artic. Off    2-1-A7
    {DIV_GREAT, 10, 40, false},     // Chimes               2-2-A0
    {DIV_GREAT, 10, 41, false},     // Choral Bass 4        2-2-A1
    {DIV_GREAT, 10, 42, false},     // Octave 8             2-2-A2
    {DIV_GREAT, 10, 43, false},     // Prinzipal 16         2-2-A3
    {DIV_GREAT, 10, 44, false},     // Contra Bass 32       2-2-A4
    {DIV_GREAT, 10, 45, false},     // MIXTUR II            2-2-A5
    {DIV_GREAT, 10, 46, false},     // Posaune 16           2-2-A6
    {DIV_GREAT, 10, 47, false},     // Gedeckt 8            2-2-A7
    {DIV_GREAT, 10, 48, false},     // Subbass 16           2-3-A0
    {DIV_GREAT, 10, 49, false},     // Celeste Tuning       2-3-A1
    {DIV_GREAT, 10, 50, false},     // Trompete 8           2-3-A2
    {DIV_GREAT, 10, 51, false},     // Flute Ouverte        2-3-A3
    // Choir division SAMs (25)
    {DIV_CHOIR, 10, 52, false},     // Lieblich Gedeckt 16  2-3-A4
    {DIV_CHOIR, 10, 53, false},     // Untersatz 32         2-3-A5
    {DIV_CHOIR, 10, 54, false},     // Sub Octaver L        2-3-A6
    {DIV_CHOIR, 10, 55, false},     // Alterable 1          2-3-A7
    {DIV_CHOIR, 10, 56, false},     // Alterable 2          2-4-A0
    {DIV_CHOIR, 10, 57, false},     // Alterable 3          2-4-A1
    {DIV_CHOIR, 10, 58, false},     // Alterable 4          2-4-A2
    {DIV_CHOIR, 10, 59, false},     // Percussion           2-4-A3
    {DIV_CHOIR, 10, 60, false},     // Clarion 4            2-4-A4
    {DIV_CHOIR, 10, 61, false},     // Contre Trompette 16  2-4-A5
    {DIV_CHOIR, 10, 62, false},     // Plein Jeu III        2-4-A6
    {DIV_CHOIR, 10, 63, false},     // Spitz Prinzipal 4    2-4-A7
    {DIV_CHOIR, 11, 0, false},      // Salizional 8         3-1-A0
    {DIV_CHOIR, 11, 1, false},      // Sub Octaver R        3-1-A1
    {DIV_CHOIR, 11, 2, false},      // Trompete 8           3-1-A2
    {DIV_CHOIR, 11, 3, false},      // Terz 1 3/5           3-1-A3
    {DIV_CHOIR, 11, 4, false},      // Nazard 2 2/3         3-1-A4
    {DIV_CHOIR, 11, 5, false},      // Gemshorn 8           3-1-A5
    {DIV_CHOIR, 11, 6, false},      // Celeste Tuning       3-1-A6
    {DIV_CHOIR, 11, 7, false},      // Tremulant            3-1-A7
    {DIV_CHOIR, 11, 8, false},      // Hautbois 8           3-2-A0
    {DIV_CHOIR, 11, 9, false},      // Flachflote 2         3-2-A1
    {DIV_CHOIR, 11, 10, false},     // Rohrflote 4          3-2-A2
    {DIV_CHOIR, 11, 11, false},     // Gedeckt 8            3-2-A3
    {DIV_CHOIR, 11, 12, false},     // Blank                3-2-A4
    // Spares (19)
    {DIV_UNUSED, 11, 13, false},    // Great to Pedal       3-2-A5
    {DIV_UNUSED, 11, 14, false},    // Swell to Pedal       3-2-A6
    {DIV_UNUSED, 11, 15, false},    // Choir to Pedal       3-2-A7
    {DIV_UNUSED, 11, 16, false},    // Swell to Great       3-3-A0
    {DIV_UNUSED, 11, 17, false},    // Choir to Great       3-3-A1
    {DIV_UNUSED, 11, 18, false},    // Swell to Choir       3-3-A2
    {DIV_UNUSED, 11, 19, false},    // Monitor Speaker      3-3-A3
    {DIV_UNUSED, 11, 20, false},    // Antiphonal Organ     3-3-A4
    {DIV_UNUSED, 11, 21, false},    // Main Organ Off       3-3-A5
    {DIV_UNUSED, 11, 22, false},    // Spare                3-3-A6
    {DIV_UNUSED, 11, 23, false},    // Spare                3-3-A7
    {DIV_UNUSED, 11, 24, false},    // Spare                3-4-A0
    {DIV_UNUSED, 11, 25, false},    // Spare                3-4-A1
    {DIV_UNUSED, 11, 26, false},    // Spare                3-4-A2
    {DIV_UNUSED, 11, 27, false},    // Spare                3-4-A3
    {DIV_UNUSED, 11, 28, false},    // Spare                3-4-A4
    {DIV_UNUSED, 11, 29, false},    // Spare                3-4-A5
    {DIV_UNUSED, 11, 30, false},    // Spare                3-4-A6
    {DIV_UNUSED, 11, 31, false},    // Spare                3-4-A7
    {DIV_UNUSED, 11, 32, false},     // Swell to Great       3-3-A0
    {DIV_UNUSED, 11, 33, false},     // Choir to Great       3-3-A1
    {DIV_UNUSED, 11, 34, false},     // Swell to Choir       3-3-A2
    {DIV_UNUSED, 11, 35, false},     // Monitor Speaker      3-3-A3
    {DIV_UNUSED, 11, 36, false},     // Antiphonal Organ     3-3-A4
    {DIV_UNUSED, 11, 37, false},     // Main Organ Off       3-3-A5
    {DIV_UNUSED, 11, 38, false},     // Spare                3-3-A6
    {DIV_UNUSED, 11, 39, false},     // Spare                3-3-A7
    {DIV_UNUSED, 11, 40, false},     // Spare                3-4-A0
    {DIV_UNUSED, 11, 41, false},     // Spare                3-4-A1
    {DIV_UNUSED, 11, 42, false},     // Spare                3-4-A2
    {DIV_UNUSED, 11, 43, false},     // Spare                3-4-A3
    {DIV_UNUSED, 11, 44, false},     // Spare                3-4-A4
    {DIV_UNUSED, 11, 45, false},     // Spare                3-4-A5
    {DIV_UNUSED, 11, 46, false},     // Spare                3-4-A6
    {DIV_UNUSED, 11, 47, false},     // Spare                3-4-A7
    {DIV_UNUSED, 11, 48, false},     // Swell to Great       3-3-A0
    {DIV_UNUSED, 11, 49, false},     // Choir to Great       3-3-A1
    {DIV_UNUSED, 11, 50, false},     // Swell to Choir       3-3-A2
    {DIV_UNUSED, 11, 51, false},     // Monitor Speaker      3-3-A3
    {DIV_UNUSED, 11, 52, false},     // Antiphonal Organ     3-3-A4
    {DIV_UNUSED, 11, 53, false},     // Main Organ Off       3-3-A5
    {DIV_UNUSED, 11, 54, false},     // Spare                3-3-A6
    {DIV_UNUSED, 11, 55, false},     // Spare                3-3-A7
    {DIV_UNUSED, 11, 56, false},     // Spare                3-4-A0
    {DIV_UNUSED, 11, 57, false},     // Spare                3-4-A1
    {DIV_UNUSED, 11, 58, false},     // Spare                3-4-A2
    {DIV_UNUSED, 11, 59, false},     // Spare                3-4-A3
    {DIV_UNUSED, 11, 60, false},     // Spare                3-4-A4
    {DIV_UNUSED, 11, 61, false},     // Spare                3-4-A5
    {DIV_UNUSED, 11, 62, false},     // Spare                3-4-A6
    {DIV_UNUSED, 11, 63, false},     // Spare                3-4-A7
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
