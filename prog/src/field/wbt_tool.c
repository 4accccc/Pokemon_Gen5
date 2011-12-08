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


#include "wbt_tool.h"
//#include "wbt_trlist_struct.h"  //WBT_TRDATA

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "wbt_system_local.h"

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h"

#include "arc_def.h"  //ARCID_～
#include "arc/wbt/wbt_trlist.naix"

SDK_COMPILER_ASSERT( BTL_RULE_SINGLE == WBTRULE_SINGLE );
SDK_COMPILER_ASSERT( BTL_RULE_DOUBLE == WBTRULE_DOUBLE );
SDK_COMPILER_ASSERT( BTL_RULE_TRIPLE == WBTRULE_TRIPLE );
SDK_COMPILER_ASSERT( BTL_RULE_ROTATION == WBTRULE_ROTATION );

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 name_id;          ///<ルール名のMsgID
  u8  need_pokemon_num; ///<必要なポケモン数
  BtlRule  btl_rule;        ///<対応するバトルルール定義
}WBT_RULE_DATA;

//------------------------------------------------------------------
//------------------------------------------------------------------
static const WBT_RULE_DATA WBTRuleData[] =
{
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
    4, BTL_RULE_ROTATION,
  }
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief WBTロムトレーナー名を取得
 */
//------------------------------------------------------------------
void WBTTOOL_GetRomTrainerName( HEAPID heapID, WBTROM_TRID wbt_tr_id, STRBUF * strbuf )
{
  GFL_MSGDATA     *man;
  u16 msg_id = wbt_tr_id;
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
 * @param[in]  heapID
 * @param[in]  wbt_tr_id
 * @param[out] strbuf
 */
//------------------------------------------------------------------
void WBTTOOL_GetRomTrainerTypeName( HEAPID heapID, WBTROM_TRID wbt_tr_id, STRBUF * strbuf )
{
  GFL_MSGDATA     *man;
  u16 msg_id = wbt_tr_id;
  man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_trtitle_dat, heapID );
  if( man )
  {
    GFL_MSG_GetString( man, msg_id, strbuf );
    GFL_MSG_Delete( man );
  }
}

//------------------------------------------------------------------
/**
 * @brief WBTロムトレーナーデータを読み込む
 * @param[in] heapID          使用するヒープID指定
 * @return    WBTROM_TRDATA   読み込んだトレーナーデータへのポインタ
 *
 * 内部でAllocするので、呼び出し元で最終的に解放する必要がある
 */
//------------------------------------------------------------------
const WBTROM_TRDATA * WBTTOOL_CreateRomTrainerData( HEAPID heapID )
{
  WBTROM_TRDATA * output;
  ARCHANDLE * handle;
  u32 size;
  handle = GFL_ARC_OpenDataHandle( ARCID_WBT_TRLIST, GFL_HEAP_LOWID(heapID) );
  size = GFL_ARC_GetDataSizeByHandle( handle, NARC_wbt_trlist_wbt_trainer_bin );
  GF_ASSERT( size % sizeof(WBT_TRDATA) == 0 );
  output = GFL_HEAP_AllocClearMemory( heapID, sizeof(u32) + size );
  output->num = size / sizeof(WBT_TRDATA);
  GFL_ARC_LoadDataByHandle( handle, NARC_wbt_trlist_wbt_trainer_bin, &output->data );
  GFL_ARC_CloseDataHandle( handle );

  GF_ASSERT( WBTROM_TRID_INVALID > output->num );
  return (const WBTROM_TRDATA *)output;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTTOOL_DeleteRomTrainerData( WBTROM_TRDATA * rom_trdata )
{
  GFL_HEAP_FreeMemory( rom_trdata );
}

//------------------------------------------------------------------
/**
 * @brief ROMトレーナーデータの取得
 * @param heapID
 * @param tr_id
 * @param tr_data
 */
//------------------------------------------------------------------
void WBTTOOL_GetRomTrainerData(
    const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id, WBT_TRDATA * tr_data )
{
  GF_ASSERT( tr_id < rom_trdata->num );
  *tr_data = rom_trdata->data[ tr_id ];
}

//------------------------------------------------------------------
/**
 * @brief WBTロムトレーナーのIDとして有効かどうかをチェック
 * @param[in] rom_trdata
 * @param[in] tr_id
 * @return  BOOL  TRUEのとき、有効なID
 */
//------------------------------------------------------------------
BOOL WBTTOOL_IsVaridRomTrainerID( const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id )
{
  return ( tr_id < rom_trdata->num );
}

//------------------------------------------------------------------
/**
 * @brief 前作主人公のWBTロムデータのIDを取得する
 * @param   sex
 * @return  u16
 */
//------------------------------------------------------------------
WBTROM_TRID WBTTOOL_GetBWRomTrainerID( const WBTROM_TRDATA * rom_trdata, u8 sex )
{
  WBTROM_TRID tr_id;
  u16 search_mmdl = HERO;
  if (sex == PM_MALE)
  {
    search_mmdl = HEROINE;
  }
  for ( tr_id = 0; tr_id < rom_trdata->num; tr_id ++ )
  {
    if ( rom_trdata->data[tr_id].mmdl_id == search_mmdl ) return tr_id;
  }
  GF_ASSERT( 0 );
  return 0; //仮
}

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
WBTROM_TRID WBTTOOL_GetRivalTrainerID( const WBTROM_TRDATA * rom_trdata )
{
  WBTROM_TRID tr_id;
  for ( tr_id = 0; tr_id < rom_trdata->num; tr_id ++ )
  {
    if ( rom_trdata->data[tr_id].mmdl_id == SWRIVAL ) return tr_id;
  }
  GF_ASSERT( 0 );
  return 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 WBTTOOL_GetRomTrainerBtlTrType( const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id )
{
  GF_ASSERT( tr_id < rom_trdata->num );
  return rom_trdata->data[ tr_id ].tr_type;
}

//------------------------------------------------------------------
/**
 * @brief ROMトレーナーデータの動作モデルID取得
 * @param[in]   rom_trdata
 * @param[in]   tr_id
 * @return      u16
 */
//------------------------------------------------------------------
u16 WBTTOOL_GetRomTrainerMMdlID( const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id )
{
  GF_ASSERT( tr_id < rom_trdata->num );
  return rom_trdata->data[ tr_id ].mmdl_id;
}

//------------------------------------------------------------------
/**
 * @brief ROMトレーナーデータの優先度取得
 */
//------------------------------------------------------------------
WBT_TRPRI WBTTOOL_GetRomTrainerPriority( const WBTROM_TRDATA * rom_trdata, WBTROM_TRID tr_id )
{
  GF_ASSERT( tr_id < rom_trdata->num );
  return rom_trdata->data[ tr_id ].priority;
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
  SDK_COMPILER_ASSERT( NELEMS(WBTRuleData) == WBTRULE_MAX );

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
  BtlRule btl_rule;
  wbt_rule = checkWBTRuleRange( wbt_rule );
  btl_rule = WBTRuleData[ wbt_rule ].btl_rule;
  GF_ASSERT( btl_rule == wbt_rule );
  return btl_rule;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTRULE WBTTOOL_BtlRuleToWBTRule( BtlRule btl_rule )
{
  WBTRULE wbt_rule;
  wbt_rule = checkWBTRuleRange( btl_rule );
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
  CHAMP_OPEN_COUNT = 10,  ///<ワールドリーダーズ１０回クリアでチャンピオントーナメント解除
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
  case WBTCUP_MIX:
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

  case WBTCUP_MIXMASTER:
    return ( win_count[ WBTWIN_COUNT_SHUFFLE ] != 0 && all_region_clear );

  case WBTCUP_WORLD:        //ワールドリーダーズトーナメント     
    return all_region_clear;

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

  if ( wbtcup == WBTCUP_POKETYPE )
  {
    //PokeTypeとWBTWIN_COUNT_が一致することはAssertで保障
    return poketype;
  }
  return WBTTOOL_GetTournamentData( wbtcup )->win_count_id;
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
  return WBTTOOL_GetTournamentData( wbtcup )->reg_type;
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
  const WBT_TOURNAMENT_DATA * target = WBTTOOL_GetTournamentData( wbtcup );
  return ( target->reception_id == reception_id );
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

  index = WBTTOOL_GetTournamentData( wbtcup ) ->btdata_id;
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
  wbtrule = checkWBTRuleRange( wbtrule );
  return WBTTOOL_GetTournamentData( wbtcup )->reward[ wbtrule ];
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
u32 WBTTOOL_GetDLTrtype( u8 wbt_dl_tr_type )
{
  static const u8 id2trtype[] = {
    TRTYPE_TANPAN,        //	01	BOY2	○	♂	たんぱんこぞう	
    TRTYPE_ELITEM,        //	02	TRAINERM	○	♂	エリートトレーナ－	
    TRTYPE_RANGERM,       //	03	RANGERM	○	♂	ポケモンレンジャー	
    TRTYPE_BREEDERM,      //	04	BREEDERM	○	♂	ポケモンブリーダー	
    TRTYPE_SCIENTISTM,    //	05	ASSISTANTM	○	♂	けんきゅういん	
    TRTYPE_MOUNT,         //	06	MOUNTMAN	○	♂	やまおとこ	
    TRTYPE_HEADS,         //	07	BADMAN	○	♂	スキンヘッズ	
    TRTYPE_KINDERGARTENM, //	08	BABYBOY2	○	♀	えんじ	
    TRTYPE_MINI,          //	09	GIRL2	○	♀	ミニスカート	
    TRTYPE_ELITEW,        //	10	TRAINERW	○	♀	エリートトレーナ－	
    TRTYPE_RANGERW,       //	11	RANGERW	○	♀	ポケモンレンジャー	
    TRTYPE_BREEDERW,      //	12	BREEDERW	○	♀	ポケモンブリーダー	
    TRTYPE_SCIENTISTW,    //	13	ASSISTANTW	○	♀	けんきゅういん	
    TRTYPE_PARASOL,       //	14	AMBRELLA	○	♀	パラソルおねえさん	
    TRTYPE_NURSE,         //	15	NURSE	○	♀	ナース	
    TRTYPE_KINDERGARTENW, //	16	BABYGIRL2	○	♀	えんじ	
    TRTYPE_BACKPACKERM,   //	17	BACKPACKERM	×	♂	バックパッカー	
    TRTYPE_BACKPACKERW,   //	18	BACKPACKERW	×	♀	バックパッカー	
    TRTYPE_VETERANM,      //	19	VETERANM	×	♂	ベテラントレーナー	
    TRTYPE_VETERANW,      //	20	VETERANW	×	♀	ベテラントレーナー	
    TRTYPE_SCHOOLB,       //	21	BOY3	×	♂	じゅくがえり	
    TRTYPE_SCHOOLG,       //	22	GIRL3	×	♀	じゅくがえり	
    TRTYPE_ESPM,          //	23	ESPM	×	♂	サイキッカー	
    TRTYPE_ESPW,          //	24	ESPW	×	♀	サイキッカー	
    TRTYPE_KARATE,        //	25	FIGHTERM	×	♂	からておう	
    TRTYPE_BATTLEG,       //	26	FIGHTERW	×	♀	バトルガール	
    TRTYPE_BUSINESS1,     //	27	BUSINESSMAN	×	♂	ビジネスマン	若い
    TRTYPE_BUSINESS2,     //	28	BUSINESSMAN	×	♂	ビジネスマン	中年
    TRTYPE_OL,            //	29	OL	×	♀	ＯＬ	
    TRTYPE_MAID,          //	30	MAID	×	♀	メイド	
  };
  if( wbt_dl_tr_type >= NELEMS(id2trtype) )
  {
    GF_ASSERT( 0 );
    wbt_dl_tr_type = 0;
  }
  return id2trtype[ wbt_dl_tr_type ];
}










