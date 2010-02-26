// [ HEAPID_STARTMENU ] 後方確保用定義
#define	HEAPID_STARTMENU_L		( GFL_HEAP_LOWID(HEAPID_STARTMENU) )

#define	EXP_BUF_SIZE		( 1024 )		// 汎用文字列バッファサイズ

// 項目インデックス
enum {
	LIST_ITEM_CONTINUE = 0,		// 続きから
	LIST_ITEM_NEW_GAME,				// 最初から
	LIST_ITEM_HUSHIGI,				// 不思議な贈り物
	LIST_ITEM_BATTLE,					// バトル大会
	LIST_ITEM_GAME_SYNC,			// ゲームシンク設定
	LIST_ITEM_WIFI_SET,				// Wi-Fi設定
	LIST_ITEM_MIC_TEST,				// マイクテスト
	LIST_ITEM_MACHINE,				// 転送マシンを使う
	LIST_ITEM_MAX
};

// BMPWIN
enum {
	BMPWIN_TITLE = 0,
	BMPWIN_NAME,
	BMPWIN_PLACE,
	BMPWIN_TIME,
	BMPWIN_ZUKAN,
	BMPWIN_BADGE,

	BMPWIN_START,
	BMPWIN_HUSHIGI,
	BMPWIN_BATTLE,
	BMPWIN_GAMESYNC,
	BMPWIN_WIFI,
	BMPWIN_MIC,
	BMPWIN_MACHINE,

	BMPWIN_MAX
};

// BMPWIN定義
#define	BMPWIN_LIST_PAL			( FONT_PALETTE_M )
#define	BMPWIN_LIST_FRM			( GFL_BG_FRAME1_M )
//「続きから始める」
#define	BMPWIN_TITLE_PX			( 2 )
#define	BMPWIN_TITLE_PY			( 0 )
#define	BMPWIN_TITLE_SX			( 18 )
#define	BMPWIN_TITLE_SY			( 3 )
// プレイヤー名
#define	BMPWIN_NAME_PX			( 7 )
#define	BMPWIN_NAME_PY			( 4 )
#define	BMPWIN_NAME_SX			( 8 )
#define	BMPWIN_NAME_SY			( 2 )
// セーブ場所
#define	BMPWIN_PLACE_PX			( 7 )
#define	BMPWIN_PLACE_PY			( 6 )
#define	BMPWIN_PLACE_SX			( 15 )
#define	BMPWIN_PLACE_SY			( 2 )
// プレイ時間
#define	BMPWIN_TIME_PX			( 2 )
#define	BMPWIN_TIME_PY			( 11 )
#define	BMPWIN_TIME_SX			( 18 )
#define	BMPWIN_TIME_SY			( 2 )
// 図鑑
#define	BMPWIN_ZUKAN_PX			( 12 )
#define	BMPWIN_ZUKAN_PY			( 9 )
#define	BMPWIN_ZUKAN_SX			( 13 )
#define	BMPWIN_ZUKAN_SY			( 2 )
// バッジ
#define	BMPWIN_BADGE_PX			( 2 )
#define	BMPWIN_BADGE_PY			( 9 )
#define	BMPWIN_BADGE_SX			( 10 )
#define	BMPWIN_BADGE_SY			( 2 )
//「最初から始める」
#define	BMPWIN_START_PX			( 2 )
#define	BMPWIN_START_PY			( 0 )
#define	BMPWIN_START_SX			( 19 )
#define	BMPWIN_START_SY			( 3 )
//「不思議な贈り物」
#define	BMPWIN_HUSHIGI_PX		( 2 )
#define	BMPWIN_HUSHIGI_PY		( 0 )
#define	BMPWIN_HUSHIGI_SX		( 19 )
#define	BMPWIN_HUSHIGI_SY		( 3 )
//「バトル大会」
#define	BMPWIN_BATTLE_PX		( 2 )
#define	BMPWIN_BATTLE_PY		( 0 )
#define	BMPWIN_BATTLE_SX		( 19 )
#define	BMPWIN_BATTLE_SY		( 3 )
//「ゲームシンク設定」
#define	BMPWIN_GAMESYNC_PX	( 2 )
#define	BMPWIN_GAMESYNC_PY	( 0 )
#define	BMPWIN_GAMESYNC_SX	( 19 )
#define	BMPWIN_GAMESYNC_SY	( 3 )
//「Wi-Fi設定」
#define	BMPWIN_WIFI_PX			( 2 )
#define	BMPWIN_WIFI_PY			( 0 )
#define	BMPWIN_WIFI_SX			( 19 )
#define	BMPWIN_WIFI_SY			( 3 )
//「マイクテスト」
#define	BMPWIN_MIC_PX				( 2 )
#define	BMPWIN_MIC_PY				( 0 )
#define	BMPWIN_MIC_SX				( 19 )
#define	BMPWIN_MIC_SY				( 3 )
//「転送マシンを使う」
#define	BMPWIN_MACHINE_PX		( 2 )
#define	BMPWIN_MACHINE_PY		( 0 )
#define	BMPWIN_MACHINE_SX		( 19 )
#define	BMPWIN_MACHINE_SY		( 3 )

// 最初から始めるの警告ウィンドウ
#define	BMPWIN_NEWGAME_WIN_FRM	( GFL_BG_FRAME0_M )
#define	BMPWIN_NEWGAME_WIN_PX		( 2 )
#define	BMPWIN_NEWGAME_WIN_PY		( 2 )
#define	BMPWIN_NEWGAME_WIN_SX		( 30 )
#define	BMPWIN_NEWGAME_WIN_SY		( 20 )
#define	BMPWIN_NEWGAME_WIN_PAL	( FONT_PALETTE_M )

// 最初から始めるの警告文表示座標
#define	NEW_GAME_WARRNING_ATTENTION_PY	( 0 )
#define	NEW_GAME_WARRNING_MESSAGE_PY		( 3*8 )
#define	NEW_GAME_WARRNING_A_BUTTON_PY		( 16*8 )
#define	NEW_GAME_WARRNING_B_BUTTON_PY		( 18*8 )
#define	NEW_GAME_WARRNING_WIN_PX				( 32 )
#define	NEW_GAME_WARRNING_WIN_PY				( 1 )
#define	NEW_GAME_WARRNING_WIN_SX				( 32 )
#define	NEW_GAME_WARRNING_WIN_SY				( 22 )

#define	WARRNING_WIN_CHAR_NUM		( 1 )		// ウィンドウキャラ番号
#define	WARRNING_WIN_PLTT_NUM		( 1 )		// ウィンドウパレット番号

// リストフレームデータ
#define	LIST_FRAME_SX						( 26 )
#define	LIST_FRAME_CONTINUE_SY	( 14 )
#define	LIST_FRAME_NEW_GAME_SY	( 3 )
#define	LIST_FRAME_HUSHIGI_SY		( 3 )
#define	LIST_FRAME_BATTLE_SY		( 3 )
#define	LIST_FRAME_GAME_SYNC_SY	( 3 )
#define	LIST_FRAME_WIFI_SET_SY	( 3 )
#define	LIST_FRAME_MIC_TEST_SY	( 3 )
#define	LIST_FRAME_MACHINE_SY		( 3 )


#define	LIST_SCRN_SX			( 32 )
#define	LIST_SCRN_SY			( 64 )

#define	LIST_ITEM_PX			( 3 )
#define	LIST_ITEM_PY			( 2 )

#define	LIST_PALETTE			( 1 )
#define	CURSOR_PALETTE		( 2 )
#define	CURSOR_PALETTE2		( 3 )
#define	FONT_PALETTE_M		( 15 )
#define	FONT_PALETTE_S		( 15 )

// パレット
#define	FCOL_MP15WN		( PRINTSYS_LSB_Make(15,2,0) )
#define	FCOL_MP15BLN	( PRINTSYS_LSB_Make(6,5,0) )
#define	FCOL_MP15RN		( PRINTSYS_LSB_Make(4,3,0) )


// OBJ INDEX
enum {
	OBJ_ID_PLAYER = 0,
	OBJ_ID_NEW_HUSHIGI,
	OBJ_ID_NEW_BATTLE,
	OBJ_ID_NEW_GAMESYNC,
	OBJ_ID_NEW_MACHINE,
	OBJ_ID_MAX
};

// OBJリソース
enum {
	// キャラ
	CHRRES_PLAYER = 0,
	CHRRES_OBJ_U,
	CHRRES_MAX,

	// パレット
	PALRES_PLAYER = 0,
	PALRES_OBJ_U,
	PALRES_MAX,

	// セル
	CELRES_PLAYER = 0,
	CELRES_OBJ_U,
	CELRES_MAX,
};

// パレット
#define	PALNUM_PLAYER		( 0 )
#define	PALSIZ_PLAYER		( 1 )
#define	PALNUM_OBJ_U		( PALNUM_PLAYER+PALSIZ_PLAYER )
#define	PALSIZ_OBJ_U		( 1 )

// 座標
#define	PLAYER_OBJ_PX		( 48 )
#define	PLAYER_OBJ_PY		( 64 )
#define	NEW_OBJ_PX			( 208 )
