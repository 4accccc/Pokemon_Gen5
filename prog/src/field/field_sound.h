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
//--------------------------------------------------------------
/// フィールドBGM退避回数
//
// フィールドBGM階層
//                    ____ME(jingle
//          ____EVENT(event,trainer...
//  ____BASE(field
//--------------------------------------------------------------
typedef enum
{
  FSND_PUSHCOUNT_NONE = 0, ///<退避なし
  FSND_PUSHCOUNT_BASEBGM = 1, ///<ベースBGM退避中
  FSND_PUSHCOUNT_EVENTBGM, ///<イベントBGM退避中
  FSND_PUSHCOUNT_OVER, ///<退避数オーバー
}FSND_PUSHCOUNT;

#define PLAY_NEXTBGM_FADEOUT_FRAME (60)
#define PLAY_NEXTBGM_FADEIN_FRAME (0)

//======================================================================
//  struct  
//======================================================================

//======================================================================
//  extern
//======================================================================
//フィールドBGM 再生
extern void FIELD_SOUND_PlayBGM( u32 bgmNo );
extern void FIELD_SOUND_PlayNextBGM_Ex( 
    FIELD_SOUND* fsnd, u32 bgmNo, u16 fadeOutFrame, u16 fadeInFrame );
#define FIELD_SOUND_PlayNextBGM( fsnd, bgmNo ) \
        FIELD_SOUND_PlayNextBGM_Ex( fsnd, bgmNo, \
            PLAY_NEXTBGM_FADEOUT_FRAME, PLAY_NEXTBGM_FADEIN_FRAME ) // 上記簡易版
extern void FIELD_SOUND_ChangePlayZoneBGM(
    FIELD_SOUND* fsnd, GAMEDATA* gdata, PLAYER_MOVE_FORM form, u32 zone_id );
extern void FIELD_SOUND_PushPlayEventBGM( FIELD_SOUND *fsnd, u32 bgmno );
extern void FIELD_SOUND_PushPlayJingleBGM( FIELD_SOUND *fsnd, u32 bgmno );
extern BOOL FIELD_SOUND_CanPlayBGM( const FIELD_SOUND *fsnd );

// フィールドBGM フェード管理
extern void FIELD_SOUND_UpdateBGM( FIELD_SOUND* fsnd );

//フィールドBGM フェード
extern void FIELD_SOUND_FadeInBGM( FIELD_SOUND* fsnd, u32 bgmNo, u16 frames );
extern void FIELD_SOUND_FadeInPopBGM( FIELD_SOUND* fsnd, u16 frames );
extern void FIELD_SOUND_FadeOutBGM( FIELD_SOUND* fsnd, u16 frames );
extern void FIELD_SOUND_FadeOutPushBGM( FIELD_SOUND* fsnd, u16 frames );
extern BOOL FIELD_SOUND_IsBGMFade( FIELD_SOUND* fsnd );

#if 0
//フィールドBGM トラック関連
extern void FIELD_SOUND_ChangeBGMTrackAction( void );
extern void FIELD_SOUND_ChangeBGMTrackStill( void );
extern void FIELD_SOUND_ChangeBGMVolume( int vol );
extern void FIELD_SOUND_ChangeBGMActionVolume( int vol );
#endif

//フィールドBGM BGMナンバー
extern u32 FIELD_SOUND_GetFieldBGMNo(
    GAMEDATA *gdata, PLAYER_MOVE_FORM form, u32 zone_id );

//トレーナー視線BGMナンバー
extern u32 FIELD_SOUND_GetTrainerEyeBgmNo( u32 trtype );

//フィールドBGM BGM退避、復帰
extern FSND_PUSHCOUNT FIELD_SOUND_GetBGMPushCount( const FIELD_SOUND *fsnd );
extern void FIELD_SOUND_PushBGM( FIELD_SOUND *fsnd );
extern void FIELD_SOUND_PopBGM( FIELD_SOUND *fsnd );
extern void FIELD_SOUND_ForcePopBGM( FIELD_SOUND *fsnd );
