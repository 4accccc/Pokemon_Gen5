//=============================================================================
/**
 *	GAME FREAK inc.
 *
 *	@file		field_subscreen.c
 *	@brief
 *	@author	 
 *	@date		2009.03.26
 *
 */
//=============================================================================

#include <gflib.h>
#include "field_subscreen.h"

#include "infowin/infowin.h"
#include "c_gear/c_gear.h"

#include "sound/snd_viewer.h"
#include "sound/pm_sndsys.h"

#include "test/camera_adjust_view.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

#define CGEAR_ON (1)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

struct _FIELD_SUBSCREEN_WORK {
	FIELD_SUBSCREEN_MODE mode;
	HEAPID heapID;
  C_GEAR_WORK* cgearWork;

	union {	
		GFL_CAMADJUST * gflCamAdjust;
		GFL_SNDVIEWER * gflSndViewer;
		void * checker;
	};
	u16 angle_h;
	u16 angle_v;
	fx32 len;
};

typedef void INIT_FUNC(FIELD_SUBSCREEN_WORK *);
typedef void UPDATE_FUNC(FIELD_SUBSCREEN_WORK *);
typedef void EXIT_FUNC(FIELD_SUBSCREEN_WORK *);
typedef struct
{	
	FIELD_SUBSCREEN_MODE mode;		//エラー検出用
	INIT_FUNC * init_func;
	UPDATE_FUNC * update_func;
	EXIT_FUNC * exit_func;
	
}FIELD_SUBSCREEN_FUNC_TABLE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void init_normal_subscreen(FIELD_SUBSCREEN_WORK * pWork);
static void update_normal_subscreen( FIELD_SUBSCREEN_WORK* pWork );
static void exit_normal_subscreen( FIELD_SUBSCREEN_WORK* pWork );

static void init_light_subscreen(FIELD_SUBSCREEN_WORK * pWork);
static void update_light_subscreen( FIELD_SUBSCREEN_WORK* pWork );
static void exit_light_subscreen( FIELD_SUBSCREEN_WORK* pWork );

static void init_touchcamera_subscreen(FIELD_SUBSCREEN_WORK * pWork);
static void update_touchcamera_subscreen( FIELD_SUBSCREEN_WORK* pWork );
static void exit_touchcamera_subscreen( FIELD_SUBSCREEN_WORK* pWork );

static void init_soundviewer_subscreen(FIELD_SUBSCREEN_WORK * pWork);
static void update_soundviewer_subscreen( FIELD_SUBSCREEN_WORK* pWork );
static void exit_soundviewer_subscreen( FIELD_SUBSCREEN_WORK* pWork );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const FIELD_SUBSCREEN_FUNC_TABLE funcTable[] =
{	
	{	
		FIELD_SUBSCREEN_NORMAL,
		init_normal_subscreen,
		update_normal_subscreen,
		exit_normal_subscreen,
	},
	{	
		FIELD_SUBSCREEN_DEBUG_LIGHT,
		init_light_subscreen,
		update_light_subscreen,
		exit_light_subscreen,
	},
	{	
		FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA,
		init_touchcamera_subscreen,
		update_touchcamera_subscreen,
		exit_touchcamera_subscreen,
	},
	{	
		FIELD_SUBSCREEN_DEBUG_SOUNDVIEWER,
		init_soundviewer_subscreen,
		update_soundviewer_subscreen,
		exit_soundviewer_subscreen,
	}
};

//----------------------------------------------------------------------------
/**
 *	@brief	下画面の初期化
 *	
 *	@param	heapID	ヒープＩＤ
 *	@param	mode		動作モード指定
 */
//-----------------------------------------------------------------------------
FIELD_SUBSCREEN_WORK* FIELD_SUBSCREEN_Init( u32 heapID, FIELD_SUBSCREEN_MODE mode )
{
  FIELD_SUBSCREEN_WORK* pWork = GFL_HEAP_AllocClearMemory(heapID, sizeof(FIELD_SUBSCREEN_WORK));
	GF_ASSERT(mode < FIELD_SUBSCREEN_MODE_MAX);
	GF_ASSERT(funcTable[mode].mode == mode);
	pWork->mode = mode;
	pWork->heapID = heapID;
	pWork->checker = NULL;

	funcTable[mode].init_func(pWork);
  
  return pWork;
}

//----------------------------------------------------------------------------
/**
 *	@brief	下画面の破棄処理
 * @param	pWork		サブスクリーン制御ワークへのポインタ
 */
//-----------------------------------------------------------------------------
void FIELD_SUBSCREEN_Exit( FIELD_SUBSCREEN_WORK* pWork )
{
	GF_ASSERT(funcTable[pWork->mode].mode == pWork->mode);
	funcTable[pWork->mode].exit_func(pWork);

  GFL_HEAP_FreeMemory(pWork);
}

//----------------------------------------------------------------------------
/**
 *	@brief	下画面の更新処理
 * @param	pWork		サブスクリーン制御ワークへのポインタ
 */
//-----------------------------------------------------------------------------
void FIELD_SUBSCREEN_Main( FIELD_SUBSCREEN_WORK* pWork )
{
	funcTable[pWork->mode].update_func(pWork);
}

//----------------------------------------------------------------------------
/**
 * @brief
 * @param	pWork
 * @param	pWork		サブスクリーン制御ワークへのポインタ
 * @param	mode
 */
//----------------------------------------------------------------------------
void FIELD_SUBSCREEN_Change( FIELD_SUBSCREEN_WORK* pWork, FIELD_SUBSCREEN_MODE new_mode)
{	
	GF_ASSERT(new_mode < FIELD_SUBSCREEN_MODE_MAX);
	GF_ASSERT(funcTable[new_mode].mode == new_mode);
	funcTable[pWork->mode].exit_func(pWork);
	funcTable[new_mode].init_func(pWork);
	pWork->mode = new_mode;
}

//----------------------------------------------------------------------------
/**
 * @brief
 * @param	pWork		サブスクリーン制御ワークへのポインタ
 * @return	FIELD_SUBSCREEN_MODE	現在の下画面モード
 */
//----------------------------------------------------------------------------
FIELD_SUBSCREEN_MODE FIELD_SUBSCREEN_GetMode(const FIELD_SUBSCREEN_WORK * pWork)
{	
	return pWork->mode;
}

#ifdef	PM_DEBUG
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void * FIELD_SUBSCREEN_DEBUG_GetControl(FIELD_SUBSCREEN_WORK * pWork)
{	
	return pWork->checker;
}
#endif	//PM_DEBUG
//=============================================================================
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	インフォーバーの初期化
 *	
 *	@param	heapID	ヒープＩＤ
 */
//-----------------------------------------------------------------------------
static void init_normal_subscreen(FIELD_SUBSCREEN_WORK * pWork)
{
  // BG3 SUB (インフォバー
  static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,0x6000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};

	GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
	GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );
	GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
	
	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

	INFOWIN_Init( FIELD_SUBSCREEN_BGPLANE , FIELD_SUBSCREEN_PALLET , pWork->heapID);
	if( INFOWIN_IsInitComm() == TRUE )
	{
		GFL_NET_ReloadIcon();
	}
#if CGEAR_ON
  pWork->cgearWork = CGEAR_Init();
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	インフォーバーの破棄
 */
//-----------------------------------------------------------------------------
static void exit_normal_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{
#if CGEAR_ON
  CGEAR_Exit(pWork->cgearWork);
#endif
	INFOWIN_Exit();
	GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
}

//----------------------------------------------------------------------------
/**
 *	@brief	インフォーバーの更新
 */
//-----------------------------------------------------------------------------
static void update_normal_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{
#if CGEAR_ON
  CGEAR_Main(pWork->cgearWork);
#endif
	INFOWIN_Update();
}

//=============================================================================
//=============================================================================
static void init_light_subscreen(FIELD_SUBSCREEN_WORK * pWork)
{	

}
static void update_light_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{	

}
static void exit_light_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{	

}

//=============================================================================
//=============================================================================
static void init_touchcamera_subscreen(FIELD_SUBSCREEN_WORK * pWork)
{
	static const GFL_CAMADJUST_SETUP camAdjustData= {
		PAD_BUTTON_SELECT,
		GFL_DISPUT_BGID_S0, GFL_DISPUT_PALID_15,
	};
	pWork->gflCamAdjust = GFL_CAMADJUST_Create(&camAdjustData, pWork->heapID);

}
static void update_touchcamera_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{	
	if( GFL_CAMADJUST_Main( pWork->gflCamAdjust ) == FALSE )
	{	
		FIELD_SUBSCREEN_Change(pWork, FIELD_SUBSCREEN_NORMAL);
		//GFL_CAMADJUST_Delete( pWork->gflCamAdjust );
		//pWork->gflCamAdjust = NULL;
	}

}
static void exit_touchcamera_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{
	GFL_CAMADJUST_Delete( pWork->gflCamAdjust );
	pWork->gflCamAdjust = NULL;
}

//=============================================================================
//=============================================================================
static void init_soundviewer_subscreen(FIELD_SUBSCREEN_WORK * pWork)
{	
	static const GFL_SNDVIEWER_SETUP sndStatusData= {
		PAD_BUTTON_SELECT,
		GFL_DISPUT_BGID_S0, GFL_DISPUT_PALID_15,
		PMSND_GetBGMhandlePointer,
		PMSND_GetBGMsoundNo,
		PMSND_GetBGMplayerNoIdx,
		PMSND_CheckOnReverb,
		GFL_SNDVIEWER_CONTROL_ENABLE | GFL_SNDVIEWER_CONTROL_EXIT,
	};

	pWork->gflSndViewer = GFL_SNDVIEWER_Create( &sndStatusData, pWork->heapID );

}

static void update_soundviewer_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{	
	if ( GFL_SNDVIEWER_Main( pWork->gflSndViewer ) == FALSE )
	{	
		GFL_SNDVIEWER_Delete( pWork->gflSndViewer );
		pWork->gflSndViewer = NULL;
	}
}

static void exit_soundviewer_subscreen( FIELD_SUBSCREEN_WORK* pWork )
{	
	GFL_SNDVIEWER_Delete( pWork->gflSndViewer );
	pWork->gflSndViewer = NULL;
}


