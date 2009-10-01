//=============================================================================
/**
 *
 *	@file		nsbtx_to_clwk.c
 *	@brief  テクスチャ(nsbtx)をOAM形式に変換し、CLWKに転送
 *	@author	hosaka genya
 *	@data		2009.10.31
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gra_tool.h"

#include "nsbtx_to_clwk.h"

#include "arc_def.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

static void OBJ_CgxChangeTransShiftVramOfs( GFL_CLWK* act, u8 * buf, u32 siz, u32 VramOfs, NNS_G2D_VRAM_TYPE vram_type );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================


//-----------------------------------------------------------------------------
/**
 *	@brief  nsbtxをニトロキャラクタ、ニトロパレットに変換してCLWKに転送
 *
 *	@param	GFL_CLWK* act
 *	@param  arc_idx
 *	@param	tex_idx
 *	@param	ptn_ofs
 *	@param	sx
 *	@param	sy
 *	@param	VramOfs
 *	@param	vram_type
 *	@param	HeapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void CLWK_TransNSBTX( GFL_CLWK* act, u32 arc_idx, u32 tex_idx, u8 ptn_ofs, u16 sx, u16 sy, u32 VramOfs, CLSYS_DRAW_TYPE vram_type, HEAPID HeapID )
{
	enum {
		TRANS_PAL_SIZE = 0x20,  ///< 一列
	};

	u32	i;
  u32 trans_size;
	ARCHANDLE*		arc;
	NNSG3dResTex*	pTex;	
	void*			pFile;
	u8*				pStart;
	void*			pRawData;
	NNSG3dResDictTexData* pDictTex;
		
  trans_size = sx * sy * 0x20;
		
	// テクスチャのアーカイブオープン (nsbtx)
	arc = GFL_ARC_OpenDataHandle( arc_idx, HeapID );

	// テクスチャファイルを取得
  pFile = GFL_ARC_LoadDataAllocByHandle( arc, tex_idx, HeapID );
	pTex = NNS_G3dGetTex( (NNSG3dResFileHeader*)pFile );
	pDictTex = NNS_G3dGetTexDataByIdx( pTex, ptn_ofs );

	//テクスチャロウデータ取得
	{
		void* pBuf;
		u32 offset = (pDictTex->texImageParam & NNS_G3D_TEXIMAGE_PARAM_TEX_ADDR_MASK)<<3;
		
		HOSAKA_Printf( "tex size = %d \n", pTex->texInfo.sizeTex );

		pStart = (u8*)pTex + pTex->texInfo.ofsTex;
		
		pBuf = GFL_HEAP_AllocMemoryLo( HeapID, trans_size );

		pRawData = &pStart[ offset ];
		ChangesInto_RawData_1D_from_2D( pRawData, sx, 0, 0, sx, sy, pBuf );

    // CLWKに流し込む
#if 0
    {
      //@TODO 何故かサイズオーバーする。
      u16 res_idx = GFL_CLACT_WK_GetCgrIndex( act ); 
      HOSAKA_Printf("res_idx=%d \n");
      GFL_CLGRP_CGR_ReplaceEx( res_idx, pBuf, trans_size, VramOfs, vram_type );
    }
#else
		OBJ_CgxChangeTransShiftVramOfs( act, pBuf, trans_size, VramOfs, vram_type );
#endif

		GFL_HEAP_FreeMemory( pBuf );
	}

	//パレットロウデータ取得
	{
		NNSG2dImagePaletteProxy* ipp = NULL;
		u32 vram;

		pStart = (u8*)pTex + pTex->plttInfo.ofsPlttData;
		DC_FlushRange( pStart, TRANS_PAL_SIZE );
		
    GFL_CLACT_WK_GetPlttProxy( act, ipp );
		vram = NNS_G2dGetImagePaletteLocation( ipp, vram_type );

		if( vram_type == NNS_G2D_VRAM_TYPE_2DMAIN )
		{
			GX_LoadOBJPltt( pStart, vram, TRANS_PAL_SIZE );
		}
		else
		{
			GXS_LoadOBJPltt( pStart, vram, TRANS_PAL_SIZE );
		}
	}

	// -- 開放処理 --
  GFL_HEAP_FreeMemory( pFile );
	GFL_ARC_CloseDataHandle( arc );
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

// tomoyaさんに GFL_CLGRP_CGR_ReplaceEx を作っていただいため、不要になりました。
#if 1
//-----------------------------------------------------------------------------
/**
 *	@brief	OBJに割り当てられているキャラデータ（CGX）を書き換える 
 *
 *	@param	GFL_CLWK act
 *	@param	* buf
 *	@param	siz
 *	@param	VramOfs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void OBJ_CgxChangeTransShiftVramOfs( GFL_CLWK* act, u8 * buf, u32 siz, u32 VramOfs, NNS_G2D_VRAM_TYPE vram_type )
{
	NNSG2dImageProxy * ipc;
	u32	cgx;

  GFL_CLACT_WK_GetImgProxy( act, ipc );

	cgx = NNS_G2dGetImageLocation( ipc, vram_type );
	cgx += VramOfs;

	DC_FlushRange( buf, siz );

	switch( vram_type )
	{
		case NNS_G2D_VRAM_TYPE_2DMAIN:
			GX_LoadOBJ( buf, cgx, siz );
			break;

		case NNS_G2D_VRAM_TYPE_2DSUB:
			GXS_LoadOBJ( buf, cgx, siz );
			break;

		default : GF_ASSERT(0);
	};
}
#endif

