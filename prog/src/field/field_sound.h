///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief フィールドのサウンド制御 ( WB特化機能 )
 * @file   field_sound.h
 * @author obata
 * @date   2010.01.15
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gflib.h"
#include "gamesystem/gamesystem.h"    // for GAMESYS_WORK
#include "gamesystem/gamedata_def.h"  // for GAMEDATA
#include "gamesystem/game_event.h"    // for GMEVENT
#include "gamesystem/iss_sys.h"       // for ISS_SYS
#include "field_sound_proc.h"         // for FIELD_SOUND


//=================================================================================
// ■定数
//================================================================================= 
// フェード フレーム数
#define FSND_FADE_NONE    (0)  // 無
#define FSND_FADE_SHORT   (6)  // 短 
#define FSND_FADE_NORMAL (60)  // 中
#define FSND_FADE_LONG   (90)  // 長


//=================================================================================
// ■BGMの操作
//=================================================================================
extern GMEVENT* EVENT_FSND_ForcePlayBGM( GAMESYS_WORK* gameSystem, u16 soundIdx );
extern GMEVENT* EVENT_FSND_FadeInBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_FadeOutBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame );
extern GMEVENT* EVENT_FSND_PushBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame );
extern GMEVENT* EVENT_FSND_PopBGM( GAMESYS_WORK* gameSystem, 
                                   u16 fadeOutFrame, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_AllPopBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_PushPlayNextBGM( GAMESYS_WORK* gameSystem, u32 soundIdx, 
                                            u16 fadeOutFrame, u16 fadeInFrame );
extern GMEVENT* EVENT_FSND_PushPlayJingleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx );
extern GMEVENT* EVENT_FSND_ChangeBGM( GAMESYS_WORK* gameSystem, u32 soundIdx, 
                                      u16 fadeOutFrame, u16 fadeInFrame );

// ベースBGMを退避し, 戦闘曲を再生する
extern GMEVENT* EVENT_FSND_PushPlayBattleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx ); 
// 戦闘曲からベースBGMに復帰する
extern GMEVENT* EVENT_FSND_PopPlayBGM_fromBattle( GAMESYS_WORK* gameSystem );

// フェード操作の完了待ち
extern GMEVENT* EVENT_FSND_WaitBGMFade( GAMESYS_WORK* gameSystem );


//=================================================================================
// ■プレイヤーの操作により発生するBGMの変更
//=================================================================================
// ゾーン チェンジ
extern void FSND_ChangeBGM_byZoneChange( FIELD_SOUND* fieldSound, GAMEDATA* gameData, 
                                         u16 prevZoneID, u16 nextZoneID ); 

// マップ チェンジ
extern void FSND_StandByNextMapBGM( FIELD_SOUND* fieldSound, GAMEDATA* gameData, 
                                    u16 prevZoneID, u16 nextZoneID );

extern void FSND_PlayStartBGM( FIELD_SOUND* fieldSound );

// 自機移動フォーム変更
extern void FSND_ChangeBGM_byPlayerFormChange( FIELD_SOUND* fieldSound, 
                                               GAMEDATA* gameData, u16 zoneID ); 


//=================================================================================
// ■BGMボリューム操作
//=================================================================================
// フィールド ⇔ アプリ 遷移時
// ※ISSとプレイヤーボリュームを操作します。
extern void FSND_HoldBGMVolume_forApp   ( FIELD_SOUND* fieldSound, ISS_SYS* iss );
extern void FSND_ReleaseBGMVolume_fromApp( FIELD_SOUND* fieldSound, ISS_SYS* iss );

// アプリ内での操作
// ※プレイヤーボリュームのみを操作します。
extern void FSND_HoldBGMVolume_inApp   ( FIELD_SOUND* fieldSound );
extern void FSND_ReleaseBGMVolume_inApp( FIELD_SOUND* fieldSound );


//=================================================================================
// ■BGM No.の取得
//=================================================================================
// 指定したフィールド上で再生すべき BGM No.
extern u32 FSND_GetFieldBGM( GAMEDATA* gdata, u32 zoneID ); 

// トレーナー視線 BGM No.
extern u32 FSND_GetTrainerEyeBGM( u32 trType ); 


//=================================================================================
// ■環境音
//=================================================================================
// 環境音の再生
extern void FSND_PlayEnvSE( FIELD_SOUND* fieldSound, u32 soundIdx );
extern void FSND_PlayEnvSEVol( FIELD_SOUND* fieldSound, u32 soundIdx, u32 vol );
// 環境音のボリューム操作
extern void FSND_SetEnvSEVol( FIELD_SOUND* fieldSound, u32 soundIdx, u32 vol );
// 環境音の停止
extern void FSND_StopEnvSE( FIELD_SOUND* fieldSound, u32 soundIdx );
// 環境音の一時停止
extern void FSND_PauseEnvSE( FIELD_SOUND* fieldSound );
// 環境音の再始動
extern void FSND_RePlayEnvSE( FIELD_SOUND* fieldSound );

//=================================================================================
// ■TVトランシーバー着信音
//=================================================================================
extern void FSND_RequestTVTRingTone( FIELD_SOUND* fieldSound );
extern void FSND_StopTVTRingTone( FIELD_SOUND* fieldSound );

