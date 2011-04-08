//============================================================================================
/**
 * @file  wbt_tool.c
 * @brief   WBT用ツール関数群
 * @date  2011.04.08
 * @author  tamada@gamefreak.co.jp
 *
 * WBTSYSの状態に依存しない関数をここに集めている
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

//#include "msg/msg_wbt_tournament.h"

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "wbt_system_local.h"

//#include "system/gfl_use.h" //GFUser_GetPublicRand0

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h"
#include "sound/pm_sndsys.h"

#include "../../../resource/wbt/wbt_trainer/wbt_trainer.cdat"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief ROMトレーナーデータの取得
 * @param heapID
 * @param tr_id
 * @param tr_data
 *
 * @todo  メモリ圧迫に備えて、バイナリから読み込むように修正
 */
//------------------------------------------------------------------
void WBTTOOL_GetTrRomData( HEAPID heapID, u8 tr_id, WBT_TRDATA * tr_data )
{
  *tr_data = WBTTrainerData[ tr_id ];
}

//------------------------------------------------------------------
/**
 * @brief トーナメント名文字列をセット
 * @todo  ダウンロードデータへの対応（引数がWBTSYSになる）
 */
//------------------------------------------------------------------
void WBTTOOL_SetTournamentName( WBTTYPE type, STRBUF * strbuf )
{
  HEAPID heapID = HEAPID_FIELDMAP;
  GFL_MSGDATA     *man;
  u16 msg_id;

  switch ( type )
  {
  default:
  case WBTTYPE_DOWNLOAD:
    //仮：ダウンロードタイトル名には未対応
  case WBTTYPE_NULL:
    GF_ASSERT(0);
    /* FALL THROUGH */
  case WBTTYPE_ALL:
    msg_id = msg_wbtsys_title_01;
    break;
  case WBTTYPE_CHAMPION:
    msg_id = msg_wbtsys_title_03;
    break;
  case WBTTYPE_POKETYPE:
    msg_id = msg_wbtsys_title_02;
    break;
  }
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}

//------------------------------------------------------------------
/**
 * @brief ルール名文字列をセット
 */
//------------------------------------------------------------------
void WBTTOOL_SetRuleName( WBTRULE rule, STRBUF * strbuf )
{
  HEAPID heapID = HEAPID_FIELDMAP;
  GFL_MSGDATA     *man;
  u16 msg_id;
  switch ( rule )
  {
  case WBTRULE_NULL:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case WBTRULE_SINGLE:
    msg_id = msg_wbtsys_rule_single;
    break;
  case WBTRULE_DOUBLE:
    msg_id = msg_wbtsys_rule_double;
    break;
  case WBTRULE_TRIPLE:
    msg_id = msg_wbtsys_rule_triple;
    break;
  case WBTRULE_ROTATION:
    msg_id = msg_wbtsys_rule_rotation;
    break;
  }
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}

//------------------------------------------------------------------
/**
 * @brief WBTロムトレーナー名を取得
 */
//------------------------------------------------------------------
void WBTTOOL_SetRomTrainerName( HEAPID heapID, u8 tr_id, STRBUF * strbuf )
{
  GFL_MSGDATA     *man;
  u16 msg_id = tr_id;
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_trname_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 * @brief WBT:パーティー全体のポケモンタイプを判定する
 * @param party ポケモンパーティへのポインタ
 * @param seed  乱数生成のタネ
 * @return  POKETYPE  タイプ（include/poke_tool/poketype_def.h参照）
 */
//-----------------------------------------------------------------------------
u8 WBTTOOL_GetPartyPokeType( POKEPARTY * party, u64 seed )
{
  u32 count, i;
  u8 type_count[ POKETYPE_NUMS ];
  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, seed );

  GFL_STD_MemClear( type_count, POKETYPE_NUMS );
  count = PokeParty_GetPokeCount( party );

  for ( i = 0; i < count; i ++ )
  {
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
    int type1, type2;
    type1 = PP_Get( pp, ID_PARA_type1, NULL );
    type2 = PP_Get( pp, ID_PARA_type2, NULL );
    if ( type1 != POKETYPE_NULL )
    {
      type_count[ type1 ] ++;
    }
    if ( type1 != type2 && type2 != POKETYPE_NULL )
    {
      type_count[ type2 ] ++;
    }
  }

  {
    u8 max_value = 0, max_count = 0;
    u8 max_pos_table[ POKETYPE_NUMS ];
    GFL_STD_MemClear( max_pos_table, POKETYPE_NUMS );
    for ( i = 0; i < POKETYPE_NUMS; i ++ )
    {
      if ( type_count[ i ] > max_value )
      {
        max_value = type_count[ i ];
      }
    }
    for ( i = 0; i < POKETYPE_NUMS; i ++ )
    {
      if ( type_count[ i ] == max_value )
      {
        max_pos_table[ max_count ] = i;
        max_count ++;
      }
    }
    GF_ASSERT( max_count > 0 );
    return GFL_STD_Rand( &rand_context, max_count );
  }
}


