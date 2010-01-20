//======================================================================
/**
 * @file	irc_battle_subproc.c
 * @brief	赤外線バトル リスト・ステータス呼び出し
 * @author	ariizumi
 * @data	09/01/19
 *
 * モジュール名：IRC_BATTLE_SUBPROC
 */
//======================================================================
#include <gflib.h>

#include "app/pokelist.h"
#include "app/p_status.h"
#include "poke_tool/pokeparty.h"
#include "savedata/mystatus.h"
#include "system/main.h"
#include "system/gfl_use.h"

#include "irc_battle_subproc.h"
#include "irc_battle_local_def.h"

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
}IRC_BATTLE_SUBPROC_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _IRC_BATTLE_SUBPROC_WORK
{
  IRC_BATTLE_SUBPROC_STATE state;
  
  POKEPARTY *pokeParty;
  
  PLIST_DATA plData;
  PSTATUS_DATA psData;
  
  u8 timeCnt;
  u32 befVCount;
  
  GFL_PROCSYS *procSys;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void IRC_BATTLE_SUBPROC_InitListData( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork , PLIST_DATA *plData );
static void IRC_BATTLE_SUBPROC_InitStatusData( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork , PSTATUS_DATA *psData );

FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(poke_status);

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
IRC_BATTLE_SUBPROC_WORK* IRC_BATTLE_SUBPROC_InitSubProc( IRC_BATTLE_WORK *work )
{
  u8 i;
  IRC_BATTLE_SUBPROC_WORK* procWork = GFL_HEAP_AllocClearMemory( work->heapId , sizeof(IRC_BATTLE_SUBPROC_WORK));
  
  procWork->procSys = GFL_PROC_LOCAL_boot( work->heapId );

  procWork->pokeParty = PokeParty_AllocPartyWork( work->heapId );
  for( i=0;i<CHAMPIONSHIP_POKE_NUM;i++ )
  {
    if( PPP_Get( work->initWork->csData->ppp[i] , ID_PARA_poke_exist , NULL ) )
    {
      POKEMON_PARAM *pp = PP_CreateByPPP( work->initWork->csData->ppp[i] , work->heapId );
      PokeParty_Add( procWork->pokeParty , pp );
      GFL_HEAP_FreeMemory( pp );
    }
  }
  
  IRC_BATTLE_SUBPROC_InitListData( work , procWork , &procWork->plData );
  IRC_BATTLE_SUBPROC_InitStatusData( work , procWork , &procWork->psData );
  
  procWork->timeCnt = 0;
  procWork->befVCount = OS_GetVBlankCount();
  procWork->state = IBSS_INIT_POKELIST;
  return procWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void IRC_BATTLE_SUBPROC_TermSubProc( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork )
{
  u8 i;
  //パーティーのセット
  work->battleParty = PokeParty_AllocPartyWork( work->heapId );
  for( i=0;i<CHAMPIONSHIP_POKE_NUM;i++ )
  {
    const u8 num = procWork->plData.in_num[i];
    if( num > 0 )
    {
      POKEMON_PARAM *pp = PP_CreateByPPP( work->initWork->csData->ppp[num-1] , work->heapId );
      PokeParty_Add( work->battleParty , pp );
      GFL_HEAP_FreeMemory( pp );
    }
  }
  


  GFL_HEAP_FreeMemory( procWork->pokeParty );

  GFL_PROC_LOCAL_Exit( procWork->procSys );
  GFL_HEAP_FreeMemory( procWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const BOOL IRC_BATTLE_SUBPROC_UpdateSubProc( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork )
{
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
      return TRUE;
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
  
  return FALSE;
}

//--------------------------------------------------------------
//	リスト初期化データ作成
//--------------------------------------------------------------
static void IRC_BATTLE_SUBPROC_InitListData( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork , PLIST_DATA *plData )
{
  u8 i;
  const u8 netId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 enemyId = (netId == 0 ? 1 : 0 );
  REGULATION *reg = work->initWork->csData->regulation;
  SAVE_CONTROL_WORK *svWork = GAMEDATA_GetSaveControlWork(work->initWork->gameData);
  IRC_BATTLE_BATTLE_DATA *enemyBtlData = work->postBattleData[enemyId];
  
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
  
  plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].pp = IRC_BATTLE_BattleData_GetPokeParty(enemyBtlData);
  plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].name = enemyBtlData->name;
  plData->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].sex = enemyBtlData->sex;
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
static void IRC_BATTLE_SUBPROC_InitStatusData( IRC_BATTLE_WORK *work , IRC_BATTLE_SUBPROC_WORK *procWork , PSTATUS_DATA *psData )
{
  SAVE_CONTROL_WORK *svWork = GAMEDATA_GetSaveControlWork(work->initWork->gameData);
  MYSTATUS *myStatus = GAMEDATA_GetMyStatus( work->initWork->gameData );

  psData->ppd = procWork->pokeParty;
  psData->game_data = work->initWork->gameData;
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
  
  psData->canExitButton = FALSE;
  psData->isExitRequest = FALSE;
}

//--------------------------------------------------------------
//	相手が終了したフラグ受信
//--------------------------------------------------------------
void IRC_BATTLE_SUBPROC_PostFinishPokelist( IRC_BATTLE_SUBPROC_WORK *procWork )
{
  procWork->plData.comm_selected_num++;
}
