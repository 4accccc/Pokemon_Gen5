//=============================================================================================
/**
 * @file	g3d_tool.c
 * @brief	g3d関連サポートツール
 * @date	2008.12.09
 * @author	tamada GAME FREAK inc.
 */
//=============================================================================================
#include <gflib.h>

#include "system/g3d_tool.h"

#ifdef	PM_DEBUG
//=============================================================================================
//=============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
//g3d_system.cの定義からほぼコピー
enum {
	TEX_SLOTSIZ	=	0x20000,
	PLT_SLOTSIZ =	0x2000,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int index;
	u32 start_addr;
	u32 end_addr;
	u32 blockMax;
	BOOL active;
}FRM_TEX_DUMP_PARAM;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GFL_G3D_VMAN_TEXSIZE texsize;
	GFL_G3D_VMAN_PLTSIZE pltsize;
	u32 frm_plt_free;
}DEBUG_GFL_G3D_INFO;

static DEBUG_GFL_G3D_INFO DebugVManConfigValue;

//=============================================================================================
//
//		内部用関数
//
//=============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
u32 DEBUG_GFL_G3D_GetVManTextureSize(void)
{
	return DebugVManConfigValue.texsize * TEX_SLOTSIZ;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u32 DEBUG_GFL_G3D_GetVManPaletteSize(void)
{
	return DebugVManConfigValue.pltsize * PLT_SLOTSIZ;
}

//------------------------------------------------------------------
/**
 * @brief	リンクドVRAMマネージャの内部情報を取得するためのコールバック
 */
//------------------------------------------------------------------
static void LinkDumpCallBack(u32 addr, u32 szByte, void* pUserData )
{
	// 合計サイズを計算
    (*((u32*)pUserData)) += szByte;
}

//------------------------------------------------------------------
/**
 * @brief	フレームVRAMマネージャの内部情報を取得するためのコールバック
 */
//------------------------------------------------------------------
static void FrmTextureDumpCallBack(int index, u32 start_addr, u32 end_addr,
		u32 blockMax, BOOL active, void *userContext)
{
	FRM_TEX_DUMP_PARAM * p = (FRM_TEX_DUMP_PARAM*)userContext;
	p->index = index;
	p->start_addr = start_addr;
	p->end_addr = end_addr;
	p->blockMax = blockMax;
	p->active = active;
}

//------------------------------------------------------------------
/**
 * @brief	フレームパレットVRAMマネージャの内部情報取得のためのコールバック
 */
//------------------------------------------------------------------
static void FrmPaletteDumpCallBack(u32 loAddr, u32 hiAddr, u32 szFree, u32 szTotal)
{
	DebugVManConfigValue.frm_plt_free = szFree;
}

//==============================================================================
/**
 * @brief	初期化用関数
 */
//==============================================================================
void DEBUG_GFL_G3D_SetVManSize(GFL_G3D_VMAN_TEXSIZE texsize, GFL_G3D_VMAN_PLTSIZE pltsize)
{
	DebugVManConfigValue.texsize = texsize;
	DebugVManConfigValue.pltsize = pltsize;
	DebugVManConfigValue.frm_plt_free = 0;
}

//==============================================================================
/**
 * @brief	VRAMへのテクスチゃ転送状態を取得する
 *
 * @return  u32		ＶＲＡＭ余り
 */
//==============================================================================
u32  DEBUG_GFL_G3D_GetBlankTextureSize(void)
{
	u32 size;
	size = 0;
	if (GFL_G3D_GetTextureManagerMode() == GFL_G3D_VMANLNK) {
		NNS_GfdDumpLnkTexVramManagerEx( LinkDumpCallBack, NULL , &size );
	} else {
		FRM_TEX_DUMP_PARAM param;
		NNS_GfdDumpFrmTexVramManagerEx( FrmTextureDumpCallBack, &param );
		size = param.end_addr - param.start_addr;
	}
	return size;
}

//==============================================================================
/**
 * @brief	VRAMへのテクスチゃ転送状態を取得する
 *
 * @return  u32		使用しているＶＲＡＭ容量
 */
//==============================================================================
u32  DEBUG_GFL_G3D_GetUseTextureSize(void)
{
	u32 size;
	size = DEBUG_GFL_G3D_GetBlankTextureSize();
	// VRAMマネージャーが保存しているのは「使用していない量」なので総量から引いて返す
	return DEBUG_GFL_G3D_GetVManTextureSize() - size;
}

//==============================================================================
/**
 * @brief	リンクドPALマネージャーの転送状態を取得する
 *
 * @return  u32		パレット余りサイズ
 */
//==============================================================================
u32  DEBUG_GFL_G3D_GetBlankPaletteSize(void)
{
	u32 size = 0;
	if (GFL_G3D_GetPaletteManagerMode() == GFL_G3D_VMANLNK) {
		NNS_GfdDumpLnkPlttVramManagerEx( LinkDumpCallBack, &size );
	} else {
		NNS_GfdDumpFrmPlttVramManagerEx( FrmPaletteDumpCallBack );
		size = DebugVManConfigValue.frm_plt_free;
	}
	return size;	
}

//==============================================================================
/**
 * @brief	リンクドPALマネージャーの転送状態を取得する
 *
 * @return  u32		リンクドPALマネージャーの転送バイト数
 */
//==============================================================================
u32  DEBUG_GFL_G3D_GetUsePaletteSize(void)
{
	u32 size;
	size = DEBUG_GFL_G3D_GetBlankPaletteSize();
	// PALマネージャーが保存しているのは「使用していない量」なので総量から引いて返す
	return DEBUG_GFL_G3D_GetVManPaletteSize() - size;	
}

#endif	//PM_DEBUG

