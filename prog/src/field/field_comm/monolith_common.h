//==============================================================================
/**
 * @file    monolith_common.h
 * @brief   モノリス画面共通ヘッダ
 * @author  matsuda
 * @date    2009.11.21(土)
 */
//==============================================================================
#pragma once

#include "field/monolith_main.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "print\wordset.h"
#include "system/palanm.h"
#include "system/actor_tool.h"
#include "system/bmp_oam.h"


//==============================================================================
//  定数定義
//==============================================================================
///モノリスメインメニューIndex
typedef enum{
  MONOLITH_MENU_TITLE,      ///<タイトル画面
  MONOLITH_MENU_MISSION,    ///<ミッションを受ける
  MONOLITH_MENU_STATUS,     ///<状態を見る
  MONOLITH_MENU_POWER,      ///<不思議なパワーをもらう
  
  MONOLITH_MENU_END,        ///<モノリス画面終了
}MONOLITH_MENU;

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

///文字色のデフォルト設定
#define MONOLITH_FONT_DEFCOLOR_LETTER   (2)
#define MONOLITH_FONT_DEFCOLOR_SHADOW   (1)
#define MONOLITH_FONT_DEFCOLOR_BACK     (0)

///共通素材Indexへのアクセス
typedef enum{
  COMMON_RESOURCE_INDEX_UP,     ///<上画面用共通素材INDEXへのアクセス
  COMMON_RESOURCE_INDEX_DOWN,   ///<下画面用共通素材INDEXへのアクセス
  
  COMMON_RESOURCE_INDEX_MAX,
}COMMON_RESOURCE_INDEX;

///共通素材セルのアニメシーケンス番号
typedef enum{
  COMMON_ANMSEQ_PANEL_LARGE,
  COMMON_ANMSEQ_PANEL_SMALL,
  COMMON_ANMSEQ_UP,
  COMMON_ANMSEQ_DOWN,
  COMMON_ANMSEQ_BLACK_TOWN,
  COMMON_ANMSEQ_WHITE_TOWN,
  COMMON_ANMSEQ_RETURN,
}COMMON_ANMSEQ;

///共通素材パレットの割り当て
typedef enum{
  COMMON_PAL_PANEL = 0,       ///<パレット(未選択)
  COMMON_PAL_PANEL_FOCUS,     ///<パレット(選択されている)
  
  COMMON_PAL_TOWN = 0,        ///<街アイコン
  
  COMMON_PAL_CURSOR = 0,      ///<上下カーソル(未選択)
  COMMON_PAL_CURSOR_FOCUS = 2,///<上下カーソル(選択されている)
  
  COMMON_PAL_RETURN = 0,      ///<戻るアイコン
}COMMON_PAL;

///パネルカラーモード
typedef enum{
  PANEL_COLORMODE_NONE,       ///<カラーアニメ無し
  PANEL_COLORMODE_FOCUS,      ///<フォーカス中のカラーアニメ
  PANEL_COLORMODE_FLASH,      ///<決定中のフラッシュアニメ
}PANEL_COLORMODE;

///ミッションが実施中の時に街番号領域にセットされる
#define SELECT_TOWN_ENFORCEMENT   (INTRUDE_TOWN_MAX)


//==============================================================================
//  構造体定義
//==============================================================================
///モノリス全画面共通設定データ
typedef struct{
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
	ARCHANDLE *hdl;
	
	struct{
    u32 pltt_index;          ///<アクター共通素材パレットIndex
    u32 char_index;          ///<アクター共通素材キャラIndex
    u32 cell_index;          ///<アクター共通素材セルIndex
    u32 pltt_bmpfont_index;  ///<BMPフォントパレットIndex
  }common_res[COMMON_RESOURCE_INDEX_MAX];
}MONOLITH_SETUP;

///モノリス全画面共用ワーク(上下PROC間、画面をまたいだワークのやり取りに使用)
typedef struct{
  u8 mission_select_town;    ///<ミッション画面：選択している街番号 or SELECT_TOWN_ENFORCEMENT
  u8 power_select_no;        ///<パワー画面：選択しているパワー番号
  u8 padding[2];
}MONOLITH_COMMON_WORK;

///パネルカラーアニメ制御構造体
typedef struct{
  u16 evy;            ///<EVY値(下位8ビット小数)
  u8 mode;            ///<PANEL_COLORMODE_???
  //フェードで使用
  u8 add_dir;         ///<EVY加算方向
  //フラッシュで使用
  u8 wait;            ///<ウェイト
  u8 count;           ///<フラッシュ回数をカウント
  u8 padding[2];
}PANEL_COLOR_CONTROL;

///共有ツールで使用するワーク
typedef struct{
  PANEL_COLOR_CONTROL panel_color;
}MONOLITH_TOOL_WORK;

///モノリスAPP PROC用ParentWork
typedef struct{
  MONOLITH_PARENT_WORK *parent;     ///<親PROCのParentWork
  MONOLITH_SETUP *setup;            ///<モノリス全画面共通設定データへのポインタ
  MONOLITH_COMMON_WORK *common;     ///<モノリス全画面共用ワークへのポインタ
  MONOLITH_TOOL_WORK tool;          ///<共有ツールで使用するワーク
  u8 next_menu_index;     ///<APP Proc終了時に次の画面のメニューIndexをセットする
  u8 up_proc_finish;      ///<TRUE:上画面PROC終了リクエスト
  u8 force_finish;        ///<TRUE:モノリス所有者が居なくなった為、強制終了
  u8 padding;
}MONOLITH_APP_PARENT;



//==============================================================================
//  外部データ
//==============================================================================
extern const GFL_PROC_DATA MonolithAppProc_Up_PalaceMap;
extern const GFL_PROC_DATA MonolithAppProc_Down_Title;
extern const GFL_PROC_DATA MonolithAppProc_Down_MissionSelect;
extern const GFL_PROC_DATA MonolithAppProc_Up_MissionExplain;
extern const GFL_PROC_DATA MonolithAppProc_Down_Status;
