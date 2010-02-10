//======================================================================
/**
 * @file	irc_battle_subproc.h
 * @brief	赤外線バトル リスト・ステータス呼び出し->WIFI_MATCHへもってきてPROC化
 * @author	ariizumi -> nagihashi
 * @data	09/01/19
 *
 */
//======================================================================
#include <gflib.h>

#include "app/pokelist.h"
#include "app/p_status.h"
#include "savedata/mystatus.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"

#include "wifibattlematch_subproc.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  IBSS_INIT_POKELIST,
  IBSS_MAIN_POKELIST,
  IBSS_EXIT_POKELIST,
  IBSS_INIT_POKESTATUS,
  IBSS_MAIN_POKESTATUS,
  IBSS_EXIT_POKESTATUS,
}WIFIBATTLEMATCH_SUBPROC_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  WIFIBATTLEMATCH_SUBPROC_STATE state;
  
  POKEPARTY *pokeParty;
  
  PLIST_DATA plData;
  PSTATUS_DATA psData;
  
  u8 timeCnt;
  u32 befVCount;
  
  GFL_PROCSYS *procSys;

  WIFIBATTLEMATCH_SUBPROC_PARAM  param;
  HEAPID  heapId;
}WIFIBATTLEMATCH_SUBPROC_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void WIFIBATTLEMATCH_SUBPROC_InitListData( WIFIBATTLEMATCH_SUBPROC_PARAM *p_param, WIFIBATTLEMATCH_SUBPROC_WORK *procWork , PLIST_DATA *plData );
static void WIFIBATTLEMATCH_SUBPROC_InitStatusData( WIFIBATTLEMATCH_SUBPROC_PARAM *p_param, WIFIBATTLEMATCH_SUBPROC_WORK *procWork , PSTATUS_DATA *psData );

static GFL_PROC_RESULT WIFIBATTLEMATCH_SUB_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_SUB_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_SUB_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );



FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(poke_status);


const GFL_PROC_DATA WifiBattleMatch_Sub_ProcData =
{ 
  WIFIBATTLEMATCH_SUB_PROC_Init,
  WIFIBATTLEMATCH_SUB_PROC_Main,
  WIFIBATTLEMATCH_SUB_PROC_Exit,
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_SUB_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  u8 i;
  WIFIBATTLEMATCH_SUBPROC_WORK   *procWork;
  WIFIBATTLEMATCH_SUBPROC_PARAM  *p_param  = p_param_adrs;
  
  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_SUB, 0x30000 );

  procWork = GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_SUBPROC_WORK), HEAPID_WIFIBATTLEMATCH_SUB );
  GFL_STD_MemClear( procWork, sizeof(WIFIBATTLEMATCH_SUBPROC_WORK) );

  procWork->param   = *p_param;
  procWork->heapId  = HEAPID_WIFIBATTLEMATCH_SUB;
  
  procWork->procSys = GFL_PROC_LOCAL_boot( procWork->heapId );

  procWork->pokeParty = PokeParty_AllocPartyWork( procWork->heapId );
  for( i=0;i<TEMOTI_POKEMAX;i++ )
  {
    if( p_param->ppp[i] )
    { 
      if( PPP_Get( p_param->ppp[i] , ID_PARA_poke_exist , NULL ) )
      {
        POKEMON_PARAM *pp = PP_CreateByPPP( p_param->ppp[i] , procWork->heapId );
        PokeParty_Add( procWork->pokeParty , pp );
        GFL_HEAP_FreeMemory( pp );
      }
    }
  }
  
  WIFIBATTLEMATCH_SUBPROC_InitListData( p_param, procWork , &procWork->plData );
  WIFIBATTLEMATCH_SUBPROC_InitStatusData( p_param, procWork , &procWork->psData );
  
  procWork->timeCnt = 0;
  procWork->befVCount = OS_GetVBlankCount();
  procWork->state = IBSS_INIT_POKELIST;

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_SUB_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{

  WIFIBATTLEMATCH_SUBPROC_WORK   *procWork = p_wk_adrs;
  WIFIBATTLEMATCH_SUBPROC_PARAM  *p_param  = p_param_adrs;

  u8 i;
  //パーティーのセット
  for( i=0;i<TEMOTI_POKEMAX;i++ )
  {
    const u8 num = procWork->plData.in_num[i];
    if( num > 0 )
    {
      POKEMON_PARAM *pp = PP_CreateByPPP( procWork->param.ppp[num-1] , procWork->heapId );
      PokeParty_Add( p_param->p_party, pp );
      GFL_HEAP_FreeMemory( pp );
    }
  }
  


  GFL_HEAP_FreeMemory( procWork->pokeParty );

  GFL_PROC_LOCAL_Exit( procWork->procSys );
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SUB );

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_SUB_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WIFIBATTLEMATCH_SUBPROC_WORK   *procWork = p_wk_adrs;
  WIFIBATTLEMATCH_SUBPROC_PARAM  *p_param  = p_param_adrs;

  //制限時間処理
  if( procWork->plData.use_tile_limit == TRUE &&
      procWork->plData.time_limit > 0 )
  {
    const u32 vCount = OS_GetVBlankCount();
    procWork->timeCnt += vCount - procWork->befVCount;
    procWork->befVCount = vCount;
    if( procWork->timeCnt > 60 )
    {
      procWork->plData.time_limit--;
      procWork->timeCnt -= 60;
    }
  }

  procWork->plData.comm_selected_num  = p_param->comm_selected_num;
  
  
  switch( procWork->state )
  {
  case IBSS_INIT_POKELIST:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(pokelist) );
    GFL_PROC_LOCAL_CallProc( procWork->procSys , NO_OVERLAY_ID , &PokeList_ProcData , &procWork->plData );
    
    procWork->state = IBSS_MAIN_POKELIST;
    break;

  case IBSS_MAIN_POKELIST:
    if( GFL_PROC_LOCAL_Main( procWork->procSys ) == GFL_PROC_MAIN_NULL )
    {
      procWork->state = IBSS_EXIT_POKELIST;
    }
    break;

  case IBSS_EXIT_POKELIST:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(pokelist) );
    if( procWork->plData.ret_mode == PL_RET_STATUS )
    {
      procWork->psData.pos = procWork->plData.ret_sel;
      procWork->state = IBSS_INIT_POKESTATUS;
    }
    else
    {
      p_param->result = WIFIBATTLEMATCH_SUBPROC_RESULT_SUCCESS;
      return GFL_PROC_RES_FINISH;
    }
    break;

  case IBSS_INIT_POKESTATUS:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(poke_status) );
    GFL_PROC_LOCAL_CallProc( procWork->procSys , NO_OVERLAY_ID , &PokeStatus_ProcData , &procWork->psData );
    
    procWork->state = IBSS_MAIN_POKESTATUS;
    break;

  case IBSS_MAIN_POKESTATUS:
    if( procWork->plData.use_tile_limit == TRUE &&
        procWork->plData.time_limit == 0 )
    {
      procWork->psData.isExitRequest = TRUE;
    }

    if( GFL_PROC_LOCAL_Main( procWork->procSys ) == GFL_PROC_MAIN_NULL )
    {
      procWork->state = IBSS_EXIT_POKESTATUS;
    }
    break;

  case IBSS_EXIT_POKESTATUS:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(poke_status) );
    procWork->plData.ret_sel = procWork->psData.pos;
    procWork->state = IBSS_INIT_POKELIST;
    break;
  }

  //エラー処理
  if( NetErr_App_CheckError() )
  { 
    const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();

    switch( cp_error->errorType )
    { 
    case DWC_ETYPE_LIGHT:
      /* fallthru */
    case DWC_ETYPE_SHOW_ERROR:
      //エラーコードorメッセージを表示するだけ
      NetErr_DispCallPushPop();
      GFL_NET_StateClearWifiError();
      break;

    case DWC_ETYPE_SHUTDOWN_FM:
    case DWC_ETYPE_SHUTDOWN_GHTTP:
    case DWC_ETYPE_SHUTDOWN_ND:
      //シャットダウン
      NetErr_App_ReqErrorDisp();
      p_param->result = WIFIBATTLEMATCH_SUBPROC_RESULT_ERROR_NEXT_LOGIN;
      return GFL_PROC_RES_FINISH;

    case DWC_ETYPE_DISCONNECT:
      //切断
      NetErr_App_ReqErrorDisp();
      p_param->result = WIFIBATTLEMATCH_SUBPROC_RESULT_ERROR_NEXT_LOGIN;
      return GFL_PROC_RES_FINISH;

    case DWC_ETYPE_FATAL:
      //Fatal
      NetErr_DispCallFatal();
      break;
    }
  } 


  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
//	リスト初期化データ作成
//--------------------------------------------------------------
static void WIFIBATTLEMATCH_SUBPROC_InitListData( WIFIBATTLEMATCH_SUBPROC_PARAM *p_param, WIFIBATTLEMATCH_SUBPROC_WORK *procWork , PLIST_DATA *plData )
{
  u8 i;
  REGULATION *reg = p_param->regulation;
  SAVE_CONTROL_WORK *svWork = GAMEDATA_GetSaveControlWork(p_param->gameData);
  
  plData->pp = procWork->pokeParty;
  plData->myitem = NULL;
  plData->mailblock = NULL;
  plData->cfg = SaveData_GetConfig(svWork);
  plData->tvwk = NULL;
  plData->reg = reg;
  
  plData->place = 0;
  plData->mode = PL_MODE_BATTLE;
  switch( reg->BATTLE_TYPE )
  {
  case REGULATION_BATTLE_SINGLE:   ///<シングル
    plData->type = PL_TYPE_SINGLE;
    plData->comm_type = PL_COMM_SINGLE;
    break;
  case REGULATION_BATTLE_DOUBLE:   ///<ダブル
    plData->type = PL_TYPE_DOUBLE;
    plData->comm_type = PL_COMM_SINGLE;
    break;
  case REGULATION_BATTLE_TRIPLE:   ///<トリプル
    plData->type = PL_TYPE_DOUBLE;
    plData->comm_type = PL_COMM_SINGLE;
    break;
  case REGULATION_BATTLE_ROTATION:   ///<ローテーション
    plData->type = PL_TYPE_DOUBLE;
    plData->comm_type = PL_COMM_SINGLE;
    break;
  case REGULATION_BATTLE_MULTI:   ///<マルチ
    plData->type = PL_TYPE_MULTI;
    plData->comm_type = PL_COMM_MULTI;
    break;
  }
  plData->ret_sel = PL_SEL_POS_POKE1;
  plData->ret_mode = PL_RET_NORMAL;

  plData->item = 0;
  plData->waza = 0;
  plData->waza_pos = 0;
  
  for( i=0;i<6;i++ )
  {
    plData->in_num[i] = 0;
  }
  plData->lv_cnt = 0;
  plData->after_mons = 0;
  plData->shinka_cond = 0;
  
  plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].pp = p_param->enemyPokeParty;
  plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].name = p_param->enemyName;
  plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].sex = p_param->enemySex;
  plData->is_disp_party = reg->SHOW_POKE;
  if( reg->SHOW_POKE_TIME != 0 )
  {
    plData->use_tile_limit = TRUE;
    plData->time_limit = reg->SHOW_POKE_TIME;
  }
  else
  {
    plData->use_tile_limit = FALSE;
    plData->time_limit = 0;
  }
  plData->comm_selected_num = 0;
}

//--------------------------------------------------------------
//	ステータス初期化データ作成
//--------------------------------------------------------------
static void WIFIBATTLEMATCH_SUBPROC_InitStatusData( WIFIBATTLEMATCH_SUBPROC_PARAM *p_param, WIFIBATTLEMATCH_SUBPROC_WORK *procWork , PSTATUS_DATA *psData )
{
  SAVE_CONTROL_WORK *svWork = GAMEDATA_GetSaveControlWork(p_param->gameData);
  MYSTATUS *myStatus = GAMEDATA_GetMyStatus( p_param->gameData );
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( p_param->gameData );

  psData->ppd = procWork->pokeParty;
  psData->game_data = p_param->gameData;
  psData->cfg = SaveData_GetConfig(svWork);

  psData->player_name = MyStatus_GetMyName( myStatus );
  psData->player_id = MyStatus_GetID( myStatus );
  psData->player_sex = MyStatus_GetMySex( myStatus );
  
  psData->ppt = PST_PP_TYPE_POKEPARTY;
  psData->mode = PST_MODE_NO_WAZACHG;
  psData->max = PokeParty_GetPokeCount(procWork->pokeParty);
  psData->pos = 0;
  psData->page = PPT_INFO;
  
  psData->ret_sel = 0;
  psData->ret_mode = 0;
  psData->waza = 0;

  psData->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
  
  psData->canExitButton = FALSE;
  psData->isExitRequest = FALSE;
}
