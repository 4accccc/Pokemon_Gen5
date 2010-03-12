//============================================================================================
/**
 * @file  ringtone_sys.h
 * @date  2010.03.11
 * @author  tamada GAME FREAK inc.
 * @brief   TVトランシーバー着信音制御
 */
//============================================================================================

#pragma once

#include <gflib.h>
#include "player_volume_fader.h"

typedef struct _RINGTONE_SYS RINGTONE_SYS;

extern RINGTONE_SYS * RINGTONE_SYS_Create( HEAPID heapID, PLAYER_VOLUME_FADER * fader );
extern void RINGTONE_SYS_Delete( RINGTONE_SYS * rtone );
extern void RINGTONE_SYS_Main( RINGTONE_SYS * rtone );
extern void RINGTONE_SYS_Play( RINGTONE_SYS * rtone );
extern void RINGTONE_SYS_Stop( RINGTONE_SYS * rtone );

