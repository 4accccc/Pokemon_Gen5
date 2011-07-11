//============================================================================
/**
 *
 *	@file		gpower_app.h
 *	@brief  デルパワー使用画面
 *	@author	
 *	@data		2011.07.11
 */
//============================================================================
#pragma once
//	lib
#include <gflib.h>

//system
#include "gamesystem/gamesystem.h"
#include "savedata/intrude_save.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern const GFL_PROC_DATA GPowerAppProcData;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
typedef enum{
  GPOWER_APP_MODE_NORMAL,   ///<通常モード
  GPOWER_APP_MODE_FMISSION, ///<フェスミッションモード(ハイリンクボタン無し
}GPOWER_APP_MODE;

typedef enum{
  GPOWER_APP_RET_NULL,    ///<イベント無し
  GPOWER_APP_RET_GPOWER,  ///<デルパワー使用
  GPOWER_APP_RET_HILINK,  ///<ハイリンクへジャンプ
}GPOWER_APP_RET_MODE;

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
///	PROCに渡す引数
//=====================================
typedef struct
{	
//	GAMESYS_WORK		*gsys;	//[in]ゲームシステム
//	GAMEDATA				*gdata;	//[in]ゲームシステム
  GPOWER_APP_MODE mode; ///<[in]呼び出しモード
  GPOWER_ID       gpower[INTRUDE_EQUIP_GPOWER_MAX]; ///<[in]現在装備中のデルパワー(配列に中抜けは無い)
  u8              gpower_num; ///<[in]現在装備中のパワー数

  ///<[out]
  GPOWER_APP_RET_MODE   ret_mode;   ///<リターンモード
  GPOWER_ID             ret_gpower; ///<使用したデルパワーID

}GPOWER_APP_PARAM;

