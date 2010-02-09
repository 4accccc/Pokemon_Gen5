//=============================================================================
/**
 * @file	worldtrade_local.h
 * @brief	世界交換画面ヘッダファイル
 *          
 * @author	Akito Mori
 * @date    	2006.04.16
 */
//=============================================================================

#pragma once

#include <dpw_tr.h>

///< DPのふりをする(??)
#define GS_DP_GISOU

#include "system/bmp_menulist.h"
#include "system/touch_subwindow.h"
//#include "system/numfont.h"
//#include "system/selbox.h"
//#include "communication/wm_icon.h"
//#include "savedata/zukanwork.h"
#include "savedata/wifilist.h"
#include "savedata/wifihistory.h"
#include "app/p_status.h"
//#include "demo/demo_trade.h"
//#include "battle/shinka.h"
#include "net_app/connect_anm.h"
#include "system/bmp_winframe.h"
#include "worldtrade_input.h"
#include "sound/pm_sndsys.h"
#include "worldtrade_adapter.h"
#include "system/mcss.h"
#include "system/mcss_tool.h"

// Proc_Mainシーケンス定義
enum {
	SEQ_INIT,
	SEQ_FADEIN,
	SEQ_MAIN,
	SEQ_FADEOUT,
	SEQ_OUT,
	SEQ_EXIT,
};

// サブプロセス制御番号
enum{
	WORLDTRADE_ENTER=0,		///< 接続画面
	WORLDTRADE_TITLE,		///< タイトル・メニュー
	WORLDTRADE_MYPOKE,		///< ようすをみる
	WORLDTRADE_PARTNER,		///< あいてのポケモンのようすをみる
	WORLDTRADE_SEARCH,		///< ポケモンを検索する
	WORLDTRADE_MYBOX,		///< 自分のボックス
	WORLDTRADE_DEPOSIT,		///< ほしいポケモンの条件
	WORLDTRADE_UPLOAD,		///< ポケモンデータをアップロード
	WORLDTRADE_STATUS,		///< ポケモンステータス呼び出し
	WORLDTRADE_DEMO,		///< ポケモンステータス呼び出し
};

// サブプロセスに渡すモード（同じサブプロセスでも使い方が違う時があるため）
enum{
	// worldtrade_title.c
	MODE_OPENING=0,				///< 初めてきたとき
	MODE_CONTINUE,				///< 継続するためにきた
	MODE_NOT_END,				///< 一回やめるを選んだが、やめなかった

	// worldtrade_mypoke.c
	MODE_VIEW,					///< 預けたポケモンのようすをみる
	MODE_DEPOSIT,				///< 検索したポケモンと交換するときにみる

	// worldtrade_box.c
	MODE_DEPOSIT_SELECT,		///< 自分からポケモンを預けるとき
	MODE_EXCHANGE_SELECT,		///< 検索したポケモンと交換するポケモンを選ぶ
	
	// worldtrade_upload.c
	MODE_UPLOAD,				///< ポケモンを預ける画面に行く
	MODE_DOWNLOAD,				///< ポケモンを引き取る
	MODE_EXCHANGE,				///< ポケモンを交換する
	MODE_DOWNLOAD_EX,
	MODE_SERVER_CHECK,			///< ポケモンが交換されているか確認する→メニューへ
	MODE_POKEMON_EVO_SAVE,		///< 交換ポケモンが進化した後のセーブ処理

	// worldtrade_search.c
	MODE_NORMAL,				///< 検索画面に普通に入る
	MODE_PARTNER_VIEW,			///< 相手を見る画面に行く
	MODE_PARTNER_RETURN,		///< 相手を見る画面から戻ってくる

	// worldtrade_partner.c
	MODE_FROM_SEARCH,			///< 検索画面からきた
	MODE_FROM_BOX,				///< BOX画面からきた
	MODE_TO_BOX,				///< BOX画面に行く
	
  // worldtrade_enter.c
  MODE_WIFILOGIN,     ///< WIFIログイン
  MODE_CONNECT,       ///< GTSサーバー接続をする
  MODE_DISCONNECT,   ///< GTSサーバー切断をする
  MODE_WIFILOGOUT,    ///< WIFIログアウト
};

///< CLACTで定義しているセルが大きすぎてサブ画面に影響がでてしまうので離してみる
#define NAMEIN_SUB_ACTOR_DISTANCE 	(512)
//		256*FX32_ONE

///< CellActorに処理させるリソースマネージャの種類の数（＝マルチセル・マルチセルアニメは使用しない）
enum
{	
	CLACT_U_CHAR_RES,
	CLACT_U_PLTT_RES,
	CLACT_U_CELL_RES,
	CLACT_RESOURCE_NUM,
};
#define NAMEIN_OAM_NUM			( 14 )

#define WORLDTRADE_MESFRAME_PAL	 ( /*10*/14 )	//＜＜ボックスリストとかぶるから変更
#define WORLDTRADE_MENUFRAME_PAL ( 11 )
#define WORLDTRADE_MESFRAME_CHR	 (  1 )
#define WORLDTRADE_MENUFRAME_CHR ( WORLDTRADE_MESFRAME_CHR + TALK_WIN_CGX_SIZ )
#define WORLDTRADE_TALKFONT_PAL	 ( 13 )
#define WORLDTRADE_SUB_TALKFONT_PAL	( 1 )

///< 会話ウインドウ表示位置定義
#define TALK_WIN_X		(  2 )
#define TALK_WIN_Y		( 19 )
#define	TALK_WIN_SX		( 27 )
#define	TALK_WIN_SY		(  4 )


///< １行メッセージウインドウ表示位置定義
#define LINE_TEXT_X		(  2 )
#define LINE_TEXT_Y		( 21 )
#define	LINE_TEXT_SX	( 27 )
#define	LINE_TEXT_SY	(  2 )


///< 画面上部タイトルウインドウ表示位置定義
#define TITLE_TEXT_X	(  2 )
#define TITLE_TEXT_Y	(  1 )
#define TITLE_TEXT_SX	( 28 )
#define TITLE_TEXT_SY	(  2 )

#define HEAD_LIST_X		( 15 )
#define HEAD_LIST_Y		(  5 )
#define HEAD_LIST_SX	(  4 )
#define HEAD_LIST_SY	( 13 )

#define NAME_LIST_X		( 21 )
#define NAME_LIST_Y		(  5 )
#define NAME_LIST_SX	( 10 )
#define NAME_LIST_SY	( 13 )


// 選択リストの位置・幅定義(_deposit.c _search.c)
// 頭文字
#define SELECT_MENU1_X	(  15 )
#define SELECT_MENU1_Y 	(   5 )
#define SELECT_MENU1_SX	(   4 )
#define SELECT_MENU1_SY	(  13 )

// ポケモン名
#define SELECT_MENU2_X	(  21 )
#define SELECT_MENU2_Y 	(   5 )
#define SELECT_MENU2_SX	(  10 )
#define SELECT_MENU2_SY	(  13 )

// 性別
#define SELECT_MENU3_X	(  21 )
#define SELECT_MENU3_Y 	(  10 )
#define SELECT_MENU3_SX	(  10 )
#define SELECT_MENU3_SY	(   8 )

// 欲しいレベル
#define SELECT_MENU4_X	(  15 )
#define SELECT_MENU4_Y 	(   5 )
#define SELECT_MENU4_SX	(  16 )
#define SELECT_MENU4_SY	(  13 )

// 国
#define SELECT_MENU5_X	(  2 )
#define SELECT_MENU5_Y 	(   5 )
#define SELECT_MENU5_SX	(  28 )
#define SELECT_MENU5_SY	(  13 )

// 「GTS」とか「ポケモンをえらんでください」とか
#define EXPLAIN_WIN_X	(  13 )
#define EXPLAIN_WIN_Y 	(  19 )
#define EXPLAIN_WIN_SX	(  16 )
#define EXPLAIN_WIN_SY	(   4 )

#define GTS_EXPLAIN_OFFSET	   ( 1 )


///< はい・いいえウインドウのY座標
#define	WORLDTRADE_YESNO_PY2	( 10 )		// 会話ウインドウが２行の時
#define	WORLDTRADE_YESNO_PY1	( 12 )		// 会話ウインドウが１行の時

///< タイトル文字列バッファ長
#define TITLE_MESSAGE_BUF_NUM	( 20 * 2 )

///< 会話ウインドウ文字列バッファ長
#define TALK_MESSAGE_BUF_NUM	( 90*2 )

#define DWC_ERROR_BUF_NUM		(16*8*2)

///< ボックストレい名
#define BOX_TRAY_NAME_BUF_NUM	( 9*2 )

///< 上下画面指定定義
#define RES_NUM	( 3 )
#define MAIN_LCD	( GFL_BG_MAIN_DISP )	// 要は０と
#define SUB_LCD		( GFL_BG_SUB_DISP )		// １なんですが。
#define CHARA_RES	( 2 )

///< サブ画面のテキストパレット転送位置
#define WORLDTRADE_MESFRAME_SUB_PAL	 	(  2 )

///< サブ画面の文字フォント転送位置
#define WORLDTRADE_TALKFONT_SUB_PAL		( 1 )


// BMPWIN指定
enum{
	BMP_NAME1_S_BG0,
	BMP_NAME2_S_BG0,
	BMP_NAME3_S_BG0,
	BMP_NAME4_S_BG0,
	BMP_NAME5_S_BG0,
	BMP_RECORD_MAX,
};

///< １画面のボックスの中のポケモンの数
#define BOX_POKE_NUM		( 30 )

///< ポケモンアイコンのパレットを転送するオフセット
#define POKEICON_PAL_OFFSET				(  3 )

///< 選択できないポケモンアイコンのパレット
#define POKEICON_NOTSELECT_PAL_OFFSET	(  6 )

///< ポケモンアイコンが始まるVRAM上でのオフセット
#define POKEICON_VRAM_OFFSET	(12 + 16)


///< ５０音順のポケモンの頭文字の数（アカサタナハマヤラワ）
#define HEADWORD_NUM		( 10 )

///< アイテム・メールアイコンのセルアニメ番号
#define CELL_ITEMICON_NO	( 40 )
#define CELL_MAILICON_NO	( 41 )
#define CELL_CBALLICON_NO	( 42 )

///< ボックス横の矢印
#define CELL_BOXARROW_NO	( 38 )


///< ポケモン検索
#define SEARCH_POKE_MAX		(  7 )

#define SUB_OBJ_NUM			(  8 )

///< 1秒待つ
#define WAIT_ONE_SECONDE_NUM	( 30 )

///< ワイプのスピード
#define WORLDTRADE_WIPE_SPPED	( 6 )

///< サーバー確認ができるようになるまでの秒数（６０秒）
#define SEVER_RETRY_WAIT		( 60*30 )

///< タッチ対応選択ボックスの表示設定
#define WORLDTRADE_SELBOX_X		( 20  )
#define WORLDTRADE_SELBOX_W		( 5*2 )

///< 画面が上下入れ替わる際のワイプにかかるシンク数
#define EXCHANGE_SCREEN_SYNC	( 16 )

///< パッシブ状態の輝度
#define PASSIVE_SCREEN_RATE		(  9 )


///< 「DSの下画面を見てね」アイコンの座標
#define DS_ICON_X	(  55 )
#define DS_ICON_Y	( 176-8 )


///< 検索条件のレベル指定の使用するメッセージテーブルを指定する
enum{
	LEVEL_PRINT_TBL_DEPOSIT,	///<預ける時のレベル条件指定テーブル
	LEVEL_PRINT_TBL_SEARCH,		///<検索する時のレベル条件指定テーブル
};

///< 交換成立タイプ
enum{
	TRADE_TYPE_DEPOSIT,		///<預けて交換成立
	TRADE_TYPE_SEARCH,		///<検索して交換成立
};



enum{
	EXPLAIN_GTS=0,
	EXPLAIN_KOUKAN,
	EXPLAIN_YOUSU,
	EXPLAIN_AZUKERU,
	EXPLAIN_SAGASU,
};

//============================================================================================
//	構造体定義
//============================================================================================

// ポケモンを預ける際の情報
typedef struct{

	// ポケモンの種族名を格納するテーブル（2重にALLOCする）
	STRBUF **pokename;

	// ポケモン預け・検索共用
	u16		headwordPos;		///< 選択頭カーソル位置
	u16		headwordListPos;	///< 選択リスト位置
	u16		namePos;			///< 名前選択カーソル位置
	u16		nameListPos;		///< 名前選択リスト位置
	int		sexPos;				///< 性別カーソル位置
	int		levelPos;			///< レベルカーソル位置

	u8		*sinouTable;		///< シンオウ図鑑データ（数値は開発NO）
	u16		*nameSortTable;		///< ポケモン名ソートテーブル読み込みポインタ
	int		nameSortNum;		///< 頭文字
	int		sex_selection;		///< 選択したポケモンの性別分布率


	// ポケモン検索画面用
	int		cursorSide;			///< 左右どちらにカーソルがあるか
	int		leftCursorPos;		///< 左列のカーソル位置
	int		rightCursorPos;		///< 右列のカーソル位置

}DEPOSIT_WORK;

typedef struct{
	Dpw_Tr_PokemonDataSimple	info[BOX_POKE_NUM];
}BOX_RESEARCH;

// 一度交換したポケモンが進化した時のために、2回目の書き込み位置を保存しておく構造体
typedef struct{
	int boxno;
	int pos;
}EVOLUTION_POKEMON_INFO;

#define NAME_HEAD_MAX	( 10 )	///< [アカサタナハマヤラワ]の数は１０個

typedef struct{
	u16 head_list,head_pos;

	u16 name_list[NAME_HEAD_MAX];
	u16 name_pos[NAME_HEAD_MAX];
}SELECT_LIST_POS;

typedef struct _WORLDTRADE_WORK{
	// 起動時に貰ってるもの
	WORLDTRADE_PARAM *param;							///< 呼び出し時パラメータ
	WIFI_LIST		*wifilist;							///< wifi接続データ・友達データ
	
	
	// 世界交換システムに関わるもの
	int				seq;								///< 現在の世界交換画面の状態
	int				nextseq;							///< 現在のシーケンスが終了した際の遷移先

	int				sub_process;						///< 世界交換サブプログラム制御ナンバー
	int				sub_nextprocess;					///< 世界交換サブNEXT制御ナンバー
	int				sub_returnprocess;					///< nextプロセスを呼んで終了した時の戻りプロセス
	int				old_sub_process;					///< 一つ前のサブプロセスを保存しておく
	int				sub_process_mode;					///< その画面にきたモードは何か？
	int				error_mes_no;

	int				subprocess_seq;						///< サブプログラムシーケンスNO
	int				subprocess_nextseq;					///< サブプログラムNEXTシーケンスNO

	u16				OpeningFlag;						///< 世界交換タイトルを見たか？
	u16				DepositFlag;						///< ポケモンを既にサーバーに預けているか？（1:預けている）
	u16				ExchangeFlag;						///< 預けたポケモンが交換されていた
	u16				serverWaitTime;						///< サーバーにアクセスできないようにさせるタイマー

	int				ConnectErrorNo;						///< DWC・またはサーバーからのエラー
	int				ErrorRet;
	int				ErrorCode;
    int ErrorType;

	void 			*heapPtr;							///< NitroDWCに渡すヒープワークの解放用ポインタ
	NNSFndHeapHandle heapHandle;						///< heapPtrを32バイトアライメントに合わせたポインタ
	DWCInetControl   stConnCtrl;						///< DWC接続ワーク

	GFL_PROCSYS			*procsys;					    ///< サブプロセス用
	PSTATUS_DATA	 statusParam;						///< スタータス呼び出し用パラメータ
	DEMO_TRADE_PARAM tradeDemoParam;					///< 交換デモパラメータ
	SHINKA_WORK		 *shinkaWork;						///< 通信進化デモ用ワーク
	int				 subprocflag;						///< ステータスや交換デモを呼び出すためのフラグ
	u16				listpos;				
	u16				dummy;

	// 各画面をぬけても保存されてるもの
	u16				TitleCursorPos;						///< タイトルメニューのカーソル位置
	u16				PartnerPageInfo;					///< 左・右ページのどちらをみているか？
	u16				BoxTrayNo;							///< 何番ボックスを見ているか？
	u16				BoxCursorPos;						///< ボックス上でのカーソルの位置
	POKEMON_PASO_PARAM	*deposit_ppp;					///< 一旦預ける指定になったポケモンのポインタ
	int				SearchResult;						///< 検索の結果返ってきた数
	int				TouchTrainerPos;					///< 検索結果の誰をタッチしたか
	MYSTATUS		*partnerStatus;						///< 交換デモようにでっちあげるMYSTATUS;
	EVOLUTION_POKEMON_INFO EvoPokeInfo;


	Dpw_Tr_Data		UploadPokemonData;					///< 送信データ
	Dpw_Tr_Data		DownloadPokemonData[SEARCH_POKE_MAX];///< 検索結果データ
	Dpw_Tr_Data		ExchangePokemonData;				///< 交換結果データ
	Dpw_Tr_PokemonDataSimple Post;						///< 預けるポケモン情報
	Dpw_Tr_PokemonSearchData Want;						///< 欲しいポケモン情報
	Dpw_Tr_PokemonSearchData Search;					///< 検索ポケモン条件
	Dpw_Tr_PokemonSearchData SearchBackup;				///< 同じ検索はできないようにするために保存
	int				SearchBackup_CountryCode;			///< 国情報の保存



	// 描画まわりのワーク（主にBMP用の文字列周り）
	WORDSET			*WordSet;								///< メッセージ展開用ワークマネージャー
	GFL_MSGDATA *MsgManager;							///< 名前入力メッセージデータマネージャー
	GFL_MSGDATA *MonsNameManager;						///< ポケモン名メッセージデータマネージャー
	GFL_MSGDATA *LobbyMsgManager;						///< 名前入力メッセージデータマネージャー
	GFL_MSGDATA *SystemMsgManager;						///< Wifiシステムメッセージデータ
	GFL_MSGDATA *CountryNameManager;					///< 国名メッセージデータマネージャー
	STRBUF			*BoxTrayNameString;						///< ボックストレイ名
	STRBUF			*EndString;								///< 文字列「やめる」
	STRBUF			*TalkString;							///< 会話メッセージ用
	STRBUF			*TitleString;							///< タイトルメッセージ用
	STRBUF			*InfoString[10];						///< タイトルメッセージ用
	STRBUF			*ErrorString;							///< エラーメッセージ表示用
	int				MsgIndex;								///< 終了検出用ワーク


	// 描画周りのワーク（主にOAM)
	GFL_CLUNIT*					clactSet;								///< セルアクターセット

	u32 resObjTbl[RES_NUM][CLACT_RESOURCE_NUM]; ///< リソースオブジェテーブル


	GFL_CLWK *			CursorActWork;							///< セルアクターワークポインタ配列
	GFL_CLWK *			SubCursorActWork;						///< セルアクターワークポインタ配列
	GFL_CLWK *			FingerActWork;							///< セルアクターワークポインタ配列
	GFL_CLWK *			PokeIconActWork[BOX_POKE_NUM];			///< ポケモンアイコンアクター
	GFL_CLWK *			ItemIconActWork[BOX_POKE_NUM];			///< アイテムアイコンアクター
	GFL_CLWK *			CBallActWork[6];						///< カスタムボールアイコンアクター
	GFL_CLWK *			PokemonActWork;							///< セルアクターワークポインタ配列
	GFL_CLWK *			SubActWork[SUB_OBJ_NUM];				///< サブ画面用ＯＢＪポインタ
	GFL_CLWK *			BoxArrowActWork[2];						///< ボックス名の横にある矢印
	GFL_CLWK *			PartnerCursorActWork;					///< 下画面で交換相手を選ぶカーソル
	GFL_CLWK *			PromptDsActWork;						///< 下画面を見なさいアイコン
	int						DrawOffset;								///< 画面全体をズラすための変数

	// BMPWIN描画周り
	GFL_BMPWIN*			MsgWin;									///< 会話ウインドウ
	GFL_BMPWIN*			NumberWin;								///< やめる
	GFL_BMPWIN*			TitleWin;								///< 「レコードコーナー　ぼしゅうちゅう！」など
	GFL_BMPWIN*			SubWin;									///< 「レコードコーナー　ぼしゅうちゅう！」など
	GFL_BMPWIN*			MenuWin[13];							///< メニュー用BMPWIN
	GFL_BMPWIN*			InfoWin[16];							///< プラチナで2個追加して金銀でも2個追加
	GFL_BMPWIN*			TalkWin;								///< 会話ウインドウ
	GFL_BMPWIN*			BackWin;								///< 「もどる」
	GFL_BMPWIN*			CountryWin[2];							///< 国・地域表示
	GFL_BMPWIN*			ExplainWin;								///< サブ画面で現在の状況を説明する
	WORLDTRADE_INPUT_WORK	*WInputWork;							///< 名前・性別・レベル・国条件入力システムワーク


	BMP_MENULIST_DATA		*BmpMenuList;
	BMPMENU_WORK			*YesNoMenuWork;
	BMPMENU_WORK 			*BmpMenuWork;
	BMPMENULIST_WORK 			*BmpListWork;
	void*					timeWaitWork;							// 会話ウインドウ横アイコンワーク

	int						wait;								///< 汎用待ちワーク


	// 画面毎に使うことがあるワーク
	DEPOSIT_WORK			*dw;								///< 預ける用ワーク
	TOUCH_SW_SYS			*tss;								///< はい・いいえウインドウ
	SELBOX_SYS				*SelBoxSys;							///< タッチキー選択ボックスシステム
	SELBOX_WORK				*SelBoxWork;						///< タッチキー選択ボックスワーク

	// worldtrade_upload.c用ワーク
	u16						saveNextSeq1st;						///< セーブの前半終了時に飛ぶシーケンス
	u16						saveNextSeq2nd;						///< セーブの後半終了時に飛ぶシーケンス

	// worldtrade_sublcd.c用ワーク
	GFL_TCB*			demotask;							///< 主人公デモ用タスクポインタ
	u16						demo_end;							///< デモ終了フラグ
	u16						SubLcdTouchOK;						///< 人物OBJがでてきてから触れるようになるフラグ
	void*					FieldObjCharaBuf;					///< 人物OBJキャラファイルデータ
	NNSG2dCharacterData*	FieldObjCharaData;					///< 人物OBJキャラデータの実ポインタ				
	void*					FieldObjPalBuf;						///< 人物OBJパレットァイルデータ
	NNSG2dPaletteData*		FieldObjPalData;					///< 人物OBJパレットファイルデータ

	// worldtrade_demo.c用ワーク
	POKEMON_PARAM *demoPokePara;								///< 交換デモ用のポケモンポインタ


	// worldtrade_box.c用ワーク
	BOX_RESEARCH			*boxWork;							///< ボックス内データの全ポケモンの基本情報
	u16						boxPokeNum;							///< ボックスにいるポケモンの総数
	u16						boxSearchFlag;						///< ボックスの総数を取得するためのフラグ
	u32	sub_out_flg;		// 下画面フェードコントロール
	
	void *boxicon;												///< ポケモンアイコン書き換えに使うキャラポインタ
	void (*vfunc)(void *);										///< Vブランクタスク(ポケモンアイコン書き換え）
	void (*vfunc2)(void *);										///< Vブランクタスク（BGスクロール）

	// worldtrade_search.cで使用
	s16						SubActY[10][2];						///< 各人物OBJの初期Y位置

	///<worldtrade_deposit.c worldtrade_search.cで兼用
	SELECT_LIST_POS			selectListPos;
	
	// プラチナから追加
	Dpw_Common_Profile			dc_profile;						// 自分の情報登録用構造体
	Dpw_Common_ProfileResult	dc_profile_result;				// 自分の情報登録レスポンス用構造体

	int							country_code;					///< 検索条件に国番号
	
	CONNECT_BG_PALANM cbp;		// Wifi接続BG画面のパレットアニメ制御システム
	
	s16 local_seq;												///< サーバー応答エラー用のシーケンスワーク
	s16 local_wait;												///< サーバー応答エラー用のウェイト
	s32 timeout_count;											///< サーバー応答エラー用の計測用

	GFL_TCB *	vblank_task;								///< VBlank関数はタスクになった
	GFL_TCBSYS* tcbsys;										///< PMDS_taskがなくなったので自前で用意
	void * task_wk;												///< タスク用ワーク
	void * task_wk_area;									///< タスクワーク作成用エリア
	WT_PRINT	print;
	GFL_G3D_CAMERA    * camera;						///< MCSSでポケモン表示するための正射影カメラ
	MCSS_SYS_WORK *mcssSys;								///< MCSS本体
	MCSS_WORK     *pokeMcss;							///< 一匹分
  void * sub_proc_wk;                   ///< WIFILOGINやデモなどサブプロセスのワーク

  BOOL is_wifilogin;                    ///< WIFIログイン

#ifdef PM_DEBUG
	int 					frame;									//
	int						framenum[9][2];							//
#endif
} WORLDTRADE_WORK;


//============================================================================================
// SE用定義
//============================================================================================
#define WORLDTRADE_MOVE_SE		(SEQ_SE_DP_SELECT)
#define WORLDTRADE_DECIDE_SE	(SEQ_SE_DP_SELECT)
//#define WORLDTRADE_PAGE_SE		(SEQ_SE_DP_SELECT78)
#define WORLDTRADE_PAGE_SE		(SEQ_SE_DP_SELECT)
//TODO 

//============================================================================================
//	デバッグ用
//============================================================================================
//#define	GTS_FADE_OSP	// フェード場所表示用定義


//============================================================================================
// extern宣言
//============================================================================================
// worldtrade.c
extern TOUCH_SW_SYS *WorldTrade_TouchWinYesNoMake( int y, int yesno_bmp_cgx, int pltt, u8 inPassive );
extern TOUCH_SW_SYS *WorldTrade_TouchWinYesNoMakeEx( int y, int yesno_bmp_cgx, int pltt, int frame, u8 inPassive );

extern u32 WorldTrade_TouchSwMain(WORLDTRADE_WORK *wk);

extern void WorldTrade_SetNextSeq( WORLDTRADE_WORK *wk, int to_seq, int next_seq );
extern void WorldTrade_SetNextProcess( WORLDTRADE_WORK *wk, int next_process );
extern  int WorldTrade_WifiLinkLevel( void );
extern void WorldTrade_BmpWinPrint( GFL_BMPWIN *win, GFL_MSGDATA *msgman, int font, int msgno, u16 dat, WT_PRINT *print );
extern void WorldTrade_SysPrint( GFL_BMPWIN *win, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color, WT_PRINT *print );
extern void WorldTrade_TalkPrint( GFL_BMPWIN *win, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color, WT_PRINT *print );
extern void WorldTrade_TouchPrint( GFL_BMPWIN *win, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color , WT_PRINT *print );

extern void WorldTrade_WifiIconAdd( WORLDTRADE_WORK *wk );
extern void Enter_MessagePrint( WORLDTRADE_WORK *wk, GFL_MSGDATA *msgman, int msgno, int wait, u16 dat );
extern void WorldTrade_SubProcessChange( WORLDTRADE_WORK *wk, int subprccess, int mode );
extern WINTYPE WorldTrade_GetMesWinType( WORLDTRADE_WORK *wk );
extern int WorldTrade_GetTalkSpeed( WORLDTRADE_WORK *wk );
extern void FreeFieldObjData( WORLDTRADE_WORK *wk );
extern void WorldTrade_BoxPokeNumGetStart( WORLDTRADE_WORK *wk );
extern void WorldTrade_SubProcessUpdate( WORLDTRADE_WORK *wk );
extern void WorldTrade_TimeIconAdd( WORLDTRADE_WORK *wk );
extern void WorldTrade_TimeIconDel( WORLDTRADE_WORK *wk );
extern void WorldTrade_CLACT_PosChange( GFL_CLWK * ptr, int x, int y );
extern void WorldTrade_CLACT_PosChangeSub( GFL_CLWK * act, int x, int y );
extern SELBOX_WORK* WorldTrade_SelBoxInit( WORLDTRADE_WORK *wk, u8 frm, int count, int y );
extern void WorldTrade_SelBoxEnd( WORLDTRADE_WORK *wk );

extern void WorldTrade_SetPassive(u8 inTarget);
extern void WorldTrade_SetPassiveMyPoke(u8 inIsMain);
extern void WorldTrade_ClearPassive(void);

extern void WorldTrade_InitSystem( WORLDTRADE_WORK *wk );
extern void WorldTrade_ExitSystem( WORLDTRADE_WORK *wk );



// worldtrade_sublcd.c
extern void WorldTrade_HeroDemo( WORLDTRADE_WORK *wk, int sex );
extern int  WorldTrade_SubLcdObjHitCheck( int max );
extern void WorldTrade_SubLcdMatchObjAppear( WORLDTRADE_WORK *wk, int num, int flag );
extern void WorldTrade_SubLcdMatchObjHide( WORLDTRADE_WORK *wk );
extern void WorldTrade_SubLcdActorAdd( WORLDTRADE_WORK *wk, int sex );
extern void WorldTrade_ReturnHeroDemo( WORLDTRADE_WORK *wk, int sex );
extern void WorldTrade_SetPartnerCursorPos( WORLDTRADE_WORK *wk, int index, int offset_y );
extern void WorldTrade_SetPartnerExchangePos( WORLDTRADE_WORK *wk );
extern void WorldTrade_SetPartnerExchangePosIsReturns( WORLDTRADE_WORK *wk );


// worldtrade_enter.c
extern int  WorldTrade_Enter_End( WORLDTRADE_WORK *wk, int seq);
extern int  WorldTrade_Enter_Main(WORLDTRADE_WORK *wk, int seq);
extern int  WorldTrade_Enter_Init(WORLDTRADE_WORK *wk, int seq);

extern void WorldTrade_ExplainPrint( GFL_BMPWIN *win,  GFL_MSGDATA *msgman, int no, WT_PRINT *print );

// worldtrade_title.c
extern int  WorldTrade_Title_End( WORLDTRADE_WORK *wk, int seq);
extern int  WorldTrade_Title_Main(WORLDTRADE_WORK *wk, int seq);
extern int  WorldTrade_Title_Init(WORLDTRADE_WORK *wk, int seq);
extern void WorldTrade_SubLcdBgGraphicSet( WORLDTRADE_WORK *wk );
extern void WorldTrade_SubLcdBgInit( int sub_bg1_y_offset, BOOL sub_bg2_no_clear );
extern void WorldTrade_SubLcdBgExit( void );

extern void WorldTrade_SubLcdWinGraphicSet( WORLDTRADE_WORK *wk );
extern void WorldTrade_SubLcdExpainPut( WORLDTRADE_WORK *wk, int explain );



// worldtrade_mypoke.c
extern int  WorldTrade_MyPoke_Init(WORLDTRADE_WORK *wk, int seq);
extern int  WorldTrade_MyPoke_Main(WORLDTRADE_WORK *wk, int seq);
extern int  WorldTrade_MyPoke_End(WORLDTRADE_WORK *wk, int seq);
extern void WorldTrade_PokeInfoPrint( 	GFL_MSGDATA *MsgManager,
							GFL_MSGDATA *MonsNameManager, 	
							WORDSET *WordSet, 
							GFL_BMPWIN *win[], 	
							POKEMON_PASO_PARAM *ppp,
							Dpw_Tr_PokemonDataSimple *post,
							WT_PRINT *print );
extern void WorldTrade_PokeInfoPrint2( GFL_MSGDATA *MsgManager, GFL_BMPWIN *win[], STRCODE *name, POKEMON_PARAM *pp, GFL_BMPWIN* oya_win[], WT_PRINT *print );
extern void WorldTrade_TransPokeGraphic( POKEMON_PARAM *pp );
//以下、ポケモン表示をMCSSに変更するために作成nagihashi
extern void WorldTrade_MyPoke_G3D_Init( WORLDTRADE_WORK *wk );
extern void WorldTrade_MyPoke_G3D_Exit( WORLDTRADE_WORK *wk );
extern void WorldTrade_MyPoke_G3D_Draw( WORLDTRADE_WORK *wk );
extern MCSS_WORK * WorldTrade_MyPoke_MCSS_Create( WORLDTRADE_WORK *wk, POKEMON_PASO_PARAM *ppp, const VecFx32 *pos );
extern void WorldTrade_MyPoke_MCSS_Delete( WORLDTRADE_WORK *wk, MCSS_WORK *poke );
//ここまで

// worldtrade_partner.c
extern int WorldTrade_Partner_Init(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Partner_Main(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Partner_End(WORLDTRADE_WORK *wk, int seq);

// worldtrade_search.c
extern  int WorldTrade_Search_Init(WORLDTRADE_WORK *wk, int seq);
extern  int WorldTrade_Search_Main(WORLDTRADE_WORK *wk, int seq);
extern  int WorldTrade_Search_End(WORLDTRADE_WORK *wk, int seq);

// worldtrade_box.c
extern int WorldTrade_Box_Init(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Box_Main(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Box_End(WORLDTRADE_WORK *wk, int seq);
extern POKEMON_PASO_PARAM *WorldTrade_GetPokePtr( POKEPARTY *party, BOX_MANAGER *box,  int  tray, int pos );
extern int WorldTrade_GetPPorPPP( int tray );
extern BOOL WorldTrade_PokemonMailCheck( POKEMON_PARAM *pp );

// workdtrade_deposit.c
extern int WorldTrade_Deposit_Init(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Deposit_Main(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Deposit_End(WORLDTRADE_WORK *wk, int seq);
extern void WodrldTrade_PokeWantPrint( GFL_MSGDATA *MsgManager, GFL_MSGDATA *MonsNameManager,
	WORDSET *WordSet, GFL_BMPWIN *win[], int monsno, int sex, int level, WT_PRINT *print );
extern void WodrldTrade_MyPokeWantPrint( GFL_MSGDATA *MsgManager, GFL_MSGDATA *MonsNameManager,
	WORDSET *WordSet, GFL_BMPWIN *win[], int monsno, int sex, int level, WT_PRINT *print );

extern BMPMENULIST_WORK *WorldTrade_WordheadBmpListMake( WORLDTRADE_WORK *wk, BMP_MENULIST_DATA **menulist, 
													GFL_BMPWIN *win, GFL_MSGDATA *MsgManager );
extern BMPMENULIST_WORK *WorldTrade_PokeNameListMake( WORLDTRADE_WORK *wk, BMP_MENULIST_DATA **menulist, GFL_BMPWIN *win, GFL_MSGDATA *MsgManager, GFL_MSGDATA *MonsNameManager, DEPOSIT_WORK* dw, ZUKAN_WORK *zukan);
extern BMPMENULIST_WORK *WorldTrade_SexSelectListMake( BMP_MENULIST_DATA **menulist, GFL_BMPWIN *win, GFL_MSGDATA *MsgManager  );
extern BMPMENULIST_WORK *WorldTrade_LevelListMake(BMP_MENULIST_DATA **menulist, GFL_BMPWIN *win, GFL_MSGDATA *MsgManager, int tbl_select  );
extern const int WorldTrade_WantLevelTable[];
extern const int WorldTrade_SexStringTable[];
extern u16* WorldTrade_ZukanSortDataGet( int heap, int idx, int* p_arry_num );
extern u16* WorldTrade_ZukanSortDataGet2( int heap, int idx, int* p_arry_num );

extern BMPMENULIST_WORK *WorldTrade_CountryListMake(BMP_MENULIST_DATA **menulist, GFL_BMPWIN *win, GFL_MSGDATA *CountryMsgManager, GFL_MSGDATA *NormalMsgManager);

extern void WorldTrade_PokeNamePrint( GFL_BMPWIN *win, GFL_MSGDATA *nameman, int monsno, int flag, int y, PRINTSYS_LSB color, WT_PRINT *print );
extern void WorldTrade_SexPrint( GFL_BMPWIN *win, GFL_MSGDATA *msgman, int sex, int flag, int y, int printflag, PRINTSYS_LSB color, WT_PRINT *print );
extern void WorldTrade_WantLevelPrint( GFL_BMPWIN *win, GFL_MSGDATA *msgman, int level, int flag, int y, PRINTSYS_LSB color, int tbl_select, WT_PRINT *print );
extern void WorldTrade_WantLevelPrint_XY(GFL_BMPWIN *win, GFL_MSGDATA *msgman, int level, int flag, int x, int y, PRINTSYS_LSB color, int tbl_select, WT_PRINT *print );
extern void WorldTrade_CountryPrint( GFL_BMPWIN *win, GFL_MSGDATA *nameman, GFL_MSGDATA *msgman, int country_code, int flag, int y, PRINTSYS_LSB color, WT_PRINT *print );
extern void WorldTrade_PostPokemonBaseDataMake( Dpw_Tr_Data *dtd, WORLDTRADE_WORK *wk );
extern int  WorldTrade_SexSelectionCheck( Dpw_Tr_PokemonSearchData *dtp, int sex_selection );
extern int  WorldTrade_LevelTermGet( int min, int max, int tbl_select );
extern void WorldTrade_LevelMinMaxSet( Dpw_Tr_PokemonSearchData *dtps, int index, int tbl_select );
extern void WorldTrade_CountryCodeSet( WORLDTRADE_WORK *wk, int country_code );
extern u8  *WorldTrade_SinouZukanDataGet( int heap  );
extern u32  WorldTrade_BmpMenuList_Main( BMPMENULIST_WORK * lw, u16 *posbackup );
extern void WorldTrade_SelectListPosInit( SELECT_LIST_POS *slp);
extern void WorldTrade_SelectNameListBackup( SELECT_LIST_POS *slp, int head, int list, int pos );
extern int  WorldTrade_LevelListAdd( BMP_MENULIST_DATA **menulist, GFL_MSGDATA *MsgManager, int tbl_select);
extern int WorldTrade_NationSortListNumGet( int start, int *number );
extern int WorldTrade_NationSortListMake( BMP_MENULIST_DATA **menulist, GFL_MSGDATA *CountryNameManager, int start );
extern void WorldTrade_PokeNamePrintNoPut( GFL_BMPWIN *win, GFL_MSGDATA *nameman, int monsno, int y, PRINTSYS_LSB color, WT_PRINT *print );
extern void WorldTrade_SexPrintNoPut( GFL_BMPWIN *win, GFL_MSGDATA *msgman, int sex, int flag, int x, int y, PRINTSYS_LSB color, WT_PRINT *print );

// workdtrade_uploade.c
extern int WorldTrade_Upload_Init(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Upload_Main(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Upload_End(WORLDTRADE_WORK *wk, int seq);

// workdtrade_status.c
extern int WorldTrade_Status_Init(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Status_Main(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Status_End(WORLDTRADE_WORK *wk, int seq);

// workdtrade_demo.c
extern int WorldTrade_Demo_Init(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Demo_Main(WORLDTRADE_WORK *wk, int seq);
extern int WorldTrade_Demo_End(WORLDTRADE_WORK *wk, int seq);


//==============================================================================
//	外部データ宣言
//==============================================================================
extern const u32 CountryListTblNum;


