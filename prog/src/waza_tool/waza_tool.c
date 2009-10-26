//============================================================================================
/**
 * @file  waza_tool.c
 * @bfief 技パラメータツール群（WB改訂版）
 * @author  HisashiSogabe
 * @date  08.11.13
 */
//============================================================================================
#include  <gflib.h>

#include  "arc_def.h"

#include  "waza_tool/waza_tool.h"
#include  "waza_tool/wazano_def.h"
#include  "waza_tool/wazadata.h"

#include  "waza_tool_def.h"

/*
typedef struct{
  u16 monsno;
  s16 hp;

  u16 hpmax;
  u8  eqp;
  u8  atc;

  u32 condition;

  u8  speabino;
  u8  sex;
  u8  type1;
  u8  type2;

}WAZA_DAMAGE_CALC_PARAM;
*/

//============================================================================================
/**
 * プロトタイプ宣言
 */
//============================================================================================
void    WT_WazaTableDataGet( void *buffer );
ARCHANDLE *WT_WazaDataParaHandleOpen( HEAPID heapID );
void    WT_WazaDataParaHandleClose( ARCHANDLE *handle );
u32     WT_WazaDataParaGet( int waza_no, int id );
u32     WT_WazaDataParaGetHandle( ARCHANDLE *handle, int waza_no, int id );
u8      WT_PPMaxGet( u16 wazano, u8 maxupcnt );
u32     WT_WazaDataParaGetAct( WAZA_TABLE_DATA *wtd, int id );

static  void  WT_WazaDataGet( int waza_no, WAZA_TABLE_DATA *wtd );

//============================================================================================
/**
 *  技データテーブルデータをAllocしたメモリに展開
 *
 * @param[in] heapID  データを展開するメモリヒープID
 */
//============================================================================================
void  WT_WazaTableDataGet( void *buffer )
{
  GFL_ARC_LoadDataOfs( buffer, ARCID_WAZA_TBL, 0, 0, sizeof( WAZA_TABLE_DATA ) * ( WAZANO_MAX + 1 ) );
}

//============================================================================================
/**
 *  技データテーブルファイルハンドルオープン
 *
 * @param[in] heapID  ハンドルをAllocするヒープID
 */
//============================================================================================
ARCHANDLE *WT_WazaDataParaHandleOpen( HEAPID heapID )
{
  return GFL_ARC_OpenDataHandle( ARCID_WAZA_TBL, heapID );
}

//============================================================================================
/**
 *  技データテーブルファイルハンドルクローズ
 *
 * @param[in] handle  クローズするHandle
 */
//============================================================================================
void  WT_WazaDataParaHandleClose( ARCHANDLE *handle )
{
  GFL_ARC_CloseDataHandle( handle );
}

//============================================================================================
/**
 *  技データテーブルから値を取得
 *
 * @param[in] waza_no   取得したい技ナンバー
 * @param[in] id      取得したい値の種類
 */
//============================================================================================
u32 WT_WazaDataParaGet( int waza_no, int id )
{
  WAZA_TABLE_DATA wtd;

  WT_WazaDataGet( waza_no, &wtd );

  return  WT_WazaDataParaGetAct( &wtd, id );
}

//============================================================================================
/**
 *  技データテーブルから値を取得（ファイルハンドルを使用する）
 *
 * @param[in] waza_no   取得したい技ナンバー
 * @param[in] id      取得したい値の種類
 */
//============================================================================================
u32 WT_WazaDataParaGetHandle( ARCHANDLE *handle, int waza_no, int id )
{
  WAZA_TABLE_DATA wtd;

  GFL_ARC_LoadDataByHandle( handle, waza_no, &wtd );

  return  WT_WazaDataParaGetAct( &wtd, id );
}

//============================================================================================
/**
 *  PPMaxを取得
 *
 * @param[in] wazano    取得する技ナンバー
 * @param[in] maxupcnt  マックスアップを使用した回数
 *
 * @retval  PPMAX
 */
//============================================================================================
u8  WT_PPMaxGet( u16 wazano, u8 maxupcnt )
{
  u8  ppmax;

  if( maxupcnt > 3 ){
    maxupcnt = 3;
  }

  ppmax = WT_WazaDataParaGet( wazano, ID_WTD_pp );
  ppmax += ( ppmax * 20 * maxupcnt ) / 100;

  return  ppmax;
}

//============================================================================================
/**
 *  技テーブル構造体データから値を取得
 *
 * @param[in] wtd   技テーブル構造体データへのポインタ
 * @param[in] id    取得したい技データのIndex
 */
//============================================================================================
u32 WT_WazaDataParaGetAct( WAZA_TABLE_DATA *wtd, int id )
{
  u32 ret;

  switch( id ){
  case ID_WTD_battleeffect:
    ret = wtd->battleeffect;
    break;
  case ID_WTD_kind:
    ret = wtd->kind;
    break;
  case ID_WTD_damage:
    ret = wtd->damage;
    break;
  case ID_WTD_wazatype:
    ret = wtd->wazatype;
    break;
  case ID_WTD_hitprobability:
    ret = wtd->hitprobability;
    break;
  case ID_WTD_pp:
    ret = wtd->pp;
    break;
  case ID_WTD_addeffect:
    ret = wtd->addeffect;
    break;
  case ID_WTD_attackrange:
    ret = wtd->attackrange;
    break;
  case ID_WTD_attackpri:
    ret = wtd->attackpri;
    break;
  case ID_WTD_flag:
    ret = wtd->flag;
    break;
  case ID_WTD_ap_no:        //コンテスト用パラメータ　アピールNo
    ret = wtd->condata.ap_no;
    break;
  case ID_WTD_contype:      //コンテスト用パラメータ　コンテスト技タイプ
    ret = wtd->condata.contype;
    break;
  }
  return ret;
}

//============================================================================================
/**
 *              外部公開しない関数郡
 */
//============================================================================================
//============================================================================================
/**
 *  技テーブル構造体データを取得
 *
 * @param[in] waza_no   取得したい技ナンバー
 * @param[out]  wtd     取得した技データの格納先を指定
 */
//============================================================================================
static  void  WT_WazaDataGet( int waza_no, WAZA_TABLE_DATA *wtd )
{
  GFL_ARC_LoadData( wtd, ARCID_WAZA_TBL, waza_no );
}



//=============================================================================================
/**
 * @file  wazadata.c
 * @brief ポケモンWB  ワザデータアクセサ
 * @author  taya
 *
 * @date  2008.10.14  作成
 */
//=============================================================================================

#include "waza_tool/wazadata.h"


enum {
  HITRATIO_MUST = 101,
};

//------------------------------------------------------------------------------
/**
 * 定数宣言
 */
//------------------------------------------------------------------------------
enum {
  WSEQ_SINMPLESICK_NEMURI = 1,
  WSEQ_SINMPLESICK_DOKU = 66,
  WSEQ_SINMPLESICK_MAHI = 67,
  WSEQ_SINMPLESICK_MOUDOKU = 33,
  WSEQ_DAMAGESICK_DOKU = 2,
  WSEQ_DAMAGESICK_YAKEDO = 4,
  WSEQ_DAMAGESICK_KOORI = 5,
  WSEQ_DAMAGESICK_MAHI = 6,
  WSEQ_DAMAGESICK_KONRAN = 76,

  WSEQ_SIMPLEEFF_ATK = 10,
  WSEQ_SIMPLEEFF_DEF = 11,
  WSEQ_SIMPLEEFF_SPATK = 13,
  WSEQ_SIMPLEEFF_AVOID = 16,
  WSEQ_SIMPLEEFF_DOWN_ATK = 18,
  WSEQ_SIMPLEEFF_DOWN_DEF = 19,
  WSEQ_SIMPLEEFF_DOWN_AGI = 20,
  WSEQ_SIMPLEEFF_DOWN_HIT = 23,
  WSEQ_SIMPLEEFF_DOWN_AVOID = 24,

  WSEQ_SIMPLEEFFEX_ATK = 50,
  WSEQ_SIMPLEEFFEX_DEF = 51,
  WSEQ_SIMPLEEFFEX_AGI = 52,
  WSEQ_SIMPLEEFFEX_SPATK = 53,
  WSEQ_SIMPLEEFFEX_SPDEF = 54,
  WSEQ_SIMPLEEFFEX_DOWN_ATK = 58,
  WSEQ_SIMPLEEFFEX_DOWN_DEF = 59,
  WSEQ_SIMPLEEFFEX_DOWN_AGI = 60,
  WSEQ_SIMPLEEFFEX_DOWN_SPDEF = 62,
  WSEQ_SIMPLEEFFCOMP_ATK_DEF = 208,
  WSEQ_SIMPLEEFFCOMP_DEF_SPDEF = 206,
  WSEQ_SIMPLEEFFCOMP_SPATK_SPDEF = 211,
  WSEQ_SIMPLEEFFCOMP_ATK_AGI = 212,

  WSEQ_DAMAGE_EFF_ATK = 68,
  WSEQ_DAMAGE_EFF_DEF = 69,
  WSEQ_DAMAGE_EFF_AGI = 70,
  WSEQ_DAMAGE_EFF_SPATK = 71,
  WSEQ_DAMAGE_EFF_SPDEF = 72,
  WSEQ_DAMAGE_EFF_AVOID = 73,
  WSEQ_SHRINK = 31,
  WSEQ_MUST_SHRINK = 158,
  WSEQ_WEATHER_RAIN = 136,
  WSEQ_WEATHER_SHINE = 137,
  WSEQ_WEATHER_STORM = 115,
  WSEQ_WEATHER_SNOW = 164,
  WSEQ_CRITICAL_UP = 43,
  WSEQ_ICHIGEKI = 38,
  WSEQ_EFF_SICK1 = 118,
  WSEQ_EFF_SICK2 = 166,
  WSEQ_SIMPLE_RECOVER = 32,
  WSEQ_SIMPLE_RECOVER2 = 214,
  WSEQ_YUUWAKU = 265,
  WSEQ_HEAT1 = 125,
  WSEQ_HEAT2 = 253,
  WSEQ_MARUKU = 156,
  WSEQ_FUMITUKE = 150,
  WSEQ_AKUMU = 107,
  WSEQ_FURAFURA = 199,
  WSEQ_MEROMERO = 120,
  WSEQ_JUDEN = 268,
  WSEQ_AKUBI = 187,
  WSEQ_HOROBI = 114,
  WSEQ_YADORIGI = 84,
  WSEQ_NOMIKOMU = 162,
  WSEQ_SASIOSAE = 232,
  WSEQ_NEWOHARU = 181,
  WSEQ_POWERTRICK = 238,
  WSEQ_BIND = 42,
  WSEQ_BIND2 = 261,

  WSEQ_REACT_HAKAI = 80,  ///< 次のターンアクションできない

  WSEQ_REACT_A1 = 48,   /// 1/4反動
  WSEQ_REACT_B1 = 198,  /// 1/3反動
  WSEQ_REACT_B2 = 253,  /// 1/3反動（フレアドライブ）
  WSEQ_REACT_B3 = 263,  /// 1/3反動（ボルテッカー）
  WSEQ_REACT_C1 = 269,  /// 1/2反動

  WSEQ_PUSHOUT = 28,
  WSEQ_DRAIN = 3,
  WSEQ_MIYABURU = 113,
  WSEQ_MIRACLEEYE = 216,
  WSEQ_MIRAIYOTI = 148,
};

// @@@ 旧バージョンのワザデータシーケンスを、新バージョンのデータテーブルとして扱うための
// 一時的な変換用テーブル
typedef struct {
  u16 wseq;
  u16 category;

  WazaSick  sick;
  u8        sickCont;
  u8        sickTurnMin;
  u8        sickTurnMax;
  struct {
    WazaRankEffect  type;
    s8              value;
  }rankEff[WAZA_RANKEFF_NUM_MAX];

  u8 shrinkFlg;
  u8 weather;
  u8 reactionRatio;
  u8 criticalRank;

}SEQ_PARAM;


static const SEQ_PARAM* getSeqParam( WazaID waza )
{
  static const SEQ_PARAM SeqParamTable[] = {
    {
      WSEQ_SINMPLESICK_NEMURI, WAZADATA_CATEGORY_SIMPLE_SICK,
      POKESICK_NEMURI, WAZASICK_CONT_TURN, 2, 5,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SINMPLESICK_DOKU, WAZADATA_CATEGORY_SIMPLE_SICK,
      POKESICK_DOKU, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SINMPLESICK_MAHI, WAZADATA_CATEGORY_SIMPLE_SICK,
      POKESICK_MAHI, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGESICK_DOKU, WAZADATA_CATEGORY_DAMAGE_SICK,
      POKESICK_DOKU, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGESICK_YAKEDO, WAZADATA_CATEGORY_DAMAGE_SICK,
      POKESICK_YAKEDO, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGESICK_KOORI, WAZADATA_CATEGORY_DAMAGE_SICK,
      POKESICK_KOORI, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGESICK_MAHI, WAZADATA_CATEGORY_DAMAGE_SICK,
      POKESICK_MAHI, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_ATK, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, 1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_DEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_DEFENCE, 1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_SPATK, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_ATTACK, 1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_AVOID, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_AVOID, 1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_DOWN_ATK, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_DOWN_DEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_DEFENCE, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_DOWN_AGI, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_AGILITY, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_DOWN_HIT, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_HIT, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFF_DOWN_AVOID, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_AVOID, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_ATK, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, 2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_DEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_DEFENCE, 2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_AGI, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_AGILITY, 2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_SPATK, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_ATTACK, 2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_SPDEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_DEFENCE, 2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFCOMP_ATK_DEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, 1 }, { WAZA_RANKEFF_DEFENCE, 1 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFCOMP_DEF_SPDEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_DEFENCE, 1 }, { WAZA_RANKEFF_SP_DEFENCE, 1 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFCOMP_SPATK_SPDEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_ATTACK, 1 }, { WAZA_RANKEFF_SP_DEFENCE, 1 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFCOMP_ATK_AGI, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, 1 }, { WAZA_RANKEFF_AGILITY, 1 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_DOWN_ATK, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, -2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_DOWN_DEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_DEFENCE, -2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_DOWN_AGI, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_AGILITY, -2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_SIMPLEEFFEX_DOWN_SPDEF, WAZADATA_CATEGORY_SIMPLE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_DEFENCE, -2 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGE_EFF_ATK, WAZADATA_CATEGORY_DAMAGE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_ATTACK, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGE_EFF_DEF, WAZADATA_CATEGORY_DAMAGE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_DEFENCE, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGE_EFF_SPATK, WAZADATA_CATEGORY_DAMAGE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_ATTACK, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGE_EFF_SPDEF, WAZADATA_CATEGORY_DAMAGE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_SP_DEFENCE, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
      WSEQ_DAMAGE_EFF_AVOID, WAZADATA_CATEGORY_DAMAGE_EFFECT,
      POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_AVOID, -1 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_SHRINK, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      TRUE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_MUST_SHRINK, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      TRUE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_WEATHER_RAIN, WAZADATA_CATEGORY_WEATHER,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_RAIN, 0, 0,
    },{
        WSEQ_WEATHER_SHINE, WAZADATA_CATEGORY_WEATHER,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_SHINE, 0, 0,
    },{
        WSEQ_WEATHER_STORM, WAZADATA_CATEGORY_WEATHER,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_SAND, 0, 0,
    },{
        WSEQ_WEATHER_SNOW, WAZADATA_CATEGORY_WEATHER,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_SNOW, 0, 0,
    },{
        WSEQ_REACT_A1, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 25, 0,
    },{
        WSEQ_REACT_B1, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 33, 0,
    },{
        WSEQ_REACT_B2, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 33, 0,
    },{
        WSEQ_REACT_B3, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 33, 0,
    },{
        WSEQ_REACT_C1, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 50, 0,
    },{
        WSEQ_REACT_HAKAI, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_CRITICAL_UP, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 1,
    },{
        WSEQ_ICHIGEKI, WAZADATA_CATEGORY_ICHIGEKI,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_PUSHOUT, WAZADATA_CATEGORY_PUSHOUT,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_DRAIN, WAZADATA_CATEGORY_DRAIN,
        POKESICK_NULL, WAZASICK_CONT_PERMANENT, 0, 0,
      { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
      FALSE, BTL_WEATHER_NONE, 50, 0,
    },{
        WSEQ_EFF_SICK1, WAZADATA_CATEGORY_EFFECT_SICK,
        WAZASICK_KONRAN, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_ATTACK, 2 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_EFF_SICK2, WAZADATA_CATEGORY_EFFECT_SICK,
        WAZASICK_KONRAN, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_SP_ATTACK, 1 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_SIMPLE_RECOVER, WAZADATA_CATEGORY_SIMPLE_RECOVER,
        WAZASICK_KONRAN, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_SIMPLE_RECOVER2, WAZADATA_CATEGORY_SIMPLE_RECOVER,
        WAZASICK_KONRAN, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_YUUWAKU, WAZADATA_CATEGORY_SIMPLE_EFFECT,
        WAZASICK_NULL, 0, 0, 0,
        { { WAZA_RANKEFF_SP_ATTACK, -2 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_HEAT1, WAZADATA_CATEGORY_DAMAGE_SICK,
        WAZASICK_YAKEDO, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_HEAT2, WAZADATA_CATEGORY_DAMAGE_SICK,
        WAZASICK_YAKEDO, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 33, 0,
    },{
        WSEQ_MARUKU, WAZADATA_CATEGORY_SIMPLE_EFFECT,
        WAZASICK_NULL, 0, 0, 0,
        { { WAZA_RANKEFF_DEFENCE, 1 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_FUMITUKE, WAZADATA_CATEGORY_SIMPLE_DAMAGE,
        WAZASICK_NULL, 0, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        TRUE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_AKUMU, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_AKUMU, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_FURAFURA, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_KONRAN, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_MEROMERO, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_MEROMERO, WAZASICK_CONT_POKE, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_JUDEN, WAZADATA_CATEGORY_SIMPLE_EFFECT,
        WAZASICK_NULL, 0, 0, 0,
        { { WAZA_RANKEFF_SP_DEFENCE, 1 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_AKUBI, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_AKUBI, WAZASICK_CONT_TURN, 2, 2,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_HOROBI, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_HOROBINOUTA, WAZASICK_CONT_TURN, 4, 4,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_YADORIGI, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_YADORIGI, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_NOMIKOMU, WAZADATA_CATEGORY_SIMPLE_RECOVER,
        WAZASICK_NULL, 0, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_MIYABURU, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_MIYABURU, WAZASICK_CONT_PERMANENT, POKETYPE_GHOST, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_MIRACLEEYE, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_MIYABURU, WAZASICK_CONT_PERMANENT, POKETYPE_AKU, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_MIRAIYOTI, WAZADATA_CATEGORY_OTHERS,
        WAZASICK_NULL, 0, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_SASIOSAE, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_SASIOSAE, WAZASICK_CONT_TURN, 5, 5,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_NEWOHARU, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_NEWOHARU, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_POWERTRICK, WAZADATA_CATEGORY_SIMPLE_SICK,
        WAZASICK_POWERTRICK, WAZASICK_CONT_PERMANENT, 0, 0,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_BIND, WAZADATA_CATEGORY_DAMAGE_SICK,
        WAZASICK_BIND, WAZASICK_CONT_POKETURN, 3, 6,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    },{
        WSEQ_BIND2, WAZADATA_CATEGORY_DAMAGE_SICK,
        WAZASICK_BIND, WAZASICK_CONT_POKETURN, 3, 6,
        { { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
        FALSE, BTL_WEATHER_NONE, 0, 0,
    }

/*
  typedef struct {
    u16 wseq;
    u16 category;

    WazaSick  sick;
    u8        sickCont;
    u8        sickTurnMin;
    u8        sickTurnMax;
    struct {
      WazaRankEffect  type;
      s8              value;
    }rankEff[WAZA_RANKEFF_NUM_MAX];

    u8 shrinkFlg;
    u8 weather;
    u8 reactionRatio;
    u8 criticalRank;

  }SEQ_PARAM;
*/
  };
  u16 seq = WT_WazaDataParaGet( waza, ID_WTD_battleeffect );
  u16 i;

  for(i=0; i<NELEMS(SeqParamTable); ++i)
  {
    if( SeqParamTable[i].wseq == seq )
    {
      return &SeqParamTable[i];
    }
  }
  return NULL;
}

u8 WAZADATA_GetMaxPP( WazaID id, u8 upCnt )
{
  return WT_PPMaxGet( id, upCnt );
}

// ワザ優先度
s8   WAZADATA_GetPriority( WazaID id )
{
  return WT_WazaDataParaGet( id, ID_WTD_attackpri );
}
// ワザカテゴリ
WazaCategory  WAZADATA_GetCategory( WazaID id )
{
  const SEQ_PARAM* seqparam = getSeqParam( id );

  if( seqparam )
  {
    return seqparam->category;
  }

  if( WAZADATA_IsDamage(id) )
  {
    return WAZADATA_CATEGORY_SIMPLE_DAMAGE;
  }
  else
  {
    return WAZADATA_CATEGORY_OTHERS;
  }
}

PokeType WAZADATA_GetType( WazaID id )
{
  return WT_WazaDataParaGet( id, ID_WTD_wazatype );
}

u16 WAZADATA_GetPower( WazaID id )
{
  return WT_WazaDataParaGet( id, ID_WTD_damage );
}

WazaDamageType WAZADATA_GetDamageType( WazaID id )
{
//  return WT_WazaDataParaGet( id, ID_WTD_kind );
  switch( WT_WazaDataParaGet( id, ID_WTD_kind ) ){
  case 0:   return  WAZADATA_DMG_PHYSIC;  ///< 物理ダメージ
  case 1:   return  WAZADATA_DMG_SPECIAL;
  default:  return WAZADATA_DMG_NONE;
  }
}

// 命中率
u8 WAZADATA_GetHitRatio( WazaID id )
{
  u8 ret = WT_WazaDataParaGet( id, ID_WTD_hitprobability );
  TAYA_Printf("[WAZATL] GetHitRatio : waza=%d, ratio=%d\n", id, ret);
  if( ret == 0 )
  {
    ret = 100;
  }
  return ret;
}

// 必中フラグ。つばめがえしなどがTRUEを返す。
BOOL WAZADATA_IsAlwaysHit( WazaID id )
{
  return WAZADATA_GetHitRatio(id) == HITRATIO_MUST;
}

// ダメージワザかどうか
BOOL WAZADATA_IsDamage( WazaID id )
{
  // @@@ いまはてきとー
  return WAZADATA_GetPower(id) != 0;
}

// 最高ヒット回数
u8 WAZADATA_GetMaxHitCount( WazaID id )
{
  return 1;
}

// 接触ワザかどうか
BOOL WAZADATA_IsTouch( WazaID id )
{
  // @@@ いまはてきとー
  return WAZADATA_IsDamage( id );
}




// 効果範囲
WazaTarget WAZADATA_GetTarget( WazaID id )
{
  enum {
    OLD_RANGE_NORMAL    = 0,
    OLD_RANGE_NONE      = 1,
    OLD_RANGE_RANDOM    = 2,
    OLD_RANGE_ENEMY2    = 4,
    OLD_RANGE_OTHER_ALL = 8,
    OLD_RANGE_USER      = 16,
    OLD_RANGE_FRIEND_ALL = 32,
    OLD_RANGE_WEATHER   = 64,
    OLD_RANGE_MAKIBISI  = 128,
    OLD_RANGE_TEDASUKE  = 256,
    OLD_RANGE_TUBO      = 512,
    OLD_RANGE_SAKIDORI  = 1024,
  };

  switch( id ){
  case WAZANO_TEKUSUTYAA2:  return WAZA_TARGET_OTHER_SELECT;
  case WAZANO_KAUNTAA:
  case WAZANO_MIRAAKOOTO:
  case WAZANO_METARUBAASUTO:
     return WAZA_TARGET_UNKNOWN;
  case WAZANO_HOROBINOUTA:
     return WAZA_TARGET_ALL;
  case WAZANO_OUMUGAESI:
     return WAZA_TARGET_OTHER_SELECT;
  default:
    break;
  }

  {
    u32 old_range = WT_WazaDataParaGet( id, ID_WTD_attackrange );

    switch( old_range ){
    case OLD_RANGE_NORMAL:    return WAZA_TARGET_OTHER_SELECT;
    case OLD_RANGE_NONE:      return WAZA_TARGET_UNKNOWN;
    case OLD_RANGE_ENEMY2:    return WAZA_TARGET_ENEMY_ALL;
    case OLD_RANGE_OTHER_ALL: return WAZA_TARGET_OTHER_ALL;
    case OLD_RANGE_USER:      return WAZA_TARGET_USER;
    case OLD_RANGE_FRIEND_ALL:return WAZA_TARGET_SIDE_FRIEND;
    case OLD_RANGE_RANDOM:    return WAZA_TARGET_ENEMY_RANDOM;
    case OLD_RANGE_WEATHER:   return WAZA_TARGET_FIELD;
    case OLD_RANGE_MAKIBISI:  return WAZA_TARGET_SIDE_ENEMY;
    case OLD_RANGE_TEDASUKE:  return WAZA_TARGET_FRIEND_SELECT;
    case OLD_RANGE_TUBO:      return WAZA_TARGET_FRIEND_USER_SELECT;
    case OLD_RANGE_SAKIDORI:  return WAZA_TARGET_ENEMY_SELECT;
    }
  }

  return WAZA_TARGET_OTHER_SELECT;///< 自分以外の１体（選択）
}


// クリティカルランク
u8 WAZADATA_GetCriticalRank( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    return seq->criticalRank;
  }
  return 0;
}

//=============================================================================================
/**
 * 反動率（相手に与えたダメージの何％を反動として自分が受けるか）を返す
 *
 * @param   id
 *
 * @retval  u8    反動率（反動なしなら0, max100）
 */
//=============================================================================================
u8 WAZADATA_GetReactionRatio( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    return seq->reactionRatio;
  }
  return 0;
}
//=============================================================================================
/**
 * ドレイン技のダメージ値->回復HP還元率
 *
 * @param   id
 *
 * @retval  u8
 */
//=============================================================================================
u8 WAZADATA_GetDrainRatio( WazaID id )
{
  // @@@ いまはてきとー
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    return seq->reactionRatio;
  }
  return 0;
}
//=============================================================================================
/**
 * ランク効果の種類数を返す
 *
 * @param   id
 *
 * @retval  u8
 */
//=============================================================================================
u8 WAZADATA_GetRankEffectCount( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    u8 cnt = 0;
    if( seq->rankEff[0].type != WAZA_RANKEFF_NULL ){ ++cnt; }
    if( seq->rankEff[1].type != WAZA_RANKEFF_NULL ){ ++cnt; }
    return cnt;
  }
  return 0;
}
//=============================================================================================
/**
 * ランク効果を取得
 *
 * @param   id        [in] ワザID
 * @param   idx       [in] 何番目のランク効果か？（0～）
 * @param   volume    [out] ランク効果の程度（+ならアップ, -ならダウン, 戻り値==WAZA_RANKEFF_NULLの時のみ0）
 *
 * @retval  WazaRankEffect    ランク効果ID
 */
//=============================================================================================
WazaRankEffect WAZADATA_GetRankEffect( WazaID id, u32 idx, int* volume )
{
  GF_ASSERT(idx < WAZA_RANKEFF_NUM_MAX);
  {
    const SEQ_PARAM* seq = getSeqParam( id );
    if( seq ){
      if( seq->rankEff[idx].type != WAZA_RANKEFF_NULL )
      {
        *volume = seq->rankEff[idx].value;
        return seq->rankEff[idx].type;
      }
    }
    *volume = 0;
    return WAZA_RANKEFF_NULL;
  }
}
//=============================================================================================
/**
 * ランク効果の発生率を返す
 *
 * @param   id    ワザID
 *
 * @retval  u32   発生率（パーセンテージ = 0～100）
 */
//=============================================================================================
u32 WAZADATA_GetRankEffPer( WazaID id )
{
  // @@@ 今はてきとうに
  return WT_WazaDataParaGet( id, ID_WTD_addeffect );
}
//=============================================================================================
/**
 * ワザによって発生する状態異常IDを取得
 *
 * @param   id
 * @param   param   [out] パラメータ取得用構造体ポインタ
 *
 * @retval  WazaSick    状態異常ID（発生しない場合 WAZASICK_NULL）
 */
//=============================================================================================
WazaSick WAZADATA_GetSick( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    return seq->sick;
  }
  return WAZASICK_NULL;
}
//=============================================================================================
/**
 * ワザによって発生する状態異常の継続パラメータを取得
 *
 * @param   id
 *
 * @retval  WAZA_SICKCONT_PARAM
 */
//=============================================================================================
WAZA_SICKCONT_PARAM WAZADATA_GetSickCont( WazaID id )
{
  WAZA_SICKCONT_PARAM  param;

  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    param.type = seq->sickCont;
    param.turnMin = seq->sickTurnMin;;
    param.turnMax = seq->sickTurnMin;
  }else{
    GF_ASSERT(0);
    param.type = 0;
    param.turnMin = 0;
    param.turnMax = 0;
  }
  return param;
}
//=============================================================================================
/**
 * 追加の状態異常発生率を返す
 *
 * @param   id    ワザID
 *
 * @retval  u32   発生率（パーセンテージ = 0～100）
 */
//=============================================================================================
u32 WAZADATA_GetSickPer( WazaID id )
{
  // @@@ 今はてきとうに

  const SEQ_PARAM* seq = getSeqParam( id );
  if( (seq != NULL)
  &&  (seq->wseq == WSEQ_BIND) || (seq->wseq == WSEQ_BIND2)
  ){
    return 100;
  }

  return WT_WazaDataParaGet( id, ID_WTD_addeffect );
}
//=============================================================================================
/**
 * 追加の「ひるみ」率を返す
 *
 * @param   id    ワザID
 *
 * @retval  u32   発生率（パーセンテージ = 0～100）
 */
//=============================================================================================
u32 WAZADATA_GetShrinkPer( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq && seq->shrinkFlg ){
    return WT_WazaDataParaGet( id, ID_WTD_addeffect );
  }
  return 0;
}
//=============================================================================================
/**
 * ワザ使用後、つかれて１ターン動けなくなってしまうかチェック
 *
 * @param   id
 *
 * @retval  BOOL    動けなくなってしまうならTRUE
 */
//=============================================================================================
BOOL WAZADATA_IsTire( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq && (seq->wseq == WSEQ_REACT_HAKAI) ){
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * 天候効果を返す
 *
 * @param   id
 *
 * @retval  BtlWeather
 */
//=============================================================================================
BtlWeather WAZADATA_GetWeather( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq ){
    return seq->weather;
  }
  return BTL_WEATHER_NONE;
}
//=============================================================================================
/**
 * ワザ使用によるHP回復率（％）を取得
 *
 * @param   id
 *
 * @retval  u8
 */
//=============================================================================================
u8 WAZADATA_GetRecoverHPRatio( WazaID id )
{
  const SEQ_PARAM* seq = getSeqParam( id );
  if( seq && (seq->wseq == WSEQ_SIMPLE_RECOVER) ){
    return 50;
  }
  return 0;
}
//=============================================================================================
/**
 * ワザイメージチェック
 *
 * @param   id    ワザID
 * @param   img   イメージID
 *
 * @retval  BOOL    ワザが指定されたイメージに属する場合はTRUE
 */
//=============================================================================================
BOOL WAZADATA_IsImage( WazaID id, WazaImage img )
{
  // @@@ 今はてきとー
  switch( img ){
  case WAZA_IMG_HEAT:
    if( id == WAZANO_SEINARUHONOO ){ return TRUE; }
    if( id == WAZANO_KAENGURUMA ){ return TRUE; }
    if( id == WAZANO_HUREADORAIBU ){ return TRUE; }
    break;

  case WAZA_IMG_PUNCH:
    if( id == WAZANO_MEGATONPANTI) { return TRUE; }
    if( id == WAZANO_DOREINPANTI) { return TRUE; }
    if( id == WAZANO_HONOONOPANTI) { return TRUE; }
    if( id == WAZANO_REITOUPANTI) { return TRUE; }
    if( id == WAZANO_KAMINARIPANTI) { return TRUE; }
    if( id == WAZANO_SYADOOPANTI) { return TRUE; }
    if( id == WAZANO_RENZOKUPANTI) { return TRUE; }
    if( id == WAZANO_PIYOPIYOPANTI) { return TRUE; }
    if( id == WAZANO_BAKURETUPANTI) { return TRUE; }
    if( id == WAZANO_MAHHAPANTI) { return TRUE; }
    if( id == WAZANO_BARETTOPANTI) { return TRUE; }
    if( id == WAZANO_KOMETTOPANTI) { return TRUE; }
    if( id == WAZANO_SUKAIAPPAA) { return TRUE; }
    break;

  case WAZA_IMG_SOUND:
    if( id == WAZANO_HOROBINOUTA ){ return TRUE; }
    break;
  }

  return FALSE;
}
//=============================================================================================
/**
 * ワザフラグ取得
 *
 * @param   id
 * @param   flag
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL WAZADATA_GetFlag( WazaID id, WazaFlag flag )
{
  GF_ASSERT( flag < WAZAFLAG_MAX );

  // @@@ 今はてきとー
  switch( flag ){
  case WAZAFLAG_MAMORU:
    return WAZADATA_IsDamage( id );

  case WAZAFLAG_TAME:
    if( id == WAZANO_KAMAITATI ){ return TRUE; }
    if( id == WAZANO_ROKETTOZUTUKI ){ return TRUE; }
    if( id == WAZANO_SORAWOTOBU ){ return TRUE; }
    if( id == WAZANO_SOORAABIIMU ){ return TRUE; }
    if( id == WAZANO_DAIBINGU ){ return TRUE; }
    if( id == WAZANO_ANAWOHORU ){ return TRUE; }
    if( id == WAZANO_GODDOBAADO ){ return TRUE; }
    if( id == WAZANO_TOBIHANERU ){ return TRUE; }
    if( id == WAZANO_SYADOODAIBU ){ return TRUE; }
    return FALSE;

  default:
    break;

  }
  return FALSE;
}



#ifdef PM_DEBUG
void WAZADATA_PrintDebug( void )
{
  static const struct {
    const char* name;
    u16 id;
  }tbl[] = {
    { "みずでっぽう（通常）",        WAZANO_MIZUDEPPOU },
    { "ねっぷう（相手２体）",        WAZANO_NEPPUU },
    { "なみのり（３体）",            WAZANO_NAMINORI },
    { "かげぶんしん（自分）",        WAZANO_KAGEBUNSIN },
    { "くろいきり（場）",            WAZANO_KUROIKIRI },
    { "げきりん（ランダム）",        WAZANO_KUROIKIRI },
    { "あまごい（場）",              WAZANO_AMAGOI },
    { "あばれる（ランダム）",        WAZANO_ABARERU },
    { "アロマセラピー（味方２体）",  WAZANO_AROMASERAPII },
    { "まきびし（まきびし）",        WAZANO_MAKIBISI },
    { "てだすけ（てだすけ）",        WAZANO_TEDASUKE },
    { "つぼつく（つぼつく）",        WAZANO_TUBOWOTUKU },
    { "さきどり（さきどり）",        WAZANO_SAKIDORI },
    { "オウムがえし（なし）",        WAZANO_OUMUGAESI },
  };
  u32 i, range;

  OS_TPrintf("***** ワザ効果範囲値チェック *****\n");
  for(i=0; i<NELEMS(tbl); ++i)
  {
    range = WT_WazaDataParaGet( tbl[i].id, ID_WTD_attackrange );
    OS_TPrintf("%s\t=%d\n", tbl[i].name, range);

  }
}

#endif
