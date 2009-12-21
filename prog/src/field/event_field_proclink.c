//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   event_field_proclink.c
 *  @brief  メニュー等から呼ばれるバッグなどのアプリケーションの繋がり
 *  @author Toru=Nagihashi
 *  @data   2009.10.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"

//フィールド
#include "field/field_msgbg.h"
#include "field/fieldmap.h"
#include "field/map_attr.h"
#include "field/itemuse_event.h"
#include "field/zonedata.h"
#include "fieldmap_ctrl.h"      //  レポートのイベント用
#include "fieldmap_ctrl_grid.h" //  レポートのイベント用
#include "field_player_grid.h"  //

//各プロセス
#include "event_fieldmap_control.h" //EVENT_FieldSubProc
#include "app/config_panel.h"   //ConfigPanelProcData
#include "app/trainer_card.h"   //TrainerCardSysProcData
#include "app/bag.h" // BAG_PARAM
#include "app/zukan.h" // 図鑑
#include "app/pokelist.h"   //PokeList_ProcData・PLIST_DATA
#include "app/p_status.h"   //PokeList_ProcData・PLIST_DATA
#include "app/townmap.h" //TOWNMAP_PARAM
#include "app/wifi_note.h" //
#include "app/mailtool.h" //MAIL_PARAM
#include "poke_tool/shinka_check.h" //進化デモ
#include "demo/shinka_demo.h" //進化デモ
#include "net_app/battle_recorder.h"  //バトルレコーダー

//アーカイブ
#include "message.naix"
#include "msg/msg_fldmapmenu.h"

//その他
#include "item/item.h"  //ITEM_GetMailDesign

//自分の外部公開
#include "field/event_field_proclink.h"

//-------------------------------------
/// 各アプリのオーバーレイ
//=====================================
FS_EXTERN_OVERLAY(poke_status);
FS_EXTERN_OVERLAY(townmap);
FS_EXTERN_OVERLAY(wifinote);
FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(app_mail);
FS_EXTERN_OVERLAY(battle_recorder);

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================
//-------------------------------------
/// RETURN関数の戻り値
//=====================================
typedef enum
{
  RETURNFUNC_RESULT_RETURN,     // メニューに戻る
  RETURNFUNC_RESULT_EXIT,       // メニューも抜けて歩ける状態まで戻る
  RETURNFUNC_RESULT_USE_SKILL,  // メニューを抜けて技を使う
  RETURNFUNC_RESULT_USE_ITEM,   // メニュを抜けてアイテムを使う
  RETURNFUNC_RESULT_NEXT,       // 次のプロセスへ行く
} RETURNFUNC_RESULT;

typedef enum
{
  PROCLINK_MODE_LIST_TO_MAIL_CREATE,  //手紙を持たせる→作成画面
  PROCLINK_MODE_LIST_TO_MAIL_VIEW,    //手紙を見る→View画面
  PROCLINK_MODE_BAG_TO_MAIL_CREATE,   //手紙を持たせる→作成画面
  PROCLINK_MODE_EVOLUTION_ITEM,       //アイテム進化(○○の石
  PROCLINK_MODE_EVOLUTION_LEVEL,      //レベルアップ進化(ふしぎなアメ
  PROCLINK_MODE_WAZAWASURE_MACHINE,   //リスト→技忘れ(技マシン
  PROCLINK_MODE_WAZAWASURE_LVUP,      //リスト→技忘れ(ふしぎなアメ

}PROCLINK_TAKEOVER_MODE;  //PROC変更の際持ち越せない値保存用
//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================
//-------------------------------------
/// 関数型定義
//
//=====================================
typedef struct _PROCLINK_WORK PROCLINK_WORK;
typedef void (* PROCLINK_EVENT_FUNC)( PROCLINK_WORK* wk, u32 param );
typedef void * (* PROCLINK_CALL_PROC_FUNC)( PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
typedef RETURNFUNC_RESULT (* PROCLINK_RETURN_PROC_FUNC)( PROCLINK_WORK *wk, void *param_adrs );

//-------------------------------------
/// 開き閉じる関数
//=====================================
typedef struct 
{
  EVENT_PROCLINK_MENUOPEN_FUNC  open_func;    //メニュー等を開く関数    (NULLで行わない)
  EVENT_PROCLINK_MENUCLOSE_FUNC close_func;   //メニュー等を閉じる関数  (NULLで行わない)
  void *wk_adrs;                              //コールバックに渡す引数
  const EVENT_PROCLINK_PARAM *param;          //引数
} PROCLINK_CALLBACK_WORK;

//-------------------------------------
/// メインワーク
//=====================================
struct _PROCLINK_WORK
{
  HEAPID                    heapID;

  EVENT_PROCLINK_CALL_TYPE  now_type;     //現在選択されているプロセス
  EVENT_PROCLINK_CALL_TYPE  pre_type;     //一つ前の選択されているプロセス（初期ならば↑と同じ）
  EVENT_PROCLINK_CALL_TYPE  next_type;    //次のロセス
  RETURNFUNC_RESULT         result;       //終了方法

  GMEVENT                   *event;       //イベント

  EVENT_PROCLINK_PARAM      *param;       //引数
  void                      *proc_param;  //プロセスのワーク

  PROCLINK_CALLBACK_WORK    callback; //イベント用にメニューなどを閉じたり開いたりする関数のコールバック構造体

  //アプリ間で引継ぎが必要なパラメータ
  ITEMCHECK_WORK            icwk;     //アイテム使用時に検査する情報が含まれている
  FLDSKILL_CHECK_WORK       scwk;     //フィールドスキルが使用可能がどうかの情報
  u8                        sel_poke; //メール画面で引き継げないので用意
  u8                        item_no;  //メール画面で引き継げないので用意
  PROCLINK_TAKEOVER_MODE    mode;     //メール画面で引き継げないので用意
};


//=============================================================================
/**
 *        プロトタイプ
 */
//=============================================================================
//-------------------------------------
/// イベント
//=====================================
static GMEVENT_RESULT ProcLinkEvent( GMEVENT *event, int *seq, void *wk_adrs );

//-------------------------------------
/// FUNCITON
//=====================================
static void PROCLINK_CALLBACK_Set( PROCLINK_CALLBACK_WORK *wk, const EVENT_PROCLINK_PARAM *param, EVENT_PROCLINK_MENUOPEN_FUNC  open_func, EVENT_PROCLINK_MENUCLOSE_FUNC close_func, void *wk_adrs );
static void PROCLINK_CALLBACK_Open( PROCLINK_CALLBACK_WORK *wk );
static void PROCLINK_CALLBACK_Close( PROCLINK_CALLBACK_WORK *wk );

//-------------------------------------
/// 各PROCごとのCALL関数とRETURN関数
//=====================================
//ポケリスト
static void * FMenuCallProc_PokeList(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_PokeList(PROCLINK_WORK* wk,void* param_adrs);
//図鑑
//static void FMenuEvent_Zukan( PROCLINK_WORK* wk, u32 param );
static void * FMenuCallProc_Zukan(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_Zukan(PROCLINK_WORK* wk,void* param_adrs);
//ステータス
static void * FMenuCallProc_PokeStatus(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_PokeStatus(PROCLINK_WORK* wk,void* param_adrs);
//バッグ
static void * FMenuCallProc_Bag(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_Bag(PROCLINK_WORK* wk,void* param_adrs);
//トレーナーカード
static void * FMenuCallProc_TrainerCard(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_TrainerCard(PROCLINK_WORK* wk,void* param_adrs);
//タウンマップ
static void * FMenuCallProc_TownMap(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_TownMap(PROCLINK_WORK* wk,void* param_adrs);
//コンフィグ
static void * FMenuCallProc_Config(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_Config(PROCLINK_WORK* wk,void* param_adrs);
//レポート
static void FMenuEvent_Report( PROCLINK_WORK* wk, u32 param );
//WiFiノート
static void * FMenuCallProc_WifiNote(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_WifiNote(PROCLINK_WORK* wk,void* param_adrs);
//メール画面
static void * FMenuCallProc_Mail(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_Mail(PROCLINK_WORK* wk,void* param_adrs);
//進化画面
static void * FMenuCallProc_Evolution(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_Evolution(PROCLINK_WORK* wk,void* param_adrs);
//バトルレコーダー
static void * FMenuCallProc_BattleRecorder(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs );
static RETURNFUNC_RESULT FMenuReturnProc_BattleRecorder(PROCLINK_WORK* wk,void* param_adrs);

//-------------------------------------
/// レポートと図鑑のイベント
//=====================================
static GMEVENT * createFMenuMsgWinEvent(
  GAMESYS_WORK *gsys, u32 heapID, u32 strID, FLDMSGBG *msgBG );
static GMEVENT_RESULT FMenuMsgWinEvent( GMEVENT *event, int *seq, void *wk );

static GMEVENT * createFMenuReportEvent(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u32 heapID, FLDMSGBG *msgBG );
static GMEVENT_RESULT FMenuReportEvent( GMEVENT *event, int *seq, void *wk );


//=============================================================================
/**
 *        データ
 */
//=============================================================================
//-------------------------------------
/// 呼び出されるアプリテーブル
//  EVENT_PROCLINK_CALL_TYPEの列挙と
//=====================================
static const struct 
{
  FSOverlayID               ovId;       //オーバーレイID
  const GFL_PROC_DATA       *proc_data; //プロセス
  PROCLINK_CALL_PROC_FUNC   call_func;    //開始前関数
  PROCLINK_RETURN_PROC_FUNC return_func;  //終了時関数
  PROCLINK_EVENT_FUNC       event_func;   //アプリがイベントだった場合
} ProcLinkData[ EVENT_PROCLINK_CALL_MAX ] =
{ 

  //EVENT_PROCLINK_CALL_POKELIST,
  { 
    FS_OVERLAY_ID(pokelist) , 
    &PokeList_ProcData ,
    FMenuCallProc_PokeList ,
    FMenuReturnProc_PokeList ,
    NULL,
  },
  //EVENT_PROCLINK_CALL_ZUKAN,
  { 
    FS_OVERLAY_ID(zukan),
    &ZUKAN_ProcData,
    FMenuCallProc_Zukan,
    FMenuReturnProc_Zukan,
//    FMenuEvent_Zukan,
    NULL,
  },
  //EVENT_PROCLINK_CALL_BAG,          
  { 
    FS_OVERLAY_ID(bag) , 
    &ItemMenuProcData  ,
    FMenuCallProc_Bag,
    FMenuReturnProc_Bag,
    NULL,
  },
  //EVENT_PROCLINK_CALL_TRAINERCARD,  
  { 
    TRCARD_OVERLAY_ID, 
    &TrCardSysProcData ,
    FMenuCallProc_TrainerCard,
    FMenuReturnProc_TrainerCard,
    NULL,
  },
  //EVENT_PROCLINK_CALL_REPORT
  { 
    0 , NULL , NULL, NULL, 
    FMenuEvent_Report,
  },
  //EVENT_PROCLINK_CALL_CONFIG,       
  { 
    FS_OVERLAY_ID(config_panel) , 
    &ConfigPanelProcData ,
    FMenuCallProc_Config,
    FMenuReturnProc_Config,
    NULL,
  },

  //EVENT_PROCLINK_CALL_STATUS
  { 
    FS_OVERLAY_ID(poke_status),
    &PokeStatus_ProcData,
    FMenuCallProc_PokeStatus,
    FMenuReturnProc_PokeStatus,
    NULL,
  },

  //EVENT_PROCLINK_CALL_TOWNMAP
  { 
    FS_OVERLAY_ID(townmap),
    &TownMap_ProcData,
    FMenuCallProc_TownMap,
    FMenuReturnProc_TownMap,
    NULL,
  },

  //EVENT_PROCLINK_CALL_WIFINOTE
  { 
    FS_OVERLAY_ID(wifinote),
    &WifiNoteProcData,
    FMenuCallProc_WifiNote,
    FMenuReturnProc_WifiNote,
    NULL,
  },
  //EVENT_PROCLINK_CALL_MAIL
  { 
    NO_OVERLAY_ID,
    &MailSysProcData,
    FMenuCallProc_Mail,
    FMenuReturnProc_Mail,
    NULL,
  },
  //EVENT_PROCLINK_CALL_EVOLUTION
  { 
    NO_OVERLAY_ID,
    &ShinkaDemoProcData,
    FMenuCallProc_Evolution,
    FMenuReturnProc_Evolution,
    NULL,
  },
  //EVENT_PROCLINK_CALL_BTLRECORDER
  { 
    FS_OVERLAY_ID(battle_recorder),
    &BattleRecorder_ProcData,
    FMenuCallProc_BattleRecorder,
    FMenuReturnProc_BattleRecorder,
    NULL,
  },
};

//=============================================================================
/**
 *          外部参照
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  メニューのイベントなどアプリを起動する
 *
 *  @param  EVENT_PROCLINK_PARAM *param 引数（アロケートしたものを渡してください）
 *  @param  heapID                      ヒープID
 *
 *  @return                             イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_ProcLink( EVENT_PROCLINK_PARAM *param, HEAPID heapID )
{ 
  GMEVENT       *event;
  PROCLINK_WORK *wk;

  //イベント作成
  event = GMEVENT_Create( param->gsys, param->event, ProcLinkEvent, sizeof(PROCLINK_WORK));

  //ワーク
  wk    = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear( wk, sizeof(PROCLINK_WORK) );
  wk->heapID  = heapID;
  wk->param   = param;
  wk->result  = RETURNFUNC_RESULT_NEXT;
  wk->event   = event;

  wk->now_type  = wk->param->call;
  wk->pre_type  = wk->param->call;
  wk->next_type = wk->param->call;
  
  wk->sel_poke = 0;
  wk->item_no = 0;
  wk->mode = 0;

  PROCLINK_CALLBACK_Set( &wk->callback, wk->param, wk->param->open_func, wk->param->close_func, wk->param->wk_adrs );

  NAGI_Printf(  "init call %d\n", wk->param->call);

  //アイテム使用検査ワーク設定
  { 
    const VecFx32 *pPos;
    GAMEDATA* pGameData = GAMESYSTEM_GetGameData(wk->param->gsys);
    PLAYER_WORK *playerWork = GAMEDATA_GetMyPlayerWork( pGameData );
    FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer(wk->param->field_wk);

    pPos = PLAYERWORK_getPosition( playerWork );
    wk->icwk.NowAttr = FIELD_PLAYER_GetMapAttr(fld_player);
    wk->icwk.FrontAttr =FIELD_PLAYER_GetDirMapAttr(fld_player, FIELD_PLAYER_GetDir(fld_player));
    wk->icwk.gsys = wk->param->gsys;
    //ゾーンＩＤ
    wk->icwk.zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(pGameData ));
    //連れ歩き
    wk->icwk.Companion = FALSE;

    wk->icwk.PlayerForm=0;  //　自機の形状（自転車に乗っているかとか）
    wk->icwk.SeedInfo=0;  //使用可能なアイテム情報（きのみ関連）  
  }
  FLDSKILL_InitCheckWork( wk->param->field_wk, &wk->scwk );

  return event;
}

//=============================================================================
/**
 *          イベント
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  PROCを繋いでいく状態変移イベント
 *
 *  @param  GMEVENT *event  イベント
 *  @param  *seq            シーケンス
 *  @param  *wk_adrs        ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ProcLinkEvent( GMEVENT *event, int *seq, void *wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_JUNCTION,
    SEQ_END,

    //イベント
    SEQ_EVENT_CALL,
    SEQ_EVENT_WAIT,
    SEQ_EVENT_RETURN,

    //フィールド抜け（プロック移動の際はここを通る）
    SEQ_FLD_FADEOUT,
    SEQ_FLD_CLOSE,
    SEQ_FLD_OPEN,
    SEQ_FLD_FADEIN,

    //プロック繋がり
    SEQ_PROC_CALL,
    SEQ_PROC_WAIT,
    SEQ_PROC_RETURN,
  };
  /*
   *  メモ
   *  アプリ移動には以下の種類がある
   *  ・プロック
   *  ・イベント
   *  よって、JUNCTIONでどちらか判断し
   *  プロックの場合は、フィールドにいたら、フィールドを抜けてプロックへ
   *  イベントの場合は、フィールドになかったら、フィールドに戻ってイベントヘ
   *  それを判断するのがSEQ_JUNCTION
   *
   */

  PROCLINK_WORK *wk = wk_adrs;

  switch( *seq )
  { 
  case SEQ_INIT:
    *seq  = SEQ_JUNCTION;
    break;

  case SEQ_JUNCTION:
    if( wk->result == RETURNFUNC_RESULT_RETURN ||
        wk->result == RETURNFUNC_RESULT_EXIT )
    {
      //終了の場合
      *seq  = SEQ_END;
    }
    else if( wk->result == RETURNFUNC_RESULT_NEXT )
    { 
      NAGI_Printf( "now_type  %d\n", wk->now_type );
      //次に行く場合
      if( ProcLinkData[ wk->now_type ].event_func == NULL )
      { 
        *seq  = SEQ_FLD_FADEOUT;
      }
      else
      {
        *seq  = SEQ_EVENT_CALL;
      }
    }
    else if( wk->result == RETURNFUNC_RESULT_USE_ITEM )
    { 
      //アイテムが選択された場合
      wk->param->result = EVENT_PROCLINK_RESULT_ITEM;
      *seq  = SEQ_END;
    }
    else if( wk->result == RETURNFUNC_RESULT_USE_SKILL )
    { 
      //技が選択された場合
      wk->param->result = EVENT_PROCLINK_RESULT_SKILL;
      *seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    if( wk->result == RETURNFUNC_RESULT_RETURN )
    { 
      wk->param->result = EVENT_PROCLINK_RESULT_RETURN;
    }
    else if( wk->result == RETURNFUNC_RESULT_EXIT )
    { 
      wk->param->result = EVENT_PROCLINK_RESULT_EXIT;
    }
    //アイテムと技はもう入っている
    return GMEVENT_RES_FINISH;

  //-------------------------------------
  /// イベント
  //=====================================
  case SEQ_EVENT_CALL:
    GF_ASSERT( wk->now_type < EVENT_PROCLINK_CALL_MAX );

    PROCLINK_CALLBACK_Close( &wk->callback );

    ProcLinkData[ wk->now_type ].event_func( wk, wk->param->data );
    *seq  = SEQ_EVENT_WAIT;
    break;

  case SEQ_EVENT_WAIT:
    *seq  = SEQ_EVENT_RETURN;
    break;

  case SEQ_EVENT_RETURN:
    PROCLINK_CALLBACK_Open( &wk->callback );
    *seq  = SEQ_JUNCTION;
    break;

  //-------------------------------------
  /// サブプロック呼び出しのためのフィールド抜け
  //=====================================
  case SEQ_FLD_FADEOUT:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(wk->param->gsys, wk->param->field_wk, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_FLD_CLOSE;
    break;
    
  case SEQ_FLD_CLOSE:
    PROCLINK_CALLBACK_Close( &wk->callback );
    GMEVENT_CallEvent(event, EVENT_FieldClose(wk->param->gsys, wk->param->field_wk));
    *seq = SEQ_PROC_CALL; //PROCへ行く
    break;
    
  case SEQ_FLD_OPEN:      //PROCから戻る
    GMEVENT_CallEvent(event, EVENT_FieldOpen(wk->param->gsys));
    *seq = SEQ_FLD_FADEIN;
    break;
    
  case SEQ_FLD_FADEIN:
    //フィールドを作り直したため、再度取得
    wk->param->field_wk = GAMESYSTEM_GetFieldMapWork(wk->param->gsys);
    FIELDMAP_InitBGMode();
    FIELDMAP_InitBG( wk->param->field_wk );
    PROCLINK_CALLBACK_Open( &wk->callback );  
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Menu(wk->param->gsys, wk->param->field_wk, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_JUNCTION;
    break;

  //-------------------------------------
  /// サブプロック呼び出し部分
  //    基本的に以下の３つのシーケンスをぐるぐる回ります
  //=====================================
  //プロセス呼び出し
  case SEQ_PROC_CALL:
    { 
      void *now_param;
      //現在のプロセスパラメータ作成
      GF_ASSERT( wk->now_type < EVENT_PROCLINK_CALL_MAX );
      GF_ASSERT( ProcLinkData[ wk->now_type ].call_func );
      now_param = ProcLinkData[ wk->now_type ].call_func( wk, wk->param->data, wk->pre_type, wk->proc_param );

      //前のプロセスパラメータ破棄
      if( wk->proc_param )
      { 
        GFL_HEAP_FreeMemory(wk->proc_param);
        wk->proc_param = NULL;
      }

      //今のプロセスパラメータ設定
      wk->proc_param  = now_param;

      //次のプロセスへ移動
      if( ProcLinkData[ wk->now_type ].proc_data )
      { 
        GAMESYSTEM_CallProc( wk->param->gsys, 
            ProcLinkData[ wk->now_type ].ovId , 
            ProcLinkData[ wk->now_type ].proc_data, 
            wk->proc_param);
        *seq  = SEQ_PROC_WAIT;
      }
    }
    break;

  //プロセス待ち
  case SEQ_PROC_WAIT:
    if( GAMESYSTEM_IsProcExists(wk->param->gsys) == GFL_PROC_MAIN_NULL )
    { 
      *seq  = SEQ_PROC_RETURN;
    }
    break;

  //プロセス戻り
  case SEQ_PROC_RETURN:
    //次への繋がり情報取得
    GF_ASSERT( wk->now_type < EVENT_PROCLINK_CALL_MAX );
    wk->result  = ProcLinkData[ wk->now_type ].return_func( wk, wk->proc_param );

    //引数へ代入
    if( wk->result == RETURNFUNC_RESULT_RETURN )
    { 
      wk->param->result = EVENT_PROCLINK_RESULT_RETURN;
    }
    else if( wk->result == RETURNFUNC_RESULT_EXIT )
    { 
      wk->param->result = EVENT_PROCLINK_RESULT_EXIT;
    }
    else if( wk->result == RETURNFUNC_RESULT_USE_ITEM )
    { 
      //アイテムが選択された場合
      wk->param->result = EVENT_PROCLINK_RESULT_ITEM;
    }
    else if( wk->result == RETURNFUNC_RESULT_USE_SKILL )
    { 
      //技が選択された場合
      wk->param->result = EVENT_PROCLINK_RESULT_SKILL;
    }

    //次へ
    if( wk->result == RETURNFUNC_RESULT_NEXT )
    { 
      //次もプロセスの場合はぐるぐる回る
      wk->pre_type  = wk->now_type;
      wk->now_type  = wk->next_type;

      NAGI_Printf(  "次のプロセスへ！ pre %d now %d\n", wk->pre_type, wk->now_type );
      *seq = SEQ_PROC_CALL;
    }
    else
    { 
      //終了するので、解放し終了状態によって外部へ設定
      if( wk->proc_param )
      { 
        GFL_HEAP_FreeMemory(wk->proc_param);
        wk->proc_param = NULL;
      }
      *seq = SEQ_FLD_OPEN;
      NAGI_Printf(  "プロセス終了！ \n" );
    }
    break;
  }


  return GMEVENT_RES_CONTINUE;
}
//=============================================================================
/**
 *  CALL関数とRETURN関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  関数構造体設定
 *
 *  @param    wk          ワーク
 *  @param    param       引数
 *  @param    open_func   開く関数
 *  @param    close_func  閉じる関数
 *  @param    wk_adrs     引数に与えたワーク
 */
//-----------------------------------------------------------------------------
static void PROCLINK_CALLBACK_Set( PROCLINK_CALLBACK_WORK *wk, const EVENT_PROCLINK_PARAM *param, EVENT_PROCLINK_MENUOPEN_FUNC  open_func, EVENT_PROCLINK_MENUCLOSE_FUNC close_func, void *wk_adrs )
{ 
  GFL_STD_MemClear( wk, sizeof(PROCLINK_CALLBACK_WORK) );
  wk->open_func   = open_func;
  wk->close_func  = close_func;
  wk->wk_adrs     = wk_adrs;
  wk->param       = param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  開く関数を実行
 *
 *  @param  PROCLINK_CALLBACK_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void PROCLINK_CALLBACK_Open( PROCLINK_CALLBACK_WORK *wk )
{ 
  if( wk->open_func )
  { 
    wk->open_func( wk->param, wk->wk_adrs );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  閉じる関数を実行
 *
 *  @param  PROCLINK_CALLBACK_WORK *wk  ワーク
 */
//-----------------------------------------------------------------------------
static void PROCLINK_CALLBACK_Close( PROCLINK_CALLBACK_WORK *wk )
{ 
  if( wk->close_func )
  { 
    wk->close_func( wk->param, wk->wk_adrs );
  }
}

//=============================================================================
/**
 *  CALL関数とRETURN関数
 */
//=============================================================================
//-------------------------------------
/// ポケリスト
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_PokeList(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  PLIST_DATA *plistData;
  GAMEDATA *gmData = GAMESYSTEM_GetGameData( wk->param->gsys );

  plistData     = GFL_HEAP_AllocClearMemory(HEAPID_PROC, sizeof(PLIST_DATA));

  plistData->pp = GAMEDATA_GetMyPokemon( gmData);
  plistData->scwk = wk->scwk;
  plistData->myitem   = GAMEDATA_GetMyItem( gmData);    // アイテムデータ
  plistData->mailblock = GAMEDATA_GetMailBlock( gmData);
  plistData->mode     = PL_MODE_FIELD;
  plistData->ret_sel  = wk->param->select_poke;
  plistData->waza = 0;
  plistData->place    = ZONEDATA_GetPlaceNameID( GAMEDATA_GetMyPlayerWork(gmData)->zoneID ); 

  if( pre == EVENT_PROCLINK_CALL_POKELIST )
  { 
    //初回時だったら
    plistData->ret_sel  = 0;
  }
  else
  if( pre == EVENT_PROCLINK_CALL_BAG )
  { 
    //１つ前がバッグで、ポケリストへ移行したいとき
    const BAG_PARAM *pBag = pre_param_adrs;

    plistData->item = pBag->ret_item;     //アイテムID

    switch( pBag->next_proc )
    { 
    case BAG_NEXTPROC_RETURN:
      if(pBag->mode == BAG_MODE_POKELIST){   //リストから呼ばれていた場合
        plistData->mode = PL_MODE_ITEMSET;    //アイテムをセットする呼び出し
      }
      break;
      
    case BAG_NEXTPROC_EVOLUTION:
      plistData->mode = PL_MODE_SHINKA;    //進化アイテム
      break;
      
    case BAG_NEXTPROC_HAVE:  // ポケモンにアイテムをもたせる
      plistData->mode = PL_MODE_ITEMSET;    //アイテムをセットする呼び出し
      break;

    case BAG_NEXTPROC_ITEMEQUIP:  //ポケモンリストのアイテム装備に戻る
      plistData->mode     = PL_MODE_ITEMSET_RET;
      break;

    case BAG_NEXTPROC_ITEMHAVE_RET:  //ポケモンリストのアイテム装備に戻るが、キャンセルのとき
      plistData->mode     = PL_MODE_ITEMSET_RET;
      break;

    case BAG_NEXTPROC_ITEMUSE:  //使う
      plistData->mode     = PL_MODE_ITEMUSE;
      break;

    case BAG_NEXTPROC_WAZASET: //技セット
      plistData->mode     = PL_MODE_WAZASET;
      break;

    default:
      plistData->mode = PL_MODE_ITEMSET;    //アイテムをセットする呼び出し
      break;
    }
  }
  else if( pre == EVENT_PROCLINK_CALL_STATUS )
  { 
    //１つ前がステータスで、ポケリストへ移行したいとき
    const PSTATUS_DATA *psData  = pre_param_adrs;
    
    switch( psData->ret_mode )
    { 
    case PST_RET_DECIDE:
      /* fallthrough */
    case PST_RET_CANCEL:
      if( psData->mode == PST_MODE_WAZAADD )
      {
        if( wk->mode == PROCLINK_MODE_WAZAWASURE_LVUP )
        {
          //不思議なアメ
          plistData->mode = PL_MODE_LVUPWAZASET_RET;
        }
        else
        {
          //技マシン
          plistData->mode = PL_MODE_WAZASET_RET;
        }
        plistData->item = wk->param->select_param;
        plistData->waza = psData->waza;
        if( psData->ret_mode == PST_RET_DECIDE )
        {
          plistData->waza_pos = psData->ret_sel;
        }
        else
        {
          plistData->waza_pos = 0xFF;
        }
      }
      else
      {
        plistData->mode = PL_MODE_FIELD;
        plistData->ret_sel = psData->pos;
      }

      break;

    default:
      GF_ASSERT( 0 );
    }
  }
  else
  if( pre == EVENT_PROCLINK_CALL_MAIL )
  {
    //メールを持たせる処理(書かなかった場合はメールのTermで分岐してる
    if( wk->mode == PROCLINK_MODE_LIST_TO_MAIL_CREATE )
    {
      plistData->mode = PL_MODE_MAILSET;
      plistData->ret_sel = wk->sel_poke;
      plistData->item = wk->item_no;
    }
    else
    if( wk->mode == PROCLINK_MODE_BAG_TO_MAIL_CREATE )
    {
      plistData->mode    = PL_MODE_MAILSET_BAG;
      plistData->ret_sel = wk->sel_poke;
      plistData->item    = wk->item_no;
    }
    else
    {
      //メールを見たとき
      //特に無し。
    }
  }

  return plistData;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_PokeList(PROCLINK_WORK* wk,void* param_adrs)
{ 
  const PLIST_DATA *plData = param_adrs;
  
  wk->param->select_poke  = plData->ret_sel;
  switch( plData->ret_mode )
  {
  case PL_RET_NORMAL:      // 通常
    if( plData->mode == PL_MODE_WAZASET || wk->param->call == EVENT_PROCLINK_CALL_BAG ) 
    { 
      wk->next_type = EVENT_PROCLINK_CALL_BAG;
      return RETURNFUNC_RESULT_NEXT;
    }
    else
    {
      if(plData->ret_sel == PL_SEL_POS_EXIT2)
      {
        return RETURNFUNC_RESULT_EXIT;
      }
      else
      { 
        return RETURNFUNC_RESULT_RETURN;
      }
    }
    break;
    
  case PL_RET_STATUS:      // メニュー「つよさをみる」
    wk->next_type = EVENT_PROCLINK_CALL_STATUS;
    return RETURNFUNC_RESULT_NEXT;
  
  //もたせる　でアイテム画面へ
  case PL_RET_ITEMSET:
    wk->next_type = EVENT_PROCLINK_CALL_BAG;
    return RETURNFUNC_RESULT_NEXT;
    
  case PL_RET_BAG:      // アイテム→リスト→アイテム
    wk->next_type = EVENT_PROCLINK_CALL_BAG;
    return RETURNFUNC_RESULT_NEXT;
    
  case PL_RET_WAZASET:  //忘れる技選択
  case PL_RET_LVUP_WAZASET:
    wk->next_type = EVENT_PROCLINK_CALL_STATUS;
    return RETURNFUNC_RESULT_NEXT;
    
  case PL_RET_MAILSET:
  case PL_RET_MAILREAD:
    wk->next_type = EVENT_PROCLINK_CALL_MAIL;
    return RETURNFUNC_RESULT_NEXT;
    break;

  case PL_RET_ITEMSHINKA:
  case PL_RET_LVUPSHINKA:
    wk->next_type = EVENT_PROCLINK_CALL_EVOLUTION;
    return RETURNFUNC_RESULT_NEXT;
    break;

  case PL_RET_IAIGIRI:     // メニュー 技：いあいぎり
  case PL_RET_NAMINORI:    // メニュー 技：なみのり
  case PL_RET_TAKINOBORI:  // メニュー 技：たきのぼり
  case PL_RET_KIRIBARAI:   // メニュー 技：きりばらい
  case PL_RET_KAIRIKI:     // メニュー 技：かいりき

  case PL_RET_IWAKUDAKI:   // メニュー 技：いわくだき
  case PL_RET_ROCKCLIMB:   // メニュー 技：ロッククライム
  case PL_RET_FLASH:       // メニュー 技：フラッシュ
  case PL_RET_TELEPORT:    // メニュー 技：テレポート
  case PL_RET_ANAWOHORU:   // メニュー 技：あなをほる
  case PL_RET_AMAIKAORI:   // メニュー 技：あまいかおり
  case PL_RET_OSYABERI:    // メニュー 技：おしゃべり
    wk->param->select_param = plData->ret_mode-PL_RET_IAIGIRI;
    return RETURNFUNC_RESULT_USE_SKILL;

  case PL_RET_SORAWOTOBU:  // メニュー 技：そらをとぶ
    wk->next_type = EVENT_PROCLINK_CALL_TOWNMAP;
//    wk->param->select_param = plData->ret_mode-PL_RET_IAIGIRI;
    return RETURNFUNC_RESULT_NEXT;

  default:
    GF_ASSERT(0);
    return RETURNFUNC_RESULT_EXIT;
    
/*
  case PL_RET_NORMAL:      // 通常
  case PL_RET_STATUS:      // メニュー「つよさをみる」
  case PL_RET_CHANGE:      // メニュー「いれかえる」
  case PL_RET_ITEMSET:     // メニュー「もたせる」

  case PL_RET_WAZASET:     // 技選択へ（技マシン）
  case PL_RET_LVUP_WAZASET:// 技選択へ（不思議なアメ）
  case PL_RET_MAILSET:     // メール入力へ
  case PL_RET_MAILREAD:    // メールを読む画面へ

  case PL_RET_ITEMSHINKA:  // 進化画面へ（アイテム進化）
  case PL_RET_LVUPSHINKA:  // 進化画面へ（レベル進化）
  
  case PL_RET_BAG:         // バッグから呼ばれた場合で、バッグへ戻る

  
  case PL_RET_IAIGIRI:     // メニュー 技：いあいぎり
  case PL_RET_SORAWOTOBU:  // メニュー 技：そらをとぶ
  case PL_RET_NAMINORI:    // メニュー 技：なみのり
  case PL_RET_KAIRIKI:     // メニュー 技：かいりき
  case PL_RET_KIRIBARAI:   // メニュー 技：きりばらい
  case PL_RET_IWAKUDAKI:   // メニュー 技：いわくだき
  case PL_RET_TAKINOBORI:  // メニュー 技：たきのぼり
  case PL_RET_ROCKCLIMB:   // メニュー 技：ロッククライム

  case PL_RET_FLASH:       // メニュー 技：フラッシュ
  case PL_RET_TELEPORT:    // メニュー 技：テレポート
  case PL_RET_ANAWOHORU:   // メニュー 技：あなをほる
  case PL_RET_AMAIKAORI:   // メニュー 技：あまいかおり
  case PL_RET_OSYABERI:    // メニュー 技：おしゃべり
  case PL_RET_MILKNOMI:    // メニュー 技：ミルクのみ
  case PL_RET_TAMAGOUMI:   // メニュー 技：タマゴうみ
*/        
  }


}
//-------------------------------------
/// 図鑑
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  EVENT関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 */
//-----------------------------------------------------------------------------
/*
static void FMenuEvent_Zukan( PROCLINK_WORK* wk, u32 param )
{ 
  GMEVENT * subevent = createFMenuMsgWinEvent( wk->param->gsys, wk->heapID,
    FLDMAPMENU_STR08, FIELDMAP_GetFldMsgBG(wk->param->field_wk) );
  GMEVENT_CallEvent(wk->event, subevent);

  wk->result  = RETURNFUNC_RESULT_RETURN;
}
*/
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_Zukan(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{
  ZUKAN_PARAM * prm = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(ZUKAN_PARAM) );

  prm->callMode = ZUKAN_MODE_LIST;

  return prm;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_Zukan(PROCLINK_WORK* wk,void* param_adrs)
{
  ZUKAN_PARAM * prm = param_adrs;

  if( prm->retMode == ZUKAN_RET_NORMAL ){
    return RETURNFUNC_RESULT_RETURN;
  }
  return RETURNFUNC_RESULT_EXIT;
}

//-------------------------------------
/// ステータス
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_PokeStatus(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  PSTATUS_DATA *psData = GFL_HEAP_AllocClearMemory( HEAPID_PROC , sizeof(PSTATUS_DATA) );
  GAMEDATA *gmData = GAMESYSTEM_GetGameData(wk->param->gsys);
  SAVE_CONTROL_WORK *svWork = GAMEDATA_GetSaveControlWork( gmData );

  psData->ppt = PST_PP_TYPE_POKEPARTY;
  psData->game_data = gmData;
  psData->isExitRequest = FALSE;

  if( pre == EVENT_PROCLINK_CALL_POKELIST )
  { 
    //リストから呼び出し
    const PLIST_DATA *plData    = pre_param_adrs;

    psData->ppd   = (void*)plData->pp;
    psData->cfg   = plData->cfg;
    psData->max   = PokeParty_GetPokeCount( plData->pp );
    psData->pos   = plData->ret_sel;

    //リストのモードによって、ステータスのモードが違う
    switch( plData->ret_mode )
    { 
    case PL_RET_WAZASET:  //忘れる技選択 
      psData->waza = plData->waza;
      psData->mode  = PST_MODE_WAZAADD;
      psData->page  = PPT_SKILL;
      psData->canExitButton = FALSE;
      wk->mode = PROCLINK_MODE_WAZAWASURE_MACHINE;
      break;

    case PL_RET_LVUP_WAZASET://忘れる技選択 
      psData->waza = plData->waza;
      psData->mode  = PST_MODE_WAZAADD;
      psData->page  = PPT_SKILL;
      psData->canExitButton = FALSE;
      wk->mode = PROCLINK_MODE_WAZAWASURE_LVUP;
      break;

    default:
      psData->page  = PPT_INFO;
      psData->mode  = PST_MODE_NORMAL;
      psData->canExitButton = TRUE;
    }
  }
  else if( pre == EVENT_PROCLINK_CALL_STATUS )
  { 
    //直接呼出し
    psData->cfg   = SaveData_GetConfig( svWork );
    psData->ppd   = (void*)GAMEDATA_GetMyPokemon( gmData );
    psData->max   = PokeParty_GetPokeCount( GAMEDATA_GetMyPokemon( gmData ) );  
    psData->pos   = 0;
    psData->mode  = PST_MODE_NORMAL;
    psData->canExitButton = TRUE;
    if( param != EVENT_PROCLINK_DATA_NONE )
    { 
      psData->page  = param;
    }
    else
    { 
      psData->page  = PPT_INFO;
    }
  }



  return psData;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_PokeStatus(PROCLINK_WORK* wk,void* param_adrs)
{ 
  const PSTATUS_DATA *psData = param_adrs;

  switch( psData->ret_mode )
  {
  case PST_RET_DECIDE:
    /* fallthrough */
  case PST_RET_CANCEL:
    if( wk->param->call == EVENT_PROCLINK_CALL_STATUS )
    { 
      return RETURNFUNC_RESULT_RETURN;
    }
    else
    { 
      wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
      return RETURNFUNC_RESULT_NEXT;
    }

  case PST_RET_EXIT:
    return RETURNFUNC_RESULT_EXIT;
  
  default:
    GF_ASSERT(0);
    return RETURNFUNC_RESULT_EXIT;
  }
}
//-------------------------------------
/// バッグ
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_Bag(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  BAG_PARAM* bag;
  GAMEDATA* gmData = GAMESYSTEM_GetGameData( wk->param->gsys );

  switch( pre )
  { 
  case EVENT_PROCLINK_CALL_BAG:
    //バッグを最初に呼び出したとき
    bag = BAG_CreateParam( gmData, &wk->icwk, BAG_MODE_FIELD, HEAPID_PROC );
    if( param != EVENT_PROCLINK_DATA_NONE )
    { 
      MYITEM_FieldBagPocketSet( bag->p_bagcursor, param );
    }
    break;
    
  case EVENT_PROCLINK_CALL_POKELIST:
    { 
      const PLIST_DATA *plData = pre_param_adrs;
      if( plData->ret_mode == PL_RET_ITEMSET )
      { 
        //もたせるでバッグ画面へ
        bag = BAG_CreateParam( gmData, &wk->icwk, BAG_MODE_POKELIST, HEAPID_PROC );
      }
      else if( plData->ret_mode == PL_RET_BAG )
      { 
        //アイテムー＞リストー＞アイテム
        bag = BAG_CreateParam( gmData, &wk->icwk, BAG_MODE_FIELD, HEAPID_PROC );
      }
      else
      { 
        //アイテムー＞リストー＞アイテム
        bag = BAG_CreateParam( gmData, &wk->icwk, BAG_MODE_FIELD, HEAPID_PROC );
      }
    }
    break;

    //バッグに戻ってくるとき
  case EVENT_PROCLINK_CALL_TOWNMAP:
    /* fallthrough */
  case EVENT_PROCLINK_CALL_WIFINOTE:
    /* fallthrough */
  default:
    bag = BAG_CreateParam( gmData, &wk->icwk, BAG_MODE_FIELD, HEAPID_PROC );
  }

  return bag;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_Bag(PROCLINK_WORK* wk,void* param_adrs)
{ 
  BAG_PARAM* pBag = param_adrs;
  switch( pBag->next_proc )
  { 
  case BAG_NEXTPROC_EXIT:
    return RETURNFUNC_RESULT_EXIT;

  case BAG_NEXTPROC_RETURN:
    if( pBag->mode == BAG_MODE_POKELIST )
    { 
      wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
      return RETURNFUNC_RESULT_NEXT;
    }
    else
    { 
      return RETURNFUNC_RESULT_RETURN;
    }

  case BAG_NEXTPROC_EVOLUTION:
    wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
    return RETURNFUNC_RESULT_NEXT;

  case BAG_NEXTPROC_RIDECYCLE:
    /* fallthrough */
  case BAG_NEXTPROC_DROPCYCLE:
    wk->param->select_param = EVENT_ITEMUSE_CALL_CYCLE;
    return RETURNFUNC_RESULT_USE_ITEM;

  case BAG_NEXTPROC_PALACEJUMP:
    wk->param->select_param = EVENT_ITEMUSE_CALL_PALACEJUMP;
    return RETURNFUNC_RESULT_USE_ITEM;

  case BAG_NEXTPROC_AMAIMITU:
    wk->param->select_param = EVENT_ITEMUSE_CALL_AMAIMITU;
    return RETURNFUNC_RESULT_USE_ITEM;

  case BAG_NEXTPROC_TURIZAO:
    wk->param->select_param = EVENT_ITEMUSE_CALL_TURIZAO;
    return RETURNFUNC_RESULT_USE_ITEM;

  case BAG_NEXTPROC_ANANUKENOHIMO:
    wk->param->select_param = EVENT_ITEMUSE_CALL_ANANUKENOHIMO;
    return RETURNFUNC_RESULT_USE_ITEM;

  case BAG_NEXTPROC_FRIENDNOTE: 
    wk->next_type = EVENT_PROCLINK_CALL_WIFINOTE;
    return RETURNFUNC_RESULT_NEXT;

  case BAG_NEXTPROC_TOWNMAP:
    wk->next_type = EVENT_PROCLINK_CALL_TOWNMAP;
    return RETURNFUNC_RESULT_NEXT;

  case BAG_NEXTPROC_BATTLERECORDER:
    wk->next_type = EVENT_PROCLINK_CALL_BTLRECORDER;
    return RETURNFUNC_RESULT_NEXT;
  case BAG_NEXTPROC_MAILVIEW:
    wk->next_type = EVENT_PROCLINK_CALL_MAIL;
    return RETURNFUNC_RESULT_NEXT;
    
  default:
    wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
    return RETURNFUNC_RESULT_NEXT;
  }
}
//-------------------------------------
/// トレーナーカード
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_TrainerCard(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  TRCARD_CALL_PARAM *tr_param;
  GAMEDATA *gdata;
  
  gdata = GAMESYSTEM_GetGameData( wk->param->gsys );

  tr_param = TRAINERCASR_CreateCallParam_SelfData( gdata, HEAPID_PROC );

  // 表か裏の指定をtr_paramに格納する
  if(param==FALSE || param==EVENT_PROCLINK_DATA_NONE){
    OS_Printf("トレーナーカードの表呼び出し\n");
  }else{
    OS_Printf("トレーナーカードの裏呼び出し\n");
  }
  return tr_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_TrainerCard(PROCLINK_WORK* wk,void* param_adrs)
{ 
  TRCARD_CALL_PARAM * tr_param  = param_adrs;
  return RETURNFUNC_RESULT_RETURN;
}
//-------------------------------------
/// タウンマップ
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_TownMap(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  TOWNMAP_PARAM*  town_param;
  town_param              = GFL_HEAP_AllocClearMemory( HEAPID_PROC , sizeof(TOWNMAP_PARAM) );
  town_param->mode        = TOWNMAP_MODE_MAP;
  town_param->p_gamesys = wk->param->gsys;


  if( wk->param->call == EVENT_PROCLINK_CALL_POKELIST )
  { 
    //リストから呼ばれるときは、空を飛ぶ
    town_param->mode        = TOWNMAP_MODE_SKY;
  }
  else
  { 
    //それ以外はタウンマップ
    town_param->mode        = TOWNMAP_MODE_MAP;
  }

  return town_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_TownMap(PROCLINK_WORK* wk,void* param_adrs)
{ 
  const TOWNMAP_PARAM*  param = param_adrs;

  if( wk->param->call == EVENT_PROCLINK_CALL_TOWNMAP )
  { 
    //タウンマップが直接呼ばれた
    switch( param->select )
    { 
    case TOWNMAP_SELECT_RETURN: //何もせず戻る
      return RETURNFUNC_RESULT_RETURN;
    case TOWNMAP_SELECT_CLOSE:    //何もせず終了する
      return RETURNFUNC_RESULT_EXIT;
    default:
      GF_ASSERT(0);
      return RETURNFUNC_RESULT_EXIT;
    }
  }
  else 
  { 
    if( wk->param->call == EVENT_PROCLINK_CALL_BAG )
    { 

      //バッグから呼ばれた
      switch( param->select )
      { 
      case TOWNMAP_SELECT_RETURN: //何もせず戻る
        wk->next_type = EVENT_PROCLINK_CALL_BAG;
        return RETURNFUNC_RESULT_NEXT;
      case TOWNMAP_SELECT_CLOSE:    //何もせず終了する
        return RETURNFUNC_RESULT_EXIT;
      default:
        GF_ASSERT(0);
        return RETURNFUNC_RESULT_EXIT;
      }
    }
    else if( wk->param->call == EVENT_PROCLINK_CALL_POKELIST )
    { 
      //リストから呼ばれた
      switch( param->select )
      { 
      case TOWNMAP_SELECT_RETURN: //何もせず戻る
        wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
        return RETURNFUNC_RESULT_NEXT;
      case TOWNMAP_SELECT_CLOSE:    //何もせず終了する
        return RETURNFUNC_RESULT_EXIT;
      case TOWNMAP_SELECT_SKY:  //空を飛ぶ
        wk->param->select_param = PL_RET_SORAWOTOBU-PL_RET_IAIGIRI;
        wk->param->zoneID       = param->zoneID;
        wk->param->grid_x       = param->grid.x;
        wk->param->grid_y       = param->grid.y;
        wk->param->grid_z       = 0;
        return RETURNFUNC_RESULT_USE_SKILL;
  
      default:
        GF_ASSERT(0);
        return RETURNFUNC_RESULT_EXIT;
      }
    }
  }

  return RETURNFUNC_RESULT_EXIT;
}
//-------------------------------------
/// コンフィグ
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_Config(PROCLINK_WORK* wk, u32 param, EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  CONFIG_PANEL_PARAM  *config_panel;

  GAMEDATA *gameData = GAMESYSTEM_GetGameData( wk->param->gsys );
  SAVE_CONTROL_WORK *saveControl = GAMEDATA_GetSaveControlWork( gameData );
  CONFIG *config = SaveData_GetConfig( saveControl );
  config_panel  = GFL_HEAP_AllocClearMemory(HEAPID_PROC, sizeof(CONFIG_PANEL_PARAM));
  config_panel->p_gamesys = wk->param->gsys;

  return config_panel;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_Config(PROCLINK_WORK* wk,void* param_adrs)
{ 
  const CONFIG_PANEL_PARAM  *config_panel = param_adrs;
  if( config_panel->is_exit )
  { 
    return RETURNFUNC_RESULT_EXIT;
  }
  else
  { 
    return RETURNFUNC_RESULT_RETURN;
  }
}
//-------------------------------------
/// レポート
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  EVENT関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 */
//-----------------------------------------------------------------------------
static void FMenuEvent_Report( PROCLINK_WORK* wk, u32 param )
{ 
  GMEVENT * subevent = createFMenuReportEvent( wk->param->gsys, wk->param->field_wk, wk->heapID,
      FIELDMAP_GetFldMsgBG(wk->param->field_wk ) );
  GMEVENT_CallEvent(wk->event, subevent);

  wk->result  = RETURNFUNC_RESULT_RETURN;
}
//-------------------------------------
/// 友達手帳
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_WifiNote(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  WIFINOTE_PROC_PARAM* pFriend;
  
  pFriend = GFL_HEAP_AllocClearMemory( HEAPID_PROC , sizeof(WIFINOTE_PROC_PARAM) );
  pFriend->pGameData = GAMESYSTEM_GetGameData(wk->param->gsys);

  return pFriend;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_WifiNote(PROCLINK_WORK* wk,void* param_adrs)
{ 
  if( wk->param->call == EVENT_PROCLINK_CALL_WIFINOTE )
  { 
    return RETURNFUNC_RESULT_RETURN;
  }
  else
  { 
    wk->next_type = EVENT_PROCLINK_CALL_BAG;
    return RETURNFUNC_RESULT_NEXT;
  }
}
//-------------------------------------
/// メール画面
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_Mail(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  GAMEDATA *gmData = GAMESYSTEM_GetGameData( wk->param->gsys );
  MAIL_PARAM *mailParam;
  
  GFL_OVERLAY_Load( FS_OVERLAY_ID(app_mail));

  if( pre == EVENT_PROCLINK_CALL_POKELIST )
  {
    //バッグ(メール選択)→リスト→メール画面
    const PLIST_DATA *plistData = pre_param_adrs;
    const POKEPARTY *party = GAMEDATA_GetMyPokemon( gmData );
    wk->sel_poke = plistData->ret_sel;
    wk->item_no = plistData->item;
    if( plistData->ret_mode == PL_RET_MAILSET )
    {
      wk->mode = PROCLINK_MODE_LIST_TO_MAIL_CREATE;
      mailParam = MAILSYS_GetWorkCreate( gmData, 
                                         MAILBLOCK_TEMOTI, 
                                         plistData->ret_sel,
                                         ITEM_GetMailDesign( plistData->item ) ,
                                         HEAPID_PROC );
    }
    else
    {
      wk->mode = PROCLINK_MODE_LIST_TO_MAIL_VIEW;
      OS_TPrintf("MailPos[%d]\n",plistData->ret_sel);
      mailParam = MailSys_GetWorkViewPoke( gmData, PokeParty_GetMemberPointer(party,plistData->ret_sel), HEAPID_PROC );
    }

  }else{
    wk->mode = PROCLINK_MODE_LIST_TO_MAIL_VIEW;
    OS_TPrintf("メールデザイン見るだけ\n");
    mailParam = MailSys_GetWorkViewPrev( gmData, wk->item_no-ITEM_GURASUMEERU, HEAPID_PROC );
  }

  return mailParam;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_Mail(PROCLINK_WORK* wk,void* param_adrs)
{ 
  GAMEDATA *gmData = GAMESYSTEM_GetGameData( wk->param->gsys );
  MAIL_PARAM *mailParam = (MAIL_PARAM*)param_adrs;
  
  if( wk->pre_type == EVENT_PROCLINK_CALL_POKELIST )
  { 
    if( wk->mode == PROCLINK_MODE_LIST_TO_MAIL_CREATE )
    {
      const BOOL isCreate = MailSys_IsDataCreate( mailParam );
      if( isCreate == TRUE )
      {
        const POKEPARTY *party = GAMEDATA_GetMyPokemon( gmData );
        MailSys_PushDataToSavePoke( mailParam , PokeParty_GetMemberPointer(party,wk->sel_poke) );
        wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
      }
      else
      {
        wk->next_type = EVENT_PROCLINK_CALL_BAG;
      }
    }
    else
    {
      wk->next_type = EVENT_PROCLINK_CALL_POKELIST;
    }
    // ワーク解放
//    MailSys_ReleaseCallWork( mailParam);

    //オーバーレイ開放！
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(app_mail));
    return RETURNFUNC_RESULT_NEXT;
  }

  // ワーク解放
//  MailSys_ReleaseCallWork( mailParam);
  wk->next_type = EVENT_PROCLINK_CALL_BAG;
  //オーバーレイ開放！！！
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(app_mail));
  return RETURNFUNC_RESULT_RETURN;
}

//-------------------------------------
/// 進化画面
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_Evolution(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  GAMEDATA *gmData = GAMESYSTEM_GetGameData( wk->param->gsys );
  SHINKA_DEMO_PARAM *demoParam = NULL;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(shinka_demo));
  if( pre == EVENT_PROCLINK_CALL_POKELIST )
  {
    const PLIST_DATA *plistData = pre_param_adrs;

    POKEPARTY *party = GAMEDATA_GetMyPokemon( gmData );
    SHINKA_TYPE type;
    SHINKA_COND cond;
    u16 newMonsNo;
    if( plistData->ret_mode == PL_RET_ITEMSHINKA )
    {
      type = SHINKA_TYPE_ITEM_CHECK;
      wk->mode = PROCLINK_MODE_EVOLUTION_ITEM;
    }
    else
    {
      type = SHINKA_TYPE_LEVELUP;
      wk->mode = PROCLINK_MODE_EVOLUTION_LEVEL;
    }
    newMonsNo = SHINKA_Check( party , 
                              PokeParty_GetMemberPointer(party,plistData->ret_sel) , 
                              type , 
                              plistData->item , 
                              &cond , 
                              HEAPID_PROC );

    demoParam = SHINKADEMO_AllocParam(HEAPID_PROC ,
                                      party ,
                                      newMonsNo ,
                                      plistData->ret_sel ,
                                      cond );

  }
  return demoParam;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_Evolution(PROCLINK_WORK* wk,void* param_adrs)
{ 
  GAMEDATA *gmData = GAMESYSTEM_GetGameData( wk->param->gsys );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(shinka_demo) );
  
  if( wk->mode == PROCLINK_MODE_EVOLUTION_ITEM )
  {
    wk->next_type = EVENT_PROCLINK_CALL_BAG;
    return RETURNFUNC_RESULT_NEXT;
  }
  else
  { 
    return RETURNFUNC_RESULT_RETURN;
  }
}

//-------------------------------------
/// バトルレコーダー画面
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  CALL関数
 *
 *  @param  wk      メインワーク
 *  @param  param   起動時引数
 *  @param  pre     一つ前のプロック
 *  @param  void* pre_param_adrs  一つ前のプロックパラメータ
 *
 *  @return プロセスパラメータ
 */
//-----------------------------------------------------------------------------
static void * FMenuCallProc_BattleRecorder(PROCLINK_WORK* wk, u32 param,EVENT_PROCLINK_CALL_TYPE pre, const void* pre_param_adrs )
{ 
  BATTLERECORDER_PARAM* p_btlrecorder_param;
  
  p_btlrecorder_param = GFL_HEAP_AllocClearMemory( HEAPID_PROC , sizeof(BATTLERECORDER_PARAM) );
  p_btlrecorder_param->p_gamedata = GAMESYSTEM_GetGameData(wk->param->gsys);

  return p_btlrecorder_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  RETURN関数
 *
 *  @param  wk      メインワーク
 *  @param  param_adrs  自分のプロックパラメータ
 *
 *  @return   RETURNFUNC_RESULT_RETURN,     //メニューに戻る
 *            RETURNFUNC_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
 *            RETURNFUNC_RESULT_USE_SKILL,    //メニューを抜けて技を使う
 *            RETURNFUNC_RESULT_USE_ITEM,   //メニュを抜けてアイテムを使う
 *            RETURNFUNC_RESULT_NEXT,       //次のプロセスへ行く
 */ 
//-----------------------------------------------------------------------------
static RETURNFUNC_RESULT FMenuReturnProc_BattleRecorder(PROCLINK_WORK* wk,void* param_adrs)
{ 
  if( wk->param->call == EVENT_PROCLINK_CALL_BTLRECORDER )
  { 
    return RETURNFUNC_RESULT_RETURN;
  }
  else
  {
    wk->next_type = EVENT_PROCLINK_CALL_BAG;
    return RETURNFUNC_RESULT_NEXT;
  }
}

//=============================================================================
/**
 *  レポートと図鑑のイベント
 */
//=============================================================================
//======================================================================
//  メッセージウィンドウ表示イベント
//======================================================================
//--------------------------------------------------------------
/// FMENU_MSGWIN_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  u32 heapID;
  u32 strID;
  FLDMSGBG *msgBG;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
}FMENU_MSGWIN_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * createFMenuMsgWinEvent(
  GAMESYS_WORK *gsys, u32 heapID, u32 strID, FLDMSGBG *msgBG )
{
  GMEVENT * msgEvent;
  FMENU_MSGWIN_EVENT_WORK *work;
  
  msgEvent = GMEVENT_Create(
    gsys, NULL, FMenuMsgWinEvent, sizeof(FMENU_MSGWIN_EVENT_WORK));
  work = GMEVENT_GetEventWork( msgEvent );
  MI_CpuClear8( work, sizeof(FMENU_MSGWIN_EVENT_WORK) );
  work->msgBG = msgBG;
  work->heapID = heapID;
  work->strID = strID;
  return msgEvent;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FMenuMsgWinEvent( GMEVENT *event, int *seq, void *wk )
{
  FMENU_MSGWIN_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    work->msgData = FLDMSGBG_CreateMSGDATA(
      work->msgBG, NARC_message_fldmapmenu_dat );
    work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
    FLDMSGWIN_Print( work->msgWin, 0, 0, work->strID );
    GXS_SetMasterBrightness(-16);
    (*seq)++;
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 2:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 3:
    FLDMSGWIN_Delete( work->msgWin );
    GFL_MSG_Delete( work->msgData );
    GXS_SetMasterBrightness(0);
/*    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gameSys );
       
      const FIELD_MENU_ITEM_TYPE type = GAMEDATA_GetSubScreenType( gameData );
      FIELD_SUBSCREEN_SetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(fieldWork) , type );
    }
 */   return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  レポート表示イベント
//======================================================================
//--------------------------------------------------------------
/// FMENU_REPORT_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  u32 heapID;
  FLDMSGBG *msgBG;
  GFL_MSGDATA *msgData;
  FLDMSGWIN *msgWin;
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
}FMENU_REPORT_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * createFMenuReportEvent(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u32 heapID, FLDMSGBG *msgBG )
{
  GMEVENT * msgEvent;
  FMENU_REPORT_EVENT_WORK *work;

  msgEvent = GMEVENT_Create(
    gsys, NULL, FMenuReportEvent, sizeof(FMENU_REPORT_EVENT_WORK));
  work = GMEVENT_GetEventWork( msgEvent );
  MI_CpuClear8( work, sizeof(FMENU_REPORT_EVENT_WORK) );
  work->msgBG = msgBG;
  work->heapID = heapID;
  work->gsys = gsys;
  work->fieldWork = fieldWork;
  return msgEvent;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FMenuReportEvent( GMEVENT *event, int *seq, void *wk )
{
  FMENU_REPORT_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    {
      GAME_COMM_SYS_PTR commSys = GAMESYSTEM_GetGameCommSysPtr( work->gsys );
      work->msgData = FLDMSGBG_CreateMSGDATA(
             work->msgBG, NARC_message_fldmapmenu_dat );
      
      work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
      

      if( GameCommSys_BootCheck(commSys) == GAME_COMM_NO_INVASION )
      {
        //通信中でレポートが書けない
        FLDMSGWIN_Print( work->msgWin, 0, 0, FLDMAPMENU_STR16 );
        GXS_SetMasterBrightness(-16);
        (*seq) = 20;
      }
      else
      {
        { //自機レポートに
          FIELD_PLAYER *fld_player =
            FIELDMAP_GetFieldPlayer( work->fieldWork );
          
          if( FIELD_PLAYER_CheckChangeEventDrawForm(fld_player) == TRUE ){
            MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
            FIELD_PLAYER_GRID *gjiki = FIELDMAP_GetPlayerGrid( work->fieldWork );

            FIELD_PLAYER_GRID_SetRequest( gjiki, FIELD_PLAYER_REQBIT_REPORT );
            FIELD_PLAYER_GRID_UpdateRequest( gjiki );
  
            //ポーズを解除しアニメするように
            MMDL_OffStatusBitMoveProcPause( mmdl );
            MMDL_OffStatusBit( mmdl, MMDL_STABIT_PAUSE_ANM );
          }
        }

        FLDMSGWIN_Print( work->msgWin, 0, 0, FLDMAPMENU_STR14 );
        //本来ならレポート用への切り替え？
        //FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_TOPMENU);
        GXS_SetMasterBrightness(-16);
        (*seq)++;
      }
    }
    break;
  case 1:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      GAMEDATA_SaveAsyncStart(GAMESYSTEM_GetGameData(work->gsys));
      (*seq)++;
    } 
    break;
  case 2:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(GAMESYSTEM_GetGameData(work->gsys));
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        (*seq)++;
        break;
      }
    }
    break;
  case 3:
    FLDMSGWIN_Delete( work->msgWin );
    
    { //自機元に戻す
      FIELD_PLAYER *fld_player =
        FIELDMAP_GetFieldPlayer( work->fieldWork );

      if( FIELD_PLAYER_CheckChangeEventDrawForm(fld_player) == TRUE ){
        FIELDMAP_CTRL_GRID *gridmap =
          FIELDMAP_GetMapCtrlWork( work->fieldWork );
        FIELD_PLAYER_GRID *gjiki = FIELDMAP_GetPlayerGrid( work->fieldWork );
        FIELD_PLAYER_GRID_SetRequest(
            gjiki, FIELD_PLAYER_REQBIT_MOVE_FORM_TO_DRAW_FORM );
        FIELD_PLAYER_GRID_UpdateRequest( gjiki );
      }
    }

    PMSND_PlaySE( SEQ_SE_SAVE );
    work->msgWin = FLDMSGWIN_AddTalkWin( work->msgBG, work->msgData );
    FLDMSGWIN_Print( work->msgWin, 0, 0, FLDMAPMENU_STR15 );
    (*seq)++;
    break;
  case 4:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 5:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 6:
    FLDMSGWIN_Delete( work->msgWin );
    GFL_MSG_Delete( work->msgData );

    //本来ならメニューへの切り替え？
    //FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(mwk->fieldWork), FIELD_SUBSCREEN_TOPMENU);
    GXS_SetMasterBrightness(0);
/*    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gameSys );
       
      const FIELD_MENU_ITEM_TYPE type = GAMEDATA_GetSubScreenType( gameData );
      FIELD_SUBSCREEN_SetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(fieldWork) , type );
    }
*/    return( GMEVENT_RES_FINISH );
    break;
    
    //セーブできません
  case 20:
    if( FLDMSGWIN_CheckPrintTrans(work->msgWin) == TRUE ){
      (*seq)++;
    } 
    break;
  case 21:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        (*seq)++;
      }
    }
    break;
  case 22:
    FLDMSGWIN_Delete( work->msgWin );
    GFL_MSG_Delete( work->msgData );
    GXS_SetMasterBrightness(0);
/*    {
      GAMESYS_WORK *gameSys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gameData = GAMESYSTEM_GetGameData( gameSys );
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gameSys );
      
      const FIELD_MENU_ITEM_TYPE type = GAMEDATA_GetSubScreenType( gameData );
      FIELD_SUBSCREEN_SetTopMenuItemNo( FIELDMAP_GetFieldSubscreenWork(fieldWork) , type );

    }
*/    return( GMEVENT_RES_FINISH );
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}


