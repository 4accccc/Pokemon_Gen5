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
	BOX2UI_INIT_ID_PTOUT_MAIN = 0,			// あずける・メイン
	BOX2UI_INIT_ID_PTOUT_BOXSEL,				// あずける・ボックス選択

	BOX2UI_INIT_ID_PTIN_MAIN,						// つれていく・メイン

	BOX2UI_INIT_ID_ARRANGE_MAIN,				// ボックス整理・メイン
	BOX2UI_INIT_ID_ARRANGE_MOVE,				// ボックス整理・ポケモン移動（ボックス間）
	BOX2UI_INIT_ID_ARRANGE_PARTY,				// ボックス整理・手持ちポケモン
	BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE,	// ボックス整理・ポケモン移動（手持ち）

	BOX2UI_INIT_ID_ITEM_MAIN,						// 持ち物整理・メイン
	BOX2UI_INIT_ID_ITEM_PARTY,					// 持ち物整理・手持ちポケモン

	BOX2UI_INIT_ID_MARKING,							// マーキング

	BOX2UI_INIT_ID_BOXTHEMA_CHG,				// ボックステーマ変更

	BOX2UI_INIT_ID_WALLPAPER_CHG,				// 壁紙変更

	BOX2UI_INIT_ID_BOXJUMP_MAIN,				// ボックスジャンプ

	BOX2UI_INIT_ID_BATTLEBOX_MAIN,			// バトルボックス・メイン
	BOX2UI_INIT_ID_BATTLEBOX_PARTY,			// バトルボックス・パーティ操作メイン

	BOX2UI_INIT_ID_SLEEP_MAIN,					// 寝かせる・メイン
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
	BOX2UI_ARRANGE_MAIN_CLOSE,		// 41: やめる

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
	BOX2UI_ARRANGE_PGT_STATUS,	// 40: ステータス
	BOX2UI_ARRANGE_PGT_RET,			// 41: 戻る

	// ボックス整理・手持ちポケモン
	BOX2UI_ARRANGE_PARTY_POKE1 = 0,	// 00: 手持ちポケモン
	BOX2UI_ARRANGE_PARTY_POKE2,
	BOX2UI_ARRANGE_PARTY_POKE3,
	BOX2UI_ARRANGE_PARTY_POKE4,
	BOX2UI_ARRANGE_PARTY_POKE5,
	BOX2UI_ARRANGE_PARTY_POKE6,

	BOX2UI_ARRANGE_PARTY_BOXLIST,		// 06:「ボックスリスト」
	BOX2UI_ARRANGE_PARTY_RETURN1,		// 07: 戻る１
	BOX2UI_ARRANGE_PARTY_RETURN2,		// 08: 戻る２

	BOX2UI_ARRANGE_PARTY_GET,				// 09: つかむ
	BOX2UI_ARRANGE_PARTY_STATUS,		// 10: ようすをみる
	BOX2UI_ARRANGE_PARTY_ITEM,			// 11: もちもの
	BOX2UI_ARRANGE_PARTY_MARKING,		// 12: マーキング
	BOX2UI_ARRANGE_PARTY_FREE,			// 13: にがす
	BOX2UI_ARRANGE_PARTY_CLOSE,			// 14: やめる

	// ボックス整理・「つかむ」トレイ移動（手持ち）
	BOX2UI_ARRANGE_PTGT_POKE1 = 0,	// 00: ポケモン
	BOX2UI_ARRANGE_PTGT_PARTY_POKE = BOX2UI_ARRANGE_PTGT_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: 手持ちポケモン
	BOX2UI_ARRANGE_PTGT_NAME = BOX2UI_ARRANGE_PTGT_PARTY_POKE + BOX2OBJ_POKEICON_MINE_MAX,	// 36: ボックス名
	BOX2UI_ARRANGE_PTGT_LEFT,			// 37: ボックス切り替え矢印（左）
	BOX2UI_ARRANGE_PTGT_RIGHT,		// 38: ボックス切り替え矢印（右）
	BOX2UI_ARRANGE_PTGT_BOXLIST,	// 39: ボックスリスト
	BOX2UI_ARRANGE_PTGT_STATUS,		// 40: ステータス
	BOX2UI_ARRANGE_PTGT_RET,			// 41: 戻る

	// バトルボックス・メイン操作
	BOX2UI_BATTLEBOX_MAIN_POKE1 = 0,	// 00: ポケモン
	BOX2UI_BATTLEBOX_MAIN_NAME = BOX2UI_BATTLEBOX_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: ボックス名
	BOX2UI_BATTLEBOX_MAIN_LEFT,			// 31: トレイ切り替え矢印・左
	BOX2UI_BATTLEBOX_MAIN_RIGHT,		// 32: トレイ切り替え矢印・右
	BOX2UI_BATTLEBOX_MAIN_PARTY,		// 33: バトルボックス
	BOX2UI_BATTLEBOX_MAIN_RETURN1,	// 34: 戻る１
	BOX2UI_BATTLEBOX_MAIN_RETURN2,	// 35: 戻る２

	BOX2UI_BATTLEBOX_MAIN_GET,			// 36: いどうする
	BOX2UI_BATTLEBOX_MAIN_STATUS,		// 37: ようすをみる
	BOX2UI_BATTLEBOX_MAIN_ITEM,			// 38: もちもの
	BOX2UI_BATTLEBOX_MAIN_CLOSE,		// 39: やめる

	// バトルボックス・パーティ操作
	BOX2UI_BATTLEBOX_PARTY_POKE1 = 0,	// 00: 手持ちポケモン
	BOX2UI_BATTLEBOX_PARTY_POKE2,
	BOX2UI_BATTLEBOX_PARTY_POKE3,
	BOX2UI_BATTLEBOX_PARTY_POKE4,
	BOX2UI_BATTLEBOX_PARTY_POKE5,
	BOX2UI_BATTLEBOX_PARTY_POKE6,

	BOX2UI_BATTLEBOX_PARTY_BOXLIST,		// 06:「ボックスリスト」
	BOX2UI_BATTLEBOX_PARTY_RETURN1,		// 07: 戻る１
	BOX2UI_BATTLEBOX_PARTY_RETURN2,		// 08: 戻る２

	BOX2UI_BATTLEBOX_PARTY_GET,				// 09: いどうする
	BOX2UI_BATTLEBOX_PARTY_STATUS,		// 10: ようすをみる
	BOX2UI_BATTLEBOX_PARTY_ITEM,			// 11: もちもの
	BOX2UI_BATTLEBOX_PARTY_CLOSE,			// 12: やめる

	// つれていく・メイン操作
	BOX2UI_PTIN_MAIN_POKE1 = 0,		// 00: ポケモン
	BOX2UI_PTIN_MAIN_NAME = BOX2UI_PTIN_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: ボックス名
	BOX2UI_PTIN_MAIN_LEFT,				// 31: トレイ切り替え矢印・左
	BOX2UI_PTIN_MAIN_RIGHT,				// 32: トレイ切り替え矢印・右
	BOX2UI_PTIN_MAIN_RETURN1,			// 33: 戻る１
	BOX2UI_PTIN_MAIN_RETURN2,			// 34: 戻る２

	BOX2UI_PTIN_MAIN_TSURETEIKU,	// 35: つれていく
	BOX2UI_PTIN_MAIN_STATUS,			// 36: ようすをみる
	BOX2UI_PTIN_MAIN_MARKING,			// 37: マーキング
	BOX2UI_PTIN_MAIN_FREE,				// 38: にがす
	BOX2UI_PTIN_MAIN_CLOSE,				// 39: とじる

	// 預ける・メイン操作
	BOX2UI_PTOUT_MAIN_POKE1 = 0,	// 00: 手持ちポケモン
	BOX2UI_PTOUT_MAIN_POKE2,
	BOX2UI_PTOUT_MAIN_POKE3,
	BOX2UI_PTOUT_MAIN_POKE4,
	BOX2UI_PTOUT_MAIN_POKE5,
	BOX2UI_PTOUT_MAIN_POKE6,

	BOX2UI_PTOUT_MAIN_RETURN1,		// 06: 戻る１
	BOX2UI_PTOUT_MAIN_RETURN2,		// 07: 戻る２

	BOX2UI_PTOUT_MAIN_AZUKERU,		// 08: あずける
	BOX2UI_PTOUT_MAIN_STATUS,			// 09: ようすをみる
	BOX2UI_PTOUT_MAIN_MARKING,		// 10: マーキング
	BOX2UI_PTOUT_MAIN_FREE,				// 11: にがす
	BOX2UI_PTOUT_MAIN_CLOSE,			// 12: とじる

	// 預ける場所選択
	BOX2UI_PTOUT_BOXSEL_NAME = 0,	// 00: トレイ名
	BOX2UI_PTOUT_BOXSEL_TRAY,			// 01: トレイ
	BOX2UI_PTOUT_BOXSEL_LEFT,			// 02: トレイ切り替え矢印・左
	BOX2UI_PTOUT_BOXSEL_RIGHT,		// 03: トレイ切り替え矢印・右
	BOX2UI_PTOUT_BOXSEL_RETURN,		// 04: 戻る


	// 持ち物整理・メイン操作
	BOX2UI_ITEM_MAIN_POKE1 = 0,		// 00: ポケモン
	BOX2UI_ITEM_MAIN_NAME = BOX2UI_ITEM_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,		// 30: ボックス名
	BOX2UI_ITEM_MAIN_LEFT,				// 31: トレイ切り替え矢印・左
	BOX2UI_ITEM_MAIN_RIGHT,				// 32: トレイ切り替え矢印・右
	BOX2UI_ITEM_MAIN_PARTY,				// 33: 手持ちポケモン
	BOX2UI_ITEM_MAIN_RETURN1,			// 34: 戻る１
	BOX2UI_ITEM_MAIN_RETURN2,			// 35: 戻る２

	BOX2UI_ITEM_MAIN_MENU1,				// 36: いどうする
	BOX2UI_ITEM_MAIN_MENU2,				// 37: バッグへ or もたせる
	BOX2UI_ITEM_MAIN_CLOSE,				// 38: とじる

	// 旧
	BOX2UI_ITEM_A_MAIN_CHANGE,		// 34: 持ち物整理

	// 持ち物整理・手持ちポケモン
	BOX2UI_ITEM_PARTY_POKE1 = 0,	// 00: 手持ちポケモン
	BOX2UI_ITEM_PARTY_POKE2,
	BOX2UI_ITEM_PARTY_POKE3,
	BOX2UI_ITEM_PARTY_POKE4,
	BOX2UI_ITEM_PARTY_POKE5,
	BOX2UI_ITEM_PARTY_POKE6,

	BOX2UI_ITEM_PARTY_BOXLIST,		// 06: ボックスリスト
	BOX2UI_ITEM_PARTY_RETURN1,		// 07: 戻る１
	BOX2UI_ITEM_PARTY_RETURN2,		// 08: 戻る２

	BOX2UI_ITEM_PARTY_MENU1,			// 09: いどうする
	BOX2UI_ITEM_PARTY_MENU2,			// 10: バッグへ or もたせる
	BOX2UI_ITEM_PARTY_CLOSE,			// 11: とじる

	// 寝かせる
	BOX2UI_SLEEP_MAIN_POKE1 = 0,	// 00: ポケモン
	BOX2UI_SLEEP_MAIN_NAME = BOX2UI_SLEEP_MAIN_POKE1 + BOX2OBJ_POKEICON_TRAY_MAX,	// 30: ボックス名
	BOX2UI_SLEEP_MAIN_LEFT,			// 31: トレイ切り替え矢印・左
	BOX2UI_SLEEP_MAIN_RIGHT,		// 32: トレイ切り替え矢印・右
	BOX2UI_SLEEP_MAIN_RETURN,		// 33: 戻る

	BOX2UI_SLEEP_MAIN_SET,			// 34: ねかせる
	BOX2UI_SLEEP_MAIN_CLOSE,		// 35: やめる

	// ボックステーマ変更
	BOX2UI_BOXTHEMA_MENU1 = 0,		// 00: ジャンプする
	BOX2UI_BOXTHEMA_MENU2,				// 01: かべがみ
	BOX2UI_BOXTHEMA_MENU3,				// 02: なまえ
	BOX2UI_BOXTHEMA_MENU4,				// 03: やめる

	// ボックスジャンプ
	BOX2UI_BOXJUMP_TRAY1 = 0,		// 00: トレイアイコン
	BOX2UI_BOXJUMP_TRAY2,				// 01: トレイアイコン
	BOX2UI_BOXJUMP_TRAY3,				// 02: トレイアイコン
	BOX2UI_BOXJUMP_TRAY4,				// 03: トレイアイコン
	BOX2UI_BOXJUMP_TRAY5,				// 04: トレイアイコン
	BOX2UI_BOXJUMP_TRAY6,				// 05: トレイアイコン
	BOX2UI_BOXJUMP_RET,					// 06: 戻る

	// 壁紙選択
	BOX2UI_WALLPAPER_MENU1 = 0,		// 00: メニュー１
	BOX2UI_WALLPAPER_MENU2,				// 01: メニュー２
	BOX2UI_WALLPAPER_MENU3,				// 02: メニュー３
	BOX2UI_WALLPAPER_MENU4,				// 03: メニュー４
	BOX2UI_WALLPAPER_MENU5,				// 04: もどる

	// マーキング
	BOX2UI_MARKING_MARK1 = 0,		// 00: ●
	BOX2UI_MARKING_MARK2,				// 01: ▲
	BOX2UI_MARKING_MARK3,				// 02: ■
	BOX2UI_MARKING_MARK4,				// 03: ハート
	BOX2UI_MARKING_MARK5,				// 04: ★
	BOX2UI_MARKING_MARK6,				// 05: ◆
	BOX2UI_MARKING_ENTER,				// 06:「けってい」
	BOX2UI_MARKING_CANCEL,			// 07:「やめる」


/*
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
*/
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動ワーク初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動ワーク切り替え
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			カーソル移動データ番号
 * @param		pos			カーソル初期位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveChange( BOX2_SYS_WORK * syswk, u32 id, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動ワーク削除
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置にカーソルＯＢＪを配置
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_PutHandCursor( BOX2_SYS_WORK * syswk, int pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_PartyOutMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」トレイ選択：指定位置からボックス名への移動処理セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_PartyOutBoxSelCursorMoveSet( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをあずける」トレイ選択：ボックス名から指定位置への移動処理セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_PartyOutBoxSelCursorMovePutSet( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンをつれていく」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_PartyInMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」メイン操作
 *
 * @param		syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxArrangeMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動時にアイコンを掴んでいるときの操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_ArrangePokeGetMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」手持ちポケモン操作
 *
 * @param		syswk	ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxArrangePartyMoveMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ボックスをせいりする」ポケモン移動時にアイコンを掴んでいるときの操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_ArrangePartyPokeGetMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxItemArrangeMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「もちものをせいりする」手持ちポケモン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BoxItemArrangePartyMoveMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BattleBoxMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「バトルボックス」パーティポケモン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_BattleBoxPartyMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「寝かすポケモンをえらぶ」メイン操作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_SleepMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動をVBLANKで行うためのワーク設定（位置指定）
 *
 * @param		appwk			ボックス画面アプリワーク
 * @param		now_pos		現在の位置
 * @param		old_pos		前回の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveVFuncWorkSet( BOX2_APP_WORK * appwk, int now_pos, int old_pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動をVBLANKで行うためのワーク設定（座標指定）
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		nx			移動位置Ｘ座標
 * @param		ny			移動位置Ｙ座標
 * @param		ox			前回位置Ｘ座標
 * @param		oy			前回位置Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2UI_CursorMoveVFuncWorkSetDirect( BOX2_APP_WORK * appwk, u8 nx, u8 ny, u8 ox, u8 oy );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイのポケモン（トリガ）
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckTrgTrayPoke(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイのポケモン（座標指定）
 *
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckPosTrayPoke( u32 x, u32 y );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：手持ちポケモン（左）
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckTrgPartyPokeLeft(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：手持ちポケモン（右）
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckTrgPartyPokeRight(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイ切り替え矢印
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckContTrayArrow(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイアイコン上下スクロール
 *
 * @param		none
 *
 * @retval	"GFL_UI_TP_HIT_NONE = タッチなし"
 * @retval	"GFL_UI_TP_HIT_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckContTrayIconScroll(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイアイコンスクロール開始
 *
 * @param		x			Ｘ座標格納場所
 * @param		y			Ｙ座標格納場所
 *
 * @retval	"TRUE = タッチあり"
 * @retval	"FALSE = タッチなし"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2UI_HitCheckTrayScrollStart( u32 * x, u32 * y );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：トレイアイコンスクロール継続
 *
 * @param		x			Ｘ座標格納場所
 * @param		y			Ｙ座標格納場所
 *
 * @retval	"TRUE = タッチあり"
 * @retval	"FALSE = タッチなし"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2UI_HitCheckTrayScroll( u32 * x, u32 * y );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定座標のトレイアイコンを取得
 *
 * @param		x			Ｘ座標
 * @param		y			Ｙ座標
 *
 * @retval	"BOX2MAIN_GETPOS_NONE = トレイアイコンなし"
 * @retval	"BOX2MAIN_GETPOS_NONE != それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2UI_HitCheckTrayIcon( u32 x, u32 y );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチチェック：手持ちフレーム（左）
 *
 * @param		none
 *
 * @retval	"TRUE = タッチあり"
 * @retval	"FALSE = タッチなし"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2UI_HitCheckPartyFrameLeft(void);
