//============================================================================================
/**
 * @file		box2_bmp.h
 * @brief		ボックス画面 BMP関連
 * @author	Hiroyuki Nakamura
 * @date		08.05.13
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================
/*
// メニューウィンドウ４つ分のサイズ
#define	BOX2BMP_POKEMENU_SX		( 11 )
#define	BOX2BMP_POKEMENU_SY		( 3 )

//「てもちポケモン」サイズ
#define	BOX2BMP_BTN_TEMOCHI_SX	( 11 )
#define	BOX2BMP_BTN_TEMOCHI_SY	( 3 )
//「ポケモンいどう」サイズ
#define	BOX2BMP_BTN_IDOU_SX		( 11 )
#define	BOX2BMP_BTN_IDOU_SY		( 3 )
//「もどる」サイズ
#define	BOX2BMP_BTN_MODORU_SX	( 8 )
#define	BOX2BMP_BTN_MODORU_SY	( 3 )
//「ポケモンをあずける」サイズ
#define	BOX2BMP_BOXMVMENU_SX	( 11 )
#define	BOX2BMP_BOXMVMENU_SY	( 6 )
//「ボックスをきりかえる」サイズ
#define	BOX2BMP_BOXMVBTN_SX		( 17 )
#define	BOX2BMP_BOXMVBTN_SY		( 3 )
*/

// メニューボタンデータ
typedef struct {
	u16	strID;		// 文字列ＩＤ
	u16	type;		// ボタンタイプ
}BOX2BMP_BUTTON_LIST;

// ボタンタイプ
enum {
	BOX2BMP_BUTTON_TYPE_WHITE = 0,	// 白
	BOX2BMP_BUTTON_TYPE_CANCEL,			// キャンセル用
//	BOX2BMP_BUTTON_TYPE_RED,				// 赤
};

// 逃がすメッセージ表示ID
enum {
	BOX2BMP_MSGID_POKEFREE_CHECK = 0,	// ほんとうに　にがしますか？
	BOX2BMP_MSGID_POKEFREE_ENTER,		// @0を　そとに　にがしてあげた
	BOX2BMP_MSGID_POKEFREE_BY,			// ばいばい　@0！
	BOX2BMP_MSGID_POKEFREE_EGG,			// タマゴを　にがすことは　できません！
	BOX2BMP_MSGID_POKEFREE_RETURN,		// @0が　もどってきちゃった！
	BOX2BMP_MSGID_POKEFREE_FEAR,		// しんぱい　だったのかな……
	BOX2BMP_MSGID_POKEFREE_ONE,			// たたかうポケモンが　いなくなります！
};

// ボックステーマ変更メッセージ表示ID
enum {
	BOX2BMP_MSGID_THEMA_INIT = 0,		// このボックスを　どうしますか？
	BOX2BMP_MSGID_THEMA_JUMP,				// どのボックスに　ジャンプしますか？
	BOX2BMP_MSGID_THEMA_MES,				// どのテーマに　しますか？
	BOX2BMP_MSGID_THEMA_WALL,				// どの　かべがみに　しますか？
};

// ポケモンをつれていくメッセージ表示ID
enum {
	BOX2BMP_MSGID_PARTYIN_INIT = 0,			// ポケモンを　タッチ！
	BOX2BMP_MSGID_PARTYIN_MENU,					// @01を　どうする？
	BOX2BMP_MSGID_PARTYIN_ERR,					// てもちが　いっぱいです！
	BOX2BMP_MSGID_PARTYOUT_INIT,				// どこへ　あずける？
	BOX2BMP_MSGID_PARTYOUT_BOXMAX,			// このボックスは　いっぱいだ！
	BOX2BMP_MSGID_PARTYOUT_MAIL,				// メールを　はずしてください！
//	BOX2BMP_MSGID_PARTYOUT_CAPSULE,		// ボールカプセルを　はずしてください！
};

// 道具整理メッセージ表示ＩＤ
enum {
	BOX2BMP_MSGID_ITEM_A_SET = 0,		// どうぐを　もたせます
	BOX2BMP_MSGID_ITEM_A_INIT,			// @0を　どうしますか？
	BOX2BMP_MSGID_ITEM_A_RET_CHECK,		// @0を　しまいますか？
	BOX2BMP_MSGID_ITEM_A_RET,			// @0を　バッグに　いれた！
	BOX2BMP_MSGID_ITEM_A_MAIL,			// メールをしまうことは　できません！
	BOX2BMP_MSGID_ITEM_A_EGG,			// タマゴは　どうぐを　もてません！
	BOX2BMP_MSGID_ITEM_A_MAX,			// バッグが　いっぱいです！
	BOX2BMP_MSGID_ITEM_A_MAIL_MOVE,		// メールは　いどうできません！
};

// VBLANKで表示するメッセージID
enum {
	BOX2BMP_MSGID_VBLANK_AZUKERU = 0,		// ボックスに　おけます！
	BOX2BMP_MSGID_VBLANK_TSURETEIKU,		// てもちに　くわえることが　できます！
	BOX2BMP_MSGID_VBLANK_AZUKERU_ERR,		// このボックスは　いっぱいです！
	BOX2BMP_MSGID_VBLANK_TSURETEIKU_ERR,	// てもちは　いっぱいです！
	BOX2BMP_MSGID_VBLANK_ITEM_SET,			// どうぐを　もたせます
	BOX2BMP_MSGID_VBLANK_ITEM_INIT,			// @0を　どうしますか？
	BOX2BMP_MSGID_VBLANK_MAIL_ERR,			// メールを　もっています！
	BOX2BMP_MSGID_VBLANK_CAPSULE_ERR,		// ボールカプセルを　もっています！
	BOX2BMP_MSGID_VBLANK_BATTLE_POKE_ERR,	// たたかうポケモンが　いなくなります！
	BOX2BMP_MSGID_VBLANK_MAIL_MOVE,			// メールは　いどうできません！
	BOX2BMP_MSGID_VBLANK_ITEM_MOVE,			// ポケモンに　セット！
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_Init( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連削除
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_Exit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		プリントメイン
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PrintUtilTrans( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイトル表示
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_TitlePut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルト文字列表示
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_DefStrPut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		上画面ポケモンデータ表示
 *
 * @param		syswk	ボックス画面システムワーク
 * @param		info	表示データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PokeDataPut( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info );

//--------------------------------------------------------------------------------------------
/**
 * @brief		上画面ポケモンデータ非表示
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PokeDataOff( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：各メニュー
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PokeMenuBgFrmWkMake( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「てもちポケモン」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 *
 * @li	バトルボックスの場合は「バトルボックス」
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_TemochiButtonBgFrmWkMake( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧウィンドウフレーム作成：「ボックスリスト」
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_BoxListButtonBgFrmWkMake( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス移動処理のボックス名表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		tray		トレイ番号
 * @param		idx			OAMフォントのインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_BoxNameWrite( BOX2_SYS_WORK * syswk, u32 tray, u32 idx );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス移動処理のボックス名表示（位置指定）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			配置位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_BoxMoveNameWrite( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス移動処理の格納数表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		tray		トレイ番号
 * @param		idx			OAMフォントのインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_WriteTrayNum( BOX2_SYS_WORK * syswk, u32 tray, u32 idx );

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレームのボタン描画
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MarkingButtonPut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレームににBMPWINをセット
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MarkingButtonFrmPut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		メニュー作成
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		list		メニューリスト
 * @param		max			メニュー数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MenuStrPrint( BOX2_SYS_WORK * syswk, const BOX2BMP_BUTTON_LIST * list, u32 max );

//--------------------------------------------------------------------------------------------
/**
 * @brief		メニュー非表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		max			メニュー数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MenuVanish( BOX2_SYS_WORK * syswk, u32 max );

//--------------------------------------------------------------------------------------------
/**
 * @brief		システムウィンドウ非表示（ＶＢＬＡＮＫで転送）
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_SysWinVanish( BOX2_APP_WORK * appwk, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		システムウィンドウ非表示（即転送）
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_SysWinVanishTrans( BOX2_APP_WORK * appwk, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックス終了メッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		winID		BMPWIN ID
 * @param		flg			タッチ or キー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_BoxEndMsgPut( BOX2_SYS_WORK * syswk, u32 winID, BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテム取得チェックメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		item		アイテム番号
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_ItemGetCheckMsgPut( BOX2_SYS_WORK * syswk, u32 item, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		メール取得エラーメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MailGetErrorPut( BOX2_SYS_WORK * syswk, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		メール移動エラーメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MailMoveErrorPut( BOX2_SYS_WORK * syswk, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテム取得メッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		item		アイテム番号
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_ItemGetMsgPut( BOX2_SYS_WORK * syswk, u32 item, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテム取得エラーメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_ItemGetErrorPut( BOX2_SYS_WORK * syswk, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテムセットメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		item		アイテム番号
 * @param		winID		BMPWIN ID
 *
 * @return	none
 *
 *	item = 0 のときは、はっきんだまを持たせようとしたときのエラーとする
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_ItemSetMsgPut( BOX2_SYS_WORK * syswk, u32 item, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_MarkingMsgPut( BOX2_SYS_WORK * syswk, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		逃がすメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		msgID		メッセージＩＤ
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PokeFreeMsgPut( BOX2_SYS_WORK * syswk, u32 msgID, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックステーマ変更メッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		msgID		メッセージＩＤ
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_BoxThemaMsgPut( BOX2_SYS_WORK * syswk, u32 msgID, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン選択メッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			ポケモンの位置
 * @param		msgID		メッセージＩＤ
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PokeSelectMsgPut( BOX2_SYS_WORK * syswk, u32 pos, u32 msgID, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		道具整理メッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		msgID		メッセージＩＤ
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_ItemArrangeMsgPut( BOX2_SYS_WORK * syswk, u32 msgID, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		移動エラーメッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		errID		エラーＩＤ
 * @param		winID		BMPWIN ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_PutPokeMoveErrMsg( BOX2_SYS_WORK * syswk, u32 errID, u32 winID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ねかせる」用メッセージ表示
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2BMP_SleepSelectMsgPut( BOX2_SYS_WORK * syswk );
