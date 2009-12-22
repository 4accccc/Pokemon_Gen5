//=============================================================================
/**
 * @file	  no_gear.c
 * @brief	  コミュニケーションギアが無い状態
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/09
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "no_gear.h"
#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"

#include "c_gear.naix"
#include "c_gear_obj_NANR_LBLDEFS.h"

#include "msg/msg_c_gear.h"
#include "msg/msg_invasion.h"

#include "field/field_beacon_message.h"

#define _NET_DEBUG (1)  //デバッグ時は１

// サウンド仮想ラベル
#define GEAR_SE_DECIDE_ (SEQ_SE_DP_DECIDE)
#define GEAR_SE_CANCEL_ (SEQ_SE_DP_SELECT)

#define MSG_COUNTDOWN_FRAMENUM (30*3)

//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (13)  // メッセージフォント

#define	_BUTTON_WIN_CGX_SIZE   ( 18+12 )
#define	_BUTTON_WIN_CGX	( 2 )


#define	_BUTTON_FRAME_CGX		( _BUTTON_WIN_CGX + ( 23 * 16 ) )	// 通信システムウィンドウ転送先

#define	FBMP_COL_WHITE		(15)


//-------------------------

#define PANEL_Y1 (6)
#define PANEL_Y2 (4)
#define PANEL_X1 (-2)

#define PANEL_HEIDHT1 (C_GEAR_PANEL_HEIGHT-1)
#define PANEL_HEIGHT2 (C_GEAR_PANEL_HEIGHT)

#define PANEL_WIDTH (C_GEAR_PANEL_WIDTH)
#define PANEL_SIZEXY (4)   //

typedef enum{
	_CLACT_PLT,
	_CLACT_CHR,
	_CLACT_ANM,
} _NOGEAR_CLACT_TYPE;




#define _NOGEAR_NET_CHANGEPAL_NUM (4)
#define _NOGEAR_NET_CHANGEPAL_POSX (0xC)
#define _NOGEAR_NET_CHANGEPAL_POSY (1)
#define _NOGEAR_NET_CHANGEPAL_MAX (3)


//--------------------------

typedef enum{
	_SELECTANIM_NONE,
	_SELECTANIM_STANDBY,
	_SELECTANIM_ANIMING,
	
} _SELECTANIM_ENUM;



//--------------------------

typedef struct {
	int leftx;
	int lefty;
	int width;
	int height;
} _WINDOWPOS;






// 表示OAMの時間とかの最大
#define _CLACT_TIMEPARTS_MAX (9)

#define _CLACT_EDITMARKOFF (7)
#define _CLACT_EDITMARKON (8)


// タイプ
#define _CLACT_TYPE_MAX (3)

//-------------------------------------------------------------------------
///	文字表示色定義(default)	-> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col)	(((col)<<4)|(col))



//--------------------------------------------
// 内部ワーク
//--------------------------------------------


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_BMPWIN_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_BUTTON_FRAME   (GFL_BG_FRAME0_S)
#define GEAR_FB_MESSAGE   (GFL_BG_FRAME3_S)


typedef void (StateFunc)(NO_GEAR_WORK* pState);
typedef BOOL (TouchFunc)(int no, NO_GEAR_WORK* pState);


struct _NO_GEAR_WORK {
	StateFunc* state;      ///< ハンドルのプログラム状態
	int selectType;   // 接続タイプ
	HEAPID heapID;
	GFL_BUTTON_MAN* pButton;
	u32 bgchar;  //GFL_ARCUTIL_TRANSINFO

	GFL_ARCUTIL_TRANSINFO subchar;
//	FIELD_SUBSCREEN_WORK* subscreen;
	GAMESYS_WORK* pGameSys;

	u16 tpx;
	u16 tpy;

	
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(NO_GEAR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(NO_GEAR_WORK* pWork,StateFunc* state, int line);
static void _modeSelectMenuWait(NO_GEAR_WORK* pWork);


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

static void _changeState(NO_GEAR_WORK* pWork,StateFunc state)
{
	pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(NO_GEAR_WORK* pWork,StateFunc state, int line)
{
	NET_PRINT("nogear: %d\n",line);
	_changeState(pWork, state);
}
#endif



//------------------------------------------------------------------------------
/**
 * @brief   ギアのARCを読み込み
 * @retval  none
 */
//------------------------------------------------------------------------------

static u32 _bgpal[]={NARC_c_gear_c_gear_NCLR,NARC_c_gear_c_gear2_NCLR,NARC_c_gear_c_gear_NCLR};
static u32 _bgcgx[]={NARC_c_gear_c_gear_NCGR,NARC_c_gear_c_gear2_NCGR,NARC_c_gear_c_gear_NCGR};


static void _gearArcCreate(NO_GEAR_WORK* pWork)
{
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, pWork->heapID );
	MYSTATUS* pMy = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(pWork->pGameSys) );
	u32 sex = MyStatus_GetMySex(pMy);
	
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, _bgpal[ sex ],
																		PALTYPE_SUB_BG, 0, 0,  pWork->heapID);



	
	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, _bgcgx[sex],
																																GEAR_MAIN_FRAME, 0, 0, pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_c_gear_c_gear01_n_NSCR,
																				 GEAR_MAIN_FRAME, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);

	GFL_ARC_CloseDataHandle( p_handle );

//	GFL_NET_ChangeIconPosition(240-22,10);
	GFL_NET_ReloadIcon();

}

//------------------------------------------------------------------------------
/**
 * @brief   サブ画面の設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _createSubBg(NO_GEAR_WORK* pWork)
{
	int i = 0;
	for(i = GFL_BG_FRAME0_S;i <= GFL_BG_FRAME3_S ; i++)
	{
		GFL_BG_SetVisible( i, VISIBLE_OFF );
	}
	{
		int frame = GEAR_MAIN_FRAME;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 3 );
		//  GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
	{
		int frame = GEAR_BMPWIN_FRAME;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 1 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}

	{
		int frame = GEAR_BUTTON_FRAME;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 2 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}

//	G2S_SetBlendAlpha( GEAR_MAIN_FRAME|GEAR_BUTTON_FRAME, GEAR_BMPWIN_FRAME , 9, 15 );

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeInit(NO_GEAR_WORK* pWork)
{
	_createSubBg(pWork);   //BGVRAM設定
	_gearArcCreate(pWork);  //ARC読み込み BG&OBJ

  _CHANGE_STATE(pWork,_modeSelectMenuWait);

}

static void _workEnd(NO_GEAR_WORK* pWork)
{


	GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));

	GFL_BG_FreeBGControl(GEAR_BUTTON_FRAME);
	GFL_BG_FreeBGControl(GEAR_BMPWIN_FRAME);
	GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);

	GFL_BG_SetVisible( GEAR_BUTTON_FRAME, VISIBLE_OFF );
	GFL_BG_SetVisible( GEAR_BMPWIN_FRAME, VISIBLE_OFF );
	GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

}


//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(NO_GEAR_WORK* pWork)
{


}





//------------------------------------------------------------------------------
/**
 * @brief   スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
NO_GEAR_WORK* NOGEAR_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys )
{
	NO_GEAR_WORK *pWork = NULL;

	pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELDMAP, sizeof( NO_GEAR_WORK ) );
	pWork->heapID = HEAPID_FIELDMAP;
//	pWork->pCGSV = pCGSV;
//	pWork->subscreen = pSub;
	pWork->pGameSys = pGameSys;

	_modeInit(pWork);



	return pWork;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
void NOGEAR_Main( NO_GEAR_WORK* pWork,BOOL bAction )
{
	StateFunc* state = pWork->state;
	
	if(state != NULL)
	{
		state(pWork);
	}
	
	
}


//------------------------------------------------------------------------------
/**
 * @brief   NOGEAR_ActionCallback
 * @retval  none
 */
//------------------------------------------------------------------------------
void NOGEAR_ActionCallback( NO_GEAR_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno)
{
}


//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
void NOGEAR_Exit( NO_GEAR_WORK* pWork )
{
//	GFL_NET_ChangeIconPosition(GFL_WICON_POSX,GFL_WICON_POSY);
//	GFL_NET_ReloadIcon();

  _workEnd(pWork);
	G2S_BlendNone();
	GFL_HEAP_FreeMemory(pWork);

}




