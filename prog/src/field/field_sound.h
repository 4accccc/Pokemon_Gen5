///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief フィールドのサウンド制御
 * @file  field_sound.h
 * @author obata
 * @date   2009.12.25
 */
///////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gflib.h"
#include "gamesystem/gamedata_def.h"  // for GAMEDATA
#include "gamesystem/game_event.h"
#include "field_sound_proc.h"


//=================================================================================
// ■定数
//=================================================================================
// BGM 退避回数
typedef enum {
  FSND_PUSHCOUNT_NONE,   // 退避なし
  FSND_PUSHCOUNT_BASE,   // ベース BGM 退避中
  FSND_PUSHCOUNT_EVENT,  // イベント BGM 退避中
  FSND_PUSHCOUNT_OVER,   // 退避数オーバー
  FSND_PUSHCOUNT_MAX = FSND_PUSHCOUNT_OVER-1  // 最大退避数
} FSND_PUSHCOUNT;

// フェードイン速度
typedef enum{
  FSND_FADEIN_NONE,    // フェードなし
  FSND_FADEIN_FAST,    // 短
  FSND_FADEIN_NORMAL,  // 中
  FSND_FADEIN_SLOW,    // 長
  FSND_FADEIN_SPEED_NUM
} FSND_FADEIN_SPEED;

// フェードアウト速度
typedef enum{
  FSND_FADEOUT_NONE,    // フェードなし
  FSND_FADEOUT_FAST,    // 短
  FSND_FADEOUT_NORMAL,  // 中
  FSND_FADEOUT_SLOW,    // 長
  FSND_FADEOUT_SPEED_NUM
} FSND_FADEOUT_SPEED;
  

//=================================================================================
// ■BGM 操作イベント
//=================================================================================
// 即時再生
extern GMEVENT* EVENT_FieldSound_ForcePlayBGM( GAMESYS_WORK* gameSystem, u32 soundIdx );

// フェードイン / フェードアウト
extern GMEVENT* EVENT_FieldSound_FadeInBGM( GAMESYS_WORK* gameSystem, 
                                            FSND_FADEIN_SPEED fadeInSpeed );
extern GMEVENT* EVENT_FieldSound_FadeOutBGM( GAMESYS_WORK* gameSystem, 
                                             FSND_FADEOUT_SPEED fadeOutSpeed );

// 退避 / 復帰
extern GMEVENT* EVENT_FieldSound_PushBGM( GAMESYS_WORK* gameSystem, 
                                          FSND_FADEOUT_SPEED fadeOutSpeed );
extern GMEVENT* EVENT_FieldSound_PopBGM( GAMESYS_WORK* gameSystem, 
                                         FSND_FADEOUT_SPEED fadeOutSpeed, 
                                         FSND_FADEIN_SPEED fadeInSpeed );
extern GMEVENT* EVENT_FieldSound_AllPopBGM( GAMESYS_WORK* gameSystem, 
                                            FSND_FADEIN_SPEED fadeInSpeed );

// 再生中のBGMを退避し, 次のBGMを鳴らす
extern GMEVENT* EVENT_FieldSound_PushPlayEventBGM( GAMESYS_WORK* gameSystem, u32 soundIdx );

// ME 再生
extern GMEVENT* EVENT_FieldSound_PushPlayJingleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx ); 

// イベントBGM / フィールドBGM 再生
extern GMEVENT* EVENT_FieldSound_PlayEventBGM( GAMESYS_WORK* gameSystem, u32 soundIdx );
extern GMEVENT* EVENT_FieldSound_PlayFieldBGM( GAMESYS_WORK* gameSystem, u16 zoneID );

// BGM 変更
extern GMEVENT* EVENT_FieldSound_ChangeFieldBGM( GAMESYS_WORK* gameSystem, u16 zoneID );
extern GMEVENT* EVENT_FieldSound_StandByFieldBGM( GAMESYS_WORK* gameSystem, u16 zoneID );
extern GMEVENT* EVENT_FieldSound_PlayStartFieldBGM( GAMESYS_WORK* gameSystem );


//=================================================================================
// ■BGM 変更リクエスト
//=================================================================================
extern BOOL FIELD_SOUND_BGMChangeRequest( FIELD_SOUND* fieldSound, 
                                          u32 soundIdx, 
                                          FSND_FADEOUT_SPEED fadeOutSpeed, 
                                          FSND_FADEIN_SPEED fadeInSpeed );

extern BOOL FIELD_SOUND_FieldBGMChangeRequest( FIELD_SOUND* fieldSound,
                                               GAMEDATA* gameData, u16 zoneID );


//=================================================================================
// ■BGM 管理
//=================================================================================
extern void FIELD_SOUND_Main( FIELD_SOUND* fieldSound );


//=================================================================================
// ■取得
//=================================================================================
// フィールド BGM No.
extern u32 FIELD_SOUND_GetFieldBGM( GAMEDATA* gdata, u32 zoneID ); 

// トレーナー視線 BGM No.
extern u32 FIELD_SOUND_GetTrainerEyeBGM( u32 trType );

// BGM退避数の取得
extern FSND_PUSHCOUNT FIELD_SOUND_GetBGMPushCount( const FIELD_SOUND* fieldSound );

// フェード中かどうか
extern BOOL FIELD_SOUND_IsBGMFade( const FIELD_SOUND* fieldSound );
