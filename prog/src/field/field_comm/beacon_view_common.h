/*
 *  @file   beacon_view_common.h
 *  @brief  すれ違いログ＆フェスミッションログ画面　共有サブルーチン群 ヘッダ
 *  @author Miyuki Iwasawa
 *  @date   10.10.20
 */

#pragma once

#include "app/app_taskmenu.h"

#define ACT_RENDER_ID (0)

//OBJリソース参照コード
enum{
 OBJ_RES_PLTT,
 OBJ_RES_CGR,
 OBJ_RES_CELLANIM,
 OBJ_RES_MAX,
};

//スクロール方向定義
typedef enum{
  SCROLL_UP,
  SCROLL_DOWN,
  SCROLL_RIGHT,
}SCROLL_DIR;

//タスクメニューID
enum{
 TMENU_ID_CYN,    //確認、はい、いいえ
 TMENU_ID_YESNO,  //はい、いいえ
 TMENU_ID_CHECK,  //確認、閉じる
 TMENU_ID_MAX,
};
enum{
 TMENU_CYN_CHECK,
 TMENU_CYN_YES,
 TMENU_CYN_NO,
};
enum{
 TMENU_YESNO_YES,
 TMENU_YESNO_NO,
};
enum{
 TMENU_CHECK_CALL,
 TMENU_CHECK_CLOSE,
};
enum{
 TMENU_STR_YES,      //はい
 TMENU_STR_NO,       //いいえ
 TMENU_STR_CHECK,    //確認
 TMENU_STR_CLOSE,    //閉じる
 TMENU_STR_MAX,
};
#define TMENU_ITEM_MAX  (TMENU_CYN_NO+1)

///ポップアップスクロール
#define POPUP_HEIGHT  (8*8)
#define POPUP_DIFF    (16)
#define POPUP_COUNT   (POPUP_HEIGHT/POPUP_DIFF)
#define POPUP_WAIT    (30*3)

///プリントキューの1syncの処理リミット設定値
#define PRINT_QUE_LIMMIT_DEFAULT (500)
#define PRINT_QUE_LIMMIT_UP      (2000)

typedef struct _POINT{
  int x,y;
}POINT;

typedef struct _TRIANGLE{
  POINT p[3];
}TRIANGLE;

//オブジェリソース管理
typedef struct _OBJ_RES{
  u32   num;
  u32*  tbl;
}OBJ_RES;

typedef struct _OBJ_RES_SRC{
  u16 num;
  u16 srcID;
}OBJ_RES_SRC;

//オブジェリソース管理構造体
typedef struct _OBJ_RES_TBL{
  OBJ_RES res[OBJ_RES_MAX];
}OBJ_RES_TBL;

//キャラクターリソース
typedef struct _RES2D_CHAR{
  NNSG2dCharacterData* p_char;
  void* buf;
}RES2D_CHAR;

//パレットリソース
typedef struct _RES2D_PLTT{
  NNSG2dPaletteData* p_pltt;
  u16* dat;
  void* buf;
}RES2D_PLTT;

// Bmpウィンドウ
typedef struct{
  GFL_BMPWIN*   win;
  GFL_BMP_DATA* bmp;
  
  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  GFL_TCBLSYS*  pTcbSys;

  PRINT_UTIL  putil;
  u8          frm;
}BMP_WIN;

// OAMフォントワーク
typedef struct {
	BMPOAM_ACT_PTR oam;
	GFL_BMP_DATA * bmp;
  
  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  GFL_TCBLSYS*  pTcbSys;
}FONT_OAM;

//TaskMenuワーク
typedef struct {
  APP_TASKMENU_WIN_WORK *work;
  APP_TASKMENU_ITEMWORK item;
}TMENU_ITEM;


//タスクメニュー管理ワーク
typedef struct _BVC_TMENU_WORK{
  HEAPID  heapID;
  APP_TASKMENU_RES* menuRes;

  u8  c_id;   //現在のメニューID
  u8  c_max;  //現在のメニューの項目数max
  STRBUF* str[TMENU_STR_MAX];
  TMENU_ITEM tmenu[TMENU_ITEM_MAX];
}BVC_TMENU_WORK;

/*
 *  @brief  三角形あたり判定
 *
 *  三角形は反時計回り
 * 1-0 p-a, b-a
 * 2-1 p-b, c-b
 * 0-2 p-c, a-c
 */
extern BOOL BVCCalc_PointInTriangle( POINT* p, TRIANGLE* tri );

/*
 *  @brief  SE再生
 */
extern void BVCSound_PlaySE( u16 se_no );

/*
 *  @brief  アクター　座標セット
 */
extern void BVCAct_SetPosition( GFL_CLWK* act, s16 x, s16 y );

/*
 *  @brief  アクター アニメスタート
 */
extern void BVCAct_AnmStart( GFL_CLWK* act, u8 anm_no );

/*
 *  @brief  プリントキュー　処理スピードアップリクエスト
 *
 *  処理分割数を減らし、処理負荷を上げる代わりに、
 *  プリント処理終了までのスピードをアップする
 */
extern void BVC_PrintQueLimmitUpSet( PRINT_QUE *printQue, BOOL flag );

//========================================================================
//選択メニュー処理系
//========================================================================
//---------------------------------------------------------------
/*
 *  @brief  TaskMenu リソース初期化
 */
//---------------------------------------------------------------
extern BVC_TMENU_WORK* BVCTMenu_Init( u8 frame, u8 plt, GFL_FONT *fontHandle, PRINT_QUE *printQue, HEAPID heapID );

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu リソース開放
 */
//---------------------------------------------------------------
extern void BVCTMenu_Release( BVC_TMENU_WORK* wk );

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu YesNo生成
 */
//---------------------------------------------------------------
extern void BVCTMenu_Create( BVC_TMENU_WORK* wk, u8 menu_idx );

//---------------------------------------------------------------
/*
 *  @brief  TaskMenu ウィンドウワークを取得する
 */
//---------------------------------------------------------------
extern APP_TASKMENU_WIN_WORK* BVCTMenu_GetWinWork( BVC_TMENU_WORK* wk, u8 select );

/*
 *  @brief  TaskMenu アップデート
 */
extern int BVCTMenu_Update( BVC_TMENU_WORK* wk );

/*
 *  @brief  TaskMenu 終了アニメ待ち
 */
extern BOOL BVCTMenu_EndWait( BVC_TMENU_WORK* wk, u8 select );

/*
 *  @brief  TaskMenu 終了
 */
extern void BVCTMenu_End( BVC_TMENU_WORK* wk );

//========================================================================
//文字列処理系
//========================================================================
//------------------------------------------------------------
/*
 *  @brief  ポップアップ　タイムウェイトチェック
 */
//------------------------------------------------------------
extern BOOL BVCPrint_PopupWindowTimeWaitCheck( u8* p_wait, u8 frame );

//------------------------------------------------------------
/**
 * ビーコン情報から表示するメッセージIDを取得する
 *
 * @param   info		対象のビーコン情報へのポインタ
 */
//------------------------------------------------------------
extern u32 BVCPrint_GetBeaconMsgID(const GAMEBEACON_INFO *info);

//------------------------------------------------------------
/**
 * FontOam構造体の初期化
 *
 * @param FONT_OAM*
 * @param BMPOAM_ACT_DATA*
 * @param bsp BMPOAM_SYS_PTR
 * @param printQue  PRINT_QUE*
 * @param fontHandle  GFL_FONT*
 * @param pTcbSys  GFL_TCBLSYS*
 */
//------------------------------------------------------------
extern void BVCPrint_FontOamAdd( FONT_OAM* foam, BMPOAM_ACT_DATA* finit, BMPOAM_SYS_PTR bsp, PRINT_QUE* que, GFL_FONT* font, GFL_TCBLSYS* tcbsys );

//------------------------------------------------------------
/**
 * FontOam構造体の削除
 *
 * @param FONT_OAM*
 */
//------------------------------------------------------------
extern void BVCPrint_FontOamDel( FONT_OAM* foam );


/////////////////////////////////////////////////////////////////////
/*
 *  @brief  BMPウィンドウメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
extern void BVCPrintReq_BmpwinPrintEx( BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct, BOOL direct_f, BOOL clear_f );
extern void BVCPrintReq_BmpwinPrint( BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct, BOOL direct_f );
extern void BVCPrintReq_FOamPrintEx( FONT_OAM* foam, STRBUF* str, PRINTSYS_LSB color, int* task_ct, BOOL direct_f, BOOL clear_f );
extern void BVCPrintReq_FOamPrint( FONT_OAM* foam, STRBUF* str, PRINTSYS_LSB color, int* task_ct, BOOL direct_f );

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ アップダウン 
 */
/////////////////////////////////////////////////////////////////////
extern void BVCTaskAdd_MsgUpdown(   GFL_TCBLSYS*  pTcbSys, u8 frame, u8 dir, int* task_ct );


