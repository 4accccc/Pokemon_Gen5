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
static const WBT_TOURNAMENT_DATA WBTTournamentData[ WBTCUP_MAX - 1 ] = {
  { //WBTCUP_CHAMPION, //チャンピオントーナメント
    msg_wbtsys_title_02,
    WBTWIN_COUNT_CHAMPION,
    WBTRECEPT_2, WBTREG_SUBWAY,
    WBTBTDATA_3,
    { 0, 0, 0, 0 },
  },
  { //WBTCUP_POKETYPE, //タイプトーナメント
    msg_wbtsys_title_03,
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
void WBTTOOL_GetTrRomData( u8 tr_id, WBT_TRDATA * tr_data )
{
  GF_ASSERT( tr_id < WBT_ROM_TRAINER_NUM );
  *tr_data = WBTTrainerData[ tr_id ];
}

//------------------------------------------------------------------
/**
 * @brief 前作主人公のWBTロムデータのIDを取得する
 * @param   sex
 * @return  u16
 */
//------------------------------------------------------------------
u16 WBTTOOL_GetBWRomTrainerID( u8 sex )
{
  int i;
  u16 search_id = HERO;
  if (sex == PM_FEMALE)
  {
    search_id = HEROINE;
  }
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if ( WBTTrainerData[i].mmdl_id == search_id ) return i;
  }
  GF_ASSERT( 0 );
  return 0; //仮
}

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
u16 WBTTOOL_GetRivalTrainerID( void )
{
  int i;
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if ( WBTTrainerData[i].mmdl_id == SWRIVAL ) return i;
  }
  GF_ASSERT( 0 );
  return 0;
}

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
 * @brief ルール名文字列をセット
 */
//------------------------------------------------------------------
void WBTTOOL_GetRuleName( WBTRULE rule, STRBUF * strbuf )
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
/// エントリー状態
//-----------------------------------------------------------------------------
typedef enum {
  ENTRY_NON = 0,    ///<エントリーされていない
  ENTRY_CANDIDATE,  ///<出場候補
  ENTRY_DECIDE,     ///<出場決定
}ENTRY_CODE;

//-----------------------------------------------------------------------------
/// 指定したENTRY_CODEを数える
//-----------------------------------------------------------------------------
static u32 countEntry( const u8 * entry_table, ENTRY_CODE entryCode )
{
  u32 count = 0;
  int i;
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if ( entry_table[i] == entryCode ) count ++;
  }
  return count;
}
//-----------------------------------------------------------------------------
/// 指定したENTRY_CODEとPRIのものを数える
//-----------------------------------------------------------------------------
static u32 countEntryWithPri( const u8 * entry_table, ENTRY_CODE entryCode, u8 pri )
{
  u32 candidate_count = 0;
  int i;
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if (entry_table[i] == entryCode)
    {
      if ( pri == WBT_TRPRI_NULL || WBTTrainerData[i].priority == pri)
      {
        candidate_count ++;
      }
    }
  }
  return candidate_count;
}
//-----------------------------------------------------------------------------
/**
 * @param entry_table   検索状態保持テーブル
 * @param pri           検索する候補者の優先度（WBT_TRPRI_NULLのとき、無条件）
 * @param step          何人目を選ぶか
 * @retval  u32         発見したentry_table上の位置を示すインデックス値
 */
//-----------------------------------------------------------------------------
static u32 selectCandidateByPri( const u8 * entry_table, u8 pri, u8 step )
{
  u32 pos_count = step;
  u32 i = 0;
  GF_ASSERT( step > 0 )
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if ( entry_table[i] == ENTRY_CANDIDATE )
    {
      if ( pri == WBT_TRPRI_NULL || WBTTrainerData[i].priority == pri )
      {
        pos_count --;
        if ( pos_count == 0 )
        {
          return i;
        }
      }
    }
  }
  GF_ASSERT( i < WBT_ROM_TRAINER_NUM );
  return i;
}

//-----------------------------------------------------------------------------
/**
 * @param entry_table   検索状態保持テーブル
 * @param rand_context  使用する乱数コンテキスト
 * @param pri           選択する候補者の優先度（WBT_TRPRI_NULLのとき、無条件）
 * @param num           選択する人数
 */
//-----------------------------------------------------------------------------
static u32 selectCharacters( u8 * entry_table, GFL_STD_RandContext * rand_context, u8 pri, u8 num )
{
  int i;
  int count = num;      //残り選択人数
  u32 candidate_count;  //候補者数
  
  candidate_count = countEntryWithPri( entry_table, ENTRY_CANDIDATE, pri );

  //候補数以上に選ぶ必要がある場合はAssert
  GF_ASSERT( count <= candidate_count );
  while ( count )
  {
    u32 select;
    if ( candidate_count == 1 )
    { //場所はランダムで決めるまでもない
      select = selectCandidateByPri( entry_table, pri, 1 );
    }
    else
    {
      u32 step;
      //何人目を選択するかはランダムで
      step = GFL_STD_Rand( rand_context, candidate_count ) + 1;
      select = selectCandidateByPri( entry_table, pri, step );
    }
    GF_ASSERT( entry_table[select] == ENTRY_CANDIDATE );
    entry_table[select] = ENTRY_DECIDE;
    TAMADA_Printf("SELECT TRAINER(%03d):pri(%d)candidate(%2d)\n", select, pri, candidate_count );
    candidate_count --;
    count --;
    if ( candidate_count == 0 && count ) return count;
  }


  //選から漏れた候補者を消す
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if ( entry_table[i] == ENTRY_CANDIDATE )
    {
      if ( pri == WBT_TRPRI_NULL || WBTTrainerData[i].priority == pri )
      {
        entry_table[i] = ENTRY_NON;
      }
    }
  }
  return 0;
}


//-----------------------------------------------------------------------------
/**
 * @brief トレーナーが参戦可能なトーナメントかどうかのチェック
 */
//-----------------------------------------------------------------------------
static BOOL checkEnableEntryRomTrainer( const WBT_MATCH_CONDITION * cond, const WBT_TRDATA * rom_data )
{
  switch ( cond->wbtcup )
  {
  default:
  case WBTCUP_NULL:
  case WBTCUP_DOWNLOAD:
    GF_ASSERT( 0 );
    break;

  case WBTCUP_WCS:
  case WBTCUP_ISSYU:
  case WBTCUP_KANTO:
  case WBTCUP_JOUTO:
  case WBTCUP_HOUEN:
  case WBTCUP_SINOU:
  case WBTCUP_CHAMPION:
    return ( rom_data->region_tournament_id == cond->wbtcup );

  case WBTCUP_POKETYPE:
    return ( rom_data->type_tournament_id == cond->poketype
      || rom_data->type_tournament_id == POKETYPE_NULL );

  case WBTCUP_HODOMOE:
    return rom_data->entry_hodomoe;
  case WBTCUP_HODOMOE_EVENT:
    return rom_data->entry_hodomoe_event;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void makeEntryTable( const WBT_MATCH_CONDITION * cond, u8 * entry_table )
{
  int i;
  GFL_STD_MemFill( entry_table, ENTRY_NON, WBT_ROM_TRAINER_NUM );
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    const WBT_TRDATA * rom_data = &WBTTrainerData[ i ];
    if ( checkEnableEntryRomTrainer( cond, rom_data ) == TRUE )
    {
      entry_table[ i ] = ENTRY_CANDIDATE;
    }
  }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void setEntries( u16 * tr_list, const u8 * entry_table )
{
  //選択された結果を戻り値テーブルに反映する
  u32 tr_pos = 0;
  u32 i;
  tr_list[tr_pos ++ ] = WBT_TRCODE_PLAYER;  //プレイヤーは登録する
  for ( i = 0; i < WBT_ROM_TRAINER_NUM; i ++ )
  {
    if ( entry_table[i] == ENTRY_DECIDE )
    {
      if ( tr_pos < WBT_TRAINER_NUM )
      {
        tr_list[ tr_pos ] = i;
        tr_pos ++;
      }
      else
      {
        GF_ASSERT( 0 );
      }
    }
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief チャンピオントーナメント出場者選出
 * @todo  前作主人公の扱いが未実装
 */
//-----------------------------------------------------------------------------
static void entryChampTournament(const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  int tr_idx, j;

  GFL_STD_MemClear( tr_list, sizeof(u16) * WBT_TRAINER_NUM );
  tr_list[0] = WBT_TRCODE_PLAYER;
  tr_list[1] = WBT_TRCODE_WBHERO;

  for ( j = 0, tr_idx = 2; j < WBT_ROM_TRAINER_NUM; j ++ )
  {
    const WBT_TRDATA * rom_data = &WBTTrainerData[ j ];
    if ( rom_data->region_tournament_id != WBTCUP_CHAMPION ) continue;
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
/**
 * @brief 参戦トレーナー選別：ワールドトーナメント
 */
//-----------------------------------------------------------------------------
static void entryWorldTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  u8 entry_table[WBT_ROM_TRAINER_NUM];

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  makeEntryTable( cond, entry_table );
  //チャンピオン一人だけ参戦
  //selectCharacters( entry_table, &rand_context, WBT_TRPRI_CHAMP, 1 );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, 7 );

  //選択された結果を戻り値テーブルに反映する
  setEntries( tr_list, entry_table );
}
//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：リーダーズトーナメント
 */
//-----------------------------------------------------------------------------
static void entryRegionTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  u8 entry_table[WBT_ROM_TRAINER_NUM];

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  makeEntryTable( cond, entry_table );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, 7 );

  //選択された結果を戻り値テーブルに反映する
  setEntries( tr_list, entry_table );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：WCSトーナメント
 */
//-----------------------------------------------------------------------------
static void entryWCSTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  u8 entry_table[WBT_ROM_TRAINER_NUM];

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  makeEntryTable( cond, entry_table );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_SPECIAL, 1 );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_CHAMP, 1 );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, 2 );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_WEAK_MOB, 3 );

  //選択された結果を戻り値テーブルに反映する
  setEntries( tr_list, entry_table );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：ホドモエトーナメント
 */
//-----------------------------------------------------------------------------
static void entryHodomoeTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  u8 entry_table[WBT_ROM_TRAINER_NUM];

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  makeEntryTable( cond, entry_table );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, 2 );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_MOB, 1 );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_WEAK_MOB, 4 );

  //選択された結果を戻り値テーブルに反映する
  setEntries( tr_list, entry_table );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：ホドモエトーナメント（イベント）
 */
//-----------------------------------------------------------------------------
static void entryHodomoeEventTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  u8 entry_table[WBT_ROM_TRAINER_NUM];

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  makeEntryTable( cond, entry_table );
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_SPECIAL, 1 ); //アクロマ
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_CHAMP, 1 );   //チェレン
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, 1 );  //ライバル
  selectCharacters( entry_table, &rand_context, WBT_TRPRI_WEAK_MOB, 4 );    //モブ

  //選択された結果を戻り値テーブルに反映する
  setEntries( tr_list, entry_table );
}

//-----------------------------------------------------------------------------
/**
 * @brief タイプトーナメント出場者選出
 */
//-----------------------------------------------------------------------------
static void entryTypeTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  int i;
  u8 entry_table[WBT_ROM_TRAINER_NUM];
  int candidate_count = 0;
  int decide_count = 0;
  int rest;
  u32 leader_num;
  u32 mob_num;

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  makeEntryTable( cond, entry_table );

  //リーダークラスの参加者を抽選する
  leader_num = countEntryWithPri( entry_table, ENTRY_CANDIDATE, WBT_TRPRI_LEADER );
  rest = selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, leader_num );
  TAMADA_Printf("SELECT LEADER:%d,rest=%d\n",leader_num, rest );

  //他の参加者を抽選する
  mob_num = WBT_TRAINER_NUM - 1 - leader_num;
  rest = selectCharacters( entry_table, &rand_context, WBT_TRPRI_NULL, mob_num );
  TAMADA_Printf("SELECT MOB:%d,rest=%d\n",mob_num, rest );

  candidate_count = countEntry( entry_table, ENTRY_CANDIDATE );
  GF_ASSERT( candidate_count == 0 );

  //選択された結果を戻り値テーブルに反映する
  setEntries( tr_list, entry_table );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WBTTOOL_MakeEntryList( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  switch (cond->wbtcup)
  {
  default:
  case WBTCUP_NULL:
  case WBTCUP_DOWNLOAD:
    GF_ASSERT_MSG( 0, "仮：まだできていません！\n");
    /* FALL THROUGH */

  case WBTCUP_HODOMOE:
    entryHodomoeTournament( cond, tr_list );
    break;
  case WBTCUP_CHAMPION:
    entryChampTournament( cond, tr_list );
    break;
  case WBTCUP_POKETYPE:
    entryTypeTournament( cond, tr_list );
    break;
  case WBTCUP_ISSYU:
  case WBTCUP_KANTO:
  case WBTCUP_JOUTO:
  case WBTCUP_HOUEN:
  case WBTCUP_SINOU:
    entryRegionTournament( cond, tr_list );
    break;
  case WBTCUP_WORLD:
    entryWorldTournament( cond, tr_list );
    break;
  case WBTCUP_WCS:
    entryWCSTournament( cond, tr_list );
    break;
  case WBTCUP_HODOMOE_EVENT:
    entryHodomoeEventTournament( cond, tr_list );
    break;
  }
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/// swap
//-----------------------------------------------------------------------------
static inline void swap_trainer( const WBTTRAINER ** array, u32 pos1, u32 pos2 )
{
  const WBTTRAINER * temp;
  GF_ASSERT( pos1 < WBT_TRAINER_NUM );
  GF_ASSERT( pos2 < WBT_TRAINER_NUM );
  temp = array[pos1];
  array[pos1] = array[pos2];
  array[pos2] = temp;
}

//-----------------------------------------------------------------------------
/// shuffle (Fisher-Yates法)
//-----------------------------------------------------------------------------
static void shuffle_trainer( GFL_STD_RandContext * rand_context, const WBTTRAINER ** array, int len )
{
  int i = len;
  while ( i )
  {
    int rand_pos = GFL_STD_Rand( rand_context, i );
    i --;
    swap_trainer( array, i, rand_pos );
  }
}

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
void WBTTOOL_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64, HEAPID heapID )
{
  enum {
    NUM_WITHOUT_PLAYER = WBT_TRAINER_NUM - 1
  };
  int i, j;
  int player_pos;
  const WBTTRAINER * sort_by_pri[WBT_TRAINER_NUM];
  const WBTTRAINER * sort_result[WBT_TRAINER_NUM];

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, seed64 );

  GFL_STD_MemClear( sort_by_pri, sizeof(sort_by_pri) );
  GFL_STD_MemClear( sort_result, sizeof(sort_result) );

  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    sort_by_pri[i] = &wbt_tr_array[i];
  }

  //まぜる
  //「①プレイヤーの場所をランダムに決定」も兼ねる
  shuffle_trainer( &rand_context, sort_by_pri, WBT_TRAINER_NUM );

  //プレイヤーを抜き出す
  for ( player_pos = -1, i = 0; i < WBT_TRAINER_NUM; i++ )
  {
    if (WBTTRAINER_GetWBTTrainerType(sort_by_pri[i]) == WBT_TRTYPE_PLAYER)
    {
      sort_result[i] = sort_by_pri[i];
      player_pos = i;
      for ( j = i; j < WBT_TRAINER_NUM - 1; j ++ )
      {
        sort_by_pri[j] = sort_by_pri[j + 1];
      }
      break;
    }
  }
  GF_ASSERT( player_pos >= 0 );
  TAMADA_Printf("WBTSORT: player_pos = %d\n", player_pos );
  sort_by_pri[ WBT_TRAINER_NUM - 1 ] = NULL;

  //優先度でソートをする
  for ( i = 0; i < NUM_WITHOUT_PLAYER - 1; i++ )
  {
    for ( j = i + 1; j < NUM_WITHOUT_PLAYER; j ++ )
    {
      u8 pri1 = WBTTRAINER_GetPriority( sort_by_pri[i] );
      u8 pri2 = WBTTRAINER_GetPriority( sort_by_pri[j] );
      if ( pri1 < pri2 )
      {
        swap_trainer(sort_by_pri, i, j );
      }
    }
  }
#ifdef PM_DEBUG
  for ( i = 0; i < NUM_WITHOUT_PLAYER; i ++ )
  {
    WBTTRAINER_DEBUG_dump( sort_by_pri[ i ] );
  }
#endif


  {//②優先順位の最も高いキャラをプレイヤーと逆側のブロックに配置
    static const u8 pri1_pos_table[ WBT_TRAINER_NUM ] = { 4,4, 4,4, 0,0, 0,0 };
    u8 pri1_pos = pri1_pos_table[ player_pos ] + GFL_STD_Rand( &rand_context, 4 );
    sort_result[ pri1_pos  ] = sort_by_pri[0];
  }
  {//③次に優先順位が高いキャラがプレイヤー側２回戦であたる場所に配置
    static const u8 pri2_pos_table[ WBT_TRAINER_NUM ] = { 2,2, 0,0, 6,6, 4,4 };
    u8 pri2_pos = pri2_pos_table[ player_pos ] + GFL_STD_Rand( &rand_context, 2 );
    sort_result[ pri2_pos  ] = sort_by_pri[1];
  }
  {//④次に優先順位が高いキャラがプレイヤーとあたるように配置
    static const u8 pri3_pos_table[ WBT_TRAINER_NUM ] = { 1,0, 3,2, 5,4, 7,6 }; 
    u8 pri3_pos = pri3_pos_table[ player_pos ];
    sort_result[ pri3_pos  ] = sort_by_pri[2];
  }

  //⑤他はランダムで配置
  //残りをあらためて混ぜる
  shuffle_trainer( &rand_context, &sort_by_pri[3], NUM_WITHOUT_PLAYER - 3 );

  { //空きを探して埋めていく
    int tr_pos, seek_pos;
    for ( tr_pos = 3, seek_pos = 0; seek_pos < WBT_TRAINER_NUM; seek_pos ++ )
    {
      if ( sort_result[seek_pos] == NULL )
      {
        sort_result[seek_pos] = sort_by_pri[tr_pos];
        tr_pos ++;
        if ( tr_pos >= NUM_WITHOUT_PLAYER )
        {
          break;
        }
      }
    }
    GF_ASSERT( seek_pos < WBT_TRAINER_NUM );
    GF_ASSERT( tr_pos >= NUM_WITHOUT_PLAYER );
  }
  { //ソート結果を受け取ったWBTトレーナーデータに書き戻す
    WBTTRAINER * copy_buf;
    copy_buf = GFL_HEAP_AllocMemoryLo( heapID, sizeof(WBTTRAINER) * WBT_TRAINER_NUM );

    for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
    {
      copy_buf[i] = *(sort_result[i]);
    }
    GFL_STD_MemCopy32( copy_buf, wbt_tr_array, sizeof(WBTTRAINER) * WBT_TRAINER_NUM );

    GFL_HEAP_FreeMemory( copy_buf );
  }
}

//============================================================================================
//============================================================================================

#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  //SYS_FLAG_GAMECLEAR

//-----------------------------------------------------------------------------
/**
 * @brief WBTの様々なイベント判定を取得する
 * @param gamedata
 * @param cond_id   wbt_tool.hのWBT_COND_IDを参照
 * @return  BOOL    結果を
 */
//-----------------------------------------------------------------------------
BOOL WBTTOOL_GetStatus( GAMEDATA * gamedata, WBT_COND_ID cond_id )
{
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  EVENTWORK * evwork = GAMEDATA_GetEventWork( gamedata );

  switch ( cond_id )
  {
  case WBT_COND_ID_ENABLE_DOWNLOAD_TOURNAMENT:
  default:
    GF_ASSERT( 0 );
  }
  return FALSE;
}





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















