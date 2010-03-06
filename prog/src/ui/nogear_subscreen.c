//============================================================================
/**
 *  @file   nogear_subscreen.c
 *  @brief  Cギア入手前のサブスクリーンの生成、破棄
 *  @author Koji Kawada
 *  @data   2010.03.04
 *  @note   
 *  モジュール名：NOGEAR_SUBSCREEN
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "pm_version.h"
#include "arc_tool.h"

#include "ui/nogear_subscreen.h"

// アーカイブ
#include "arc_def.h"
#include "c_gear.naix"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// サブBGフレーム
#define BG_FRAME_S_NOGEAR      (GFL_BG_FRAME0_S)

// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_NOGEAR  (0)

// サブBGパレット
// 本数
enum
{
  BG_PAL_NUM_S_NOGEAR        = 0,  // 全転送
};
// 位置
enum
{
  BG_PAL_POS_S_NOGEAR       = 0,
};

// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_            = 0,
};
// 位置
enum
{
  OBJ_PAL_POS_S_            = 0,
};

// ＢＧ設定
// BGグラフィックモード設定
static const GXBGMode bgModeSub = GX_BGMODE_0; //サブBG面設定
// BG面設定
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
		BG_FRAME_S_NOGEAR,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x8000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, BG_FRAME_PRI_S_NOGEAR, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
};

// Cギア入手前のサブスクリーンのデータ
typedef enum
{
  TYPE_MALE,
  TYPE_FEMALE,
  TYPE_MAX,
}
TYPE;
enum
{
  DATA_NCL,
  DATA_NCG,
  DATA_NSC,
  DATA_MAX,
};

static ARCDATID arcdatid[TYPE_MAX][DATA_MAX] =
{
  {
    NARC_c_gear_c_gear_NCLR,
    NARC_c_gear_c_gear_NCGR,
    NARC_c_gear_c_gear01_n_NSCR,
  },
  {
    NARC_c_gear_c_gear2_NCLR,
    NARC_c_gear_c_gear2_NCGR,
    NARC_c_gear_c_gear01_n_NSCR,
  },
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           サブスクリーン初期化処理
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void NOGEAR_SUBSCREEN_Init(void)
{
	//レジスタ初期化
	G2S_BlendNone();	
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );

	//ディスプレイON
  GXS_DispOn();
	
  //表示
	GFL_DISP_GXS_InitVisibleControl();

	//グラフィックモード設定
	{	
    GXS_SetGraphicsMode( bgModeSub );
    GFL_DISP_GXS_InitVisibleControlBG();
	}

	//BG面設定
	{	
		int i;
		for( i = 0; i < NELEMS(sc_bgsetup); i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, sc_bgsetup[i].is_visible );
		}
	}
}

//------------------------------------------------------------------
/**
 *  @brief           サブスクリーン終了処理
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void NOGEAR_SUBSCREEN_Exit(void)
{
	//BG面破棄
	{	
		int i;
		for( i = 0; i < NELEMS(sc_bgsetup); i++ )
		{	
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

	//レジスタ初期化
	G2S_BlendNone();	
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );
}

//------------------------------------------------------------------
/**
 *  @brief           サブスクリーン読み込んで転送
 *
 *  @param[in]       heap_id       ﾃﾞｰﾀ読み込みﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *  @param[in]       sex           PM_MALE or PM_FEMALE  // include/pm_version.h
 *
 *  @retval          
 */
//------------------------------------------------------------------
void NOGEAR_SUBSCREEN_Trans(
               HEAPID   heap_id,     // ﾃﾞｰﾀ読み込みﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
               u8       sex          // PM_MALE or PM_FEMALE  // include/pm_version.h
            )
{
  ARCHANDLE* handle;
  TYPE       type     = ( sex == PM_FEMALE ) ? ( TYPE_FEMALE ) : ( TYPE_MALE );

  // Cギア入手前のサブスクリーンの生成
  handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, heap_id );

  GFL_ARCHDL_UTIL_TransVramPalette(
      handle,
      arcdatid[type][DATA_NCL],
      PALTYPE_SUB_BG,
      BG_PAL_POS_S_NOGEAR * 0x20,
      BG_PAL_NUM_S_NOGEAR * 0x20,
      heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      arcdatid[type][DATA_NCG],
      BG_FRAME_S_NOGEAR,
			0,
      0,  // 全転送
      FALSE,
      heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      arcdatid[type][DATA_NSC],
      BG_FRAME_S_NOGEAR,
      0,
      0,  // 全転送
      FALSE,
      heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_S_NOGEAR );
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 
//=====================================

