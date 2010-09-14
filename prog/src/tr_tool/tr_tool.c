
//============================================================================================
/**
 * @file  tr_tool.c
 * @bfief トレーナーデータツール郡
 * @author  HisashiSogabe
 * @date  05.12.14
 */
//============================================================================================
#include  <gflib.h>

#include  "arc_def.h"

#include  "waza_tool/wazano_def.h"
#include  "tr_tool/tr_tool.h"
#include  "tr_tool/trno_def.h"
#include  "tr_tool/trtype_def.h"
#include  "tr_tool/trtype_sex.h"
#include  "poke_tool/poke_tool.h"
#include  "poke_tool/pokeparty.h"

#include  "pm_define.h"

#include  "message.naix"

#ifdef PM_DEBUG
#if defined DEBUG_ONLY_FOR_sogabe | defined DEBUG_ONLY_FOR_nishino
#define SEIKAKU_PRINT
#endif
#endif

//============================================================================================
/**
 * 構造体宣言
 */
//============================================================================================

/*
 *  @brief  トレーナータイプに依存して決定する各種パラメータ定義
 */
typedef struct _TRTYPE_GRP_PARAM{
  u16  tr_type:7;      ///<トレーナータイプ 7
  u16  type_grp:4;     ///<トレーナータイプグループ 4
  u16  btl_bg_attr:5;  ///<戦闘背景BGアトリビュート 5
}TRTYPE_GRP_PARAM;

//============================================================================================
/**
 * 定数宣言
 */
//============================================================================================
/*
 *  @brief  トレーナータイプをグループ化するための対応テーブル
 *
 *  拡張する場合、岩澤に確認をお願いします
 */
static const TRTYPE_GRP_PARAM DATA_TrTypeGrpParam[TRTYPE_GRP_IDX_MAX] = {
 { TRTYPE_RIVAL,    TRTYPE_GRP_RIVAL,	  BATTLE_BG_ATTR_MAX },
 { TRTYPE_SUPPORT,	TRTYPE_GRP_SUPPORT,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER1A,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER1B,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER1C,	TRTYPE_GRP_LEADER,  BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER2,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER3,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER4,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER5,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER6,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER7,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER8A,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_LEADER8B,	TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR1,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR2,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR3,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR4,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_CHAMPION, TRTYPE_GRP_CHAMPION,BATTLE_BG_ATTR_MAX	},
 { TRTYPE_BOSS,	    TRTYPE_GRP_TRAINER_BOSS,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BOSS_1,   TRTYPE_GRP_PLASMA_BOSS,	  BATTLE_BG_ATTR_MAX },
 { TRTYPE_BOSS_2,   TRTYPE_GRP_LAST_BOSS,	    BATTLE_BG_ATTR_MAX },
 { TRTYPE_SAGE1,	  TRTYPE_GRP_SAGE,	  BATTLE_BG_ATTR_SAGE },
 { TRTYPE_HAKAIM1,	TRTYPE_GRP_PLASMA,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_HAKAIW1,	TRTYPE_GRP_PLASMA,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_DPCHAMPION,TRTYPE_GRP_DPCHAMP,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BCHAMP,	  TRTYPE_GRP_BCHAMP,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BCHAMP2,  TRTYPE_GRP_BCHAMP,	BATTLE_BG_ATTR_MAX },
};
#ifdef _NITRO
// 構造体が想定のサイズとなっているかチェック
SDK_COMPILER_ASSERT(sizeof(TRTYPE_GRP_PARAM) == 2);
#endif


//============================================================================================
/**
 * プロトタイプ宣言
 */
//============================================================================================
static  void  rnd_tmp_get( int mons_no, int form_no, int para, u32* rnd_tmp, HEAPID heapID );
static  void  pp_set_param( TrainerID tr_id, POKEMON_PARAM *pp, int form_no, int para );
static inline u32 TTL_SETUP_POW_PACK( u8 pow );

//============================================================================================
/**
 *  @brief  トレーナーデータを取得する
 *
 *  @param[in]  tr_id   取得したいトレーナーID
 *  @param[in]  id      取得したい値の種類
 *
 *  @retval 取得した値
 */
//============================================================================================
u32 TT_TrainerDataParaGet( TrainerID tr_id, int id )
{
  u32           ret;
  TRAINER_DATA  td;

  TT_TrainerDataGet( tr_id, &td );

  switch( id ){
  case ID_TD_data_type:   //データタイプ
    ret = td.data_type;
    break;
  case ID_TD_tr_type:     //トレーナー分類
    ret = td.tr_type;
    break;
  case ID_TD_fight_type:
    ret = td.fight_type;
    break;
  case ID_TD_poke_count:  //所持ポケモン数
    ret = td.poke_count;
    break;
  case ID_TD_use_item1:   //使用道具1
  case ID_TD_use_item2:   //使用道具2
  case ID_TD_use_item3:   //使用道具3
  case ID_TD_use_item4:   //使用道具4
    ret = td.use_item[ id - ID_TD_use_item1 ];
    break;
  case ID_TD_aibit:       //AIパターン
    ret = td.aibit;
    break;
  case ID_TD_hp_recover_flag: //戦闘後回復するか？
    ret = td.hp_recover_flag;
    break;
  case ID_TD_gold:            //おこづかい係数
    ret = td.gold;
    break;
  case ID_TD_gift_item:       //贈呈するアイテム
    ret = td.gift_item;
    break;
  default:
    //定義されていないIDです
    GF_ASSERT( 0 );
    break;
  }
  return ret;
}

//============================================================================================
/**
 *  @briefトレーナーメッセージデータが存在するかチェック
 *
 *  @param[in]  tr_id   チェックしたいトレーナーID
 *  @param[in]  kindID  チェックしたいメッセージの種類
 *  @param[in]  heapID  テンポラリ領域の生成先ヒープID
 *
 * @retval  TRUE:存在する　FALSE:存在しない
 */
//============================================================================================
BOOL  TT_TrainerMessageCheck( TrainerID tr_id, int kindID, HEAPID heapID )
{
  ARCHANDLE*  handle;
  int         size;
  u16         ofs;
  u16         data[ 2 ];
  BOOL        ret;

  ret=FALSE;

  size = GFL_ARC_GetDataSize( ARCID_TRMSGTBL, 0 );
  GFL_ARC_LoadDataOfs( &ofs, ARCID_TRMSGTBL_OFS, 0, tr_id * 2, 2 );
  handle = GFL_ARC_OpenDataHandle( ARCID_TRMSGTBL, heapID );

  while( ofs != size ){
    GFL_ARC_LoadDataOfsByHandle( handle, 0, ofs, 4, &data[ 0 ] );
    if( ( data[ 0 ] == tr_id ) && ( data[ 1 ] == kindID ) ){
      ret = TRUE;
      break;
    }
    if( data[ 0 ] != tr_id ){
      break;
    }
    ofs += 4;
  }
  GFL_ARC_CloseDataHandle( handle );

  return ret;
}

//============================================================================================
/**
 *  @brief  トレーナーメッセージデータを取得
 *
 *  @param[in]  tr_id   取得したいトレーナーID
 *  @param[in]  kindID  取得したいメッセージの種類
 *  @param[out] msg     取得したメッセージの格納先へのポインタ
 *  @param[in]  heapID  テンポラリ領域の生成先ヒープID
 */
//============================================================================================
void  TT_TrainerMessageGet( TrainerID tr_id, int kindID, STRBUF* str, HEAPID heapID )
{
  ARCHANDLE*    handle;
  int           size;
  int           ofs_size;
  u16           ofs;
  u16           data[ 2 ];
  GFL_MSGDATA*  msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_trmsg_dat, heapID );

  size = GFL_ARC_GetDataSize( ARCID_TRMSGTBL, 0 );
#ifdef PM_DEBUG
  ofs_size = GFL_ARC_GetDataSize( ARCID_TRMSGTBL_OFS, 0 );
  if( ofs_size < tr_id * 2 )
  {
    GFL_STR_ClearBuffer( str );
    return;
  }
#endif
  GFL_ARC_LoadDataOfs( &ofs, ARCID_TRMSGTBL_OFS, 0, tr_id * 2, 2 );
  handle = GFL_ARC_OpenDataHandle( ARCID_TRMSGTBL, heapID );

  while( ofs != size ){
    GFL_ARC_LoadDataOfsByHandle( handle, 0, ofs, 4, &data[ 0 ] );
    if( ( data[ 0 ] == tr_id ) && ( data[ 1 ] == kindID ) ){
      GFL_MSG_GetString( msg, ofs / 4, str );
      break;
    }
    ofs += 4;
  }
  GFL_ARC_CloseDataHandle( handle );
  GFL_MSG_Delete( msg );

//  GF_ASSERT_MSG(ofs!=size,"TrainerMessage Not Found");
  //メッセージが見つからなかったら、STRBUFをクリアして返す
  if( ofs == size )
  {
    GFL_STR_ClearBuffer( str );
  }
}

//============================================================================================
/**
 *  @briefトレーナー構造体データを取得
 *
 *  @param[in]  tr_id   取得したいトレーナーID
 *  @param[in]  td      取得したトレーナーデータの格納先を指定
 */
//============================================================================================
void  TT_TrainerDataGet( TrainerID tr_id, TRAINER_DATA* td )
{
  GFL_ARC_LoadData( td, ARCID_TRDATA, tr_id );
}

//============================================================================================
/**
 *  @brief  トレーナー持ちポケモン構造体データを取得
 *
 *  @param[in]  tr_id   取得したいトレーナーID
 *  @param[in]  td      取得したトレーナー持ちポケモンデータの格納先を指定
 */
//============================================================================================
void  TT_TrainerPokeDataGet( TrainerID tr_id, void* tpd )
{
  GFL_ARC_LoadData( tpd, ARCID_TRPOKE, tr_id );
}

//============================================================================================
/**
 *  @brief  トレーナータイプから性別を取得する
 *
 *  @param[in]  trtype    取得したいトレーナータイプ
 */
//============================================================================================
u8  TT_TrainerTypeSexGet( int trtype )
{
  return TrTypeSexTable[ trtype ];
}

//----------------------------------------------------------
/*
 *  @brief  トレーナータイプをトレーナータイプグループIdxに変換
 */
//----------------------------------------------------------
u8 TT_TrainerTypeGrpEntryIdxGet( int trtype )
{
  int i;
  const TRTYPE_GRP_PARAM* prm;

  for(i = 0;i < TRTYPE_GRP_IDX_MAX; i++){
    if( DATA_TrTypeGrpParam[i].tr_type == trtype ){
      return i;
    }
  }
  return TRTYPE_GRP_IDX_MAX;
}
//----------------------------------------------------------
/*
 *  @brief  トレーナータイプをトレーナータイプグループに変換
 */
//----------------------------------------------------------
TRTYPE_GRP_ID TT_TrainerTypeGrpGet( int trtype )
{
  u8 idx = TT_TrainerTypeGrpEntryIdxGet( trtype );

  if(idx >= TRTYPE_GRP_IDX_MAX){
    return TRTYPE_GRP_NONE;
  }
  return DATA_TrTypeGrpParam[idx].type_grp;
}
//----------------------------------------------------------
/*
 *  @brief  トレーナータイプからBtlBgAttrを取得
 */
//----------------------------------------------------------
BtlBgAttr TT_TrainerTypeBtlBgAttrGet( int trtype )
{
  u8 idx = TT_TrainerTypeGrpEntryIdxGet( trtype );

  if(idx >= TRTYPE_GRP_IDX_MAX){
    return BATTLE_BG_ATTR_MAX;
  }
  return DATA_TrTypeGrpParam[idx].btl_bg_attr;
}

//============================================================================================
/**
 *  @brief  エンカウントトレーナーデータを取得する
 *
 *  @param[in/out]  bp      戦闘システムに引き渡す構造体のポインタ
 *  @param[in]      num     何番目のトレーナーID？（タッグの時のため）
 *  @param[in]      heapID  メモリ確保するためのヒープID
 */
//============================================================================================
void TT_EncountTrainerPersonalDataMake( TrainerID tr_id, BSP_TRAINER_DATA* data, HEAPID heapID )
{
  TRAINER_DATA*   td;

  td = GFL_HEAP_AllocMemory( heapID, sizeof( TRAINER_DATA ) );

  TT_TrainerDataGet( tr_id, td );

  data->tr_id = tr_id;
  data->tr_type = td->tr_type;
  data->ai_bit = td->aibit;

  {
    GFL_MSGDATA* man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_trname_dat, heapID );
    GFL_MSG_GetString( man, tr_id, data->name );
    GFL_MSG_Delete(man);
  }

  MI_CpuCopy16( td->use_item, data->use_item, sizeof(u16)*4);
  PMSDAT_Clear( &data->win_word );
  PMSDAT_Clear( &data->lose_word );

  GFL_HEAP_FreeMemory( td );
}

//============================================================================================
/**
 *  @brief  エンカウントトレーナー持ちポケモンデータを生成する
 *
 *  @param[in/out]  bp      戦闘システムに引き渡す構造体のポインタ
 *  @param[in]      num     何番目のトレーナーID？（タッグの時のため）
 *  @param[in]      heapID  メモリ確保するためのヒープID
 */
//============================================================================================
void TT_EncountTrainerPokeDataMake( TrainerID tr_id, POKEPARTY* pparty, HEAPID heapID )
{
  TRAINER_DATA*   td;
  void*           buf;
  int             i, j;
  u32             rnd_tmp, rnd;
  u8              pow;
  POKEMON_PARAM*  pp;
  GFL_STD_RandContext rand_c;

  PokeParty_Init( pparty, TEMOTI_POKEMAX );

  td = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID(heapID), sizeof( TRAINER_DATA ) );
  buf = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID(heapID), sizeof( POKEDATA_TYPE_MULTI ) * TEMOTI_POKEMAX );
  pp = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID(heapID), POKETOOL_GetWorkSize() );

  TT_TrainerDataGet( tr_id, td );
  TT_TrainerPokeDataGet( tr_id, buf );

  if( TT_TrainerTypeSexGet( td->tr_type ) == PTL_SEX_FEMALE )
  {
    rnd_tmp = 120;
  }
  else
  {
    rnd_tmp = 136;
  }

  switch( td->data_type ){
  case DATATYPE_NORMAL:
    {
      POKEDATA_TYPE_NORMAL* ptn;

      ptn = ( POKEDATA_TYPE_NORMAL * )buf;
      for( i = 0 ; i < td->poke_count ; i++ )
      {
        rnd_tmp_get( ptn[ i ].monsno, ptn[ i ].form_no, ptn[ i ].para, &rnd_tmp, heapID );
        //持ちポケモンデータのパラメータを使用して、ポケモンの個性乱数を生成
        rnd = ptn[ i ].pow + ptn[ i ].level + ptn[ i ].monsno + tr_id;
        GFL_STD_RandInit( &rand_c, rnd );
        for( j = 0 ; j < td->tr_type ; j++ )
        {
          rnd = GFL_STD_Rand( &rand_c, 0x10000 );
        }
        rnd = ( rnd << 8 ) + rnd_tmp;
        pow = ptn[ i ].pow * 31 / 255;
        PP_SetupEx( pp,
                    ptn[ i ].monsno,
                    ptn[ i ].level,
                    PTL_SETUP_ID_NOT_RARE,
                    TTL_SETUP_POW_PACK( pow ),
                    rnd );
#ifdef BUGFIX_BTS7992_20100914
        PP_SetWazaDefault( pp );
#endif
        pp_set_param( tr_id, pp, ptn[ i ].form_no, ptn[ i ].para );
        PokeParty_Add( pparty, pp );
      }
    }
    break;
  case DATATYPE_WAZA:
    {
      POKEDATA_TYPE_WAZA* ptw;

      ptw = ( POKEDATA_TYPE_WAZA * )buf;
      for( i = 0 ; i < td->poke_count ; i++ )
      {
        rnd_tmp_get( ptw[ i ].monsno, ptw[ i ].form_no, ptw[ i ].para, &rnd_tmp, heapID );
        //持ちポケモンデータのパラメータを使用して、ポケモンの個性乱数を生成
        rnd = ptw[ i ].pow + ptw[ i ].level + ptw[i].monsno + tr_id;
        GFL_STD_RandInit( &rand_c, rnd );
        for( j = 0 ; j < td->tr_type ; j++ )
        {
          rnd = GFL_STD_Rand( &rand_c, 0x10000 );
        }
        rnd = ( rnd << 8 ) + rnd_tmp;
        pow = ptw[ i ].pow * 31 / 255;
        PP_SetupEx( pp,
                    ptw[ i ].monsno,
                    ptw[ i ].level,
                    PTL_SETUP_ID_NOT_RARE,
                    TTL_SETUP_POW_PACK( pow ),
                    rnd );
        for( j = 0 ; j < 4 ; j++ )
        {
          PP_SetWazaPos( pp, ptw[ i ].waza[ j ], j );
        }
        pp_set_param( tr_id, pp, ptw[ i ].form_no, ptw[ i ].para );
        PokeParty_Add( pparty, pp );
      }
    }
    break;
  case DATATYPE_ITEM:
    {
      POKEDATA_TYPE_ITEM* pti;

      pti = ( POKEDATA_TYPE_ITEM * )buf;
      for( i = 0 ; i < td->poke_count ; i++ )
      {
        rnd_tmp_get( pti[ i ].monsno, pti[ i ].form_no, pti[ i ].para, &rnd_tmp, heapID );
        //持ちポケモンデータのパラメータを使用して、ポケモンの個性乱数を生成
        rnd = pti[ i ].pow + pti[ i ].level + pti[ i ].monsno + tr_id;
        GFL_STD_RandInit( &rand_c, rnd );
        for( j = 0 ; j < td->tr_type ; j++ )
        {
          rnd = GFL_STD_Rand( &rand_c, 0x10000 );
        }
        rnd = ( rnd << 8 ) + rnd_tmp;
        pow = pti[ i ].pow * 31 / 255;
        PP_SetupEx( pp,
                    pti[ i ].monsno,
                    pti[ i ].level,
                    PTL_SETUP_ID_NOT_RARE,
                    TTL_SETUP_POW_PACK( pow ),
                    rnd );
        PP_Put( pp, ID_PARA_item, pti[ i ].itemno );
        pp_set_param( tr_id, pp, pti[ i ].form_no, pti[ i ].para );
        PokeParty_Add( pparty, pp );
      }
    }
    break;
  case DATATYPE_MULTI:
    {
      POKEDATA_TYPE_MULTI*  ptm;

      ptm = ( POKEDATA_TYPE_MULTI * )buf;
      for( i = 0 ; i < td->poke_count ; i++ )
      {
        rnd_tmp_get( ptm[ i ].monsno, ptm[ i ].form_no, ptm[ i ].para, &rnd_tmp, heapID );
        //持ちポケモンデータのパラメータを使用して、ポケモンの個性乱数を生成
        rnd = ptm[ i ].pow + ptm[ i ].level + ptm[ i ].monsno + tr_id;
        GFL_STD_RandInit( &rand_c, rnd );
        for( j = 0 ; j < td->tr_type ; j++ )
        {
          rnd = GFL_STD_Rand( &rand_c, 0x10000 );
        }
        rnd = ( rnd << 8 ) + rnd_tmp;
        pow = ptm[ i ].pow * 31 / 255;
        PP_SetupEx( pp,
                    ptm[ i ].monsno,
                    ptm[ i ].level,
                    PTL_SETUP_ID_NOT_RARE,
                    TTL_SETUP_POW_PACK( pow ),
                    rnd );
        PP_Put( pp, ID_PARA_item, ptm[ i ].itemno );
        for( j = 0 ; j < 4 ; j++ )
        {
          PP_SetWazaPos( pp, ptm[ i ].waza[ j ], j );
        }
        pp_set_param( tr_id, pp, ptm[ i ].form_no, ptm[ i ].para );
        PokeParty_Add( pparty, pp );
      }
    }
    break;
  }
  GFL_HEAP_FreeMemory( td );
  GFL_HEAP_FreeMemory( buf );
  GFL_HEAP_FreeMemory( pp );
}

//============================================================================================
/**
 *              外部公開しない関数郡
 */
//============================================================================================

//============================================================================================
/**
 * @brief ポケモンの特性と性別を決定する乱数値を算出する
 *
 * @param[in]     mons_no 算出するモンスターナンバー
 * @param[in]     form_no 算出するフォルムナンバー
 * @param[in]     para    特性と性別パラメータ
 * @param[in/out] rnd_tmp 求める乱数値
 */
//============================================================================================
static  void  rnd_tmp_get( int mons_no, int form_no, int para, u32* rnd_tmp, HEAPID heapID )
{
  int sex     = para & 0x0f;
  int speabi  = ( para & 0xf0 ) >> 4;
  POKEMON_PERSONAL_DATA*  ppd;

  if( para == 0 ){
    return;
  }

  ppd = POKE_PERSONAL_OpenHandle( mons_no, form_no, heapID );

  if( sex ){
    *rnd_tmp = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
    if( sex == 1 ){
      *rnd_tmp += 2;
    }
    else{
      *rnd_tmp -= 2;
    }
  }

  if( speabi == 1 ){
    *rnd_tmp &= 0xfffffffe;
  }
  else if( speabi == 2 ){
    *rnd_tmp |= 0x00000001;
  }

  POKE_PERSONAL_CloseHandle( ppd );
}

//============================================================================================
/**
 * @brief PP_Setup後のパラメータ設定
 *
 * @param[in] pp      POKEMON_PARAM構造体のポインタ
 * @param[in] form_no フォルムナンバー
 */
//============================================================================================
static  void  pp_set_param( TrainerID tr_id, POKEMON_PARAM* pp, int form_no, int para )
{
  //なつき度の初期値は255
  u8  friend = 255;
  int no;
  u16 mons_no = PP_Get( pp, ID_PARA_monsno, NULL );

  for( no = 0 ; no < PTL_WAZA_MAX ; no++ ){
    //やつあたりを持っているときは、なつき度を0にする
    if( PP_Get( pp, ID_PARA_waza1 + no , NULL ) == WAZANO_YATUATARI ){
      friend = 0;
    }
  }
  PP_Put( pp, ID_PARA_friend, friend );

  //フォルムナンバーセット
  PP_Put( pp, ID_PARA_form_no, form_no );

  //特性セット
  { 
    if( ( para & 0xf0 ) == 0x30 )
    { 
      PP_SetTokusei3( pp, mons_no, form_no );
    }
    else if( para & 0xf0 )
    { 
      u16 param = POKEPER_ID_speabi1;
      if( POKETOOL_GetPersonalParam( mons_no, form_no, POKEPER_ID_speabi2 ) )
      { 
        if( ( para & 0xf0 ) == 0x20 )
        { 
          param = POKEPER_ID_speabi2;
        }
      }
      PP_Put( pp, ID_PARA_speabino, POKETOOL_GetPersonalParam( mons_no, form_no, param ) );
    }
  }

  //アッパーバージョンへのコメント
  //１：性格を決める乱数生成からパワー乱数を外す（パワー乱数で弱く調整しても性格が変わってかえって強くなったりする）
  //２：上記の理由からトレーナーデータに性格きめうちも入れる（パワー乱数で調整しきれないとき用に）
  //性格セット
  { 
    //特定のトレーナーの特定ポケモンの性格を固定にする
    //レシラム
    if( ( tr_id == TRID_BOSS_2_06 ) && ( mons_no == MONSNO_650 ) )
    { 
      PP_SetSeikaku( pp, PTL_SEIKAKU_YOUKI );
    }
    //ゼクロム
    else if( ( tr_id == TRID_BOSS_2_07 ) && ( mons_no == MONSNO_651 ) )
    { 
      PP_SetSeikaku( pp, PTL_SEIKAKU_OKUBYOU );
    }
    else
    { 
      int rnd = PP_Get( pp, ID_PARA_personal_rnd, NULL );

      //下位1バイトは性別のために偏りがあるので、上位24bitで計算する
      rnd >>= 8;

      PP_SetSeikaku( pp, rnd % PTL_SEIKAKU_MAX );
    }
  }
#ifdef SEIKAKU_PRINT
  OS_TPrintf("tr_id:%d mons_no:%d seikaku:%d\n",tr_id,PP_Get(pp,ID_PARA_monsno,NULL),PP_Get(pp,ID_PARA_seikaku,NULL));
#endif
}

//============================================================================================
/**
 * @brief 個体値を任意の値に設定するための変数パッキング処理
 *
 * @param[in] pow パワー乱数値
 */
//============================================================================================
static inline u32 TTL_SETUP_POW_PACK( u8 pow )
{
  return ( (pow&0x1f)|((pow&0x1f)<<5)|((pow&0x1f)<<10)|((pow&0x1f)<<15)|((pow&0x1f)<<20)|((pow&0x1f)<<25) );
}

