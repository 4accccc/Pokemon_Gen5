//============================================================================================
/**
 * @file		zukan_savedata_local.h
 * @brief		図鑑セーブデータ内部参照定義
 * @author	mori / tamada GAME FREAK inc.
 * @date		2009.10.26
*/
//============================================================================================
#pragma	once

enum {
  POKEZUKAN_ARRAY_LEN = 21, ///<ポケモン図鑑フラグ配列のおおきさ 21 * 32 = 672まで大丈夫

  MAGIC_NUMBER = 0xbeefcafe,

  POKEZUKAN_TEXT_POSSIBLE  = MONSNO_ARUSEUSU, // 海外図鑑テキストが読める最大数（＝シンオウポケモンまで）

	// 見た用定義
	SEE_FLAG_MALE = 0,		// ♂
	SEE_FLAG_FEMALE,			// ♀
	SEE_FLAG_M_RARE,			// ♂レア
	SEE_FLAG_F_RARE,			// ♀レア
	SEE_FLAG_MAX,

	// フォルム数	
	FORM_MAX_ANNOON = 28,					// アンノーン
	FORM_MAX_DEOKISISU = 4,				// デオキシス
	FORM_MAX_SHEIMI = 2,					// シェイミ
	FORM_MAX_GIRATHINA = 2,				// ギラティナ
	FORM_MAX_ROTOMU = 6,					// ロトム
	FORM_MAX_KARANAKUSI = 2,			// カラナクシ
	FORM_MAX_TORITODON = 2,				// トリトドン
	FORM_MAX_MINOMUTTI = 3,				// ミノムッチ
	FORM_MAX_MINOMADAMU = 3,			// ミノマダム
	FORM_MAX_POWARUN = 4,					// ポワルン ※特殊
	FORM_MAX_THERIMU = 2,					// チェリム ※特殊
	FORM_MAX_BANBIINA = 4,				// バンビーナ ※新規
	FORM_MAX_SIKIZIKA = 4,				// シキジカ ※新規
	FORM_MAX_MERODHIA = 2,				// メロディア ※新規
	FORM_MAX_HIHIDARUMA = 2,			// ヒヒダルマ ※新規
	FORM_MAX_BASURAO = 2,					// バスラオ ※新規
	/* 合計: 72 */

	FORM_ARRAY_LEN = 9,		///< フォルムフラグ配列の大きさ 9 * 8 = 72まで大丈夫

	// 色定義
	COLOR_NORMAL = 0,		// 通常カラー	
	COLOR_RARE,					// レアカラー
	COLOR_MAX,

	TEXTVER_VER_MAX = 7,				///< 言語種類数		日/英/仏/伊/独/西/韓
	TEXTVER_ARRAY_LEN = 432,		///< 言語フラグ		ポケモン数 * 言語数 / 8 = 493 * 7 / 8 = 432
};


//----------------------------------------------------------
/**
 * @brief 自分状態データ型定義
 */
//----------------------------------------------------------
struct ZUKAN_SAVEDATA {
  u32 zukan_magic;					///< マジックナンバー

	u32	zenkoku_flag:1;				///< 全国図鑑保持フラグ
	u32	ver_up_flg:10;				///< バーションアップフラグ
	u32	zukan_mode:1;					///< 閲覧中の図鑑モード
	u32	defaultMonsNo:10;			///< 閲覧中のポケモン番号
	u32	shortcutMonsNo:10;		///< Ｙ登録されたポケモン番号（諸事情により未使用）

  u32 get_flag[POKEZUKAN_ARRAY_LEN];    ///< 捕まえたフラグ用ワーク

  u32 sex_flag[SEE_FLAG_MAX][POKEZUKAN_ARRAY_LEN];		///< オスメスフラグ用ワーク
	u32	draw_sex[SEE_FLAG_MAX][POKEZUKAN_ARRAY_LEN];		///< 閲覧中オスメスフラグ

  u8	form_flag[COLOR_MAX][FORM_ARRAY_LEN];						///< フォルムフラグ用ワーク
	u8	draw_form[COLOR_MAX][FORM_ARRAY_LEN];						///< 閲覧中フォルムフラグ

  u8 TextVersionUp[TEXTVER_ARRAY_LEN];	///< 言語バージョンアップマスク

  u32 PachiRandom;            ///< パッチール用個性乱数保持ワーク
};

