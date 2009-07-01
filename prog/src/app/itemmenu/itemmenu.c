//============================================================================================
/**
 * @file	  itemmenu.h
 * @brief	  アイテムメニュー
 * @author	k.ohno
 * @date	  2009.06.30
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "item/item.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"

#include "message.naix"
#include "msg/msg_d_field.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "field/fieldmap.h"
#include "font/font.naix" //NARC_font_large_nftr
#include "sound/pm_sndsys.h"
#include "system/wipe.h"

#include "itemmenu.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "system/main.h"			//GFL_HEAPID_APP参照

//============================================================================================
//============================================================================================

#define DEBUG_ITEMDISP_FRAME (GFL_BG_FRAME1_S)


#define _DISP_INITX (1)
#define _DISP_INITY (1)
#define _DISP_SIZEX (30)
#define _DISP_SIZEY (20)
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント
#define	FBMP_COL_WHITE		(15)
#define WINCLR_COL(col)	(((col)<<4)|(col))

#define FLD_SUBSCR_FADE_DIV (1)
#define FLD_SUBSCR_FADE_SYNC (1)

//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(FIELD_ITEMMENU_WORK* pWork,StateFunc* state);
static void _changeStateDebug(FIELD_ITEMMENU_WORK* pWork,StateFunc* state, int line);
static void _itemNumSelectMenu(FIELD_ITEMMENU_WORK* wk);


#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(FIELD_ITEMMENU_WORK* wk,StateFunc state)
{
  wk->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(FIELD_ITEMMENU_WORK* wk,StateFunc state, int line)
{
  NET_PRINT("EvDebugItem: %d\n",line);
  _changeState(wk, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   絵の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createSubBg(void)
{

  // 背景面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
    GFL_BG_SetPriority( GFL_BG_FRAME0_M, 0 );
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

  }

  {
    int frame = GFL_BG_FRAME0_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME1_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME3_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}


static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_32K,
};


static void _graphicInit(FIELD_ITEMMENU_WORK* wk)
{
	G2_BlendNone();

	GFL_BG_Init(wk->heapID);
	GFL_BMPWIN_Init(wk->heapID);
	GFL_FONTSYS_Init();
	GFL_DISP_SetBank( &_defVBTbl );

	{
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &BGsys_data );
	}
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	_createSubBg();

	
	GFL_FONTSYS_SetDefaultColor();

	
	wk->bgchar = BmpWinFrame_GraphicSetAreaMan(DEBUG_ITEMDISP_FRAME,
																						 _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, wk->heapID);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, wk->heapID);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, wk->heapID);
}


static void _windowCreate(FIELD_ITEMMENU_WORK* wk)
{

	wk->win = GFL_BMPWIN_Create(
		DEBUG_ITEMDISP_FRAME,
    _DISP_INITX, _DISP_INITY,
    _DISP_SIZEX, _DISP_SIZEY,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win), WINCLR_COL(FBMP_COL_WHITE) );
  GFL_BMPWIN_MakeScreen( wk->win );
	BmpWinFrame_Write( wk->win, WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar), _BUTTON_WIN_PAL );

}



static void _windowRewrite(FIELD_ITEMMENU_WORK* wk, int type)
{
	int i;
	ITEM_ST * item;
	MYITEM_PTR pMyItem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(wk->gsys));
	
	GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win), WINCLR_COL(FBMP_COL_WHITE) );

	for(i = 0 ; i < 9; i++){
		item = MYITEM_PosItemGet( pMyItem, wk->itemtype, i );
		if(item==NULL){
			break;
		}
		ITEM_GetItemName(wk->pStrBuf, item->id, wk->heapID);
		PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win), 10, (i+1)*16, wk->pStrBuf, wk->fontHandle);		

		GFL_MSG_GetString(  wk->MsgManager, DEBUG_FIELD_STR34, wk->pStrBuf );
		WORDSET_RegisterNumber(wk->WordSet, 2, item->no,
													 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
		WORDSET_ExpandStr( wk->WordSet, wk->pExpStrBuf, wk->pStrBuf );
		PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win), 160, (i+1)*16, wk->pExpStrBuf, wk->fontHandle);

	}

	GFL_BMPWIN_TransVramCharacter(wk->win);
}







static void _itemKindSelectMenu(FIELD_ITEMMENU_WORK* wk)
{















	
	if(GFL_UI_KEY_GetTrg()== PAD_BUTTON_START){
		_CHANGE_STATE(wk, NULL);
	}

	
}



//------------------------------------------------------------------------------
/**
 * @brief   プロセス初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  FIELD_ITEMMENU_WORK* wk = pwk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ITEMMENU, 0x28000 );
	wk->heapID = HEAPID_ITEMMENU;

	_graphicInit(wk);


	
  wk->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
																	 NARC_message_d_field_dat, wk->heapID );
	wk->pStrBuf = GFL_STR_CreateBuffer(100,wk->heapID);
	wk->pExpStrBuf = GFL_STR_CreateBuffer(100,wk->heapID);
  wk->WordSet    = WORDSET_Create( wk->heapID );
  wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr ,
																		GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapID );

	GFL_UI_KEY_SetRepeatSpeed(1,6);
	_windowCreate(wk);
	_windowRewrite(wk,0);
	
	WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
									WIPE_FADE_BLACK , FLD_SUBSCR_FADE_DIV , FLD_SUBSCR_FADE_SYNC , wk->heapID );
	
	_CHANGE_STATE(wk, _itemKindSelectMenu);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   プロセスMain
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  FIELD_ITEMMENU_WORK* wk = pwk;
  StateFunc* state = wk->state;

	if(state == NULL){
		return GFL_PROC_RES_FINISH;
	}
	if( WIPE_SYS_EndCheck() != TRUE ){
		return GFL_PROC_RES_CONTINUE;
	}
	state(wk);
	return GFL_PROC_RES_CONTINUE;
	
}

//------------------------------------------------------------------------------
/**
 * @brief   プロセスEnd
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  FIELD_ITEMMENU_WORK* wk = pwk;

	GFL_BG_FreeCharacterArea(DEBUG_ITEMDISP_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(wk->bgchar));
	GFL_BMPWIN_ClearScreen(wk->win);
	BmpWinFrame_Clear(wk->win, WINDOW_TRANS_OFF);
	GFL_BG_LoadScreenV_Req(DEBUG_ITEMDISP_FRAME);
	GFL_BMPWIN_Delete(wk->win);
	GFL_MSG_Delete(wk->MsgManager);
	GFL_STR_DeleteBuffer(wk->pStrBuf);
	GFL_STR_DeleteBuffer(wk->pExpStrBuf);
  WORDSET_Delete(wk->WordSet);
  GFL_FONT_Delete(wk->fontHandle);

	GFL_BMPWIN_Exit();
	GFL_BG_Exit();


	GFL_HEAP_DeleteHeap(  HEAPID_ITEMMENU );
	
	return GFL_PROC_RES_FINISH;

}



// プロセス定義データ
const GFL_PROC_DATA ItemMenuProcData = {
  FieldItemMenuProc_Init,
  FieldItemMenuProc_Main,
  FieldItemMenuProc_End,
};
