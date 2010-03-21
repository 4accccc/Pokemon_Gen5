/////////////////////////////////////////////////////////////////////////////
/**
 * @breif すれ違い調査隊 関連イベント
 * @file  event_research_team.c
 * @auhor obata
 * @date  2010.03.21
 */
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "event_research_team.h"

#include "gamesystem/gamesystem.h"   // for GAMESYSTEM
#include "gamesystem/game_data.h"    // for GAMEDATA
#include "savedata/save_control.h"   // for SAVE_CONTROL_WORK
#include "savedata/misc.h"           // for MISC
#include "field/field_msgbg.h"       // for FLDMSGBG, FLDMSGWIN
#include "field/research_team_def.h" // for RESEARCH_TEAM_RANK_xxxx
#include "print/wordset.h"           // for WORDSET

#include "scrcmd.h"   // for EVENT_WAIT_LAST_KEY
#include "fieldmap.h" // for FIELDMAP_xxxx

#include "arc/arc_def.h"             // for ARCID_xxxx
#include "arc/script_message.naix"   // for NARC_script_message_xxxx
#include "msg/script/msg_c03r0101.h" // for msg_c03r0101_xxxx


//===========================================================================
// ■定数
//===========================================================================
#define WINPOS_X   (1) // ウィンドウの表示x座標 ( キャラ単位 )
#define WINPOS_Y   (1) // ウィンドウの表示y座標 ( キャラ単位 )
#define WINSIZE_X (30) // ウィンドウの表示xサイズ ( キャラ単位 )
#define WINSIZE_Y (10) // ウィンドウの表示yサイズ ( キャラ単位 )


//===========================================================================
// ■イベントワーク
//===========================================================================
typedef struct
{
  HEAPID         heapID;
  GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
  FIELDMAP_WORK* fieldmap;
  FLDMSGBG*      fieldMsgBG;
  FLDMSGWIN*     fieldMsgWin;
  GFL_MSGDATA*   msgData;
  WORDSET*       wordset;

} EVENT_WORK;


//===========================================================================
// ■関数インデックス
//===========================================================================
static void InitEventWork( EVENT_WORK* work, GAMESYS_WORK* gameSystem ); // イベントワークを初期化する
static void CreateMsgData( EVENT_WORK* work ); // メッセージデータを生成する
static void DeleteMsgData( EVENT_WORK* work ); // メッセージデータを破棄する
static void CreateFldMsgWin( EVENT_WORK* work ); // メッセージウィンドウを生成する
static void DeleteFldMsgWin( EVENT_WORK* work ); // メッセージウィンドウを破棄する
static void CreateWordset( EVENT_WORK* work ); // ワードセットを生成する
static void DeleteWordset( EVENT_WORK* work ); // ワードセットを破棄する
static void SetupWinMsg( EVENT_WORK* work ); // ウィンドウに表示する文字列をセットアップする
static void ClearWinMsg( EVENT_WORK* work ); // ウィンドウの表示をクリアする
static int GetResearchTeamRank( const EVENT_WORK* work ); // 隊員ランクを取得する
static int GetCrossCount( const EVENT_WORK* work ); // すれ違い人数を取得する
static int GetThanksCount( const EVENT_WORK* work ); // お礼回数を取得する
static u32 GetStringID( int rank ); // 表示するメッセージIDを取得する


//---------------------------------------------------------------------------
/**
 * @brief プレイヤーの隊員情報表示イベント
 */
//---------------------------------------------------------------------------
static GMEVENT_RESULT DispResearchTeamInfoEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq ) {
  // メッセージを表示
  case 0:
    CreateMsgData( work );
    CreateFldMsgWin( work );
    CreateWordset( work );
    SetupWinMsg( work );
    (*seq)++;
    break;

  // ウィンドウの描画待ち
  case 1:
    if( FLDMSGWIN_CheckPrintTrans( work->fieldMsgWin ) == TRUE ) { (*seq)++; }
    break;

  // キー入力待ち
  case 2:
    if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ) { (*seq)++; }
    break;

  // イベント終了
  case 3:
    ClearWinMsg( work );
    DeleteWordset( work );
    DeleteFldMsgWin( work );
    DeleteMsgData( work );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//---------------------------------------------------------------------------
/**
 * @brief プレイヤーの隊員情報表示イベントを生成する
 *
 * @param gameSystem
 *
 * @return 生成したイベント
 */
//---------------------------------------------------------------------------
GMEVENT* EVENT_DispResearchTeamInfo( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // イベントを生成
  event = GMEVENT_Create( 
      gameSystem, NULL, DispResearchTeamInfoEvent, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  InitEventWork( work, gameSystem );

  return event;
} 


//---------------------------------------------------------------------------
/**
 * @brief イベントワークを初期化する
 *
 * @param work イベントワーク
 * @param gameSystem 
 */
//---------------------------------------------------------------------------
static void InitEventWork( EVENT_WORK* work, GAMESYS_WORK* gameSystem )
{
  FIELDMAP_WORK* fieldmap;

  fieldmap = GAMESYSTEM_GetFieldMapWork( gameSystem );

  // ゼロクリア
  GFL_STD_MemClear( work, sizeof(EVENT_WORK));

  // 初期化
  work->heapID      = FIELDMAP_GetHeapID( fieldmap );
  work->gameSystem  = gameSystem;
  work->gameData    = GAMESYSTEM_GetGameData( gameSystem );
  work->fieldmap    = fieldmap;
  work->fieldMsgBG  = FIELDMAP_GetFldMsgBG( fieldmap );
}

//---------------------------------------------------------------------------
/**
 * @brief メッセージデータを生成する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void CreateMsgData( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->msgData == NULL );

  work->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
                                  ARCID_SCRIPT_MESSAGE, 
                                  NARC_script_message_c03r0101_dat, 
                                  work->heapID );
}

//---------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void DeleteMsgData( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->msgData );

  GFL_MSG_Delete( work->msgData );
  work->msgData = NULL;
}

//---------------------------------------------------------------------------
/**
 * @brief メッセージウィンドウを生成する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void CreateFldMsgWin( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->msgData );
  GF_ASSERT( work->fieldMsgBG );
  GF_ASSERT( work->fieldMsgWin == NULL );

  work->fieldMsgWin = FLDMSGWIN_Add( work->fieldMsgBG, work->msgData, 
                                     WINPOS_X, WINPOS_Y, WINSIZE_X, WINSIZE_Y );
}

//---------------------------------------------------------------------------
/**
 * @brief メッセージウィンドウを破棄する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void DeleteFldMsgWin( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->fieldMsgWin );

  FLDMSGWIN_Delete( work->fieldMsgWin );
  work->fieldMsgWin = NULL;
} 

//---------------------------------------------------------------------------
/**
 * @brief ワードセットを生成する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void CreateWordset( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->wordset == NULL );

  work->wordset = WORDSET_Create( work->heapID );
}

//---------------------------------------------------------------------------
/**
 * @brief ワードセットを破棄する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void DeleteWordset( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->wordset );

  WORDSET_Delete( work->wordset );
  work->wordset = NULL;
}

//---------------------------------------------------------------------------
/**
 * @brief ウィンドウに表示する文字列をセットアップする
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static void SetupWinMsg( EVENT_WORK* work )
{
  int rank;
  int crossCount;
  int thanksCount;
  u32 strID;
  STRBUF* strbuf;
  STRBUF* strbuf_ex;

  GF_ASSERT( work );
  GF_ASSERT( work->msgData );
  GF_ASSERT( work->fieldMsgWin );

  // バッファを確保
  strbuf    = GFL_STR_CreateBuffer( 256, work->heapID );
  strbuf_ex = GFL_STR_CreateBuffer( 256, work->heapID );

  // 各種データを取得
  rank        = GetResearchTeamRank( work ); // 隊員ランク
  crossCount  = GetCrossCount( work ); // すれ違い人数
  thanksCount = GetThanksCount( work ); // お礼回数
  strID       = GetStringID( rank ); // メッセージID

  // 表示する文字列を作成
  { 
    // ワードセット登録
    WORDSET_RegisterPlayerName( 
        work->wordset, 0, GAMEDATA_GetMyStatus( work->gameData ) );
    WORDSET_RegisterNumber( 
        work->wordset, 1, crossCount, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( 
        work->wordset, 2, thanksCount, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );

    // ワードセット展開
    GFL_MSG_GetString( work->msgData, strID, strbuf );
    WORDSET_ExpandStr( work->wordset, strbuf_ex, strbuf ); 
    WORDSET_ClearAllBuffer( work->wordset );
  }

  // 作成した文字列を表示
  FLDMSGWIN_PrintStrBuf( work->fieldMsgWin, 1, 0, strbuf_ex );
  GFL_BG_LoadScreenReq( 
      GFL_BMPWIN_GetFrame( FLDMSGWIN_GetBmpWin( work->fieldMsgWin ) ) );

  // バッファを解放
  GFL_STR_DeleteBuffer( strbuf );
  GFL_STR_DeleteBuffer( strbuf_ex );
}

//---------------------------------------------------------------------------
/**
 * @brief ウィンドウの表示をクリアする
 *
 * @param work
 */
//---------------------------------------------------------------------------
static void ClearWinMsg( EVENT_WORK* work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->fieldMsgWin );

  FLDMSGWIN_ClearWindow( work->fieldMsgWin );
  GFL_BG_LoadScreenReq( 
      GFL_BMPWIN_GetFrame( FLDMSGWIN_GetBmpWin( work->fieldMsgWin ) ) );
}

//---------------------------------------------------------------------------
/**
 * @brief 隊員ランクを取得する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static int GetResearchTeamRank( const EVENT_WORK* work )
{
  const SAVE_CONTROL_WORK* save;
  const MISC* misc;

  GF_ASSERT( work );
  GF_ASSERT( work->gameData );

  save = GAMEDATA_GetSaveControlWorkConst( work->gameData );
  misc = SaveData_GetMiscConst( save );

  return MISC_CrossComm_GetResearchTeamRank( misc );
}

//---------------------------------------------------------------------------
/**
 * @brief すれ違い人数を取得する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static int GetCrossCount( const EVENT_WORK* work )
{
  const SAVE_CONTROL_WORK* save;
  const MISC* misc;

  GF_ASSERT( work );
  GF_ASSERT( work->gameData );

  save = GAMEDATA_GetSaveControlWorkConst( work->gameData );
  misc = SaveData_GetMiscConst( save );

  return MISC_CrossComm_GetSuretigaiCount( misc );
}

//---------------------------------------------------------------------------
/**
 * @brief お礼回数を取得する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------
static int GetThanksCount( const EVENT_WORK* work )
{
  const SAVE_CONTROL_WORK* save;
  const MISC* misc;

  GF_ASSERT( work );
  GF_ASSERT( work->gameData );

  save = GAMEDATA_GetSaveControlWorkConst( work->gameData );
  misc = SaveData_GetMiscConst( save );

  return MISC_CrossComm_GetThanksRecvCount( misc );
}

//---------------------------------------------------------------------------
/**
 * @brief 表示するメッセージIDを取得する
 *
 * @param rank 隊員ランク
 *
 * @return 指定した隊員ランクの情報を表示するためのメッセージID
 */
//---------------------------------------------------------------------------
static u32 GetStringID( int rank )
{
  switch( rank ) {
  case RESEARCH_TEAM_RANK_0: return msg_c03r0101_none_debug_01;
  case RESEARCH_TEAM_RANK_1: return msg_c03r0101_none_debug_02a;
  case RESEARCH_TEAM_RANK_2: return msg_c03r0101_none_debug_02b;
  case RESEARCH_TEAM_RANK_3: return msg_c03r0101_none_debug_02c;
  case RESEARCH_TEAM_RANK_4: return msg_c03r0101_none_debug_02d;
  case RESEARCH_TEAM_RANK_5: return msg_c03r0101_none_debug_02e;
  default: GF_ASSERT(0);
  }

  GF_ASSERT(0);
  return 0;
}
