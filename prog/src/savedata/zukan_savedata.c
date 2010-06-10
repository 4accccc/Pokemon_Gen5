//============================================================================================
/**
 * @file		zukan_savedata.c
 * @brief		図鑑セーブデータアクセス処理群ソース
 * @author	mori / tamada GAME FREAK inc.
 * @date		2009.10.26
 *
 * ジョウト→イッシュ→？とその度に名前を書き換えるのもなんなので、
 * 全国は「ZENKOKU」、地方図鑑の事は「LOCAL」と記述する事にします。
 * コメントでも「全国図鑑」「地方図鑑」と記述します。
 *
 */
//============================================================================================
#include <gflib.h>

#include "pm_version.h"
#include "arc_def.h"
#include "system/gfl_use.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "gamesystem/game_data.h"
#include "savedata/zukan_savedata.h"
#include "poke_tool/poke_personal.h"

#include "zukan_data.naix"


//============================================================================================
//	定数定義
//============================================================================================
#include "zukan_savedata_local.h"   //自分状態データ型やPOKEZUKAN_ARRAY_LENなど内部定義


//============================================================================================
//	グローバル
//============================================================================================

// フォルムテーブル
static const u16 FormTable[][2] =
{
	// ポケモン番号, フォルム数
	{ MONSNO_ANNOON, FORM_MAX_ANNOON },						// アンノーン

	{ MONSNO_DEOKISISU, FORM_MAX_DEOKISISU },			// デオキシス

	{ MONSNO_SHEIMI, FORM_MAX_SHEIMI },						// シェイミ
	{ MONSNO_GIRATHINA, FORM_MAX_GIRATHINA },			// ギラティナ
	{ MONSNO_ROTOMU, FORM_MAX_ROTOMU },						// ロトム
	{ MONSNO_KARANAKUSI, FORM_MAX_KARANAKUSI },		// カラナクシ
	{ MONSNO_TORITODON, FORM_MAX_TORITODON },			// トリトドン
	{ MONSNO_MINOMUTTI, FORM_MAX_MINOMUTTI },			// ミノムッチ
	{ MONSNO_MINOMADAMU, FORM_MAX_MINOMADAMU },		// ミノマダム

	{ MONSNO_POWARUN, FORM_MAX_POWARUN },					// ポワルン ※特殊
	{ MONSNO_THERIMU, FORM_MAX_THERIMU },					// チェリム ※特殊

	{ MONSNO_511, FORMNO_511_MAX },				        // シキジカ ※新規
	{ MONSNO_527, FORMNO_527_MAX },				        // アントレス ※新規
	{ MONSNO_655, FORMNO_655_MAX },								// メロディア ※新規
	{ MONSNO_HIHIDARUMA, FORM_MAX_HIHIDARUMA },		// ヒヒダルマ ※新規
	{ MONSNO_BASURAO, FORM_MAX_BASURAO },					// バスラオ ※新規

	{ 0, 0 },
};



//============================================================================================
//	内部関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ずかんワークの整合性チェック
 *
 * @param		図鑑セーブデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void zukan_incorrect(const ZUKAN_SAVEDATA * zw)
{
  GF_ASSERT(zw->zukan_magic == MAGIC_NUMBER);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンナンバーの範囲チェック
 *
 * @param		monsno		ポケモン番号
 *
 * @retval	"TRUE = 範囲外"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL monsno_incorrect(u16 monsno)
{
  if (monsno == 0 || monsno > MONSNO_END) {
    GF_ASSERT_MSG(0, "ポケモンナンバー異常：%d\n", monsno);
    return TRUE;
  } else {
    return FALSE;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		汎用ビットチェック
 *
 * @param		array			ビットテーブル
 * @param		flag_id		フラグＩＤ
 *
 * @retval	"TRUE = ON"
 * @retval	"FALSE = OFF"
 */
//--------------------------------------------------------------------------------------------
static inline BOOL check_bit(const u8 * array, u16 flag_id)
{
  return 0 != (array[flag_id >> 3] & (1 << (flag_id & 7)));
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		汎用ビットチェック（フラグ = ポケモン番号）
 *
 * @param		array			ビットテーブル
 * @param		flag_id		フラグＩＤ
 *
 * @retval	"TRUE = ON"
 * @retval	"FALSE = OFF"
 */
//--------------------------------------------------------------------------------------------
static inline BOOL check_bit_mons(const u8 * array, u16 flag_id)
{
  flag_id --;
	return check_bit( array, flag_id );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		汎用ビットセット
 *
 * @param		array			ビットテーブル
 * @param		flag_id		フラグＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void set_bit(u8 * array, u16 flag_id)
{
  array[flag_id >> 3] |= 1 << (flag_id & 7);
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ZUKAN)
  SVLD_SetCrc(GMDATA_ID_ZUKAN);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		汎用ビットセット（フラグ = ポケモン番号）
 *
 * @param		array			ビットテーブル
 * @param		flag_id		フラグＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void set_bit_mons(u8 * array, u16 flag_id)
{
  flag_id --;
	set_bit( array, flag_id );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		汎用ビットリセット
 *
 * @param		array			ビットテーブル
 * @param		flag_id		フラグＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void reset_bit( u8 * array, u16 flag_id )
{
  array[flag_id >> 3] &= (1 << (flag_id & 7)) ^ 0xff;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		汎用ビットリセット（フラグ = ポケモン番号）
 *
 * @param		array			ビットテーブル
 * @param		flag_id		フラグＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void reset_bit_mons( u8 * array, u16 flag_id )
{
	flag_id--;
	reset_bit( array, flag_id );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		みたビット設定
 *
 * @param		zw				図鑑セーブデータ
 * @param		flag_id		ポケモン番号
 * @param		type			性別
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void set_see_bit( ZUKAN_SAVEDATA * zw, u16 flag_id, u16 type )
{
	set_bit_mons( (u8*)zw->sex_flag[type], flag_id );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		つかまえたビット設定
 *
 * @param		zw				図鑑セーブデータ
 * @param		flag_id		ポケモン番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void set_get_bit(ZUKAN_SAVEDATA * zw, u16 flag_id)
{
  set_bit_mons( (u8*)zw->get_flag, flag_id );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		みたビットON/OFFチェック
 *
 * @param		zw				図鑑セーブデータ
 * @param		monsno		ポケモン番号
 * @param		type			性別
 *
 * @retval	"TRUE = ON"
 * @retval	"FALSE = OFF"
 */
//--------------------------------------------------------------------------------------------
static inline BOOL check_see_bit( const ZUKAN_SAVEDATA * zw, u16 monsno, u16 type )
{
	return check_bit_mons( (const u8*)zw->sex_flag[type], monsno );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		つかまえたビットON/OFFチェック
 *
 * @param		zw				図鑑セーブデータ
 * @param		monsno		ポケモン番号
 *
 * @retval	"TRUE = ON"
 * @retval	"FALSE = OFF"
 */
//--------------------------------------------------------------------------------------------
static inline BOOL check_get_bit(const ZUKAN_SAVEDATA * zw, u16 monsno )
{
  return check_bit_mons((const u8*)zw->get_flag, monsno );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		固体乱数を設定
 *
 * @param		zw				図鑑セーブデータ
 * @param		monsno		ポケモン番号
 * @param		rand			個性乱数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static inline void SetZukanRandom( ZUKAN_SAVEDATA * zw, u16 monsno, u32 rand )
{
  if( monsno == MONSNO_PATTIIRU && zw->PachiRandom == 0 ){
    zw->PachiRandom = rand;
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ZUKAN)
  SVLD_SetCrc(GMDATA_ID_ZUKAN);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		テキストバージョンフラグ設定
 *
 * @param		zw				図鑑セーブデータ
 * @param		monsno		ポケモン番号
 * @param		lang			国コード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetZukanTextVersionUp( ZUKAN_SAVEDATA * zw, u16 monsno, u32 lang )
{
	// 国コード最大 or 欠番コード
	if( lang > 8 || lang == 6 ){
		return;
	}

	// 新規ポケモンは表示不可
	if( monsno > POKEZUKAN_TEXT_POSSIBLE ){
		return;
	}

	// 言語６番が欠番なので、スペイン語以降は-1
	if( lang >= LANG_SPAIN ){
		lang -= 1;
	}

	set_bit( zw->TextVersionUp, (monsno-1)*TEXTVER_VER_MAX+(lang-1) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		全国図鑑完成に必要なポケモンかチェック
 *
 * @param		monsno		ポケモン番号
 *
 * @retval	"TRUE = 必要"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL check_ZenkokuCompMonsno( u16 monsno )
{
  int i;
  static const u16 cut_check_monsno[] = {
    MONSNO_MYUU,				// ミュウ
    MONSNO_SEREBHI,			// セレビィ
    MONSNO_ZIRAATI,			// ジラーチ
    MONSNO_DEOKISISU,		// デオキシス
    MONSNO_FIONE,				// フィオネ
    MONSNO_MANAFI,			// マナフィ
    MONSNO_DAAKURAI,		// ダークライ
    MONSNO_SHEIMI,			// シェイミ
    MONSNO_ARUSEUSU,		// アルセウス
		MONSNO_652,					// ツチノカミ
		MONSNO_653,					// ライ
		MONSNO_654,					// ダルタニス
		MONSNO_655,					// メロディア
		MONSNO_656,					// インセクタ
		MONSNO_657,					// ビクティ
  };

  // チェック除外ポケモンチェック
  for( i=0; i<NELEMS(cut_check_monsno); i++ ){
    if( cut_check_monsno[i] == monsno ){
      return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		地方図鑑完成に必要なポケモンかチェック
 *
 * @param		monsno		ポケモン番号
 *
 * @retval	"TRUE = 必要"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL check_LocalCompMonsno( u16 monsno )
{
  int i;
  static const u16 cut_check_monsno[] = {
		MONSNO_652,			// ツチノカミ
		MONSNO_653,			// ライ
		MONSNO_654,			// ダルタニス
		MONSNO_655,			// メロディア
		MONSNO_656,			// インセクタ
		MONSNO_657,			// ビクティ
  };

  // チェック除外ポケモンチェック
  for( i=0; i<NELEMS(cut_check_monsno); i++ ){
    if( cut_check_monsno[i] == monsno ){
      return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルム設定ビットを取得
 *
 * @param		mons			ポケモン番号
 *
 * @retval	"-1 != フォルム設定ビット"
 * @retval	"-1 = 別フォルムがないポケモン"
 */
//--------------------------------------------------------------------------------------------
static s32 GetPokeFormBit( u16 mons )
{
	s32	cnt = 0;
	s32	i = 0;

	while( FormTable[i][0] != 0 ){
		if( FormTable[i][0] == mons ){
			return cnt;
		}
		cnt += FormTable[i][1];
		i++;
	}
	return -1;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルムテーブル位置を取得
 *
 * @param		mons			ポケモン番号
 *
 * @retval	"-1 != フォルムテーブル位置"
 * @retval	"-1 = 別フォルムがないポケモン"
 */
//--------------------------------------------------------------------------------------------
static s32 GetPokeFormTablePos( u16 mons )
{
	s32	i = 0;

	while( FormTable[i][0] != 0 ){
		if( FormTable[i][0] == mons ){
			return i;
		}
		i++;
	}
	return -1;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		通常の見たフラグ設定
 *
 * @param		zw      図鑑ワークへのポインタ
 * @param		mons		ポケモン番号
 * @param		sex			性別
 * @param		rare		レアフラグ TRUE = RARE
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetNormalSeeFlag( ZUKAN_SAVEDATA * zw, u32 mons, u32 sex, BOOL rare )
{
	if( sex == PTL_SEX_MALE ){
		if( rare == TRUE ){
			set_see_bit( zw, mons, SEE_FLAG_M_RARE );
		}else{
			set_see_bit( zw, mons, SEE_FLAG_MALE );
		}
	}else if( sex == PTL_SEX_FEMALE ){
		if( rare == TRUE ){
			set_see_bit( zw, mons, SEE_FLAG_F_RARE );
		}else{
			set_see_bit( zw, mons, SEE_FLAG_FEMALE );
		}
	}else{
		// 性別がない場合は♂のフラグを立てる
		if( rare == TRUE ){
			set_see_bit( zw, mons, SEE_FLAG_M_RARE );
		}else{
			set_see_bit( zw, mons, SEE_FLAG_MALE );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルムみたフラグ設定
 *
 * @param		zw      図鑑ワークへのポインタ
 * @param		mons		ポケモン番号
 * @param		sex			性別
 * @param		rare		レアフラグ TRUE = RARE
 * @param		form		フォルム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetFormSeeFlag( ZUKAN_SAVEDATA * zw, u32 mons, u32 sex, BOOL rare, u32 form )
{
	s32	pos = GetPokeFormBit( mons );

	if( pos != -1 ){
		u16	max;
		u16	i;
		// 一部のポケモンは全てのフォルムをONにする
		// ポワルン
		if( mons == MONSNO_POWARUN ){
			form = 0;
			max  = FORM_MAX_POWARUN;
		// チェリム
		}else if( mons == MONSNO_THERIMU ){
			form = 0;
			max  = FORM_MAX_THERIMU;
		// ヒヒダルマ
		}else if( mons == MONSNO_HIHIDARUMA ){
			form = 0;
			max  = FORM_MAX_HIHIDARUMA;
		// メロディア
		}else if( mons == MONSNO_MERODHIA ){
			form = 0;
			max  = FORM_MAX_MERODHIA;
		}else{
			max  = 1;
		}

		if( rare == TRUE ){
			for( i=0; i<max; i++ ){
				set_bit( zw->form_flag[COLOR_RARE], pos+form+i );
			}
		}else{
			for( i=0; i<max; i++ ){
				set_bit( zw->form_flag[COLOR_NORMAL], pos+form+i );
			}
		}
	}
}


//============================================================================================
//	ずかん情報操作用の外部公開関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		図鑑セーブデータのサイズ取得
 *
 * @param		none
 *
 * @return	図鑑セーブデータのサイズ
 */
//--------------------------------------------------------------------------------------------
int ZUKANSAVE_GetWorkSize(void)
{
  return sizeof(ZUKAN_SAVEDATA);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		図鑑セーブデータの初期化
 *
 * @param		zw		図鑑セーブデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_Init( ZUKAN_SAVEDATA * zw )
{
	GFL_STD_MemClear( zw, sizeof(ZUKAN_SAVEDATA) );
  zw->zukan_magic = MAGIC_NUMBER;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		図鑑セーブデータ取得（セーブデータから）
 *
 * @param		sv		セーブデータ保持ワークへのポインタ
 *
 * @return  図鑑セーブデータ
 *
 * @li	通常は使用しないでください！
 */
//--------------------------------------------------------------------------------------------
ZUKAN_SAVEDATA * ZUKAN_SAVEDATA_GetZukanSave( SAVE_CONTROL_WORK * sv )
{
	return SaveControl_DataPtrGet( sv, GMDATA_ID_ZUKAN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		図鑑セーブデータ取得（GAMEDETAから）
 *
 * @param		gamedata		GAMEDATA
 *
 * @return  図鑑セーブデータ
 *
 * @li	ゲーム中はこちらを使用してください
 */
//--------------------------------------------------------------------------------------------
ZUKAN_SAVEDATA * GAMEDATA_GetZukanSave( GAMEDATA * gamedata )
{
	return ZUKAN_SAVEDATA_GetZukanSave( GAMEDATA_GetSaveControlWork(gamedata) );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		全国図鑑モードをセット
 *
 * @param		zw		図鑑セーブデータ
 *
 * @return	none
 *
 *	全国図鑑を表示可能にする
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetZenkokuZukanFlag( ZUKAN_SAVEDATA * zw )
{
  zukan_incorrect(zw);
  zw->zenkoku_flag = TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		全国ずかんモードかどうか？の問い合わせ
 *
 * @param		zw		図鑑セーブデータ
 *
 * @retval	"TRUE = 全国図鑑可"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_GetZenkokuZukanFlag( const ZUKAN_SAVEDATA * zw )
{
  zukan_incorrect(zw);
  return zw->zenkoku_flag;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief  グラフィックバージョン用機能拡張フラグ設定
 *
 * @param		zw		図鑑セーブデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetGraphicVersionUpFlag( ZUKAN_SAVEDATA * zw )
{
  zukan_incorrect(zw);
  zw->ver_up_flg = TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		グラフィックバージョン用機能拡張フラグ取得
 *
 * @param		zw		図鑑セーブデータ
 *
 * @retval	"TRUE = バージョンアップ済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_GetGraphicVersionUpFlag( const ZUKAN_SAVEDATA * zw )
{
  zukan_incorrect(zw);
  return zw->ver_up_flg;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		閲覧中の図鑑モードを設定
 *
 * @param		zw		図鑑セーブデータ
 * @param		mode	図鑑モード TRUE = 全国, FALSE = 地方
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetZukanMode( ZUKAN_SAVEDATA * zw, BOOL mode )
{
  zukan_incorrect(zw);
	zw->zukan_mode = mode;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		閲覧中の図鑑モードを取得
 *
 * @param		zw		図鑑セーブデータ
 *
 * @retval	"TRUE = 全国"
 * @retval	"FALSE = 地方"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_GetZukanMode( const ZUKAN_SAVEDATA * zw )
{
  zukan_incorrect(zw);
	return zw->zukan_mode;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		閲覧中のポケモン番号を設定
 *
 * @param		zw		図鑑セーブデータ
 * @param		mons	ポケモン番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetDefaultMons( ZUKAN_SAVEDATA * zw, u16 mons )
{
  zukan_incorrect(zw);
	zw->defaultMonsNo = mons;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		閲覧中のポケモン番号を取得
 *
 * @param		zw		図鑑セーブデータ
 *
 * @return	ポケモン番号
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetDefaultMons( const ZUKAN_SAVEDATA * zw )
{
  zukan_incorrect(zw);
	return zw->defaultMonsNo;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		閲覧中データ設定
 *
 * @param		zw			図鑑セーブデータ
 * @param		mons		ポケモン番号
 * @param		sex			性別
 * @param		rare		レアフラグ TRUE = RARE
 * @param		form		フォルム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetDrawData( ZUKAN_SAVEDATA * zw, u16 mons, u32 sex, BOOL rare, u32 form )
{
	{	// 閲覧中フォルムデータ
		s32	cnt = 0;
		s32	i = 0;

		while( FormTable[i][0] != 0 ){
			if( FormTable[i][0] == mons ){
				u32	j;
				for( j=0; j<FormTable[i][1]; j++ ){
					reset_bit( (u8 *)zw->draw_form[0], cnt+j );
					reset_bit( (u8 *)zw->draw_form[1], cnt+j );
				}
				if( rare == TRUE ){
					set_bit( (u8 *)zw->draw_form[COLOR_RARE], cnt+form );
				}else{
					set_bit( (u8 *)zw->draw_form[COLOR_NORMAL], cnt+form );
				}
				break;
			}
			cnt += FormTable[i][1];
			i++;
		}
	}

	{	// 閲覧中通常データ
		u32	i;

		for( i=0; i<SEE_FLAG_MAX; i++ ){
			reset_bit_mons( (u8 *)zw->draw_sex[i], mons );
		}
		if( sex == PTL_SEX_MALE ){
			if( rare == TRUE ){
				set_bit_mons( (u8 *)zw->draw_sex[SEE_FLAG_M_RARE], mons );
			}else{
				set_bit_mons( (u8 *)zw->draw_sex[SEE_FLAG_MALE], mons );
			}
		}else if( sex == PTL_SEX_FEMALE ){
			if( rare == TRUE ){
				set_bit_mons( (u8 *)zw->draw_sex[SEE_FLAG_F_RARE], mons );
			}else{
				set_bit_mons( (u8 *)zw->draw_sex[SEE_FLAG_FEMALE], mons );
			}
		}else{
			// 性別がない場合は♂のフラグを立てる
			if( rare == TRUE ){
				set_bit_mons( (u8 *)zw->draw_sex[SEE_FLAG_M_RARE], mons );
			}else{
				set_bit_mons( (u8 *)zw->draw_sex[SEE_FLAG_MALE], mons );
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		最大フォルム数取得
 *
 * @param		mons		ポケモン番号
 *
 * @return	最大フォルム数
 */
//--------------------------------------------------------------------------------------------
u32	ZUKANSAVE_GetFormMax( u16 mons )
{
	s32	pos = GetPokeFormTablePos( mons );

	if( pos != -1 ){
		return FormTable[pos][1];
	}
	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		表示するフォルムデータを取得
 *
 * @param		zw			図鑑セーブデータ
 * @param		mons		ポケモン番号
 * @param		sex			性別
 * @param		rare		レアフラグ TRUE = RARE
 * @param		form		フォルム
 * @param		heapID	ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_GetDrawData( ZUKAN_SAVEDATA * zw, u16 mons, u32 * sex, BOOL * rare, u32 * form, HEAPID heapID )
{
	POKEMON_PERSONAL_DATA * ppd;
	s32	tp;
	s32	bit;
	u32	i;

	tp = GetPokeFormTablePos( mons );

	if( tp != -1 ){
		bit = GetPokeFormBit( mons );
		for( i=0; i<FormTable[tp][1]; i++ ){
			if( check_bit( zw->draw_form[COLOR_NORMAL], bit+i ) ){
				*form = i;
				*rare = FALSE;
				break;
			}
			if( check_bit( zw->draw_form[COLOR_RARE], bit+i ) ){
				*form = i;
				*rare = TRUE;
				break;
			}
		}
		if( check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_MALE], mons ) ||
				check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_M_RARE], mons ) ){
			*sex  = PTL_SEX_MALE;
		}
		if( check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_FEMALE], mons ) ||
				check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_F_RARE], mons ) ){
			*sex  = PTL_SEX_FEMALE;
		}
	}else{
		*form = 0;
		if( check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_MALE], mons ) ){
			*sex  = PTL_SEX_MALE;
			*rare = FALSE;
		}
		if( check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_FEMALE], mons ) ){
			*sex  = PTL_SEX_FEMALE;
			*rare = FALSE;
		}
		if( check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_M_RARE], mons ) ){
			*sex  = PTL_SEX_MALE;
			*rare = TRUE;
		}
		if( check_bit_mons( (const u8 *)zw->draw_sex[SEE_FLAG_F_RARE], mons ) ){
			*sex  = PTL_SEX_FEMALE;
			*rare = TRUE;
		}
	}

	ppd = POKE_PERSONAL_OpenHandle( mons, *form, heapID );
	if( POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex ) == POKEPER_SEX_UNKNOWN ){
		*sex  = PTL_SEX_UNKNOWN;
	}
	POKE_PERSONAL_CloseHandle( ppd );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		捕獲データセット
 *
 * @param		zw		図鑑セーブデータ
 * @param		pp		捕獲ポケモンデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetPokeGet( ZUKAN_SAVEDATA * zw, POKEMON_PARAM * pp )
{
	BOOL	fast;
	BOOL	rare;
  u32 rand;
	u32	form;
  u32 lang;
  u32 sex;
  u16 mons;

  zukan_incorrect(zw);

	fast = PP_FastModeOn( pp );
	mons = PP_Get( pp, ID_PARA_monsno, NULL );
	rand = PP_Get( pp, ID_PARA_personal_rnd, NULL );
	form = PP_Get( pp, ID_PARA_form_no, NULL );
  lang = PP_Get( pp, ID_PARA_country_code, NULL );
	sex  = PP_GetSex( pp );
	rare = PP_CheckRare( pp );
	PP_FastModeOff( pp, fast );

  if( monsno_incorrect(mons) ){
    return;
  }

	// 見たことない場合
	if( ZUKANSAVE_GetPokeSeeFlag( zw, mons ) == FALSE ){
		ZUKANSAVE_SetDrawData( zw, mons, sex, rare, form );
	}

	// 固体乱数
	SetZukanRandom( zw, mons, rand );
	// 通常
	SetNormalSeeFlag( zw, mons, sex, rare );
	// フォルムあり
	SetFormSeeFlag( zw, mons, sex, rare, form );
	// 国コードセット
	SetZukanTextVersionUp( zw, mons, lang );
	// 捕獲フラグセット
  set_get_bit( zw, mons );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		捕獲チェック
 *
 * @param		zw			図鑑セーブデータ
 * @param		monsno	ポケモン番号
 *
 * @retval	"TRUE = 捕獲済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_GetPokeGetFlag( const ZUKAN_SAVEDATA * zw, u16 monsno )
{
  zukan_incorrect(zw);

  if( monsno_incorrect(monsno) ) {
    return FALSE;
  }
  if( check_get_bit(zw,monsno) ){
    return TRUE;
  }
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		現在の図鑑モードのポケモン捕獲数取得
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	捕獲数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetZukanPokeGetCount( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  if( ZUKANSAVE_GetZenkokuZukanFlag( zw ) ){
    return ZUKANSAVE_GetPokeGetCount( zw );
  }
  return ZUKANSAVE_GetLocalPokeGetCount( zw, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン捕獲数取得（全国）
 *
 * @param		zw			図鑑セーブデータ
 *
 * @return	捕獲数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetPokeGetCount( const ZUKAN_SAVEDATA * zw )
{
  int count;
  int i;

  zukan_incorrect(zw);

  count= 0;
  for( i=1; i<=MONSNO_END; i++ ){
    if( ZUKANSAVE_GetPokeGetFlag( zw, i ) == TRUE ){
      count++;
    }
  }
  return count;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン捕獲数取得（地方）
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	捕獲数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetLocalPokeGetCount( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  u16 * buf;
  u16 i;
  u16 num=0;

  zukan_incorrect(zw);

	buf = POKE_PERSONAL_GetZenkokuToChihouArray( heapID, NULL );
  num = 0;

  for( i=1; i<=MONSNO_END; i++ ){
    if( ZUKANSAVE_GetPokeGetFlag( zw, i ) == TRUE ){
      // 地方図鑑にいるかチェック
      if( buf[i] != POKEPER_CHIHOU_NO_NONE ){
        num++;
      }
    }
  }

  GFL_HEAP_FreeMemory( buf );

  return num;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		見たデータセット
 *
 * @param		zw		図鑑セーブデータ
 * @param		pp		見たポケモンのデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZUKANSAVE_SetPokeSee( ZUKAN_SAVEDATA * zw, POKEMON_PARAM * pp )
{
	BOOL	fast;
	BOOL	rare;
  u32 rand;
	u32	form;
  u32 sex;
  u16 mons;

  zukan_incorrect(zw);

	fast = PP_FastModeOn( pp );
	mons = PP_Get( pp, ID_PARA_monsno, NULL );
	rand = PP_Get( pp, ID_PARA_personal_rnd, NULL );
	form = PP_Get( pp, ID_PARA_form_no, NULL );
	sex  = PP_GetSex( pp );
	rare = PP_CheckRare( pp );
	PP_FastModeOff( pp, fast );

  if( monsno_incorrect(mons) ){
    return;
  }

	// 見たことない場合
	if( ZUKANSAVE_GetPokeSeeFlag( zw, mons ) == FALSE ){
		ZUKANSAVE_SetDrawData( zw, mons, sex, rare, form );
	}

	// 固体乱数
	SetZukanRandom( zw, mons, rand );
	// 通常
	SetNormalSeeFlag( zw, mons, sex, rare );
	// フォルムあり
	SetFormSeeFlag( zw, mons, sex, rare, form );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		見たチェック
 *
 * @param		zw			図鑑セーブデータ
 * @param		monsno	ポケモン番号
 *
 * @retval	"TRUE = 見た"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_GetPokeSeeFlag( const ZUKAN_SAVEDATA * zw, u16 monsno )
{
	u32	i;

  zukan_incorrect(zw);
  if( monsno_incorrect(monsno) ){
    return FALSE;
  }
	for( i=0; i<SEE_FLAG_MAX; i++ ){
		if( check_see_bit(zw,monsno,i) ){
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		現在の図鑑モードのポケモンを見た数取得
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	見た数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetZukanPokeSeeCount( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  if( ZUKANSAVE_GetZenkokuZukanFlag( zw ) ){
    return ZUKANSAVE_GetPokeSeeCount( zw );
  }
  return ZUKANSAVE_GetLocalPokeSeeCount( zw, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンを見た数取得（全国）
 *
 * @param		zw			図鑑セーブデータ
 *
 * @return	見た数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetPokeSeeCount( const ZUKAN_SAVEDATA * zw )
{
  int count;
  int i;

  zukan_incorrect(zw);

  count= 0;
  for( i=1; i<=MONSNO_END; i++ ){
    if( ZUKANSAVE_GetPokeSeeFlag( zw, i ) == TRUE ){
      count++;
    }
  }
  return count;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンを見た数取得（地方）
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	見た数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetLocalPokeSeeCount( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  u16 * buf;
  u16 i;
  u16 num=0;

  zukan_incorrect(zw);

	buf = POKE_PERSONAL_GetZenkokuToChihouArray( heapID, NULL );
  num = 0;

  for( i=1; i<=MONSNO_END; i++ ){
    if( ZUKANSAVE_GetPokeSeeFlag( zw, i ) == TRUE ){
      // 地方図鑑にいるかチェック
      if( buf[i] != POKEPER_CHIHOU_NO_NONE ){
        num++;
      }
    }
  }

  GFL_HEAP_FreeMemory( buf );

  return num;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		指定タイプのポケモンを見たか
 *
 * @param		zw			図鑑セーブデータ
 * @param		monsno	ポケモン番号
 * @param		sex			性別
 * @param		rare		レア
 * @param		form		フォルム
 *
 * @retval	"TRUE = 見た"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_CheckPokeSeeForm( const ZUKAN_SAVEDATA * zw, u16 monsno, int sex, int rare, int form )
{
  zukan_incorrect(zw);

  if( monsno_incorrect(monsno) ){
    return ZUKANSAVE_GET_SEX_ERR;
  }

	{	// フォルムあり
		s32	pos = GetPokeFormBit( monsno );
		if( pos != -1 ){
			return check_bit( (const u8*)zw->form_flag[rare], pos+form );
		}
	}

	// 通常
	if( sex == PTL_SEX_MALE ){
		if( rare == 1 ){
			return check_see_bit( zw, monsno, SEE_FLAG_M_RARE );
		}else{
			return check_see_bit( zw, monsno, SEE_FLAG_MALE );
		}
	}else if( sex == PTL_SEX_FEMALE ){
		if( rare == 1 ){
			return check_see_bit( zw, monsno, SEE_FLAG_F_RARE );
		}else{
			return check_see_bit( zw, monsno, SEE_FLAG_FEMALE );
		}
	}else{
		if( rare == 1 ){
			if( check_see_bit( zw, monsno, SEE_FLAG_M_RARE ) == TRUE ||
					check_see_bit( zw, monsno, SEE_FLAG_F_RARE ) == TRUE ){
				return TRUE;
			}
		}else{
			if( check_see_bit( zw, monsno, SEE_FLAG_MALE ) == TRUE ||
					check_see_bit( zw, monsno, SEE_FLAG_FEMALE ) == TRUE ){
				return TRUE;
			}
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		特殊ポケモンの個性乱数取得
 *
 * @param		zw			図鑑セーブデータ
 * @param		monsno	ポケモン番号
 *
 * @return	個性乱数
 */
//--------------------------------------------------------------------------------------------
u32 ZUKANSAVE_GetPokeRandomFlag( const ZUKAN_SAVEDATA * zw, u8 monsno )
{
  u32 random;

  zukan_incorrect(zw);

	// パッチールのみ
	if( monsno == ZUKANSAVE_RANDOM_PACHI ){
		return zw->PachiRandom;
	}
	GF_ASSERT_MSG( 0, "monsno = %d\n", monsno );
	return 0;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		全国図鑑完成チェック
 *
 * @param		zw			図鑑セーブデータ
 *
 * @retval	"TRUE = 完成"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_CheckZenkokuComp( const ZUKAN_SAVEDATA * zw )
{
  u16 num;

  // ゼンコク図鑑完成に必要なポケモンを何匹捕まえたか
  num = ZUKANSAVE_GetZenkokuGetCompCount( zw );

  if( num >= ZUKANSAVE_ZENKOKU_COMP_NUM ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		地方図鑑完成チェック（捕獲数）
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @retval	"TRUE = 完成"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_CheckLocalGetComp( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  u16 num;

  // 地方図鑑完成に必要なポケモンを何匹捕まえたか
  num = ZUKANSAVE_GetLocalGetCompCount( zw, heapID );

  if( num >= ZUKANSAVE_LOCAL_COMP_NUM ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		地方図鑑完成チェック（見た数）
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @retval	"TRUE = 完成"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_CheckLocalSeeComp( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  u16 num;

  // 地方図鑑完成に必要なポケモンを何匹捕まえたか
  num = ZUKANSAVE_GetLocalSeeCompCount( zw, heapID );

  if( num >= ZUKANSAVE_LOCAL_COMP_NUM ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		全国図鑑完成に必要なポケモンの捕獲数
 *
 * @param		zw			図鑑セーブデータ
 *
 * @return	捕獲数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetZenkokuGetCompCount( const ZUKAN_SAVEDATA * zw )
{
  int i;
  u16 num;

  num = 0;
  for( i=1; i<=ZUKANSAVE_ZENKOKU_MONSMAX; i++ ){
    if( ZUKANSAVE_GetPokeGetFlag( zw, i ) == TRUE ){
      if( check_ZenkokuCompMonsno( i ) == TRUE ){
        num ++;
      }
    }
  }
  return num;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		地方図鑑完成に必要なポケモンの捕獲数
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	捕獲数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetLocalGetCompCount( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  u16 * buf;
  u16 i;
  u16 num=0;

	buf = POKE_PERSONAL_GetZenkokuToChihouArray( heapID, NULL );

  for( i=1; i<=ZUKANSAVE_ZENKOKU_MONSMAX; i++ ){
    if( ZUKANSAVE_GetPokeGetFlag( zw, i ) == TRUE ){
      if( buf[i] != POKEPER_CHIHOU_NO_NONE ){
        if( check_LocalCompMonsno( i ) == TRUE ){
          num++;
        }
      }
    }
  }

  GFL_HEAP_FreeMemory( buf );

  return num;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		地方図鑑完成に必要なポケモンを見た数
 *
 * @param		zw			図鑑セーブデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	見た数
 */
//--------------------------------------------------------------------------------------------
u16 ZUKANSAVE_GetLocalSeeCompCount( const ZUKAN_SAVEDATA * zw, HEAPID heapID )
{
  u16 * buf;
  u16 i;
  u16 num=0;

	buf = POKE_PERSONAL_GetZenkokuToChihouArray( heapID, NULL );

  for( i=1; i<=ZUKANSAVE_ZENKOKU_MONSMAX; i++ ){
    if( ZUKANSAVE_GetPokeSeeFlag( zw, i ) == TRUE ){
      if( buf[i] != POKEPER_CHIHOU_NO_NONE ){
        if( check_LocalCompMonsno( i ) == TRUE ){
          num++;
        }
      }
    }
  }

  GFL_HEAP_FreeMemory( buf );

  return num;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		図鑑テキストバージョンチェック
 *
 * @param		zw						図鑑セーブデータ
 * @param		monsno				ポケモン番号
 * @param		country_code	国コード
 *
 * @retval	"TRUE = 表示可"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZUKANSAVE_GetTextVersionUpFlag( const ZUKAN_SAVEDATA * zw, u16 monsno, u32 country_code )
{
  zukan_incorrect(zw);

	// 国コード最大 or 欠番コード
	if( country_code > 8 || country_code == 6 ){
		return FALSE;
	}

	// 新規追加ポケモンは表示できない
	if( monsno > POKEZUKAN_TEXT_POSSIBLE ){
		return FALSE;
	}

	// 言語６番が欠番なので、スペイン語以降は-1
	if( country_code >= LANG_SPAIN ){
		country_code -= 1;
	}

	return check_bit( zw->TextVersionUp, (monsno-1)*TEXTVER_VER_MAX+(country_code-1) );
}





//============================================================================================
//	デバッグ処理
//============================================================================================
#ifdef	PM_DEBUG

static void DebugDataClearCore( ZUKAN_SAVEDATA * zw, u16 mons )
{
	u32	i;

	reset_bit_mons( (u8 *)zw->get_flag, mons );				// 捕獲フラグクリア

	for( i=SEE_FLAG_MALE; i<=SEE_FLAG_F_RARE; i++ ){
		reset_bit_mons( (u8 *)zw->sex_flag[i], mons );	// ♂♀フラグクリア
		reset_bit_mons( (u8 *)zw->draw_sex[i], mons );	// 閲覧中♂♀フラグクリア
	}

	{	// フォルムデータクリア
		s32	cnt = 0;
		i = 0;
		while( FormTable[i][0] != 0 ){
			if( FormTable[i][0] == mons ){
				u16	j, k;
				for( j=0; j<FormTable[i][1]; j++ ){
					for( k=COLOR_NORMAL; k<=COLOR_RARE; k++ ){
						reset_bit( (u8 *)zw->form_flag[k], cnt+j );			// フォルムフラグクリア
						reset_bit( (u8 *)zw->draw_form[k], cnt+j );			// 閲覧中フォルムフラグクリア
					}
				}
			}
			cnt += FormTable[i][1];
			i++;
		}
	}

	// 言語フラグクリア
	if( mons <= POKEZUKAN_TEXT_POSSIBLE ){
		for( i=0; i<TEXTVER_VER_MAX; i++ ){
			reset_bit( zw->TextVersionUp, (mons-1)*TEXTVER_VER_MAX+i );
		}
	}

	// パッチール用個性乱数クリア
  if( mons == MONSNO_PATTIIRU ){
		zw->PachiRandom = 0;
	}
}

// データクリア
void ZUKANSAVE_DebugDataClear( ZUKAN_SAVEDATA * zw, u16 start, u16 end )
{
	u32	i;

	if( zw->defaultMonsNo >= start && zw->defaultMonsNo <= end ){
		zw->defaultMonsNo = 0;		// 閲覧中のポケモン番号クリア
	}

	for( i=start; i<=end; i++ ){
		DebugDataClearCore( zw, i );
	}
}

// 見た設定
void ZUKANSAVE_DebugDataSetSee( ZUKAN_SAVEDATA * zw, u16 start, u16 end, HEAPID heapID )
{
	POKEMON_PARAM * pp;
	u32	i;

	for( i=start; i<=end; i++ ){
		DebugDataClearCore( zw, i );
    pp = PP_Create( i, 50, 0, heapID );
		ZUKANSAVE_SetPokeSee( zw, pp );
    GFL_HEAP_FreeMemory( pp );
	}
}

// 捕獲設定
void ZUKANSAVE_DebugDataSetGet( ZUKAN_SAVEDATA * zw, u16 start, u16 end, HEAPID heapID )
{
	POKEMON_PARAM * pp;
	u32	i;

	for( i=start; i<=end; i++ ){
		DebugDataClearCore( zw, i );
    pp = PP_Create( i, 50, 0, heapID );
		ZUKANSAVE_SetPokeGet( zw, pp );
    GFL_HEAP_FreeMemory( pp );
	}
}

#endif	// PM_DEBUG