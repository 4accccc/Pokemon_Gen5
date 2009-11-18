//============================================================================================
/**
 * @file		box2_ui.h
 * @brief		ボックス画面 キータッチ関連
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2_UI
 */
//============================================================================================
#pragma once


//============================================================================================
//	定数定義
//============================================================================================

// 初期化ＩＤ
enum {
	BOX2UI_INIT_ID_PTOUT_MAIN = 0,		// あずける・メイン
	BOX2UI_INIT_ID_PTOUT_BOXSEL,		// あずける・ボックス選択

	BOX2UI_INIT_ID_PTIN_MAIN,			// つれていく・メイン

	BOX2UI_INIT_ID_ARRANGE_MAIN,		// ボックス整理・メイン
	BOX2UI_INIT_ID_ARRANGE_MOVE,		// ボックス整理・ポケモン移動
	BOX2UI_INIT_ID_ARRANGE_PARTY,		// ボックス整理・手持ちポケモン

	BOX2UI_INIT_ID_ITEM_A_MAIN,		// 持ち物整理・メイン
	BOX2UI_INIT_ID_ITEM_A_PARTY,		// 持ち物整理・手持ちポケモン

	BOX2UI_INIT_ID_MARKING,			// マーキング
	BOX2UI_INIT_ID_BOXTHEMA_CHG,		// ボックステーマ変更
	BOX2UI_INIT_ID_WALLPAPER_CHG,		// 壁紙変更
};

// ボタンＩＤ
enum {
	// ボックス整理・メイン操作
	BOX2UI_ARRANGE_MAIN_POKE1 = 0,	// 00: ポケモン
	BOX2UI_ARRANGE_MAIN_NAME = BOX2UI_ARRANGE_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: ボックス名
	BOX2UI_ARRANGE_MAIN_LEFT,			// 31: トレイ切り替え矢印・左
	BOX2UI_ARRANGE_MAIN_RIGHT,		// 32: トレイ切り替え矢印・右
	BOX2UI_ARRANGE_MAIN_PARTY,		// 33: 手持ちポケモン
	BOX2UI_ARRANGE_MAIN_RETURN1,	// 34: 戻る１
	BOX2UI_ARRANGE_MAIN_RETURN2,	// 35: 戻る２

	BOX2UI_ARRANGE_MAIN_GET,			// 36: つかむ
	BOX2UI_ARRANGE_MAIN_STATUS,		// 37: ようすをみる
	BOX2UI_ARRANGE_MAIN_ITEM,			// 38: もちもの
	BOX2UI_ARRANGE_MAIN_MARKING,	// 39: マーキング
	BOX2UI_ARRANGE_MAIN_FREE,			// 40: にがす
	BOX2UI_ARRANGE_MAIN_CLOSE,		// 41: とじる

	// ボックス整理・「つかむ」トレイ移動
	BOX2UI_ARRANGE_PGT_POKE1 = 0,	// 00: ポケモン
	BOX2UI_ARRANGE_PGT_NAME = BOX2UI_ARRANGE_PGT_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,		// 30: ボックス名
	BOX2UI_ARRANGE_PGT_LEFT,		// 31: ボックス切り替え矢印（左）
	BOX2UI_ARRANGE_PGT_RIGHT,		// 32: ボックス切り替え矢印（右）

	BOX2UI_ARRANGE_PGT_TRAY1,		// 33: トレイアイコン
	BOX2UI_ARRANGE_PGT_TRAY2,		// 34: トレイアイコン
	BOX2UI_ARRANGE_PGT_TRAY3,		// 35: トレイアイコン
	BOX2UI_ARRANGE_PGT_TRAY4,		// 36: トレイアイコン
	BOX2UI_ARRANGE_PGT_TRAY5,		// 37: トレイアイコン
	BOX2UI_ARRANGE_PGT_TRAY6,		// 38: トレイアイコン

	BOX2UI_ARRANGE_PGT_PARTY,		// 39: 手持ちポケモン
	BOX2UI_ARRANGE_PGT_STTUS,		// 40: ステータス
	BOX2UI_ARRANGE_PGT_RET,			// 41: 戻る













	// 預ける・メイン操作
	BOX2UI_PTOUT_MAIN_POKE1 = 0,	// 手持ちポケモン
	BOX2UI_PTOUT_MAIN_POKE2,
	BOX2UI_PTOUT_MAIN_POKE3,
	BOX2UI_PTOUT_MAIN_POKE4,
	BOX2UI_PTOUT_MAIN_POKE5,
	BOX2UI_PTOUT_MAIN_POKE6,

	BOX2UI_PTOUT_MAIN_RETURN,		//「もどる」

	BOX2UI_PTOUT_MAIN_AZUKERU,		//「あずける」
	BOX2UI_PTOUT_MAIN_STATUS,		//「ようすをみる」
	BOX2UI_PTOUT_MAIN_MARKING,		//「マーキング」
	BOX2UI_PTOUT_MAIN_FREE,		//「にがす」
	BOX2UI_PTOUT_MAIN_CLOSE,		//「とじる」

	// 預ける場所選択
	BOX2UI_PTOUT_BOXSEL_TRAY1 = 0,	// トレイアイコン
	BOX2UI_PTOUT_BOXSEL_TRAY2,		// トレイアイコン
	BOX2UI_PTOUT_BOXSEL_TRAY3,		// トレイアイコン
	BOX2UI_PTOUT_BOXSEL_TRAY4,		// トレイアイコン
	BOX2UI_PTOUT_BOXSEL_TRAY5,		// トレイアイコン
	BOX2UI_PTOUT_BOXSEL_TRAY6,		// トレイアイコン

	BOX2UI_PTOUT_BOXSEL_LEFT,		// トレイ矢印（左）
	BOX2UI_PTOUT_BOXSEL_RIGHT,		// トレイ矢印（右）

	BOX2UI_PTOUT_BOXSEL_ENTER,		//「ポケモンをあずける」
	BOX2UI_PTOUT_BOXSEL_CANCEL,	//「やめる」

	// つれていく・メイン操作
	BOX2UI_PTIN_MAIN_POKE1 = 0,	// 00: ポケモン
	BOX2UI_PTIN_MAIN_NAME = BOX2UI_PTIN_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: ボックス名
	BOX2UI_PTIN_MAIN_LEFT,			// 31: トレイ切り替え矢印・左
	BOX2UI_PTIN_MAIN_RIGHT,		// 32: トレイ切り替え矢印・右
	BOX2UI_PTIN_MAIN_RETURN,		// 33: もどる

	BOX2UI_PTIN_MAIN_TSURETEIKU,	// 34: つれていく
	BOX2UI_PTIN_MAIN_STATUS,		// 35: ようすをみる
	BOX2UI_PTIN_MAIN_MARKING,		// 36: マーキング
	BOX2UI_PTIN_MAIN_FREE,			// 37: にがす
	BOX2UI_PTIN_MAIN_CLOSE,		// 38: とじる

	// ボックス整理・ポケモン移動
	BOX2UI_ARRANGE_MOVE_POKE1 = 0,	// 00: ポケモン
	BOX2UI_ARRANGE_MOVE_PARTY1 = BOX2UI_ARRANGE_MOVE_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: 手持ちポケモン
	BOX2UI_ARRANGE_MOVE_RETURN = BOX2UI_ARRANGE_MOVE_PARTY1 + BOX2OBJ_POKEICON_MINE_MAX,	// 36: やめる

	BOX2UI_ARRANGE_MOVE_TRAY1,		// 37: トレイアイコン
	BOX2UI_ARRANGE_MOVE_TRAY2,		// 38: トレイアイコン
	BOX2UI_ARRANGE_MOVE_TRAY3,		// 39: トレイアイコン
	BOX2UI_ARRANGE_MOVE_TRAY4,		// 40: トレイアイコン
	BOX2UI_ARRANGE_MOVE_TRAY5,		// 41: トレイアイコン
	BOX2UI_ARRANGE_MOVE_TRAY6,		// 42: トレイアイコン

	BOX2UI_ARRANGE_MOVE_LEFT,		// 43: トレイ矢印（左）
	BOX2UI_ARRANGE_MOVE_RIGHT,		// 44: トレイ矢印（右）

	BOX2UI_ARRANGE_MOVE_CHANGE,	// 45: ボックスをきりかえる

	// ボックス整理・手持ちポケモン
	BOX2UI_ARRANGE_PARTY_POKE1 = 0,	// 手持ちポケモン
	BOX2UI_ARRANGE_PARTY_POKE2,
	BOX2UI_ARRANGE_PARTY_POKE3,
	BOX2UI_ARRANGE_PARTY_POKE4,
	BOX2UI_ARRANGE_PARTY_POKE5,
	BOX2UI_ARRANGE_PARTY_POKE6,

	BOX2UI_ARRANGE_PARTY_CHANGE,		//「いれかえ」
	BOX2UI_ARRANGE_PARTY_RETURN,		//「もどる」

	BOX2UI_ARRANGE_PARTY_MOVE,			//「ポケモンいどう」（メニュー）
	BOX2UI_ARRANGE_PARTY_STATUS,		//「ようすをみる」
	BOX2UI_ARRANGE_PARTY_ITEM,			//「もちもの」
	BOX2UI_ARRANGE_PARTY_MARKING,		//「マーキング」
	BOX2UI_ARRANGE_PARTY_FREE,			//「にがす」

	BOX2UI_ARRANGE_PARTY_CLOSE,		//「とじる」

	// 持ち物整理・メイン操作
	BOX2UI_ITEM_A_MAIN_POKE1 = 0,	// 00: ポケモン
	BOX2UI_ITEM_A_MAIN_NAME = BOX2UI_ITEM_A_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: ボックス名
	BOX2UI_ITEM_A_MAIN_LEFT,		// 31: トレイ切り替え矢印・左
	BOX2UI_ITEM_A_MAIN_RIGHT,		// 32: トレイ切り替え矢印・右
	BOX2UI_ITEM_A_MAIN_PARTY,		// 33: 手持ちポケモン
	BOX2UI_ITEM_A_MAIN_CHANGE,		// 34: 持ち物整理
	BOX2UI_ITEM_A_MAIN_RETURN,		// 35: もどる

	BOX2UI_ITEM_A_MAIN_MENU1,		// 36: バッグへ or もたせる
	BOX2UI_ITEM_A_MAIN_CLOSE,		// 37: とじる

	// 持ち物整理・手持ちポケモン
	BOX2UI_ITEM_A_PARTY_POKE1 = 0,	// 手持ちポケモン
	BOX2UI_ITEM_A_PARTY_POKE2,
	BOX2UI_ITEM_A_PARTY_POKE3,
	BOX2UI_ITEM_A_PARTY_POKE4,
	BOX2UI_ITEM_A_PARTY_POKE5,
	BOX2UI_ITEM_A_PARTY_POKE6,

	BOX2UI_ITEM_A_PARTY_CHANGE,	//「いれかえ」
	BOX2UI_ITEM_A_PARTY_RETURN,	//「もどる」

	BOX2UI_ITEM_A_PARTY_MENU1,		// バッグへ or もたせる
	BOX2UI_ITEM_A_PARTY_CLOSE,		//「とじる」

	// マーキング
	BOX2UI_MARKING_MARK1 = 0,		// 00: ●
	BOX2UI_MARKING_MARK2,			// 01: ▲
	BOX2UI_MARKING_MARK3,			// 02: ■
	BOX2UI_MARKING_MARK4,			// 03: ハート
	BOX2UI_MARKING_MARK5,			// 04: ★
	BOX2UI_MARKING_MARK6,			// 05: ◆
	BOX2UI_MARKING_ENTER,			// 06:「けってい」
	BOX2UI_MARKING_CANCEL,			// 07:「やめる」

	// ボックステーマ変更
	BOX2UI_BOXTHEMA_CHG_TRAY1 = 0,		// 00: トレイアイコン
	BOX2UI_BOXTHEMA_CHG_TRAY2,			// 01: トレイアイコン
	BOX2UI_BOXTHEMA_CHG_TRAY3,			// 02: トレイアイコン
	BOX2UI_BOXTHEMA_CHG_TRAY4,			// 03: トレイアイコン
	BOX2UI_BOXTHEMA_CHG_TRAY5,			// 04: トレイアイコン
	BOX2UI_BOXTHEMA_CHG_TRAY6,			// 05: トレイアイコン

	BOX2UI_BOXTHEMA_CHG_LEFT,			// 06: トレイ矢印（左）
	BOX2UI_BOXTHEMA_CHG_RIGHT,			// 07: トレイ矢印（右）

	BOX2UI_BOXTHEMA_CHG_MOVE,			// 08: ボックスをきりかえる
	BOX2UI_BOXTHEMA_CHG_WP,			// 09: かべがみ
	BOX2UI_BOXTHEMA_CHG_NANE,			// 10: なまえ
	BOX2UI_BOXTHEMA_CHG_CANCEL,		// 11: やめる

	// 壁紙変更
	BOX2UI_WALLPAPER_CHG_ICON1 = 0,	// 00: 壁紙アイコン
	BOX2UI_WALLPAPER_CHG_ICON2,		// 01: 壁紙アイコン
	BOX2UI_WALLPAPER_CHG_ICON3,		// 02: 壁紙アイコン
	BOX2UI_WALLPAPER_CHG_ICON4,		// 03: 壁紙アイコン

	BOX2UI_WALLPAPER_CHG_LEFT,			// 04: トレイ矢印（左）
	BOX2UI_WALLPAPER_CHG_RIGHT,		// 05: トレイ矢印（右）

	BOX2UI_WALLPAPER_CHG_ENTER,		// 06: 変更
	BOX2UI_WALLPAPER_CHG_CANCEL,		// 07: キャンセル
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動ワーク初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動ワーク切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	id		カーソル移動データ番号
 * @param	pos		カーソル初期位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveChange( BOX2_SYS_WORK * syswk, u32 id, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動ワーク削除
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンをあずける」メイン操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_PartyOutMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンをつれていく」メイン操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_PartyInMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをせいりする」メイン操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxArrangeMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをせいりする」ポケモン移動操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxArrangePokeMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをせいりする」ポケモン移動時にアイコンを掴んでいるときの操作
 *
 * @param	syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_ArrangePokeGetMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをせいりする」手持ちポケモン操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxArrangePartyMoveMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「もちものをせいりする」メイン操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxItemArrangeMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「もちものをせいりする」手持ちポケモン操作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxItemArrangePartyMoveMain( BOX2_SYS_WORK * syswk );


//--------------------------------------------------------------------------------------------
/**
 * 「もちものをせいりする」持ち物整理時にアイコンを掴んでいるときの操作
 *
 * @param	syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 *
 *	※ボックス整理・ポケモン移動のワークを使用
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxItemArrangeMoveHand( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動をVBLANKで行うためのワーク設定
 *
 * @param	appwk		ボックス画面アプリワーク
 * @param	now_pos		現在の位置
 * @param	old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveVFuncWorkSet( BOX2_APP_WORK * appwk, int now_pos, int old_pos );

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンチェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 起動可"
 * @retval	"FALSE = 起動不可"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2UI_YStatusButtonCheck( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * タッチチェック：トレイのポケモン
 *
 * @param	none
 *
 * @return	GFL_UI_TP_HIT_NONE = 触っていない
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitTrayPokeTrg(void);

//--------------------------------------------------------------------------------------------
/**
 * タッチチェック：手持ちポケモン（左）
 *
 * @param	none
 *
 * @return	GFL_UI_TP_HIT_NONE = 触っていない
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckTrgPartyPokeLeft(void);

//--------------------------------------------------------------------------------------------
/**
 * タッチチェック：手持ちポケモン（右）
 *
 * @param	none
 *
 * @return	GFL_UI_TP_HIT_NONE = 触っていない
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckTrgPartyPokeRight(void);


extern u32 BOX2UI_HitCheckContTrayArrow(void);


extern BOOL BOX2UI_HitCheckTrayScroll( u32 * x, u32 * y );

