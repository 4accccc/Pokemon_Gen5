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

//タスクメニュー ワーク
typedef struct _APP_TASKMENU_WORK APP_TASKMENU_WORK;

//位置指定が左上か右下か
typedef enum
{
  ATPT_LEFT_UP,     //左上
  ATPT_RIGHT_DOWN,  //右下
  
}APP_TASKMENU_POS_TYPE;

//タスクメニュー 初期化ワーク １項目
typedef struct
{
  STRBUF  *str;
  PRINTSYS_LSB msgColor;
  BOOL    isReturn;       //戻るマークの表示
}APP_TASKMENU_ITEMWORK;

//タスクメニュー 初期化ワーク
typedef struct
{
  int heapId;
  
  u8  itemNum;
  APP_TASKMENU_ITEMWORK *itemWork;
  
  //BG関係
  u8  bgFrame;
  u8  palNo;    //パレット2本使います
  
  //位置
  APP_TASKMENU_POS_TYPE posType;
  u8  charPosX; //ウィンドウ開始位置(キャラ単位
  u8  charPosY;
  
  //メッセージ関係
  GFL_MSGDATA *msgHandle;
  GFL_FONT    *fontHandle;
  PRINT_QUE   *printQue;
}APP_TASKMENU_INITWORK;
//※APP_TASKMENU_INITWORKはOpenMenu後開放しても問題ありません。

extern APP_TASKMENU_WORK* APP_TASKMENU_OpenMenu( APP_TASKMENU_INITWORK *initWork );
extern void APP_TASKMENU_CloseMenu( APP_TASKMENU_WORK *work );
extern void APP_TASKMENU_UpdateMenu( APP_TASKMENU_WORK *work );
extern const BOOL APP_TASKMENU_IsFinish( APP_TASKMENU_WORK *work );
extern const u8 APP_TASKMENU_GetCursorPos( APP_TASKMENU_WORK *work );

extern void APP_TASKMENU_UpdatePalletAnime( u16 *anmCnt , u16 *transBuf , u8 pltNo );

