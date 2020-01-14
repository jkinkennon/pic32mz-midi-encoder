/* ************************************************************************** */
/** Descriptive File Name

  @Company
 Kinkennon Services

  @File Name
    mididef.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _EXAMPLE_FILE_NAME_H    /* Guard against multiple inclusion */
#define _EXAMPLE_FILE_NAME_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */
    
/** MIDI CIN Values where cable # is zero *************************************/
#define CIN_MISC_RESERVED   0x00    // Misc. Reserved.
#define CIN_CABLE_EVENTS    0x01    // Cable Events. Reserved
#define CIN_2_BYTE_SYS_COM  0x02    // Two-byte System Common  (MTC, Song Select)
#define CIN_3_BYTE_SYS_COM  0x03    // Three-byte System Common (SSP)
#define CIN_SYSEX_STRT_CONT 0x04    // SysEx starts or continues
#define CIN_SYSEX_ENDS_1    0x05    // SysEx ends with following single byte
#define CIN_SYSEX_ENDS_2    0x06    // SysEx ends with following two bytes
#define CIN_SYSEX_ENDS_3    0x07    // SysEx ends with following three bytes
#define CIN_NOTE_OFF        0x08    // Note-off
#define CIN_NOTE_ON         0x09    // Note-on
#define CIN_POLY_KEYPRESS   0x0A    // Poly-KeyPress
#define CIN_CONTROL_CHANGE  0x0B    // Control Change
#define CIN_PROGRAM_CHANGE  0x0C    // Program Change
#define CIN_CHANNEL_PRESSURE  0x0D    // Channel Pressure
#define CIN_PITCHBEND_CHANGE  0x0E  // PitchBend Change
#define CIN_SINGLE_BYTE     0x0F    // Single Byte

/** MIDI Default Values *******************************************************/
#define M_VELOCITY_ON   0x7f    // default where no touch sensitivity
#define M_VELOCITY_OFF  0x00    // default for note off msg
#define M_KBD_FIRST_KEY 0x24    // first key on a 61 key kybd

/** MIDI Channel Voice Messages ***********************************************/
#define M_NOTE_OFF      0x80  // 0x8n - where n is the channel
#define M_NOTE_ON       0x90  // 0x9n -	(MIDI ch 1 is n=0)
#define M_AFTERTOUCH	0xA0  // 0xAn - polyphonic key pressure
#define M_CTRL_CHANGE	0xB0  // 0xBn - control change
#define M_PROG_CHANGE	0xC0  // 0xCn - program change
#define M_CH_PRESSURE	0xD0  // 0xDn - also called aftertouch
#define M_PITCH_WHEEL	0xE0  // 0xEn - pitch wheel change

/** MIDI Channel Mode Messages - only for CC's with channel > 119 **/
#define M_ALL_SOUND_OFF 0x78  // 0x78 - all sound off
#define M_RESET_ALL_C	0x79  // 0x79 - reset all controllers
#define M_LOCAL_CONTROL 0x7A  // 0x7A - local control (0=OFF, 127=ON)
#define M_ALL_NOTES_OFF 0x7B  // 0x7B - all notes off
#define M_OMNI_OFF      0x7C  // 0x7C - omni mode off
#define M_OMNI_ON       0x7D  // 0x7D - omin mode on
#define M_MONO_ON       0x7E  // 0x7E - mono mode on
#define M_POLY_ON       0x7F  // 0x7F - poly mode on

/** MIDI System Common Messages ***********************************************/
#define M_SYSTEM_EX     0xF0  // 0xF0 - system exclusive
#define M_SYSEX_ID      0x7D  // 0x7D - test or development id
#define M_TIME_CODE_QF	0xF1  // 0xF1 - time code quarter frame
#define M_SONG_POS_PTR	0XF2  // 0xF2 - song position pointer
#define M_SONG_SELECT	0xF3  // 0xF3 - song select
#define M_TUNE_REQUEST	0xF6  // 0xF6 - tune request (tune osc's)
#define M_END_EXCLUSIVE 0xF7  // 0xF7 - end of exclusive (see F0)

/** MIDI System Real-Time Messages ********************************************/
#define M_TIMING_CLOCK	0xF8  // 0xF8 - timing clock
#define M_START         0xFA  // 0xFA - start
#define M_CONTINUE      0xFB  // 0xFB - continue
#define M_STOP          0xFC  // 0xFC - stop
#define M_ACTIVE_SENSE	0xFE  // 0xFE - active sensing
#define M_RESET         0xFF  // 0xFF - reset

/** MIDI Channel Control Messages *********************************************/
#define M_CC_BANK_SEL	0x00  // 0x00 - bank select
#define M_CC_MOD_WHEEL	0x01  // 0x01 - modulation wheel
#define M_CC_BREATH_CTL	0x02  // 0x02 - breath controller
#define M_CC_FOOT_CTL	0x04  // 0x04 - foot controller
#define M_CC_PORTAMENTO	0x05  // 0x05 - portamento
#define M_CC_VOLUME     0x07  // 0x07 - ch volume
#define M_CC_BALANCE	0x08  // 0x08 - balance
#define M_CC_PAN        0x0A  // 0x0A - pan
#define M_CC_EXPRESSION	0x0B  // 0x0B - expression   

    
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
