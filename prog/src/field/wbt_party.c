//============================================================================================
/**
 * @file  wbt_party.c
 * @brief WBT：自分＆相手の手持ちポケモン生成関連
 * @author  tamada GAMEFREAK inc.
 * @date  2011.08.20
 *
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"

#include "system/main.h"

#include "wbt_system_local.h"
#include "wbt_tool.h"


#include "savedata/bsubway_savedata.h"  //bsubway_battle_data.hの内部定義のため
#include "battle/bsubway_battle_data.h" //BSUBWAY_PARTNER_DATA
#include "fld_btl_inst_tool.h"
#include "message.naix"    //NARC_message_wbt_trname_dat
#include "tr_tool/ex_trainer_def.h" //TRID_OFFSET_WBT
#include "savedata/etc_save.h"    //ETC_SAVE_WORK

#include "savedata/save_tbl.h"

#include "savedata/wbt_download.h"

#include "sound/swan_sound_data.sadl"

#include "msg/msg_wbt_system.h"   //msg_wbtsys_oyaname
#include "poke_tool/poke_memo.h"  //POKE_MEMO_


//============================================================================================
//    定義
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  MY_RENTAL_POWER_RND = 20,
};
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  NG_POKE_MAX = 12,
  NG_ITEM_MAX = 12,
  NG_INDEX_MAX = 12,
};

enum {
  NG_INDEX_NULL = 0xffff, ///< u16の最大値を無効値として使用する
};

//--------------------------------------------------------------
/// ポケモン抽選処理でのNGポケモン情報
//--------------------------------------------------------------
struct _WBT_NGPMINFO
{
  BOOL disableMonsCheck;  ///<ポケモンNoのチェック無効
  u16 ng_mons_max;        ///<NGポケモンNo.の最大数
  u16 ng_item_max;        ///<NGアイテムNo.の最大数
  u16 ng_index_max;       ///<NGインデックスの最大数
  u16 * ng_mons_no;       ///<NGポケモンNo.の配列
  u16 * ng_item_no;       ///<NGアイテムNo.の配列
  u16 * ng_index_no;      ///<NGインデックスの配列
};

//--------------------------------------------------------------
/**
 * @brief 候補ポケモンデータ
 */
//--------------------------------------------------------------
typedef struct {
  u8 available;                 ///<BOOL TRUEのとき選択可能
  u16 index_no;                 ///<参照アーカイブ上のインデックス値
  BSUBWAY_POKEMON_ROM_DATA prd; ///<参照アーカイブから読み込んだ実データ
}CANDIDATE_DATA;

//--------------------------------------------------------------
/**
 * @brief 候補全体データ
 */
//--------------------------------------------------------------
typedef struct {
  u16 num;                ///<候補ポケモンの要素数
  CANDIDATE_DATA * array; ///<候補ポケモンの配列
}POKECANDIDATE;

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 GetRand( GFL_STD_RandContext *rand_context )
{
  if( rand_context == NULL )
  {
    return( GFUser_GetPublicRand(0xffffffff)/65535 );
  }
  else
  {
    return( GFL_STD_Rand( rand_context, 0xffffffff )/65535 );
  }
}

//============================================================================================
//
//  登録NG情報関連
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief NG情報オブジェクト生成
 */
//--------------------------------------------------------------
WBT_NGPMINFO * NGINFO_Create( HEAPID heapID, WBTCUP wbtcup )
{
  WBT_NGPMINFO * ng_info;
  ng_info = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBT_NGPMINFO) );
  ng_info->disableMonsCheck = FALSE;
  if ( wbtcup != WBTCUP_RENTAL && WBTTOOL_GetRegulationID( wbtcup ) == WBTREG_ALL )
  { //レンタルを除くなんでもルールの場合はポケモンNo.をチェックしない
    ng_info->disableMonsCheck = TRUE;
  }
  ng_info->ng_mons_max = NG_POKE_MAX;
  ng_info->ng_item_max = NG_ITEM_MAX;
  ng_info->ng_index_max = NG_INDEX_MAX;
  ng_info->ng_mons_no = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16) * NG_POKE_MAX );
  ng_info->ng_item_no = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16) * NG_ITEM_MAX );
  ng_info->ng_index_no = GFL_HEAP_AllocMemory( heapID, sizeof(u16) * NG_INDEX_MAX );
  GFL_STD_MemFill16( ng_info->ng_index_no, NG_INDEX_NULL, sizeof(u16) * NG_INDEX_MAX );
  return ng_info;
}

//--------------------------------------------------------------
/**
 * @brief NG情報オブジェクト削除
 */
//--------------------------------------------------------------
void WBT_NGINFO_Delete( WBT_NGPMINFO * ng_info )
{
  GFL_HEAP_FreeMemory( ng_info->ng_mons_no );
  GFL_HEAP_FreeMemory( ng_info->ng_item_no );
  GFL_HEAP_FreeMemory( ng_info->ng_index_no );
  GFL_HEAP_FreeMemory( ng_info );
}
//--------------------------------------------------------------
/**
 * @brief NG登録：アイテム
 * @param[in/out] ng_info
 * @param[in]     item_no
 */
//--------------------------------------------------------------
void WBT_NGINFO_AddNGItem( WBT_NGPMINFO * ng_info, u16 item_no )
{
  int i;
  for ( i = 0; i < ng_info->ng_item_max; i ++ )
  {
    if (ng_info->ng_item_no[ i ] == 0 )
    {
      ng_info->ng_item_no[ i ] = item_no;
      TAMADA_Printf("NG item(%d)\n", item_no );
      return;
    }
  }
  GF_ASSERT(0);
}
//--------------------------------------------------------------
/**
 * @brief NG登録：ポケモンNo.
 * @param[in/out] ng_info
 * @param[in]     mons_no
 */
//--------------------------------------------------------------
void WBT_NGINFO_AddNGMons( WBT_NGPMINFO * ng_info, u16 mons_no )
{
  int i;
  for ( i = 0; i < ng_info->ng_mons_max; i ++ )
  {
    if (ng_info->ng_mons_no[ i ] == 0 )
    {
      ng_info->ng_mons_no[ i ] = mons_no;
      TAMADA_Printf("NG pokemon(%d)\n", mons_no );
      return;
    }
  }
  GF_ASSERT(0);
}

//--------------------------------------------------------------
/**
 * @brief NG登録：index No.
 * @param[in/out] ng_info
 * @param[in]     ng_index
 */
//--------------------------------------------------------------
void WBT_NGINFO_AddNGIndex( WBT_NGPMINFO * ng_info, s16 ng_index )
{
  int i;
  for ( i = 0; i < ng_info->ng_index_max; i ++ )
  {
    if (ng_info->ng_index_no[ i ] == NG_INDEX_NULL )
    {
      ng_info->ng_index_no[ i ] = ng_index;
      TAMADA_Printf("NG index(%d)\n", ng_index );
      return;
    }
  }
  GF_ASSERT(0);
}

//--------------------------------------------------------------
/**
 * @brief NG情報との照合チェック
 * @param[in]   ng_info
 * @param[in]   mons_no
 * @param[in]   item_no
 * @param[in]   index_no
 * @retval  TRUE  問題あり（NGのものと一致した）
 * @retval  FALSE 問題ない
 */
//--------------------------------------------------------------
static BOOL WBT_NGINFO_Check( const WBT_NGPMINFO * ng_info, u16 mons_no, u16 item_no, s16 index_no )
{
  int i;
  //データ無し→チェックOKする
  if ( ng_info == NULL ) return TRUE;

  if ( ng_info->disableMonsCheck == FALSE )
  {
    for ( i = 0; i < ng_info->ng_mons_max; i ++ )
    {
      if ( mons_no != 0 && mons_no == ng_info->ng_mons_no[i] ) return TRUE;
    }
  }
  for ( i = 0; i < ng_info->ng_item_max; i ++ )
  {
    if ( item_no != 0 && item_no == ng_info->ng_item_no[i] ) return TRUE;
  }
  for ( i = 0; i < ng_info->ng_index_max; i ++ )
  {
    if ( index_no != NG_INDEX_NULL && index_no == ng_info->ng_index_no[i] ) return TRUE;
  }
  return FALSE;
}

//============================================================================================
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief   候補データオブジェクトの生成
 * @param[in]     heapID        使用するヒープID
 * @param[in]     poke_arc_id   ポケモンデータが格納されているアーカイブID
 * @param[in]     use_poke_cnt
 * @param[in]     use_poke_table
 * @return        POKECANDIDATE
 *
 * BSUBWAY_TRAINER_ROM_DATAを解析、BSUBWAY_POKEMON_ROM_DATAを含んだ候補ポケモン情報を作成する
 */
//--------------------------------------------------------------
static POKECANDIDATE * POKECANDIDATE_Create(
    HEAPID heapID, ARCID poke_arc_id, u16 use_poke_cnt, const u16 * use_poke_table )
{
  POKECANDIDATE * pcan;
  pcan = GFL_HEAP_AllocClearMemory( heapID, sizeof(POKECANDIDATE) );
  pcan->num = use_poke_cnt;
  pcan->array = GFL_HEAP_AllocClearMemory( heapID, sizeof(CANDIDATE_DATA) * use_poke_cnt );
  {
    int i;
    ARCHANDLE * handle;
    handle = GFL_ARC_OpenDataHandle( poke_arc_id, GFL_HEAP_LOWID(heapID) );
    for ( i = 0; i < use_poke_cnt; i ++ )
    { 
      CANDIDATE_DATA * candidate = &pcan->array[i];
      candidate->available = TRUE;
      candidate->index_no = use_poke_table[ i ];
      GFL_ARC_LoadDataByHandle( handle, candidate->index_no, &(candidate->prd) );
    }
    GFL_ARC_CloseDataHandle( handle );
  }
  return pcan;
}
//--------------------------------------------------------------
/**
 * @brief 候補データオブジェクトの破棄
 */
//--------------------------------------------------------------
static void POKECANDIDATE_Delete( POKECANDIDATE * pcan )
{
  GFL_HEAP_FreeMemory( pcan->array );
  GFL_HEAP_FreeMemory( pcan );
}

//--------------------------------------------------------------
/**
 * @brief 候補データの利用可能状況を更新する
 * @param[in,out] pcan    候補ポケモンデータ
 * @param[in]     ng_info NGデータ
 * @return  u16   選択可能な候補数
 *
 * NGデータと候補データのマッチングを行い、候補データを使えるかどうかの状態を更新している
 */
//--------------------------------------------------------------
static u16 POKECANDIDATE_updateNG( POKECANDIDATE * pcan, const WBT_NGPMINFO * ng_info )
{
  int i;
  u32 available_count = 0;
  for ( i = 0; i < pcan->num; i ++ )
  {
    CANDIDATE_DATA * candidate = &pcan->array[i];
    if ( WBT_NGINFO_Check( ng_info,
          candidate->prd.mons_no, candidate->prd.item_no, candidate->index_no ) == TRUE )
    { //NGのものは使用できないようにする
      candidate->available = FALSE;
    }
    if (candidate->available) available_count ++;
  }
  return available_count;
}

//--------------------------------------------------------------
/**
 * @brief   候補データへのインデックスを取得する
 * @param[in,out] pcan    候補ポケモンデータ
 * @param[in]     choice  選択可能数の中での番号
 * @return  u16   選択した候補データへのインデックス
 */
//--------------------------------------------------------------
static u16 POKECANDIDATE_CountAvailableIndex( const POKECANDIDATE * pcan, u32 choice )
{
  u32 poke_index;
  for ( poke_index = 0; poke_index < pcan->num; poke_index ++ )
  {
    if ( pcan->array[poke_index].available )
    {
      if ( choice == 0 ) break;
      choice --;
    }
  }
  GF_ASSERT( poke_index < pcan->num );
  return poke_index;
}
//--------------------------------------------------------------
/**
 * @brief 指定した候補データを利用できない状態にする
 * @param[in,out] pcan    候補ポケモンデータ
 * @param[in,out] ng_info NGデータ
 */
//--------------------------------------------------------------
static void POKECANDIDATE_MarkSelected( POKECANDIDATE * pcan, WBT_NGPMINFO * ng_info, u32 index )
{
  const BSUBWAY_POKEMON_ROM_DATA * prd_d;
  GF_ASSERT( index < pcan->num );
  GF_ASSERT( pcan->array[index].available == TRUE );
  pcan->array[index].available = FALSE;
  prd_d = &pcan->array[ index ].prd;

  //同一のポケモンを含むデータは使えなくなる
  WBT_NGINFO_AddNGMons( ng_info, prd_d->mons_no );
  //同一のアイテムを持つデータは使えなくなる
  WBT_NGINFO_AddNGItem( ng_info, prd_d->item_no );
}
#ifdef  PM_DEBUG
//--------------------------------------------------------------
//--------------------------------------------------------------
static void DEBUG_POKECANDIDATE_dump( const POKECANDIDATE * pcan )
{
  int i;
  TAMADA_Printf("DEBUG_dumpCandidates\n");
  for ( i = 0; i < pcan->num; i ++ )
  {
    const CANDIDATE_DATA * candidate = &pcan->array[i];
    TAMADA_Printf("Index(%3d) monsno(%3d) itemno(%3d) =:%d\n",
        candidate->index_no, candidate->prd.mons_no, candidate->prd.item_no, candidate->available );
  }
}
#endif

//--------------------------------------------------------------
/**
 * @brief バトルサブウェイ形式データからの抽選処理
 *
 * @param[out]    index_array   BSubwayデータ内のインデックスを保持する配列
 * @param[in,out] pcan          候補ポケモンのリスト
 * @param[in]     cnt           トレーナーに持たせるポケモンの数
 * @param[in,out] ng_info       NGにするポケモンの情報
 * @param[in]     rand_context  乱数値ポインタ(NULL GFUser_GetPublicRandを使用
 * @param[in]     tempHeapID    テンポラリに使用するヒープID
 *
 * @todo  異常なデータに対するリカバリ処理が必要
 *
 * (1)pcan中のNG登録のアイテム、ポケモンを含むデータを使用できないようにする
 * (2)pcanの利用可能なものからランダムで選択
 * (3)(2)で選択したのと同じポケモン・アイテムをNGINFOに登録
 * (4)　(1)～(3)を必要なだけ繰り返し、完了
 */
//--------------------------------------------------------------
static void choiceBSubwayData(
    u16 * index_array,
    POKECANDIDATE * pcan,
    u8 cnt, WBT_NGPMINFO * ng_info, GFL_STD_RandContext *rand_context, HEAPID tempHeapID )
{
  int i;
  int  set_count;
  u8 selected[TEMOTI_POKEMAX];

  GFL_STD_MemClear( selected, sizeof(u8) * TEMOTI_POKEMAX );

  for( set_count = 0; set_count != cnt; set_count ++ )
  {
    u8  poke_index;
    u32 available_count;
    u32 choice;

    //(1)NGなデータを更新し、抽選可能な対象を数える
    available_count = POKECANDIDATE_updateNG( pcan, ng_info );
#ifdef  PM_DEBUG
//    DEBUG_POKECANDIDATE_dump( pcan );
#endif
    //抽選対象が必要な数に満たない場合Assert
    GF_ASSERT( available_count >= cnt - set_count );

    //(2)利用可能なものからランダムで選択
    choice = GetRand( rand_context ) % available_count;
    TAMADA_Printf("need = %d,", cnt - set_count );
    TAMADA_Printf(" choose range(0-%d), choice = %d\n", ( available_count - 1 ), choice );
    poke_index = POKECANDIDATE_CountAvailableIndex( pcan, choice );
    TAMADA_Printf(" get index = %3d\n", poke_index );

    //(3)(2)で選択したデータと同じポケモン・アイテムをNGINFOに登録
    POKECANDIDATE_MarkSelected( pcan, ng_info, poke_index );

    selected[ set_count ] = poke_index;
  }

  TAMADA_Printf("indexes = [");
  for(i = 0; i < set_count; i++)
  {
    index_array[ i ] = pcan->array[ selected[i] ].index_no;
    TAMADA_Printf("%03d,", index_array[ i ] );
  }
  TAMADA_Printf("]\n");
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief レンタルの抽選範囲を決定する
 * @param[in]   wbtcup      トーナメント種別
 * @param[in]   isStrong
 * @param[out]  idx_s       範囲開始位置
 * @param[out]  idx_e       範囲終了位置
 */
//------------------------------------------------------------------
static void getRentalRange( WBTCUP wbtcup, BOOL isStrong, u16 * idx_s, u16 * idx_e )
{
  static u16 rent_idx_table[2][2][2] = {
  //WBTRENT_TYPE_WEAK
    {
      {   1, 132 }, { 209, 558 },
    },
  //WBTRENT_TYPE_STRONG
    {
      { 133, 208 }, { 559, 908 },
    }
  };

  int index;

  switch ( wbtcup )
  {
  default:
    GF_ASSERT( 0 );
  case WBTCUP_RENTAL:       index = 0; break;
  case WBTCUP_RENTALMASTER: index = 1; break;
  }

  *idx_s = rent_idx_table[isStrong][index][0];
  *idx_e = rent_idx_table[isStrong][index][1];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static u16 * createRangeTable( WBTCUP wbtcup, BOOL isStrong, HEAPID tempHeapID, u16 * length )
{
  u16 use_poke_cnt;
  u16 * use_poke_table;
  u16 index_s, index_e;
  int i;

  getRentalRange( wbtcup, isStrong, &index_s, &index_e );

  TAMADA_Printf("choose range = %3d-%3d\n", index_s, index_e );
  use_poke_cnt = index_e - index_s + 1;
  use_poke_table = GFL_HEAP_AllocClearMemory( tempHeapID, sizeof(u16) * use_poke_cnt );
  for ( i = 0; i < use_poke_cnt; i ++ )
  {
    use_poke_table[i] = index_s + i;
  }
  *length = use_poke_cnt;
  return use_poke_table;
}

//------------------------------------------------------------------
/**
 * @brief バトルサブウェイ形式データからの抽選処理
 * @param[out]    index_array     選ばれたポケモンのインデックス（ROMポケモン内のアーカイブIndex配列）
 * @param[in,out] ng_info         除外ポケモン指定データ
 * @param[in]     tempHeapID      テンポラリに使用するヒープ
 * @param[in]     use_poke_cnt    候補となるポケモンの数
 * @param[in]     use_poke_table  候補ポケモンのインデックス配列
 * @param[in]     arc_id          候補ポケモンのROMアーカイブID
 * @param[in]     num             選ぶポケモンの数
 * @param[in,out] rand            ランダムコンテキスト（NULLのとき、GlobalなRandを使用する）
 */
//------------------------------------------------------------------
void WBTSYS_ChooseBSubwayData( u16 * index_array, WBT_NGPMINFO * ng_info,
    HEAPID tempHeapID,
    u16 use_poke_cnt, const u16 * use_poke_table,
    ARCID arc_id, u8 num, GFL_STD_RandContext * rand )
{
  u64 start;
  u64 end;
  POKECANDIDATE * pcan;

  start = OS_GetTick();
  //候補ポケモンのリストを作成
  pcan = POKECANDIDATE_Create( tempHeapID, arc_id, use_poke_cnt, use_poke_table );

  choiceBSubwayData( index_array, pcan, num, ng_info, rand, tempHeapID );

  POKECANDIDATE_Delete( pcan );
  end = OS_GetTick();
  TAMADA_Printf("Time To Choose:%8ld\n", end - start );
}


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static void WBTSYS_SetupRentalParty(
    WBTSYS * wbtsys, POKEPARTY * party, u32 tr_id, u8 pow_rnd, u8 num, const u16 * ng_mons_tbl )
{
  enum {
    DUMMY_LEVEL_VALUE = 50, //後でレベル調整するので何でもよい
  };
  u16 index_array[ TEMOTI_POKEMAX ];
  int i;

  HEAPID tempHeapID = GFL_HEAP_LOWID( wbtsys->heapID );

  u16 use_poke_cnt;
  u16 * use_poke_table;

  WBT_NGPMINFO * ng_info;

  ng_info = NGINFO_Create( tempHeapID, wbtsys->wbtcup );
  if ( ng_mons_tbl )
  { //NGテーブルがある場合は、そこからNGのポケモンナンバーを登録する
    for ( i = 0; i < num; i ++ )
    {
      WBT_NGINFO_AddNGMons( ng_info, ng_mons_tbl[i] );
    }
  }

  //バトルサブウェイデータから抽選するのに必要なパラメータを生成
  GFL_STD_MemClear( index_array, sizeof(index_array) );

  switch ( wbtsys->rental_type )
  {
  default:
    GF_ASSERT(0);
    /* FALL THROUGH */
  case WBTRENT_TYPE_WEAK:
    use_poke_table = createRangeTable( wbtsys->wbtcup, FALSE, tempHeapID, &use_poke_cnt );
    WBTSYS_ChooseBSubwayData( index_array, ng_info, tempHeapID,
        use_poke_cnt, use_poke_table,
        ARCID_WBT_RENT_PD, num, NULL );
    GFL_HEAP_FreeMemory( use_poke_table );
    break;
  case WBTRENT_TYPE_STRONG:
    use_poke_table = createRangeTable( wbtsys->wbtcup, TRUE, tempHeapID, &use_poke_cnt );
    WBTSYS_ChooseBSubwayData( index_array, ng_info, tempHeapID,
        use_poke_cnt, use_poke_table,
        ARCID_WBT_RENT_PD, num, NULL );
    GFL_HEAP_FreeMemory( use_poke_table );
    break;
  case WBTRENT_TYPE_ALL:
    use_poke_table = createRangeTable( wbtsys->wbtcup, TRUE, tempHeapID, &use_poke_cnt );
    WBTSYS_ChooseBSubwayData( index_array, ng_info, tempHeapID,
        use_poke_cnt, use_poke_table,
        ARCID_WBT_RENT_PD, 3, NULL );
    GFL_HEAP_FreeMemory( use_poke_table );
    if ( num > 3 )
    {
      use_poke_table = createRangeTable( wbtsys->wbtcup, FALSE, tempHeapID, &use_poke_cnt );
      WBTSYS_ChooseBSubwayData( index_array + 3, ng_info, tempHeapID,
          use_poke_cnt, use_poke_table,
          ARCID_WBT_RENT_PD, num - 3, NULL );
      GFL_HEAP_FreeMemory( use_poke_table );
    }
    break;
  }

  WBT_NGINFO_Delete( ng_info );

  { //抽選したポケモンを使ってPOKEPARTYを設定する
    BSUBWAY_POKEMON * btpwd;
    btpwd = GFL_HEAP_AllocMemoryLo( tempHeapID, sizeof(BSUBWAY_POKEMON) * TEMOTI_POKEMAX );
    for(i = 0; i < num; i++)
    {
      u32 index_no = index_array[ i ];
      FBI_TOOL_MakePokemonParam( &(btpwd[i]), ARCID_WBT_RENT_PD,
        index_no, tr_id, 0, pow_rnd, i, FALSE, tempHeapID);
    }
    FBI_TOOL_WBT_SetPokemon( party, btpwd, DUMMY_LEVEL_VALUE, num, tempHeapID );
   
    GFL_HEAP_FreeMemory( btpwd );
  }
}

//------------------------------------------------------------------
/**
 * @brief レンタルトーナメント用手持ちポケモン生成
 * @param[in]   wbtsys
 *
 *  ここから手持ちを選択するため、6体生成する
 */
//------------------------------------------------------------------
void WBTSYS_SetupMyRentalParty(
    WBTSYS * wbtsys, WBTRENT_TYPE rent_type, const MYSTATUS * mystatus, u16 place_id )
{
  u32 tr_id = GFUser_GetPublicRand( 0 );
    
  WBTSYS_SetupRentalParty( wbtsys, wbtsys->sp_party, tr_id, MY_RENTAL_POWER_RND, TEMOTI_POKEMAX, NULL );

  //「ようすをみる」で正常に表示がされるように各種パラメータを上書きする
  {
    int i;
    HEAPID tempHeapID = GFL_HEAP_LOWID( wbtsys->heapID );
    STRBUF * oyaname; 
    GFL_MSGDATA     *man;
    oyaname = GFL_STR_CreateBuffer( SAVELEN_TRAINER_NAME, tempHeapID );
    man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, tempHeapID );
    if( man )
    {
      GFL_MSG_GetString( man, msg_wbtsys_oyaname, oyaname );
      GFL_MSG_Delete( man );
    }
    for(i = 0; i < TEMOTI_POKEMAX; i++)
    {
      POKEMON_PARAM * pp = PokeParty_GetMemberPointer( wbtsys->sp_party, i );
      //自分が捕まえたようなポケメモをセット
      POKE_MEMO_SetTrainerMemoPP( pp, POKE_MEMO_SET_CAPTURE,
          mystatus, place_id, tempHeapID );
      PP_Put( pp, ID_PARA_oyaname, (u32)oyaname );  //親の名前は「？？？？？」を上書き
      PP_Put( pp, ID_PARA_oyasex, PM_MALE );  //仮：とりあえず男で
    }
    GFL_STR_DeleteBuffer( oyaname );
  }
}

//------------------------------------------------------------------
/**
 * @brief レンタルトーナメント用対戦相手ポケモン生成
 * @param[in]   wbtsys
 *
 */
//------------------------------------------------------------------
void WBTSYS_SetupEnemyRentalParty( WBTSYS * wbtsys )
{
  u16 ng_mons_table[TEMOTI_POKEMAX];
  int i;
  int num = WBTSYS_GetPartyMax( wbtsys );
  int max = PokeParty_GetPokeCount( wbtsys->my_party );
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  u8 pow_rnd      = WBTTRAINER_GetPowerRnd(wbt_tr);
  //自分の手持ちとポケモンがかぶらないように抽選する
  GFL_STD_MemClear( ng_mons_table, sizeof(ng_mons_table) );
  for ( i = 0; i < max; i ++ )
  {
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( wbtsys->my_party, i );
    ng_mons_table[i] = PP_Get( pp, ID_PARA_monsno, NULL );
  }
  WBTSYS_SetupRentalParty( wbtsys, wbtsys->npc_party, 12345, pow_rnd, num, ng_mons_table );
}



