//============================================================================================
/**
 * @file  wbt_makematch.c
 * @brief   WBT用ツール：参戦トレーナーの抽選処理
 * @date  2011.07.28
 * @author  tamada@gamefreak.co.jp
 *
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"

#include "wbt_tool.h"
#include "wbt_trlist_struct.h"  //WBT_TRDATA

#include "wbt_system_local.h"

//#include "system/gfl_use.h" //GFUser_GetPublicRand0

#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h"
#include "sound/pm_sndsys.h"



//============================================================================================
//============================================================================================

//------------------------------------------------------------------
/// エントリー状態
//------------------------------------------------------------------
typedef enum {
  ENTRY_NON = 0,    ///<エントリーされていない
  ENTRY_CANDIDATE,  ///<出場候補
  ENTRY_DECIDE,     ///<出場決定
}ENTRY_CODE;

typedef enum {
  BTLCOND_BEFORE,
  BTLCOND_AFTER,
  BTLCOND_NON,
}BTLCOND;
//------------------------------------------------------------------
/// エントリー抽選用ワーク
//------------------------------------------------------------------
typedef struct {
  const WBT_MATCH_CONDITION * cond;

  GFL_STD_RandContext rand_context;
  u8 * entry_table; ///<候補者の選別状態（ENTRY_CODE）
  u8 * battle_flag; ///<対戦済みかどうか（BTLCOND)
}MAKE_ENTRY_WORK;


//============================================================================================
//
//
//  トーナメント出場者選出
//
//
//============================================================================================
static void makeEntryTable( MAKE_ENTRY_WORK * mew, const WBTSAVE * wbtsave );
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static MAKE_ENTRY_WORK * mew_Create(
    HEAPID heapID, const WBTSAVE * wbtsave, const WBT_MATCH_CONDITION * cond )
{
  MAKE_ENTRY_WORK * mew = GFL_HEAP_AllocMemoryLo( heapID, sizeof(MAKE_ENTRY_WORK) );
  mew->cond = cond;

  mew->entry_table = GFL_HEAP_AllocMemoryLo( heapID, sizeof(u8) * cond->rom_trainer_num );
  mew->battle_flag = GFL_HEAP_AllocMemoryLo( heapID, sizeof(u8) * cond->rom_trainer_num );
  makeEntryTable( mew, wbtsave );
  GFL_STD_RandInit( &mew->rand_context, cond->seed64 );

  return mew;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void mew_Delete( MAKE_ENTRY_WORK * mew )
{
  GFL_HEAP_FreeMemory( mew->entry_table );
  GFL_HEAP_FreeMemory( mew->battle_flag );
  GFL_HEAP_FreeMemory( mew );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void mew_SetEntryResult( const MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  //選択された結果を戻り値テーブルに反映する
  u32 tr_pos = 0;
  u32 i;
  tr_list[tr_pos ++ ].code = WBT_TRCODE_PLAYER;  //プレイヤーは登録する
  for ( i = 0; i < mew->cond->rom_trainer_num; i ++ )
  {
    if ( mew->entry_table[i] == ENTRY_DECIDE )
    {
      if ( tr_pos < WBT_TRAINER_NUM )
      {
        tr_list[ tr_pos ].type = WBT_TRCODE_TYPE_ROM;
        tr_list[ tr_pos ].tr_id = i;
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
/// 指定したENTRY_CODEとPRIのものを数える
//-----------------------------------------------------------------------------
static u32 mew_CountEntry( const MAKE_ENTRY_WORK * mew, ENTRY_CODE entryCode, u8 pri, BTLCOND bcond )
{
  u32 candidate_count = 0;
  int i;
  for ( i = 0; i < mew->cond->rom_trainer_num; i ++ )
  {
    if (mew->entry_table[i] != entryCode) continue;
    if ( pri != WBT_TRPRI_NULL && mew->cond->rom_trainer_data[i].priority != pri) continue;
    if ( bcond != BTLCOND_NON && bcond != mew->battle_flag[i] ) continue;
    candidate_count ++;
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
static u32 selectCandidate( const MAKE_ENTRY_WORK * mew, u8 pri, BTLCOND bcond, u8 step )
{
  u32 pos_count = step;
  u32 i = 0;
  GF_ASSERT( step > 0 )
  for ( i = 0; i < mew->cond->rom_trainer_num; i ++ )
  {
    if (mew->entry_table[i] != ENTRY_CANDIDATE) continue;
    if ( pri != WBT_TRPRI_NULL && mew->cond->rom_trainer_data[i].priority != pri) continue;
    if ( bcond != BTLCOND_NON && bcond != mew->battle_flag[i] ) continue;

    pos_count --;
    if ( pos_count == 0 )
    {
      return i;
    }
  }
  GF_ASSERT( i < mew->cond->rom_trainer_num );
  return i;
}

//-----------------------------------------------------------------------------
/**
 * @param mew     抽選処理ワーク
 * @param pri     選択する候補者の優先度（WBT_TRPRI_NULLのとき、無条件）
 * @param num     選択する人数
 * @return  u32   選択人数に足りなかった数
 */
//-----------------------------------------------------------------------------
static u32 mew_SelectPerson( MAKE_ENTRY_WORK * mew, u8 pri, u8 num )
{
 // entry_table   検索状態保持テーブル
  u8 * entry_table = mew->entry_table;
 // rand_context  使用する乱数コンテキスト
  GFL_STD_RandContext * rand_context = &mew->rand_context;
  int i;
  int count = num;      //残り選択人数
  u32 candidate_count;  //候補者数
  u32 unknown_count;    //候補者中の戦ったことがない人の数
  
  candidate_count = mew_CountEntry( mew, ENTRY_CANDIDATE, pri, BTLCOND_NON );
  unknown_count = mew_CountEntry( mew, ENTRY_CANDIDATE, pri, BTLCOND_BEFORE );

  //候補数以上に選ぶ必要がある場合はAssert
  GF_ASSERT( count <= candidate_count );
  while ( count )
  {
    u32 select;
    if ( candidate_count == 1 )
    { //場所はランダムで決めるまでもない
      select = selectCandidate( mew, pri, BTLCOND_NON, 1 );
    }
    else
    {
      u32 step;
      if ( unknown_count == 0 )
      {//優先する人はいない：何人目を選択するかはランダムで
        step = GFL_STD_Rand( rand_context, candidate_count ) + 1;
        select = selectCandidate( mew, pri, BTLCOND_NON, step );
      }
      else if ( unknown_count == 1 )
      {//優先する人が１人：場所はランダムで決めるまでもない
        select = selectCandidate( mew, pri, BTLCOND_BEFORE, 1 );
      }
      else
      {//優先する人が複数：何人目を選択するかはランダムで
        step = GFL_STD_Rand( rand_context, unknown_count ) + 1;
        select = selectCandidate( mew, pri, BTLCOND_BEFORE, step );
      }
    }
    GF_ASSERT( entry_table[select] == ENTRY_CANDIDATE );
    entry_table[select] = ENTRY_DECIDE;
    TAMADA_Printf("SELECT TRAINER(%03d):pri(%d)btl(%d)candidate(%2d)\n", select, pri, mew->battle_flag[select], candidate_count );
    candidate_count --;
    unknown_count = mew_CountEntry( mew, ENTRY_CANDIDATE, pri, BTLCOND_BEFORE );
    count --;
    if ( candidate_count == 0 && count ) return count;
  }


  //選から漏れた候補者を消す
  for ( i = 0; i < mew->cond->rom_trainer_num; i ++ )
  {
    if ( entry_table[i] == ENTRY_CANDIDATE )
    {
      if ( pri == WBT_TRPRI_NULL || mew->cond->rom_trainer_data[i].priority == pri )
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
    GF_ASSERT( 0 );
    break;
  case WBTCUP_MIX:
    return rom_data->entry_mix;
  case WBTCUP_MIXMASTER:
    return rom_data->entry_mixmaster;
  case WBTCUP_RENTAL:
    return rom_data->entry_rental;
  case WBTCUP_RENTALMASTER:
    return rom_data->entry_rentalmaster;

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
  case WBTCUP_WORLD:
    return rom_data->entry_world;
  case WBTCUP_DOWNLOAD:
    return TRUE;  //仮：とりいそぎ、ROM上のトレーナーは全員参戦可能にする
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void makeEntryTable( MAKE_ENTRY_WORK * mew, const WBTSAVE * wbtsave )
{
  int i;
  GFL_STD_MemFill( mew->entry_table, ENTRY_NON, mew->cond->rom_trainer_num );
  GFL_STD_MemFill( mew->battle_flag, BTLCOND_AFTER, mew->cond->rom_trainer_num );
  for ( i = 0; i < mew->cond->rom_trainer_num; i ++ )
  {
    const WBT_TRDATA * rom_data = &mew->cond->rom_trainer_data[ i ];
    if ( checkEnableEntryRomTrainer( mew->cond, rom_data ) == TRUE )
    {
      mew->entry_table[ i ] = ENTRY_CANDIDATE;
      if ( i < WBTSAVE_FLAG_MAX && WBTSAVE_GetFlag( wbtsave, i ) == FALSE )
      {
        mew->battle_flag[ i ] = BTLCOND_BEFORE;
      }
    }
  }
  TAMADA_Printf("CANDIDATES:%d\n", mew_CountEntry( mew, ENTRY_CANDIDATE, WBT_TRPRI_NULL, BTLCOND_NON ) );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：ダウンロードトーナメント
 */
//-----------------------------------------------------------------------------
static void entryDownloadTournament(
    const WBTDL_MATCH * wbtdl_match, MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  int index = 0;

  GFL_STD_MemClear( tr_list, sizeof(WBT_TRCODE) * WBT_TRAINER_NUM );
  tr_list[ index ++ ].code = WBT_TRCODE_PLAYER;
  for ( ; index < WBT_TRAINER_NUM; index ++ )
  {
    int rom_index = index - 1;
    const WBTDL_MATCH * match = &wbtdl_match[ rom_index ];
    OS_Printf( "WBTDL MATCH type=%d,pri=%d,id=%d\n",match->type, match->pri, match->id );
    switch ( match->type )
    {
    default:
      GF_ASSERT( 0 );
      /* FALL THROUGH */
    case WBTDL_MATCH_TRAINER_RND:
    case WBTDL_MATCH_TRAINER_RND_DMY:
      //戦わないモブがあたるはず
      tr_list[ index ].type = WBT_TRCODE_TYPE_ROM;
      tr_list[ index ].tr_id = WBTROM_TRID_NOBATTLE_MOB_START + GFUser_GetPublicRand0( 10 );
      break;
    case WBTDL_MATCH_TRAINER_ROM:
      tr_list[ index ].type = WBT_TRCODE_TYPE_ROM;
      tr_list[ index ].tr_id = match->id;
      break;
    case WBTDL_MATCH_TRAINER_DATA:
      tr_list[ index ].type = WBT_TRCODE_TYPE_DL1 + rom_index;
      tr_list[ index ].tr_id = match->id;
      break;
    }
  }
}
//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：チャンピオントーナメント
 */
//-----------------------------------------------------------------------------
static void entryChampTournament(MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  int tr_idx, j;

  GFL_STD_MemClear( tr_list, sizeof(WBT_TRCODE) * WBT_TRAINER_NUM );
  tr_list[0].code = WBT_TRCODE_PLAYER;
  tr_list[1].code = WBT_TRCODE_WBHERO;

  for ( j = 0, tr_idx = 2; j < mew->cond->rom_trainer_num; j ++ )
  {
    const WBT_TRDATA * rom_data = &mew->cond->rom_trainer_data[ j ];
    if ( rom_data->region_tournament_id != WBTCUP_CHAMPION ) continue;
    tr_list[tr_idx].type = WBT_TRCODE_TYPE_ROM;
    tr_list[tr_idx].tr_id = j;
    tr_idx ++;
    if ( tr_idx >= WBT_TRAINER_NUM )
    {
      break;
    }
  }
  GF_ASSERT( j < mew->cond->rom_trainer_num );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：ワールドトーナメント
 */
//-----------------------------------------------------------------------------
static void entryWorldTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 7 );

  mew_SetEntryResult( mew, tr_list );
}
//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：リーダーズトーナメント
 */
//-----------------------------------------------------------------------------
static void entryRegionTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 7 );

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：ホドモエトーナメント
 */
//-----------------------------------------------------------------------------
static void entryHodomoeTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 2 );
  mew_SelectPerson( mew, WBT_TRPRI_MOB, 1 );
  mew_SelectPerson( mew, WBT_TRPRI_WEAK_MOB, 4 );

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：ホドモエトーナメント（イベント）
 */
//-----------------------------------------------------------------------------
static void entryHodomoeEventTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_SPECIAL, 1 ); //アクロマ
  mew_SelectPerson( mew, WBT_TRPRI_CHAMP, 1 );   //チェレン
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 1 );  //ライバル
  mew_SelectPerson( mew, WBT_TRPRI_WEAK_MOB, 4 );    //モブ

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：レンタルトーナメント
 */
//-----------------------------------------------------------------------------
static void entryRentalTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 2 );
  mew_SelectPerson( mew, WBT_TRPRI_MOB, 2 );
  mew_SelectPerson( mew, WBT_TRPRI_WEAK_MOB, 3 );

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：レンタルトーナメント
 */
//-----------------------------------------------------------------------------
static void entryMixTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 2 );
  mew_SelectPerson( mew, WBT_TRPRI_MOB, 2 );
  mew_SelectPerson( mew, WBT_TRPRI_WEAK_MOB, 3 );

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：レンタルトーナメント
 */
//-----------------------------------------------------------------------------
static void entryRentalMasterTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 7 );

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：レンタルトーナメント
 */
//-----------------------------------------------------------------------------
static void entryMixMasterTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  mew_SelectPerson( mew, WBT_TRPRI_LEADER, 7 );

  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief 参戦トレーナー選別：タイプトーナメント
 */
//-----------------------------------------------------------------------------
static void entryTypeTournament( MAKE_ENTRY_WORK * mew, WBT_TRCODE * tr_list )
{
  int i;
  int candidate_count = 0;
  int rest;
  u32 leader_num, mob_num, weak_mob_num;

  //リーダークラスの参加者を抽選する
  leader_num = mew_CountEntry( mew, ENTRY_CANDIDATE, WBT_TRPRI_LEADER, BTLCOND_NON );
  rest = mew_SelectPerson( mew, WBT_TRPRI_LEADER, leader_num );
  TAMADA_Printf("SELECT LEADER:%d,rest=%d\n", leader_num, rest );
  TAMADA_Printf(" REST CANDIDATE:%d\n", mew_CountEntry( mew, ENTRY_CANDIDATE, WBT_TRPRI_NULL, BTLCOND_NON ) );

  //他の参加者を抽選する
  mob_num = mew_CountEntry( mew, ENTRY_CANDIDATE, WBT_TRPRI_MOB, BTLCOND_NON );
  rest = mew_SelectPerson( mew, WBT_TRPRI_MOB, mob_num );
  TAMADA_Printf("SELECT MOB:%d,rest=%d\n", mob_num, rest );
  TAMADA_Printf(" REST CANDIDATE:%d\n", mew_CountEntry( mew, ENTRY_CANDIDATE, WBT_TRPRI_NULL, BTLCOND_NON ) );
  weak_mob_num = WBT_TRAINER_NUM - 1 - leader_num - mob_num;
  rest = mew_SelectPerson( mew, WBT_TRPRI_WEAK_MOB, weak_mob_num );
  TAMADA_Printf("SELECT WEAK MOB:%d,rest=%d\n", weak_mob_num, rest );
  TAMADA_Printf(" REST CANDIDATE:%d\n", mew_CountEntry( mew, ENTRY_CANDIDATE, WBT_TRPRI_NULL, BTLCOND_NON ) );

  GF_ASSERT( 1 + leader_num + mob_num + weak_mob_num == WBT_TRAINER_NUM );

  //選択された結果を戻り値テーブルに反映する
  mew_SetEntryResult( mew, tr_list );
}

//-----------------------------------------------------------------------------
/**
 * @brief
 * @param[in]   cond
 * @param[out]  tr_list
 */
//-----------------------------------------------------------------------------
void WBTSYS_MakeEntryList( GAMEDATA * gamedata, const WBT_MATCH_CONDITION * cond, WBT_TRCODE * tr_list )
{
  const WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  MAKE_ENTRY_WORK * mew = mew_Create( cond->heapID, wbtsave, cond );

  switch (cond->wbtcup)
  {
  default:
  case WBTCUP_NULL:
    GF_ASSERT_MSG( 0, "仮：まだできていません！\n");
    /* FALL THROUGH */

  case WBTCUP_HODOMOE:
    entryHodomoeTournament( mew, tr_list );
    break;
  case WBTCUP_CHAMPION:
    entryChampTournament( mew, tr_list );
    break;
  case WBTCUP_POKETYPE:
    entryTypeTournament( mew, tr_list );
    break;
  case WBTCUP_ISSYU:
  case WBTCUP_KANTO:
  case WBTCUP_JOUTO:
  case WBTCUP_HOUEN:
  case WBTCUP_SINOU:
    entryRegionTournament( mew, tr_list );
    break;
  case WBTCUP_WORLD:
    entryWorldTournament( mew, tr_list );
    break;
  case WBTCUP_HODOMOE_EVENT:
    entryHodomoeEventTournament( mew, tr_list );
    break;
  case WBTCUP_MIX:
    entryMixTournament( mew, tr_list );
    break;
  case WBTCUP_MIXMASTER:
    entryMixMasterTournament( mew, tr_list );
    break;
  case WBTCUP_RENTAL:
    entryRentalTournament( mew, tr_list );
    break;
  case WBTCUP_RENTALMASTER:
    entryRentalMasterTournament( mew, tr_list );
    break;
  case WBTCUP_DOWNLOAD:
    entryDownloadTournament( cond->wbtdl_match, mew, tr_list );
    break;
  }

  mew_Delete( mew );
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
void WBTSYS_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64, HEAPID heapID )
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

