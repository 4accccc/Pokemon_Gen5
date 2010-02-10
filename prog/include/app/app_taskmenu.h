//======================================================================
/**
 * @file	bmp_taskmenu.h
 * @brief	タスクバー用 共通メニュー
 * @author	ariizumi
 * @data	09/07/27
 *
 * モジュール名：BMP_TASKMENU
 */
//======================================================================

#pragma once
#include "print/printsys.h"

//決定時点滅アニメ
#define APP_TASKMENU_ANM_CNT (16)
#define APP_TASKMENU_ANM_INTERVAL (4)

//デフォルト幅
#define APP_TASKMENU_PLATE_WIDTH (13)	
#define APP_TASKMENU_PLATE_HEIGHT (3)
//デフォルト幅(はい・いいえ用
#define APP_TASKMENU_PLATE_WIDTH_YN_WIN (8)	
#define APP_TASKMENU_PLATE_HEIGHT_YN_WIN (APP_TASKMENU_PLATE_HEIGHT)

//デフォルト文字色カラー
#define APP_TASKMENU_ITEM_MSGCOLOR	(PRINTSYS_LSB_Make(0xe,0xf,0x3))

//プレートのアニメ。sin使うので0～0xFFFFのループ
#define APP_TASKMENU_ANIME_VALUE (0x400)

//単発まどの種類
typedef enum
{	
	APP_TASKMENU_WIN_TYPE_NORMAL,	//通常の何も無し窓
	APP_TASKMENU_WIN_TYPE_RETURN,	//もどる記号が入った窓（←┘）←こんなの
}APP_TASKMENU_WIN_TYPE;

//タスクメニュー用リソース保持ワーク
typedef struct _APP_TASKMENU_RES APP_TASKMENU_RES;

//タスクメニュー ワーク
typedef struct _APP_TASKMENU_WORK APP_TASKMENU_WORK;

//タスクメニュー 単発１つまどワーク
typedef struct _APP_TASKMENU_WIN_WORK APP_TASKMENU_WIN_WORK;

//位置指定が左上か右下か
typedef enum
{
  ATPT_LEFT_UP,     //左上
  ATPT_RIGHT_DOWN,  //右下
  
}APP_TASKMENU_POS_TYPE;

//タスクメニュー 初期化ワーク １項目
typedef struct
{
  STRBUF  *str;											//メニューに表示する文字列
  PRINTSYS_LSB msgColor;						//文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
  APP_TASKMENU_WIN_TYPE type;       //戻るマークの表示
}APP_TASKMENU_ITEMWORK;

//タスクメニュー 初期化ワーク
typedef struct
{
  int heapId;
  
  u8  itemNum;
  APP_TASKMENU_ITEMWORK *itemWork;
  
  //位置
  APP_TASKMENU_POS_TYPE posType;
  u8  charPosX; //ウィンドウ開始位置(キャラ単位
  u8  charPosY;

	//幅	（以前は幅のパラメータはありませんでした。
	//			以前との互換性のため、0が渡ると内部で、デフォルト値にします。
	//			互換性のためだけですので、0は今後渡さないようにしてください）
	u8	w;	//キャラ単位
	u8	h;	//キャラ単位
}APP_TASKMENU_INITWORK;
//※APP_TASKMENU_INITWORKはOpenMenu後開放しても問題ありません。

//リソース読み込み
extern APP_TASKMENU_RES* APP_TASKMENU_RES_Create( u8 frame, u8 plt, GFL_FONT *fontHandle, PRINT_QUE *printQue, HEAPID heapID );
extern void APP_TASKMENU_RES_Delete( APP_TASKMENU_RES *wk );

//縦メニュー
extern APP_TASKMENU_WORK* APP_TASKMENU_OpenMenu( APP_TASKMENU_INITWORK *initWork, const APP_TASKMENU_RES *res );
extern void APP_TASKMENU_CloseMenu( APP_TASKMENU_WORK *work );
extern void APP_TASKMENU_UpdateMenu( APP_TASKMENU_WORK *work );
extern const BOOL APP_TASKMENU_IsFinish( APP_TASKMENU_WORK *work );
extern const u8 APP_TASKMENU_GetCursorPos( APP_TASKMENU_WORK *work );
extern const void APP_TASKMENU_SetActive( APP_TASKMENU_WORK *work, BOOL isActive );

//横メニューなどで使用する、単発窓
//横メニューの際は、「けってい」「やめる」など、複数ウィンドウを作成してください
extern APP_TASKMENU_WIN_WORK * APP_TASKMENU_WIN_Create( const APP_TASKMENU_RES *res, const APP_TASKMENU_ITEMWORK *item, u8 x, u8 y, u8 w, HEAPID heapID );
extern APP_TASKMENU_WIN_WORK * APP_TASKMENU_WIN_CreateEx( const APP_TASKMENU_RES *res, const APP_TASKMENU_ITEMWORK *item, u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
extern void APP_TASKMENU_WIN_Delete( APP_TASKMENU_WIN_WORK *wk );
extern void APP_TASKMENU_WIN_Update( APP_TASKMENU_WIN_WORK *wk );
extern void APP_TASKMENU_WIN_SetActive( APP_TASKMENU_WIN_WORK *wk, BOOL isActive );
extern void APP_TASKMENU_WIN_SetDecide( APP_TASKMENU_WIN_WORK *wk, BOOL isDecide );
extern const BOOL APP_TASKMENU_WIN_IsFinish( APP_TASKMENU_WIN_WORK *work );
extern const BOOL APP_TASKMENU_WIN_IsTrg( const APP_TASKMENU_WIN_WORK *wk );
