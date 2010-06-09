//=============================================================================================
/**
 * @file  btl_tables.c
 * @brief ポケモンWB てきとうなサイズのテーブルをVRAM_Hに置いてアクセスする
 * @author  taya
 *
 * @date  2010.05.27  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"
#include "poke_tool\monsno_def.h"
#include "waza_tool\wazano_def.h"
#include "item\itemsym.h"
#include "item\item.h"
#include "battle\battle_bg_def.h"

#include "btl_common.h"
#include "btl_tables.h"

//----------------------------------------------------------------------------------
/**
 * テーブル要素に該当値が存在するか判定
 *
 * @param   value       判定する値
 * @param   table       テーブルアドレス
 * @param   tableElems  テーブル要素数
 *
 * @retval  BOOL    該当値がテーブルに含まれたらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL checkTableElems( u16 value, const u16* table, u32 tableElems )
{
  u32 i;
  for(i=0; i<tableElems; ++i)
  {
    if( table[i] == value ){
      return TRUE;
    }
  }
  return FALSE;
}

/**
 *  アンコール除外対象のワザか判定
 */
BOOL BTL_TABLES_IsMatchEncoreFail( WazaID waza )
{
  static const u16 table[] = {
    WAZANO_NULL,    WAZANO_ANKOORU,  WAZANO_OUMUGAESI,
    WAZANO_HENSIN,  WAZANO_MONOMANE, WAZANO_SUKETTI,
  };

  return checkTableElems( waza, table, NELEMS(table) );
}
/**
 *  ものまね失敗対象のワザ判定
 */
BOOL BTL_TABLES_IsMatchMonomaneFail( WazaID waza )
{
  static const u16 table[] = {
    WAZANO_NULL,    WAZANO_SUKETTI,   WAZANO_MONOMANE,
    WAZANO_HENSIN,  WAZANO_WARUAGAKI, WAZANO_YUBIWOHURU,
    WAZANO_OSYABERI
  };

  return checkTableElems( waza, table, NELEMS(table) );
}
/**
 *  さきどり失敗ワザ判定
 */
BOOL BTL_TABLES_IsSakidoriFailWaza( WazaID waza )
{
  static const u16 table[] = {
    WAZANO_NULL, WAZANO_MIRAAKOOTO, WAZANO_KAUNTAA, WAZANO_METARUBAASUTO,
    WAZANO_KIAIPANTI, WAZANO_SAKIDORI,
  };

  return checkTableElems( waza, table, NELEMS(table) );
}

/**
 *  みちづれ失敗ワザ判定
 */
BOOL BTL_TABLES_IsMitidureFailWaza( WazaID waza )
{
  static const u16 table[] = {
    WAZANO_NULL, WAZANO_MIRAIYOTI, WAZANO_HAMETUNONEGAI,
  };

  return checkTableElems( waza, table, NELEMS(table) );
}

/**
 *  プレッシャー対象ワザ判定
 */
BOOL BTL_TABLES_IsPressureEffectiveWaza( WazaID waza )
{
  static const u16 table[] = {
    WAZANO_YOKODORI, WAZANO_HUUIN,
    WAZANO_MAKIBISI, WAZANO_DOKUBISI, WAZANO_SUTERUSUROKKU,

  };

  return checkTableElems( waza, table, NELEMS(table) );
}

/**
 *  アイテムコールで発動してはいけないアイテムか判定
 */
BOOL BTL_TABLES_CheckItemCallNoEffect( u16 itemID )
{
  static const u16 table[] = {
    ITEM_KIAINOHATIMAKI,  ITEM_PAWAHURUHAABU,   ITEM_DASSYUTUBOTAN,
    ITEM_ZYAPONOMI,       ITEM_RENBUNOMI,       ITEM_IBANNOMI,
    ITEM_SENSEINOTUME,    ITEM_KYUUKON,         ITEM_ZYUUDENTI,
  };

  return checkTableElems( itemID, table, NELEMS(table) );
}


/**
 *  使用する対象ポケモンの選択を行う必要が無いアイテムか判別
 */
BOOL BTL_TABLES_IsNoTargetItem( u16 itemID )
{
  static const u16 table[] = {
    ITEM_ENEKONOSIPPO,  ITEM_PIPPININGYOU, ITEM_POKEZYARASI,
  };

  return checkTableElems( itemID, table, NELEMS(table) );
}

/**
 *  「なりきり」に失敗するとくせい判定
 */
BOOL BTL_TABLES_CheckNarikiriFailTokusei( u16 tokuseiID )
{
  static const u16 table[] = {
    POKETOKUSEI_TOREESU,       POKETOKUSEI_TENKIYA,         POKETOKUSEI_MARUTITAIPU,
    POKETOKUSEI_FURAWAAGIFUTO, POKETOKUSEI_DARUMAMOODO,     POKETOKUSEI_IRYUUJON,
    POKETOKUSEI_KAWARIMONO,    POKETOKUSEI_FUSIGINAMAMORI,
  };

  if( BTL_TABLES_IsNeverChangeTokusei(tokuseiID) ){
    return TRUE;
  }

  return checkTableElems( tokuseiID, table, NELEMS(table) );
}

/**
 *  「なかまづくり」に失敗するとくせい判定
 */

BOOL BTL_TABLES_CheckNakamaDukuriFailTokusei( u16 tokuseiID )
{
  // 「なりきり」と一緒
  return BTL_TABLES_CheckTraceFailTokusei( tokuseiID );
}

/**
 *  トレースできないとくせい判定
 */
BOOL BTL_TABLES_CheckTraceFailTokusei( u16 tokuseiID )
{
  // トレースの場合は「ふしぎなまもり」を許可する
  if( tokuseiID == POKETOKUSEI_FUSIGINAMAMORI ){
    return FALSE;
  }

  // あとは「なりきり」と一緒
  return BTL_TABLES_CheckNarikiriFailTokusei( tokuseiID );
}

/**
 *  スキルスワップできないとくせい判定（相手・自分のどちらか一方でも該当したらダメ）
 */
BOOL BTL_TABLES_CheckSkillSwapFailTokusei( u16 tokuseiID )
{
  static const u16 table[] = {
    POKETOKUSEI_FUSIGINAMAMORI, POKETOKUSEI_MARUTITAIPU,
  };

  if( BTL_TABLES_IsNeverChangeTokusei(tokuseiID) ){
    return TRUE;
  }

  return checkTableElems( tokuseiID, table, NELEMS(table) );
}

/**
 *  絶対に書き換わってはいけないとくせい判定
 */
BOOL BTL_TABLES_IsNeverChangeTokusei( u16 tokuseiID )
{
  // 今んとこマルチタイプだけ
  return ( tokuseiID == POKETOKUSEI_MARUTITAIPU );
}


/**
 *  アルセウスのプレート判定
 */
BOOL BTL_TABLES_IsMatchAruseusPlate( u16 itemID )
{
  static const u16 table[] = {
    ITEM_HINOTAMAPUREETO,
    ITEM_SIZUKUPUREETO,
    ITEM_IKAZUTIPUREETO,
    ITEM_MIDORINOPUREETO,
    ITEM_TURARANOPUREETO,
    ITEM_KOBUSINOPUREETO,
    ITEM_MOUDOKUPUREETO,
    ITEM_DAITINOPUREETO,
    ITEM_AOZORAPUREETO,
    ITEM_HUSIGINOPUREETO,
    ITEM_TAMAMUSIPUREETO,
    ITEM_GANSEKIPUREETO,
    ITEM_MONONOKEPUREETO,
    ITEM_RYUUNOPUREETO,
    ITEM_KOWAMOTEPUREETO,
    ITEM_KOUTETUPUREETO,
  };

  return checkTableElems( itemID, table, NELEMS(table) );
}

/**
 *  インセクタのカセット判定
 */
BOOL BTL_TABLES_IsMatchInsectaCasette( u16 itemID )
{
  static const u16 table[] = {
    ITEM_AKUAKASETTO,
    ITEM_INAZUMAKASETTO,
    ITEM_BUREIZUKASETTO,
    ITEM_HURIIZUKASETTO,
  };

  return checkTableElems( itemID, table, NELEMS(table) );
}


/**
 *  BGタイプから屋外かどうか判定（ダークボール用）
 */
BOOL BTL_TABLES_IsOutdoorBGType( u16 bgType )
{
  static const u16 table[] = {
    BATTLE_BG_TYPE_GRASS,         ///< 草むら
    BATTLE_BG_TYPE_GRASS_SEASON,  ///< 草むら(四季有り)
    BATTLE_BG_TYPE_CITY,          ///< 街
    BATTLE_BG_TYPE_CITY_SEASON,   ///< 街(四季有り)
    BATTLE_BG_TYPE_CAVE,          ///< 洞窟
    BATTLE_BG_TYPE_CAVE_DARK,     ///< 洞窟(暗い)
    BATTLE_BG_TYPE_FOREST,        ///< 森
    BATTLE_BG_TYPE_MOUNTAIN,      ///< 山
    BATTLE_BG_TYPE_SEA,           ///< 海
    BATTLE_BG_TYPE_SAND,          ///< 砂漠
  };
  return checkTableElems( bgType, table, NELEMS(table) );
}

/**
 *  かたやぶりが無効化する対象のとくせい判定
 */
BOOL BTL_TABLES_IsMatchKatayaburiTarget( u16 tokuseiID )
{
  static const u16 table[] = {
    POKETOKUSEI_FUSIGINAMAMORI, POKETOKUSEI_BOUON,        POKETOKUSEI_FUYUU,
    POKETOKUSEI_SUNAGAKURE,     POKETOKUSEI_YUKIGAKURE,   POKETOKUSEI_TYOSUI,
    POKETOKUSEI_KABUTOAAMAA,    POKETOKUSEI_HIRAISIN,     POKETOKUSEI_YOBIMIZU,
    POKETOKUSEI_SHERUAAMAA,     POKETOKUSEI_TENNEN,       POKETOKUSEI_HIRAISIN,
    POKETOKUSEI_YOBIMIZU,       POKETOKUSEI_KYUUBAN,      POKETOKUSEI_TANJUN,
    POKETOKUSEI_TIDORIASI,      POKETOKUSEI_HAADOROKKU,   POKETOKUSEI_FIRUTAA,
    POKETOKUSEI_MORAIBI,        POKETOKUSEI_DENKIENJIN,   POKETOKUSEI_NENCHAKU,
    POKETOKUSEI_FUSIGINAUROKO,  POKETOKUSEI_ATUISIBOU,    POKETOKUSEI_TAINETU,
    POKETOKUSEI_SIROIKEMURI,    POKETOKUSEI_KURIABODY,    POKETOKUSEI_SURUDOIME,
    POKETOKUSEI_KAIRIKIBASAMI,  POKETOKUSEI_SEISINRYOKU,  POKETOKUSEI_RINPUN,
    POKETOKUSEI_GANJOU,         POKETOKUSEI_SIMERIKE,     POKETOKUSEI_DONKAN,
    POKETOKUSEI_JUUNAN,         POKETOKUSEI_MAIPEESU,     POKETOKUSEI_MIZUNOBEERU,
    POKETOKUSEI_RIIFUGAADO,     POKETOKUSEI_FUMIN,        POKETOKUSEI_HATOMUNE,
    POKETOKUSEI_YARUKI,         POKETOKUSEI_MENEKI,       POKETOKUSEI_MAGUMANOYOROI,
    POKETOKUSEI_AMANOJAKU,      POKETOKUSEI_FURENDOGAADO, POKETOKUSEI_MARUTISUKEIRU,
    POKETOKUSEI_TEREPASII,      POKETOKUSEI_HATOMUNE,     POKETOKUSEI_MIRAKURUSUKIN,
    POKETOKUSEI_MAJIKKUMIRAA,   POKETOKUSEI_SOUSYOKU,     POKETOKUSEI_TIKUDEN,
    POKETOKUSEI_KANSOUHADA,     POKETOKUSEI_FURAWAAGIFUTO,
  };

  return checkTableElems( tokuseiID, table, NELEMS(table) );

}

//=============================================================================================
/**
 * メンタルハーブで治す状態異常テーブル順番アクセス
 *
 * @param   idx   0～
 *
 * @retval  WazaSick    有効Idxの時、状態異常コード／それ以外 WAZASICK_NULL
 */
//=============================================================================================
WazaSick  BTL_TABLES_GetMentalSickID( u32 idx )
{
  static const WazaSick table[] = {
    WAZASICK_MEROMERO,      WAZASICK_ICHAMON,   WAZASICK_KANASIBARI,
    WAZASICK_KAIHUKUHUUJI,  WAZASICK_ENCORE,    WAZASICK_TYOUHATSU,
  };

  if( idx < NELEMS(table) ){
    return table[idx];
  }
  return WAZASICK_NULL;
}



//--------------------------------------------------------------------------
/**
 *  ゆびをふるで出ないワザテーブル
 */
//--------------------------------------------------------------------------
static const WazaID YubiFuruOmmit[] = {
  WAZANO_YUBIWOHURU,    WAZANO_NEGOTO,      WAZANO_NEKONOTE,
  WAZANO_MANEKKO,       WAZANO_SAKIDORI,    WAZANO_OUMUGAESI,
  WAZANO_SIZENNOTIKARA, WAZANO_OSYABERI,    WAZANO_WARUAGAKI,

  WAZANO_SUKETTI,       WAZANO_MONOMANE,    WAZANO_KAUNTAA,
  WAZANO_MIRAAKOOTO,    WAZANO_MAMORU,      WAZANO_MIKIRI,
  WAZANO_KORAERU,       WAZANO_MITIDURE,    WAZANO_KONOYUBITOMARE,
  WAZANO_YOKODORI,      WAZANO_TEDASUKE,    WAZANO_DOROBOU,
  WAZANO_HOSIGARU,      WAZANO_TORIKKU,     WAZANO_KIAIPANTI,
  WAZANO_FEINTO,
};

//=============================================================================================
/**
 * ゆびをふる出ないワザテーブル情報取得
 *
 * @param   elems   [out] テーブル要素数
 *
 * @retval  const WazaID*   テーブルアドレス
 */
//=============================================================================================
const WazaID* BTL_TABLES_GetYubiFuruOmmitTable( u32* elems )
{
  *elems = NELEMS(YubiFuruOmmit);
  return YubiFuruOmmit;
}
//=============================================================================================
/**
 * ゆびをふるで出ないワザか判定
 *
 * @param   waza
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_TABLES_IsYubiFuruOmmit( WazaID waza )
{
  u32 i;
  for(i=0; i<NELEMS(YubiFuruOmmit); ++i)
  {
    if( YubiFuruOmmit[i] == waza ){
      return TRUE;
    }
  }
  return FALSE;
}

/*--------------------------------------------------------------------------------------------*/
/* デバッグ用「ゆびをふる」コントロール                                                       */
/*--------------------------------------------------------------------------------------------*/

#ifdef PM_DEBUG

/**
 *  デバッグ「ゆびをふるテーブル（グローバル）」
 */
int GYubiFuruDebugNumber[ BTL_POS_MAX ];

/**
 *  「ゆびをふるテーブル」リセット
 */
void BTL_TABLES_YubifuruDebugReset( void )
{
  u32 i;
  for(i=0; i<NELEMS(GYubiFuruDebugNumber); ++i){
    GYubiFuruDebugNumber[i] = 0;
  }
}
/**
 *  外部から適当に操作した「ゆびをふるテーブル」に禁止ワザが設定されていたら補正
 */
void BTL_TABLES_YubifuruDebugSetEnd( void )
{
  u32 i;
  for(i=0; i<NELEMS(GYubiFuruDebugNumber); ++i){
    if( BTL_TABLES_IsYubiFuruOmmit(GYubiFuruDebugNumber[i]) )
    {
      BTL_TABLES_YubifuruDebugInc(i);
    }
  }
}
/**
 *  「ゆびをふるテーブル」のナンバーをインクリメント（禁止ワザはスキップ）
 */
void BTL_TABLES_YubifuruDebugInc( u8 pos )
{
  while(1)
  {
    GYubiFuruDebugNumber[pos]++;
    if( GYubiFuruDebugNumber[pos] >= WAZANO_MAX )
    {
      GYubiFuruDebugNumber[pos] = 0;
    }
    if( !BTL_TABLES_IsYubiFuruOmmit( GYubiFuruDebugNumber[pos] ) ){
      break;
    }
  };
}

#endif
