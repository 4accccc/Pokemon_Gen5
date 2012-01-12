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

#include "font/font.naix"

#include "src/field/wbt_system.h"
#include "src/field/wbt_tool.h"

#include "arc/wbt/wbt_subscreen.naix"
#include "wbt_list_gra.naix"

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

// 定義
enum{
	WBTS_CANVAS_MAXNUM = 2,
};

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
	//
	GFL_FONT		*font;							// フォント
//	PRINT_QUE		*print_que;						// que(念のため使わない)
	PRINT_UTIL		PrintUtil[WBTS_CANVAS_MAXNUM];	// 文字表示用
	
	u16 tpx;
	u16 tpy;
};

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}WBTS_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}WBTS_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}WBTS_BGSCN_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
	u8				frmnumber;		// BG上下(frame定義なので注意)(0xffの場合は終了判定)
	u8				x;				// 座標
	u8				y;
	u8				width;			// サイズ
	u8				height;
	u8				palnum;			// パレット番号
}WBTS_CANVAS_DATA;

//-------------------------------------
///	BG面設定
//-------------------------------------
static const struct 
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
	BOOL								is_visible;
}	sc_bgsetup[]	=
{
	//SUB---------------------------
	{
		GFL_BG_FRAME0_S,	// 文字フォント用
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_128x128,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME1_S,	// マーク用
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ(BG2と共有)
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME2_S,	// 下地(白ライン用)
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ(BG1と共有)
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
};

//---------------
// BG関連
//---------------
// BGPAL
static const WBTS_BGPAL_DATA wbts_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_wbt_list_gra_wbt_list_bg_d_NCLR,	0,	3,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};
// BGCHR
static const WBTS_BGCHR_DATA wbts_bgchr_tbl[] = {
	{ GFL_BG_FRAME2_S,	NARC_wbt_list_gra_wbt_list_bg_d_NCGR,	0,	0,},		// 下地、マークの共有

	{ 0xff, 0, 0, 0,}, // end code
};
// BGSCN
static const WBTS_BGSCN_DATA wbts_bgscn_tbl[] = {
	{ GFL_BG_FRAME1_S,	NARC_wbt_list_gra_wbt_list_bg_d_02_NSCR,	0,	0,},	// マーク
	{ GFL_BG_FRAME2_S,	NARC_wbt_list_gra_wbt_list_bg_d_01_NSCR,	0,	0,},	// 下地

	{ 0xff, 0, 0, 0,}, // end code
};

// CANVASの設定
// (printutilと対応)
const WBTS_CANVAS_DATA wbts_canvas_tbl[] = {
	{ GFL_BG_FRAME0_S,	2,	4,	28,	2,	2,},		// 下、up
	{ GFL_BG_FRAME0_S,	2,	17,	28,	2,	2,},		// 下、down

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
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
//
static void _wbts_setup_bg( WBTSUBSCRN_WORK* wk );
static void _wbts_init_bg(  WBTSUBSCRN_WORK* wk );
//
static void _wbts_init_string( WBTSUBSCRN_WORK* wk );
static void _wbts_disp_string( WBTSUBSCRN_WORK* wk, const s32 mode, const s32 x, const s32 y, STRBUF* strbuf );
static void _wbts_exit_string( WBTSUBSCRN_WORK* wk );
static void _wbts_disp_info(   WBTSUBSCRN_WORK* wk );
//
static void _wbts_init_obj( WBTSUBSCRN_WORK* wk );

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


#if 0	// 保留
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
#endif

#if 0	// 保留
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
#endif

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(WBTSUBSCRN_WORK* pWork)
{
//	_createSubBg(pWork);   //BGVRAM設定
//	_loadArc(pWork);  //ARC読み込み BG
	//
	// BGVRAM設定
	_wbts_setup_bg( pWork );
	// データの読み込み
	_wbts_init_bg(  pWork );
	// string初期化
	_wbts_init_string( pWork );
	// トーナメント情報表示
	_wbts_disp_info( pWork );
	// objの初期化
	_wbts_init_obj( pWork );
	
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
	//
	// 終了処理
	_wbts_exit_string( pWork );
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
  
  // PRINT_QUE
//  PRINTSYS_QUE_Main( pWork->print_que );
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
	//
	OS_Printf("wbt subscreen end\n");
}

//------------------------------------------------------------------------------
/**
 * @brief   bg初期化
 *
 * @param   WBTSUBSCRN_WORK*
 *
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _wbts_setup_bg( WBTSUBSCRN_WORK* wk )
{
	s32 i;
	
	// 一度クリア
	for(i=GFL_BG_FRAME0_S;i<=GFL_BG_FRAME3_S;i++){
		GFL_BG_SetVisible( i, VISIBLE_OFF );
	}
	
	// セット
	for(i=0;i<NELEMS(sc_bgsetup);i++){
		GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
		GFL_BG_ClearFrame( sc_bgsetup[i].frame );
		GFL_BG_SetVisible( sc_bgsetup[i].frame, sc_bgsetup[i].is_visible );
	}
	//
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	//PRINT_QUE作成
//	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGデータの転送
 *
 *	@param	WBTSUBSCRN_WORK*
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _wbts_init_bg( WBTSUBSCRN_WORK* wk )
{
	u16		*adr;
	s32		i;
	s32		j;
	s32		tmp;
	s32		off;
	const WBTS_BGPAL_DATA	*padr = &wbts_bgpal_tbl[0];
	const WBTS_BGCHR_DATA	*cadr = &wbts_bgchr_tbl[0];
	const WBTS_BGSCN_DATA	*sadr = &wbts_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_WBTLIST_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<4;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		off = padr->offset;
		off *= 0x20;
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, off, tmp, wk->heapID );
		padr++;
	}
	//
	// キャラ転送
	for(i=0;i<4;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<4;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, wk->heapID );
		sadr++;
	}
	// 終了
	GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備
 *
 *	@param	WBTSUBSCRN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _wbts_init_string( WBTSUBSCRN_WORK* wk )
{
	s32		i;
	const WBTS_CANVAS_DATA	*adr = &wbts_canvas_tbl[0];
	
	for(i=0;i<WBTS_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット(参照位置だけ注意:GFL_BMP_CHRAREA_GET_B)
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	WBTSUBSCRN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x,y)
 *	@param	STRBUF* 			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _wbts_disp_string( WBTSUBSCRN_WORK* wk, const s32 mode, const s32 x, const s32 y, STRBUF* strbuf )
{
	s32		size = PRINTSYS_GetStrWidth( strbuf, wk->font, 0 ) / 2;
	//
//	PRINT_UTIL_PrintColor( &wk->PrintUtil[mode], wk->print_que, x-size, y, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
	PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), x-size, y, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
	// 転送
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// 転送
//	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	WBTSUBSCRN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _wbts_exit_string( WBTSUBSCRN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<WBTS_CANVAS_MAXNUM;i++){
		GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
	}
	//PRINT_QUE
//	PRINTSYS_QUE_Delete( wk->print_que );
	//FONT
	GFL_FONT_Delete( wk->font );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	WBTSUBSCRN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _wbts_disp_info( WBTSUBSCRN_WORK* wk )
{
	GAMEDATA* gdata = GAMESYSTEM_GetGameData( wk->pGameSys );
	WBTSYS ** wbtsys = GAMEDATA_GetWBTSysPointer( gdata );
	STRBUF	*strbuf;
	//
	// 念のため
	if(wbtsys == NULL){
		return;
	}
	if(*wbtsys == NULL){
		return;
	}
	//
	// データが存在するので表示
	// トーナメント名
	strbuf  = GFL_STR_CreateBuffer( 128, wk->heapID );
	WBTSYS_GetTournamentName( *wbtsys, WBTSYS_GetWBTCup(*wbtsys), strbuf );
	_wbts_disp_string( wk, 0, 112, 0, strbuf );
	GFL_STR_DeleteBuffer( strbuf );
	// ルール
	strbuf  = GFL_STR_CreateBuffer( 128, wk->heapID );
	WBTTOOL_GetRuleName( WBTSYS_GetRule(*wbtsys), strbuf );
	_wbts_disp_string( wk, 1, 112, 0, strbuf );
	GFL_STR_DeleteBuffer( strbuf );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの初期化(現在は通信アイコン)
 *
 *	@param	WBTSUBSCRN_WORK*
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _wbts_init_obj( WBTSUBSCRN_WORK* wk )
{
	// 通信アイコンセット(下画面)
	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->heapID );
	// 表示
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

