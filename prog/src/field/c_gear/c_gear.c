//=============================================================================
/**
 * @file	  c_gear.c
 * @brief	  コミュニケーションギア
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "c_gear.h"
#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"

#include "c_gear.naix"
#include "c_gear_obj_NANR_LBLDEFS.h"

#include "msg/msg_c_gear.h"

#define _NET_DEBUG (1)  //デバッグ時は１
#define _BRIGHTNESS_SYNC (0)  // フェードのＳＹＮＣは要調整

// サウンド仮想ラベル
#define GEAR_SE_DECIDE_ (SEQ_SE_DP_DECIDE)
#define GEAR_SE_CANCEL_ (SEQ_SE_DP_SELECT)


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
} _CGEAR_CLACT_TYPE;



//--------------------------


typedef struct {
	int leftx;
	int lefty;
	int width;
	int height;
} _WINDOWPOS;




static const GFL_UI_TP_HITTBL bttndata[] = {  //上下左右
	//タッチパネル全部
	{	PANEL_Y2 * 8,  PANEL_Y2 * 8 + (PANEL_SIZEXY * 8 * PANEL_HEIGHT2), 0,32*8-1 },
	{GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};


// 表示OAMの時間とかの最大
#define _CLACT_TIMEPARTS_MAX (7)
// タイプ
#define _CLACT_TYPE_MAX (3)

//-------------------------------------------------------------------------
///	文字表示色定義(default)	-> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col)	(((col)<<4)|(col))



//--------------------------------------------
// 内部ワーク
//--------------------------------------------

enum _BATTLETYPE_SELECT {
	_SELECTBT_SINGLE = 0,
	_SELECTBT_DOUBLE,
	_SELECTBT_TRI,
	_SELECTBT_EXIT
};


enum _IBMODE_SELECT {
	_SELECTMODE_BATTLE = 0,
	_SELECTMODE_POKE_CHANGE,
	_SELECTMODE_EXIT
};

enum _IBMODE_ENTRY {
	_ENTRYNUM_DOUBLE = 0,
	_ENTRYNUM_FOUR,
	_ENTRYNUM_EXIT,
};

enum _IBMODE_CHANGE {
	_CHANGE_FRIENDCHANGE = 0,
	_CHANGE_EXIT,
};


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_BMPWIN_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_BUTTON_FRAME   (GFL_BG_FRAME0_S)


typedef void (StateFunc)(C_GEAR_WORK* pState);
typedef BOOL (TouchFunc)(int no, C_GEAR_WORK* pState);


struct _C_GEAR_WORK {
	StateFunc* state;      ///< ハンドルのプログラム状態
	TouchFunc* touch;
	int selectType;   // 接続タイプ
	HEAPID heapID;
	GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
	GFL_BUTTON_MAN* pButton;
	GFL_MSGDATA *pMsgData;  //
	WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
	GFL_FONT* pFontHandle;
	STRBUF* pStrBuf;
	u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
	//    BMPWINFRAME_AREAMANAGER_POS aPos;
	GFL_ARCUTIL_TRANSINFO subchar;
	int windowNum;
	BOOL IsIrc;
	GAMESYS_WORK *gameSys_;
	FIELD_MAIN_WORK *fieldWork_;
	GMEVENT* event_;
	CGEAR_SAVEDATA* pCGSV;
	u32 objRes[3];  //CLACTリソース

  GFL_CLUNIT *cellUnit;
  GFL_CLWK  *cellCursor[_CLACT_TIMEPARTS_MAX];
  GFL_CLWK  *cellType[_CLACT_TYPE_MAX];

};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(C_GEAR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc* state, int line);
static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork);
static void _modeSelectMenuInit(C_GEAR_WORK* pWork);
static void _modeSelectMenuWait(C_GEAR_WORK* pWork);
static void _modeSelectEntryNumInit(C_GEAR_WORK* pWork);
static void _modeSelectEntryNumWait(C_GEAR_WORK* pWork);
static void _modeReportInit(C_GEAR_WORK* pWork);
static void _modeReportWait(C_GEAR_WORK* pWork);
static BOOL _modeSelectMenuButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static BOOL _modeSelectBattleTypeButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static void _modeSelectBattleTypeInit(C_GEAR_WORK* pWork);
static BOOL _modeSelectChangeButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static void _modeSelectChangWait(C_GEAR_WORK* pWork);
static void _modeSelectChangeInit(C_GEAR_WORK* pWork);

static void _timeAnimation(C_GEAR_WORK* pWork);
static void _typeAnimation(C_GEAR_WORK* pWork);


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

static void _changeState(C_GEAR_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc state, int line)
{
	NET_PRINT("ircbtl: %d\n",line);
	_changeState(pWork, state);
}
#endif



//------------------------------------------------------------------------------
/**
 * @brief   ギアｘｙをスクリーンに変換
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearXY2PanelScreen(int x,int y, int* px, int* py)
{
	int ypos[2] = {PANEL_Y1,PANEL_Y2};
	*px = PANEL_X1 + x * PANEL_SIZEXY;
	*py = ypos[ x % 2 ] + y * PANEL_SIZEXY;
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルの数を取得
 * @retval  none
 */
//------------------------------------------------------------------------------

static int _gearPanelTypeNum(C_GEAR_WORK* pWork, CGEAR_PANELTYPE_ENUM type)
{
	int x,y,i;

	i=0;
	for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
	{
		for(x = 0; x < C_GEAR_PANEL_WIDTH; x++)
		{
			if(CGEAR_SV_GetPanelType(pWork->pCGSV,x,y)==type)
			{
				i++;
			}
		}
	}
	return i;
}






//------------------------------------------------------------------------------
/**
 * @brief   パネルタイプをスクリーンに書き込む
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearPanelBgScreenMake(C_GEAR_WORK* pWork,int xs,int ys, CGEAR_PANELTYPE_ENUM type)
{
	int ypos[2] = {PANEL_Y1,PANEL_Y2};
	int x,y,i,j;
	int typepos[] = {0,0x60,0x64,0x68};
	int palpos[] =  {0,0x1000,0x2000,0x3000};
	u16* pScrAddr = GFL_BG_GetScreenBufferAdrs(GEAR_BUTTON_FRAME );
	int xscr;
	int yscr;

	_gearXY2PanelScreen(xs,ys,&xscr,&yscr);
	for(y = yscr, i = 0; i < PANEL_SIZEXY; y++, i++){
		for(x = xscr, j = 0; j < PANEL_SIZEXY; x++, j++){
			if((x >= 0) && (x < 32)){
				int charpos = typepos[type] + i * 0x20 + j;
				int scr = x + (y * 32);
				if(type == CGEAR_PANELTYPE_NONE){
					charpos = 0;
				}
				pScrAddr[scr] = palpos[type] + charpos;
				//		NET_PRINT("x%d y%d  %d \n",x,y,palpos[type] + charpos);
			}
		}
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   ギアセーブデータをみて、文字を出す良い場所を返す
 * @retval  見つからなかったらFALSE
 */
//------------------------------------------------------------------------------

static BOOL _gearGetTypeBestPosition(C_GEAR_WORK* pWork,CGEAR_PANELTYPE_ENUM type, int* px, int* py)
{
	int x,y;

	for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++){
		for(x = 0; x < C_GEAR_PANEL_WIDTH; x++){
			if(CGEAR_SV_GetPanelType(pWork->pCGSV,x,y) == type){
				_gearXY2PanelScreen(x,y,px,py);
				return TRUE;
			}
		}
	}
	return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   ギアのパネルをセーブデータにしたがって作る
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _gearPanelBgCreate(C_GEAR_WORK* pWork)
{
	int x , y;
	int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

	for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
		for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
			_gearPanelBgScreenMake(pWork, x, y, CGEAR_SV_GetPanelType(pWork->pCGSV,x,y));
		}
	}
	GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
}


//------------------------------------------------------------------------------
/**
 * @brief   ギアのARCを読み込み
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearArcCreate(C_GEAR_WORK* pWork)
{


	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, pWork->heapID );

	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_c_gear_c_gear_NCLR,
																		PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_c_gear_c_gear_NCGR,
																																GEAR_MAIN_FRAME, 0, 0, pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_c_gear_c_gear01_NSCR,
																				 GEAR_MAIN_FRAME, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_c_gear_c_gear00_NSCR,
																				 GEAR_BMPWIN_FRAME, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);

	pWork->objRes[_CLACT_PLT] = GFL_CLGRP_PLTT_Register( p_handle ,
																											 NARC_c_gear_c_gear_obj_NCLR ,
																											 CLSYS_DRAW_SUB , 0 , pWork->heapID );

	pWork->objRes[_CLACT_CHR] = GFL_CLGRP_CGR_Register( p_handle ,
																											NARC_c_gear_c_gear_obj_NCGR ,
																											FALSE , CLSYS_DRAW_SUB , pWork->heapID );

	pWork->objRes[_CLACT_ANM] = GFL_CLGRP_CELLANIM_Register( p_handle ,
																													 NARC_c_gear_c_gear_obj_NCER ,
																													 NARC_c_gear_c_gear_obj_NANR ,
																													 pWork->heapID );


	//パレットアニメシステム作成
	//    ConnectBGPalAnm_Init(&pWork->cbp, p_handle, NARC_ircbattle_connect_anm_NCLR, pWork->heapID);
	GFL_ARC_CloseDataHandle( p_handle );


}

//------------------------------------------------------------------------------
/**
 * @brief   サブ画面の設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _createSubBg(C_GEAR_WORK* pWork)
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
			GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 2 );
		//  GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
	{
		int frame = GEAR_BMPWIN_FRAME;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 0 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}

	{
		int frame = GEAR_BUTTON_FRAME;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 1 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}

	//  G2S_SetBlendAlpha( GEAR_MAIN_FRAME, GEAR_BMPWIN_FRAME , 3, 16 );
	//   G2S_SetBlendAlpha( GEAR_MAIN_FRAME, GEAR_BUTTON_FRAME , 16, 16 );
	G2S_SetBlendAlpha( GEAR_MAIN_FRAME|GEAR_BUTTON_FRAME, GEAR_BMPWIN_FRAME , 3, 16 );

}


//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork)
{
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _buttonWindowDelete(C_GEAR_WORK* pWork)
{
	int i;

	GFL_BMN_Delete(pWork->pButton);
	pWork->pButton = NULL;
	for(i=0;i < pWork->windowNum;i++){
		GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
		GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
		GFL_BMPWIN_Delete(pWork->buttonWin[i]);
		pWork->buttonWin[i] = NULL;
	}
	pWork->windowNum = 0;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
	C_GEAR_WORK *pWork = p_work;
	u32 friendNo;
	u32 touchx,touchy;
	int xp,yp;

	switch( event ){
	case GFL_BMN_EVENT_TOUCH:		///< 触れた瞬間
		if(GFL_UI_TP_GetPointCont(&touchx,&touchy)){
			int ypos[2] = {PANEL_Y1,PANEL_Y2};
			int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};
			touchx = touchx / 8;
			touchy = touchy / 8;
			xp = (touchx - PANEL_X1) / PANEL_SIZEXY;
			yp = (touchy - ypos[xp % 2]) / PANEL_SIZEXY;
			if((xp < C_GEAR_PANEL_WIDTH) && (yp < yloop[ xp % 2 ])){
				int type = CGEAR_SV_GetPanelType(pWork->pCGSV,xp,yp);

				if(_gearPanelTypeNum(pWork,type) > 1){
					CGEAR_SV_SetPanelType(pWork->pCGSV,xp,yp,(type+1) % CGEAR_PANELTYPE_MAX);
				}
				_gearPanelBgScreenMake(pWork, xp, yp, CGEAR_SV_GetPanelType(pWork->pCGSV,xp,yp));
				GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );

			}
		}
		break;

	default:
		break;
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   OBJの初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjCreate(C_GEAR_WORK* pWork)
{
	int i;
	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 56+_CLACT_TIMEPARTS_MAX , 0 , pWork->heapID );
  
  for(i=0;i < _CLACT_TIMEPARTS_MAX ;i++)
	{
		int anmbuff[]=
		{
			NANR_c_gear_obj_CellAnime_ampm,
			NANR_c_gear_obj_CellAnime_NO2,NANR_c_gear_obj_CellAnime_NO10a,
			NANR_c_gear_obj_CellAnime_colon,
			NANR_c_gear_obj_CellAnime_NO6,NANR_c_gear_obj_CellAnime_NO10b,
			NANR_c_gear_obj_CellAnime_batt1,
		};
		int xbuff[]=
		{
			32,
			42,
			48,
			52,
			57,
			63,
			218,
		};
		
    GFL_CLWK_DATA cellInitData;
    //セルの生成
    cellInitData.pos_x = xbuff[i];
    cellInitData.pos_y =  18;
    cellInitData.anmseq = anmbuff[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    //↑矢印
    pWork->cellCursor[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                          pWork->objRes[_CLACT_CHR],
                          pWork->objRes[_CLACT_PLT],
                          pWork->objRes[_CLACT_ANM],
                          &cellInitData , 
                          CLSYS_DEFREND_SUB ,
                          pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], TRUE );
  }

  for(i=0;i < _CLACT_TYPE_MAX ;i++)
	{
		int anmbuff[]=
		{
			NANR_c_gear_obj_CellAnime_IR,
			NANR_c_gear_obj_CellAnime_WIRELESS,
			NANR_c_gear_obj_CellAnime_WIFI,
		};
		
    GFL_CLWK_DATA cellInitData;
    //セルの生成

		cellInitData.pos_x = 8;
    cellInitData.pos_y = 8;
    cellInitData.anmseq = anmbuff[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    //↑矢印
    pWork->cellType[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                          pWork->objRes[_CLACT_CHR],
                          pWork->objRes[_CLACT_PLT],
                          pWork->objRes[_CLACT_ANM],
                          &cellInitData , 
                          CLSYS_DEFREND_SUB,
                          pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellType[i], TRUE );
  }
	_timeAnimation(pWork);
	_typeAnimation(pWork);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(C_GEAR_WORK* pWork)
{
	_createSubBg(pWork);   //BGVRAM設定
	_gearArcCreate(pWork);  //ARC読み込み BG&OBJ
	_gearPanelBgCreate(pWork);	// パネル作成

	_gearObjCreate(pWork); //CLACT設定


	pWork->IsIrc=FALSE;

	//  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
	//  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
	//  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_c_gear_dat, pWork->heapID );
	_CHANGE_STATE(pWork,_modeSelectMenuInit);
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuInit(C_GEAR_WORK* pWork)
{
	pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
	pWork->touch = &_modeSelectMenuButtonCallback;

	_CHANGE_STATE(pWork,_modeSelectMenuWait);
}

static void _workEnd(C_GEAR_WORK* pWork)
{
	GFL_FONTSYS_SetDefaultColor();

	if(pWork->pButton){
		GFL_BMN_Delete(pWork->pButton);
	}
	{
		int i;
		for(i=0;i < _CLACT_TIMEPARTS_MAX;i++){
			GFL_CLACT_WK_Remove( pWork->cellCursor[i] );
		}
		for(i=0;i < _CLACT_TYPE_MAX;i++){
			GFL_CLACT_WK_Remove( pWork->cellType[i] );
		}
	}
  GFL_CLACT_UNIT_Delete( pWork->cellUnit );
  GFL_CLGRP_CELLANIM_Release( pWork->objRes[_CLACT_ANM] );
  GFL_CLGRP_CGR_Release( pWork->objRes[_CLACT_CHR] );
  GFL_CLGRP_PLTT_Release( pWork->objRes[_CLACT_PLT] );
	
	//    _buttonWindowDelete(pWork);
	//  GFL_BG_FillCharacterRelease( GEAR_MAIN_FRAME, 1, 0);
	//  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
	//                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));

	GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));

	GFL_BG_FreeBGControl(GEAR_BUTTON_FRAME);
	GFL_BG_FreeBGControl(GEAR_BMPWIN_FRAME);
	GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);

	if(pWork->pMsgData)
	{
		GFL_MSG_Delete( pWork->pMsgData );
	}
	if(pWork->pFontHandle){
		GFL_FONT_Delete(pWork->pFontHandle);
	}
	if(pWork->pStrBuf)
	{
		GFL_STR_DeleteBuffer(pWork->pStrBuf);
	}
	GFL_BG_SetVisible( GEAR_BUTTON_FRAME, VISIBLE_OFF );
	GFL_BG_SetVisible( GEAR_BMPWIN_FRAME, VISIBLE_OFF );
	GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

}


//------------------------------------------------------------------------------
/**
 * @brief   時間アニメーション
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _timeAnimation(C_GEAR_WORK* pWork)
{
	RTCTime time;
	BOOL battflg = FALSE;  //バッテリー表示更新フラグ

	GFL_RTC_GetTime( &time );

	{  //AMPM
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_ampm];
		int num = time.hour / 12;
		if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
			GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
		}
	}
	{  //時10
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO2];
		int num = (time.hour % 12) / 10;
		
		if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
			GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
		}
	}
	{  //時1
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO10a];
		int num = (time.hour % 12) % 10;
		
		if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
			GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
		}
	}

	{  //ころん
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_colon];
		int num = time.second % 2;
		
		if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
			GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
		}
	}

	
	{//秒１０
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO6];
		int num = time.minute / 10;
		
		if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
			GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
			battflg = TRUE;
		}
	}
	{//秒１
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO10b];
		int num = time.minute % 10;
		
		if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
			GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
		}
	}
	if(battflg){//BATT
		GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_batt1];
		PMBattery buf;
		if( PM_GetBattery(&buf) == PM_RESULT_SUCCESS )
		{
			int num = (buf==PM_BATTERY_HIGH ? 1:0);
			if(GFL_CLACT_WK_GetAnmFrame(cp_wk) !=  num){
				GFL_CLACT_WK_SetAnmFrame(cp_wk,num);
			}
		}
	}



	
	
}

//------------------------------------------------------------------------------
/**
 * @brief   タイプのアニメーション 位置調整
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _typeAnimation(C_GEAR_WORK* pWork)
{
	int i;

	for(i=0;i < _CLACT_TYPE_MAX ;i++)
	{
		int x,y;
		GFL_CLACTPOS pos;
		_gearGetTypeBestPosition(pWork, CGEAR_PANELTYPE_IR+i, &x, &y);
		x *= 8;
		y *= 8;
//		GFL_CLACT_WK_GetPos( pWork->cellType[i], &pos , CLSYS_DEFREND_SUB);
//		if((pos.x != x) || (pos.y != y)){
		pos.x = x+32;  // OBJ表示の為の補正値
		pos.y = y+16;
		GFL_CLACT_WK_SetPos(pWork->cellType[i], &pos, CLSYS_DEFREND_SUB);
//		}
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectMenuButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
	switch( bttnid ){
	case _SELECTMODE_BATTLE:
		_CHANGE_STATE(pWork,_modeSelectEntryNumInit);
		_buttonWindowDelete(pWork);
		return TRUE;
	case _SELECTMODE_POKE_CHANGE:
		_CHANGE_STATE(pWork,_modeSelectChangeInit);
		_buttonWindowDelete(pWork);
		return TRUE;
	case _SELECTMODE_EXIT:
		_CHANGE_STATE(pWork,NULL);        // 終わり
		_buttonWindowDelete(pWork);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(C_GEAR_WORK* pWork)
{
	GFL_BMN_Main( pWork->pButton );
	_timeAnimation(pWork);
	_typeAnimation(pWork);
}


//------------------------------------------------------------------------------
/**
 * @brief   交換画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectChangeInit(C_GEAR_WORK* pWork)
{
	int aMsgBuff[]={gear_001, gear_001};

	_buttonWindowCreate(NELEMS(aMsgBuff),aMsgBuff,pWork);

	pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
	pWork->touch = &_modeSelectChangeButtonCallback;

	_CHANGE_STATE(pWork,_modeSelectChangWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   交換画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectChangeButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
	switch(bttnid){
	case _CHANGE_FRIENDCHANGE:
		//    pWork->selectType = EVENTIRCBTL_ENTRYMODE_FRIEND;
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeReportInit);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   交換画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectChangWait(C_GEAR_WORK* pWork)
{
	GFL_BMN_Main( pWork->pButton );

}






//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectEntryNumInit(C_GEAR_WORK* pWork)
{
	int aMsgBuff[]={gear_001,gear_001,gear_001};

	_buttonWindowCreate(3,aMsgBuff,pWork);

	pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
	pWork->touch = &_modeSelectEntryNumButtonCallback;

	_CHANGE_STATE(pWork,_modeSelectEntryNumWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
	switch(bttnid){
	case _ENTRYNUM_DOUBLE:
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeSelectBattleTypeInit);
		return TRUE;
	case _ENTRYNUM_FOUR:
		//    pWork->selectType = EVENTIRCBTL_ENTRYMODE_MULTH;
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeReportInit);
		return TRUE;
	case _ENTRYNUM_EXIT:
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeSelectMenuInit);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectBattleTypeInit(C_GEAR_WORK* pWork)
{
	int aMsgBuff[]={gear_001,gear_001,gear_001,gear_001};

	_buttonWindowCreate(4,aMsgBuff,pWork);

	pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
	pWork->touch = &_modeSelectBattleTypeButtonCallback;

	_CHANGE_STATE(pWork,_modeSelectEntryNumWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectBattleTypeButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
	switch(bttnid){
	case _SELECTBT_SINGLE:
		//    pWork->selectType = EVENTIRCBTL_ENTRYMODE_SINGLE;
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeReportInit);
		break;
	case _SELECTBT_DOUBLE:
		//    pWork->selectType = EVENTIRCBTL_ENTRYMODE_DOUBLE;
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeReportInit);
		break;
	case _SELECTBT_TRI:
		//    pWork->selectType = EVENTIRCBTL_ENTRYMODE_TRI;
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeReportInit);
		break;
	default:
		_buttonWindowDelete(pWork);
		_CHANGE_STATE(pWork,_modeSelectMenuInit);
		break;
	}
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectEntryNumWait(C_GEAR_WORK* pWork)
{
	GFL_BMN_Main( pWork->pButton );
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportInit(C_GEAR_WORK* pWork)
{

	//    GAMEDATA_Save(GAMESYSTEM_GetGameData(GMEVENT_GetGameSysWork(event)));

	_CHANGE_STATE(pWork,_modeReportWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait(C_GEAR_WORK* pWork)
{
	pWork->IsIrc = TRUE;  //赤外線接続開始
	_CHANGE_STATE(pWork,NULL);
}


//------------------------------------------------------------------------------
/**
 * @brief   スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
C_GEAR_WORK* CGEAR_Init( CGEAR_SAVEDATA* pCGSV )
{
	C_GEAR_WORK *pWork = NULL;

	//GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_CGEAR, 0x8000 );

	pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELDMAP, sizeof( C_GEAR_WORK ) );
	pWork->heapID = HEAPID_FIELDMAP;
	pWork->pCGSV = pCGSV;

//	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, _BRIGHTNESS_SYNC);
	_modeInit(pWork);
	
//	_CHANGE_STATE( pWork, _modeInit);
	return pWork;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_Main( C_GEAR_WORK* pWork )
{
	StateFunc* state = pWork->state;
	if(state != NULL){
		state(pWork);
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_Exit( C_GEAR_WORK* pWork )
{

	//  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, _BRIGHTNESS_SYNC);
	_workEnd(pWork);

	GFL_HEAP_FreeMemory(pWork);
	//  GFL_HEAP_DeleteHeap(HEAPID_CGEAR);

}


