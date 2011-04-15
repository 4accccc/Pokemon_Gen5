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

#include "wbt_tool.h"
#include "wbt_trlist_struct.h"  //WBT_TRDATA

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "wbt_system_local.h"

//#include "system/gfl_use.h" //GFUser_GetPublicRand0

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h"
#include "sound/pm_sndsys.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
#include "../../../resource/wbt/wbt_trainer/wbt_trainer.cdat"


//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_ROM_TRAINER_NUM = NELEMS(WBTTrainerData),
};

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

//------------------------------------------------------------------
/**
 * @brief ルールごとの参加ポケモン数
 */
//------------------------------------------------------------------
u8 WBTTOOL_GetPartyMaxByRule( WBTRULE rule )
{
  switch ( rule )
  {
  default:
  case WBTRULE_NULL:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case WBTRULE_SINGLE:
    return 3;
  case WBTRULE_DOUBLE:
    return 4;
  case WBTRULE_TRIPLE:
  case WBTRULE_ROTATION:
    return 6;
  }
}

//------------------------------------------------------------------
/**
 * @brief WBTRULEとBTLRuleの間の変換
 */
//------------------------------------------------------------------
BtlRule WBTTOOL_WBTRuleToBtlRule( WBTRULE wbt_rule )
{
  SDK_COMPILER_ASSERT( BTL_RULE_SINGLE == WBTRULE_SINGLE - 1);
  SDK_COMPILER_ASSERT( BTL_RULE_DOUBLE == WBTRULE_DOUBLE - 1);
  SDK_COMPILER_ASSERT( BTL_RULE_TRIPLE == WBTRULE_TRIPLE - 1);
  SDK_COMPILER_ASSERT( BTL_RULE_ROTATION == WBTRULE_ROTATION - 1);
  GF_ASSERT( wbt_rule <= WBTRULE_ROTATION );
  return wbt_rule - 1;
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

//============================================================================================
//
//
//  トーナメント出場者選出
//
//
//============================================================================================

//-----------------------------------------------------------------------------
/**
 * @brief チャンピオントーナメント出場者選出
 * @todo  前作主人公の扱いが未実装
 */
//-----------------------------------------------------------------------------
static void entryChampTournament( u16 * tr_list )
{
  int tr_idx, j;

  GFL_STD_MemClear( tr_list, sizeof(u16) * WBT_TRAINER_NUM );
  tr_list[0] = WBT_TRCODE_PLAYER;
  tr_list[1] = WBT_TRCODE_WBHERO;

  for ( j = 0, tr_idx = 2; j < WBT_ROM_TRAINER_NUM; j ++ )
  {
    const WBT_TRDATA * rom_data = &WBTTrainerData[ j ];
    if ( rom_data->entry_champ == FALSE ) continue;
    tr_list[tr_idx] = j;
    tr_idx ++;
    if ( tr_idx >= WBT_TRAINER_NUM )
    {
      break;
    }
  }
  GF_ASSERT( j < WBT_ROM_TRAINER_NUM );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const u16 presentation_trid[WBT_TRAINER_NUM] = {
  34,//イブキ
  27,//ハヤト
  32,//ミカン
  35,//アンズ

  29,//アカネ
  33,//ヤナギ
  28,//ツクシ
  WBT_TRCODE_PLAYER//主人公
};
//-----------------------------------------------------------------------------
/**
 * @brief トーナメントリスト例を追加
 */
//-----------------------------------------------------------------------------
static void entrySampleTournament( u16 * tr_list )
{
#if 0
  static const u16 sample_trid[WBT_TRAINER_NUM] = {
    18,//シロナ
    55,//モブ
    13,//ハチク
    56,//モブ
    11,//デント
    53,//モブ
    12,//アロエ
    WBT_TRCODE_PLAYER,//プレイヤー
  };
  GFL_STD_MemCopy( sample_trid, tr_list, sizeof(u16) * WBT_TRAINER_NUM );
#endif
  GFL_STD_MemCopy( presentation_trid, tr_list, sizeof(u16) * WBT_TRAINER_NUM );
}

//-----------------------------------------------------------------------------
/**
 * @brief オールトーナメント出場者選出
 * @todo  未実装
 */
//-----------------------------------------------------------------------------
static void entryAllTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  entrySampleTournament( tr_list );
  //GFL_STD_MemClear( tr_list, sizeof(u16) * WBT_TRAINER_NUM );
  //tr_list[0] = WBT_TRCODE_PLAYER;   //プレイヤーをセット
  //  未実装…
}

//-----------------------------------------------------------------------------
/**
 * @brief タイプトーナメント出場者選出
 * @todo  未実装
 */
//-----------------------------------------------------------------------------
static void entryTypeTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  entrySampleTournament( tr_list );
  //GFL_STD_MemClear( tr_list, sizeof(u16) * WBT_TRAINER_NUM );
  //tr_list[0] = WBT_TRCODE_PLAYER;   //プレイヤーをセット
  //  未実装…
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WBTTOOL_MakeEntryList( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  switch (cond->type)
  {
  case WBTTYPE_NULL:
  case WBTTYPE_DOWNLOAD:
    GF_ASSERT_MSG( 0, "仮：まだできていません！\n");
    /* FALL THROUGH */
  case WBTTYPE_POKETYPE:
    entryTypeTournament( cond, tr_list );
    break;
  case WBTTYPE_ALL:
    entryAllTournament( cond, tr_list );
    break;
  case WBTTYPE_CHAMPION:
    entryChampTournament( tr_list );
    break;
  }
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 * @brief 対戦組み合わせのソート
 * @param wbt_tr   対戦トレーナー配列へのポインタ
 * @param
 *
 * ①プレイヤーの場所をランダムに決定
 * ②優先順位の最も高いキャラをプレイヤーと逆側のブロックに配置
 * ③次に優先順位が高いキャラがプレイヤー側２回戦であたる場所に配置
 * ④次に優先順位が高いキャラがプレイヤーとあたるように配置
 * ⑤他はランダムで配置
 */
//-----------------------------------------------------------------------------
void WBTTOOL_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64 )
{
    GFL_STD_RandContext rand_context;
    GFL_STD_RandInit( &rand_context, seed64 );
    //未実装！
}



