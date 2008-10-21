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

//#define	USE_RENDER		//有効にすることでNNSのレンダラを使用して描画する

//--------------------------------------------------------------------------
/**
 * プロトタイプ宣言
 */
//--------------------------------------------------------------------------
MCSS_SYS_WORK*	MCSS_Init( int max, GFL_G3D_CAMERA *camera, HEAPID heapID );
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
					  ARCDATID		nmar,
					  ARCDATID		ncec );
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
								   ARCDATID			nmar,
								   ARCDATID			ncec );
static	void	MCSS_GetNewMultiCellAnimation(MCSS_WORK *mcss, NNSG2dMCType	mcType );
static	void	MCSS_MaterialSetup( void );
static	void	MCSS_InitRenderer( MCSS_SYS_WORK *mcss_sys );
static NNSG2dMultiCellAnimation*     GetNewMultiCellAnim_( u16 num );

int	rot;

//--------------------------------------------------------------------------
/**
 * システム初期化
 */
//--------------------------------------------------------------------------
MCSS_SYS_WORK*	MCSS_Init( int max, GFL_G3D_CAMERA *camera,HEAPID heapID )
{
	MCSS_SYS_WORK	*mcss_sys;

	mcss_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(MCSS_SYS_WORK) );

	mcss_sys->max = max;
	mcss_sys->camera = camera;
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

#ifndef USE_RENDER
//独自描画
//--------------------------------------------------------------------------
/**
 * 描画システム
 */
//--------------------------------------------------------------------------
void	MCSS_Draw( MCSS_SYS_WORK *mcss_sys )
{
	MCSS_WORK					*mcss;
	MCSS_NCEC					*ncec;
    NNSG2dImageProxy			*image_p;
    NNSG2dImagePaletteProxy		*palette_p;
	int							index;
	int							cell;
	VecFx32						pos,scale;
	fx32						tex_s, tex_t, pos_z_default;
	NNSG2dAnimController		*anim_ctrl_mc;
	NNSG2dAnimDataSRT			*anim_SRT_mc;
	NNSG2dAnimController		*anim_ctrl_c;
	NNSG2dAnimDataT				*anim_T_p;
	NNSG2dAnimDataSRT			anim_SRT;
	NNSG2dMCNodeCellAnimArray	*MC_Array;
	MtxFx44						mtx_44,mc_mtx_44,rot_44,scale_44;
	MtxFx43						camera;
	MtxFx33						rotate;
	VecFx32						camPos,target;
	VecFx32						camPos_p,target_p;

	G3_PushMtx();

	G3_MtxMode( GX_MTXMODE_TEXTURE );
	G3_Identity();
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

	for( index = 0 ; index < mcss_sys->max ; index++ ){
		if( mcss_sys->mcss[index] != NULL ){

			mcss		= mcss_sys->mcss[index];
			image_p		= &mcss->mcss_image_proxy;
			palette_p	= &mcss->mcss_palette_proxy;
			anim_ctrl_mc= NNS_G2dGetMCAnimAnimCtrl(&mcss->mcss_mcanim);
			anim_SRT_mc	= anim_ctrl_mc->pCurrent->pContent;
			MC_Array = (NNSG2dMCNodeCellAnimArray*)&mcss->mcss_mcanim.multiCellInstance.pCellAnimInstasnces;
			pos_z_default = -1 << FX32_SHIFT;

			MCSS_MaterialSetup();
			//G3_MaterialColorDiffAmb(GX_RGB(31,31,31),GX_RGB(16,16,16),FALSE);
			//G3_MaterialColorSpecEmi(GX_RGB(16,16,16),GX_RGB(0,0,0),FALSE);
			//G3_PolygonAttr(GX_LIGHTMASK_NONE,GX_POLYGONMODE_MODULATE,GX_CULL_NONE,0,31,GX_POLYGON_ATTR_MISC_NONE);

			G3_TexImageParam(image_p->attr.fmt,
							 GX_TEXGEN_TEXCOORD,
							 image_p->attr.sizeS,
							 image_p->attr.sizeT,
							 GX_TEXREPEAT_ST,
							 GX_TEXFLIP_NONE,
							 image_p->attr.plttUse,
							 image_p->vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN]);
			G3_TexPlttBase(palette_p->vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN],
						   palette_p->fmt);
			for( cell = 0 ; cell < mcss->mcss_mcanim.pMultiCellDataBank[anim_SRT_mc->index].pMultiCellDataArray->numNodes ; cell++ ){
				anim_ctrl_c = NNS_G2dGetCellAnimAnimCtrl(&MC_Array->cellAnimArray[cell].cellAnim);
				switch( anim_ctrl_c->pAnimSequence->animType & 0xffff ){
				case NNS_G2D_ANIMELEMENT_INDEX:		// Index のみ
					anim_SRT.index	= *( (u16 *)anim_ctrl_c->pCurrent->pContent );
					anim_SRT.rotZ	= 0;
					anim_SRT.sx		= 0x1000;
					anim_SRT.sy		= 0x1000;
					anim_SRT.px		= 0;
					anim_SRT.py		= 0;
					break;
				case NNS_G2D_ANIMELEMENT_INDEX_SRT:	// Index + SRT 
					anim_SRT = *( (NNSG2dAnimDataSRT *)anim_ctrl_c->pCurrent->pContent );
					break;
				case NNS_G2D_ANIMELEMENT_INDEX_T:	// Index + T 
					anim_T_p		= anim_ctrl_c->pCurrent->pContent;
					anim_SRT.index	= anim_T_p->index;
					anim_SRT.rotZ	= 0;
					anim_SRT.sx		= 0x1000;
					anim_SRT.sy		= 0x1000;
					anim_SRT.px		= anim_T_p->px;
					anim_SRT.py		= anim_T_p->py;
					break;
				default:
					GF_ASSERT(0);
					break;
				}
				//if( anim_SRT.index != 2 ) continue;
				ncec = &mcss->mcss_ncec->ncec[anim_SRT.index];

				//セルデータから取得した位置で書き出し
				pos.x = FX32_CONST( ncec->pos_x );
				pos.y = FX32_CONST( ncec->pos_y );
				scale.x = FX32_CONST( ncec->size_x );
				scale.y = FX32_CONST( ncec->size_y );
				tex_s = FX32_CONST( ( ncec->char_no % ( ( 8 << image_p->attr.sizeS ) / 8) ) * 8 );
				tex_t = FX32_CONST( ( ncec->char_no / ( ( 8 << image_p->attr.sizeS ) / 8) ) * 8 );

				//メパチ実験
				if( mcss->mepachi_flag ){
					if( ( anim_SRT.index == 0 ) || ( anim_SRT.index == 1 )){
						tex_t += scale.y;
					}
				}

				MTX_Identity44(&mtx_44);
				MTX_TransApply44( &mtx_44, &mtx_44, pos.x, pos.y, 0 );
				MTX_RotZ44(&rot_44, -FX_SinIdx( anim_SRT.rotZ ), FX_CosIdx( anim_SRT.rotZ ) );
				MTX_Concat44(&mtx_44,&rot_44,&mtx_44);
				pos.x = FX32_CONST( anim_SRT.px );
				pos.y = -FX32_CONST( anim_SRT.py );
				MTX_Identity44(&mc_mtx_44);
				MTX_TransApply44( &mc_mtx_44, &mc_mtx_44, pos.x, pos.y, 0 );
				MTX_Concat44(&mtx_44,&mc_mtx_44,&mtx_44);
				MTX_Scale44( &scale_44, anim_SRT.sx, anim_SRT.sy, FX32_ONE );
				MTX_Concat44(&mtx_44,&scale_44,&mtx_44);

				switch( anim_ctrl_mc->pAnimSequence->animType & 0xffff ){
				case NNS_G2D_ANIMELEMENT_INDEX:		// Index のみ
					anim_SRT.index	= *( (u16 *)anim_ctrl_mc->pCurrent->pContent );
					anim_SRT.rotZ	= 0;
					anim_SRT.sx		= 0x1000;
					anim_SRT.sy		= 0x1000;
					anim_SRT.px		= 0;
					anim_SRT.py		= 0;
					break;
				case NNS_G2D_ANIMELEMENT_INDEX_SRT:	// Index + SRT 
					anim_SRT = *( (NNSG2dAnimDataSRT *)anim_ctrl_mc->pCurrent->pContent );
					break;
				case NNS_G2D_ANIMELEMENT_INDEX_T:	// Index + T 
					anim_T_p		= anim_ctrl_mc->pCurrent->pContent;
					anim_SRT.index	= anim_T_p->index;
					anim_SRT.rotZ	= 0;
					anim_SRT.sx		= 0x1000;
					anim_SRT.sy		= 0x1000;
					anim_SRT.px		= anim_T_p->px;
					anim_SRT.py		= anim_T_p->py;
					break;
				default:
					GF_ASSERT(0);
					break;
				}

				//ビルボード回転
				{
					MTX_Inverse43( NNS_G3dGlbGetCameraMtx(), &camera );	//カメラ逆行列取得
					MTX_Copy43To33( &camera, &rotate );		//カメラ逆行列から回転行列を取り出す
				}

				//マルチセルデータから取得した位置で書き出し
				pos.x = FX32_CONST( mcss->mcss_mcanim.pMultiCellDataBank[anim_SRT_mc->index].pHierarchyDataArray[cell].posX );
				pos.y = -FX32_CONST( mcss->mcss_mcanim.pMultiCellDataBank[anim_SRT_mc->index].pHierarchyDataArray[cell].posY );
				MTX_Identity44(&mc_mtx_44);
				MTX_TransApply44( &mc_mtx_44, &mc_mtx_44, pos.x, pos.y, 0 );
				MTX_Concat44(&mtx_44,&mc_mtx_44,&mtx_44);
				MTX_RotZ44(&rot_44, -FX_SinIdx( anim_SRT.rotZ ), FX_CosIdx( anim_SRT.rotZ ) );
				MTX_Concat44(&mtx_44,&rot_44,&mtx_44);

				pos.x = FX32_CONST( anim_SRT.px );
				pos.y = -FX32_CONST( anim_SRT.py );
				MTX_Identity44(&mc_mtx_44);
				MTX_TransApply44( &mc_mtx_44, &mc_mtx_44, pos.x, pos.y, 0 );
				MTX_Concat44(&mtx_44,&mc_mtx_44,&mtx_44);

				MTX_Scale44( &scale_44, anim_SRT.sx, anim_SRT.sy, FX32_ONE );
				MTX_Concat44(&mtx_44,&scale_44,&mtx_44);

//				rot += 2;
//				MTX_RotY44( &rot_44, FX_SinIdx( rot & 0xffff ), FX_CosIdx( rot & 0xffff ) );
				MTX_Copy33To44( &rotate, &rot_44 );
				MTX_Concat44(&mtx_44,&rot_44,&mtx_44);

				pos.x = mcss->pos.x;
				pos.y = mcss->pos.y;
				pos.z = pos_z_default + mcss->pos.z;
				MTX_Identity44(&mc_mtx_44);
				MTX_TransApply44( &mc_mtx_44, &mc_mtx_44, pos.x, pos.y, pos.z );
				MTX_Concat44(&mtx_44,&mc_mtx_44,&mtx_44);

				//カメラ行列を掛け合わせ
				G3_LoadMtx43( NNS_G3dGlbGetCameraMtx() );
				G3_MultMtx44( &mtx_44 );
				G3_Scale( scale.x, scale.y, FX32_ONE );
				G3_Begin(GX_BEGIN_QUADS);
				G3_TexCoord( tex_s,				tex_t );
				G3_Vtx( 0,			0,			0 );
				G3_TexCoord( tex_s + scale.x,	tex_t );
				G3_Vtx( FX32_ONE,	0,			0 );
				G3_TexCoord( tex_s + scale.x,	tex_t + scale.y );
				G3_Vtx( FX32_ONE,	-FX32_ONE,	0 );
				G3_TexCoord( tex_s,				tex_t + scale.y );
				G3_Vtx( 0,			-FX32_ONE,	0 );
				G3_End();
				pos_z_default -= FX32_HALF / 2;
			}
		}
	}
	G3_PopMtx(1);
}
#else
//レンダラシステムを用いた描画
//--------------------------------------------------------------------------
/**
 * 描画システム
 */
//--------------------------------------------------------------------------
void	MCSS_Draw( MCSS_SYS_WORK *mcss_sys )
{
	MCSS_WORK	*mcss;
	int			index;
	int			cell;
	VecFx32		pos,scale;

	NNS_G2dSetupSoftwareSpriteCamera();
	G3_MtxMode( GX_MTXMODE_TEXTURE );
	G3_Identity();
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

	for( index = 0 ; index < mcss_sys->max ; index++ ){
		if( mcss_sys->mcss[index] != NULL ){
			mcss=mcss_sys->mcss[index];
			//暫定でRendererを使用しているが、実際は独自で描画処理を作成しなければならない
			NNS_G2dBeginRendering( &mcss_sys->mcss_render );
			NNS_G2dPushMtx();
			NNS_G2dTranslate( mcss->pos.x,
							  mcss->pos.y,
							  mcss->pos.z);
			NNS_G2dSetRendererImageProxy( &mcss_sys->mcss_render,
										  &mcss->mcss_image_proxy,
										  &mcss->mcss_palette_proxy );
			NNS_G2dDrawMultiCellAnimation( &mcss->mcss_mcanim );
			NNS_G2dPopMtx();
			NNS_G2dEndRendering();
		}
	}
}
#endif

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
					  ARCDATID		nmar,
					  ARCDATID		ncec )
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
							   nmar,
							   ncec );
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
								   ARCDATID			nmar,
								   ARCDATID			ncec )
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
		MCSS_GetNewMultiCellAnimation( mcss, NNS_G2D_MCTYPE_SHARE_CELLANIM);

		// マルチセルアニメーションに再生するシーケンスをセット
		NNS_G2dSetAnimSequenceToMCAnimation( &mcss->mcss_mcanim, pSequence);
	}

	//1枚の板ポリで表示するための情報の読み込み（独自フォーマット）
	mcss->mcss_ncec = GFL_ARC_LoadDataAlloc(arcID, ncec, mcss->heapID);

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

#if 0
	//アニメーションフレーム表示実験
	NNSG2dAnimController		*anim_ctrl_mc;
	NNSG2dAnimDataSRT			*anim_SRT_mc;
	NNSG2dAnimController		*anim_ctrl_c;
	NNSG2dAnimDataSRT			*anim_SRT_c;
	NNSG2dAnimDataT				*anim_T_c;
	NNSG2dAnimDataSRT			anim_SRT;
	int							mcell_num;
	int							cell_anm_index;
	NNSG2dMCNodeCellAnimArray	*MC_Array;
    NNSG2dCellAnimation			*cellAnim;

	anim_ctrl_mc=NNS_G2dGetMCAnimAnimCtrl(&mcss_sys->mcss[0]->mcss_mcanim);
	anim_SRT_mc=anim_ctrl_mc->pCurrent->pContent;
//	OS_TPrintf("index:%d rotZ:%d sx:%d sy:%d px:%d py:%d\n",anim_SRT_mc->index,
//															anim_SRT_mc->rotZ,
//															anim_SRT_mc->sx,
//															anim_SRT_mc->sy,
//															anim_SRT_mc->px,
//															anim_SRT_mc->py);

	for( mcell_num = 0 ; mcell_num < mcss_sys->mcss[0]->mcss_mcanim.pMultiCellDataBank[anim_SRT_mc->index].pMultiCellDataArray->numNodes ; mcell_num++ ){
		cell_anm_index = mcss_sys->mcss[0]->mcss_mcanim.pMultiCellDataBank[anim_SRT_mc->index].pHierarchyDataArray[mcell_num].animSequenceIdx;
		//OS_TPrintf("cell_anm_index:%d\n",cell_anm_index);
		MC_Array = (NNSG2dMCNodeCellAnimArray*)&mcss_sys->mcss[0]->mcss_mcanim.multiCellInstance.pCellAnimInstasnces;
		anim_ctrl_c = NNS_G2dGetCellAnimAnimCtrl(&MC_Array->cellAnimArray[mcell_num].cellAnim);
		switch(anim_ctrl_c->pAnimSequence->animType&0xffff){
		case NNS_G2D_ANIMELEMENT_INDEX:		// Index のみ
			anim_SRT.index=*((u16 *)anim_ctrl_c->pCurrent->pContent);
			anim_SRT.rotZ=0;
			anim_SRT.sx=0x1000;
			anim_SRT.sy=0x1000;
			anim_SRT.px=0;
			anim_SRT.py=0;
			break;
		case NNS_G2D_ANIMELEMENT_INDEX_SRT:	// Index + SRT 
			anim_SRT_c=anim_ctrl_c->pCurrent->pContent;
			anim_SRT=*anim_SRT_c;
			break;
		case NNS_G2D_ANIMELEMENT_INDEX_T:	// Index + T 
			anim_T_c=anim_ctrl_c->pCurrent->pContent;
			anim_SRT.index=anim_T_c->index;
			anim_SRT.rotZ=anim_T_c->pad_;
			anim_SRT.sx=0x1000;
			anim_SRT.sy=0x1000;
			anim_SRT.px=anim_T_c->px;
			anim_SRT.py=anim_T_c->py;
			break;
		default:
			GF_ASSERT(0);
			break;
		}
		OS_TPrintf("index:%d rotZ:%d sx:%d sy:%d px:%d py:%d\n",anim_SRT.index,
																anim_SRT.rotZ,
																anim_SRT.sx,
																anim_SRT.sy,
																anim_SRT.px,
																anim_SRT.py);
	}
#endif
