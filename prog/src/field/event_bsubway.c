//======================================================================
/**
 * @file  event_bsubway.c
 * @brief  バトルサブウェイ　イベント
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem\btl_setup.h"
#include "event_fieldmap_control.h"
#include "event_fldmmdl_control.h"
#include "poke_tool/poke_regulation.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "app/leader_board.h"
#include "battle/battle.h"

#include "event_battle.h"
#include "field/event_battle_call.h"
#include "event_fieldmap_control.h"
#include "event_ircbattle.h"

#include "savedata/save_tbl.h"

#include "fieldmap.h"

#include "msgdata.h"
#include "field/field_msgbg.h"
#include "print/wordset.h"
#include "message.naix"

#include "bsubway_scr.h"
#include "bsubway_scrwork.h"
#include "scrcmd_bsubway.h"
#include "bsubway_tr.h"


#include "event_bsubway.h"

#include "effect_encount.h"

#include "scrcmd_menuwin.h"

#include "script_message.naix"
#include "../../../resource/message/dst/script/msg_bsubway_scr.h"
#include "../../../resource/message/dst/msg_tower_trainer.h"

#include "system/net_err.h"

FS_EXTERN_OVERLAY(pokelist);

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// EVENT_BSW_COMM_BATTLE_WORK
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  BATTLE_SETUP_PARAM *para;
  COMM_BTL_DEMO_PARAM *demo_prm;
}EVENT_BSW_COMM_BATTLE_WORK;

//======================================================================
//  proto
//======================================================================

//======================================================================
//  バトルサブウェイ　イベント　ポケモンリスト
//======================================================================
//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  
  PSTATUS_DATA status_data;
  PLIST_DATA list_data;
  
  u16 *return_mode;
  u16 *result_select;
  u8 *result_num;
  
  POKEPARTY *pp_btl_box;
}EVENT_WORK_POKE_LIST;

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_SelectPokeList( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_POKE_LIST *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
		GMEVENT_CallEvent( event,
        EVENT_FieldFadeOut_Black(gsys,work->fieldmap,FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 1:
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys,work->fieldmap) );
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_PokeSelect(
          gsys,work->fieldmap,&work->list_data,&work->status_data) );
    (*seq)++;
    break;
  case 3:
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
    (*seq)++;
    break;
  case 4:
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black(
          gsys,work->fieldmap, FIELD_FADE_WAIT) );
    MI_CpuCopy8( work->list_data.in_num, work->result_num, 6 );
    *work->result_select = work->list_data.ret_sel;
    *work->return_mode = work->list_data.ret_mode;
    GFL_HEAP_FreeMemory( work->list_data.reg );
    (*seq)++;
    break;
  case 5:
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント　呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_SetSelectPokeList(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, BOOL btl_box )
{
  GMEVENT *event;
  GAMEDATA *gdata;
  POKEPARTY *pp;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_POKE_LIST *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  event = GMEVENT_Create( gsys, NULL,
      ev_SelectPokeList, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->result_num = bsw_scr->pokelist_select_num;
  work->result_select = &bsw_scr->pokelist_result_select;
  work->return_mode = &bsw_scr->pokelist_return_mode;
  
  if( btl_box == FALSE ){
    pp = GAMEDATA_GetMyPokemon( gdata );
  }else{
    pp = bsw_scr->btl_box_party;
  }
  
  GF_ASSERT( pp != NULL );

  {
    int reg = REG_SUBWAY_SINGLE;
    int type = PL_TYPE_SINGLE;
    PLIST_DATA *list = &work->list_data;
    
    MI_CpuClear8( list, sizeof(PLIST_DATA) );
    list->pp = pp;
    
    switch( bsw_scr->play_mode ){
    case BSWAY_MODE_DOUBLE:
    case BSWAY_MODE_S_DOUBLE:
      reg = REG_SUBWAY_DOUBLE;
      type = PL_TYPE_DOUBLE;
      break;
    case BSWAY_MODE_MULTI:
    case BSWAY_MODE_S_MULTI:
    case BSWAY_MODE_COMM_MULTI:
    case BSWAY_MODE_S_COMM_MULTI:
      reg = REG_SUBWAY_MALTI;
      type = PL_TYPE_MULTI;
      break;
    }
    
#ifdef DEBUG_BSW_PRINT    
    KAGAYA_Printf( "レギュレーション　%d\n", reg );
#endif
    
    list->reg = (void*)PokeRegulation_LoadDataAlloc( reg, HEAPID_PROC );
    list->mode = PL_MODE_BATTLE_SUBWAY;
    list->type = type;
    
    //debug
    #ifdef DEBUG_ONLY_FOR_kagaya
    {
      REGULATION *pReg = list->reg;
      
#ifdef DEBUG_BSW_PRINT    
      KAGAYA_Printf( "BSWAY REG NUM_LO = %d, HI =%d, LV = %d\n",
        pReg->NUM_LO, pReg->NUM_HI, pReg->LEVEL );
#endif
    }
    #endif
  }
  
  {
    PSTATUS_DATA *st = &work->status_data;
    ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gdata );
    MI_CpuClear8( st, sizeof(PSTATUS_DATA) );
    st->ppd = pp;
    st->ppt = PST_PP_TYPE_POKEPARTY;
    st->max = PokeParty_GetPokeCount( pp );
    st->mode = PST_MODE_NORMAL;
    st->page = PPT_INFO;
    st->game_data = gdata;

    st->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
  }
   
  return( event );
}

//======================================================================
//  バトルサブウェイ　戦闘
//======================================================================
//--------------------------------------------------------------
/**
 * バトルサブウェイ　トレーナーバトル
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_TrainerBattle(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT * event;
  BATTLE_SETUP_PARAM *bp;
  
  //対戦録画データを作成する
  BattleRec_Init( HEAPID_PROC );
  
  //バトルパラム作成
  bsw_scr->btl_setup_param =
    BSUBWAY_SCRWORK_CreateBattleParam( bsw_scr, gsys );
  
  if( bsw_scr->play_mode == BSWAY_MODE_COMM_MULTI ||
      bsw_scr->play_mode == BSWAY_MODE_S_COMM_MULTI ){
    return BSUBWAY_EVENT_CommBattle( bsw_scr, gsys, fieldmap );
  }else{
    event = EVENT_BSubwayTrainerBattle(
        gsys, fieldmap, bsw_scr->btl_setup_param );
  }
  
  return( event );
}

//======================================================================
//  バトルサブウェイ　イベント　対戦前メッセージ
//======================================================================
//--------------------------------------------------------------
/// 対戦前メッセージ
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  BSUBWAY_SCRWORK *bsw_scr;
  
  VecFx32 balloonWinPos;
  FLDTALKMSGWIN *balloonWin;
  STRBUF *strBuf;
  u16 obj_id;
  u8 padding[2];
}EVENT_WORK_TRAINER_BEFORE_MSG;

//--------------------------------------------------------------
/**
 * 対戦前メッセージ
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_TrainerMsgBefore(
    GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_TRAINER_BEFORE_MSG *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
    if( FLDTALKMSGWIN_Print(work->balloonWin) == FALSE ){
      break;
    }
    (*seq)++;
  case 1:
    FLDTALKMSGWIN_WriteKeyWaitCursor( work->balloonWin );
     
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      FLDTALKMSGWIN_StartClose( work->balloonWin );
      (*seq)++;
    }
    break;
  case 2:
    if( FLDTALKMSGWIN_WaitClose(work->balloonWin) == TRUE ){
      GFL_STR_DeleteBuffer( work->strBuf );
      return( GMEVENT_RES_FINISH );
    }
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * 対戦前メッセージ 呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_TrainerBeforeMsg(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys,
    int tr_idx, u16 obj_id, u8 pos_type )
{
  GMEVENT *event;
  GAMEDATA *gdata;
  FLDMSGBG *msgbg;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_TRAINER_BEFORE_MSG *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  msgbg = FIELDMAP_GetFldMsgBG( fieldmap );
  
  event = GMEVENT_Create( gsys, NULL,
    ev_TrainerMsgBefore, sizeof(EVENT_WORK_POKE_LIST) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->obj_id = obj_id;

  if( bsw_scr->tr_data[tr_idx].bt_trd.appear_word[0] == 0xffff ){ //ROM MSG
    u16 msg_no = 0;
    GFL_MSGDATA *msgdata;
    
    work->strBuf = GFL_STR_CreateBuffer( 512, HEAPID_PROC );
    
    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    msg_no = bsw_scr->tr_data[tr_idx].bt_trd.appear_word[1];

#ifdef DEBUG_BSW_PRINT    
    KAGAYA_Printf( "BSW TRAINER BEFORE MSG : IDX = %d, ROM MSG NUM %d\n",
        tr_idx, msg_no );
#endif

    if( msg_no >= msg_tower_trainer_max ){
      GF_ASSERT( 0 && "ERROR BSW TRAINER MSG : INDEX OVER" );
      msg_no = 0;
    }
    
    GFL_MSG_GetString( msgdata, msg_no, work->strBuf );
    GFL_MSG_Delete( msgdata );
  }else{ //簡易会話
    PMS_DATA *pms = (PMS_DATA*)bsw_scr->tr_data[tr_idx].bt_trd.appear_word;
    work->strBuf = PMSDAT_ToString( pms, HEAPID_PROC );
#ifdef DEBUG_BSW_PRINT    
    KAGAYA_Printf( "BSW TRAINER BEFORE MSG : IDX = %d, KAIWA MSG\n", tr_idx );
#endif
  }
  
  { //mmdl
    MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys( gdata );
    MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, work->obj_id );
    MMDL_GetVectorPos( mmdl, &work->balloonWinPos );
  }
  
  { //offset
    VecFx32 offs;
    const FIELD_CAMERA *fld_camera =
      FIELDMAP_GetFieldCamera( fieldmap );
    const GFL_G3D_CAMERA *g3d_camera =
      FIELD_CAMERA_GetCameraPtr( fld_camera );
    
    EvCmdBalloonWin_GetOffsetPos(
        &work->balloonWinPos, &offs, 
        g3d_camera, fld_camera, pos_type );
    
    work->balloonWinPos.x += offs.x;
    work->balloonWinPos.y += offs.y;
    work->balloonWinPos.z += offs.z;
  }
  
  { //吹き出しウィンドウ
    FLDTALKMSGWIN_IDX win;
    TAIL_SETPAT tail;
    
    EvCmdBalloonWin_GetWinType( pos_type, &win, &tail );
    
    work->balloonWin = FLDTALKMSGWIN_AddStrBuf(
       msgbg,
       win,
       &work->balloonWinPos,
       work->strBuf, TALKMSGWIN_TYPE_NORMAL, tail );
  }
  
  return( event );
}

//======================================================================
//  歴代リーダー情報の表示
//======================================================================
//-------------------------------------
///	イベントワーク
//=====================================
typedef struct {
  FIELDMAP_WORK* fieldmap;
  GAMESYS_WORK* gsys;
  LEADERBOARD_PARAM param;
}EVENT_WORK_CALL_LEADERBOARD;

static GMEVENT_RESULT ev_CallLeaderBoard(
    GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_CALL_LEADERBOARD* work = wk;
  
  switch( *seq ){
  case 0:
    {
      GMEVENT* ev;
      BSUBWAY_WIFI_DATA* bsw_wifi;

      work->param.gamedata = GAMESYSTEM_GetGameData( work->gsys );

      //BTS：5355対処
      bsw_wifi = GAMEDATA_GetBSubwayWifiData( work->param.gamedata );
      work->param.rank_no   = BSUBWAY_WIFIDATA_GetLeaderRank( bsw_wifi );
      work->param.train_no  = BSUBWAY_WIFIDATA_GetLeaderRoomNo( bsw_wifi );

      ev = EVENT_FieldSubProc( work->gsys, work->fieldmap,
		          FS_OVERLAY_ID(leader_board), &LeaderBoardProcData, &work->param);
  		GMEVENT_CallEvent( event, ev );

      (*seq) ++;
    }
    break;
  case 1:
    // 終了
    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  歴代リーダーボード呼び出し
 */
//-----------------------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_CallLeaderBoard( GAMESYS_WORK *gsys )
{
  GMEVENT * event;
  EVENT_WORK_CALL_LEADERBOARD* work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );


  event = GMEVENT_Create( gsys, NULL,
    ev_CallLeaderBoard, sizeof(EVENT_WORK_CALL_LEADERBOARD) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys      = gsys;
  work->fieldmap  = fieldmap;

  return event;
}

//======================================================================
//  通信バトル
//======================================================================
#include "net/network_define.h"

static GFL_PROC_RESULT CommBattleCallProc_Init(
    GFL_PROC *proc, int *seq, void* pwk, void* mywk );
static GFL_PROC_RESULT CommBattleCallProc_Main(
    GFL_PROC *proc, int *seq, void* pwk, void* mywk );
static GFL_PROC_RESULT CommBattleCallProc_End(
    GFL_PROC *proc, int *seq, void* pwk, void* mywk );

const GFL_PROC_DATA bsw_CommBattleCommProcData = 
{ 
  CommBattleCallProc_Init,
  CommBattleCallProc_Main,
  CommBattleCallProc_End,
};

typedef struct{
  GFL_PROCSYS* procsys_up; 
}COMM_BTL_DEMO_PROC_WORK;

typedef struct{
  // [IN]
  GAMESYS_WORK * gsys;  
  BATTLE_SETUP_PARAM  *btl_setup_prm;
  COMM_BTL_DEMO_PARAM *demo_prm;
  // [PRIVATE]
  COMM_BATTLE_CALL_PROC_PARAM cbc;
}BSW_EVENT_BATTLE_CALL_WORK;

#define BATTLE_CALL_HEAP_SIZE (0x4000) // PROC内用ヒープのサイズ
#define BATTLE_ADD_CMD_TBL_TIMING 200 ///< 同期NO

// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];

FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------
/**
 *	@brief  通信バトル呼び出しPROC 初期化処理
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	pwk
 *	@param	mywk 
 *	@retval
 */
//-------------------------------------------------------
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

//---------------------------------------------------
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
//--------------------------------------------------
static GFL_PROC_RESULT CommBattleCallProc_End(  GFL_PROC *proc, int *seq, void* pwk, void* mywk )
{
  COMM_BTL_DEMO_PROC_WORK*   work = mywk;

  GFL_PROC_LOCAL_Exit( work->procsys_up );
  GFL_PROC_FreeWork( proc );
  
  // ヒープ開放
  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_CALL );
  return GFL_PROC_RES_FINISH;
}

//-------------------------------------------------
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
//------------------------------------------------
static GFL_PROC_RESULT CommBattleCallProc_Main(
    GFL_PROC *proc, int *seq, void* pwk, void* mywk )
{
  enum
  {
    SEQ_BATTLE_TIMING_INIT,
    SEQ_BATTLE_TIMING_WAIT,
    SEQ_BATTLE_INIT,
    SEQ_BATTLE_WAIT,
    SEQ_BATTLE_END,
    SEQ_END
  };
  
  COMM_BTL_DEMO_PROC_WORK *work = mywk;
  COMM_BATTLE_CALL_PROC_PARAM * bcw = pwk;
  GFL_PROC_MAIN_STATUS up_status;
  
  GF_ASSERT(work);
  GF_ASSERT(bcw);
  
  up_status = GFL_PROC_LOCAL_Main( work->procsys_up );
  
  switch (*seq) {
  case SEQ_BATTLE_TIMING_INIT:
    {
      GFL_OVERLAY_Load( FS_OVERLAY_ID(battle) );
      
      GFL_NET_AddCommandTable(
          GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL );
      GFL_NET_TimingSyncStart(
          GFL_NET_HANDLE_GetCurrentHandle(), BATTLE_ADD_CMD_TBL_TIMING );
      
      #ifdef DEBUG_BSW_PRINT
      KAGAYA_Printf("戦闘用通信コマンドテーブルをAddしたので同期取り\n");
      #endif
      
      (*seq) = SEQ_BATTLE_TIMING_WAIT;
    }
    break;
  case SEQ_BATTLE_TIMING_WAIT:
    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE ){
      #ifdef DEBUG_BSW_PRINT
      KAGAYA_Printf("戦闘用通信開始同期待ちでエラーが発生\n" );
      #endif
      (*seq) = SEQ_BATTLE_END;
      break;
    }
    
    if( GFL_NET_IsTimingSync(
          GFL_NET_HANDLE_GetCurrentHandle(),BATTLE_ADD_CMD_TBL_TIMING) ){
      (*seq) = SEQ_BATTLE_INIT;
      #ifdef DEBUG_BSW_PRINT
      KAGAYA_Printf("戦闘用通信コマンドテーブルをAdd後の同期取り完了\n");
      #endif
    }
    break;
  case SEQ_BATTLE_INIT:
    GFL_PROC_LOCAL_CallProc(
        work->procsys_up, NO_OVERLAY_ID, &BtlProcData, bcw->btl_setup_prm);
    (*seq) = SEQ_BATTLE_WAIT;
    break;
  case SEQ_BATTLE_WAIT:
    if( up_status != GFL_PROC_MAIN_VALID ){
      (*seq) = SEQ_BATTLE_END;
    }
    break;
  case SEQ_BATTLE_END:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
    
    #if 0 //必要なくなった
    BattleRec_LoadToolModule();                       // 録画
    BattleRec_StoreSetupParam( bcw->btl_setup_prm );  // 録画
    BattleRec_UnloadToolModule();                     // 録画
    #endif
    
    (*seq) = SEQ_END;
    
    #ifdef DEBUG_BSW_PRINT    
    KAGAYA_Printf("バトル完了\n");
    #endif
    break;
  case SEQ_END:
    return GFL_PROC_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------
/**
 *	@brief  イベント：通信バトル呼び出し
 *	@param	* event
 *	@param	*  seq
 *	@param	* work
 *	@retval
 */
//----------------------------------------------------
static GMEVENT_RESULT bsw_EventCommBattleMain(
    GMEVENT * event, int *  seq, void * work)
{
  BSW_EVENT_BATTLE_CALL_WORK *bcw = work;
  GAMESYS_WORK * gsys = bcw->gsys;
  
  switch (*seq) 
  {
  case 0:
    {
      COMM_BATTLE_CALL_PROC_PARAM *prm = &bcw->cbc;
      prm->gdata = GAMESYSTEM_GetGameData( gsys );
      prm->btl_setup_prm = bcw->btl_setup_prm;
      prm->demo_prm = bcw->demo_prm;
      GAMESYSTEM_CallProc( gsys, NO_OVERLAY_ID,
          &bsw_CommBattleCommProcData, prm);
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
static GMEVENT * bsw_CommBattle(GAMESYS_WORK * gsys, BATTLE_SETUP_PARAM *btl_setup_prm, COMM_BTL_DEMO_PARAM *demo_prm)
{
  BSW_EVENT_BATTLE_CALL_WORK *bcw;
  GMEVENT * event;

  event = GMEVENT_Create(
      gsys, NULL, bsw_EventCommBattleMain,
      sizeof(BSW_EVENT_BATTLE_CALL_WORK) );
  bcw = GMEVENT_GetEventWork(event);
  bcw->gsys = gsys;
  bcw->btl_setup_prm = btl_setup_prm;
  bcw->demo_prm = demo_prm;
  return event;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　通信バトル　メイン
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT bsw_CommBattleMain(
    GMEVENT *event, int *seq, void *wk )
{
  EVENT_BSW_COMM_BATTLE_WORK *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  FIELDMAP_WORK *fieldmap = work->fieldmap;
  
  switch (*seq){
  case 0: //NPCポーズ
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, work->fieldmap) );
    (*seq)++;
    break;
  case 1: //戦闘BGM再生
    GMEVENT_CallEvent( event, 
      EVENT_FSND_PushPlayBattleBGM(gsys,work->para->musicDefault) ); 
    (*seq)++;
    break;
  case 2: //エンカウントエフェクト
    EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap) );
    ENCEFF_SetEncEff(
        FIELDMAP_GetEncEffCntPtr(work->fieldmap), event, ENCEFFID_SUBWAY );
    (*seq)++;
    break;
  case 3: //フィールドクローズ
    GMEVENT_CallEvent( event, EVENT_FieldClose(gsys,work->fieldmap) );
    (*seq)++;
    break;
  case 4: //戦闘
    GMEVENT_CallEvent( event,
        bsw_CommBattle(gsys,work->para,work->demo_prm) );
    (*seq)++;
    break;
  case 5: //フィールドBGM復帰
    GMEVENT_CallEvent( event, EVENT_FSND_PopPlayBGM_fromBattle(gsys) );
    (*seq)++;
    break;
  case 6: //フィールド復帰
    #if 0
    BATTLE_PARAM_Delete( work->para ); //SETUP_PARAM解放はサブウェイ側で
    #endif
    
    #ifdef DEBUG_BSW_PRINT    
    KAGAYA_Printf("_FIELD_OPEN\n");
    #endif

    GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
    (*seq) ++;
    break;
  case 7: //BGMの復帰待ち
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMPop( gsys ) );
    (*seq)++;
    break;
  case 8: //フェードイン
    {
      GMEVENT* fade_event = EVENT_FieldFadeIn_Black(
          gsys, work->fieldmap, FIELD_FADE_WAIT );
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq)++;
    break;
  case 9:
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    return GMEVENT_RES_FINISH;
    break;
  }
  
  return GMEVENT_RES_CONTINUE;
}

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
static GMEVENT * ev_CommBattle(
    GAMESYS_WORK * gsys,
    FIELDMAP_WORK *fieldmap,
    BATTLE_SETUP_PARAM *btl_setup_prm,
    COMM_BTL_DEMO_PARAM *demo_prm)
{
  EVENT_BSW_COMM_BATTLE_WORK *bcw;
  GMEVENT * event;

  event = GMEVENT_Create(
      gsys, NULL, bsw_CommBattleMain, sizeof(EVENT_BSW_COMM_BATTLE_WORK) );
  bcw = GMEVENT_GetEventWork(event);
  bcw->gsys = gsys;
  bcw->para = btl_setup_prm;
  bcw->demo_prm = demo_prm;
  bcw->fieldmap = fieldmap;
  return event;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　通信バトル
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_CommBattle(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT * event;
  BATTLE_SETUP_PARAM *bp;
  COMM_BTL_DEMO_PARAM *demo;
  
  demo = BSUBWAY_SCRWORK_CreateBattleDemoParam( bsw_scr, gsys );
  event = ev_CommBattle( gsys, fieldmap, bsw_scr->btl_setup_param, demo );
  return( event );
}

//======================================================================
//  バトルサブウェイ　WiFiホームNPCメッセージ表示
//======================================================================
//--------------------------------------------------------------
/// 
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  BSUBWAY_SCRWORK *bsw_scr;
  
  VecFx32 balloonWinPos;
  FLDTALKMSGWIN *balloonWin;
  STRBUF *strBuf;
}EVENT_WORK_MSG_WIFI_HOME_NPC;

//--------------------------------------------------------------
/**
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_MsgWifiHomeNPC(
    GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_MSG_WIFI_HOME_NPC *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
    if( FLDTALKMSGWIN_Print(work->balloonWin) == TRUE ){
      (*seq)++;
    }
    break;
  case 1:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      FLDTALKMSGWIN_StartClose( work->balloonWin );
      (*seq)++;
    }
    break;
  case 2:
    if( FLDTALKMSGWIN_WaitClose(work->balloonWin) == TRUE ){
      GFL_STR_DeleteBuffer( work->strBuf );
      return( GMEVENT_RES_FINISH );
    }
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_MsgWifiHomeNPC(
    GAMESYS_WORK *gsys, u16 leader_no, u16 obj_id )
{
  MMDL *mmdl;
  PMS_DATA *pms;
  GMEVENT *event;
  GAMEDATA *gdata;
  FLDMSGBG *msgbg;
  u8 pos_type;
  FIELDMAP_WORK *fieldmap;
  BSUBWAY_LEADER_DATA *leader;
  EVENT_WORK_MSG_WIFI_HOME_NPC *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  msgbg = FIELDMAP_GetFldMsgBG( fieldmap );
  
  event = GMEVENT_Create( gsys, NULL,
    ev_MsgWifiHomeNPC, sizeof(EVENT_WORK_MSG_WIFI_HOME_NPC) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  
  { //mmdl
    MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys( gdata );
    MMDL *jiki = MMDLSYS_SearchMMdlPlayer( mmdlsys );
    
    mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
    MMDL_GetVectorPos( mmdl, &work->balloonWinPos );
  
    //pos type
    pos_type = EvCmdBalloonWin_GetWinTypeDefault(
        jiki, &work->balloonWinPos );
  }
  
  { //offset
    VecFx32 offs;
    const FIELD_CAMERA *fld_camera =
      FIELDMAP_GetFieldCamera( fieldmap );
    const GFL_G3D_CAMERA *g3d_camera =
      FIELD_CAMERA_GetCameraPtr( fld_camera );
    
    EvCmdBalloonWin_GetOffsetPos(
        &work->balloonWinPos, &offs, 
        g3d_camera, fld_camera, pos_type );
    
    work->balloonWinPos.x += offs.x;
    work->balloonWinPos.y += offs.y;
    work->balloonWinPos.z += offs.z;
  }

  { //leader data
    SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
    BSUBWAY_WIFI_DATA *wifiData = SaveControl_DataPtrGet(
        save, GMDATA_ID_BSUBWAY_WIFIDATA );
    leader = BSUBWAY_WIFIDATA_GetLeaderDataAlloc( wifiData, HEAPID_PROC );
  }  
  
  { //簡易会話
    pms = (PMS_DATA*)leader[leader_no].leader_word;
    work->strBuf = PMSDAT_ToString( pms, HEAPID_PROC );
  }
  
  //free leader
  GFL_HEAP_FreeMemory( leader );
  
  { //吹き出しウィンドウ
    FLDTALKMSGWIN_IDX win;
    TAIL_SETPAT tail;
    
    EvCmdBalloonWin_GetWinType( pos_type, &win, &tail );
    
    work->balloonWin = FLDTALKMSGWIN_AddStrBuf(
        msgbg,
        win,
        &work->balloonWinPos,
        work->strBuf, TALKMSGWIN_TYPE_NORMAL, tail );
  }
  
  return( event );
}

//======================================================================
//  バトルサブウェイ wifi
//======================================================================
#if 0
///Wifiイベントワーク
typedef struct _BTWR_WIFI_EVENT
{
	int	ret_val;
	int	seq;
	WBTOWER_PARAM*	app_wk;
	u16**			scr_ret_wk;
	u16				scr_ret_wkno;
	u16	mode;
	u16	dpw_code;
}BTWR_WIFI_EVENT;

typedef enum
{
	BTWR_WIFI_CONNECT_CALL,
	BTWR_WIFI_CONNECT_WAIT,
	BTWR_WIFI_EV_EXIT,
};

//--------------------------------------------------------------
/// EVENT_WORK_WIFI_CONNECT
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  u16 mode;
  u16 ret_wk_no;
  u16 dpw_code;

  GAMESYS_WORK *gsys;
  BSUBWAY_SCRWORK *bsw_scr;
}EVENT_WORK_WIFI_CONNECT;

/**
 *	@brief	バトルタワー　WiFiコネクト呼び出し
 */
static int BtlTower_WifiConnectCall(
    BTWR_WIFI_EVENT* wk, FIELDSYS_WORK* fsys )
{
	SAVEDATA* sv;
  
	if( mydwc_checkMyGSID() ){
		wk->app_wk = Field_WifiBattleTowerConnect_SetProc(
        fsys,wk->mode,wk->dpw_code);
		return BTWR_WIFI_CONNECT_WAIT;
	}else{
		wk->ret_val = WIFI_BTOWER_RESULT_CANCEL;
		return BTWR_WIFI_EV_EXIT;
	}
}

/**
 *	@brief	バトルタワー　WiFiコネクト終了待ち
 */
static int BtlTower_WifiConnectWait(BTWR_WIFI_EVENT* wk,FIELDSYS_WORK* fsys)
{
	u8	i;
	WBTOWER_PARAM	*param;

	// サブプロセス終了待ち
	if( FieldEvent_Cmd_WaitSubProcEnd(fsys) ) {
		return BTWR_WIFI_CONNECT_WAIT;
	}
//	param = (WBTOWER_PARAM*)wk->app_wk;

	//ステータスを取得
	wk->ret_val = wk->app_wk->result;//param->result;
#ifdef DEBUG_BSW_PRINT    
	KAGAYA_Printf("wifi_ret_val = %d,%d\n",wk->app_wk->result,wk->ret_val);
#endif
	//メモリ解放
	sys_FreeMemoryEz(wk->app_wk);
	return BTWR_WIFI_EV_EXIT;
}

//--------------------------------------------------------------
/**
 * @brief	ゲームイベントコントローラ　WiFi接続
 * @param	ev	GMEVENT_CONTROL *
 * @retval	BOOL	TRUE=イベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT BtlTowerEv_WifiConnect(
    GMEVENT *event, int *seq, void *work )
{
	u16	*buf16;
	BTWR_WIFI_EVENT *wk = work;
  
	switch( (*seq) ){
	case BTWR_WIFI_CONNECT_CALL:
		(*seq) = BtlTower_WifiConnectCall(wk,fsys);
		break;
	case BTWR_WIFI_CONNECT_WAIT:
		(*seq) = BtlTower_WifiConnectWait(wk,fsys);
		break;
	case BTWR_WIFI_EV_EXIT:
		//戻り値指定
		buf16 = GetEventWorkAdrs(fsys,wk->scr_ret_wkno);//*(wk->scr_ret_wk);
#ifdef DEBUG_BSW_PRINT    
		KAGAYA_Printf("wifi_ret_val = %d\n",wk->ret_val);
#endif
		*buf16 = wk->ret_val;
	  return GMEVENT_RES_FINISH;
	}
  
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief	WiFi接続　サブイベント呼び出し
 * @param	event	GMEVENT_CONTROL*
 * @param	mode	接続モード
 * @param	ret_wk_no	返り値を返すワークID
 * @param	dpw_code	DPW_INIT_PROCが返した返り値
 * @retval  nothing
 */
//--------------------------------------------------------------
void EventCmd_BTowerWifiCall(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap,
    GMEVENT *m_event, u16 mode, u16 ret_wk_no, u16 dpw_code )
{
  GMEVENT *event;
	BTWR_WIFI_EVENT *wk;
  
  event = GMEVENT_Create( gsys, NULL,
      BtlTowerEv_WifiConnect, sizeof(BTWR_WIFI_EVENT) );

  wk = GMEVENT_GetEventWork( event );
	wk->mode = mode;
	wk->dpw_code = dpw_code;
	wk->scr_ret_wkno = ret_wk_no;
}
#endif

#if 0
//==============================================================================
/**
 * $brief   WIFIバトルタワー接続画面を呼び出す
 *
 * @param   fsys		
 *
 * @retval  none		
 */
//==============================================================================
static void * Field_WifiBattleTowerConnect_SetProc( FIELDSYS_WORK *fsys, int mode, int connect )
{
	WBTOWER_PARAM *param;
	FS_EXTERN_OVERLAY(dpw_tower);
  
  WIFILOGIN_PARAM *p_param;
  
  work->p_sub_wk = GFL_HEAP_AllocClearMemory(
      HEAPID_PROC, sizeof(WIFILOGIN_PARAM) );
  
  p_param = work->p_sub_wk;
  p_param->gamedata = GAMESYSTEM_GetGameData( gsys );
  
  GAMESYSTEM_CallProc( gsys, FS_OVERLAY_ID(wifi_login),
      &WiFiLogin_ProcData, work->p_sub_wk );
  
  (*seq)++;
  break;
  
  if( GAMESYSTEM_IsProcExist(gsys) == GFL_PROC_MAIN_NULL ){
    WIFILOGIN_PARAM *p_param = work->p_sub_wk;
    
    if( p_param->result == WIFILOGIN_RESULT_LOGIN ){ 
      (*seq)++;
    }else if( p_param->result == WIFILOGIN_RESULT_CANCEL )
      { 
        (*seq)  = _WAIT_NET_END;
      }
      GFL_HEAP_FreeMemory( work->p_sub_wk );
    }
    
    p_param = work->

	// プロセス定義データ
	const PROC_DATA WBTowerProcData = {
		WBTowerProc_Init,
		WBTowerProc_Main,
		WBTowerProc_End,
		FS_OVERLAY_ID(dpw_tower),
	};
	

	param = sys_AllocMemoryLo(HEAPID_WORLD, sizeof(WBTOWER_PARAM));
  
	param->btowerscore   = SaveData_GetTowerScoreData(fsys->savedata);
	param->wifitowerdata = SaveData_GetTowerWifiData(fsys->savedata);
	param->systemdata    = SaveData_GetSystemData(fsys->savedata);
	param->config        = SaveData_GetConfig(fsys->savedata);
	param->MyUserData    = WifiList_GetMyUserInfo( SaveData_GetWifiListData(fsys->savedata) );
	param->savedata      = fsys->savedata;
	param->profileId     = mydwc_getMyGSID(SaveData_GetWifiListData(fsys->savedata));
	param->mode          = mode;	// btower.h参照
	param->connect       = connect;

#if 1
	//プラチナBTS 通信 090
	//原因：resultの初期化が行われていない
	//
	//DPの時は、resultに適当な値が入っていたので、
	//スクリプトでWIFI_BTOWER_RESULT_SUCCESSの分岐に引っかからなかった。
	//
	//WBTOWER_PARAMを0クリアすると、何があるかわからないので、resultのみキャンセルを代入
	param->result		= WIFI_BTOWER_RESULT_CANCEL;
#endif

	GameSystem_StartSubProc(fsys,  &WBTowerProcData, param );
	return (void*)param;
} 

//--------------------------------------------------------------
/**
 * @brief	バトルタワー　WiFiコネクト呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
static int call_WifiConnect( EVENT_WORK_WIFI_CONNECT *wk )
{
	if( mydwc_checkMyGSID() == TRUE ){
    /*
		wk->app_wk = Field_WifiBattleTowerConnect_SetProc(
        fsys,wk->mode,wk->dpw_code);
		return BTWR_WIFI_CONNECT_WAIT;
    */
	}
  
  wk->ret_val = WIFI_BTOWER_RESULT_CANCEL;
	return BTWR_WIFI_EV_EXIT;
}

//--------------------------------------------------------------
/**
 * @brief	ゲームイベントコントローラ　WiFi接続
 * @param	ev	GMEVENT_CONTROL *
 * @retval	BOOL	TRUE=イベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_WifiConnect( GMEVENT *ev, int *seq, void *wk )
{
	u16	*buf16;
  EVENT_WORK_WIFI_CONNECT *work = wk;
  
	switch( (*seq) ){
  case 0:
    if( mydwc_checkMyGSID() == FALSE ){ //error
      work->ret_val = WIFI_BTOWER_RESULT_CANCEL;
      (*seq) = exit;
      break;
    }
    //FALL THROUGH	
  case 1:
    if( GAME_COMM_NO_NULL == GameCommSys_BootCheck(
          GAMESYSTEM_GetGameCommSysPtr(gsys)) ){
      work->fieldmap = NULL; //クローズしたら使用禁止！
      (*seq)++;
    }
    break;
  case 2:
    GAMESYSTEM_CallProc( gsys, FS_OVERLAY_ID(wifi_login),
        &WiFiLogin_ProcData, &dbw->login );
    (*seq)++;
    break;
  case 3:
    if( GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL ){
      if( work->login.result == WIFILOGIN_RESULT_LOGIN ){ 
        (*seq)++;
      }else{ 
        (*seq) = _WAIT_NET_END;
      }
    }
    break;
  case 4:
    //タワーデータ処理へ
    (*seq)++;
    break;
  case 5:
    //タワーデータ処理終了待ち
    if( GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL ){
      (*seq)++;
      dbw->aPokeTr.ret = POKEMONTRADE_MOVE_START;
    }
    break;
	case 6:
		//戻り値指定
		buf16 = GetEventWorkAdrs(fsys,wk->scr_ret_wkno);//*(wk->scr_ret_wk);
#ifdef DEBUG_BSW_PRINT    
		KAGAYA_Printf("wifi_ret_val = %d\n",wk->ret_val);
#endif
		*buf16 = wk->ret_val;
		sys_FreeMemoryEz(wk);
		return TRUE;
	}
	return FALSE;
}


//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * BSUBWAY_EVENT_SetWifiConnect(
    BSUWBAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys,
    u16 mode, u16 ret_wk_no, u16 dpw_code )
{
  GMEVENT *event;
  EVENT_WORK_WIFI_CONNECT *work;
  
  event = GMEVENT_Create( gsys, NULL,
      NULL, sizeof(EVENT_WORK_WIFI_CONNECT) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->bsw_scr = bsw_scr;
  work->mode = mode;
  work->ret_wk_no = ret_wk_no;
  work->dpw_code = dpw_code;
}
#endif



