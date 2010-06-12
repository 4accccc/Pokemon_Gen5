//=============================================================================
/**
 *
 *	@file		event_battle_call.c
 *	@brief  イベント：バトル呼び出し
 *	@author	hosaka genya
 *	@data		2010.01.22
 *
 */
//=============================================================================
#include <gflib.h>
#include "net/network_define.h"

#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_fieldmap_control.h"

#include "sound/pm_sndsys.h"
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"

#include "../battle/btl_net.h"

#include "../battle/app/vs_multi_list.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#include "net_app/union/union_types.h"
#include "net_app/union/colosseum_types.h"
#include "field/event_fldmmdl_control.h"
#include "field/event_colosseum_battle.h"

#include "gamesystem/btl_setup.h"

#include "field/event_battle_call.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT CommBattleCallProc_Init(  GFL_PROC *proc, int *seq, void* pwk, void* mywk );
static GFL_PROC_RESULT CommBattleCallProc_Main(  GFL_PROC *proc, int *seq, void* pwk, void* mywk );
static GFL_PROC_RESULT CommBattleCallProc_End(  GFL_PROC *proc, int *seq, void* pwk, void* mywk );

const GFL_PROC_DATA CommBattleCommProcData = 
{ 
  CommBattleCallProc_Init,
  CommBattleCallProc_Main,
  CommBattleCallProc_End,
};

//==============================================================================
//  PROC内構造体
//==============================================================================
typedef struct{
  GFL_PROCSYS* procsys_up; 
	VS_MULTI_LIST_PARAM	listParam;
	BOOL battle_overlay_load;
	BOOL battle_rec_load;
}COMM_BTL_DEMO_PROC_WORK;

#define BATTLE_CALL_HEAP_SIZE (0x4000) // PROC内用ヒープのサイズ

//-----------------------------------------------------------------------------
/**
 *	@brief  通信バトル呼び出しPROC 初期化処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	pwk
 *	@param	mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CommBattleCallProc_Init(  GFL_PROC *proc, int *seq, void* pwk, void* mywk )
{
  COMM_BTL_DEMO_PROC_WORK*   work;

  // ヒープ生成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_CALL, BATTLE_CALL_HEAP_SIZE );

  // ワーク アロケート
  work = GFL_PROC_AllocWork( proc , sizeof(COMM_BTL_DEMO_PROC_WORK) , HEAPID_BATTLE_CALL );

  work->procsys_up = GFL_PROC_LOCAL_boot( HEAPID_BATTLE_CALL );
  
  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  通信バトル呼び出しPROC 終了処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	pwk
 *	@param	mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CommBattleCallProc_End(  GFL_PROC *proc, int *seq, void* pwk, void* mywk )
{
  COMM_BTL_DEMO_PROC_WORK*   work = mywk;
  COMM_BATTLE_CALL_PROC_PARAM * bcw = pwk;

  if(bcw->error_auto_disp == TRUE && NetErr_App_CheckError()){
    NetErr_App_ReqErrorDisp();
  }
  
  GFL_PROC_LOCAL_Exit( work->procsys_up );
  
  GFL_PROC_FreeWork( proc );

  // ヒープ開放
  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_CALL );

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  通信バトル呼び出しPROC 主処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	pwk
 *	@param	mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CommBattleCallProc_Main(  GFL_PROC *proc, int *seq, void* pwk, void* mywk )
{
  enum
  {
    SEQ_CALL_START_DEMO = 0,  ///< バトル前デモ呼び出し
    SEQ_WAIT_START_DEMO,

    SEQ_LIST_TIMING_INIT,
    SEQ_LIST_TIMING_WAIT,

		SEQ_CALL_LIST_DEMO,				///< マルチ用リストデモ呼び出し
		SEQ_WAIT_LIST_DEMO,

    SEQ_BATTLE_TIMING_INIT,
    SEQ_BATTLE_TIMING_WAIT,

    SEQ_BATTLE_INIT,
    SEQ_BATTLE_WAIT,
    SEQ_BATTLE_END,
    SEQ_CALL_END_DEMO,        ///< バトル後デモ呼び出し
    SEQ_WAIT_END_DEMO,
    SEQ_CALL_BTL_REC_SEL,     ///< 通信対戦後の録画選択画面
    SEQ_BGM_POP,
    SEQ_END
  };

  COMM_BTL_DEMO_PROC_WORK * work = mywk;
  COMM_BATTLE_CALL_PROC_PARAM * bcw = pwk;
  GFL_PROC_MAIN_STATUS up_status;

  GF_ASSERT(work);
  GF_ASSERT(bcw);

  up_status = GFL_PROC_LOCAL_Main( work->procsys_up );

  //エラー終了
  if(up_status != GFL_PROC_MAIN_VALID && NetErr_App_CheckError()){
    if((*seq) >= SEQ_CALL_START_DEMO && (*seq) < SEQ_CALL_END_DEMO){
      if(work->battle_overlay_load == TRUE){
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
      }
      if(work->battle_rec_load == TRUE){
        BattleRec_Exit();
      }
      return GFL_PROC_RES_FINISH;
    }
  }
  
  switch (*seq) {
  case SEQ_CALL_START_DEMO:
    // 通信バトル前デモ呼び出し
    {
      // マルチバトル判定
      if( bcw->btl_setup_prm->multiMode == 0 )
      {
        bcw->demo_prm->type = COMM_BTL_DEMO_TYPE_NORMAL_START;
      }
      else
      {
        bcw->demo_prm->type = COMM_BTL_DEMO_TYPE_MULTI_START;
      }
      HOSAKA_Printf("comm battle demo type=%d\n",bcw->demo_prm->type);
      GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID( comm_btl_demo ), &CommBtlDemoProcData, bcw->demo_prm);
    }
    (*seq) = SEQ_WAIT_START_DEMO;
    break;
  
  case SEQ_WAIT_START_DEMO:
    if ( up_status != GFL_PROC_MAIN_VALID ){
      if( bcw->btl_setup_prm->multiMode == 0 ){
				(*seq) = SEQ_BATTLE_TIMING_INIT;
			}else{
				(*seq) = SEQ_LIST_TIMING_INIT;
			}
    }
    break;

	case SEQ_LIST_TIMING_INIT:
		GFL_NET_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_LIST_DEMO_TIMING );
		(*seq) = SEQ_LIST_TIMING_WAIT;
		break;

	case SEQ_LIST_TIMING_WAIT:
    if( GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),EVENT_BATTLE_LIST_DEMO_TIMING) ){
      (*seq) = SEQ_CALL_LIST_DEMO;
    }
		break;

	case SEQ_CALL_LIST_DEMO:	// マルチ用リストデモ呼び出し
		if( bcw->btl_setup_prm->commPos == 0 || bcw->btl_setup_prm->commPos == 1 ){
			work->listParam.pos  = VS_MULTI_LIST_POS_LEFT;
			work->listParam.myPP = bcw->demo_prm->trainer_data[COMM_BTL_DEMO_TRDATA_A].party;
			work->listParam.ptPP = bcw->demo_prm->trainer_data[COMM_BTL_DEMO_TRDATA_B].party;
		}else{
			work->listParam.pos  = VS_MULTI_LIST_POS_RIGHT;
			work->listParam.myPP = bcw->demo_prm->trainer_data[COMM_BTL_DEMO_TRDATA_B].party;
			work->listParam.ptPP = bcw->demo_prm->trainer_data[COMM_BTL_DEMO_TRDATA_A].party;
		}
    GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID(vs_multi_list), &VS_MULTI_LIST_ProcData, &work->listParam );
		(*seq) = SEQ_WAIT_LIST_DEMO;
		break;

	case SEQ_WAIT_LIST_DEMO:
    if ( up_status != GFL_PROC_MAIN_VALID ){
      (*seq) = SEQ_BATTLE_TIMING_INIT;
    }
		break;
  
  case SEQ_BATTLE_TIMING_INIT:
    {
      work->battle_overlay_load = TRUE;
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
//    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_ADD_CMD_TBL_TIMING, WB_NET_BATTLE_ADD_CMD );
      GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_ADD_CMD_TBL_TIMING);
      (*seq) = SEQ_BATTLE_TIMING_WAIT;
    }
    break;
  case SEQ_BATTLE_TIMING_WAIT:
//  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_ADD_CMD_TBL_TIMING, WB_NET_BATTLE_ADD_CMD) ){
    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_ADD_CMD_TBL_TIMING)){
      (*seq) = SEQ_BATTLE_INIT;
    }
    break;
  case SEQ_BATTLE_INIT:
    work->battle_rec_load = TRUE;
    BattleRec_Init( HEAPID_BATTLE_CALL );                                // 録画
//  GMEVENT_CallEvent(event, EVENT_FSND_PushPlayNextBGM(gsys, bcw->btl_setup_prm->musicDefault, FSND_FADE_SHORT, FSND_FADE_NONE)); 
    GFL_PROC_LOCAL_CallProc(work->procsys_up, NO_OVERLAY_ID, &BtlProcData, bcw->btl_setup_prm);
    (*seq) = SEQ_BATTLE_WAIT;
    break;
  case SEQ_BATTLE_WAIT:
    if ( up_status != GFL_PROC_MAIN_VALID ){
      (*seq) = SEQ_BATTLE_END;
    }
    break;
  case SEQ_BATTLE_END:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
    work->battle_overlay_load = FALSE;

    BattleRec_LoadToolModule();                       // 録画
    BattleRec_StoreSetupParam( bcw->btl_setup_prm );  // 録画
    BattleRec_UnloadToolModule();                     // 録画

    (*seq) = SEQ_CALL_END_DEMO;
    break;
  case SEQ_CALL_END_DEMO:
    // 通信バトル後デモ呼び出し
    {
      // マルチバトル判定
      if( bcw->btl_setup_prm->multiMode == 0 )
      {
        bcw->demo_prm->type = COMM_BTL_DEMO_TYPE_NORMAL_END;
      }
      else
      {
        bcw->demo_prm->type = COMM_BTL_DEMO_TYPE_MULTI_END;
      }
      HOSAKA_Printf("comm battle demo type=%d\n",bcw->demo_prm->type);

      // 勝敗設定
      switch( bcw->btl_setup_prm->result )
      {
      case BTL_RESULT_RUN_ENEMY:
      case BTL_RESULT_WIN :
        bcw->demo_prm->result = COMM_BTL_DEMO_RESULT_WIN;
        break;
      case BTL_RESULT_LOSE :
      case BTL_RESULT_RUN :
        bcw->demo_prm->result = COMM_BTL_DEMO_RESULT_LOSE;
        break;
      case BTL_RESULT_DRAW :
        bcw->demo_prm->result = COMM_BTL_DEMO_RESULT_DRAW;
        break;
      case BTL_RESULT_COMM_ERROR:
        //OS_Printf( "event_battle_call.c : BTL_RESULT_COMM_ERROR\n" );
        //bcw->demo_prm->result = COMM_BTL_DEMO_RESULT_DRAW;  //通信エラーでもすすめるように  // 通信エラーのときは後の処理を飛ばすことにした。
        break;
      default : 
        GF_ASSERT(0);
        bcw->demo_prm->result = COMM_BTL_DEMO_RESULT_DRAW;  //アサートしてもすすめるように
      }

      if( bcw->btl_setup_prm->result == BTL_RESULT_COMM_ERROR )  // 通信エラーのときは後の処理を飛ばして終了し、コロシアムでエラーメッセージを出してもらう
      {
        (*seq) = SEQ_BGM_POP;
      }
      else
      {
        GFL_PROC_LOCAL_CallProc(work->procsys_up, FS_OVERLAY_ID( comm_btl_demo ), &CommBtlDemoProcData, bcw->demo_prm);
        (*seq) = SEQ_WAIT_END_DEMO;
      }
    }
    break;
  case SEQ_WAIT_END_DEMO:
    if ( up_status != GFL_PROC_MAIN_VALID ){
      (*seq) = SEQ_CALL_BTL_REC_SEL;
    }
    break;
  case SEQ_CALL_BTL_REC_SEL:
    {
      BOOL b_rec = TRUE;  // TRUEのとき、「通信相手 <= 自分」となり録画できる。
      //バトルセットアップの録画バッファがなかったら録画しない
      if( bcw->btl_setup_prm->recBuffer == NULL )
      { 
        b_rec = FALSE;
      }
      else
      {
        // 通信対戦時のサーババーション
//        if( bcw->btl_setup_prm->commServerVer <= BTL_NET_SERVER_VERSION )
        if( BattleRec_ServerVersionCheck( bcw->btl_setup_prm->commServerVer ) )
        {
          // 自分と同じサーバーバージョン OR 自分が一番高いならば録画可能
          b_rec = TRUE;
        }
        else
        {
          // 自分よりも高いサーバーバージョンがいるので録画不可
          b_rec = FALSE;
        }
      }
      // 通信対戦後の録画選択画面へ移行(録画しない人も移行します)
      {
        bcw->btl_rec_sel_param.gamedata  = bcw->gdata;
        bcw->btl_rec_sel_param.b_rec     = b_rec;
        bcw->btl_rec_sel_param.b_sync    = TRUE;
        bcw->btl_rec_sel_param.battle_mode = bcw->battle_mode;
        bcw->btl_rec_sel_param.fight_count = bcw->fight_count;
        GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID( btl_rec_sel ), &BTL_REC_SEL_ProcData, &bcw->btl_rec_sel_param );
      }
      (*seq) = SEQ_BGM_POP;
    }
    break;
  case SEQ_BGM_POP:
    if ( up_status != GFL_PROC_MAIN_VALID){
      BattleRec_Exit();  // 録画
      work->battle_rec_load = FALSE;

      (*seq) = SEQ_END;
    }
    break;
  case SEQ_END:
    return GFL_PROC_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//============================================================================================
//============================================================================================

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------

FS_EXTERN_OVERLAY(battle);


static GMEVENT * EVENT_CommBattle(GAMESYS_WORK * gsys, BATTLE_SETUP_PARAM *btl_setup_prm, COMM_BTL_DEMO_PARAM *demo_prm, BOOL error_auto_disp);
static GMEVENT_RESULT EVENT_CommBattleMain(GMEVENT * event, int *  seq, void * work);


//----------------------------------------------------------------------------
/**
 *	@brief  イベント作成：通信バトル呼び出し
///	GMEVENT_CreateOverlayEventCall関数用コールバック関数
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_CallCommBattle(GAMESYS_WORK * gsys, void *work )
{
  EV_BATTLE_CALL_PARAM* param = work;
  return EVENT_CommBattle( gsys, param->btl_setup_prm, param->demo_prm, param->error_auto_disp );
}


//============================================================================================
//
//    サブイベント
//
//============================================================================================
//--------------------------------------------------------------
//  通信バトル呼び出しイベント用ワーク
//--------------------------------------------------------------
typedef struct{
  // [IN]
  GAMESYS_WORK * gsys;  
  BATTLE_SETUP_PARAM  *btl_setup_prm;
  COMM_BTL_DEMO_PARAM *demo_prm;
  BOOL error_auto_disp;         ///<TRUE:エラーが発生した場合、画面を抜ける時にエラー画面を表示
                                ///<FALSE:エラーが発生しても画面を抜けるだけ
  // [PRIVATE]
  COMM_BATTLE_CALL_PROC_PARAM cbc;
}EVENT_BATTLE_CALL_WORK;

//==================================================================
/**
 * イベント作成：通信バトル呼び出し
 *
 * @param   gsys		
 * @param   para		
 * @param   demo_prm		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
static GMEVENT * EVENT_CommBattle(GAMESYS_WORK * gsys, BATTLE_SETUP_PARAM *btl_setup_prm, COMM_BTL_DEMO_PARAM *demo_prm, BOOL error_auto_disp)
{
  EVENT_BATTLE_CALL_WORK *bcw;
  GMEVENT * event;

  event = GMEVENT_Create( gsys, NULL, EVENT_CommBattleMain, sizeof(EVENT_BATTLE_CALL_WORK) );
  bcw = GMEVENT_GetEventWork(event);
  bcw->gsys = gsys;
  bcw->btl_setup_prm = btl_setup_prm;
  bcw->demo_prm = demo_prm;
  bcw->error_auto_disp = error_auto_disp;
  return event;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  イベント：通信バトル呼び出し
 *
 *	@param	* event
 *	@param	*  seq
 *	@param	* work
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_CommBattleMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_BATTLE_CALL_WORK * bcw = work;
  GAMESYS_WORK * gsys = bcw->gsys;

  switch (*seq) 
  {
  case 0:
    {
      COMM_BATTLE_CALL_PROC_PARAM* prm = &bcw->cbc;

      prm->gdata = GAMESYSTEM_GetGameData( gsys );
      prm->btl_setup_prm = bcw->btl_setup_prm;
      prm->demo_prm = bcw->demo_prm;
      prm->battle_mode = bcw->demo_prm->battle_mode;
      prm->fight_count = bcw->demo_prm->fight_count;
      prm->error_auto_disp = bcw->error_auto_disp;
      GAMESYSTEM_CallProc(gsys, NO_OVERLAY_ID, &CommBattleCommProcData, prm);
    }
    (*seq)++;
    break;

  case 1:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

