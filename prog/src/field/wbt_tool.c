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

#define ROM_201104ALPHA

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
  typedef enum {
    ENTRY_NON = 0,
    ENTRY_CANDIDATE,
    ENTRY_DECIDE,
  }ENTRY_CODE;
  enum {
    CHAMP_ENTRY_RATE = 10,  ///<チャンピオンがエントリーする確率
  };
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static u32 setupEntryTable( const WBT_MATCH_CONDITION * cond, u8 * entry_table )
{
  int candidate_count = 0;
  int i;
  GFL_STD_MemFill( entry_table, ENTRY_NON, WBT_TRCODE_NUM );

  //全体から出場者を抽出
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
  {
    const WBT_TRDATA * rom_data = &WBTTrainerData[ i ];
    if ( rom_data->entry_timing <= cond->level ) continue;
    entry_table[ i ] = ENTRY_CANDIDATE;
    candidate_count ++;
  }
  return candidate_count; //候補者数
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static u32 countEntry( const u8 * entry_table, ENTRY_CODE entryCode )
{
  u32 count = 0;
  int i;
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
  {
    if ( entry_table[i] == entryCode ) count ++;
  }
  return count;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static u32 countEntryWithPri( const u8 * entry_table, ENTRY_CODE entryCode, u8 pri )
{
  u32 candidate_count = 0;
  int i;
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
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
//-----------------------------------------------------------------------------
static u32 selectCandidateByPri( const u8 * entry_table, u8 pri, u8 step )
{
  u32 pos_count = step;
  u32 i = 0;
  GF_ASSERT( step > 0 )
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
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
  GF_ASSERT( i < WBT_TRCODE_NUM );
  return i;
}

//-----------------------------------------------------------------------------
/**
 * @param entry_table
 * @param rand_context
 * @param pri
 * @param num
 */
//-----------------------------------------------------------------------------
static u32 selectCharacters( u8 * entry_table, GFL_STD_RandContext * rand_context, u8 pri, u8 num )
{
  int i;
  int count = num;
  u32 candidate_count = countEntryWithPri( entry_table, ENTRY_CANDIDATE, pri );

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
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
  {
    if ( entry_table[i] == ENTRY_CANDIDATE )
    {
      if ( pri == WBT_TRPRI_NULL ||WBTTrainerData[i].priority == pri )
      {
        entry_table[i] = ENTRY_NON;
      }
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
//チャンピオンの登録処理（確率によって一人だけ参加することがある）
//-----------------------------------------------------------------------------
static BOOL selectChamp( u8 * entry_table, GFL_STD_RandContext * rand_context )
{
  int i;
  int keep_id = -1;
  if ( GFL_STD_Rand( rand_context, 100 ) < CHAMP_ENTRY_RATE )
  {
    keep_id = GFL_STD_Rand( rand_context, WBT_TRCODE_NUM_CHAMP );
    entry_table[ WBT_TRCODE_CHAMP_START + keep_id ] = ENTRY_DECIDE;
  }
  for ( i = 0; i < WBT_TRCODE_NUM_CHAMP; i++ )
  {
    if ( keep_id != i )
    {
      entry_table[ WBT_TRCODE_CHAMP_START + i ] = ENTRY_NON;
    }
  }
  return ( keep_id != -1 );
}

//-----------------------------------------------------------------------------
/**
 * @brief オールトーナメント出場者選出
 */
//-----------------------------------------------------------------------------
static void entryAllTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  int i;
  u8 entry_table[WBT_TRCODE_NUM];
  int candidate_count = 0;
  int decide_count = 0;
  int rest;
  u32 leader_num  = cond->leader_num;
  u32 mob_num     = WBT_TRAINER_NUM - 1 - leader_num;

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  candidate_count = setupEntryTable( cond, entry_table );

#ifdef  ROM_201104ALPHA
  //αバージョン用：/カントー、ジョウト、ホウエンのリーダーを無理やりエントリー
  leader_num = 7;
  mob_num = 0;
  GFL_STD_MemFill( entry_table, ENTRY_NON, WBT_TRCODE_NUM );
  for ( i = WBT_TRCODE_OTHER_LEADER_START; i < WBT_TRCODE_OTHER_LEADER_START + 16+8+1; i ++ )
  {
    entry_table[i] = ENTRY_CANDIDATE;
  }
  candidate_count = countEntry( entry_table, ENTRY_CANDIDATE );
#else
  TAMADA_Printf("TOTAL CANDIDATES=%d\n", candidate_count );
  //全体からオールトーナメント出場者以外を排除
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
  {
    const WBT_TRDATA * rom_data = &WBTTrainerData[ i ];
    if ( rom_data->entry_all ) continue;
    entry_table[ i ] = ENTRY_NON;
  }

  //チャンピオンの登録処理（確率によって一人だけ参加することがある）
  if (cond->champ_entry && selectChamp( entry_table, &rand_context ) == TRUE )
  {
    decide_count ++;
    leader_num --;
  }
#endif
  //リーダークラスの参加者を抽選する
  rest = selectCharacters( entry_table, &rand_context, WBT_TRPRI_LEADER, leader_num );
  TAMADA_Printf("SELECT LEADER:%d,rest=%d\n",leader_num, rest );

  //他の参加者を抽選する
  rest = selectCharacters( entry_table, &rand_context, WBT_TRPRI_NULL, mob_num );
  TAMADA_Printf("SELECT MOB:%d,rest=%d\n",mob_num, rest );

  candidate_count = countEntry( entry_table, ENTRY_CANDIDATE );
  GF_ASSERT( candidate_count == 0 );

  //選択された結果を戻り値テーブルに反映する
  {
    u32 tr_pos = 0;
    tr_list[tr_pos ++ ] = WBT_TRCODE_PLAYER;  //プレイヤーは登録する
    for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
    {
      if ( entry_table[i] == ENTRY_DECIDE )
      {
        tr_list[ tr_pos ] = i;
        tr_pos ++;
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
#if 1
  GFL_STD_MemCopy( presentation_trid, tr_list, sizeof(u16) * WBT_TRAINER_NUM );
#else
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
}

//-----------------------------------------------------------------------------
/**
 * @brief タイプトーナメント出場者選出
 */
//-----------------------------------------------------------------------------
static void entryTypeTournament( const WBT_MATCH_CONDITION * cond, u16 * tr_list )
{
  int i;
  u8 entry_table[WBT_TRCODE_NUM];
  int candidate_count = 0;
  int decide_count = 0;
  int rest;
  u32 leader_num;
  u32 mob_num;

  GFL_STD_RandContext rand_context;
  GFL_STD_RandInit( &rand_context, cond->seed64 );

  GFL_STD_MemFill( entry_table, ENTRY_CANDIDATE, WBT_TRCODE_NUM );
  candidate_count = countEntry( entry_table, ENTRY_CANDIDATE );

  TAMADA_Printf("TOTAL CANDIDATES=%d\n", candidate_count );
  //全体からトーナメント出場者以外を排除
  for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
  {
    const WBT_TRDATA * rom_data = &WBTTrainerData[ i ];
    //タイプ一致、あるいはオールマイティの場合は参戦する
    if ( rom_data->type_tournament_id == cond->poketype
        || rom_data->type_tournament_id == POKETYPE_NULL )
    {
      TAMADA_Printf("TYPE TOURNAMENT CANDIDATE %3d (poketype=%2d)\n",
          i, rom_data->type_tournament_id );
      continue;
    }
    entry_table[ i ] = ENTRY_NON;
  }

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
  {
    u32 tr_pos = 0;
    tr_list[tr_pos ++ ] = WBT_TRCODE_PLAYER;  //プレイヤーは登録する
    for ( i = 0; i < WBT_TRCODE_NUM; i ++ )
    {
      if ( entry_table[i] == ENTRY_DECIDE )
      {
        tr_list[ tr_pos ] = i;
        tr_pos ++;
      }
    }
  }

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
  case WBTTYPE_ALL:
    entryAllTournament( cond, tr_list );
    break;
  case WBTTYPE_CHAMPION:
    entryChampTournament( tr_list );
    break;
  case WBTTYPE_POKETYPE:
    entryTypeTournament( cond, tr_list );
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
  sort_by_pri[ WBT_TRAINER_NUM - 1 ] = NULL;

  //優先度でソートをする
  for ( i = 0; i < NUM_WITHOUT_PLAYER - 1; i++ )
  {
    for ( j = i + 1; j < NUM_WITHOUT_PLAYER - 1; j ++ )
    {
      u8 pri1 = WBTTRAINER_GetPriority( sort_by_pri[i] );
      u8 pri2 = WBTTRAINER_GetPriority( sort_by_pri[j] );
      if ( pri1 < pri2 )
      {
        swap_trainer(sort_by_pri, i, j );
      }
    }
  }

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
  enum {
    CHAMP_OPEN_COUNT = 10,  ///<仮：オールトーナメント１０回クリアでチャンプトーナメント解除
  };

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
  case WBT_COND_ID_ENABLE_POKETYPE_TOURNAMENT:
    //ゲームクリアしていたらタイプトーナメントは開催する
    return EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR );
    break;
  case WBT_COND_ID_ENABLE_CHAMPION_TOURNAMENT:
    //オールトーナメントを規定回数以上優勝していたらチャンピオントーナメントは開催する
    return (WBTSAVE_GetWinCount( wbtsave, WBTWIN_COUNT_ALLTOURNAMENT) >= CHAMP_OPEN_COUNT);
    break;
  case WBT_COND_ID_ENABLE_DOWNLOAD_TOURNAMENT:
    return FALSE;  //仮：ダウンロードデータには未対応
    break;
  case WBT_COND_ID_ENTRY_CHAMP_IN_ALL:
    //ゲームクリアしていたらチャンピオンはオールトーナメントに参戦できる
    return EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR );
  default:
    GF_ASSERT( 0 );
  }
  return FALSE;
}


//-----------------------------------------------------------------------------
/**
 * @brief リーダーの参戦比率を取得する
 * @param gamedata
 * @param context
 * @return リーダーの人数（自分を除く7人のうち占める数）
 */
//-----------------------------------------------------------------------------
u32 WBTTOOL_GetAllTournamentLeaderRate( GAMEDATA * gamedata, GFL_STD_RandContext * context )
{
  EVENTWORK * evwork = GAMEDATA_GetEventWork( gamedata );
  u32 badge_count = MISC_GetBadgeCount( GAMEDATA_GetMiscWork( gamedata ) );


  if (EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR ) == TRUE )
  {
    return GFL_STD_Rand( context, 3 ) + 5;
  }
  else if ( badge_count >= 7 )
  {
    return 5;
  }
  else if ( badge_count >= 5 )
  {
    return 4;
  }
  return 3;
}

//-----------------------------------------------------------------------------
/**
 * @brief オールトーナメントでのトレーナー参戦レベルを取得する
 * @param gamedata
 * @return  WBT_ALLTOUR_LEVEL
 */
//-----------------------------------------------------------------------------
WBT_ALLTOUR_LEVEL WBTTOOL_GetAllTournamentEntryLevel( GAMEDATA * gamedata )
{
  EVENTWORK * evwork = GAMEDATA_GetEventWork( gamedata );
  u32 badge_count = MISC_GetBadgeCount( GAMEDATA_GetMiscWork( gamedata ) );

  if (EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_GAME_CLEAR ) == TRUE )
  {
    return WBT_ALLTOUR_LEVEL3;
  }
  else if ( badge_count >= 8 )
  {
    return WBT_ALLTOUR_LEVEL2;
  }
  else if ( badge_count >= 6 )
  {
    return WBT_ALLTOUR_LEVEL1;
  }
  return WBT_ALLTOUR_LEVEL0;
}

























