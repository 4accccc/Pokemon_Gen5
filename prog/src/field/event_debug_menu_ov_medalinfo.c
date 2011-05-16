//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_menu_ov_medalinfo.c
 *	@brief  メダル詳細画面呼び出し
 *	@author	Masayuki Onoue
 *	@date		2011.05.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "app/medal_info.h"

#include <gflib.h>

#include "system/gfl_use.h"       // GFUser_VIntr_CreateTCB
#include "system/main.h"          // HEAPID
#include "arc_def.h"              // ARC_ID
#include "message.naix"           // MESSAGE_ARC_ID
#include "debug_message.naix"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "field_debug.h" 
#include "fieldmap.h"
#include "fieldmap/zone_id.h"
#include "event_mapchange.h"
#include "app/medal_info.h"
#include "script.h"
#include "event_fieldmap_control.h"

#include "../../../resource/fldmapdata/script/debug_scr_def.h"  //SCRID_DEBUG_MEDAL_INFO

// 外部公開
#include "event_debug_menu_ov_medalinfo.h"
#include "msg/debug/msg_debug_medalinfo.h"

// グローバル変数
extern int  g_debug_inf_tr_pos_type;
extern BOOL g_debug_inf_data_change;
extern u32  g_debug_inf_master_seed;
extern BOOL g_debug_inf_is_presentation;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
typedef struct {
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  GFL_ARCUTIL_TRANSINFO bgchar;
  FLDSYSWIN * win;
  GFL_MSGDATA * msgData;
  GAMEDATA *p_gamedata;
  MEDALINFO_INIT_PARAM mip;
  STRBUF     *p_tr_name;
} MEDALINFO_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void _SetMedalInfoParam( MEDALINFO_INIT_PARAM *p_info, STRBUF *p_trainer_name,
    MedalID medal_id, u16 medal_cnt, u8 box_rank,
    u8 get_year, u8 get_month ,u8 get_day,
    MEDALINFO_CALL_MODE call_mode ,HEAPID parent_heap_id );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

///メニューリスト
static const DEBUG_MENU_LIST MenuList[] =
{
  { DEBUG_STR_MEDALINFO_OPEN , (void*)DEBUG_STR_MEDALINFO_OPEN , GFL_OVERLAY_BLANK_ID },  // 詳細画面起動
};

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER MenuHeader =
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
///   メニュー初期化変数
//=====================================
static const DEBUG_MENU_INITIALIZER DebugMenuMedalInfoImitializer = {
  NARC_debug_message_debug_medalinfo_dat,
  NELEMS(MenuList),
  MenuList,
  &MenuHeader,
  1, 1, 30, 16,
  NULL,
  NULL
};

//----------------------------------------------------------------------------
/**
 *	@brief  メニュー選択
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMedalInfo( GMEVENT *event, int *seq, void *wk )
{
  MEDALINFO_WORK *work = wk;

  switch( *seq ){
    // メニュー初期化
  case 0:
    {
      work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuMedalInfoImitializer );
      work->p_tr_name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, HEAPID_PROC );
      (*seq)++;
    }
    break;

    // メニュー選択処理
  case 1:
    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){
      // 操作なし
    case FLDMENUFUNC_NULL:
      break;
      // キャンセル
    case FLDMENUFUNC_CANCEL:
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        GFL_STR_DeleteBuffer( work->p_tr_name );
      }
      return GMEVENT_RES_FINISH;
      // メダル詳細画面起動
    case DEBUG_STR_MEDALINFO_OPEN:
      {
        GMEVENT *p_ev; 

        // 名前の設定
        MYSTATUS  *p_my_status= GAMEDATA_GetMyStatus(work->p_gamedata);
        const STRCODE *my_name = MyStatus_GetMyName( p_my_status );
        GFL_STR_SetStringCode( work->p_tr_name ,my_name);
        
        // メニュー削除
        FLDMENUFUNC_DeleteMenu( work->menuFunc );

        // パラメータの作成
        _SetMedalInfoParam( &work->mip ,work->p_tr_name, 11 ,56 ,0 ,1 ,2 ,3 ,MEDALINFO_CALL_TOP ,GFL_HEAPID_APP );
        p_ev = EVENT_FieldSubProc(work->gmSys ,work->fieldWork ,FS_OVERLAY_ID(medalinfo), &medalInfo_ProcData, &work->mip);

        // イベント呼び出し
        GMEVENT_CallEvent( event ,p_ev);
        (*seq) = 2;
      }
      break;
    }
    break;
  case 2:
    GFL_STR_DeleteBuffer( work->p_tr_name );
    return GMEVENT_RES_FINISH;
  }

  return( GMEVENT_RES_CONTINUE );
}

//=============================================================================================
/**
 *  @brief メダル詳細画面初期化データを設定する
 *
 *	@param	*p_info         パラメータ格納先
 *	@param	*p_trainer_name トレーナー名
 *	@param	medal_id      メダルID
 *	@param	medal_cnt     所持しているメダル数
 *	@param	box_rank      メダルボックスのランク
 *	@param	get_year      取得年
 *	@param	get_month     取得月
 *	@param	get_day       取得日
 *	@param	parent_heap_id メモリ領域確保用ヒープID 
*
 *	@retval
 */
//=============================================================================================
static void _SetMedalInfoParam( MEDALINFO_INIT_PARAM *p_info, STRBUF *p_trainer_name,
    MedalID medal_id, u16 medal_cnt, u8 box_rank,
    u8 get_year, u8 get_month ,u8 get_day,
    MEDALINFO_CALL_MODE call_mode ,HEAPID parent_heap_id )
{
  p_info->p_trainer_name  = p_trainer_name;
  p_info->medal_id        = medal_id;
  p_info->medal_cnt       = medal_cnt;
  p_info->box_rank        = box_rank;
  p_info->get_year        = get_year;
  p_info->get_month       = get_month;
  p_info->get_day         = get_day;
  p_info->parent_heap_id  = parent_heap_id;
  p_info->call_mode       = call_mode;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル詳細画面デバッグメニュー起動
 *
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_MedalInfo(DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  MEDALINFO_WORK *work;

  GMEVENT_Change( event, debugMenuMedalInfo, sizeof(MEDALINFO_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(MEDALINFO_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->p_gamedata = GAMESYSTEM_GetGameData( gsys );

  return TRUE;
}

