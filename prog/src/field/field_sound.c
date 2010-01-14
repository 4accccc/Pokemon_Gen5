///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief フィールドのサウンド操作
 * @file  field_sound.c
 * @author obata
 * @date   2009.12.25
 */
///////////////////////////////////////////////////////////////////////////////////
#include "field_sound.h"
#include "sound/wb_sound_data.sadl"
#include "sound/pm_sndsys.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"
#include "field/zonedata.h"


//================================================================================= 
// ■定数
//================================================================================= 
#define FSND_PRINT_NO (1)  // printf出力先コンソール番号
#define FSND_BGM_NULL (0)  // BGM番号の無効値

// フェード フレーム数
static const u16 FadeInFrame [FSND_FADEIN_SPEED_NUM]  = {0, 9, 30, 45};
static const u16 FadeOutFrame[FSND_FADEOUT_SPEED_NUM] = {0, 9, 30, 45};

// 内部状態
typedef enum{
  FSND_STATE_STOP,              // 停止中
  FSND_STATE_PLAY,              // 再生中
  FSND_STATE_WAIT,              // 待機中
  FSND_STATE_FADE_OUT,          // BGMフェードアウト中
  FSND_STATE_FADE_IN,           // BGMフェードイン中
  FSND_STATE_PUSH,              // BGM退避中(フェードアウト完了待ち)
  FSND_STATE_POP_out,           // BGM復帰中(フェードアウト完了待ち)
  FSND_STATE_POP_in,            // BGM復帰中(pop実行待ち)
  FSND_STATE_CHANGE_out,        // BGM変更中(フェードアウト完了待ち)
  FSND_STATE_CHANGE_load,       // BGM変更中(分割ロード完了待ち)
  FSND_STATE_CHANGE_in,         // BGM変更中(フェードイン完了待ち)
  FSND_STATE_CHANGE_PUSH_out,   // BGM変更→退避中(フェードアウト完了待ち)
  FSND_STATE_CHANGE_PUSH_load,  // BGM変更→退避中(分割ロード完了待ち)
  FSND_STATE_STAND_BY_out,      // BGM再生準備中(フェードアウト完了待ち)
  FSND_STATE_STAND_BY_load,     // BGM再生準備中(分割ロード完了待ち)
  FSND_STATE_NUM
} FSND_STATE;

// リクエスト
typedef enum{
  FSND_BGM_REQUEST_NONE,        // リクエストなし
  FSND_BGM_REQUEST_FADE_IN,     // BGM フェードイン
  FSND_BGM_REQUEST_FADE_OUT,    // BGM フェードアウト
  FSND_BGM_REQUEST_PUSH,        // BGM 退避
  FSND_BGM_REQUEST_POP,         // BGM 復帰
  FSND_BGM_REQUEST_CHANGE,      // BGM 変更
  FSND_BGM_REQUEST_STAND_BY,    // BGM 再生準備
  FSND_BGM_REQUEST_FORCE_PLAY,  // 即時再生
  FSND_BGM_REQUEST_NUM
} FSND_BGM_REQUEST;

// 各内部状態に対する, リクエストの可不可テーブル
static BOOL RequestAcceptTable[ FSND_STATE_NUM ][ FSND_BGM_REQUEST_NUM ] = 
{
                        /* NONE    F_IN     F_OUT    PUSH     POP      CHANGE   S_BY     F_PLAY */
  /* STOP             */{  TRUE,   FALSE,   FALSE,   FALSE,    TRUE,    TRUE,    TRUE,    TRUE,  },
  /* PLAY             */{  TRUE,   FALSE,    TRUE,    TRUE,    TRUE,    TRUE,    TRUE,    TRUE,  },
  /* WAIT             */{  TRUE,    TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,    TRUE,  },
  /* FADE_OUT         */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* FALDE_IN         */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* PUSH             */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* POP_out          */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* POP_in           */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* CHANGE_out       */{  TRUE,   FALSE,   FALSE,    TRUE,   FALSE,    TRUE,   FALSE,   FALSE,  },
  /* CHANGE_load      */{  TRUE,   FALSE,   FALSE,    TRUE,   FALSE,    TRUE,   FALSE,   FALSE,  },
  /* CHANGE_in        */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,    TRUE,   FALSE,   FALSE,  },
  /* CHANGE_PUSH_out  */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* CHANGE_PUSH_load */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,  },
  /* STAND_BY_out     */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,    TRUE,   FALSE,  },
  /* STAND_BY_load    */{  TRUE,   FALSE,   FALSE,   FALSE,   FALSE,   FALSE,    TRUE,   FALSE,  },
};


//================================================================================= 
// ■フィールドサウンド管理ワーク
//================================================================================= 
struct _FIELD_SOUND
{
  FSND_STATE         state;  // 現在の状態
  FSND_BGM_REQUEST request;  // リクエスト
  u32           currentBGM;  // 現在再生中のBGM
  u32           requestBGM;  // リクエストBGM
  u32              loadBGM;  // 分割ロード対象BGM
  u32              loadSeq;  // 分割ロードのシーケンス
  u16          fadeInFrame;  // フェードイン フレーム数 
  u16         fadeOutFrame;  // フェードアウト フレーム数
  FSND_PUSHCOUNT pushCount;  // BGM退避数
  u32 pushBGM[FSND_PUSHCOUNT_MAX];  // 退避中のBGN
};


//================================================================================= 
// ■非公開関数
//================================================================================= 
// BGM 番号取得
static u32 GetZoneBGM( GAMEDATA* gameData, u32 zoneID );
// フェード フレーム数の取得
static u16 GetFadeInFrame( FSND_FADEIN_SPEED fadeInSpeed );
static u16 GetFadeOutFrame( FSND_FADEOUT_SPEED fadeOutSpeed );
// リクエストの発行
static void ThrowRequest_FADE_IN( FIELD_SOUND* fieldSound, u16 fadeInFrame );
static void ThrowRequest_FADE_OUT( FIELD_SOUND* fieldSound, u16 fadeOutFrame );
static void ThrowRequest_PUSH( FIELD_SOUND* fieldSound, u16 fadeOutFrame );
static void ThrowRequest_POP( FIELD_SOUND* fieldSound, u16 fadeOutFrame, u16 fadeInFrame );
static void ThrowRequest_CHANGE( FIELD_SOUND* fieldSound, u32 soundIdx, 
                                 u16 fadeOutFrame, u16 fadeInFrame );
static void ThrowRequest_STAND_BY( FIELD_SOUND* fieldSound, u32 soundIdx, u16 fadeOutFrame );
static void ThrowRequest_FORCE_PLAY( FIELD_SOUND* fieldSound, u16 soundIdx );
// リクエストを受理できるかどうかの判定
static BOOL CanAcceptRequest( const FIELD_SOUND* fieldSound, FSND_BGM_REQUEST request );
// リクエストに対する動作
static void RequestCheck_STOP            ( FIELD_SOUND* fieldSound );
static void RequestCheck_PLAY            ( FIELD_SOUND* fieldSound );
static void RequestCheck_WAIT            ( FIELD_SOUND* fieldSound );
static void RequestCheck_FADE_OUT        ( FIELD_SOUND* fieldSound );
static void RequestCheck_FADE_IN         ( FIELD_SOUND* fieldSound );
static void RequestCheck_PUSH            ( FIELD_SOUND* fieldSound );
static void RequestCheck_POP_out         ( FIELD_SOUND* fieldSound );
static void RequestCheck_POP_in          ( FIELD_SOUND* fieldSound );
static void RequestCheck_CHANGE_out      ( FIELD_SOUND* fieldSound );
static void RequestCheck_CHANGE_load     ( FIELD_SOUND* fieldSound );
static void RequestCheck_CHANGE_in       ( FIELD_SOUND* fieldSound );
static void RequestCheck_CHANGE_PUSH_out ( FIELD_SOUND* fieldSound );
static void RequestCheck_CHANGE_PUSH_load( FIELD_SOUND* fieldSound );
static void RequestCheck_STAND_BY_out    ( FIELD_SOUND* fieldSound );
static void RequestCheck_STAND_BY_load   ( FIELD_SOUND* fieldSound );
// 内部動作
static void Main_STOP            ( FIELD_SOUND* fieldSound );
static void Main_PLAY            ( FIELD_SOUND* fieldSound );
static void Main_WAIT            ( FIELD_SOUND* fieldSound );
static void Main_FADE_OUT        ( FIELD_SOUND* fieldSound );
static void Main_FADE_IN         ( FIELD_SOUND* fieldSound );
static void Main_PUSH            ( FIELD_SOUND* fieldSound );
static void Main_POP_out         ( FIELD_SOUND* fieldSound );
static void Main_POP_in          ( FIELD_SOUND* fieldSound );
static void Main_CHANGE_out      ( FIELD_SOUND* fieldSound );
static void Main_CHANGE_load     ( FIELD_SOUND* fieldSound );
static void Main_CHANGE_in       ( FIELD_SOUND* fieldSound );
static void Main_CHANGE_PUSH_out ( FIELD_SOUND* fieldSound );
static void Main_CHANGE_PUSH_load( FIELD_SOUND* fieldSound );
static void Main_STAND_BY_out    ( FIELD_SOUND* fieldSound );
static void Main_STAND_BY_load   ( FIELD_SOUND* fieldSound );
// 内部状態の変更
static void ChangeState( FIELD_SOUND* fieldSound, FSND_STATE nextState );
// リクエスト完了時の処理
static void RequestFinished( FIELD_SOUND* fieldSound );
// BGM操作
static void FadeInBGM ( FIELD_SOUND* fieldSound );
static void FadeOutBGM( FIELD_SOUND* fieldSound );
static void PushBGM( FIELD_SOUND* fieldSound );
static void PopBGM ( FIELD_SOUND* fieldSound );
static void DivLoadBGM_start( FIELD_SOUND* fieldSound );
static BOOL DivLoadBGM_load ( FIELD_SOUND* fieldSound );
static void ForcePlayBGM( FIELD_SOUND* fieldSound );


//================================================================================= 
// ■システム作成/破棄
//================================================================================= 

//---------------------------------------------------------------------------------
/**
 * @brief フィールドサウンド作成
 *
 * @param heapID ヒープID
 *
 * @return 作成したフィールドサウンド
 */
//---------------------------------------------------------------------------------
FIELD_SOUND* FIELD_SOUND_Create( HEAPID heapID )
{
  int i;
  FIELD_SOUND* fieldSound;
  fieldSound               = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_SOUND) );
  fieldSound->state        = FSND_STATE_STOP;
  fieldSound->request      = FSND_BGM_REQUEST_NONE;
  fieldSound->currentBGM   = FSND_BGM_NULL;
  fieldSound->requestBGM   = FSND_BGM_NULL;
  fieldSound->loadBGM      = FSND_BGM_NULL;
  fieldSound->fadeInFrame  = 0;
  fieldSound->fadeOutFrame = 0;
  fieldSound->pushCount    = FSND_PUSHCOUNT_NONE;
  for( i=0; i<FSND_PUSHCOUNT_MAX; i++ ){ fieldSound->pushBGM[i] = FSND_BGM_NULL; }
  return fieldSound;
}

//---------------------------------------------------------------------------------
/**
 * @brief フィールドサウンド破棄
 *
 * @param fieldSound フィールドサウンド
 */
//---------------------------------------------------------------------------------
void FIELD_SOUND_Delete( FIELD_SOUND* fieldSound )
{
  GFL_HEAP_FreeMemory( fieldSound );
}


//=================================================================================
// ■BGM 操作イベント
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
  u32             soundIdx;
  u16          fadeInFrame;
  u16         fadeOutFrame;

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
  case 0:  // 即時再生リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_FORCE_PLAY ) )
    {
      ThrowRequest_FORCE_PLAY( fieldSound, work->soundIdx );
      (*seq)++;
    }
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
GMEVENT* EVENT_FieldSound_ForcePlayBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, ForcePlayBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
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
  case 0:  // フェードイン リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_FADE_IN ) )
    {
      ThrowRequest_FADE_IN( fieldSound, work->fadeInFrame );
      (*seq)++;
    }
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
 * @param fadeInSpeed フェードイン速度
 * 
 * @return BGM フェードイン イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_FadeInBGM( GAMESYS_WORK* gameSystem, FSND_FADEIN_SPEED fadeInSpeed )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, FadeInBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeInFrame = GetFadeInFrame( fadeInSpeed );
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
  case 0:  // フェードアウト リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_FADE_OUT ) )
    {
      ThrowRequest_FADE_OUT( fieldSound, work->fadeOutFrame );
      (*seq)++;
    }
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
 * @param fadeOutSpeed フェードアウト速度
 * 
 * @return BGM フェードアウト イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_FadeOutBGM( GAMESYS_WORK* gameSystem, FSND_FADEOUT_SPEED fadeOutSpeed )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, FadeOutBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeOutFrame = GetFadeOutFrame( fadeOutSpeed );
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
  case 0:  // BGM退避リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_PUSH ) )
    {
      ThrowRequest_PUSH( fieldSound, work->fadeOutFrame );
      (*seq)++;
    }
    break;
  case 1:  // 退避完了待ち
    if( fieldSound->request == FSND_BGM_REQUEST_NONE )
    {
      (*seq)++;
    }
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
 * @param fadeOutSpeed フェードアウト速度
 * 
 * @return BGM 退避イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_PushBGM( GAMESYS_WORK* gameSystem, FSND_FADEOUT_SPEED fadeOutSpeed )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeOutFrame = GetFadeOutFrame( fadeOutSpeed );
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
  case 0:  // BGM復帰リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_POP ) )
    {
      ThrowRequest_POP( fieldSound, work->fadeOutFrame, work->fadeInFrame );
      (*seq)++;
    }
    break;
  case 1:  // 復帰BGMのフェード開始待ち
    if( (fieldSound->state == FSND_STATE_POP_in) ||
        (fieldSound->state == FSND_STATE_PLAY) )
    {
      (*seq)++;
    }
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
 * @param fadeOutSpeed 再生中のBGMのフェードアウト速度
 * @param fadeInSpeed  復帰したBGMのフェードイン速度
 * 
 * @return BGM 復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_PopBGM( GAMESYS_WORK* gameSystem, 
                                  FSND_FADEOUT_SPEED fadeOutSpeed, 
                                  FSND_FADEIN_SPEED fadeInSpeed )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PopBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeInFrame = GetFadeInFrame( fadeInSpeed );
  work->fadeOutFrame = GetFadeOutFrame( fadeOutSpeed );
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 全復帰イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT AllPopBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work; 
  FSND_PUSHCOUNT pushCount;
  u16 fadeInFrame;

  work = (FSND_EVWORK*)wk;
  pushCount = FIELD_SOUND_GetBGMPushCount( work->fieldSound );

  switch( *seq )
  {
  case 0:
    if( FSND_PUSHCOUNT_NONE < pushCount )
    {
      if( pushCount == FSND_PUSHCOUNT_BASE ){ fadeInFrame = work->fadeInFrame; }
      else{ fadeInFrame = GetFadeInFrame(FSND_FADEIN_NONE); }

      {
        u16 fadeOutFrame;
        GMEVENT* popEvent;
        
        fadeOutFrame = GetFadeOutFrame(FSND_FADEOUT_NONE);
        popEvent = EVENT_FieldSound_PopBGM( work->gameSystem, fadeOutFrame, fadeInFrame );
        GMEVENT_CallEvent( event, popEvent ); 
      }
    }
    else
    {
      (*seq)++;
    }
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
 * @param fadeInSpeed フェードイン速度
 * 
 * @return BGM 全復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_AllPopBGM( GAMESYS_WORK* gameSystem, FSND_FADEIN_SPEED fadeInSpeed )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, AllPopBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeInFrame = GetFadeInFrame( fadeInSpeed );
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief イベントBGM 再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayEventBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // BGM退避リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_PUSH ) )
    {
      ThrowRequest_PUSH( fieldSound, work->fadeOutFrame );
      (*seq)++;
    }
    break;
  case 1:  // BGM即時再生リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_FORCE_PLAY ) )
    {
      // BGM退避数オーバー
      if( FSND_PUSHCOUNT_BASE < fieldSound->pushCount )
      {
        OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't play event-BGM. (push count over!)\n" );
        GF_ASSERT(0);
      }

      ThrowRequest_FORCE_PLAY( fieldSound, work->soundIdx );
      (*seq)++;
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//---------------------------------------------------------------------------------
/**
 * @brief イベントBGM 再生イベント 取得
 *
 * @param gameSystem
 * @param soundIdx   再生するBGM
 * 
 * @return ME 再生イベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FieldSound_PushPlayEventBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayEventBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->soundIdx = soundIdx;
  work->fadeOutFrame = GetFadeOutFrame(FSND_FADEOUT_FAST);
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
  case 0:  // BGM退避リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_PUSH ) )
    {
      ThrowRequest_PUSH( fieldSound, work->fadeOutFrame );
      (*seq)++;
    }
    break;
  case 1:  // BGM即時再生リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_FORCE_PLAY ) )
    {
      // BGM退避数オーバー
      if( FSND_PUSHCOUNT_EVENT < fieldSound->pushCount )
      {
        OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't play jingle. (push count over!)\n" );
        GF_ASSERT(0);
      }

      ThrowRequest_FORCE_PLAY( fieldSound, work->soundIdx );
      (*seq)++;
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief ME 再生イベント 取得
 *
 * @param gameSystem
 * @param soundIdx   再生するME
 * 
 * @return ME 再生イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_PushPlayJingleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayJingleBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->soundIdx = soundIdx;
  work->fadeOutFrame = GetFadeOutFrame(FSND_FADEOUT_NONE);
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM変更イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT ChangeFieldBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // BGM変更リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_CHANGE ) )
    {
      ThrowRequest_CHANGE( fieldSound, work->soundIdx, work->fadeOutFrame, work->fadeInFrame );
      (*seq)++;
    }
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM変更イベント 取得
 *
 * @param gameSystem
 * @param gameData   
 * @param zoneID     BGM を再生する場所
 * 
 * @return BGM変更イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_ChangeFieldBGM( GAMESYS_WORK* gameSystem, u16 zoneID )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, ChangeFieldBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->soundIdx = FIELD_SOUND_GetFieldBGM( gameData, zoneID );
  work->fadeOutFrame = GetFadeOutFrame(FSND_FADEOUT_NORMAL);
  work->fadeInFrame = GetFadeInFrame(FSND_FADEIN_NONE);
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM再生準備イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT StandByFieldBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // BGM再生準備リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_STAND_BY ) )
    {
      ThrowRequest_STAND_BY( fieldSound, work->soundIdx, work->fadeOutFrame );
      (*seq)++;
    }
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM再生準備イベント 取得
 *
 * @param gameSystem
 * @param gameData   
 * @param zoneID     BGM を再生する場所
 * 
 * @return BGM再生準備イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_StandByFieldBGM( GAMESYS_WORK* gameSystem, u16 zoneID )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, StandByFieldBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->soundIdx = FIELD_SOUND_GetFieldBGM( gameData, zoneID );
  work->fadeOutFrame = GetFadeOutFrame(FSND_FADEOUT_NORMAL);
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM再生開始イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PlayStartFieldBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // すでに再生中なら, イベント終了
    if( fieldSound->state == FSND_STATE_PLAY )
    {
      return GMEVENT_RES_FINISH;
    }
    (*seq)++;
    break;
  case 1:  // BGM再生開始リクエスト発行
    if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_FADE_IN ) )
    {
      ThrowRequest_FADE_IN( fieldSound, work->fadeInFrame );
      (*seq)++;
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM再生開始イベント 取得
 *
 * @param gameSystem
 * 
 * @return BGM再生開始イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FieldSound_PlayStartFieldBGM( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PlayStartFieldBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeInFrame = GetFadeInFrame(FSND_FADEIN_NONE);
  return event;
}


//=================================================================================
// ■BGM 変更リクエスト
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief BGM 変更リクエスト発行 (BGM 直接指定)
 *
 * @param fieldSound   
 * @param soundIdx     再生する BGM
 * @param fadeOutSpeed フェードアウト速度
 * @param fadeInSpeed  フェードイン速度
 *
 * @return リクエストを受理した場合 TRUE
 *         そうでなければ           FALSE
 */
//---------------------------------------------------------------------------------
BOOL FIELD_SOUND_BGMChangeRequest( FIELD_SOUND* fieldSound, 
                                   u32 soundIdx, 
                                   FSND_FADEOUT_SPEED fadeOutSpeed, 
                                   FSND_FADEIN_SPEED fadeInSpeed )
{
  u16 fadeOutFrame, fadeInFrame;

  // フェード フレーム数を取得
  fadeOutFrame = GetFadeOutFrame( fadeOutSpeed );
  fadeInFrame = GetFadeInFrame( fadeInSpeed );

  // リクエスト発行
  if( CanAcceptRequest( fieldSound, FSND_BGM_REQUEST_CHANGE ) )
  { 
    ThrowRequest_CHANGE( fieldSound, soundIdx, fadeOutFrame, fadeInFrame );
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 変更リクエスト発行 (BGM 自動選択)
 *
 * @param fieldSound 
 * @param gameData   
 * @param zoneID     BGM を再生する場所
 *
 * @return リクエストを受理した場合 TRUE
 *         そうでなければ           FALSE
 */
//---------------------------------------------------------------------------------
BOOL FIELD_SOUND_FieldBGMChangeRequest( FIELD_SOUND* fieldSound, 
                                        GAMEDATA* gameData, u16 zoneID )
{
  u32 soundIdx;
  
  // BGMを決定
  soundIdx = FIELD_SOUND_GetFieldBGM( gameData, zoneID );
  
  return FIELD_SOUND_BGMChangeRequest( fieldSound, soundIdx, 
                                       FSND_FADEOUT_NORMAL, FSND_FADEIN_NONE );
}


//=================================================================================
// ■BGM 管理
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief BGMの状態を管理し, BGMの変更を行う
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
void FIELD_SOUND_Main( FIELD_SOUND* fieldSound )
{
  // リクエストに対する反応
  switch( fieldSound->state )
  {
  case FSND_STATE_STOP:              RequestCheck_STOP(fieldSound);              break;
  case FSND_STATE_PLAY:              RequestCheck_PLAY(fieldSound);              break;
  case FSND_STATE_WAIT:              RequestCheck_WAIT(fieldSound);              break;
  case FSND_STATE_FADE_OUT:          RequestCheck_FADE_OUT(fieldSound);          break;
  case FSND_STATE_FADE_IN:           RequestCheck_FADE_IN(fieldSound);           break;
  case FSND_STATE_PUSH:              RequestCheck_PUSH(fieldSound);              break;
  case FSND_STATE_POP_out:           RequestCheck_POP_out(fieldSound);           break;
  case FSND_STATE_POP_in:            RequestCheck_POP_in(fieldSound);            break;
  case FSND_STATE_CHANGE_out:        RequestCheck_CHANGE_out(fieldSound);        break;
  case FSND_STATE_CHANGE_load:       RequestCheck_CHANGE_load(fieldSound);       break;
  case FSND_STATE_CHANGE_in:         RequestCheck_CHANGE_in(fieldSound);         break;
  case FSND_STATE_CHANGE_PUSH_out:   RequestCheck_CHANGE_PUSH_out(fieldSound);   break;
  case FSND_STATE_CHANGE_PUSH_load:  RequestCheck_CHANGE_PUSH_load(fieldSound);  break;
  case FSND_STATE_STAND_BY_out:      RequestCheck_STAND_BY_out(fieldSound);      break;
  case FSND_STATE_STAND_BY_load:     RequestCheck_STAND_BY_load(fieldSound);     break;
  default: GF_ASSERT(0);
  }

  // 内部動作
  switch( fieldSound->state )
  {
  case FSND_STATE_STOP:              Main_STOP(fieldSound);              break;
  case FSND_STATE_PLAY:              Main_PLAY(fieldSound);              break;
  case FSND_STATE_WAIT:              Main_WAIT(fieldSound);              break;
  case FSND_STATE_FADE_OUT:          Main_FADE_OUT(fieldSound);          break;
  case FSND_STATE_FADE_IN:           Main_FADE_IN(fieldSound);           break;
  case FSND_STATE_PUSH:              Main_PUSH(fieldSound);              break;
  case FSND_STATE_POP_out:           Main_POP_out(fieldSound);           break;
  case FSND_STATE_POP_in:            Main_POP_in(fieldSound);            break;
  case FSND_STATE_CHANGE_out:        Main_CHANGE_out(fieldSound);        break;
  case FSND_STATE_CHANGE_load:       Main_CHANGE_load(fieldSound);       break;
  case FSND_STATE_CHANGE_in:         Main_CHANGE_in(fieldSound);         break;
  case FSND_STATE_CHANGE_PUSH_out:   Main_CHANGE_PUSH_out(fieldSound);   break;
  case FSND_STATE_CHANGE_PUSH_load:  Main_CHANGE_PUSH_load(fieldSound);  break;
  case FSND_STATE_STAND_BY_out:      Main_STAND_BY_out(fieldSound);      break;
  case FSND_STATE_STAND_BY_load:     Main_STAND_BY_load(fieldSound);     break;
  default: GF_ASSERT(0);
  }
}


//=================================================================================
// ■取得
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief フィールド BGM 再生するBGMナンバー取得
 *
 * @param gameData  ゲームデータ
 * @param zoneID    ゾーンID
 *
 * @return フィールドBGMナンバー
 */
//---------------------------------------------------------------------------------
u32 FIELD_SOUND_GetFieldBGM( GAMEDATA* gameData, u32 zoneID )
{
  PLAYER_WORK* player;
  PLAYER_MOVE_FORM form;

  // 自機フォーム取得
  player = GAMEDATA_GetMyPlayerWork( gameData );
  form   = PLAYERWORK_GetMoveForm( player );

  // なみのり
  if( form == PLAYER_MOVE_FORM_SWIM ){ return( SEQ_BGM_NAMINORI ); } 
  // 自転車
  if( form == PLAYER_MOVE_FORM_CYCLE ){ return( SEQ_BGM_BICYCLE ); }
  // それ以外なら, ゾーンのBGM
  return GetZoneBGM( gameData, zoneID );
}

#include "tr_tool/trtype_def.h"
#include "../../../resource/trtype_bgm/trtype_bgm.cdat" 
//---------------------------------------------------------------------------------
/**
 * @brief トレーナー視線曲 取得
 *
 * @param trType トレーナータイプ
 *
 * @return 指定したトレーナータイプの視線曲
 */
//---------------------------------------------------------------------------------
u32 FIELD_SOUND_GetTrainerEyeBGM( u32 trType )
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
  
  OS_TFPrintf( FSND_PRINT_NO, "ERROR:UNKNOW TRAINER TYPE BGM\n" );
  GF_ASSERT( 0 );
  return SEQ_BGM_EYE_01;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMプッシュ回数を取得
 *
 * @param fieldSound
 *
 * @return 積まれているBGMの数
 */
//---------------------------------------------------------------------------------
FSND_PUSHCOUNT FIELD_SOUND_GetBGMPushCount( const FIELD_SOUND* fieldSound )
{
  return fieldSound->pushCount;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMフェード検出
 *
 * @param fieldSound
 * 
 * @return フェード中なら TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
BOOL FIELD_SOUND_IsBGMFade( const FIELD_SOUND* fieldSound )
{
  if( (fieldSound->state == FSND_STATE_FADE_IN) ||
      (fieldSound->state == FSND_STATE_FADE_OUT) ||
      (fieldSound->state == FSND_STATE_POP_out) ||
      (fieldSound->state == FSND_STATE_POP_in) ||
      (fieldSound->state == FSND_STATE_PUSH) ||
      (fieldSound->state == FSND_STATE_CHANGE_out) ||
      (fieldSound->state == FSND_STATE_CHANGE_in) ||
      (fieldSound->state == FSND_STATE_CHANGE_PUSH_out) ||
      (fieldSound->state == FSND_STATE_STAND_BY_out) )
  {
    return TRUE;
  } 
  return FALSE;
}


//================================================================================= 
// ■非公開関数
//================================================================================= 

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
  u16 sound_idx;
  u8 season_id;

  season_id = GAMEDATA_GetSeasonID( gameData );
  sound_idx = ZONEDATA_GetBGMID( zoneID, season_id );
  return( sound_idx );
}

//---------------------------------------------------------------------------------
/**
 * @brief フェードイン フレーム数を取得する
 *
 * @param fadeInSpeed フェードイン速度
 *
 * @return 指定した速度のフェードイン フレーム数
 */
//---------------------------------------------------------------------------------
static u16 GetFadeInFrame( FSND_FADEIN_SPEED fadeInSpeed )
{
  return FadeInFrame[ fadeInSpeed ];
}

//---------------------------------------------------------------------------------
/**
 * @brief フェードアウト フレーム数を取得する
 *
 * @param fadeInSpeed フェードアウト速度
 *
 * @return 指定した速度のフェードアウト フレーム数
 */
//---------------------------------------------------------------------------------
static u16 GetFadeOutFrame( FSND_FADEOUT_SPEED fadeOutSpeed )
{
  return FadeOutFrame[ fadeOutSpeed ];
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_FADE_IN )
 *
 * @param fieldSound
 * @param fadeInFrame フェードイン フレーム数
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_FADE_IN( FIELD_SOUND* fieldSound, u16 fadeInFrame )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM fade in request\n" );
    GF_ASSERT(0);
    return;
  }

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_FADE_IN;
  fieldSound->fadeInFrame = fadeInFrame;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(FADE_IN)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_FADE_OUT )
 *
 * @param fieldSound
 * @param fadeOutFrame フェードアウト フレーム数
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_FADE_OUT( FIELD_SOUND* fieldSound, u16 fadeOutFrame )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM fade out request\n" );
    GF_ASSERT(0);
    return;
  }

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_FADE_OUT;
  fieldSound->fadeOutFrame = fadeOutFrame;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(FADE_OUT)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_PUSH )
 *
 * @param fieldSound
 * @param fadeOutFrame フェードアウト フレーム数
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_PUSH( FIELD_SOUND* fieldSound, u16 fadeOutFrame )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM push request\n" );
    GF_ASSERT(0);
    return;
  }

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_PUSH;
  fieldSound->fadeOutFrame = fadeOutFrame;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(PUSH)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_POP )
 *
 * @param fieldSound
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_POP( FIELD_SOUND* fieldSound,
                              u16 fadeOutFrame, u16 fadeInFrame )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM pop request\n" );
    GF_ASSERT(0);
    return;
  }

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_POP;
  fieldSound->fadeInFrame = fadeInFrame;
  fieldSound->fadeOutFrame = fadeOutFrame;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(POP)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_CHANGE )
 *
 * @param fieldSound
 * @param soundIdx     再生するBGM
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  再生するBGMのフェードイン フレーム数
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_CHANGE( FIELD_SOUND* fieldSound, u32 soundIdx, 
                                 u16 fadeOutFrame, u16 fadeInFrame )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM change request\n" );
    GF_ASSERT(0);
    return;
  } 
  // 再生中のBGMを指定
  if( fieldSound->currentBGM == soundIdx ){ return; } 

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_CHANGE;
  fieldSound->requestBGM = soundIdx;
  fieldSound->fadeInFrame = fadeInFrame;
  fieldSound->fadeOutFrame = fadeOutFrame;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(CHANGE)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_STAND_BY )
 *
 * @param fieldSound
 * @param soundIdx     再生の準備をするするBGM
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_STAND_BY( FIELD_SOUND* fieldSound, 
                                   u32 soundIdx, u16 fadeOutFrame )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM stand-by request\n" );
    GF_ASSERT(0);
    return;
  }
  // 再生中のBGMを指定
  if( fieldSound->currentBGM == soundIdx ){ return; } 

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_STAND_BY;
  fieldSound->requestBGM = soundIdx;
  fieldSound->fadeOutFrame = fadeOutFrame;
  
#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(STAND_BY)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストの発行 ( FSND_BGM_REQUEST_FORCE_PLAY )
 *
 * @param fieldSound
 * @param soundIdx   再生するBGM
 */
//---------------------------------------------------------------------------------
static void ThrowRequest_FORCE_PLAY( FIELD_SOUND* fieldSound, u16 soundIdx )
{
  // リクエストを受理できない
  if( CanAcceptRequest == FALSE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't Accept BGM force play request\n" );
    GF_ASSERT(0);
    return;
  }

  // リクエストを受理
  fieldSound->request = FSND_BGM_REQUEST_FORCE_PLAY;
  fieldSound->requestBGM = soundIdx;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: accept request(FORCE_PLAY)\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストを受理できるかどうかを判定する
 *
 * @param fieldSound
 * @param request    チェックするリクエスト
 *
 * @return 指定したリクエストを受理できる状態なら TRUE
 *         そうでなければ                         FALSE
 */
//---------------------------------------------------------------------------------
static BOOL CanAcceptRequest( const FIELD_SOUND* fieldSound, FSND_BGM_REQUEST request )
{
  return RequestAcceptTable[ fieldSound->state ][ request ];
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 STOP 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_STOP( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE:  
    break;
  case FSND_BGM_REQUEST_POP:
    PopBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_POP_in );
    break;
  case FSND_BGM_REQUEST_CHANGE:  
    DivLoadBGM_start( fieldSound );
    ChangeState( fieldSound, FSND_STATE_CHANGE_load );
    break;
  case FSND_BGM_REQUEST_STAND_BY:  
    ChangeState( fieldSound, FSND_STATE_STAND_BY_load );
    break;
  case FSND_BGM_REQUEST_FORCE_PLAY:
    ForcePlayBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_PLAY );
    RequestFinished( fieldSound );
    break;
  default:  
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 PLAY 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_PLAY( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_FADE_OUT: 
    FadeOutBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_FADE_OUT );
    break;
  case FSND_BGM_REQUEST_PUSH: 
    ChangeState( fieldSound, FSND_STATE_PUSH );
    break;
  case FSND_BGM_REQUEST_POP: 
    FadeOutBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_POP_out );
    break;
  case FSND_BGM_REQUEST_CHANGE: 
    FadeOutBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_CHANGE_out );
    break;
  case FSND_BGM_REQUEST_STAND_BY: 
    FadeOutBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_STAND_BY_out );
    break;
  case FSND_BGM_REQUEST_FORCE_PLAY:
    ForcePlayBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_PLAY );
    RequestFinished( fieldSound );
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 WAIT 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_WAIT( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_FADE_IN: 
    FadeInBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_FADE_IN );
    break;
  case FSND_BGM_REQUEST_FORCE_PLAY: 
    ForcePlayBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_PLAY );
    RequestFinished( fieldSound );
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 FADE_OUT 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_FADE_OUT( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_FADE_OUT: 
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 FADE_IN 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_FADE_IN( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_FADE_IN: 
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 PUSH 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_PUSH( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_PUSH:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 POP_out 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_POP_out( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_POP:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 POP_in 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_POP_in( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_POP:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_out 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_CHANGE_out( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_PUSH: 
    ChangeState( fieldSound, FSND_STATE_CHANGE_PUSH_out);
    break;
  case FSND_BGM_REQUEST_CHANGE:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_load 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_CHANGE_load( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_PUSH: 
    ChangeState( fieldSound, FSND_STATE_CHANGE_PUSH_load );
    break;
  case FSND_BGM_REQUEST_CHANGE:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_in 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_CHANGE_in( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_CHANGE:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_PUSH_out 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_CHANGE_PUSH_out ( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_PUSH:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_PUSH_load 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_CHANGE_PUSH_load( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_PUSH:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 STAND_BY_out 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_STAND_BY_out( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_STAND_BY:
    break;
  default:
    GF_ASSERT(0);
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 STAND_BY_load 時の, 各種リクエストに対する動作
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestCheck_STAND_BY_load( FIELD_SOUND* fieldSound )
{
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_NONE: 
    break;
  case FSND_BGM_REQUEST_STAND_BY:
    break;
  default:
    GF_ASSERT(0);
  }
}


//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 STOP 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_STOP( FIELD_SOUND* fieldSound )
{ 
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 PLAY 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_PLAY( FIELD_SOUND* fieldSound )
{
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 WAIT 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_WAIT( FIELD_SOUND* fieldSound )
{
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 FADE_OUT 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_FADE_OUT( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    ChangeState( fieldSound, FSND_STATE_WAIT );
    RequestFinished( fieldSound );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 FADE_IN 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_FADE_IN( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    ChangeState( fieldSound, FSND_STATE_PLAY );
    RequestFinished( fieldSound );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 PUSH 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_PUSH( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    PushBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_STOP );
    RequestFinished( fieldSound );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 POP_out 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_POP_out( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    PopBGM( fieldSound );
    FadeInBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_POP_in );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 POP_in 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_POP_in( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    ChangeState( fieldSound, FSND_STATE_PLAY );
    RequestFinished( fieldSound );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_out 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_CHANGE_out( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    DivLoadBGM_start( fieldSound );
    ChangeState( fieldSound, FSND_STATE_CHANGE_load );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_load 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_CHANGE_load( FIELD_SOUND* fieldSound )
{
  BOOL loadFinished;

  // 分割ロード実行
  loadFinished = DivLoadBGM_load( fieldSound );

  // 分割ロード完了(再生開始)
  if( loadFinished )
  { 
    // フェードイン開始
    FadeInBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_CHANGE_in );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_in 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_CHANGE_in( FIELD_SOUND* fieldSound )
{
  // フェードイン完了
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    // 再生開始以降に BGM変更リクエストを受けた場合
    if( fieldSound->requestBGM != FSND_BGM_NULL )
    {
      // フェードアウト開始
      FadeOutBGM( fieldSound );
      ChangeState( fieldSound, FSND_STATE_CHANGE_out );
    }
    else
    {
      ChangeState( fieldSound, FSND_STATE_PLAY );
      RequestFinished( fieldSound );
    }
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_PUSH_out 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_CHANGE_PUSH_out( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    DivLoadBGM_start( fieldSound );
    ChangeState( fieldSound, FSND_STATE_CHANGE_PUSH_load );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 CHANGE_PUSH_load 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_CHANGE_PUSH_load( FIELD_SOUND* fieldSound )
{
  BOOL loadFinished;

  // 分割ロード実行
  loadFinished = DivLoadBGM_load( fieldSound );

  // 分割ロード完了(再生開始)
  if( loadFinished )
  { 
    // BGM 退避
    PushBGM( fieldSound );
    ChangeState( fieldSound, FSND_STATE_STOP );
    RequestFinished( fieldSound );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 STAND_BY_out 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_STAND_BY_out( FIELD_SOUND* fieldSound )
{
  if( PMSND_CheckFadeOnBGM() == FALSE )
  {
    DivLoadBGM_start( fieldSound );
    ChangeState( fieldSound, FSND_STATE_STAND_BY_load );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態 STAND_BY_load 時の動作
 */
//---------------------------------------------------------------------------------
static void Main_STAND_BY_load( FIELD_SOUND* fieldSound )
{
  BOOL loadFinished;

  // 分割ロード実行
  loadFinished = DivLoadBGM_load( fieldSound );

  // 分割ロード完了(再生開始)
  if( loadFinished )
  { 
    // BGM一時停止
    PMSND_PauseBGM( TRUE );
    ChangeState( fieldSound, FSND_STATE_WAIT );
    RequestFinished( fieldSound );
  }
} 

//---------------------------------------------------------------------------------
/**
 * @brief 内部状態を変更する
 *
 * @param fieldSound 
 * @param nextState  遷移先の状態
 */
//---------------------------------------------------------------------------------
static void ChangeState( FIELD_SOUND* fieldSound, FSND_STATE nextState )
{
#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  switch( fieldSound->state )
  {
  case FSND_STATE_STOP:              OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (STOP ==> " );  break;
  case FSND_STATE_PLAY:              OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (PLAY ==> " );  break;
  case FSND_STATE_WAIT:              OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (WAIT ==> " );  break;
  case FSND_STATE_FADE_OUT:          OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (FADE OUT ==> " );  break;
  case FSND_STATE_FADE_IN:           OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (FADE IN ==> " );  break;
  case FSND_STATE_PUSH:              OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (PUSH ==> " );  break;
  case FSND_STATE_POP_out:           OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (POP out ==> " );  break;
  case FSND_STATE_POP_in:            OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (POP in ==> " );  break;
  case FSND_STATE_CHANGE_out:        OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (CHANGE out ==> " );  break;
  case FSND_STATE_CHANGE_load:       OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (CHANGE load ==> " );  break;
  case FSND_STATE_CHANGE_in:         OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (CHANGE in ==> " );  break;
  case FSND_STATE_CHANGE_PUSH_out:   OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (CHANGE PUSH out ==> " );  break;
  case FSND_STATE_CHANGE_PUSH_load:  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (CHANGE PUSH load ==> " );  break;
  case FSND_STATE_STAND_BY_out:      OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (STAND BY out ==> " );  break;
  case FSND_STATE_STAND_BY_load:     OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: change state (STAND BY load ==> " );  break;
  default: GF_ASSERT(0);
  }
  switch( nextState )
  {
  case FSND_STATE_STOP:              OS_TFPrintf( FSND_PRINT_NO, "STOP)\n" );  break;
  case FSND_STATE_PLAY:              OS_TFPrintf( FSND_PRINT_NO, "PLAY)\n" );  break;
  case FSND_STATE_WAIT:              OS_TFPrintf( FSND_PRINT_NO, "WAIT)\n" );  break;
  case FSND_STATE_FADE_OUT:          OS_TFPrintf( FSND_PRINT_NO, "FADE OUT)\n" );  break;
  case FSND_STATE_FADE_IN:           OS_TFPrintf( FSND_PRINT_NO, "FADE IN)\n" );  break;
  case FSND_STATE_PUSH:              OS_TFPrintf( FSND_PRINT_NO, "PUSH)\n" );  break;
  case FSND_STATE_POP_out:           OS_TFPrintf( FSND_PRINT_NO, "POP out)\n" );  break;
  case FSND_STATE_POP_in:            OS_TFPrintf( FSND_PRINT_NO, "POP in)\n" );  break;
  case FSND_STATE_CHANGE_out:        OS_TFPrintf( FSND_PRINT_NO, "CHANGE out)\n" );  break;
  case FSND_STATE_CHANGE_load:       OS_TFPrintf( FSND_PRINT_NO, "CHANGE load)\n" );  break;
  case FSND_STATE_CHANGE_in:         OS_TFPrintf( FSND_PRINT_NO, "CHANGE in)\n" );  break;
  case FSND_STATE_CHANGE_PUSH_out:   OS_TFPrintf( FSND_PRINT_NO, "CHANGE PUSH out)\n" );  break;
  case FSND_STATE_CHANGE_PUSH_load:  OS_TFPrintf( FSND_PRINT_NO, "CHANGE PUSH load)\n" );  break;
  case FSND_STATE_STAND_BY_out:      OS_TFPrintf( FSND_PRINT_NO, "STAND BY out)\n" );  break;
  case FSND_STATE_STAND_BY_load:     OS_TFPrintf( FSND_PRINT_NO, "STAND BY load)\n" );  break;
  default: GF_ASSERT(0);
  }
#endif

  // 更新
  fieldSound->state = nextState; 
}

//---------------------------------------------------------------------------------
/**
 * @brief リクエストに対する動作の完了処理
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void RequestFinished( FIELD_SOUND* fieldSound )
{
  // リクエストを持っていない
  if( fieldSound->request == FSND_BGM_REQUEST_NONE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Don't have BGM request\n" );
    GF_ASSERT(0);
    return;
  }

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  switch( fieldSound->request )
  {
  case FSND_BGM_REQUEST_FADE_IN:     OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(FADE IN) finished!!\n" );  break;
  case FSND_BGM_REQUEST_FADE_OUT:    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(FADE OUT) finished!!\n" );  break;
  case FSND_BGM_REQUEST_PUSH:        OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(PUSH) finished!!\n" );  break;
  case FSND_BGM_REQUEST_POP:         OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(POP) finished!!\n" );  break;
  case FSND_BGM_REQUEST_CHANGE:      OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(CHANGE) finished!!\n" );  break;
  case FSND_BGM_REQUEST_STAND_BY:    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(STAND BY) finished!!\n" );  break;
  case FSND_BGM_REQUEST_FORCE_PLAY:  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: request(FORCE_PLAY) finished!!\n" );  break;
  }
#endif

  // 内部情報を更新
  fieldSound->request = FSND_BGM_REQUEST_NONE; 
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMのフェードインを開始する
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void FadeInBGM( FIELD_SOUND* fieldSound )
{
  // BGMが鳴っていない
  if( fieldSound->currentBGM == FSND_BGM_NULL )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't fade in BGM (Don't playing BGM)\n" );
    GF_ASSERT(0);
    return;
  } 

  PMSND_FadeInBGM( fieldSound->fadeInFrame );
  PMSND_PauseBGM( FALSE );

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: BGM fade in start\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMのフェードアウトを開始する
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void FadeOutBGM( FIELD_SOUND* fieldSound )
{
  // BGMが鳴っていない
  if( fieldSound->currentBGM == FSND_BGM_NULL )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't fade out BGM (Don't playing BGM)\n" );
    GF_ASSERT(0);
    return;
  } 

  PMSND_FadeOutBGM( fieldSound->fadeOutFrame );

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: BGM fade out start\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMを退避する
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void PushBGM( FIELD_SOUND* fieldSound )
{
  // BGMが鳴っていない
  if( fieldSound->currentBGM == FSND_BGM_NULL )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't push BGM (Don't playing BGM)\n" );
    GF_ASSERT(0);
    return;
  } 
  // 退避数オーバー
  if( FSND_PUSHCOUNT_MAX <= fieldSound->pushCount )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't push BGM (Push count over)\n" );
    GF_ASSERT(0);
    return;
  }

  // 退避
  PMSND_PushBGM();

  // 内部情報を更新
  fieldSound->pushBGM[ fieldSound->pushCount ] = fieldSound->currentBGM;
  fieldSound->pushCount++;
  fieldSound->currentBGM = FSND_BGM_NULL; 

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: push BGM(%d)\n", fieldSound->pushBGM[ fieldSound->pushCount-1 ] );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMを復帰する
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void PopBGM( FIELD_SOUND* fieldSound )
{
  // 退避されていない
  if( fieldSound->pushCount <= FSND_PUSHCOUNT_NONE )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't pop BGM (There is no pushed BGM)\n" );
    GF_ASSERT(0);
    return;
  }

  // 復帰
  PMSND_PopBGM();

  // 内部情報を更新
  fieldSound->pushCount--;
  fieldSound->currentBGM = fieldSound->pushBGM[ fieldSound->pushCount ];

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: pop BGM(%d)\n", fieldSound->currentBGM );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMの分割ロードを開始する
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void DivLoadBGM_start( FIELD_SOUND* fieldSound )
{
  // 読み込むBGMが予約されていない
  if( fieldSound->requestBGM == FSND_BGM_NULL )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't start load BGM (Don't know target BGM)\n" );
    GF_ASSERT(0);
    return;
  }

  // 分割ロード開始
  PMSND_PlayBGMdiv( fieldSound->requestBGM, &(fieldSound->loadSeq), TRUE );

  // 内部情報を更新
  fieldSound->loadBGM = fieldSound->requestBGM;
  fieldSound->requestBGM = FSND_BGM_NULL;
  fieldSound->currentBGM = FSND_BGM_NULL;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: div load BGM(%d) start\n", fieldSound->loadBGM );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMの分割ロードを進める
 *
 * @param fieldSound
 *
 * @return 分割ロードが終了し, 再生が開始された場合 TRUE
 *         分割ロード未完了なら　FALSE
 */
//---------------------------------------------------------------------------------
static BOOL DivLoadBGM_load( FIELD_SOUND* fieldSound )
{
  BOOL loadFinished;

  // ロードが開始されていない
  if( fieldSound->loadBGM == FSND_BGM_NULL )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't load BGM (Don't know target BGM)\n" );
    GF_ASSERT(0); 
  }

  // 分割ロード実行
  loadFinished = PMSND_PlayBGMdiv( fieldSound->loadBGM, &(fieldSound->loadSeq), FALSE );

  // 分割ロード完了(再生開始)
  if( loadFinished )
  {
    // 内部情報を更新
    fieldSound->currentBGM = fieldSound->loadBGM;
    fieldSound->loadBGM = FSND_BGM_NULL;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: div load BGM(%d) finish\n", fieldSound->currentBGM );
#endif
  } 
  return loadFinished;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMの即時再生を行う
 *
 * @param fieldSound
 */
//---------------------------------------------------------------------------------
static void ForcePlayBGM( FIELD_SOUND* fieldSound )
{
  // 再生するBGMが指定されていない
  if( fieldSound->requestBGM == FSND_BGM_NULL )
  {
    OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: Can't force play BGM (Don't know target BGM)\n" );
    GF_ASSERT(0);
    return;
  }

  // 即時再生
  PMSND_PlayBGM( fieldSound->requestBGM );

  // 内部情報を更新
  fieldSound->currentBGM = fieldSound->requestBGM;
  fieldSound->requestBGM = FSND_BGM_NULL;
  fieldSound->loadBGM = FSND_BGM_NULL;

#ifdef DEBUG_ONLY_FOR_obata_toshihiro
  OS_TFPrintf( FSND_PRINT_NO, "FIELD-SOUND: force play BGM(%d) finish\n", fieldSound->currentBGM );
#endif
} 
