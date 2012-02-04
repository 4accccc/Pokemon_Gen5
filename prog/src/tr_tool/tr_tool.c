
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
//#include  "tr_tool/trtype_sex.h"
#include  "tr_tool/ex_trainer_def.h"
#include  "poke_tool/poke_tool.h"
#include  "poke_tool/pokeparty.h"

#include  "pm_define.h"

#include  "sound/swan_sound_data.sadl"
#include  "message.naix"

#include  "gamesystem/game_data.h"
#include  "savedata/etc_save.h"

#include  "field/infinity_dungeon_def.h"

#ifdef PM_DEBUG
#if defined DEBUG_ONLY_FOR_sogabe | defined DEBUG_ONLY_FOR_nishino | defined DEBUG_ONLY_FOR_nakatsui | defined DEBUG_ONLY_FOR_nagihashi_toru
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
  u8  tr_type;      ///<トレーナータイプ
  u8  type_grp;     ///<トレーナータイプグループ
  u8  btl_bg_attr;  ///<戦闘背景BGアトリビュート
}TRTYPE_GRP_PARAM;

/**
 *  @brief  トレーナータイプ別の設定データ
 */
typedef struct {
  u8 trfno;     ///<グラフィック指定ID（表向き）
  u8 sex;       ///<性別
  u16 bgmNo;    ///<視線BGM
}TRAINER_TYPE_DATA;

#include "trtype_data.cdat"
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
 { TRTYPE_SWLEADER1,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER2,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER3,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER4,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER5,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER6,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER7,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWLEADER8,TRTYPE_GRP_LEADER,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR1,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR2,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR3,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BIGFOUR4,	TRTYPE_GRP_BIGFOUR,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_DPCHAMPION,TRTYPE_GRP_DPCHAMP,BATTLE_BG_ATTR_MAX },
 // swan
 { TRTYPE_SWRIVAL,  TRTYPE_GRP_RIVAL,	  BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWBOSS_1, TRTYPE_GRP_TRAINER_SWBOSS,  BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWBOSS,   TRTYPE_GRP_PLASMA_SWBOSS,   BATTLE_BG_ATTR_MAX	},
 { TRTYPE_BOSS,     TRTYPE_GRP_N,       BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWPLAZMAM,TRTYPE_GRP_PLASMA,  BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWPLAZMAW,TRTYPE_GRP_PLASMA,  BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWSAGE3,  TRTYPE_GRP_SAGE3,   BATTLE_BG_ATTR_MAX	},
 { TRTYPE_NINJA,    TRTYPE_GRP_PLASMA,  BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWCHAMPION,TRTYPE_GRP_CHAMPION,BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWSAGE1,	TRTYPE_GRP_SAGE1,	  BATTLE_BG_ATTR_MAX },
 { TRTYPE_MUGENBOSS,TRTYPE_GRP_INFINITY,BATTLE_BG_ATTR_MAX },
 { TRTYPE_SWBOSS_2, TRTYPE_GRP_CLEAR_SWBOSS,   BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SWLEADER1_1, TRTYPE_GRP_RIVAL2,  BATTLE_BG_ATTR_MAX	},
 { TRTYPE_SUPPORT,  TRTYPE_GRP_RIVAL2,  BATTLE_BG_ATTR_MAX	},
 { TRTYPE_BCHAMP,	  TRTYPE_GRP_BCHAMP,	BATTLE_BG_ATTR_MAX },
 { TRTYPE_BCHAMP2,  TRTYPE_GRP_BCHAMP,	BATTLE_BG_ATTR_MAX },
};

/*
 *  @brief  トレーナータイプでBtlBgTypeを書き換えるためのテーブル
 */
static const u16 DATA_TrType2BgType[][2]={
  { TRTYPE_SWSAGE1, BATTLE_BG_TYPE_KYUREMU },   //ゲーチス
};

#ifdef _NITRO
// 構造体が想定のサイズとなっているかチェック
SDK_COMPILER_ASSERT(sizeof(TRTYPE_GRP_PARAM) == 3);
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

  if( TT_ExTrainerTypeCheck( tr_id ) == EX_TRTYPE_NONE )
  {
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
  }
  else
  {
    if( kindID == TRMSG_FIGHT_LOSE || kindID == TRMSG_FIGHT_LOSE_2 )
    {
      ret = TRUE;
    }
  }

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
  u8            ex_trtype = TT_ExTrainerTypeCheck( tr_id );

  if( ex_trtype == EX_TRTYPE_NONE )
  {
    GFL_MSGDATA*  msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_trmsg_dat, heapID );

    size = GFL_ARC_GetDataSize( ARCID_TRMSGTBL, 0 );
// デバッグだけでなく、製品版でも対応するメッセージがない場合に誤動作しないようにする
//#ifdef PM_DEBUG
    ofs_size = GFL_ARC_GetDataSize( ARCID_TRMSGTBL_OFS, 0 );
    if( ofs_size < tr_id * 2 )
    {
      GFL_STR_ClearBuffer( str );
      GFL_MSG_Delete( msg );
      return;
    }
//#endif
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
  else
  {
    switch( ex_trtype )
    {
    case EX_TRTYPE_INFINITY_DUNGEON:
      {
        GFL_MSGDATA*  msg = NULL;

        // 種類ごとのGMM読みわけ
        switch( kindID )
        {
        case TRMSG_FIGHT_START:
          {
            msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                ARCID_MESSAGE, NARC_message_infinity_dungeon_btl_startmsg_dat, heapID );
          }
          break;
        case TRMSG_FIGHT_LOSE:
          {
            msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                ARCID_MESSAGE, NARC_message_infinity_dungeon_btl_losemsg_dat, heapID );
          }
          break;
        default:
          {
            GF_ASSERT_MSG(0,"無限ダンジョン内トレーナーが持ってないタイプのメッセージ表示をしようとした\n");
          }
          break;
        }

        // メッセージの作成
        if( msg )
        {
          GFL_MSG_GetString( msg, (tr_id - TRID_OFFSET_INFINITY_DUNGEON), str );
          GFL_MSG_Delete( msg );
        }
      }
      break;
    default:
      GF_ASSERT(0);
      break;
    }
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
  if( tr_id <  TRID_OFFSET_INFINITY_DUNGEON )
  {
    GFL_ARC_LoadData( td, ARCID_TRDATA, tr_id );
  }
  else
  {
    TrainerID id = tr_id - TRID_OFFSET_INFINITY_DUNGEON;
    if((INF_TRID_START_MONEY<=id)&&(INF_TRID_START_MONEY+INF_TRID_CNT_MONEY > id))
    {
      ONOUE_Printf("トレーナーデータ差し替え(お金)\n");
      GFL_ARC_LoadData( td, ARCID_TRDATA, TRID_GENTLE_09 );
    }
    else
    {
      ONOUE_Printf("トレーナーデータ差し替え(通常)\n");
      GFL_ARC_LoadData( td, ARCID_TRDATA, TRID_CHILDCARE_10 );
    }
  }
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
  return DATA_TrainerType[ trtype ].sex;
  //return TrTypeSexTable[ trtype ];
}

//============================================================================================
/**
 *  @brief  トレーナータイプからグラフィックデータへのインデックスを取得する
 *
 *  @param[in]  trtype
 *  @param[in]  dir     最下位ビットが  0:正面　1:背面
 *  @return     u8      グラフィックデータへのインデックス
 *
 */
//============================================================================================
u8  TT_TrainerTypeCGIndexGet( int trtype, int dir )
{
  SDK_COMPILER_ASSERT( TRTYPE_HERO == 0 );
  SDK_COMPILER_ASSERT( TRTYPE_HEROINE == 1 );

  if ( dir & 1 )
  {
    //背面の場合の分岐
    int i;
    for ( i = 0; i < NELEMS(DATA_TrainerBackCGIndex); i ++ )
    {
      if ( DATA_TrainerBackCGIndex[i][0] == trtype )
      {
        return DATA_TrainerBackCGIndex[i][1];
      }
    }
    if ( TT_TrainerTypeSexGet( trtype ) == PTL_SEX_FEMALE )
    {
      return DATA_TrainerBackCGIndex[0][1]; //0 == TRTYPE_HERO
    }
    return DATA_TrainerBackCGIndex[1][1];   //1 == TRTYPE_HEROINE
  }

  //正面の場合は素直にテーブル引き
  if ( trtype < NELEMS(DATA_TrainerType) )
  {
    return DATA_TrainerType[ trtype ].trfno;
  }
  GF_ASSERT( 0 );
  return 0;
}

//============================================================================================
/**
 *  @brief    トレーナータイプから視線BGM指定を取得する
 *
 *  @param[in]  trtype
 *  @return     u16
 */
//============================================================================================
u16 TT_TrainerTypeEyeBGMGet( int trtype )
{
  return DATA_TrainerType[ trtype ].bgmNo;
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

//----------------------------------------------------------
/*
 *  @brief  トレーナータイプからBtlBgTypeを取得
 */
//----------------------------------------------------------
BtlBgType TT_TrainerTeypBtlBgTypeGet( int trtype, BtlBgAttr BgType )
{
  int i;
  for(i=0;i<NELEMS(DATA_TrType2BgType);i++){
    if(trtype==DATA_TrType2BgType[i][0]){
      return DATA_TrType2BgType[i][1];
    }
  }
  return BgType;
}

//============================================================================================
/**
 *  @brief  エンカウントトレーナーデータを取得する
 *
 *  @param[in]      gamedata ゲームデータ
 *  @param[in/out]  bp      戦闘システムに引き渡す構造体のポインタ
 *  @param[in]      num     何番目のトレーナーID？（タッグの時のため）
 *  @param[in]      heapID  メモリ確保するためのヒープID
 */
//============================================================================================
void	TT_EncountTrainerPersonalDataMake( GAMEDATA* gamedata, TrainerID tr_id, BSP_TRAINER_DATA* data, HEAPID heapID )
{
  TRAINER_DATA*   td;

  td = GFL_HEAP_AllocMemory( heapID, sizeof( TRAINER_DATA ) );

  TT_TrainerDataGet( tr_id, td );

  data->tr_id = tr_id;
  data->tr_type = td->tr_type;
  data->ai_bit = td->aibit;

  if( data->tr_type == TRTYPE_SWRIVAL )
  {
    // ライバルならセーブデータから名前を取得
    SAVE_CONTROL_WORK * sv_ctrl_work = GAMEDATA_GetSaveControlWork( gamedata );
    ETC_SAVE_WORK * etcsave = SaveData_GetEtc( sv_ctrl_work );
    GFL_STR_SetStringCode( data->name, EtcSave_GetRivalName( etcsave ) );
  }
  else
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
#ifdef BUGFIX_AF_BTS7992_20100914
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
    int rnd = PP_Get( pp, ID_PARA_personal_rnd, NULL );

    //下位1バイトは性別のために偏りがあるので、上位24bitで計算する
    rnd >>= 8;

    PP_SetSeikaku( pp, rnd % PTL_SEIKAKU_MAX );
  }
#ifdef SEIKAKU_PRINT
  {
    static const char * scp_seikaku[PTL_SEIKAKU_MAX] =
    {
      "がんばりや",
      "さみしがり",
      "ゆうかん",
      "いじっぱり",
      "やんちゃ",
      "ずぶとい",
      "すなお",
      "のんき",
      "わんぱく",
      "のうてんき",
      "おくびょう",
      "せっかち",
      "まじめ",
      "ようき",
      "むじゃき",
      "ひかえめ",
      "おっとり",
      "れいせい",
      "てれや",
      "うっかりや",
      "おだやか",
      "おとなしい",
      "なまいき",
      "しんちょう",
      "きまぐれ",
    };

    OS_TPrintf("tr_id:%d mons_no:%d seikaku:%s\n",tr_id,PP_Get(pp,ID_PARA_monsno,NULL),scp_seikaku[PP_Get(pp,ID_PARA_seikaku,NULL)]);
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  特殊トレーナータイプを取得する
 *
 *	@param	tr_id 
 *
 *	@retval 特殊トレーナータイプ
 */
//-----------------------------------------------------------------------------
u8 TT_ExTrainerTypeCheck( TrainerID tr_id )
{
  if( tr_id >= TRID_OFFSET_WBT )
  {
    GF_ASSERT( 0 );
  }
  if( tr_id >= TRID_OFFSET_INFINITY_DUNGEON )
  {
    return EX_TRTYPE_INFINITY_DUNGEON;
  }

  return EX_TRTYPE_NONE;
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

