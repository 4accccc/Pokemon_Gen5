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
//#include "wbt_trlist_struct.h"  //WBT_TRDATA

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "wbt_system_local.h"

//#include "system/gfl_use.h" //GFUser_GetPublicRand0

#include "poke_tool/poketype_def.h"

//#include "tr_tool/trtype_def.h"
//#include "sound/pm_sndsys.h"

#include "arc_def.h"  //ARCID_～

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 title_id;       ///<トーナメント名のMsgID
  u8 win_count_id;    ///<WBTSAVE::WBTWIN_COUNT
  u8 reception_id;    ///<WBTRECEPT
  u8 reg_type;        ///<WBTREG_ID
  u8 btdata_id;       ///<WBTBTDATA

  u8 reward[4];       ///<ごほうびのBP数
}WBT_TOURNAMENT_DATA;


//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 name_id;          ///<ルール名のMsgID
  u8  need_pokemon_num; ///<必要なポケモン数
  BtlRule  btl_rule;        ///<対応するバトルルール定義
}WBT_RULE_DATA;

//------------------------------------------------------------------
//------------------------------------------------------------------
static const WBT_TOURNAMENT_DATA WBTTournamentData[ WBTCUP_MAX - 1 ] = {
  { //WBTCUP_CHAMPION, //チャンピオントーナメント
    msg_wbtsys_title_03,
    WBTWIN_COUNT_CHAMPION,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 0, 0, 0, 0 },
  },
  { //WBTCUP_POKETYPE, //タイプトーナメント
    msg_wbtsys_title_02,
    POKETYPE_NORMAL,   //ダミー
    WBTRECEPT_2, WBTREG_POKETYPE,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_DOWNLOAD, //ダウンロードトーナメント
    msg_wbtsys_title_04,
    WBTWIN_COUNT_MAX,   //ダミー
    WBTRECEPT_1, WBTREG_DOWNLOAD,
    WBTBTDATA_3,
    { 0, 0, 0, 0 },
  },

  {//WBTCUP_HODOMOE,     //ホドモエトーナメント           
    msg_wbtsys_title_05,
    WBTWIN_COUNT_HODOMOE,
    WBTRECEPT_1, WBTREG_ALL,
    WBTBTDATA_3,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_ISSYU,       //イッシュリーダーズトーナメント 
    msg_wbtsys_title_06,
    WBTWIN_COUNT_ISSYU,
    WBTRECEPT_2, WBTREG_ALL,
    WBTBTDATA_3,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_KANTO,       //カントーリーダーズトーナメント 
    msg_wbtsys_title_07,
    WBTWIN_COUNT_KANTO,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_JOUTO,       //ジョウトリーダーズトーナメント 
    msg_wbtsys_title_08,
    WBTWIN_COUNT_JOUTO,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_HOUEN,       //ホウエンリーダーズトーナメント 
    msg_wbtsys_title_09,
    WBTWIN_COUNT_HOUEN,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_SINOU,       //シンオウリーダーズトーナメント 
    msg_wbtsys_title_10,
    WBTWIN_COUNT_SINOU,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_2,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_WORLD,       //ワールドトーナメント     
    msg_wbtsys_title_11,
    WBTWIN_COUNT_WORLD,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 0, 0, 0, 0 },
  },
  {//WBTCUP_WCS,         //WCSトーナメント
    msg_wbtsys_title_17,
    WBTWIN_COUNT_WCS,
    WBTRECEPT_1, WBTREG_WCS,
    WBTBTDATA_1,
    { 0, 0, 0, 0 },
  },
  {//WBTCUP_HODOMOE_EVENT, //ホドモエトーナメント（イベント）           
    msg_wbtsys_title_05,
    WBTWIN_COUNT_HODOMOE,
    WBTRECEPT_1, WBTREG_ALL,
    WBTBTDATA_1,
    { 1, 0, 0, 0 },
  },
  {//WBTCUP_RENTAL,         //レンタルトーナメント
    msg_wbtsys_title_12,
    WBTWIN_COUNT_RENTAL,
    WBTRECEPT_1, WBTREG_SUBWAY,
    WBTBTDATA_1,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_RENTALMASTER,         //レンタルマスタートーナメント
    msg_wbtsys_title_13,
    WBTWIN_COUNT_RENTALMASTER,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },
  {//WBTCUP_SHUFFLE,         //シャッフルトーナメント
    msg_wbtsys_title_14,
    WBTWIN_COUNT_SHUFFLE,
    WBTRECEPT_1, WBTREG_SUBWAY,
    WBTBTDATA_1,
    { 1, 1, 2, 2 },
  },
  {//WBTCUP_SHUFFLEMASTER,         //シャッフルマスタートーナメント
    msg_wbtsys_title_15,
    WBTWIN_COUNT_SHUFFLEMASTER,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 2, 2, 3, 3 },
  },

};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const WBT_RULE_DATA WBTRuleData[] =
{
  {// WBTRULE_NULL（ダミーデータ）
    msg_wbtsys_rule_single,
    3, BTL_RULE_SINGLE,
  },
  {// WBTRULE_SINGLE
    msg_wbtsys_rule_single,
    3, BTL_RULE_SINGLE,
  },
  {// WBTRULE_DOUBLE
    msg_wbtsys_rule_double,
    4, BTL_RULE_DOUBLE,
  },
  {// WBTRULE_TRIPLE
    msg_wbtsys_rule_triple,
    6, BTL_RULE_TRIPLE,
  },
  {// WBTRULE_ROTATION
    msg_wbtsys_rule_rotation,
    6, BTL_RULE_ROTATION,
  }
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief トーナメント名文字列をセット
 * @todo  ダウンロードデータへの対応（引数がWBTSYSになる）
 */
//------------------------------------------------------------------
void WBTTOOL_GetTournamentName( WBTCUP wbtcup, STRBUF * strbuf )
{
  HEAPID heapID = HEAPID_FIELDMAP;
  GFL_MSGDATA     *man;
  u16 msg_id;

  if ( wbtcup == WBTCUP_NULL || wbtcup >= WBTCUP_MAX )
  {
    GF_ASSERT( 0 );
    wbtcup = WBTCUP_HODOMOE;
  }
  switch ( wbtcup )
  {
  case WBTCUP_DOWNLOAD:
    //仮：ダウンロードタイトル名には未対応
  case WBTCUP_NULL:
    GF_ASSERT(0);
    /* FALL THROUGH */
  default:
    msg_id = WBTTournamentData[ wbtcup - 1 ].title_id;
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
void WBTTOOL_GetRomTrainerName( HEAPID heapID, u8 tr_id, STRBUF * strbuf )
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
 * @brief WBTロムトレーナータイプ名を取得
 * @todo  「ポケモントレーナー」ばっかりのWBTでの肩書アーカイブからに置き換える
 */
//------------------------------------------------------------------
void WBTTOOL_GetRomTrainerTypeName( HEAPID heapID, u8 tr_id, STRBUF * strbuf )
{
  GFL_MSGDATA     *man;
  u16 msg_id = tr_id;
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_trtype_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}


//============================================================================================
//
//  ルール関連データ取得
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static WBTRULE checkWBTRuleRange( WBTRULE wbt_rule )
{
  if ( WBTRULE_SINGLE > wbt_rule && wbt_rule > WBTRULE_ROTATION )
  {
    GF_ASSERT( 0 );
    wbt_rule = WBTRULE_SINGLE;
  }
  return wbt_rule;
}

//------------------------------------------------------------------
/**
 * @brief ルール名文字列をセット
 */
//------------------------------------------------------------------
void WBTTOOL_GetRuleName( WBTRULE rule, STRBUF * strbuf )
{
  HEAPID heapID = HEAPID_FIELDMAP;
  GFL_MSGDATA     *man;
  u16 msg_id;
  rule = checkWBTRuleRange( rule );
  msg_id = WBTRuleData[rule].name_id;
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, heapID );
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
  rule = checkWBTRuleRange( rule );
  return WBTRuleData[ rule ].need_pokemon_num;
}

//------------------------------------------------------------------
/**
 * @brief WBTRULEとBTLRuleの間の変換
 */
//------------------------------------------------------------------
BtlRule WBTTOOL_WBTRuleToBtlRule( WBTRULE wbt_rule )
{
  wbt_rule = checkWBTRuleRange( wbt_rule );
  return WBTRuleData[ wbt_rule ].btl_rule;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTRULE WBTTOOL_BtlRuleToWBTRule( BtlRule btl_rule )
{
SDK_COMPILER_ASSERT( BTL_RULE_SINGLE == WBTRULE_SINGLE - 1);
SDK_COMPILER_ASSERT( BTL_RULE_DOUBLE == WBTRULE_DOUBLE - 1);
SDK_COMPILER_ASSERT( BTL_RULE_TRIPLE == WBTRULE_TRIPLE - 1);
SDK_COMPILER_ASSERT( BTL_RULE_ROTATION == WBTRULE_ROTATION - 1);

  WBTRULE wbt_rule;

  wbt_rule = btl_rule + 1;
  wbt_rule = checkWBTRuleRange( wbt_rule );
  return wbt_rule;
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
//============================================================================================

#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  //SYS_FLAG_GAMECLEAR

enum {
  CHAMP_OPEN_COUNT = 10,  ///<仮：１０回クリアでチャンプトーナメント解除
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void makeWinCountArray( GAMEDATA * gamedata, u16 * win_count_array )
{
  int i;
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );

  for ( i = 0; i < WBTWIN_COUNT_MAX; i ++ )
  {
    win_count_array[i] = WBTSAVE_GetWinCount( wbtsave, i );
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL checkCupEnable( GAMEDATA * gamedata, WBTCUP wbtcup, const u16 * win_count )
{
  EVENTWORK * evwork = GAMEDATA_GetEventWork( gamedata );
  BOOL all_region_clear;

  //全地方のリーダーズトーナメントをクリアか？
  all_region_clear =
    ( win_count[ WBTWIN_COUNT_ISSYU ] != 0 &&
      win_count[ WBTWIN_COUNT_KANTO ] != 0 &&
      win_count[ WBTWIN_COUNT_JOUTO ] != 0 &&
      win_count[ WBTWIN_COUNT_HOUEN ] != 0 &&
      win_count[ WBTWIN_COUNT_SINOU ] != 0 );

  switch ( (WBTCUP)wbtcup )
  {
  case WBTCUP_NULL:     //トーナメント未指定
    GF_ASSERT( 0 );
    break;
  case WBTCUP_CHAMPION: //チャンピオントーナメント
    return ( win_count[ WBTWIN_COUNT_WORLD ] >= CHAMP_OPEN_COUNT );

  case WBTCUP_POKETYPE: //タイプトーナメント
    //すべてのリーダーズトーナメント
    return all_region_clear;

  case WBTCUP_DOWNLOAD: //ダウンロードトーナメント
    return ( win_count[ WBTWIN_COUNT_HODOMOE ] != 0 );

  case WBTCUP_HODOMOE_EVENT:  ///ホドモエトーナメント（イベント）           
    return ( win_count[ WBTWIN_COUNT_HODOMOE ] == 0 );

  case WBTCUP_RENTAL:
  case WBTCUP_SHUFFLE:
  case WBTCUP_HODOMOE:     //ホドモエトーナメント           
    return ( win_count[ WBTWIN_COUNT_HODOMOE ] != 0 );
  case WBTCUP_ISSYU:       //イッシュリーダーズトーナメント 
    return EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR );

  case WBTCUP_KANTO:       //カントーリーダーズトーナメント 
  case WBTCUP_JOUTO:       //ジョウトリーダーズトーナメント 
  case WBTCUP_HOUEN:       //ホウエンリーダーズトーナメント 
  case WBTCUP_SINOU:       //シンオウリーダーズトーナメント 
    return (win_count[ WBTWIN_COUNT_ISSYU ] != 0);

  case WBTCUP_RENTALMASTER:
    return ( win_count[ WBTWIN_COUNT_RENTAL ] != 0 && all_region_clear );

  case WBTCUP_SHUFFLEMASTER:
    return ( win_count[ WBTWIN_COUNT_SHUFFLE ] != 0 && all_region_clear );

  case WBTCUP_WORLD:        //ワールドリーダーズトーナメント     
    return all_region_clear;

  case WBTCUP_WCS:          //WCSトーナメント
    return ( win_count[ WBTWIN_COUNT_HODOMOE ] != 0 );
  }
  //よくわからないものは開催できない
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 * @brief トーナメントが開催可能かどうかの判定処理
 * @param gamedata
 * @param wbtcup
 * @retval  BOOL  TRUEのとき、開催可能
 */
//-----------------------------------------------------------------------------
BOOL WBTTOOL_CheckCupEnable( GAMEDATA * gamedata, WBTCUP wbtcup )
{
  u16 win_count[ WBTWIN_COUNT_MAX ];

  makeWinCountArray( gamedata, win_count );
  return checkCupEnable( gamedata, wbtcup, win_count );
}

//-----------------------------------------------------------------------------
/**
 * @brief トーナメントが解除されるかどうかの判定処理
 * @param gamedata
 * @param try_cup     現在挑戦しようとしているトーナメント
 * @param check_cup   解除されるかどうか判定する対象のトーナメント
 *
 * トーナメント勝利以外で解除されるものは判定できない
 */
//-----------------------------------------------------------------------------
BOOL WBTTOOL_CheckCupUnlock( GAMEDATA * gamedata, WBTCUP try_cup, WBTCUP check_cup )
{
  u16 win_count[ WBTWIN_COUNT_MAX ];
  WBTWIN_COUNT count_id;
  BOOL before_status, after_status;

  makeWinCountArray( gamedata, win_count );
  //現在、開催可能かどうか
  before_status = checkCupEnable( gamedata, check_cup, win_count );

  //現在のトーナメントに勝ったら…という条件にする
  count_id = WBTTOOL_WBTCupToWinCount( try_cup, 0 );
  if ( count_id != WBTWIN_COUNT_MAX )
  {
    win_count[ count_id ] ++;
  }
  after_status = checkCupEnable( gamedata, check_cup, win_count );

  //現在開催不可能で、トーナメントに勝ったら開催可能の場合→解除
  return ( before_status == FALSE && after_status != before_status );
}


//-----------------------------------------------------------------------------
/**
 * @brief   開催トーナメントIDから優勝回数カウンタの指定IDを取得する
 * @param wbtcup    開催トーナメント指定
 * @param poketype  ポケモンタイプ指定（タイプトーナメント用）
 * @retval  WBTWIN_COUNT  優勝回数カウンタ指定（WBTWIN_COUNT_MAXの場合、該当なし）
 */
//-----------------------------------------------------------------------------
WBTWIN_COUNT WBTTOOL_WBTCupToWinCount( WBTCUP wbtcup, u8 poketype )
{
  WBTWIN_COUNT id;

  if ( 0 <= wbtcup && wbtcup < WBTCUP_MAX )
  {
    if ( wbtcup == WBTCUP_POKETYPE )
    {
      //PokeTypeとWBTWIN_COUNT_が一致することはAssertで保障
      return poketype;
    }
    return WBTTournamentData[ wbtcup - 1 ].win_count_id;
  }
  GF_ASSERT_MSG( 0, "error wbtcup == %d\n", wbtcup );
  return WBTWIN_COUNT_MAX;
}
//-----------------------------------------------------------------------------
/**
 * @brief 優勝回数カウンタ指定IDから開催トーナメントIDを取得する
 * @param counter_id　WBTWIN_COUNT（include/savedata/wbt_save.h参照）
 * @retval  WBTCUP（include/field/wbt.h参照）
 */
//-----------------------------------------------------------------------------
WBTCUP WBTTOOL_WBTWinCountToCup( WBTWIN_COUNT counter_id )
{
  int i;

  if ( counter_id <= WBTWIN_COUNT_POKETYPE_AKU )
  { //タイプエキスパートのカウンタ→タイプエキスパートで
    return WBTCUP_POKETYPE;
  }

  for ( i = 0; i < NELEMS(WBTTournamentData); i ++ )
  {
    if ( WBTTournamentData[i].win_count_id == counter_id )
    {
      return i + 1; //WBTTournamentDataはWBTCUP_NULL分がないので
    }
  }
  GF_ASSERT_MSG( 0, "error wbt win count id = %d\n", counter_id );
  return WBTCUP_HODOMOE;  //念のため
}

//-----------------------------------------------------------------------------
/**
 * @brief 開催トーナメントIDからレギュレーション種別を取得する
 * @param   wbtcup    開催トーナメント指定
 * @retval  WBTREG_ID レギュレーション種別
 */
//-----------------------------------------------------------------------------
WBTREG_ID WBTTOOL_GetRegulationID( WBTCUP wbtcup )
{
  if ( 0 <= wbtcup && wbtcup < WBTCUP_MAX )
  {
    return WBTTournamentData[ wbtcup - 1 ].reg_type;
  }
  GF_ASSERT_MSG( 0, "error wbtcup == %d\n", wbtcup );
  return WBTREG_ALL;
}

//-----------------------------------------------------------------------------
/**
 * @brief 指定トーナメントが指定受付IDから呼び出し可能かどうかのチェック
 * @param wbtcup
 * @param reception_id
 */
//-----------------------------------------------------------------------------
BOOL WBTTOOL_CheckReceptionID( WBTCUP wbtcup, WBTRECEPT reception_id )
{
  if ( 0 <= wbtcup && wbtcup < WBTCUP_MAX )
  {
    return ( WBTTournamentData[ wbtcup - 1 ].reception_id == reception_id );
  }
  GF_ASSERT_MSG( 0, "error wbtcup == %d\n", wbtcup );
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 * @brief トレーナーデータのアーカイブ指定を取得する
 * @param wbtcup
 * @return  u32   アーカイブ指定ID
 */
//-----------------------------------------------------------------------------
u32 WBTTOOL_GetRomArcID( WBTCUP wbtcup )
{
  static const u16 btdIdTable[WBTBTDATA_MAX] = {
    ARCID_WBT_TD1A,
    ARCID_WBT_TD2A,
    ARCID_WBT_TD3A,
  };

  u32 index;

  if ( 0 <= wbtcup && wbtcup < WBTCUP_MAX )
  {
    index = WBTTournamentData[ wbtcup - 1 ].btdata_id;
  }
  else
  {
    GF_ASSERT_MSG( 0, "error wbtcup == %d\n", wbtcup );
    index = WBTBTDATA_1;
  }
  return btdIdTable[ index ];
}

//-----------------------------------------------------------------------------
/**
 * @brief   ごほうびのBP数取得
 * @param wbtcup
 * @param wbtrule
 * @return  u32   BPの数
 */
//-----------------------------------------------------------------------------
u32 WBTTOOL_GetRewardCount( WBTCUP wbtcup, WBTRULE wbtrule )
{
  if ( wbtcup <= WBTCUP_NULL || wbtcup >= WBTCUP_MAX )
  {
    GF_ASSERT_MSG( 0, "error wbtcup == %d\n", wbtcup );
    wbtcup = WBTCUP_HODOMOE;
  }
  if ( wbtrule <= WBTRULE_NULL || wbtrule >= WBTRULE_MAX )
  {
    GF_ASSERT_MSG( 0, "error wbtrule == %d\n", wbtrule );
    wbtrule = WBTRULE_SINGLE;
  }
  return WBTTournamentData[ wbtcup - 1].reward[ wbtrule - 1];
}















