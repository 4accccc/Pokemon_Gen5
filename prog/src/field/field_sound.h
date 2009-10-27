//======================================================================
/**
 * @file	field_sound.c
 * @brief	フィールドのサウンド関連
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"

#include "sound/pm_sndsys.h"

#include "field_sound_proc.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct  
//======================================================================

//======================================================================
//  extern
//======================================================================
//フィールドBGM 再生
extern void FIELD_SOUND_PlayBGM( u32 bgmNo );
extern void FIELD_SOUND_PlayNextBGM( u32 bgmNo );
extern void FIELD_SOUND_ChangePlayZoneBGM(
    GAMEDATA *gdata, PLAYER_MOVE_FORM form, u32 zone_id );
extern void FIELD_SOUND_PushPlayEventBGM( FIELD_SOUND *fsnd, u32 bgmno );
extern void FIELD_SOUND_PushPlayJingleBGM( FIELD_SOUND *fsnd, u32 bgmno );

//フィールドBGM フェード
extern void FIELD_SOUND_FadeInBGM( u16 frames );
extern void FIELD_SOUND_FadeOutBGM( u16 frames );
extern BOOL FIELD_SOUND_IsBGMFade();

//フィールドBGM トラック関連
extern void FIELD_SOUND_ChangeBGMTrackAction( void );
extern void FIELD_SOUND_ChangeBGMTrackStill( void );
extern void FIELD_SOUND_ChangeBGMVolume( int vol );
extern void FIELD_SOUND_ChangeBGMActionVolume( int vol );

//フィールドBGM BGMナンバー
extern u32 FIELD_SOUND_GetFieldBGMNo(
    GAMEDATA *gdata, PLAYER_MOVE_FORM form, u32 zone_id );

//トレーナー視線BGMナンバー
extern u32 FIELD_SOUND_GetTrainerEyeBgmNo( u32 trtype );

//フィールドBGM BGM退避、復帰
extern void FIELD_SOUND_PushBGM( FIELD_SOUND *fsnd );
extern void FIELD_SOUND_PopBGM( FIELD_SOUND *fsnd );
extern void FIELD_SOUND_ForcePopBGM( FIELD_SOUND *fsnd );
