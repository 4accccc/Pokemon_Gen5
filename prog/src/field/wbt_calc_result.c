//============================================================================================
/**
 * @file  wbt_calc_result.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.03.30
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"
//#include "app/wbt_list.h"

//#include "poke_tool/regulation_def.h"
//#include "wbt_trlist_struct.h"  //WBT_TRDATA

//#include "arc_def.h"
//#include "arc/wbt/wbt_trlist.naix"
//#include "arc/fieldmap/fldmmdl_mdlres.naix"

#include "msg/msg_wbt_tournament.h"
//#include "arc/message.naix"
//#include "system/main.h"

#include "wbt_system_local.h"

//#include "fldmmdl.h"
//#include "fieldmap/fldmmdl_mdlparam.naix"

//#include "tr_tool/trtype_def.h"
//#include "sound/pm_sndsys.h"
//#include "../../../resource/wbt/wbt_trainer/wbt_trainer.cdat"

//============================================================================================
//============================================================================================
static u8 BTL_CALC_TypeAff( PokeType wazaType, PokeType pokeType );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void DEBUG_DumpWBTResult( const WBTRESULT * result,
    const WBTTRAINER * tr_a, const WBTTRAINER * tr_b )
{
  TAMADA_Printf("upside_win %d \n", result->upside_win );
  TAMADA_Printf("msg_id = %d\n", result->msg_id );
  TAMADA_Printf("tr_a:pri(%d) type(%d) sex(%d) id(%d)\n",
      tr_a->pri, tr_a->poketype, tr_a->sex, tr_a->tr_id );
  TAMADA_Printf("tr_b:pri(%d) type(%d) sex(%d) id(%d)\n",
      tr_b->pri, tr_b->poketype, tr_b->sex, tr_b->tr_id );
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
static void calcWBTResult( WBTRESULT * result,
    GFL_STD_RandContext * context,
    const WBTTRAINER * tr_a, const WBTTRAINER * tr_b )
{
  u32 pri_a = tr_a->pri;
  u32 pri_b = tr_b->pri;

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
  if ( tr_a->wbt_type == WBT_TRTYPE_PLAYER )
  {
    result->upside_win = TRUE;
  }
  else if ( tr_b->wbt_type == WBT_TRTYPE_PLAYER )
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

    result->msg_id = msg_table[msg_layer][msg_side];
  }
}

//--------------------------------------------------------------
/**
 * @brief WBTトーナメント勝敗結果の生成
 * @param wbtsys    WBTシステムへのポインタ
 * @param context   乱数コンテキスト
 *
 * セット済みのトレーナーデータから結果生成を行う
 */
//--------------------------------------------------------------
void WBTSYS_CalcResult( WBTSYS * wbtsys, GFL_STD_RandContext * context )
{
  int i, j;

  //1回戦結果生成
  for ( i = 0; i < 4 ; i ++ )
  {
    WBTRESULT * result = &wbtsys->wbt_result[ i ];
    WBTTRAINER * tr_a = &wbtsys->wbt_trainer[ i * 2 + 0 ];
    WBTTRAINER * tr_b = &wbtsys->wbt_trainer[ i * 2 + 1 ];
    calcWBTResult( result, context, tr_a, tr_b );
    DEBUG_DumpWBTResult( result, tr_a, tr_b );
  }

  //2回戦結果生成
  for ( i = 0; i < 2 ; i ++ )
  {
    WBTRESULT * result = &wbtsys->wbt_result[ WBT_GAME_2ND_1 + i];
    WBTTRAINER * tr[2];
    for ( j = 0; j < 2; j ++ )
    {
      if ( wbtsys->wbt_result[ i * 2 + j ].upside_win )
      {
        tr[j] = &wbtsys->wbt_trainer[ i * 4 + j * 2 + 0 ];
      }
      else
      {
        tr[j] = &wbtsys->wbt_trainer[ i * 4 + j * 2 + 1 ];
      }
    }
    calcWBTResult( result, context, tr[0], tr[1] );
    DEBUG_DumpWBTResult( result, tr[0], tr[1] );
  }
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







