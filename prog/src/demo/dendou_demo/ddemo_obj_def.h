//============================================================================================
/**
 * @file		ddemo_obj_def.h
 * @brief		殿堂入りデモ画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		10.04.07
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

#define	DDEMOOBJ_ID_2ND_EFF_MAX		( 24 )		// シーン２で表示できる光ＯＢＪの最大数

// OBJ ID
enum {
	DDEMOOBJ_ID_POKE = 0,			// ポケモン（正面）
	DDEMOOBJ_ID_POKE_B,				// ポケモン（背面）
	DDEMOOBJ_ID_MES,
	DDEMOOBJ_ID_INFO,

	DDEMOOBJ_ID_EFF,

	DDEMOOBJ_ID_PLAYER_M,			// プレイヤー（メイン画面）
	DDEMOOBJ_ID_PLAYER_S,			// プレイヤー（サブ画面）
	DDEMOOBJ_ID_2ND_MES,
	DDEMOOBJ_ID_2ND_INFO,

	DDEMOOBJ_ID_2ND_EFF,

	DDEMOOBJ_ID_MAX = DDEMOOBJ_ID_2ND_EFF + DDEMOOBJ_ID_2ND_EFF_MAX
};

// キャラリソース
enum {
	DDEMOOBJ_CHRRES_POKE = 0,		// ポケモン（正面）
	DDEMOOBJ_CHRRES_POKE_B,			// ポケモン（背面）
	DDEMOOBJ_CHRRES_ETC,
	DDEMOOBJ_CHRRES_PLAYER_M,		// プレイヤー（メイン画面）
	DDEMOOBJ_CHRRES_PLAYER_S,		// プレイヤー（サブ画面）
	DDEMOOBJ_CHRRES_ETC_S,
	DDEMOOBJ_CHRRES_MAX
};

// パレットリソース
enum {
	DDEMOOBJ_PALRES_POKE = 0,		// ポケモン（正面）
	DDEMOOBJ_PALRES_POKE_B,			// ポケモン（背面）
	DDEMOOBJ_PALRES_ETC,
	DDEMOOBJ_PALRES_FOAM,
	DDEMOOBJ_PALRES_PLAYER_M,		// プレイヤー（メイン画面）
	DDEMOOBJ_PALRES_PLAYER_S,		// プレイヤー（サブ画面）
	DDEMOOBJ_PALRES_ETC_S,
	DDEMOOBJ_PALRES_MAX
};

// セルリソース
enum {
	DDEMOOBJ_CELRES_POKE = 0,		// ポケモン（正面）
	DDEMOOBJ_CELRES_POKE_B,			// ポケモン（背面）
	DDEMOOBJ_CELRES_ETC,
	DDEMOOBJ_CELRES_PLAYER_M,		// プレイヤー（メイン画面）
	DDEMOOBJ_CELRES_PLAYER_S,		// プレイヤー（サブ画面）
	DDEMOOBJ_CELRES_ETC_S,
	DDEMOOBJ_CELRES_MAX
};

// font OAM
enum {
	DDEMOOBJ_FOAM_MES = 0,
	DDEMOOBJ_FOAM_INFO,
	DDEMOOBJ_FOAM_MES2,
	DDEMOOBJ_FOAM_PLAYER,
	DDEMOOBJ_FOAM_MAX,
};
