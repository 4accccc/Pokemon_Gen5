//============================================================================================
/**
 * @file  zukan_savedata_local.h
 * @brief 図鑑セーブデータ内部参照定義
 * @author  mori / tamada GAME FREAK inc.
 * @date  2009.10.26
*/
//============================================================================================


enum {
  POKEZUKAN_ARRAY_LEN = 21, ///<ポケモン図鑑フラグ配列のおおきさ 21 * 32 = 672まで大丈夫

  POKEZUKAN_UNKNOWN_NUM = UNK_END,///<アンノーン数

  MAGIC_NUMBER = 0xbeefcafe,

  POKEZUKAN_DEOKISISU_MSK = 0xf,  // bitマスク  POKEZUKAN_ARRAY_LENの後ろ15bit分をデオキシスに使用してます。

  POKEZUKAN_TEXT_POSSIBLE  = MONSNO_ARUSEUSU, // 海外図鑑テキストが読める最大数（＝シンオウポケモンまで）

  POKEZUKAN_TEXTVER_UP_NUM = POKEZUKAN_TEXT_POSSIBLE + 1, //  見た目アップデートバッファ数  +1はパディング

	// 見た用定義
	SEE_FLAG_MALE = 0,
	SEE_FLAG_FEMALE,
	SEE_FLAG_M_RARE,
	SEE_FLAG_F_RARE,
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
	FORM_MAX_HIHIDARUMA = 2,			// シキジカ ※新規
	FORM_MAX_BASURAO = 2,					// バスラオ ※新規
	/* 合計: 72 */

	FORM_ARRAY_LEN = 9,		///< フォルムフラグ配列の大きさ 9 * 8 = 72まで大丈夫

	// 色定義
	COLOR_NORMAL = 0,
	COLOR_RARE,
	COLOR_MAX,

	TEXTVER_ARRAY_LEN = 370,		///< 言語フラグ
};

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

	{ MONSNO_BANBIINA, FORM_MAX_BANBIINA },				// バンビーナ ※新規
	{ MONSNO_SIKIZIKA, FORM_MAX_SIKIZIKA },				// シキジカ ※新規
	{ MONSNO_MERODHIA, FORM_MAX_MERODHIA },				// メロディア ※新規
	{ MONSNO_HIHIDARUMA, FORM_MAX_HIHIDARUMA },		// シキジカ ※新規
	{ MONSNO_BASURAO, FORM_MAX_BASURAO },					// バスラオ ※新規

	{ 0, 0 },
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
	u32	shortcutMonsNo:10;		///< Ｙ登録されたポケモン番号

  u32 get_flag[POKEZUKAN_ARRAY_LEN];    ///< 捕まえたフラグ用ワーク

  u32 sex_flag[SEE_FLAG_MAX][POKEZUKAN_ARRAY_LEN];		///< オスメスフラグ用ワーク
	u32	draw_sex[SEE_FLAG_MAX][POKEZUKAN_ARRAY_LEN];		///< 閲覧中オスメスフラグ
//	u32	draw_rare[COLOR_MAX][POKEZUKAN_ARRAY_LEN];			///< 閲覧中レアフラグ

  u8	form_flag[COLOR_MAX][FORM_ARRAY_LEN];						///< フォルムフラグ用ワーク
	u8	draw_form[COLOR_MAX][FORM_ARRAY_LEN];						///< 閲覧中フォルムフラグ
//	u8	draw_form_rare[COLOR_MAX][FORM_ARRAY_LEN];			///< 閲覧中フォルムレアフラグ

  u8 TextVersionUp[TEXTVER_ARRAY_LEN];	///< 言語バージョンアップマスク

  u32 PachiRandom;            ///< パッチール用個性乱数保持ワーク

/*
  u32 see_flag[POKEZUKAN_ARRAY_LEN];    ///<見つけたフラグ用ワーク

  u8 SiiusiTurn;              ///<シーウシ用見つけた順保持ワーク    2bit必要  1bit*2種類
  u8 SiidorugoTurn;           ///<シードルゴ用見つけた順保持ワーク  2bit必要  1bit*2種類
  u8 MinomuttiTurn;           ///<ミノムッチ用見つけた順保持ワーク  6bit必要  2bit*3種類
  u8 MinomesuTurn;            ///<ミノメス用見つけた順保持ワーク    6bit必要  2bit*3種類
  u8 UnknownTurn[ POKEZUKAN_UNKNOWN_NUM ];///<アンノーンを見つけた番号
  u8 UnknownGetTurn[ POKEZUKAN_UNKNOWN_NUM ];///<アンノーンを捕獲した番号
  u8 TextVersionUp[ POKEZUKAN_TEXTVER_UP_NUM ];///<言語バージョンアップマスク

  u8 GraphicVersionUp;          ///<グラフィックバージョン用機能拡張フラグ
  u8 TextVersionUpMasterFlag;       ///<言語バージョンアップMasterフラグ

  ///<ロトム用見つけた順番保持ワーク    18bit必要 3bit*6種類
  ///<シェイミ用見つけた順番保持ワーク  2bit必要  1bit*2種類
  ///<ギラティナ用見つけた順番保持ワーク  2bit必要  1bit*2種類
  u32 rotomu_turn;
  u8  syeimi_turn;
  u8  giratyina_turn;
  u8  pityuu_turn;  ///< ピチュー用見つけた順保持ワーク 2bit*3種類( ♂/♀/ギザミミ )
  u8  turn_pad;   ///< 余り
*/

};

