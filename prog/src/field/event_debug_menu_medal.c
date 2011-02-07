//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_menu_medal.c
 *	@brief  デバッグメニュー：メダル関連
 *	@author	onoue
 *	@data		2011.02.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
#include "font/font.naix"
#include "system/medaldata.h" // fro MEDALDATA

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "event_mapchange.h"

//アプリ
#include "savedata/shortcut.h"

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 
#include "event_gts.h"
#include "bsubway_scr.h"
#include "event_wifi_bsubway.h"
#include "event_bsubway.h"
#include "scrcmd_bsubway.h"
#include "net_app\wifi_country.h"
#include "savedata\medalbox.h"
#include "msg/debug/msg_debug_medal.h"

//外部公開
#include "event_debug_menu_ov_staff.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
// ===========================================================================
//
//    メダルボックスデバッグメニュー
//
// ===========================================================================

//-------------------------------------
/// メダルボックスデバッグワーク	
//=====================================
typedef struct {
  int seq_no;
  int sub_seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  GFL_ARCUTIL_TRANSINFO bgchar;
  FLDSYSWIN * win;
  GFL_MSGDATA * msgData;
  MedalID       draw_medal_id;
} DEBUG_MEDALBOX_WORK;

///メニューリスト
static const DEBUG_MENU_LIST MedalBoxMenuList[] =
{
  { DEBUG_STR_MEDALBOX_MENU_MAKEDATA , (void*)DEBUG_STR_MEDALBOX_MENU_MAKEDATA , GFL_OVERLAY_BLANK_ID },  // ボックスをランクアップ
  { DEBUG_STR_MEDALBOX_MENU_ALL_TITLE_OPEN , (void*)DEBUG_STR_MEDALBOX_MENU_ALL_TITLE_OPEN , GFL_OVERLAY_BLANK_ID },  // メダルボックスを開く
  { DEBUG_STR_MEDALBOX_MENU_ALL_HINT_OPEN , (void*)DEBUG_STR_MEDALBOX_MENU_ALL_HINT_OPEN , GFL_OVERLAY_BLANK_ID },  // メダルボックスを開く
  { DEBUG_STR_MEDALBOX_MENU_ALL_TITLE_ACHIVE , (void*)DEBUG_STR_MEDALBOX_MENU_ALL_TITLE_ACHIVE , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
  { DEBUG_STR_MEDALBOX_MENU_ALL_HINT_ACHIVE , (void*)DEBUG_STR_MEDALBOX_MENU_ALL_HINT_ACHIVE , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
  { DEBUG_STR_MEDALBOX_MENU_RANDOMSET , (void*)DEBUG_STR_MEDALBOX_MENU_RANDOMSET , GFL_OVERLAY_BLANK_ID },  // ボックスをランクアップ
  { DEBUG_STR_MEDALBOX_MENU_OUTSAVE , (void*)DEBUG_STR_MEDALBOX_MENU_OUTSAVE , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
  { DEBUG_STR_MEDALBOX_MENU_BOXRANK , (void*)DEBUG_STR_MEDALBOX_MENU_BOXRANK , GFL_OVERLAY_BLANK_ID },  // メダルデータをランダム出力
  { DEBUG_STR_MEDALBOX_MENU_OUTDATA , (void*)DEBUG_STR_MEDALBOX_MENU_OUTDATA , GFL_OVERLAY_BLANK_ID },  // メダルデータを全出力
};

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER MedalBoxMenuHeader =
{
  7,    //リスト項目数
  8,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

//-------------------------------------
///   メダルボックス初期化変数
//=====================================
static const DEBUG_MENU_INITIALIZER DebugMenuMedalBoxImitializer = {
  NARC_debug_message_debug_medal_dat,
  NELEMS(MedalBoxMenuList),
  MedalBoxMenuList,
  &MedalBoxMenuHeader,
  1, 1, 30, 16,
  NULL,
  NULL
};

// =======================
// プロトタイプ宣言
static GMEVENT_RESULT debugMenuMedalBoxEvent( GMEVENT *event, int *seq, void *wk );

//----------------------------------------------------------------------------
/**
 *	@brief  全タイトルメダル開放
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuAllTitleMedalOpen( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    // 設定前のメダルIDと開放状況を表示
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

    if( medal_state < MEDAL_STATE_TITLE_GET )
    {
      OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

      // タイトルメダルを開放
      
      // ヒントメダルを開放
      if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
      {
        MEDALBOX_SetTitleMedalAchive( medal_box , medal_id );
      }
      MEDALBOX_SetTitleMedalGetFlg( medal_box , medal_id , 99 , 12 , 31 );

      // 設定後の開放状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
      OS_Printf(" %u\n",medal_state);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  全ヒントメダル開放
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuAllHintMedalOpen( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , wk->heapID );
    MEDALDATA medal_data;
    GFL_ARC_LoadDataByHandle( p_handle , medal_id ,&medal_data );
    GFL_ARC_CloseDataHandle(p_handle);
    if( medal_data.hint_type != 0 )
    {
      // 設定前のメダルIDと開放状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

      if( medal_state < MEDAL_STATE_HINT_GET )
      {
        OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

        // ヒントメダルを開放
        if( medal_state < MEDAL_STATE_HINT_ACHIVE )
        {
          MEDALBOX_SetHintMedalAchive( medal_box , medal_id );
        }
        MEDALBOX_SetHintMedalGetFlg( medal_box , medal_id , GFL_STD_MtRand( 99 ) + 1 , GFL_STD_MtRand( 12 ) + 1 , GFL_STD_MtRand( 31 ) + 1 );

        // 設定後の開放状況を表示
        medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
        OS_Printf(" %u\n",medal_state);
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメダル全達成
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuTitleMedalAchive( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    // 設定前のタイトルメダルIDと達成状況を表示
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
    
    if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
    {
      OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

      // タイトルメダルを達成
      MEDALBOX_SetTitleMedalAchive( medal_box , medal_id );

      // 設定後のタイトル達成状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
      OS_Printf(" %u\n",medal_state);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメダル全達成
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuHintMedalAchive( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    // 設定前のヒントメダルIDと達成状況を表示
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

    if( medal_state < MEDAL_STATE_HINT_ACHIVE )
    { 
      OS_Printf("MEDALID %d: %u ->",medal_id , medal_state);

      // ヒントメダルを達成
      MEDALBOX_SetHintMedalAchive( medal_box , medal_id );

      // 設定後のヒント達成状況を表示
      medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
      OS_Printf(" %u\n",medal_state);
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータをデバッグプリント
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalDataOutPut( DEBUG_MEDALBOX_WORK* wk )
{
  int i =0;
  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , wk->heapID );

  for( i = 0; i < MDLID_MAX ; i++ )
  {
    MedalID medal_id =i;
    MEDALDATA medal_data;

    // アーカイブからデータを取得
    GFL_ARC_LoadDataByHandle( p_handle , medal_id ,&medal_data );
    OS_Printf( "=======================================================\n");
    OS_Printf( "メダルID %u\n",medal_id);
    OS_Printf( "target_count : %u\n" ,medal_data.target_count );
    OS_Printf( "cmp_type : %u\n" ,medal_data.cmp_type );
    OS_Printf( "cmp_id : %u\n" ,medal_data.cmp_id );
    OS_Printf( "graphic_type : %u\n" ,medal_data.graphic_type );
    OS_Printf( "hint_type : %u\n" ,medal_data.hint_type );
    OS_Printf( "hint_level : %u\n" ,medal_data.hint_level );
  }
  GFL_ARC_CloseDataHandle(p_handle);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスをランクアップ
 *
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalBoxRankUp( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );

  // メダルボックスをランクアップさせる
  MEDALBOX_RankUpMedalBox( medal_box );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルセーブをデバッグプリント
 *
 *	@param	wk ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalSaveOutPut( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    u8 y,m,d;
    u8 medal_state = 0;
    OS_Printf("MEDALID %d---------------------------------\n",medal_id);
    
    medal_state =  MEDALBOX_GetMedalState( medal_box , medal_id );
    OS_Printf("メダル状態     : ");
    // メダル状態
    switch( medal_state )
    {
    case MEDAL_STATE_NONE:
      OS_Printf("未取得\n");
      break;
    case MEDAL_STATE_HINT_ACHIVE:
      OS_Printf("ヒント達成\n");
      break;
    case MEDAL_STATE_HINT_GET:
      OS_Printf("ヒント取得\n");
      break;
    case MEDAL_STATE_TITLE_ACHIVE:
      OS_Printf("タイトル達成\n");
      break;
    case MEDAL_STATE_TITLE_GET:
      OS_Printf("タイトル取得\n");
      break;
    default:
      GF_ASSERT(0);
      break;
    }
    OS_Printf("新着           : %u\n", MEDALBOX_GetNewMedalFlg( medal_box , medal_id ) );
    MEDALBOX_GetDate( medal_box , medal_id , &y , &m , &d );
    OS_Printf("年月日         : 20%u年 %2u月 %u日\n",y,m,d);

  }
  OS_Printf("ボックスランク : %u\n", MEDALBOX_GetMedalBoxRank( medal_box) );
  OS_Printf("イチオシメダル : %u\n", MEDALBOX_GetPushMedalID( medal_box) );

}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void DrawMedalData( DEBUG_MEDALBOX_WORK* work , MedalID medal_id )
{
  STRBUF *str;
  STRBUF *exp;
  FLDMSGBG * msgBG;
  WORDSET * wset;
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( work->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state = MEDALBOX_GetMedalState( medal_box , work->draw_medal_id );

  str = GFL_MSG_CreateString( work->msgData, DEBUG_STR_MEDALBOX_MAKEDATA );
  wset = WORDSET_Create( work->heapID );
  exp  = GFL_STR_CreateBuffer( 128, work->heapID );
  WORDSET_RegisterMedalName( wset, 0,medal_id );
  WORDSET_RegisterNumber( wset, 1, medal_state, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wset, 2, medal_id, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( wset, exp, str );

  msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
  FLDSYSWIN_ClearWindow( work->win );
  FLDSYSWIN_PrintStrBuf( work->win, 0, 0, exp );

  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( exp );
  WORDSET_Delete( wset );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータ作成
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuMedalDataMake( DEBUG_MEDALBOX_WORK* work )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( work->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );

  switch( work->sub_seq_no )
  {
  case 0:
    {
      FLDMSGBG * msgBG;
      work->draw_medal_id = 0;
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA(work->fieldWork, NARC_debug_message_d_field_dat );
      msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->win = FLDSYSWIN_AddEx( msgBG, work->msgData , 1, 1, 30, 2 );
      DrawMedalData( work , work->draw_medal_id );
      work->sub_seq_no++;
    }
    break;
  case 1:
    {
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
      {
        work->draw_medal_id--;
        if( work->draw_medal_id < 0 )
        {
          work->draw_medal_id = MDLID_MAX-1;
        }
        DrawMedalData( work , work->draw_medal_id );
      }

      if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
      {
        work->draw_medal_id++;
        if( work->draw_medal_id >= MDLID_MAX )
        {
          work->draw_medal_id = 0;
        }
        DrawMedalData( work , work->draw_medal_id );
      }

      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        u8 medal_state = MEDALBOX_GetMedalState( medal_box , work->draw_medal_id );
        medal_state = medal_state + 1;

        if( medal_state >= MEDAL_STATE_MAX )
        {
          medal_state = 0;
        }
        DEBUG_SetMedalState( medal_box , work->draw_medal_id , medal_state );
        DrawMedalData( work , work->draw_medal_id );
      }
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
      {
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief      メダルデータのランダムセット
 *
 *	@param	wk  ワークデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debugMenuMedalDataRandomSet( DEBUG_MEDALBOX_WORK* wk )
{
  GAMEDATA *p_gm_data=GAMESYSTEM_GetGameData( wk->gmSys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gm_data );
  MEDALBOX* medal_box = SaveData_GetMedalBox( p_sv );
  u8 medal_state;
  MedalID medal_id;

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    u32 rand;

    // 乱数の取得
    rand = GFL_STD_MtRand( 5 );
    
    medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );

    // 設定されていないもののみランダムにせっていする
    if( medal_state != MEDAL_STATE_NONE )
    {
      continue;
    }

    switch( rand )
    {
    case 0:
      {

        // 設定前のメダルIDと開放状況を表示
        OS_Printf("タイトル開放 %d: %u ->",medal_id , medal_state);

        // タイトルメダルを開放
        MEDALBOX_SetTitleMedalAchive( medal_box , medal_id );
        //MEDALBOX_SetTitleMedalGetFlg( medal_box , medal_id , GFL_STD_MtRand( 99 ) + 1 , GFL_STD_MtRand( 12 ) + 1 , GFL_STD_MtRand( 31 ) + 1  );

        // 設定後の開放状況を表示
        medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
        OS_Printf(" %u\n",medal_state);
      }
      break;
    case 1:
      {
        ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , wk->heapID );
        MEDALDATA medal_data;
        GFL_ARC_LoadDataByHandle( p_handle , medal_id ,&medal_data );
        GFL_ARC_CloseDataHandle(p_handle);
        if( medal_data.hint_type != 0 )
        {
          // 設定前のメダルIDと開放状況を表示
          OS_Printf("ヒント開放   %d: %u ->",medal_id , medal_state);

          // ヒントメダルを開放
          MEDALBOX_SetHintMedalAchive( medal_box , medal_id );
          MEDALBOX_SetHintMedalGetFlg( medal_box , medal_id , GFL_STD_MtRand( 99 ) + 1 , GFL_STD_MtRand( 12 ) + 1 , GFL_STD_MtRand( 31 ) + 1 );

          // 設定後の開放状況を表示
          medal_state = MEDALBOX_GetMedalState( medal_box , medal_id );
          OS_Printf(" %u\n",medal_state);
        }
      }
      break;
    default:
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスメニュー起動
 *
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_MedalBox(DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_MEDALBOX_WORK *work;

  GMEVENT_Change( event, debugMenuMedalBoxEvent, sizeof(DEBUG_MEDALBOX_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MEDALBOX_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  return TRUE;
}

//--------------------------------------------------------------
/**
 * イベント：メダルボックス
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMedalBoxEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MEDALBOX_WORK *work = wk;

  switch( *seq ){

  // メニュー初期化
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuMedalBoxImitializer );
    work->sub_seq_no=0;
    (*seq)++;
    break;

  // メニュー選択処理
  case 1:
    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){

    // 操作無し
    case FLDMENUFUNC_NULL:
      break;

    // キャンセル
    case FLDMENUFUNC_CANCEL:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // タイトル全部開放
    case DEBUG_STR_MEDALBOX_MENU_ALL_TITLE_OPEN:
      {
        debugMenuAllTitleMedalOpen( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // ヒント全開放
    case DEBUG_STR_MEDALBOX_MENU_ALL_HINT_OPEN:
      {
        debugMenuAllHintMedalOpen( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // タイトルメダル全達成
    case DEBUG_STR_MEDALBOX_MENU_ALL_TITLE_ACHIVE:
      {
        debugMenuTitleMedalAchive( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // ヒントメダル全達成
    case DEBUG_STR_MEDALBOX_MENU_ALL_HINT_ACHIVE:
      {
        debugMenuHintMedalAchive( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルセーブデータをデバッグ出力
    case DEBUG_STR_MEDALBOX_MENU_OUTSAVE:
      {
        debugMenuMedalSaveOutPut( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルデータをデバッグ出力
    case DEBUG_STR_MEDALBOX_MENU_OUTDATA:     
      {
        debugMenuMedalDataOutPut( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // ボックスをランクアップ
    case DEBUG_STR_MEDALBOX_MENU_BOXRANK:     
      {
        debugMenuMedalBoxRankUp( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルセーブをランダム生成
    case DEBUG_STR_MEDALBOX_MENU_RANDOMSET:     
      {
        debugMenuMedalDataRandomSet( work );
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
      }
      return GMEVENT_RES_FINISH;

    // メダルセーブをランダム生成
    case DEBUG_STR_MEDALBOX_MENU_MAKEDATA:     
      {
        (*seq)++;
      }
      return GMEVENT_RES_CONTINUE;
    }
    break;
  case 2:
    {
      BOOL end = debugMenuMedalDataMake( work );
      if( end == TRUE )
      {
        FLDSYSWIN_ClearWindow( work->win );
        FLDSYSWIN_Delete( work->win );
        GFL_MSG_Delete( work->msgData );
        return GMEVENT_RES_FINISH;
      }
    }
    return GMEVENT_RES_CONTINUE;

  }

  return GMEVENT_RES_CONTINUE;
}

