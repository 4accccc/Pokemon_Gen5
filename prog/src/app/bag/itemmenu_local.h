

//======================================================================
/**
 * @file	  itemmenu.h
 * @brief	  アイテムメニュー
 * @author	k.ohno
 * @date	  2009.06.30
 */
//======================================================================

#pragma once

#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "item/item.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/touch_subwindow.h"

#include "message.naix"
#include "msg/msg_d_field.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "field/fieldmap.h"
#include "font/font.naix" //NARC_font_large_nftr
#include "sound/pm_sndsys.h"
#include "itemmenu.h"
#include "app/itemuse.h"
#include "app/app_taskmenu.h"

extern const GFL_PROC_DATA ItemMenuProcData;


#define ITEM_LIST_NUM (8)
#define _CELLUNIT_NUM (30)

#define _SCROLL_TOP_Y  (8+8)  //スクロールバーの最初の位置
#define _SCROLL_BOTTOM_Y  (8*19)  //スクロールバーの最後の位置

#define _SUBMENU_LISTMAX (8)

enum _ITEMLISTCELL_RESOURCE
{
  _PLT_CUR,
  _PLT_BAR,
  _PLT_COMMON,
  _NCG_CUR,
  _NCG_COMMON,
  _ANM_CUR,
  _ANM_LIST,
  _ANM_COMMON,
  _PTL_CHECK,
  _NCG_CHECK,
  _ANM_CHECK,
  _PLT_BAGPOCKET,  
  _NCG_BAGPOCKET,  
  _ANM_BAGPOCKET,  
  SCR_MAX ,
};


//技アイコンのリソース
enum WAZAICON_CELL
{
  SCR_PLT_SUB_POKE_TYPE,
  SCR_ANM_SUB_POKE_TYPE,
  SCR_NCG_SKILL_TYPE_HENKA,
  SCR_NCG_SKILL_TYPE_BUTURI,
  SCR_NCG_SKILL_TYPE_TOKUSHU,
};


typedef struct _DEBUGITEM_PARAM FIELD_ITEMMENU_WORK;
typedef void (StateFunc)(FIELD_ITEMMENU_WORK* wk);
typedef int (MenuFunc)(FIELD_ITEMMENU_WORK* wk);

struct _DEBUGITEM_PARAM {
  // [IN] 初期化時に外部から受け渡されるメンバ
  FIELDMAP_WORK       * fieldmap;
  GAMESYS_WORK        * gsys;
  SAVE_CONTROL_WORK   * ctrl;
  MYSTATUS            * mystatus;
  ITEMCHECK_WORK      icwk;             ///< アイテムチェックワーク FMENU_EVENT_WORKからコピー
  BAG_MODE            mode;             ///< バッグ呼び出しモード
  BOOL                cycle_flg;        ///< 自転車かどうか？
//  GMEVENT * event;

  // [PRIVATE]
	ITEM_ST ScrollItem[ BAG_MYITEM_MAX ];	///< スクロールする為のアイテムリスト
  StateFunc * state;                    ///< ハンドルのプログラム状態
  MYITEM_PTR  pMyItem;
	GFL_BMPWIN* win;
  GFL_BMPWIN* itemInfoDispWin;
  GFL_BMPWIN* pocketNameWin;
  PRINT_UTIL  SysMsgPrintUtil;      ///< システムウインドウPrintUtil
  PRINT_QUE*  SysMsgQue;
  GFL_MSGDATA* MsgManager;		      ///< 名前入力メッセージデータマネージャー
  GFL_MSGDATA* MsgManagerItemInfo;	///< 名前入力メッセージデータマネージャー
  GFL_MSGDATA* MsgManagerPocket;
  WORDSET*    WordSet;							///< メッセージ展開用ワークマネージャー
  STRBUF*     pStrBuf;
	STRBUF*     pExpStrBuf;
  GFL_FONT*   fontHandle;
  BAG_CURSOR* pBagCursor;
  MenuFunc*   menu_func[BAG_MENUTBL_MAX];
	HEAPID      heapID;

	u32 objRes[3];          ///< CLACTリソース
  u32 cellRes[SCR_MAX];   ///< アイテムカーソル

  u32 commonCellTypeNcg[POKETYPE_MAX];
  u32 commonCell[5];

  u32 listRes[ITEM_LIST_NUM];  //アイテムリスト
  GFL_BMP_DATA* listBmp[ITEM_LIST_NUM];
  GFL_CLWK  *listCell[ITEM_LIST_NUM];     //アイテムの一覧OBJ
  u32 nListEnable[ITEM_LIST_NUM];
  GFL_CLWK  *listMarkCell[ITEM_LIST_NUM];  //チェックマーク
  
	GFL_BUTTON_MAN* pButton;
	GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
  GFL_CLUNIT *cellUnit;
  GFL_CLWK  *cellicon;
  GFL_CLWK  *clwkCur;
  GFL_CLWK  *scrollCur;
  GFL_CLWK  *clwkPocketIcon;
  GFL_CLWK  *clwkWazaKind;
  GFL_CLWK  *clwkWazaType;
  GFL_CLWK  *clwkBarIcon[5];
	GFL_TCBLSYS *pMsgTcblSys;
  PRINT_STREAM* pStream;
  TOUCH_SW_SYS* pTouchSWSys;
  GFL_BMPWIN* winWaza;
  GFL_BMPWIN* winItemName;
  GFL_BMPWIN* winItemNum;
  GFL_BMPWIN* winItemReport;
  GFL_BMPWIN* menuWin[5];
  GFL_BMPWIN* winNumFrame;

  APP_TASKMENU_WORK* pAppTask;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
	APP_TASKMENU_RES* pAppTaskRes;
  
  //メニュー土台
  NNSG2dCharacterData *ncgData;
  void *ncgRes; 

  int mainbg;
  int subbg;
  int subbg2;
  int barbg;
  int numFrameBg;
  int trashNum;
	int pocketno;  //今さしているポケット番号
	int curpos;   //今さしているカーソル番号
  int oamlistpos; //OAMLIST の 先頭位置 -1から開始
  int moveMode;  //移動モードになる
  int menuNum;          //サブメニューの項目数
  int subListCursor;  //サブメニューのカーソル位置
  int submenuList[BAG_MENUTBL_MAX];  //サブメニューの項目
  u32 NowAttr;
  u32 FrontAttr;
	u32 bgchar;
  BOOL bChange;
  int count;

  int ret_code;  //バッグメニューを終わる際の次の動作
  int ret_code2;
  int ret_item;  //選んだアイテム
  
};

#define DEBUG_ITEMDISP_FRAME (GFL_BG_FRAME1_M)


#define _OBJPLT_SUB_POKE_TYPE (4) //サブ画面技タイプアイコンパレット位置


#define _DISP_INITX (18)
#define _DISP_INITY (1)
#define _DISP_SIZEX (12)
#define _DISP_SIZEY (20)
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット
#define _SUBLIST_SELECT_PAL   (10)  //サブメニューの選択パレット
#define _SUBLIST_CANCEL_PAL   (11)  //サブメニューのキャンセルパレット
#define _BUTTON_MSG_PAL   (12)  // メッセージフォント
#define _BUTTON_WIN_PAL   (13)  // ウインドウ

#define _PAL_BAG_PARTS_CELL (10)  // バッグのパーツCELLのパレット展開位置
#define _PAL_MENU_CHECKBOX_CELL (12)  // チェックボックスのパレット展開位置

#define	FBMP_COL_WHITE		(15)
#define WINCLR_COL(col)	(((col)<<4)|(col))


#define _ITEMUSE_DISP_INITX (17)
#define _ITEMUSE_DISP_INITY (10)
#define _ITEMUSE_DISP_SIZEX (8)
#define _ITEMUSE_DISP_SIZEY (10)


#define _POCKETNAME_DISP_INITX (4)
#define _POCKETNAME_DISP_INITY (21)
#define _POCKETNAME_DISP_SIZEX (12)
#define _POCKETNAME_DISP_SIZEY (3)

//数字入力のウインドウの位置
#define _WINNUM_INITX (17)
#define _WINNUM_INITY (13)
#define _WINNUM_SIZEX (11)
#define _WINNUM_SIZEY (2)
#define _WINNUM_PAL (1)

#define _WINNUM_SCR_INITX (16)
#define _WINNUM_SCR_INITY (12)



#define BUTTONID_LEFT    (5)
#define BUTTONID_RIGHT   (6)
#define BUTTONID_EXIT   (7)
#define BUTTONID_RETURN   (8)
#define BUTTONID_ITEM_AREA (9)
#define BUTTONID_CHECK_AREA (15)


#define FLD_SUBSCR_FADE_DIV (1)
#define FLD_SUBSCR_FADE_SYNC (1)

extern void _createSubBg(void);
extern void ITEMDISP_graphicInit(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_upMessageRewrite(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_upMessageDelete(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_upMessageCreate(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_graphicDelete(FIELD_ITEMMENU_WORK* pWork);
extern void _dispMain(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_CellResourceCreate( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_CellCreate( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_CellMessagePrint( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_CellVramTrans( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_scrollCursorMove(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_scrollCursorChangePos(FIELD_ITEMMENU_WORK* pWork, int num);


extern int ITEMMENU_GetItemIndex(FIELD_ITEMMENU_WORK* pWork);
extern int ITEMMENU_GetItemPocketNumber(FIELD_ITEMMENU_WORK* pWork);
extern ITEM_ST* ITEMMENU_GetItem(FIELD_ITEMMENU_WORK* pWork,int no);

extern void ITEMDISP_InitPocketCell( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_ChangePocketCell( FIELD_ITEMMENU_WORK* pWork,int pocketno );
extern void ITEMDISP_MenuWinDisp(  FIELD_ITEMMENU_WORK *work , int *menustr,int num );
extern void ITEMDISP_ListPlateClear( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_ListPlateSelectChange( FIELD_ITEMMENU_WORK* pWork , int selectNo);
extern void ITEMDISP_ItemInfoWindowChange(FIELD_ITEMMENU_WORK *pWork,int pocketno  );
extern void ITEMDISP_WazaInfoWindowChange( FIELD_ITEMMENU_WORK *pWork );

extern void ITEMDISP_ItemInfoWindowDisp( FIELD_ITEMMENU_WORK *pWork );
extern void ITEMDISP_ItemInfoMessageMake( FIELD_ITEMMENU_WORK *pWork,int id );
extern BOOL ITEMDISP_MessageEndCheck(FIELD_ITEMMENU_WORK* pWork);

extern int ITEMMENU_GetPosCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no);
extern BOOL ITEMMENU_AddCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no);
extern void ITEMMENU_RemoveCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no);
extern void ITEMDISP_SetVisible(void);
extern void ITEMDISP_NumFrameDisp(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_TrashNumDisp(FIELD_ITEMMENU_WORK* pWork,int num);
extern void ITEMDISP_YesNoStart(FIELD_ITEMMENU_WORK* pWork);


