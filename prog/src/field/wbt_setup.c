//============================================================================================
/**
 * @file  wbt_setup.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.03.30
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"
#include "app/wbt_list.h"

#include "poke_tool/regulation_def.h"
#include "wbt_trlist_struct.h"  //WBT_TRDATA

#include "arc_def.h"
#include "arc/wbt/wbt_trlist.naix"
#include "arc/fieldmap/fldmmdl_mdlres.naix"

#include "msg/msg_wbt_tournament.h"
#include "arc/message.naix"
#include "system/main.h"

#include "wbt_system_local.h"

#include "fldmmdl.h"
#include "fieldmap/fldmmdl_mdlparam.naix"

#include "tr_tool/trtype_def.h"
#include "sound/pm_sndsys.h"
#include "../../../resource/wbt/wbt_trainer/wbt_trainer.cdat"

//============================================================================================
//============================================================================================

static void setupWBTLISTtrainer( HEAPID heapID, GAMEDATA * gamedata,
    WBTLIST_TRAINER * wbtlist_tr, const WBTTRAINER * wbt_tr );

static void setupWBTLISTResultAll( WBTLIST_PARAM * wbtlist_param, GFL_STD_RandContext * context );
static void getMMdlObjCodeParam( ARCHANDLE * handle, u16 mmdl_id, u8 * anm_id, u16 * rsc_idx );
static u8 BTL_CALC_TypeAff( PokeType wazaType, PokeType pokeType );

//============================================================================================
//
//  WBTトーナメントリスト用パラメータ生成
//
//============================================================================================
//------------------------------------------------------------------
///  WBTトーナメントリスト用パラメータOBJECT生成
//------------------------------------------------------------------
WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys )
{
  int i;
  WBTLIST_PARAM * wbtlist_param;

  wbtlist_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTLIST_PARAM) );
  wbtlist_param->gamesys = gsys;
  wbtlist_param->stage = WBTLIST_STAGE_ENTRY;

  wbtlist_param->tournament_name = GFL_STR_CreateBuffer( BUFLEN_TOURNAMENT_NAME, heapID );
  wbtlist_param->tournament_rule = GFL_STR_CreateBuffer( BUFLEN_TOURNAMENT_NAME, heapID );
  //トレーナー情報の初期化
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTLIST_TRAINER * wbtlist_tr = &wbtlist_param->trainer[i];
    wbtlist_tr->name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
    wbtlist_tr->mmdl_id = NARC_fldmmdl_mdlres_trainerm_nsbtx;
    wbtlist_tr->pattern_no = 2;
    wbtlist_tr->sex = PM_MALE;
    wbtlist_tr->priority = 1;
    wbtlist_tr->poketype = POKETYPE_NORMAL;
  }
  //勝敗結果情報の初期化

  TAMADA_Printf("WBTLIST_PARAM:Create:%08x\n", wbtlist_param );
  return wbtlist_param;
}

//------------------------------------------------------------------
///  WBTトーナメントリスト用パラメータOBJECT削除
//------------------------------------------------------------------
void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTLIST_TRAINER * wbtlist_tr = &wbtlist_param->trainer[i];
    GFL_STR_DeleteBuffer( wbtlist_tr->name );
  }
  GFL_STR_DeleteBuffer( wbtlist_param->tournament_name );
  GFL_STR_DeleteBuffer( wbtlist_param->tournament_rule );
  GFL_HEAP_FreeMemory( wbtlist_param );
  TAMADA_Printf("WBTLIST_PARAM:Delete:%08x\n", wbtlist_param );
}

//------------------------------------------------------------------
/**
 * @brief トーナメントリスト用パラメータのセットアップ
 */
//------------------------------------------------------------------
void WBTLIST_PARAM_Setup( GAMESYS_WORK * gsys, WBTSYS * wbtsys, WBTLIST_PARAM * wbtlist_param )
{
  int i;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );

  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, i );
    setupWBTLISTtrainer( HEAPID_FIELDMAP, gamedata, &wbtlist_param->trainer[i], wbt_tr );
  }
  {
    GFL_STD_RandContext rand_context;
    GFL_STD_RandInit( &rand_context, WBTSYS_GetRandomSeed( wbtsys ) );
    setupWBTLISTResultAll( wbtlist_param, &rand_context );
  }

  wbtlist_param->stage = WBTSYS_GetStage( wbtsys );
  WBTTOOL_SetTournamentName( WBTSYS_GetType(wbtsys), wbtlist_param->tournament_name );
  WBTTOOL_SetRuleName( WBTSYS_GetRule(wbtsys), wbtlist_param->tournament_rule );
}


//------------------------------------------------------------------
/**
 * @brief トーナメントリスト用パラメータのセットアップ（デバッグ用）
 *
 * デバッグ用にWBTSYSを生成し、それを元にセットアップを行う
 */
//------------------------------------------------------------------
void WBTLIST_PARAM_Setup_Debug( WBTLIST_PARAM * wbtlist_param, WBTLIST_STAGE stage, GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  WBTSYS * wbtsys;

  wbtsys = WBTSYS_Create( HEAPID_FIELDMAP );
  WBTSYS_Setup_Debug( wbtsys, gamedata, stage );
  WBTLIST_PARAM_Setup( gsys, wbtsys, wbtlist_param );
  WBTSYS_Delete( wbtsys );
}

//============================================================================================
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void WBT_TRDATA_Copy( HEAPID heapID, u8 tr_id, WBT_TRDATA * tr_data )
{
  *tr_data = WBTTrainerData[ tr_id ];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void setupWBTLISTtrainer( HEAPID heapID, GAMEDATA * gamedata,
    WBTLIST_TRAINER * wbtlist_tr, const WBTTRAINER * wbt_tr )
{
  WBT_TRDATA tr_rom_data;
  ARCHANDLE * handle;
  u16 mmdl_id;

  GF_ASSERT( wbt_tr );
  GF_ASSERT( wbtlist_tr );

  WBT_TRDATA_Copy( heapID, wbt_tr->tr_id, &tr_rom_data );

  switch ( wbt_tr->wbt_type )
  {
  case WBT_TRTYPE_DL:
  case WBT_TRTYPE_SAVE:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case WBT_TRTYPE_ROM:
    wbtlist_tr->is_player = FALSE;
    wbtlist_tr->priority = tr_rom_data.priority;
    wbtlist_tr->poketype = tr_rom_data.type;
    wbtlist_tr->sex      = tr_rom_data.sex;
    {
      GFL_MSGDATA * man;
      man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
          NARC_message_wbt_trname_dat, heapID );
      GFL_MSG_GetString( man, wbt_tr->tr_id, wbtlist_tr->name );
      GFL_MSG_Delete( man );
    }
    mmdl_id = tr_rom_data.mmdl_id;
    break;
  case WBT_TRTYPE_PLAYER:
    wbtlist_tr->is_player = TRUE;
    wbtlist_tr->priority = wbt_tr->pri;
    wbtlist_tr->poketype = POKETYPE_NORMAL; //仮
    wbtlist_tr->sex = wbt_tr->sex;
    if ( wbt_tr->sex == PM_MALE ) {
      mmdl_id = HERO;
    } else {
      mmdl_id = HEROINE;
    }
    {
      const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
      MyStatus_CopyNameString( mystat, wbtlist_tr->name );
    }
    break;
  }

  handle = GFL_ARC_OpenDataHandle( ARCID_MMDL_PARAM, heapID );
  getMMdlObjCodeParam( handle, mmdl_id, &wbtlist_tr->pattern_no, &wbtlist_tr->mmdl_id );
  GFL_ARC_CloseDataHandle( handle );
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static void DEBUG_DumpResult( const WBTLIST_RESULT * result,
    const WBTLIST_TRAINER * tr_a, const WBTLIST_TRAINER * tr_b )
{
  TAMADA_Printf("upside_win %d \n", result->upside_win );
  TAMADA_Printf("msg_id = %d\n", result->msg_type );
  TAMADA_Printf("tr_a:pri(%d) type(%d) sex(%d) id(%d)\n",
      tr_a->priority, tr_a->poketype, tr_a->sex, tr_a->mmdl_id );
  TAMADA_Printf("tr_b:pri(%d) type(%d) sex(%d) id(%d)\n",
      tr_b->priority, tr_b->poketype, tr_b->sex, tr_b->mmdl_id );
}


//============================================================================================
//
//  対戦結果生成処理
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  MSG_GROUP_A = msg_wbt_comment_01,
  MSG_GROUP_B = msg_wbt_comment_06,
  MSG_GROUP_C = msg_wbt_comment_11,
  MSG_GROUP_D = msg_wbt_comment_16,
  MSG_GROUP_E = msg_wbt_comment_21,

  MSG_LAYER_NICEGAME = 0,
  MSG_LAYER_ONESIDE,
  MSG_LAYER_ADVANTAGE,
  MSG_LAYER_EVEN,
  MSG_LAYER_DISADVANTAGE,
};
//------------------------------------------------------------------
/**
 * @brief 対戦結果の生成処理
 */
//------------------------------------------------------------------
static void setupWBTLISTResult( WBTLIST_RESULT * result,
    GFL_STD_RandContext * context,
    const WBTLIST_TRAINER * tr_a, const WBTLIST_TRAINER * tr_b )
{
  u32 pri_a = tr_a->priority;
  u32 pri_b = tr_b->priority;

  u8 aff_a = BTL_CALC_TypeAff( tr_a->poketype, tr_b->poketype );
  u8 aff_b = BTL_CALC_TypeAff( tr_b->poketype, tr_a->poketype );

  // -----  勝敗判定 --------------------
  if ( pri_a == pri_b )
  { //優先度が同じ場合
    if ( aff_a == aff_b )
    { //タイプ相性も同じ場合はランダム
      result->upside_win = ( GFL_STD_Rand( context, 2 ) == 0 );
    }
    else
    { //タイプ相性が高いほうが勝利
      result->upside_win = ( aff_a > aff_b );
    }
    //result->msg_type = msg_wbt_comment_11;
  }
  else
  { //優先度が違う場合、高いほうが勝利
    result->upside_win = ( pri_a > pri_b );

  }

  //とりあえずプレイヤーは勝つ。MSGの整合性は後ほど
  if ( tr_a->is_player )
  {
    result->upside_win = TRUE;
  }
  else if ( tr_b->is_player )
  {
    result->upside_win = FALSE;
  }

  // ----- メッセージの割り当て --------------------
  {
    int msg_layer;
    int msg_side;
    static const u8 msg_table[][3] = {
      { MSG_GROUP_D, MSG_GROUP_D, MSG_GROUP_E },  //MSG_LAYER_NICEGAME
      { MSG_GROUP_A, MSG_GROUP_A, MSG_GROUP_A },  //MSG_LAYER_ONESIDE
      { MSG_GROUP_B, MSG_GROUP_B, MSG_GROUP_B },  //MSG_LAYER_ADVANTAGE
      { MSG_GROUP_C, MSG_GROUP_C, MSG_GROUP_E },  //MSG_LAYER_EVEN
      { MSG_GROUP_D, MSG_GROUP_D, MSG_GROUP_E },  //MSG_LAYER_DISADVANTAGE
    };

  //MSGの振り分け
    if ( pri_a >= 4 && pri_b >= 4 )
    {
      msg_layer = MSG_LAYER_NICEGAME;
    }
    else if (
        ( pri_a - pri_b >= 2 && result->upside_win == TRUE ) ||
        ( pri_b - pri_a >= 2 && result->upside_win == FALSE ) )
    {
      msg_layer = MSG_LAYER_ONESIDE;
    }
    else if (
      ( pri_a - pri_b == 1 && result->upside_win == TRUE ) ||
      ( pri_b - pri_a == 1 && result->upside_win == FALSE ) )
    {
      msg_layer = MSG_LAYER_ADVANTAGE;
    }
    else if ( pri_a == pri_b )
    {
      msg_layer = MSG_LAYER_EVEN;
    }
    else
    {
      msg_layer = MSG_LAYER_DISADVANTAGE;
    }
    //タイプ相性の有利・互角・不利
    if ( aff_a > aff_b )
    {
      msg_side = -1;
    }
    else if ( aff_a < aff_b )
    {
      msg_side = +1;
    }
    else
    {
      msg_side = 0;
    }
    if ( result->upside_win == TRUE)
    {
      msg_side *= -1;
    }
    msg_side += 1;

    result->msg_type = msg_table[msg_layer][msg_side];
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupWBTLISTResultAll( WBTLIST_PARAM * wbtlist_param, GFL_STD_RandContext * context )
{
  int i, j;

  //1回戦結果生成
  for ( i = 0; i < 4 ; i ++ )
  {
    WBTLIST_RESULT * result = &wbtlist_param->first_stage[ i ];
    WBTLIST_TRAINER * tr_a = &wbtlist_param->trainer[ i * 2 + 0 ];
    WBTLIST_TRAINER * tr_b = &wbtlist_param->trainer[ i * 2 + 1 ];
    setupWBTLISTResult( result, context, tr_a, tr_b );
    DEBUG_DumpResult( result, tr_a, tr_b );
  }

  //2回戦結果生成
  for ( i = 0; i < 2 ; i ++ )
  {
    WBTLIST_RESULT * result = &wbtlist_param->second_stage[i];
    WBTLIST_TRAINER * tr[2];
    for ( j = 0; j < 2; j ++ )
    {
      if ( wbtlist_param->first_stage[ i * 2 + j ].upside_win )
      {
        tr[j] = &wbtlist_param->trainer[ i * 4 + j * 2 + 0 ];
      }
      else
      {
        tr[j] = &wbtlist_param->trainer[ i * 4 + j * 2 + 1 ];
      }
    }
    setupWBTLISTResult( result, context, tr[0], tr[1] );
    DEBUG_DumpResult( result, tr[0], tr[1] );
  }
}

//============================================================================================
//
//  動作モデルデータの参照
//
//============================================================================================
extern const MMDL_BBDACT_ANMTBL DATA_MMDL_BBDACT_ANM_ListTable[MMDL_BLACT_ANMTBLNO_MAX];
//-----------------------------------------------------------------------------
/**
 * @brief 動作モデル正面絵パターンナンバーの取得
 *
 * fldmmdlのOBJCODE_PARAMから正面絵の位置を引っ張る
 * OBJCODE_PARAM::anm_idからアニメパターンの種類がわかるので、
 * それぞれのアニメパターンの種類から下向き静止アニメをとりだし、
 * その指定パターンを正面絵のパターンナンバーとして取り出している
 */
//-----------------------------------------------------------------------------
static u8 getFrontAnimePatternNo( anm_id )
{
  const MMDL_BBDACT_ANMTBL * mmdl_anm_tbl;
  GF_ASSERT( anm_id < MMDL_BLACT_ANMTBLNO_MAX );
  mmdl_anm_tbl = &DATA_MMDL_BBDACT_ANM_ListTable[ anm_id ];
  GF_ASSERT( AC_DIR_D < mmdl_anm_tbl->anm_max );

  return mmdl_anm_tbl->pAnmTbl[AC_DIR_D]->anmData.celIdx;
}

//-----------------------------------------------------------------------------
/**
 * @brief 動作モデルIDから動作モデルパラメータを取得する
 * @param handle
 * @param mmdl_id
 * @param anm_id
 * @param rsc_idx
 */
//-----------------------------------------------------------------------------
static void getMMdlObjCodeParam( ARCHANDLE * handle, u16 mmdl_id, u8 * anm_id, u16 * rsc_idx )
{
  OBJCODE_PARAM objcode_param;
  u32 size = sizeof( OBJCODE_PARAM );
  u32 offs = OBJCODE_PARAM_TOTAL_NUMBER_SIZE + ( size * MMDL_TOOL_OBJCodeToDataNumber(mmdl_id) );
  GFL_ARC_LoadDataOfsByHandle( handle, NARC_fldmmdl_mdlparam_fldmmdl_mdlparam_bin,
      offs, size, &objcode_param );

  TAMADA_Printf("OBJCODE_PARAM:mmdl_id = %d\n",mmdl_id );
  TAMADA_Printf("OBJCODE_PARAM:offs = %d\n", (offs - OBJCODE_PARAM_TOTAL_NUMBER_SIZE) / size );
  TAMADA_Printf("OBJCODE_PARAM:anm_id = %d\n",objcode_param.anm_id );
  TAMADA_Printf("OBJCODE_PARAM:res_idx = %d\n",
      ((OBJCODE_PARAM_BUF_BBD *)objcode_param.buf)->res_idx );

  *anm_id = getFrontAnimePatternNo( objcode_param.anm_id );
  *rsc_idx = ((OBJCODE_PARAM_BUF_BBD *)objcode_param.buf)->res_idx;
}


//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//============================================================================================
//
//    以下はsrc/battle/btl_calc.cからコピペ
//
//============================================================================================
//--------------------------------------------------------------
/**
 *  タイプ相性値
 */
//--------------------------------------------------------------
typedef enum {
    x0 = 0,
    xH = 2,
    x1 = 4,
    x2 = 8,
}TypeAffValue;


//--------------------------------------------------------------
/**
 *  タイプ相性テーブル
 */
//--------------------------------------------------------------
static const u8 TypeAffTbl[ POKETYPE_NUMS ][ POKETYPE_NUMS ] = {
//          ノ,  格,  飛,  毒,  地,  岩,  虫,  幽,  鋼,  炎,  水,  草,  電,  超,  氷,  竜,  悪,
/* ノ */ {  x1,  x1,  x1,  x1,  x1,  xH,  x1,  x0,  xH,  x1,  x1,  x1,  x1,  x1,  x1,  x1,  x1, },
/* 格 */ {  x2,  x1,  xH,  xH,  x1,  x2,  xH,  x0,  x2,  x1,  x1,  x1,  x1,  xH,  x2,  x1,  x2, },
/* 飛 */ {  x1,  x2,  x1,  x1,  x1,  xH,  x2,  x1,  xH,  x1,  x1,  x2,  xH,  x1,  x1,  x1,  x1, },
/* 毒 */ {  x1,  x1,  x1,  xH,  xH,  xH,  x1,  xH,  x0,  x1,  x1,  x2,  x1,  x1,  x1,  x1,  x1, },
/* 地 */ {  x1,  x1,  x0,  x2,  x1,  x2,  xH,  x1,  x2,  x2,  x1,  xH,  x2,  x1,  x1,  x1,  x1, },
/* 岩 */ {  x1,  xH,  x2,  x1,  xH,  x1,  x2,  x1,  xH,  x2,  x1,  x1,  x1,  x1,  x2,  x1,  x1, },
/* 虫 */ {  x1,  xH,  xH,  xH,  x1,  x1,  x1,  xH,  xH,  xH,  x1,  x2,  x1,  x2,  x1,  x1,  x2, },
/* 幽 */ {  x0,  x1,  x1,  x1,  x1,  x1,  x1,  x2,  xH,  x1,  x1,  x1,  x1,  x2,  x1,  x1,  xH, },
/* 鋼 */ {  x1,  x1,  x1,  x1,  x1,  x2,  x1,  x1,  xH,  xH,  xH,  x1,  xH,  x1,  x2,  x1,  x1, },
/* 炎 */ {  x1,  x1,  x1,  x1,  x1,  xH,  x2,  x1,  x2,  xH,  xH,  x2,  x1,  x1,  x2,  xH,  x1, },
/* 水 */ {  x1,  x1,  x1,  x1,  x2,  x2,  x1,  x1,  x1,  x2,  xH,  xH,  x1,  x1,  x1,  xH,  x1, },
/* 草 */ {  x1,  x1,  xH,  xH,  x2,  x2,  xH,  x1,  xH,  xH,  x2,  xH,  x1,  x1,  x1,  xH,  x1, },
/* 電 */ {  x1,  x1,  x2,  x1,  x0,  x1,  x1,  x1,  x1,  x1,  x2,  xH,  xH,  x1,  x1,  xH,  x1, },
/* 超 */ {  x1,  x2,  x1,  x2,  x1,  x1,  x1,  x1,  xH,  x1,  x1,  x1,  x1,  xH,  x1,  x1,  x0, },
/* 氷 */ {  x1,  x1,  x2,  x1,  x2,  x1,  x1,  x1,  xH,  xH,  xH,  x2,  x1,  x1,  xH,  x2,  x1, },
/* 竜 */ {  x1,  x1,  x1,  x1,  x1,  x1,  x1,  x1,  xH,  x1,  x1,  x1,  x1,  x1,  x1,  x2,  x1, },
/* 悪 */ {  x1,  xH,  x1,  x1,  x1,  x1,  x1,  x2,  xH,  x1,  x1,  x1,  x1,  x2,  x1,  x1,  xH, },
};

//--------------------------------------------------------------
/**
 *  タイプ相性計算
 */
//--------------------------------------------------------------
static u8 BTL_CALC_TypeAff( PokeType wazaType, PokeType pokeType )
{
  if( (wazaType == POKETYPE_NULL) || (pokeType == POKETYPE_NULL) ){
    return 2;//BTL_TYPEAFF_100;
  }
  else
  {
    u8 affinity = TypeAffTbl[ wazaType ][ pokeType ];

    switch( affinity ){
    case x0:    return 0; //BTL_TYPEAFF_0;
    case xH:    return 1; //BTL_TYPEAFF_50;
    case x1:    return 2; //BTL_TYPEAFF_100;
    case x2:    return 3; //BTL_TYPEAFF_200;
    }
  }

  GF_ASSERT(0);
  return 0; //BTL_TYPEAFF_0;
}









