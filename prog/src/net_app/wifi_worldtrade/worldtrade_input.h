//============================================================================================
/**
 * @file	worldtrade_input.h
 * @brief	世界交換条件入力ウインドウ処理ヘッダファイル
 * @author	Akito Mori
 * @date	08.11.01
 */
//============================================================================================
#ifndef __WORLDTRADE_INPUT_H__
#define __WORLDTRADE_INPUT_H__

// ・「ポケモン検索画面」と「ポケモン預け条件入力」に同時に使う
// 　ポケモン名・性別・レベル条件・国名入力システム。
// ・指定のBG面を一つ使用し、同じ面にBMPWINでテキストも描画する
// ・カーソルを一つGFL_CLACT_WK_Createしておく必要がある。（してからInitに渡す）

// 入力モードの定義
enum{
	MODE_POKEMON_NAME,	// ポケモン名前指定
	MODE_SEX,			// 性別条件指定
	MODE_LEVEL,			// レベル条件指定
	MODE_NATION,		// 国名

	// ここまではモード指定としても使う。
	// ここから下は内部の入力モード用の定義

	MODE_HEADWORD_1,	// 頭文字子音指定
	MODE_HEADWORD_2,	// 頭文字母音指定
	MODE_NATION_HEAD1,	// 国名頭文字指定
	MODE_NATION_HEAD2,	// 国名頭文字指定
};


// 入力システムを使うシチュエーション
enum{
	SITUATION_DEPOSIT=0,		// 預ける時
	SITUATION_SEARCH,			// 検索する時
};

// スクリーンに書き込むボックス定義
enum{
	SELECT_BOX_N_LETTER=0,
	SELECT_BOX_R_LETTER,
	SELECT_BOX_N_TEXT,
	SELECT_BOX_R_TEXT,
	SELECT_BOX_N_NATION,
	SELECT_BOX_R_NATION,
	SELECT_BOX_END,
};

// BMPWINを指定するための定義
enum{
	BMPWIN_HEADWORD1_WIN0=0,
	BMPWIN_HEADWORD1_WIN1,
	BMPWIN_HEADWORD1_WIN2,
	BMPWIN_HEADWORD1_WIN3,
	BMPWIN_HEADWORD1_WIN4,
	BMPWIN_HEADWORD1_WIN5,
	BMPWIN_HEADWORD1_WIN6,
	BMPWIN_HEADWORD1_WIN7,
	BMPWIN_HEADWORD1_WIN8,
	BMPWIN_HEADWORD1_WIN9,
	BMPWIN_SELECT_END_WIN,
	BMPWIN_NONE_SELECT_WIN,
	

	BMPWIN_HEADWORD2_WIN0=0,
	BMPWIN_HEADWORD2_WIN1,
	BMPWIN_HEADWORD2_WIN2,
	BMPWIN_HEADWORD2_WIN3,
	BMPWIN_HEADWORD2_WIN4,
	BMPWIN_HEADWORD2_WIN5,
	
	BMPWIN_POKENAME_WIN0=0,
	BMPWIN_POKENAME_WIN1,
	BMPWIN_POKENAME_WIN2,
	BMPWIN_POKENAME_WIN3,
	BMPWIN_POKENAME_WIN4,
	BMPWIN_POKENAME_WIN5,
	BMPWIN_POKENAME_PAGE_WIN,
	
	BMPWIN_NATION_WIN0=0,
	BMPWIN_NATION_WIN1,
	BMPWIN_NATION_WIN2,
	BMPWIN_NATION_WIN3,
	BMPWIN_NATION_WIN4,
	BMPWIN_NATION_PAGE_WIN,
	
	
};


// ウインドウ内文字列のパレット指定（ウインドウと同じ）
#define WORLDTRADE_INPUT_PAL	( 1 )

// ウインドウ描画面がテキストを配置できるスタート位置
#define INPUT_BMPWIN_OFFSET		( 16*3 )

// 入力モード毎のBMPWINの使用数
#define MODE_HEADWORD1_WIN_NUM			( 10 )
#define MODE_HEADWORD2_WIN_NUM			(  6 )
#define MODE_POKENAME_WIN_NUM			(  4 )
#define MODE_PAGE_WIN_NUM				(  1 ) 
#define MODE_SEX_WIN_NUM				(  3 )
#define MODE_LEVEL_WIN_NUM				(  4 )
#define MODE_NATION_HEADWORD1_WIN_NUM	( 10 )
#define MODE_NATION_WIN_NUM				(  5 )

#define SEE_CHECK_MAX                   (50)    //五十音分

typedef struct{
	GFL_BMPWIN	**MenuBmp;				// メニュー用BMPWIN構造体格納配列
	GFL_BMPWIN	**BackBmp;				// 「もどる」表示用BMPWIN構造体ポインタ
	GFL_CLWK*	CursorAct;				// カーソルアクターポインタ
	GFL_CLWK*	ArrowAct[2];			// 左右矢印アクターポインタ
	GFL_CLWK*	SearchCursorAct;		// 検索画面用カーソルアクターポインタ（預ける画面では使わない）

	GFL_MSGDATA *MsgManager;			// 名前入力メッセージデータマネージャー
	GFL_MSGDATA *MonsNameManager;		// ポケモン名メッセージデータマネージャー
	GFL_MSGDATA *CountryNameManager;	// 名前入力メッセージデータマネージャー
	ZUKAN_WORK		*Zukan;					// ずかん情報データ
	u8				*SinouTable;			// シンオウポケモンかどうかがフラグで格納されているテーブル
	const CONFIG	*config;				//	コンフィグデータ


}WORLDTRADE_INPUT_HEADER;

typedef struct{
	GFL_BMPWIN	**MenuBmp;	            // メニュー用BMPWIN構造体格納配列
	GFL_BMPWIN	**BackBmp;               // 「もどる」表示用BMPWIN構造体ポインタ
	GFL_CLWK*	CursorAct;				// 選択カーソル
	GFL_CLWK*  ArrowAct[2];			// 左右ページカーソル
	GFL_CLWK*	SearchCursorAct;		// 検索画面用カーソルアクターポインタ（預ける画面では使わない）

	BGWINFRM_WORK   *BgWinFrm;				// BGスクリーン加工ルーチンワーク
	ZUKAN_WORK		*zukan;					// ずかん情報データ	

	GFL_MSGDATA *MsgManager;			// 名前入力メッセージデータマネージャー
	GFL_MSGDATA *MonsNameManager;		// ポケモン名メッセージデータマネージャー
	GFL_MSGDATA *CountryNameManager;	// 名前入力メッセージデータマネージャー

	u8*				SinouTable;				// シンオウポケモンかどうかがフラグで格納されているテーブル
	BMPLIST_DATA	*NameList;				// ポケモン名・世界各国名を格納するテーブルポインタ
	NUMFONT			*NumFontSys;			// 数字フォント描画システム構造体

	s16		Head1;			// 頭文字（子音）
	s16		Head2;			// 頭文字（母音）
	s16		Poke;			// ポケモンNO
	s16		Nation;			// 国NO
	s8		Sex;			// 性別No
	s8		Level;			// 欲しいれべる
	int		listpos;		// カーソル位置
	int		seq;			// 入力用シーケンス
	int		next;			// 次回シーケンス
	int		BgFrame;		// 描画BGフレーム

	s16		type;			// 入力タイプ
	s16		page;			// 現在の表示ページ（４項目以上の場合）
	int		listMax;		// 表示リストの項目数
	int		Situation;		// 利用するシチュエーション（あずける？検索？）

    u8     see_check[SEE_CHECK_MAX];   // 五十音のリストの中にポケモンが1匹以上含まれているか
    u8     listpos_backup_x;   // X位置記憶の為のバックアップ
    u8      padding;

		WT_PRINT	print;
}WORLDTRADE_INPUT_WORK;

extern void WorldTrade_Input_SysPrint( BGWINFRM_WORK *wfwk, GFL_BMPWIN *win, STRBUF *strbuf, int x, PRINTSYS_LSB color, WT_PRINT *print );

WORLDTRADE_INPUT_WORK * WorldTrade_Input_Init( WORLDTRADE_INPUT_HEADER *wih, int frame, int situation );
extern u32 WorldTrade_Input_Main( WORLDTRADE_INPUT_WORK *wk );
extern void WorldTrade_Input_Exit( WORLDTRADE_INPUT_WORK *wk );
extern void WorldTrade_Input_Start( WORLDTRADE_INPUT_WORK *wk, int type );


#endif
