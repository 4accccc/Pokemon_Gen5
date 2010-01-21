//============================================================================================
/**
 * @file		vs_multi_list.c
 * @brief		戦闘ポケモンリスト 共通定義
 * @author	Hiroyuki Nakamura
 * @date		10.01.15
 *
 *	モジュール名：BPL_COMM
 */
//============================================================================================
#pragma	once

// パレット定義
#define	BPL_COMM_PAL_NN_PLATE	( 0 )		///< パレット00：プレート（通常）
#define	BPL_COMM_PAL_MN_PLATE	( 5 )		///< パレット01：プレート（マルチ）
#define	BPL_COMM_PAL_DN_PLATE	( 7 )		///< パレット02：プレート（瀕死）
#define	BPL_COMM_PAL_HPGAGE		( 9 )		///< パレット09：HPゲージ

#define	BPL_PAL_B_RED		( 10 )		///< パレット09：ボタン（赤）
#define	BPL_PAL_B_GREEN		( 12 )		///< パレット09：ボタン（緑）

#define	BPL_PAL_SYS_FONT	( 13 )		///< パレット13：システムフォント
#define	BPL_PAL_TALK_WIN	( 14 )		///< パレット14：会話ウィンドウ
#define	BPL_PAL_TALK_FONT	( BPL_PAL_SYS_FONT )		///< パレット15：会話フォント

// プレートサイズ
#define	BPL_COMM_BSX_PLATE		( 16 )
#define	BPL_COMM_BSY_PLATE		( 6 )

// プレート配置位置
#define	BPL_COMM_SCR_POKE1_PX		( 0 )
#define	BPL_COMM_SCR_POKE1_PY		( 0 )
#define	BPL_COMM_SCR_POKE2_PX		( 16 )
#define	BPL_COMM_SCR_POKE2_PY		( 1 )
#define	BPL_COMM_SCR_POKE3_PX		( 0 )
#define	BPL_COMM_SCR_POKE3_PY		( 6 )
#define	BPL_COMM_SCR_POKE4_PX		( 16 )
#define	BPL_COMM_SCR_POKE4_PY		( 7 )
#define	BPL_COMM_SCR_POKE5_PX		( 0 )
#define	BPL_COMM_SCR_POKE5_PY		( 12 )
#define	BPL_COMM_SCR_POKE6_PX		( 16 )
#define	BPL_COMM_SCR_POKE6_PY		( 13 )

// ＢＭＰ
// ポケモン１
#define	BPL_COMM_WIN_P1_POKE1_PX		( 0 )
#define	BPL_COMM_WIN_P1_POKE1_PY		( 0 )
#define	BPL_COMM_WIN_P1_POKE1_SX		( 15 )
#define	BPL_COMM_WIN_P1_POKE1_SY		( 5 )
#define	BPL_COMM_WIN_P1_POKE1_PAL	( BPL_COMM_PAL_HPGAGE )
// ポケモン２
#define	BPL_COMM_WIN_P1_POKE2_PX		( 16 )
#define	BPL_COMM_WIN_P1_POKE2_PY		( 1 )
#define	BPL_COMM_WIN_P1_POKE2_SX		( 15 )
#define	BPL_COMM_WIN_P1_POKE2_SY		( 5 )
#define	BPL_COMM_WIN_P1_POKE2_PAL	( BPL_COMM_PAL_HPGAGE )
// ポケモン３
#define	BPL_COMM_WIN_P1_POKE3_PX		( 0 )
#define	BPL_COMM_WIN_P1_POKE3_PY		( 6 )
#define	BPL_COMM_WIN_P1_POKE3_SX		( 15 )
#define	BPL_COMM_WIN_P1_POKE3_SY		( 5 )
#define	BPL_COMM_WIN_P1_POKE3_PAL	( BPL_COMM_PAL_HPGAGE )
// ポケモン４
#define	BPL_COMM_WIN_P1_POKE4_PX		( 16 )
#define	BPL_COMM_WIN_P1_POKE4_PY		( 7 )
#define	BPL_COMM_WIN_P1_POKE4_SX		( 15 )
#define	BPL_COMM_WIN_P1_POKE4_SY		( 5 )
#define	BPL_COMM_WIN_P1_POKE4_PAL	( BPL_COMM_PAL_HPGAGE )
// ポケモン５
#define	BPL_COMM_WIN_P1_POKE5_PX		( 0 )
#define	BPL_COMM_WIN_P1_POKE5_PY		( 12 )
#define	BPL_COMM_WIN_P1_POKE5_SX		( 15 )
#define	BPL_COMM_WIN_P1_POKE5_SY		( 5 )
#define	BPL_COMM_WIN_P1_POKE5_PAL	( BPL_COMM_PAL_HPGAGE )
// ポケモン６
#define	BPL_COMM_WIN_P1_POKE6_PX		( 16 )
#define	BPL_COMM_WIN_P1_POKE6_PY		( 13 )
#define	BPL_COMM_WIN_P1_POKE6_SX		( 15 )
#define	BPL_COMM_WIN_P1_POKE6_SY		( 5 )
#define	BPL_COMM_WIN_P1_POKE6_PAL	( BPL_COMM_PAL_HPGAGE )

// 名前表示座標
#define BPL_COMM_P1_NAME_PX		( 32 )
#define BPL_COMM_P1_NAME_PY		( 8-1 )
// ＨＰ表示座標
#define BPL_COMM_P1_HP_PX			( 92 )
#define BPL_COMM_P1_HP_PY		  ( 32 )
#define BPL_COMM_P1_HP_SX			( 24 )
#define BPL_COMM_P1_HP_SY			( 8 )
// ＨＰゲージ表示座標
#define BPL_COMM_P1_HPGAGE_PX	( 48+16 )
#define BPL_COMM_P1_HPGAGE_PY	( 24 )
#define BPL_COMM_P1_HPGAGE_SX	( 64 )
#define BPL_COMM_P1_HPGAGE_SY	( 8 )
// Ｌｖ表示座標
#define BPL_COMM_P1_LV_PX			( 8 )
#define BPL_COMM_P1_LV_PY			( 32 )

#define	BPL_COMM_HP_DOTTO_MAX				( 48 )		// HPゲージのドットサイズ

#define BPL_COMM_HP_GAGE_COL_G1  ( 1 ) // HPゲージカラー緑１
#define BPL_COMM_HP_GAGE_COL_G2  ( 2 ) // HPゲージカラー緑２
#define BPL_COMM_HP_GAGE_COL_Y1  ( 3 ) // HPゲージカラー黄１
#define BPL_COMM_HP_GAGE_COL_Y2  ( 4 ) // HPゲージカラー黄２
#define BPL_COMM_HP_GAGE_COL_R1  ( 5 ) // HPゲージカラー赤１
#define BPL_COMM_HP_GAGE_COL_R2  ( 6 ) // HPゲージカラー赤２

// ＯＢＪ
// ポケモンアイコン
#define	BPL_COMM_POKEICON_LPX		( 16 )
#define	BPL_COMM_POKEICON_RPX		( 144 )
#define	BPL_COMM_POKEICON1_PY		( 16 )
#define	BPL_COMM_POKEICON2_PY		( 24 )
#define	BPL_COMM_POKEICON3_PY		( 64 )
#define	BPL_COMM_POKEICON4_PY		( 72 )
#define	BPL_COMM_POKEICON5_PY		( 112 )
#define	BPL_COMM_POKEICON6_PY		( 120 )
// アイテムアイコン
#define	BPL_COMM_ITEMICON_X			( 8 )
#define	BPL_COMM_ITEMICON_Y			( 8 )
// 状態異常アイコン
#define	BPL_COMM_STATUSICON_X		( 12 )
#define	BPL_COMM_STATUSICON_Y		( 24 )
// ＨＰゲージ背景
#define	BPL_COMM_HPGAUGE_X			( 71 )
#define	BPL_COMM_HPGAUGE_Y			( 12 )
