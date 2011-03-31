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


#include "fldmmdl.h"
#include "fieldmap/fldmmdl_mdlparam.naix"

#include "tr_tool/trtype_def.h"
#include "sound/pm_sndsys.h"
#include "../../../resource/wbt/wbt_trainer/wbt_trainer.cdat"

//============================================================================================
//============================================================================================

static void setupWBTLISTTrainer( WBTLIST_TRAINER * wbtlist_tr, HEAPID heapID, u8 tr_id );
static void setupWBTLISTResultAll( WBTLIST_PARAM * wbtlist_param, GFL_STD_RandContext * context );
static void getMMdlObjCodeParam( ARCHANDLE * handle, u16 mmdl_id,
    u8 * anm_id, u16 * rsc_idx, u8 * sex );
static u8 BTL_CALC_TypeAff( PokeType wazaType, PokeType pokeType );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys )
{
  int i;
  WBTLIST_PARAM * wbtlist_param;

  wbtlist_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTLIST_PARAM) );
  wbtlist_param->gamesys = gsys;
  wbtlist_param->stage = WBTLIST_STAGE_ENTRY;

  //トレーナー情報の初期化
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTLIST_TRAINER * trdata = &wbtlist_param->trainer[i];
    trdata->name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
    trdata->mmdl_id = NARC_fldmmdl_mdlres_trainerm_nsbtx;
    trdata->pattern_no = 2;
    trdata->sex = PM_MALE;
    trdata->priority = 1;
    trdata->poketype = POKETYPE_NORMAL;
  }
  //勝敗結果情報の初期化

  TAMADA_Printf("WBTLIST_PARAM:Create:%08x\n", wbtlist_param );
  return wbtlist_param;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTLIST_TRAINER * trdata = &wbtlist_param->trainer[i];
    GFL_STR_DeleteBuffer( trdata->name );
  }
  GFL_HEAP_FreeMemory( wbtlist_param );
  TAMADA_Printf("WBTLIST_PARAM:Delete:%08x\n", wbtlist_param );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTLIST_PARAM_Setup_Debug( WBTLIST_PARAM * wbtlist_param, WBTLIST_STAGE stage, HEAPID heapID )
{
  GFL_STD_RandContext rand_context;
  int i;
  //仮
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    static const u16 sample_trid[WBT_TRAINER_NUM] = {
      0,//チェレン
      53,//モブ
      2,//アーティ
      1,//？？
      54,//モブ
      3,//カミツレ
      64,//モブ
      18,//シロナ
    };
    setupWBTLISTTrainer( &wbtlist_param->trainer[i], heapID, sample_trid[i] );
    wbtlist_param->stage = stage;
  }
  {
    u64 seed64;
    seed64 = GFL_STD_MtRand( 0 );
    seed64 = (seed64 << 32) + GFL_STD_MtRand( 0 );
    GFL_STD_RandInit( &rand_context, seed64 );
  }
  setupWBTLISTResultAll( wbtlist_param, &rand_context );
}


//============================================================================================
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void setupWBTLISTTrainer( WBTLIST_TRAINER * wbtlist_tr, HEAPID heapID, u8 tr_id )
{
  WBT_TRDATA tr_rom_data;
  ARCHANDLE * handle;

  handle = GFL_ARC_OpenDataHandle( ARCID_MMDL_PARAM, heapID );

#if 0
  GFL_ARC_LoadDataOfs( &tr_rom_data, ARCID_WBT_TRLIST, NARC_wbt_trlist_wbt_trainer_bin,
      sizeof(WBT_TRDATA) * tr_id, sizeof(WBT_TRDATA) );
#endif
  tr_rom_data = WBTTrainerData[ tr_id ];
  TAMADA_Printf("TR_ROM_DATA:tr_id = %d\n", tr_id );
  TAMADA_Printf("TR_ROM_DATA:priority = %d\n", tr_rom_data.priority );
  TAMADA_Printf("TR_ROM_DATA:type = %d\n", tr_rom_data.type );
  TAMADA_Printf("TR_ROM_DATA:mmdl_id = %d\n", tr_rom_data.mmdl_id );

  wbtlist_tr->priority = tr_rom_data.priority;
  wbtlist_tr->poketype = tr_rom_data.type;

  getMMdlObjCodeParam( handle, tr_rom_data.mmdl_id,
      &wbtlist_tr->pattern_no, &wbtlist_tr->mmdl_id, &wbtlist_tr->sex );

  GFL_ARC_CloseDataHandle( handle );

  {
    GFL_MSGDATA * man;
    man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
        NARC_message_wbt_trname_dat, heapID );
    GFL_MSG_GetString( man, tr_id, wbtlist_tr->name );
    GFL_MSG_Delete( man );
  }
}

//------------------------------------------------------------------
/**
 * @todo  プレイヤーについての判定が必要
 */
//------------------------------------------------------------------
static void setupWBTLISTResult( WBTLIST_RESULT * result,
    GFL_STD_RandContext * context,
    const WBTLIST_TRAINER * tr_a, const WBTLIST_TRAINER * tr_b )
{
  u32 pri_a = tr_a->priority;
  u32 pri_b = tr_b->priority;

  if ( pri_a == pri_b )
  { //優先度が同じ場合
    u8 aff_a = BTL_CALC_TypeAff( tr_a->poketype, tr_b->poketype );
    u8 aff_b = BTL_CALC_TypeAff( tr_b->poketype, tr_a->poketype );
    if ( aff_a == aff_b )
    { //タイプ相性も同じ場合はランダム
      result->upside_win = ( GFL_STD_Rand( context, 2 ) == 0 );
    }
    else
    { //タイプ相性が高いほうが勝利
      result->upside_win = ( aff_a > aff_b );
    }
    result->msg_type = msg_wbt_comment_11;
  }
  else
  { //優先度が違う場合、高いほうが勝利
    result->upside_win = ( pri_a > pri_b );

    if ( pri_a >= 4 && pri_b >= 4 )
    {
      result->msg_type = msg_wbt_comment_16;
    }
    else if ( MATH_IAbs( pri_a - pri_b ) >= 2 )
    {
      result->msg_type = msg_wbt_comment_01;
    }
    else if ( MATH_IAbs( pri_a - pri_b ) >= 1 )
    {
      result->msg_type = msg_wbt_comment_06;
    }
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
  }
}

//============================================================================================
//============================================================================================
extern const MMDL_BBDACT_ANMTBL DATA_MMDL_BBDACT_ANM_ListTable[MMDL_BLACT_ANMTBLNO_MAX];
//-----------------------------------------------------------------------------
//  fldmmdlのOBJCODE_PARAMから正面絵の位置を引っ張る
//  OBJCODE_PARAM::anm_idからアニメパターンの種類がわかるので、
//  それぞれのアニメパターンの何番目が正面絵かを判定する
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
//-----------------------------------------------------------------------------
static void getMMdlObjCodeParam( ARCHANDLE * handle, u16 mmdl_id,
    u8 * anm_id, u16 * rsc_idx, u8 * sex )
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
  TAMADA_Printf("OBJCODE_PARAM:sex = %d\n",objcode_param.sex );

  *anm_id = getFrontAnimePatternNo( objcode_param.anm_id );
  *rsc_idx = ((OBJCODE_PARAM_BUF_BBD *)objcode_param.buf)->res_idx;
  *sex = objcode_param.sex;
}



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









