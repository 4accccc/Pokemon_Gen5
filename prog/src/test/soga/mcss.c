//============================================================================================
/**
 * @file	mcss.c
 * @brief	マルチセルソフトウエアスプライト描画用関数
 * @author	soga
 * @date	2008.09.16
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>

#include "mcss.h"

//--------------------------------------------------------------------------
/**
 * プロトタイプ宣言
 */
//--------------------------------------------------------------------------
MCSS_SYS_WORK*	MCSS_Init( int max, VecFx32 *camPos,VecFx32 *target, VecFx32 *camUp,HEAPID heapID );
void	MCSS_Exit( MCSS_SYS_WORK *mcss_sys );
void	MCSS_Main( MCSS_SYS_WORK *mcss_sys );
void	MCSS_Draw( MCSS_SYS_WORK *mcss_sys );
MCSS_WORK*	MCSS_Add( MCSS_SYS_WORK *mcss_sys,
					  fx32			pos_x,
					  fx32			pos_y,
					  fx32			pos_z,
					  ARCID			arcID,
					  ARCDATID		ncbr,
					  ARCDATID		nclr,
					  ARCDATID		ncer,
					  ARCDATID		nanr,
					  ARCDATID		nmcr,
					  ARCDATID		nmar );
void	MCSS_Del( MCSS_SYS_WORK *mcss_sys, MCSS_WORK *mcss );

static	void	MCSS_LoadResource( MCSS_SYS_WORK	*mcss_sys,
								   MCSS_WORK		*mcss,
								   int				chr_ofs,
								   int				pal_ofs,
								   ARCID			arcID,
								   ARCDATID			ncbr,
								   ARCDATID			nclr,
								   ARCDATID			ncer,
								   ARCDATID			nanr,
								   ARCDATID			nmcr,
								   ARCDATID			nmar );
//static	NNSG2dMultiCellAnimation*	MCSS_GetNewMultiCellAnimation(MCSS_WORK *mcss, NNSG2dMCType	mcType );
static	void	MCSS_GetNewMultiCellAnimation(MCSS_WORK *mcss, NNSG2dMCType	mcType );
static	void	MCSS_MaterialSetup( void );
static	void	MCSS_InitRenderer( MCSS_SYS_WORK *mcss_sys );
static NNSG2dMultiCellAnimation*     GetNewMultiCellAnim_( u16 num );

//--------------------------------------------------------------------------
/**
 * システム初期化
 */
//--------------------------------------------------------------------------
MCSS_SYS_WORK*	MCSS_Init( int max, VecFx32 *camPos,VecFx32 *target, VecFx32 *camUp,HEAPID heapID )
{
	MCSS_SYS_WORK	*mcss_sys;

	mcss_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(MCSS_SYS_WORK) );

	mcss_sys->max = max;
	mcss_sys->camPos = camPos;
	mcss_sys->target = target;
	mcss_sys->camUp = camUp;
	mcss_sys->heapID = heapID;

	mcss_sys->mcss =GFL_HEAP_AllocClearMemory( heapID, 4 * max );

	MCSS_InitRenderer( mcss_sys );
	
	return mcss_sys;
}

//--------------------------------------------------------------------------
/**
 * システム終了
 */
//--------------------------------------------------------------------------
void	MCSS_Exit( MCSS_SYS_WORK *mcss_sys )
{
	int	i;

	for( i = 0 ; i < mcss_sys->max ; i++ ){
		if( mcss_sys->mcss[i] != NULL ){
			MCSS_Del( mcss_sys, mcss_sys->mcss[i] );
		}
	}
	GFL_HEAP_FreeMemory( mcss_sys );
}

//--------------------------------------------------------------------------
/**
 * システムメイン
 */
//--------------------------------------------------------------------------
void	MCSS_Main( MCSS_SYS_WORK *mcss_sys )
{
	int	index;

	for( index = 0 ; index < mcss_sys->max ; index++ ){
		if( mcss_sys->mcss[index] != NULL ){
			// アニメーションを更新します
			NNS_G2dTickMCAnimation( &mcss_sys->mcss[index]->mcss_mcanim, FX32_ONE );
		}
	}
}

MCSS_WORK	*mcss_p;

//--------------------------------------------------------------------------
/**
 * 描画システム
 */
//--------------------------------------------------------------------------
void	MCSS_Draw( MCSS_SYS_WORK *mcss_sys )
{
	MCSS_WORK	*mcss;
	int	index;

	for( index = 0 ; index < mcss_sys->max ; index++ ){
		if( mcss_sys->mcss[index] != NULL ){
			mcss_p=mcss_sys->mcss[index];
			mcss=mcss_sys->mcss[index];
			//暫定でRendererを使用しているが、実際は独自で描画処理を作成しなければならない
			NNS_G2dBeginRendering( &mcss_sys->mcss_render );
			NNS_G2dPushMtx();
//			G3_LookAt(mcss_sys->camPos, mcss_sys->camUp, mcss_sys->target, &mcss_sys->mcss_render.rendererCore.mtxFor3DGE);
			NNS_G2dTranslate( mcss->pos.x,
							  mcss->pos.y,
							  mcss->pos.z);
			NNS_G2dScale( FX32_ONE, -FX32_ONE , FX32_ONE );
//			NNS_G2dRotZ( FX_SinIdx( cell->rotate ), FX_CosIdx( cell->rotate ) );
			NNS_G2dSetRendererImageProxy( &mcss_sys->mcss_render,
										  &mcss->mcss_image_proxy,
										  &mcss->mcss_palette_proxy );
			NNS_G2dDrawMultiCellAnimation( &mcss->mcss_mcanim );
			NNS_G2dPopMtx();
			NNS_G2dEndRendering();
		}
	}
}

//--------------------------------------------------------------------------
/**
 * マルチセル登録
 */
//--------------------------------------------------------------------------
MCSS_WORK*	MCSS_Add( MCSS_SYS_WORK *mcss_sys,
					  fx32			pos_x,
					  fx32			pos_y,
					  fx32			pos_z,
					  ARCID			arcID,
					  ARCDATID		ncbr,
					  ARCDATID		nclr,
					  ARCDATID		ncer,
					  ARCDATID		nanr,
					  ARCDATID		nmcr,
					  ARCDATID		nmar )
{
	int			count;

	for( count = 0 ; count < mcss_sys->max ; count++ ){
		if( mcss_sys->mcss[ count ] == NULL ){
			mcss_sys->mcss[ count ] = GFL_HEAP_AllocClearMemory( mcss_sys->heapID, sizeof(MCSS_WORK) );
			mcss_sys->mcss[ count ]->index = count;
			mcss_sys->mcss[ count ]->heapID = mcss_sys->heapID;
			mcss_sys->mcss[ count ]->pos.x = pos_x;
			mcss_sys->mcss[ count ]->pos.y = pos_y;
			mcss_sys->mcss[ count ]->pos.z = pos_z;
			MCSS_LoadResource( mcss_sys,
							   mcss_sys->mcss[ count ],
							   0x8000+0x4000*count,0x100+0x20*count,
							   arcID,
							   ncbr,
							   nclr,
							   ncer,
							   nanr,
							   nmcr,
							   nmar );
			break;
		}
	}
	//登録MAX値をオーバー
	GF_ASSERT( count < mcss_sys->max );

	return mcss_sys->mcss[ count ];
}

//--------------------------------------------------------------------------
/**
 * マルチセル削除
 */
//--------------------------------------------------------------------------
void	MCSS_Del( MCSS_SYS_WORK *mcss_sys, MCSS_WORK *mcss )
{
	GFL_HEAP_FreeMemory( mcss->mcss_ncer_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_nanr_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_nmcr_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_nmar_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_mcanim_buf );

	mcss_sys->mcss[ mcss->index ] = NULL;
	GFL_HEAP_FreeMemory( mcss );
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
/**
 * リソースロード
 */
//--------------------------------------------------------------------------
static	void	MCSS_LoadResource( MCSS_SYS_WORK	*mcss_sys,
								   MCSS_WORK		*mcss,
								   int				chr_ofs,
								   int				pal_ofs,
								   ARCID			arcID,
								   ARCDATID			ncbr,
								   ARCDATID			nclr,
								   ARCDATID			ncer,
								   ARCDATID			nanr,
								   ARCDATID			nmcr,
								   ARCDATID			nmar )
{
	//プロキシ初期化
	NNS_G2dInitImageProxy( &mcss->mcss_image_proxy );
	NNS_G2dInitImagePaletteProxy( &mcss->mcss_palette_proxy );

	// セルデータ、セルアニメーション、マルチセルデータ、
	// マルチセルアニメーションをロード。
	mcss->mcss_ncer_buf = GFL_ARC_UTIL_LoadCellBank(		arcID, ncer, FALSE, &mcss->mcss_ncer, mcss->heapID );
	GF_ASSERT( mcss->mcss_ncer_buf != NULL );
	mcss->mcss_nanr_buf = GFL_ARC_UTIL_LoadAnimeBank(		arcID, nanr, FALSE, &mcss->mcss_nanr, mcss->heapID );
	GF_ASSERT( mcss->mcss_nanr_buf != NULL );
	mcss->mcss_nmcr_buf = GFL_ARC_UTIL_LoadMultiCellBank(	arcID, nmcr, FALSE, &mcss->mcss_nmcr, mcss->heapID );
	GF_ASSERT( mcss->mcss_nmcr_buf != NULL );
	mcss->mcss_nmar_buf = GFL_ARC_UTIL_LoadMultiAnimeBank(	arcID, nmar, FALSE, &mcss->mcss_nmar, mcss->heapID );
	GF_ASSERT( mcss->mcss_nmar_buf != NULL );

	//
	// マルチセルアニメーションの実体を初期化します
	//
	{
		const NNSG2dMultiCellAnimSequence* pSequence;

		// 再生するシーケンスを取得
		pSequence = NNS_G2dGetAnimSequenceByIdx( mcss->mcss_nmar, 0 );
		GF_ASSERT( pSequence != NULL );

		// マルチセルアニメーションを構築
		MCSS_GetNewMultiCellAnimation( mcss, NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM);

		// マルチセルアニメーションに再生するシーケンスをセット
		NNS_G2dSetAnimSequenceToMCAnimation( &mcss->mcss_mcanim, pSequence);
	}

    //
    // VRAM 関連の初期化
    //
    {
		void* pBuf;

//当面、OAMでの表示はしないのでコメントアウト
#if 0
		// load character data for 2D
		{
			NNSG2dCharacterData*	pCharData = NULL;

			pBuf = GFL_ARC_UTIL_LoadBGCharacter( arcID, ncgr, FALSE, &pCharData, mcss->heapID );
			GF_ASSERT( pBuf != NULL);

			// Loading For 2D Graphics Engine.（本来は、VRAMマネージャを使用したい）
			NNS_G2dLoadImage2DMapping(
				pCharData,
				CHARA_BASE,
				NNS_G2D_VRAM_TYPE_2DMAIN,
				&mcss_sys->mcss_image_proxy );

			// キャラクタデータを VRAM にコピーしたので
			// この pBuf は開放します。以下同じ。
			GFL_HEAP_FreeMemory( pBuf );
		}
#endif

		// load character data for 3D (software sprite)
		{
			NNSG2dCharacterData* pCharData;

			pBuf = GFL_ARC_UTIL_LoadBGCharacter( arcID, ncbr, FALSE, &pCharData, mcss->heapID );
			GF_ASSERT( pBuf != NULL);

			// Loading For 3D Graphics Engine.（本来は、VRAMマネージャを使用したい）
			NNS_G2dLoadImage2DMapping(
				pCharData,
				chr_ofs,
				NNS_G2D_VRAM_TYPE_3DMAIN,
				&mcss->mcss_image_proxy );

			GFL_HEAP_FreeMemory( pBuf );
        }

		// load palette data
		{
			NNSG2dPaletteData* pPlttData;

			pBuf = GFL_ARC_UTIL_LoadPalette( arcID, nclr, &pPlttData, mcss->heapID );
			GF_ASSERT( pBuf != NULL);

//当面、OAMでの表示はしないのでコメントアウト
#if 0
			// Loading For 2D Graphics Engine.
			NNS_G2dLoadPalette(
				pPlttData,
				PLTT_BASE,
				NNS_G2D_VRAM_TYPE_2DMAIN,
				&mcss_sys->mcss_palette_proxy );
#endif

            // Loading For 3D Graphics Engine.
			NNS_G2dLoadPalette(
				pPlttData,
				pal_ofs,
				NNS_G2D_VRAM_TYPE_3DMAIN,
				&mcss->mcss_palette_proxy );

			GFL_HEAP_FreeMemory( pBuf );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MCSS_GetNewMultiCellAnimation

  Description:  新しい NNSG2dMultiCellAnimation を取得して初期化します。

  Arguments:    pAnimBank:          アニメーションデータバンク
                pCellDataBank:      セルデータバンク
                pMultiCellDataBank: マルチセルデータバンク
                mcType:             マルチセル実体の種類
                
  Returns:      プールから取得した初期化済みの NNSG2dMultiCellAnimation へ
                のポインタ
 *---------------------------------------------------------------------------*/
static	void	MCSS_GetNewMultiCellAnimation(MCSS_WORK *mcss, NNSG2dMCType	mcType )
{
	const u32 szWork = NNS_G2dGetMCWorkAreaSize( mcss->mcss_nmcr, mcType );
	mcss->mcss_mcanim_buf = GFL_HEAP_AllocMemory( mcss->heapID, szWork );

	// 初期化
	NNS_G2dInitMCAnimationInstance( &mcss->mcss_mcanim, 
                                    mcss->mcss_mcanim_buf,
                                    mcss->mcss_nanr, 
                                    mcss->mcss_ncer, 
                                    mcss->mcss_nmcr, 
                                    mcType );
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_MaterialSetup

  Description:  マテリアルの共通設定を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static	void	MCSS_MaterialSetup(void)
{
	// for software sprite-setting
	{
		G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),		// diffuse
								GX_RGB(16, 16, 16),		// ambient
								TRUE					// use diffuse as vtx color if TRUE
								);

		G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),		// specular
								GX_RGB( 0,  0,  0),		// emission
                                FALSE					// use shininess table if TRUE
                                );

		G3_PolygonAttr(GX_LIGHTMASK_NONE,				// no lights
					   GX_POLYGONMODE_MODULATE,			// modulation mode
					   GX_CULL_NONE,					// cull back
					   0,								// polygon ID(0 - 63)
					   31,								// alpha(0 - 31)
					   0								// OR of GXPolygonAttrMisc's value
					   );
	}
}

/*---------------------------------------------------------------------------*
  Name:         MCSS_InitRenderer

  Description:  Renderer と Surface を初期化します。

  Arguments:    pRenderer:  初期化する Renderer へのポインタ。
                pSurface:   初期化する Surface へのポインタ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static	void	MCSS_InitRenderer( MCSS_SYS_WORK *mcss_sys )
{
	NNSG2dViewRect* pRect = &(mcss_sys->mcss_surface.viewRect);
 
	NNS_G2dInitRenderer( &mcss_sys->mcss_render );
	NNS_G2dInitRenderSurface( &mcss_sys->mcss_surface );

	pRect->posTopLeft.x = FX32_ONE * 0;
	pRect->posTopLeft.y = FX32_ONE * 0;
	pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
	pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

	//とりあえずOAM描画はしないので、コメントアウト
//	pSurface->pFuncOamRegister       = CallBackAddOam;
//	pSurface->pFuncOamAffineRegister = CallBackAddAffine;

	NNS_G2dAddRendererTargetSurface( &mcss_sys->mcss_render, &mcss_sys->mcss_surface );
	NNS_G2dSetRendererSpriteZoffset( &mcss_sys->mcss_render, DEFAULT_Z_OFFSET );

	mcss_sys->mcss_surface.type = NNS_G2D_SURFACETYPE_MAIN3D;
}

