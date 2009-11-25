//==============================================================================
/**
 * @file    monolith_common.h
 * @brief   モノリス画面共通ヘッダ
 * @author  matsuda
 * @date    2009.11.21(土)
 */
//==============================================================================
#pragma once

#include "print\printsys.h"
#include "print\gf_font.h"
#include "print\wordset.h"
#include "system/palanm.h"
#include "system/actor_tool.h"
#include "system/bmp_oam.h"


//==============================================================================
//  定数定義
//==============================================================================
///共通素材展開アドレス
enum{
  MONO_BG_COMMON_CHARBASE = GX_BG_CHARBASE_0x04000,   ///<共通素材BGキャラクタベース
  MONO_BG_COMMON_CHAR_SIZE = 0x8000,                  ///<共通素材キャラクタサイズ
  
  MONO_BG_COMMON_SCRBASE = GX_BG_SCRBASE_0x0000,      ///<共通素材BGスクリーンベース
  MONO_BG_COMMON_SCR_SIZE = 0x800,                    ///<共通素材BGスクリーンサイズ
};

///上画面フォントパレット番号
#define MONOLITH_BG_UP_FONT_PALNO   (13)
///下画面フォントパレット番号
#define MONOLITH_BG_DOWN_FONT_PALNO   (MONOLITH_BG_UP_FONT_PALNO)

///共通素材Indexへのアクセス
typedef enum{
  COMMON_RESOURCE_INDEX_UP,     ///<上画面用共通素材INDEXへのアクセス
  COMMON_RESOURCE_INDEX_DOWN,   ///<下画面用共通素材INDEXへのアクセス
  
  COMMON_RESOURCE_INDEX_MAX,
}COMMON_RESOURCE_INDEX;

//==============================================================================
//  構造体定義
//==============================================================================
///モノリス全画面共通設定データ
typedef struct{
  GAMESYS_WORK *gsys;
  PALETTE_FADE_PTR pfd;
  PLTTSLOT_SYS_PTR plttslot;
  PRINT_QUE *printQue;
  GFL_FONT *font_handle;
  WORDSET *wordset;
	GFL_CLUNIT *clunit;
	GFL_TCBLSYS *tcbl_sys;
	GFL_MSGDATA *mm_mission_mono;   ///<mision_monolith.gmm
	GFL_MSGDATA *mm_power;          ///<power.gmm
	GFL_MSGDATA *mm_power_explain;  ///<power_explain.gmm
	GFL_MSGDATA *mm_monolith;       ///<monolith.gmm
	BMPOAM_SYS_PTR bmpoam_sys;
	
	struct{
    u32 pltt_index;          ///<アクター共通素材パレットIndex
    u32 char_index;          ///<アクター共通素材キャラIndex
    u32 cell_index;          ///<アクター共通素材セルIndex
    u32 pltt_bmpfont_index;  ///<BMPフォントパレットIndex
  }common_res[COMMON_RESOURCE_INDEX_MAX];
}MONOLITH_SETUP;

///モノリス全画面共用ワーク(上下PROC間、画面をまたいだワークのやり取りに使用)
typedef struct{
  u8 mission_select_town;           ///<ミッション画面：選択している街番号
  u8 power_select_no;               ///<パワー画面：選択しているパワー番号
  u8 padding[2];
}MONOLITH_COMMON_WORK;

///モノリスAPP PROC用ParentWork
typedef struct{
  MONOLITH_SETUP *setup;            ///<モノリス全画面共通設定データへのポインタ
  MONOLITH_COMMON_WORK *common;     ///<モノリス全画面共用ワークへのポインタ
  u8 next_menu_index;     ///<APP Proc終了時に次の画面のメニューIndexをセットする
  u8 padding[3];
}MONOLITH_APP_PARENT;



//==============================================================================
//  外部データ
//==============================================================================
extern const GFL_PROC_DATA MonolithAppProc_Down_MissionSelect;
