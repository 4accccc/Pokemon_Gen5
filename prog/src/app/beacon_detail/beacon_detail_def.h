/*
 *  @file beacon_detail_def.h
 *  @brief  すれ違いビーコン詳細画面リテラル定義
 *  @author Miyuki Iwasawa
 *  @date   10.02.02
 */

#pragma once

#define  BEACON_DETAIL_HEAP_SIZE  (0x30000)  ///< ヒープサイズ

enum{
 SEQ_FADEIN,
 SEQ_MAIN,
 SEQ_FADEOUT,
 SEQ_EXIT,
};


//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BAR_M	= GFL_BG_FRAME0_M,
	BG_FRAME_MSG_M	= GFL_BG_FRAME1_M,
	BG_FRAME_MAP01_M	= GFL_BG_FRAME2_M,
	BG_FRAME_MAP02_M	= GFL_BG_FRAME3_M,
	BG_FRAME_DAT01_S	= GFL_BG_FRAME0_S,
	BG_FRAME_WIN01_S	= GFL_BG_FRAME1_S,
	BG_FRAME_DAT02_S	= GFL_BG_FRAME2_S,
	BG_FRAME_WIN02_S	= GFL_BG_FRAME3_S,
};
//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_BACK_M				= 0,
	PLTID_BG_POKE_M				= 1,
	PLTID_BG_TASKMENU_M		= 11,
	PLTID_BG_TOUCHBAR_M		= 13,
	PLTID_BG_LOCALIZE_M		= 15, //ローカライズ用

	//サブBG
	PLTID_BG_BACK_S				=	0,
	PLTID_BG_LOCALIZE_S		= 15, //ローカライズ用

	//メインOBJ
	PLTID_OBJ_TOUCHBAR_M	= 0, // 3本使用
	PLTID_OBJ_LOCALIZE_M	= 14,		//ローカライズ用
  PLTID_OBJ_WMI_M = 15,	  //通信アイコン

  //サブOBJ
  PLTID_OBJ_PMS_DRAW = 0, // 5本使用
	PLTID_OBJ_LOCALIZE_S	= 15,		//ローカライズ用
};


