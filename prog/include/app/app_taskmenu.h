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


//タスクメニュー 初期化ワーク １項目
typedef struct
{
  STRBUF  *str;
  PRINTSYS_LSB msgColor;
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
  
  //メッセージ関係
  GFL_MSGDATA *msgHandle;
  GFL_FONT    *fontHandle;
  PRINT_QUE   *printQue;
}APP_TASKMENU_INITWORK;


extern APP_TASKMENU_WORK* APP_TASKMENU_OpenMenu( APP_TASKMENU_INITWORK *initWork );
extern void APP_TASKMENU_CloseMenu( APP_TASKMENU_WORK *work );
extern void APP_TASKMENU_UpdateMenu( APP_TASKMENU_WORK *work );
extern const BOOL APP_TASKMENU_IsFinish( APP_TASKMENU_WORK *work );
extern const u8 APP_TASKMENU_GetCursorPos( APP_TASKMENU_WORK *work );

