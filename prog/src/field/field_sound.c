///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief フィールドのサウンド制御 ( WB特化機能 )
 * @file   field_sound.c
 * @author obata
 * @date   2010.01.15
 */
///////////////////////////////////////////////////////////////////////////////////
#include "field_sound_system.h"
#include "field_sound.h"
#include "sound/wb_sound_data.sadl"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"
#include "field/zonedata.h"
#include "tr_tool/trtype_def.h"
#include "../../../resource/trtype_bgm/trtype_bgm.cdat" 


//================================================================================= 
// ■非公開関数
//================================================================================= 
// 自機フォームの取得
static PLAYER_MOVE_FORM GetPlayerMoveForm( GAMEDATA* gameData );
// BGM 番号取得
static u32 GetZoneBGM( GAMEDATA* gameData, u32 zoneID );
static u32 GetFieldBGM( GAMEDATA* gameData, u32 zoneID );
static u32 GetZoneChangeBGM( GAMEDATA* gameData, u32 prevZoneID, u32 nextZoneID );
static u32 GetMapChangeBGM( GAMEDATA* gameData, u32 prevZoneID, u32 nextZoneID ); 


//=================================================================================
// ■BGM 操作
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief イベントワーク
 */
//---------------------------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK* gameSystem;
  FIELD_SOUND*  fieldSound;
  u32             soundIdx;  // 再生するBGM
  u16          fadeInFrame;  // 再生するBGMのフェードイン フレーム数
  u16         fadeOutFrame;  // 再生中のBGMのフェードアウト フレーム数

} FSND_EVWORK;


//---------------------------------------------------------------------------------
/**
 * @brief BGM 即時再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT ForcePlayBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 即時再生イベント 取得
 *
 * @param gameSystem
 * @param soundIdx 再生する BGM
 * 
 * @return BGM 即時再生イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_ForcePlayBGM( GAMESYS_WORK* gameSystem, u16 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, ForcePlayBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードイン イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT FadeInBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_IN( fieldSound, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードイン イベント 取得
 *
 * @param gameSystem
 * @param fadeInFrame フェードイン時間
 * 
 * @return BGM フェードイン イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_FadeInBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, FadeInBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードアウト処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT FadeOutBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, work->fadeOutFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードアウト イベント 取得
 *
 * @param gameSystem
 * @param fadeOutFrame フェードアウト時間
 * 
 * @return BGM フェードアウト イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_FadeOutBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, FadeOutBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 退避イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, work->fadeOutFrame );
    (*seq)++;
    break;
  case 1:  // 退避完了待ち
    if( FIELD_SOUND_HaveRequest(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 退避イベント 取得
 *
 * @param gameSystem
 * @param fadeOutFrame フェードアウト時間
 * 
 * @return BGM 退避イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PushBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeOutFrame = fadeOutFrame;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM復帰イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PopBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_POP( fieldSound, work->fadeOutFrame, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:  // BGM復帰完了待ち
    if( FIELD_SOUND_HaveRequest(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 2: 
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 復帰イベント 取得
 *
 * @param gameSystem
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return BGM 復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PopBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PopBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 全復帰イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT AllPopBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FIELD_SOUND* fieldSound;
  FSND_EVWORK* work; 
  FSND_PUSHCOUNT pushCount;
  u16 fadeInFrame;
  int i;


  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;
  pushCount = FIELD_SOUND_GetBGMPushCount( fieldSound );

  switch( *seq )
  {
  case 0:
    if( FSND_PUSHCOUNT_NONE < pushCount )
    {
      for( i=0; i<pushCount-1; i++ )
      {
        FIELD_SOUND_RegisterRequest_POP( fieldSound, FSND_FADE_NONE, FSND_FADE_NONE );
      }
      FIELD_SOUND_RegisterRequest_POP( fieldSound, FSND_FADE_NONE, work->fadeInFrame );
    }
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE; 
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 全復帰イベント 取得
 *
 * @param gameSystem
 * @param fadeInFrame フェードイン時間
 * 
 * @return BGM 全復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_AllPopBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, AllPopBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM 退避・再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayNextBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, work->fadeOutFrame );
    FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, work->soundIdx, 
                                        FSND_FADE_NONE, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//---------------------------------------------------------------------------------
/**
 * @brief BGM退避・再生イベント取得
 *
 * @param gameSystem
 * @param soundIdx     再生するBGM
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return 退避・再生イベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PushPlayNextBGM( GAMESYS_WORK* gameSystem, u32 soundIdx,
                                     u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayNextBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief ME 再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayJingleBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, FSND_FADE_NONE );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief ME 再生イベント 取得
 *
 * @param gameSystem
 * @param soundIdx 再生する ME
 * 
 * @return ME 再生イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PushPlayJingleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayJingleBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  work->fadeOutFrame = FSND_FADE_NONE;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM 変更イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT ChangeBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, work->soundIdx, 
                                        work->fadeOutFrame, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//---------------------------------------------------------------------------------
/**
 * @brief BGM 変更イベント取得
 *
 * @param gameSystem
 * @param soundIdx 再生する ME
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return 退避・再生イベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_ChangeBGM( GAMESYS_WORK* gameSystem, u32 soundIdx,
                               u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, ChangeBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief ベースBGM 退避イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayBattleBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    if( FIELD_SOUND_GetBGMPushCount(fieldSound) == FSND_PUSHCOUNT_NONE )
    { // ベースBGM再生中
      FIELD_SOUND_RegisterRequest_PUSH( fieldSound, FSND_FADE_SHORT );
    }
    else
    { // 視線曲再生中(ベースBGMはすでに退避済み)
      FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, FSND_FADE_SHORT );
    }
    (*seq)++;
    break;
  case 1:  // BGM退避orフェードアウト完了待ち
    if( FIELD_SOUND_HaveRequest(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 2:  // バトルBGM再生
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief ベースBGMの退避, 戦闘BGM再生イベント取得
 *
 * @param gameSystem
 * @param soundIdx   再生する戦闘BGM
 * 
 * @return BGM 退避イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PushPlayBattleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayBattleBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM フェード完了待ちイベント
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT WaitBGMFadeEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // フェード完了待ち
    if( FIELD_SOUND_IsBGMFade(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------------------------- 
/**
 * @brief BGMフェード完了待ちイベント取得
 *
 * @param gameSystem
 *
 * @return BGMフェード完了待ちイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_WaitBGMFade( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, WaitBGMFadeEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  return event;
}


//=================================================================================
// ■プレイヤーの操作により発生するBGMの変更
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief ゾーン チェンジによるBGMの変更
 *
 * @param fieldSound
 * @param gameData
 * @param prevZoneID チェンジ前のゾーンID
 * @param nextZoneID チェンジ後のゾーンID
 */
//---------------------------------------------------------------------------------
void FSND_ChangeBGM_byZoneChange( FIELD_SOUND* fieldSound, GAMEDATA* gameData, 
                                  u16 prevZoneID, u16 nextZoneID )
{
  u32 soundIdx;
  u16 fadeOutFrame, fadeInFrame;

  soundIdx = GetZoneChangeBGM( gameData, prevZoneID, nextZoneID ); 

  // フェード フレーム数を決定
  if( GetPlayerMoveForm(gameData) == PLAYER_MOVE_FORM_CYCLE )
  { // 自転車
    fadeOutFrame = FSND_FADE_NORMAL;
    fadeInFrame = FSND_FADE_NORMAL;
  }
  else
  { // 歩き
    fadeOutFrame = FSND_FADE_NORMAL;
    fadeInFrame = FSND_FADE_NONE;
  }

  // リクエスト登録
  FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, soundIdx, fadeOutFrame, fadeInFrame );
}

//---------------------------------------------------------------------------------
/**
 * @brief マップ チェンジ時のBGM再生準備
 *
 * @param fieldSound
 * @param gameData
 * @param prevZoneID チェンジ前のゾーンID
 * @param nextZoneID チェンジ後のゾーンID
 */
//---------------------------------------------------------------------------------
void FSND_StandByNextMapBGM( FIELD_SOUND* fieldSound, GAMEDATA* gameData, 
                             u16 prevZoneID, u16 nextZoneID )
{
  u32 soundIdx;

  soundIdx = GetMapChangeBGM( gameData, prevZoneID, nextZoneID ); 

  // リクエスト登録
  FIELD_SOUND_RegisterRequest_STAND_BY( fieldSound, soundIdx, FSND_FADE_NORMAL );
}

//---------------------------------------------------------------------------------
/**
 * @brief マップ チェンジ時のBGM再生開始
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
void FSND_PlayStartBGM( FIELD_SOUND* fieldSound )
{
  // リクエスト登録
  FIELD_SOUND_RegisterRequest_FADE_IN( fieldSound, FSND_FADE_NONE );
}

//---------------------------------------------------------------------------------
/**
 * @brief 自機移動フォームの変更によるBGM変更
 *
 * @param fieldSound
 * @param gameData
 * @param zoneID      現在のゾーンID
 */
//---------------------------------------------------------------------------------
void FSND_ChangeBGM_byPlayerFormChange( FIELD_SOUND* fieldSound, 
                                        GAMEDATA* gameData, u16 zoneID )
{
  u32 soundIdx;

  soundIdx = GetFieldBGM( gameData, zoneID );

  // リクエスト登録
  FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, soundIdx, FSND_FADE_LONG, FSND_FADE_NONE );
}


//=================================================================================
// ■BGM No.取得
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンでの再生すべきBGMを取得する
 *
 * @param gameData
 * @param zoneID   ゾーンID
 *
 * @return 指定したゾーンで再生すべきBGM No.
 */
//---------------------------------------------------------------------------------
u32 FSND_GetFieldBGM( GAMEDATA* gameData, u32 zoneID )
{
  return GetFieldBGM( gameData, zoneID );
}

//---------------------------------------------------------------------------------
/**
 * @brief ゾーンチェンジ時に再生すべきBGM No.
 *
 * @param gameData
 * @param prevZoneID チェンジ前のゾーンID
 * @param nextZoneID チェンジ後のゾーンID
 *
 * @return 指定したゾーン間をまたいだ時に再生すべきBGM
 */
//---------------------------------------------------------------------------------
u32 FSND_GetZoneChangeBGM( GAMEDATA* gdata, u32 prevZoneID, u32 nextZoneID )
{
  return GetZoneChangeBGM( gdata, prevZoneID, nextZoneID );
}

//---------------------------------------------------------------------------------
/**
 * @brief マップチェンジ時に再生すべきBGM No.
 *
 * @param gameData
 * @param prevZoneID チェンジ前のゾーンID
 * @param nextZoneID チェンジ後のゾーンID
 *
 * @return 指定したゾーン間のマップ切り替え時に再生すべきBGM
 */
//---------------------------------------------------------------------------------
u32 FSND_GetMapChangeBGM( GAMEDATA* gdata, u32 prevZoneID, u32 nextZoneID )
{
  return GetMapChangeBGM( gdata, prevZoneID, nextZoneID );
}

//---------------------------------------------------------------------------------
/**
 * @brief トレーナー視線曲 取得
 *
 * @param trType トレーナータイプ
 *
 * @return 指定したトレーナータイプの視線曲
 */
//---------------------------------------------------------------------------------
u32 FSND_GetTrainerEyeBGM( u32 trType )
{
  int i = 0;
  while( DATA_TrainerTypeToEyeBGMNo[i][0] != TRTYPE_MAX )
  {
    if( DATA_TrainerTypeToEyeBGMNo[i][0] == trType )
    {
      return( DATA_TrainerTypeToEyeBGMNo[i][1]  );
    }
    i++;
  }
  
  GF_ASSERT(0 && "FIELD-SOUND: error unknown trainer type\n");
  return SEQ_BGM_EYE_01;
} 


//=================================================================================
// ■非公開関数
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief 自機のフォームを取得する
 *
 * @param gameData
 *
 * @return 自機のフォーム
 */
//---------------------------------------------------------------------------------
static PLAYER_MOVE_FORM GetPlayerMoveForm( GAMEDATA* gameData )
{
  PLAYER_WORK* player;
  PLAYER_MOVE_FORM playerForm;

  player = GAMEDATA_GetMyPlayerWork( gameData );
  playerForm = PLAYERWORK_GetMoveForm( player );

  return playerForm;
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定ゾーンのBGMを取得
 *
 * @param gameData
 * @param zoneID ゾーンID
 *
 * @return 指定ゾーンのBGM
 */
//---------------------------------------------------------------------------------
static u32 GetZoneBGM( GAMEDATA* gameData, u32 zoneID )
{
  u32 soundIdx;
  u8 seasonID;

  seasonID = GAMEDATA_GetSeasonID( gameData );
  soundIdx = ZONEDATA_GetBGMID( zoneID, seasonID );

  return soundIdx;
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定ゾーンにおける, 再生すべきBGMを取得する
 *
 * @param gameData
 * @param zoneID
 *
 * @return 指定したゾーンで再生すべきBGM
 */
//---------------------------------------------------------------------------------
static u32 GetFieldBGM( GAMEDATA* gameData, u32 zoneID )
{
  u32 soundIdx;
  PLAYER_MOVE_FORM playerForm;
  FIELD_SOUND* fieldSound;

  playerForm = GetPlayerMoveForm( gameData );
  fieldSound = GAMEDATA_GetFieldSound( gameData );

  // なみのり
  if( playerForm == PLAYER_MOVE_FORM_SWIM ){ soundIdx = SEQ_BGM_NAMINORI; } 
  // 自転車
  else if( playerForm == PLAYER_MOVE_FORM_CYCLE ){ soundIdx = SEQ_BGM_BICYCLE; }
  // 歩き
  else{ soundIdx = GetZoneBGM( gameData, zoneID ); }

  return soundIdx;
}

//---------------------------------------------------------------------------------
/**
 * @brief ゾーンチェンジ時における, 再生すべきBGMを取得する
 *
 * @param gameData
 * @param prevZoneID チェンジ前のゾーンID
 * @param nextZoneID チェンジ後のゾーンID
 *
 * @return 指定したゾーンをまたいだ時に再生すべきBGM
 */
//---------------------------------------------------------------------------------
static u32 GetZoneChangeBGM( GAMEDATA* gameData, u32 prevZoneID, u32 nextZoneID )
{
  u32 soundIdx;
  PLAYER_MOVE_FORM playerForm;

  playerForm = GetPlayerMoveForm( gameData );

  // 自転車に乗っている
  if( playerForm == PLAYER_MOVE_FORM_CYCLE )
  {
    u32 prevZoneBGM, nextZoneBGM;

    prevZoneBGM = GetZoneBGM( gameData, prevZoneID );
    nextZoneBGM = GetZoneBGM( gameData, nextZoneID );

    if( prevZoneBGM == nextZoneBGM ){ soundIdx = SEQ_BGM_BICYCLE; }
    else{ soundIdx = nextZoneBGM; }
  }
  else
  {
    soundIdx = GetFieldBGM( gameData, nextZoneID );
  }

  return soundIdx;
}

//---------------------------------------------------------------------------------
/**
 * @brief マップチェンジ時における, 再生すべきBGMを取得する
 *
 * @param gameData
 * @param prevZoneID チェンジ前のゾーンID
 * @param nextZoneID チェンジ後のゾーンID
 *
 * @return 指定したゾーンをマップチェンジした時に再生すべきBGM
 */
//---------------------------------------------------------------------------------
static u32 GetMapChangeBGM( GAMEDATA* gameData, u32 prevZoneID, u32 nextZoneID )
{
  u32 soundIdx;

  soundIdx = GetZoneChangeBGM( gameData, prevZoneID, nextZoneID );

  // 次のマップが自転車に乗れない場所
  if( (soundIdx == SEQ_BGM_BICYCLE) &&
      (ZONEDATA_BicycleEnable(nextZoneID) == FALSE) )
  {
    soundIdx = GetZoneBGM( gameData, nextZoneID );
  }

  return soundIdx;
} 
