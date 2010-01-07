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

// SE再定義
enum
{ 
  SE_BAG_DECIDE       = SEQ_SE_DECIDE1, ///< 決定音
  SE_BAG_CANCEL       = SEQ_SE_CANCEL1, ///< キャンセル音
  SE_BAG_REGIST_Y     = SEQ_SE_SYS_07,  ///< バッグYボタン登録音
  SE_BAG_SELL         = SEQ_SE_SYS_22,  ///< 売却音
  SE_BAG_TRASH        = SEQ_SE_SYS_08,  ///< 捨てる音
  SE_BAG_CURSOR_MOVE  = SEQ_SE_SELECT1, ///< バッグカーソル移動(上下キー)
  SE_BAG_POCKET_MOVE  = SEQ_SE_SELECT4, ///< バッグポケット選択(左右キー)
  SE_BAG_SRIDE        = SEQ_SE_SELECT1, ///< スライドバー
  SE_BAG_WAZA         = SEQ_SE_PC_LOGIN,  ///< ワザマシン起動音
  SE_BAG_SORT         = SEQ_SE_SYS_36,    ///< ソート音
};


#define ITEM_LIST_NUM (8)
#define _CELLUNIT_NUM (30)

#define _SCROLL_TOP_Y  (8+8)  //スクロールバーの最初の位置
#define _SCROLL_BOTTOM_Y  (8*19)  //スクロールバーの最後の位置

#define _SUBMENU_LISTMAX (8)

// 数値入力
enum
{ 
  BAG_UI_TP_CUR_REPEAT_SYNC = 30,          ///< このフレームタッチし続けるとそれ以降毎フレームタッチ処理
  BAG_UI_TP_CUR_HIGHSPEED_SYNC = 60 * 2,   ///< タッチしている時間がこのフレームを超えると移動量UP
};

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
  _PLT_SORT,
  _NCG_SORT,
  _PLT_LIST,
  // _NCG_LIST >> listResでリストの項目分用意されている
  _ANM_SORT,
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

// バーアイコンのCLWK配列インデックス
enum
{ 
  BAR_ICON_LEFT = 0,
  BAR_ICON_RIGHT,
  BAR_ICON_CHECK_BOX,
  BAR_ICON_EXIT,
  BAR_ICON_RETURN,
  BAR_ICON_INPUT_U, ///< 数値入力上
  BAR_ICON_INPUT_D, ///< 数値入力下
  BAR_ICON_MAX,

  BAR_ICON_ANM_EXIT     = 0,
  BAR_ICON_ANM_RETURN   = 1,
  BAR_ICON_ANM_LEFT     = 4,
  BAR_ICON_ANM_RIGHT    = 5,
  BAR_ICON_ANM_CHECKBOX = 6,
  BAR_ICON_ANM_EXIT_OFF   = 14,
  BAR_ICON_ANM_LEFT_OFF   = 18,
  BAR_ICON_ANM_RIGHT_OFF  = 19,
  BAR_ICON_ANM_CHECKBOX_OFF  = 20,
  BAR_ICON_ANM_MAX,
};

//--------------------------------------------------------------
///	数値入力モード
//==============================================================
typedef enum
{ 
  BAG_INPUT_MODE_NULL = 0,
  BAG_INPUT_MODE_TRASH,   ///< すてるモード
  BAG_INPUT_MODE_SELL,    ///< うるモード（金額表示あり）
} BAG_INPUT_MODE;


typedef struct _FIELD_ITEMMENU_PARAM FIELD_ITEMMENU_WORK;
typedef void (StateFunc)(FIELD_ITEMMENU_WORK* wk);
typedef int (MenuFunc)(FIELD_ITEMMENU_WORK* wk);

struct _FIELD_ITEMMENU_PARAM {
  // [IN] 初期化時に外部から受け渡されるメンバ
  GAMEDATA            * gamedata;       ///< Yボタン登録に使用
  CONFIG              * config;              
  MYSTATUS            * mystatus;
  ITEMCHECK_WORK      * icwk;           ///< アイテムチェックワーク FMENU_EVENT_WORKからコピー
  BAG_MODE            mode;             ///< バッグ呼び出しモード
  BAG_CURSOR*         pBagCursor;
  MYITEM_PTR          pMyItem;
  BOOL                cycle_flg;        ///< 自転車かどうか？
//  GMEVENT             * event;
//  FIELDMAP_WORK       * fieldmap;

  // [PRIVATE]
	ITEM_ST ScrollItem[ BAG_MYITEM_MAX ];	///< スクロールする為のアイテムリスト
  StateFunc * state;                    ///< ハンドルのプログラム状態
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
  GFL_CLWK  *clwkBarIcon[ BAR_ICON_MAX ];
  GFL_CLWK  *clwkSort;
	GFL_TCBLSYS *pMsgTcblSys;
  PRINT_STREAM* pStream;
  TOUCH_SW_SYS* pTouchSWSys;
  GFL_BMPWIN* winWaza;
  GFL_BMPWIN* winItemName;
  GFL_BMPWIN* winItemNum;
  GFL_BMPWIN* winItemReport;
  GFL_BMPWIN* menuWin[5];
  GFL_BMPWIN* winNumFrame;
  GFL_BMPWIN* winGoldCap;
  GFL_BMPWIN* winGold;
  GFL_BMPWIN* winSellGold;

  APP_TASKMENU_WORK* pAppTask;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
	APP_TASKMENU_RES* pAppTaskRes;
  
  //メニュー土台
  NNSG2dCharacterData *ncgData;
  void *ncgRes; 

  BAG_INPUT_MODE InputMode;
  int countTouch; ///< タッチカウンタ
  int InputNum;
  int mainbg;
  int subbg;
  int subbg2;
  int barbg;
  int numFrameBg;
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
  BOOL bChange;   ///< CELL更新フラグ

  enum BAG_NEXTPROC_ENUM ret_code;  //バッグメニューを終わる際の次の動作
  int ret_code2;
  int ret_item;       ///< 選んだアイテム
  
  BOOL sort_mode;     ///< ソートモード
};

#define _OBJPLT_SUB_POKE_TYPE (4) //サブ画面技タイプアイコンパレット位置


#define _DISP_INITX (18)
#define _DISP_INITY (1)
#define _DISP_SIZEX (12)
#define _DISP_SIZEY (20)
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット
#define _SUBLIST_SELECT_PAL   (10)  //サブメニューの選択パレット
#define _SUBLIST_CANCEL_PAL   (11)  //サブメニューのキャンセルパレット
#define _BUTTON_MSG_PAL   (12)  // メッセージフォント
#define _SUBBUTTON_MSG_PAL   (0)  // メッセージフォント
#define _BUTTON_WIN_PAL   (13)  // ウインドウ

#define _PAL_WIN01_CELL (0)     // リストウィンドウ用パレット転送位置
#define _PAL_CUR_CELL (1)       // スライドバーつまみ カーソル用パレット転送位置
#define _PAL_CUR_CELL_NUM (2)
#define _PAL_MENU_CHECKBOX_CELL (12)  // チェックボックスのパレット展開位置
#define _PAL_SORT_CELL (3)      // ソートボタンのパレット展開位置
#define _PAL_SORT_CELL_NUM (2)  // ソートボタンのパレット本数
#define _PAL_BAG_PARTS_CELL (10)      // バッグのパーツCELLのパレット展開位置
#define _PAL_COMMON_CELL (12)          // 共通パレット展開位置
#define _PAL_COMMON_CELL_NUM ( APP_COMMON_BARICON_PLT_NUM )

// ポケット名の文字色
#define _POCKETNAME_FONT_PAL_L (0xF)
#define _POCKETNAME_FONT_PAL_S (0x2)
#define _POCKETNAME_FONT_PAL_B (0x0)


#define	FBMP_COL_WHITE		(15)
#define WINCLR_COL(col)	(((col)<<4)|(col))


#define _ITEMUSE_DISP_INITX (17)
#define _ITEMUSE_DISP_INITY (10)
#define _ITEMUSE_DISP_SIZEX (8)
#define _ITEMUSE_DISP_SIZEY (10)

// 「おこづかい」
#define _GOLD_CAP_DISP_INITX (1)
#define _GOLD_CAP_DISP_INITY (21)
#define _GOLD_CAP_DISP_SIZEX (8)
#define _GOLD_CAP_DISP_SIZEY (3)

// おこづかい XXXXXX円
#define _GOLD_DISP_INITX (9)
#define _GOLD_DISP_INITY (21)
#define _GOLD_DISP_SIZEX (9)
#define _GOLD_DISP_SIZEY (3)

// 数値入力ウィンドウ 売却金額
#define _SELL_GOLD_DISP_INITX (17)
#define _SELL_GOLD_DISP_INITY (15)
#define _SELL_GOLD_DISP_SIZEX (11)
#define _SELL_GOLD_DISP_SIZEY (2)

#define _POCKETNAME_DISP_INITX (3)
#define _POCKETNAME_DISP_INITY (21)
#define _POCKETNAME_DISP_SIZEX (12)
#define _POCKETNAME_DISP_SIZEY (3)

//数字入力のウインドウの位置
#define _WINNUM_INITX (17)
#define _WINNUM_INITY (13)
#define _WINNUM_SIZEX (11)
#define _WINNUM_SIZEY (2)
#define _WINNUM_PAL (3)

#define _WINNUM_SCR_INITX (16)
#define _WINNUM_SCR_INITY (12)

#define _BAR_CELL_CURSOR_EXIT (200-8)    //EXIT xボタン
#define _BAR_CELL_CURSOR_RETURN (232-8) //RETURN Enterボタン

enum
{ 
  BUTTONID_ITEM_AREA_NUM = 6,   ///< アイテムエリアの項目数
  BUTTONID_CHECK_AREA_NUM = 6,  ///< チェックエリアの項目数
  // 0-4 : バッグのポケット
  BUTTONID_LEFT = 5,
  BUTTONID_RIGHT,
  BUTTONID_SORT,
  BUTTONID_CHECKBOX,
  BUTTONID_EXIT,
  BUTTONID_RETURN,
  BUTTONID_ITEM_AREA,
  BUTTONID_CHECK_AREA = BUTTONID_ITEM_AREA + BUTTONID_ITEM_AREA_NUM,
  BUTTONID_MAX = BUTTONID_CHECK_AREA + BUTTONID_CHECK_AREA_NUM,
};

#define FLD_SUBSCR_FADE_DIV (1)
#define FLD_SUBSCR_FADE_SYNC (1)


extern void _createSubBg(void);
extern void ITEMDISP_graphicInit(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_upMessageRewrite(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_upMessageDelete(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_upMessageCreate(FIELD_ITEMMENU_WORK* pWork);
//extern void ITEMDISP_RemoveSubDispItemIcon(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_graphicDelete(FIELD_ITEMMENU_WORK* pWork);
extern void _dispMain(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_CellResourceCreate( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_CellCreate( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_CellMessagePrint( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_CellVramTrans( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_scrollCursorMove(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_scrollCursorChangePos(FIELD_ITEMMENU_WORK* pWork, int num);

extern s32 ITEMMENU_SellPrice( int item_no, int input_num, HEAPID heapID );
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
extern void ITEMDISP_BarMessageCreate( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_BarMessageDelete( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_PocketMessage(FIELD_ITEMMENU_WORK* pWork,int newpocket);
extern void ITEMDISP_GoldDispWrite( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_GoldDispIn( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_GoldDispOut( FIELD_ITEMMENU_WORK* pWork );
extern void ITEMDISP_YesNoStart(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_YesNoExit(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_NumFrameDisp(FIELD_ITEMMENU_WORK* pWork);
extern void ITEMDISP_InputNumDisp(FIELD_ITEMMENU_WORK* pWork,int num);


