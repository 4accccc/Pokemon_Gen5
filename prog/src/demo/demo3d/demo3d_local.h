/*
 *  @file   demo3d_local.h
 *  @brief  demo3d共通関連ファイル共有ヘッダ
 *
 *  @author Miyuki Iwasawa
 *  @date   10.02.25
 */

#pragma once

#include "demo/demo3d.h"
#include "demo/demo3d_demoid.h"

///シーン呼び出しパラメータ格納構造体
typedef struct _DEMO3D_SCENE_ENV{
  DEMO3D_ID demo_id;
  u32       start_frame;

  u8        scene_id;
  u8        time_zone;
  u8        player_sex;
}DEMO3D_SCENE_ENV;

// キー同時押しマクロ
#define CHECK_KEY_TRG( key ) ( (GFL_UI_KEY_GetTrg() & (key) ) == (key) )
#define CHECK_KEY_CONT( key ) ( (GFL_UI_KEY_GetCont() & (key) ) == (key) )

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BAR_M	= GFL_BG_FRAME1_M,
	BG_FRAME_POKE_M	= GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M	= GFL_BG_FRAME3_M,
	BG_FRAME_BACK_S	= GFL_BG_FRAME2_S,
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
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
	PLTID_BG_INFOWIN_M		= 15,
	//サブBG
	PLTID_BG_BACK_S				=	0,

	//メインOBJ
	PLTID_OBJ_TOUCHBAR_M	= 0, // 3本使用
	PLTID_OBJ_TOWNMAP_M	= 14,		

  //サブOBJ
  PLTID_OBJ_COMMON_S = 0,
};


