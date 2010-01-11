//=============================================================================
/**
 * @file	wifi_p2pmatch_local.h
 * @brief	WIFICLUBローカル定義
 * @author	k.ohno
 * @date    2006.4.5
 */
//=============================================================================


#pragma once


#include <gflib.h>
#include "wifi_status.h"
#include "savedata/wifilist.h"
#include "savedata/config.h"
#include "print/wordset.h"
#include "wifi_p2pmatchroom.h"
#include "system/bmp_oam.h"
#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "net_app/connect_anm.h"
#include "poke_tool/pokeparty.h"


#define WIFIP2PMATCH_MEMBER_MAX  (WIFILIST_FRIEND_MAX)
#define WIFIP2PMATCH_DISP_MEMBER_MAX  (5)

// 機能ボタン用定義
#define FUNCBUTTON_NUM	( 7 ) 		// 機能ボタンの数
#define START_WORDPANEL ( 0 )		// 最初の文字入力パネルの番号（０＝ひらがな）

// CLACTで定義しているセルが大きすぎてサブ画面に影響がでてしまうので離してみる
#define NAMEIN_SUB_ACTOR_DISTANCE 	(256)

// CellActorに処理させるリソースマネージャの種類の数（＝マルチセル・マルチセルアニメは使用しない）
#define CLACT_RESOURCE_NUM		(  4 )
#define _OAM_NUM			( 5 )

#define FRIENDCODE_MAXLEN     (12)

// マッチングルームにはいてくる人の数（自分も入れて）
#define MATCHROOM_IN_NPCNUM	(32)
#define MATCHROOM_IN_OBJNUM	(MATCHROOM_IN_NPCNUM+1)

// メッセージ表示後のWAIT
#define WIFIP2PMATCH_CORNER_MESSAGE_END_WAIT	( 60 )


// 文字パネルの遷移用
enum{
  WIFIP2PMATCH_MODE_INIT  = 0,
  WIFIP2PMATCH_NORMALCONNECT_YESNO,
  WIFIP2PMATCH_NORMALCONNECT_WAIT,
  WIFIP2PMATCH_DIFFER_MACHINE_INIT,
  WIFIP2PMATCH_DIFFER_MACHINE_NEXT,
  WIFIP2PMATCH_DIFFER_MACHINE_ONEMORE,
  WIFIP2PMATCH_FIRST_YESNO,
  WIFIP2PMATCH_POWEROFF_INIT,
  WIFIP2PMATCH_POWEROFF_YESNO,
  WIFIP2PMATCH_POWEROFF_WAIT,
  WIFIP2PMATCH_RETRY_INIT,        //10
  WIFIP2PMATCH_RETRY_YESNO,
  WIFIP2PMATCH_RETRY_WAIT,
  WIFIP2PMATCH_RETRY,
  WIFIP2PMATCH_CONNECTING_INIT,
  WIFIP2PMATCH_CONNECTING,
  WIFIP2PMATCH_FIRST_ENDMSG,
  WIFIP2PMATCH_FIRST_ENDMSG_WAIT,
  WIFIP2PMATCH_FRIENDLIST_INIT,
  WIFIP2PMATCH_MODE_FRIENDLIST,
  WIFIP2PMATCH_VCHATWIN_WAIT,           //20
  WIFIP2PMATCH_MODE_VCT_CONNECT_INIT2,
  WIFIP2PMATCH_MODE_VCT_CONNECT_INIT,
  WIFIP2PMATCH_MODE_VCT_CONNECT_WAIT,
  WIFIP2PMATCH_MODE_VCT_CONNECT,
  WIFIP2PMATCH_MODE_VCT_CONNECTEND_YESNO,
  WIFIP2PMATCH_MODE_VCT_CONNECTEND_WAIT,
  WIFIP2PMATCH_MODE_VCT_DISCONNECT,
  WIFIP2PMATCH_MODE_BATTLE_DISCONNECT,
  WIFIP2PMATCH_MODE_DISCONNECT,
  WIFIP2PMATCH_MODE_BATTLE_CONNECT_INIT,  //30
  WIFIP2PMATCH_MODE_BATTLE_CONNECT_WAIT,
  WIFIP2PMATCH_MODE_BATTLE_CONNECT,
  WIFIP2PMATCH_MODE_MAIN_MENU,
  WIFIP2PMATCH_MODE_END_WAIT,
  WIFIP2PMATCH_MODE_CHECK_AND_END,
  WIFIP2PMATCH_MODE_SELECT_INIT,
  WIFIP2PMATCH_MODE_SELECT_WAIT,
  WIFIP2PMATCH_MODE_SUBBATTLE_WAIT,
  WIFIP2PMATCH_MODE_SELECT_REL_INIT,
  WIFIP2PMATCH_MODE_SELECT_REL_YESNO,       //40
  WIFIP2PMATCH_MODE_SELECT_REL_WAIT,
  WIFIP2PMATCH_MODE_MATCH_INIT,
  WIFIP2PMATCH_MODE_MATCH_INIT2,
  WIFIP2PMATCH_MODE_MATCH_WAIT,
  WIFIP2PMATCH_MODE_MATCH_LOOP,
  WIFIP2PMATCH_MODE_BCANCEL_YESNO,
  WIFIP2PMATCH_MODE_BCANCEL_WAIT,
  WIFIP2PMATCH_MODE_CALL_INIT,
  WIFIP2PMATCH_MODE_CALL_YESNO,
  WIFIP2PMATCH_MODE_CALL_SEND,
  WIFIP2PMATCH_MODE_CALL_CHECK,             //50
  WIFIP2PMATCH_MODE_MYSTATUS_WAIT,
  WIFIP2PMATCH_MODE_CALL_WAIT,
  WIFIP2PMATCH_MODE_PERSONAL_INIT,
  WIFIP2PMATCH_MODE_PERSONAL_WAIT,
  WIFIP2PMATCH_MODE_PERSONAL_END,
  WIFIP2PMATCH_MODE_EXIT_YESNO,
  WIFIP2PMATCH_MODE_EXIT_WAIT,
  WIFIP2PMATCH_MODE_EXITING,
  WIFIP2PMATCH_MODE_EXIT_END,
  WIFIP2PMATCH_NEXTBATTLE_YESNO,         //60
  WIFIP2PMATCH_NEXTBATTLE_WAIT,
  WIFIP2PMATCH_MODE_VCHAT_NEGO,
  WIFIP2PMATCH_MODE_VCHAT_NEGO_WAIT,
  WIFIP2PMATCH_RECONECTING_WAIT,
  WIFIP2PMATCH_MODE_BCANCEL_YESNO_VCT,
  WIFIP2PMATCH_MODE_BCANCEL_WAIT_VCT,
  WIFIP2PMATCH_PARENT_RESTART,
  WIFIP2PMATCH_FIRST_SAVING,
  WIFIP2PMATCH_MODE_CANCEL_ENABLE_WAIT,
  WIFIP2PMATCH_FIRST_SAVING2,       //70
  WIFIP2PMATCH_MODE_CALLGAME_INIT, 

};


enum{
	WIFIP2PMATCH_SYNCHRONIZE_END=201,
};

// 上下画面指定定義
#define BOTH_LCD	( 2 )
#define MAIN_LCD	( GF_BGL_MAIN_DISP )	// 要は０と
#define SUB_LCD		( GF_BGL_SUB_DISP )		// １なんですが。

// BMPWIN指定
enum{
	BMP_NAME1_S_BG0,
	BMP_NAME2_S_BG0,
	BMP_NAME3_S_BG0,
	BMP_NAME4_S_BG0,
	BMP_NAME5_S_BG0,
	BMP_WIFIP2PMATCH_MAX,
};


typedef enum{
  _REGULATION_BATTLE_TOWER     // バトルタワー対戦方式
} REGULATION_BATTLE_e;


//トータル189バイト送信できるがとりあえず下位プログラムは範囲分だけ送る
#define _POKEMON_NUM   (6)

#define _CANCELENABLE_TIMER (60*30)   // キャンセルになる為のタイマー60min


typedef enum{
	WF_VIEW_STATUS,
	WF_VIEW_VCHAT,
	WF_VIEW_STATUS_NUM,
} WF_VIEW_STATUS_e;
/*
typedef enum{
	WF_USERDISPTYPE_NRML,	// 通常
	WF_USERDISPTYPE_MINI,	// ミニゲーム
	WF_USERDISPTYPE_BLTW,	// バトルタワー
	WF_USERDISPTYPE_BLFT,	// バトルフロンティア
	WF_USERDISPTYPE_BLKS,	// バトルキャッスル
	WF_USERDISPTYPE_BTST,	// バトルステージ
	WF_USERDISPTYPE_BTRT,	// バトルルーレット
	WF_USERDISPTYPE_NUM,
} WF_USERDISPTYPE__e;
*/
// ユーザ表示ボタン数
typedef enum{
	MCV_USERD_BTTN_LEFT = 0,
	MCV_USERD_BTTN_BACK,
	MCV_USERD_BTTN_RIGHT,
	MCV_USERD_BTTN_NUM,
} MCV_USERD_BTTN_e;

typedef struct _WIFI_MACH_STATUS_tag{
  u16 pokemonType[_POKEMON_NUM];
  u16 hasItemType[_POKEMON_NUM];
  u8 version;
  u8 regionCode;
  u8 pokemonZukan;
  u8 status;
  u8 regulation;
  u8 trainer_view;
  u8 sex;
  u8 nation;
  u8 area;
  u8 vchat;
  u8 vchat_org;
} _WIFI_MACH_STATUS;


//============================================================================================
//	構造体定義
//============================================================================================

typedef struct {
  _WIFI_MACH_STATUS myMatchStatus;   // 自分のマッチング状態データ
  //_WIFI_MACH_STATUS friendMatchStatus[WIFIP2PMATCH_MEMBER_MAX]; // 相手のマッチング状態データ
} TEST_MATCH_WORK;

//-------------------------------------
///	Iconグラフィック
//=====================================
typedef struct {
	void* p_buff;
	NNSG2dScreenData* p_scrn;

	void* p_charbuff;
	NNSG2dCharacterData* p_char;
} WIFIP2PMATCH_ICON;


//-------------------------------------
///	データビューアーデータ
//=====================================
typedef struct {
	// ワードセット
	WORDSET*	p_wordset;			// メッセージ展開用ワークマネージャー
	
	// ボタングラフィック
	void* p_bttnbuff;
	NNSG2dScreenData* p_bttnscrn;

	void* p_userbuff[1];
	NNSG2dScreenData* p_userscrn[1];

	void* p_useretcbuff;
	NNSG2dScreenData* p_useretcscrn;
	
	GFL_BUTTON_MAN* p_bttnman;	// ボタン管理システム
	u8 bttnfriendNo[ MATCHROOM_IN_NPCNUM ];
	u8 frame_no;	// 今表示している床のナンバー
	u8 touch_friendNo;	// 触れている友達番号+1
	u8 touch_frame;	// フレーム数
	u8 user_disp;		// ユーザーデータ表示ONOFF
	s8 user_dispno;	// ユーザーDISP内容
	u8 button_on;		// ボタン表示更新
	u8 button_count;	// ボタンフレームカウンタ
	u8 bttn_chg_friendNo;	// ボタンを更新してほしい友達番号
	BOOL bttn_allchg;	// ボタンすべてを更新するか 

	// 表示ビットマップ面
	GFL_BMPWIN*	  nameWin[ WCR_MAPDATA_1BLOCKOBJNUM ];
	GFL_BMPWIN*	  statusWin[ WCR_MAPDATA_1BLOCKOBJNUM ][ WF_VIEW_STATUS_NUM ];
	GFL_BMPWIN*	  userWin;

	// ボタン
	u32	button_res[ 4 ];
  u32 btnCGRid;
  u32 btnCLRid;
  u32 btnCERid;

//	CLACT_WORK_PTR button_act[MCV_USERD_BTTN_NUM];
   GFL_CLWK* button_act[MCV_USERD_BTTN_NUM];
#if 1
//  CHAR_MANAGER_ALLOCDATA back_fontoam_cg;
  BMPOAM_SYS_PTR back_fontoam;	// もどる用FONTOAM
  BMPOAM_ACT_PTR BmpOamAct;
  GFL_BMP_DATA*  BmpOamBmp;
#endif
  u32 buttonact_on;			// ボタン動作モード
	u32 touch_button;
	u32 touch_button_event;
	GFL_BUTTON_MAN* p_oambttnman;	// ボタン管理システム
} WIFIP2PMATCH_VIEW;



struct _WIFIP2PMATCH_WORK{
	GFL_TCB					*vblankFunc;
  WIFI_LIST* pList;				// セーブデータ内のユーザーとフレンドデータ
  WIFI_STATUS* pMatch;		// サーバーと送受信する自分の状態
//  TEST_MATCH_WORK* pMatch;		// サーバーと送受信する自分の状態

  int friendMatchReadCount;		// ビーコンを受信した友達数
  u8 index2No[WIFIP2PMATCH_MEMBER_MAX];	// フレンドナンバー配列
  u8 index2NoBackUp[WIFIP2PMATCH_MEMBER_MAX];	// フレンドナンバー配列バックアップ
  int matchStatusBackup[WIFIP2PMATCH_MEMBER_MAX];// 友達ビーコンデータが変わったかﾁｪｯｸ用
  int matchVchatBackup[WIFIP2PMATCH_MEMBER_MAX];// 友達ビーコンデータが変わったかﾁｪｯｸ用
//  NAMEIN_PARAM*		nameinParam;
  
  // メインリスト用ワーク
  BMP_MENULIST_DATA*   menulist;
  BMPMENULIST_WORK* lw;		// BMPメニューワーク
  
  BMP_MENULIST_DATA*   submenulist;
  BMPMENULIST_WORK* sublw;		// BMPメニューワーク
//  GFL_BG_INI		*bgl;									// GF_BGL_INI
  SAVE_CONTROL_WORK*  pSaveData;
GAMEDATA* pGameData;
  POKEPARTY* pMyPoke;
  void* pEmail;
  void* pFrontier;
  CONFIG* pConfig;
  WORDSET			*WordSet;								// メッセージ展開用ワークマネージャー
  GFL_MSGDATA *MsgManager;							// 名前入力メッセージデータマネージャー
  GFL_MSGDATA *SysMsgManager;  //
  GFL_FONT 			*fontHandle;
 // STRBUF			*TrainerName[WIFIP2PMATCH_MEMBER_MAX];		// 名前
//  STRBUF			*MsgString;								// メッセージ
  STRBUF*         pExpStrBuf;
  STRBUF			*TalkString;							// 会話メッセージ用
  STRBUF			*TitleString;							// タイトルメッセージ用
//  STRBUF			*MenuString[4];							// メニューメッセージ用
  STRBUF*         pTemp;        // 入力登録時の一時バッファ
  
  int				MsgIndex;								// 終了検出用ワーク
  BMPMENU_WORK* pYesNoWork;
  void* timeWaitWork;			// タイムウエイトアイコンワーク
//  CLACT_SET_PTR 			clactSet;								// セルアクターセット
  GFL_CLUNIT* clactSet;								// セルアクターセット
  GFL_CLSYS_REND*          renddata;						// 簡易レンダーデータ
  //CLACT_U_EASYRENDER_DATA	renddata;								// 簡易レンダーデータ
  //CLACT_U_RES_MANAGER_PTR	resMan[CLACT_RESOURCE_NUM];				// リソースマネージャ
//  FONTOAM_SYS_PTR			fontoam;								// フォントOAMシステム  @@OO
/*
  CLACT_U_RES_OBJ_PTR 	resObjTbl[BOTH_LCD][CLACT_RESOURCE_NUM];// リソースオブジェテーブル
  CLACT_HEADER			clActHeader_m;							// セルアクターヘッダー
  CLACT_HEADER			clActHeader_s;							// セルアクターヘッダー
  CLACT_WORK_PTR			MainActWork[_OAM_NUM];				// セルアクターワークポインタ配列
	//CLACT_WORK_PTR			SubActWork[_OAM_NUM];				// セルアクターワークポインタ配列
//*/

  GFL_BMPWIN*			MsgWin;									// 会話ウインドウ
  GFL_BMPWIN*			MyInfoWin;								// 自分の状態表示
  GFL_BMPWIN*			MyInfoWinBack;								// タイトル
  GFL_BMPWIN*			SysMsgWin;								// システムウィンドウで描画するもの	えらーや、DWCルールのメッセージ
  GFL_BMPWIN*			ListWin;									// フレンドリスト
  GFL_BMPWIN*			SubListWin;									// 募集するタイプなどを描画するリスト
  GFL_BMPWIN*			MyWin;									// 友達の個人情報表示
  PRINT_UTIL            SysMsgPrintUtil;    // システムウインドウPrintUtil
  PRINT_QUE*            SysMsgQue;

   int cancelEnableTimer;   // キャンセル許可になる為のタイマー
  int localTime;
  int seq;									// 現在の文字入力状態（入力OK/アニメ中）など
  int       endSeq;
  int						nextseq;
  int initSeq;
  int						mode;									// 現在最前面の文字パネル
  int						timer;			// 待ち時間や、フラグなどに使用
  MYSTATUS                *pMyStatus;		// 自分のステータス
  u8     mainCur;
  u8     subCur;
  BOOL bInitMessage;		// 初回接続か
  GFL_PROC*		subProc;
   u16 matchState[WIFIP2PMATCH_MEMBER_MAX];   ///<CNM_WFP2PMF_STATUS
  int preConnect;			// 新しく来た友達(-1=なし)
  u16 battleCur;			// バトルタイプ選択メニューカーソル
  u16 singleCur[3];			// バトルの詳細部分のメニューカーソル
  u16 bSingle;				// SINGLEバトル　ダブルバトルスイッチ

 WIFI_STATUS targetStatus;  //接続しようとしている人のステータス

//  u16 keepStatus;			// 接続しようとしたら相手のステータスが変わったときのﾁｪｯｸ用
//  u16 keepVChat;			// 接続しようとしたらボイスチャット状態が変わったときのﾁｪｯｸ用
  u16 friendNo;			// 今つながっている友達ナンバー
  BOOL bRetryBattle;
   int vctEnc;
	WIFI_MATCHROOM matchroom;	// マッチングルームワーク
	MCR_MOVEOBJ*	p_matchMoveObj[ MATCHROOM_IN_OBJNUM ];
	WIFIP2PMATCH_ICON icon;	// アイコングラフィック
	WIFIP2PMATCH_VIEW view;	// 友達データビューアー

	BOOL friend_request_wait;	// 友達募集中にTRUEになるフラグ
	
	CONNECT_BG_PALANM cbp;		// Wifi接続画面のBGパレットアニメ制御構造体

	u16 brightness_reset;	// _retryでマスター輝度をVBlankでリセットするため
	u16 friend_num;			// P2Pmatch画面初期化タイミングの友達数
};


extern void WifiP2PMatchRecvGameStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);




