//=============================================================================
/**
 *
 *	@file		fmission_app_sub.c
 *	@brief  フェスミッションアプリ サブ
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "fmission_app_local.h"
#include "fmission_app_sub.h"

/*
 *  @brief  フェスミッションアプリ画面　入力取得
 */
int FMissionApp_InputCheck( FMISSION_APP_WORK* wk )
{
  TOUCHBAR_ICON icon = TOUCHBAR_GetTouch( wk->touchbar );
  int trg = GFL_UI_KEY_GetTrg();
  
  if( icon == TOUCHBAR_ICON_RETURN ){
    return SEQ_FADEOUT;
  }
  return SEQ_MAIN;
}

