//======================================================================
/**
 * @file  plist_local_def.h
 * @brief ポケモンリスト ローカル定義
 * @author  ariizumi
 * @data  09/06/10
 */
//======================================================================
#pragma once

#include "app/pokelist.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "sound/pm_sndsys.h"
#include "debug/debugwin_sys.h"
#include "app/app_taskmenu.h"

#define PLIST_LIST_MAX (6)

//Vram転送タスク
#define PLIST_VTRANS_TASK_NUM (3)

//BG面定義(アルファ設定のところは定義が違うので変えたら変更すること
#define PLIST_BG_MAIN_BG (GFL_BG_FRAME3_M)
#define PLIST_BG_PLATE   (GFL_BG_FRAME2_M)
#define PLIST_BG_PARAM   (GFL_BG_FRAME1_M)
#define PLIST_BG_MENU    (GFL_BG_FRAME0_M)

#define PLIST_BG_SUB_BG (GFL_BG_FRAME3_S)
//バトル上用
#define PLIST_BG_SUB_BATTLE_WIN (GFL_BG_FRAME2_S)
#define PLIST_BG_SUB_BATTLE_BAR (GFL_BG_FRAME1_S)
#define PLIST_BG_SUB_BATTLE_STR (GFL_BG_FRAME0_S)

//土台とパラメータのスクロール分(キャラ単位
#define PLIST_BG_SCROLL_X_CHAR (16)

//BGパレット
#define PLIST_BG_PLT_MENU_ACTIVE (0x01)
#define PLIST_BG_PLT_MENU_NORMAL (0x02)
#define PLIST_BG_PLT_BAR (0x0b)
#define PLIST_BG_PLT_BMPWIN (0x0c)
#define PLIST_BG_PLT_HP_BAR (0x0d)
#define PLIST_BG_PLT_FONT (0x0e)

#define PLIST_BG_SUB_PLT_BACK (0x00)  //1本
#define PLIST_BG_SUB_PLT_PLATE (0x01) //1本
#define PLIST_BG_SUB_PLT_BAR (0x02) //1本
#define PLIST_BG_SUB_PLT_FONT (0x0e)      //1本


//BGキャラ(BmpWin
#define PLIST_BG_WINCHAR_TOP (1)

//文字色
#define PLIST_FONT_MSG_BACK (0xF)
#define PLIST_FONT_MSG_LETTER (0x1)
#define PLIST_FONT_MSG_SHADOW (0x2)
#define PLIST_FONT_COLOR_BLACK (PRINTSYS_LSB_Make(1,2,0))

//プレートの文字はHPバーと同じパレット
#define PLIST_FONT_PARAM_LETTER (0x1)
#define PLIST_FONT_PARAM_SHADOW (0x2)
#define PLIST_FONT_PARAM_LETTER_BLUE (0x5)
#define PLIST_FONT_PARAM_SHADOW_BLUE (0x6)
#define PLIST_FONT_PARAM_LETTER_RED (0x3)
#define PLIST_FONT_PARAM_SHADOW_RED (0x4)

//メニューの文字はフォントパレットではなくメニュープレートのパレットを使う
#define PLIST_FONT_MENU_BACK (0x0)
#define PLIST_FONT_MENU_LETTER (0xe)
#define PLIST_FONT_MENU_WAZA_LETTER (0xd)
#define PLIST_FONT_MENU_SHADOW (0xf)

//バトル用のアイコンは普通のフォント
#define PLIST_FONT_BATTLE_PARAM PRINTSYS_LSB_Make( 0xF , 2 , 0 )
#define PLIST_FONT_BATTLE_BLUE PRINTSYS_LSB_Make( 5 , 6 , 0 )
#define PLIST_FONT_BATTLE_RED  PRINTSYS_LSB_Make( 3 , 0xE , 0 )

//文字表示位置
#define PLIST_MSG_STR_OFS_X (1)
#define PLIST_MSG_STR_OFS_Y (1)

//HP色
#define PLIST_HPBAR_COL_GREEN_IN  (0xa)
#define PLIST_HPBAR_COL_GREEN_OUT (0xb)
#define PLIST_HPBAR_COL_YELLOW_IN  (0xc)
#define PLIST_HPBAR_COL_YELLOW_OUT (0xd)
#define PLIST_HPBAR_COL_RED_IN  (0xe)
#define PLIST_HPBAR_COL_RED_OUT (0xf)


//プレートのアニメする色
#define PLIST_MENU_ANIME_COL (0x7)


//OBJリソースIdx
typedef enum
{
  PCR_PLT_OBJ_COMMON,
  PCR_PLT_BAR_ICON,
  PCR_PLT_ITEM_ICON,
  PCR_PLT_ITEM_ICON_SUB,
  PCR_PLT_CONDITION,
  PCR_PLT_HP_BASE,
  PCR_PLT_POKEICON,
  PCR_PLT_POKEICON_SUB,
  
  PCR_NCG_CURSOR,
  PCR_NCG_BALL,
  PCR_NCG_BAR_ICON,
  PCR_NCG_ITEM_ICON,
  PCR_NCG_ITEM_ICON_SUB,
  PCR_NCG_CONDITION,
  PCR_NCG_HP_BASE,
  //ポケアイコンのNCGは各箇所で持つ
  
  PCR_ANM_CURSOR,
  PCR_ANM_BALL,
  PCR_ANM_BAR_ICON,
  PCR_ANM_ITEM_ICON,
  PCR_ANM_ITEM_ICON_SUB,
  PCR_ANM_CONDITION,
  PCR_ANM_HP_BASE,
  PCR_ANM_POKEICON,
  
  PCR_MAX,
  
  PCR_PLT_START = PCR_PLT_OBJ_COMMON,
  PCR_NCG_START = PCR_NCG_CURSOR,
  PCR_ANM_START = PCR_ANM_CURSOR,
  
  PCR_NONE = 0xFFFFFFFF,  //空
  
}PLIST_CEL_RESOURCE;

//CELL_UNITの表示順
#define PLIST_CELLUNIT_PRI_MAIN (8)
#define PLIST_CELLUNIT_PRI_PLATE (0)

//OBJパレットの開始位置
#define PLIST_OBJPLT_COMMON (0)     //3本
#define PLIST_OBJPLT_ITEM_ICON (3)  //1本
#define PLIST_OBJPLT_CONDITION (4)  //1本
#define PLIST_OBJPLT_HP_BASE   (5)  //1本
#define PLIST_OBJPLT_POKEICON  (6)  //3本
#define PLIST_OBJPLT_BAR_ICON  (9)  //3本

//バーのアイコンの種類
enum PLIST_BARICON_TYPE
{
  PBT_RETURN, //戻る(矢印
  PBT_EXIT,   //抜ける(×マーク
  
  PBT_MAX,
};
//バーのアイコンのアニメ
enum PLIST_BARICON_ANIME
{
  PBA_RETURN_NORMAL,  //戻る(通常
  PBA_EXIT_NORMAL,    //抜ける(通常
};

//メインとなるシーケンス(work->mainSeq
typedef enum
{
  PSMS_FADEIN,
  PSMS_FADEIN_WAIT,
  PSMS_SELECT_POKE, //ポケモン選択中
  PSMS_CHANGE_POKE,
  PSMS_USE_POKE,    //使用対象選択
  PSMS_CHANGE_ANM,
  PSMS_MENU,        //メニュー処理中
  PSMS_MSG_WAIT,    //メッセージ諸々
  PSMS_YESNO_WAIT,    //はい・いいえ待ち
  PSMS_INIT_HPANIME,
  PSMS_HPANIME,     //HPバー処理中
  PSMS_BATTLE_ANM_WAIT, //バトルメニュー時終了アニメ待ち
  PSMS_DISP_PARAM,      //レベルアップ時・パラメータ表示
  PSMS_FADEOUT,
  PSMS_FADEOUT_FORCE, //強制終了
  PSMS_FADEOUT_WAIT,

  PSMS_MAX,
}PLIST_SYS_MAIN_SEQ;

//Proc切り替え用シーケンス(work->changeProcSeq
typedef enum
{
  PSCS_INIT,
  PSCS_MAIN,
  PSCS_TERM,
  
  PSCS_MAX,
}PLIST_SYS_CHANGEPROC_SEQ;

typedef struct _PLIST_WORK PLIST_WORK;
typedef struct _PLIST_PLATE_WORK PLIST_PLATE_WORK;
typedef struct _PLIST_MSG_WORK   PLIST_MSG_WORK;
typedef struct _PLIST_MENU_WORK  PLIST_MENU_WORK;
typedef struct _PLIST_BATTLE_PARAM_WORK PLIST_BATTLE_PARAM_WORK;
typedef struct _PLIST_DEBUG_WORK PLIST_DEBUG_WORK;

typedef void (*PLIST_CallbackFunc)(PLIST_WORK *work);
typedef void (*PLIST_CallbackFuncYesNo)(PLIST_WORK *work,const int retVal);

struct _PLIST_WORK
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;
  
  int ktst;     //KeyToushState
  u8  mainSeq;
  u8  nextMainSeq;
  u8  subSeq;
  u8  selectState;
  BOOL isActiveWindowMask;  //メニュー出たときのWndMaskのON/OFFのVBlank切り替えよう
  BOOL canExit;   //Xボタンが効くか？
  BOOL isSetWazaMode;  //技用のMODE_SELECTか？
  
  PL_SELECT_POS pokeCursor;
  PL_SELECT_POS changeTarget;
  PL_SELECT_POS useTarget;
  POKEMON_PARAM *selectPokePara;
  int  menuRet; //PLIST_MENU_ITEM_TYPE
  
  u16 platePalAnm[16];
  u16 platePalTrans[16];
  u8  platePalAnmCnt;

  //入れ替えアニメ用
  u8  anmCnt;

  //バトルメニュー用
  u8  btlJoinNum;   //バトル参加人数
//  GFL_BMPWIN *btlMenuWin[2];  //バトル用決定・戻る
  APP_TASKMENU_WIN_WORK *btlMenuWin[2]; //バトル用決定・戻る
  u16 btlMenuAnmCnt;       //↑のアニメ用
  u16 btlMenuTransBuf;
  u8  btlTermAnmCnt;

  //HPアニメ処理
  u16 befHp;  //回復前のHP
  PLIST_CallbackFunc hpAnimeCallBack;
  
  //LvUp用bmpWin
  u16          befParam[6]; //レベルアップ前の数値
  GFL_BMPWIN   *paramWin;

  
  //MSG系
  GFL_MSGDATA *msgHandle;
  GFL_FONT *fontHandle;
  GFL_FONT *sysFontHandle;
  PRINT_QUE *printQue;
  BOOL isMsgWaitKey;    //メッセージの完了のキー待ちをするか？
  PLIST_CallbackFunc msgCallBack;  //メッセージ終了時のコールバック
  PLIST_CallbackFuncYesNo yesNoCallBack;  //はい・いいえコールバック
  
  //画面下ウィンドウ管理
  PLIST_MSG_WORK  *msgWork;

  //メニュー管理
  PLIST_MENU_WORK *menuWork;
  
  //プレートScr
  void  *plateScrRes;
  NNSG2dScreenData *plateScrData;

  //プレートデータ
  PLIST_PLATE_WORK  *plateWork[PLIST_LIST_MAX];
  
  //Cell系
  u32 cellRes[PCR_MAX];
  GFL_CLUNIT  *cellUnit;
  GFL_CLWK    *clwkCursor[2];
  GFL_CLWK    *clwkBarIcon[PBT_MAX];
  GFL_CLWK    *clwkExitButton;    //終了時アニメ待ちをするときにポインタを入れておく

  //バトル上画面用
  GFL_CLUNIT  *cellUnitBattle;
  PLIST_BATTLE_PARAM_WORK *battleParty[PL_COMM_PLAYER_TYPE_MAX];
  GFL_BMPWIN   *timeLimitStr;
  GFL_BMPWIN   *timeLimitNumStr;
  BOOL  isUpdateLimitStr;
  BOOL  isUpdateLimitNum;
  u8    befSelectedNum;
  u8    befTimeLimit;
  u16   barPalletAnm[16];
  u16   barPalletTrans[16];
  u16   barPalletAnmCnt;
  BOOL  isCallForceExit;

	//taskmenuリソース
	APP_TASKMENU_RES *taskres;

  PLIST_DATA *plData;
#if USE_DEBUGWIN_SYSTEM
  PLIST_DEBUG_WORK *debWork;
#endif
};

