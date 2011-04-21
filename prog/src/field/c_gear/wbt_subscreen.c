//=============================================================================
/**
 * @file	  wbt_subscreen.c
 * @brief	  WBT専用下画面
 * @author	tamada@gamefreak.co.jp
 * @date	  2011.04.20
 *
 * src/field/c_gear/no_gear.cからコピペで作成開始
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"

#include "system/main.h"  //HEAPID

#include "wbt_subscreen.h"

#include "net/wih_dwc.h"  //WIH_DWC_GetAllBeaconTypeBit

#include "arc/wbt/wbt_subscreen.naix"

//#define _NET_DEBUG (1)  //デバッグ時は１

// サウンド仮想ラベル
//#define GEAR_SE_DECIDE_ (SEQ_SE_DP_DECIDE)
//#define GEAR_SE_CANCEL_ (SEQ_SE_DP_SELECT)


//--------------------------------------------
// 画面構成定義
//--------------------------------------------
//--------------------------------------------
// 内部ワーク
//--------------------------------------------


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_BMPWIN_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_BUTTON_FRAME   (GFL_BG_FRAME0_S)
#define GEAR_FB_MESSAGE   (GFL_BG_FRAME3_S)


typedef void (StateFunc)(WBTSUBSCRN_WORK* pState);
typedef BOOL (TouchFunc)(int no, WBTSUBSCRN_WORK* pState);


struct _WBTSUBSCRN_WORK {
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
static void _changeState(WBTSUBSCRN_WORK* pWork,StateFunc* state);
static void _changeStateDebug(WBTSUBSCRN_WORK* pWork,StateFunc* state, int line);
static void _modeSelectMenuWait(WBTSUBSCRN_WORK* pWork);


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
static void _changeState(WBTSUBSCRN_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(WBTSUBSCRN_WORK* pWork,StateFunc state, int line)
{
	NET_PRINT("wbtsubscreen: %d\n",line);
	_changeState(pWork, state);
}
#endif



//------------------------------------------------------------------------------
/**
 * @brief   ARCを読み込み
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _loadArc(WBTSUBSCRN_WORK* pWork)
{
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_WBT_SUBSCREEN, pWork->heapID );
	MYSTATUS* pMy = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(pWork->pGameSys) );
	u32 sex = MyStatus_GetMySex(pMy);
	
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wbt_subscreen_wbt_d_NCLR,
																		PALTYPE_SUB_BG, 0, 0,  pWork->heapID);

	
	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wbt_subscreen_wbt_d_NCGR,
																																GEAR_MAIN_FRAME, 0, 0, pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                         NARC_wbt_subscreen_wbt_d_NSCR,
																				 GEAR_MAIN_FRAME, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);


	GFL_ARC_CloseDataHandle( p_handle );

  GFL_NET_ReloadIconTopOrBottom( FALSE, pWork->heapID );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON ); // SUB DISP OBJ ON
}

//------------------------------------------------------------------------------
/**
 * @brief   サブ画面の設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _createSubBg(WBTSUBSCRN_WORK* pWork)
{
	int i = 0;
	for(i = GFL_BG_FRAME0_S;i <= GFL_BG_FRAME3_S ; i++)
	{
		GFL_BG_SetVisible( i, VISIBLE_OFF );
	}
	{
		const int frame = GEAR_MAIN_FRAME;
		const GFL_BG_BGCNT_HEADER TextBgCntDat = {
    //X, Y, ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ, ｽｸﾘｰﾝｻｲｽﾞ、            ｶﾗｰﾓｰﾄﾞ
			0, 0, 0x800,      0,          GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      //ｽｸﾘｰﾝﾍﾞｰｽ、         ｷｬﾗﾍﾞｰｽ、               ｷｬﾗｻｲｽﾞ
			GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
      //拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、 表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
      GX_BG_EXTPLTT_01, 0,          0,        0,    FALSE
			};

		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 3 );
		//  GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
#if 0
	{
		int frame = GEAR_BMPWIN_FRAME;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,GX_BG_EXTPLTT_01,
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
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		//GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_SetPriority( frame, 2 );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
#endif

//	G2S_SetBlendAlpha( GEAR_MAIN_FRAME|GEAR_BUTTON_FRAME, GEAR_BMPWIN_FRAME , 9, 15 );

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(WBTSUBSCRN_WORK* pWork)
{
	_createSubBg(pWork);   //BGVRAM設定
	_loadArc(pWork);  //ARC読み込み BG

  _CHANGE_STATE(pWork,_modeSelectMenuWait);

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void _workEnd(WBTSUBSCRN_WORK* pWork)
{
	GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));

	GFL_BG_FreeBGControl(GEAR_BUTTON_FRAME);
	GFL_BG_FreeBGControl(GEAR_BMPWIN_FRAME);
	GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);

	GFL_BG_SetVisible( GEAR_BUTTON_FRAME, VISIBLE_OFF );
	GFL_BG_SetVisible( GEAR_BMPWIN_FRAME, VISIBLE_OFF );
	GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF ); //SUB DISP OBJ ON
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(WBTSUBSCRN_WORK* pWork)
{
  /* do nothing */
}

//------------------------------------------------------------------------------
/**
 * @brief   スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
WBTSUBSCRN_WORK* WBTSUBSCRN_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys )
{
	WBTSUBSCRN_WORK *pWork = NULL;

	pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELDMAP, sizeof( WBTSUBSCRN_WORK ) );
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
void WBTSUBSCRN_Main( WBTSUBSCRN_WORK* pWork,BOOL bAction )
{
	StateFunc* state = pWork->state;
	
	if(state != NULL)
	{
		state(pWork);
	}
	
  { //ビーコンの電波強度アイコン反映	
    GAMEDATA *pGameData = GAMESYSTEM_GetGameData( pWork->pGameSys );
    WIH_DWC_GetAllBeaconTypeBit( GAMEDATA_GetWiFiList(pGameData) );
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   WBTSUBSCRN_ActionCallback
 * @retval  none
 */
//------------------------------------------------------------------------------
void WBTSUBSCRN_ActionCallback( WBTSUBSCRN_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno)
{
}


//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
void WBTSUBSCRN_Exit( WBTSUBSCRN_WORK* pWork )
{
  _workEnd(pWork);
	G2S_BlendNone();
	GFL_HEAP_FreeMemory(pWork);
}




