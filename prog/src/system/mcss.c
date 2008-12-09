
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

//#define	USE_RENDER		//有効にすることでNNSのレンダラを使用して描画する
#include "system/mcss.h"	//内部でUSE_RENDERを参照しているのでここより上に移動は不可
#include "mcss_def.h"

#define	MCSS_DEFAULT_SHIFT	( FX32_SHIFT - 4 )		//ポリゴン１辺の基準の長さにするシフト値
#define	MCSS_DEFAULT_LINE	( 1 << MCSS_DEFAULT_SHIFT )	//ポリゴン１辺の基準の長さ
#define	MCSS_DEFAULT_Z		( 1 << 6 )
#define	MCSS_TEX_OFS		( FX32_ONE >> 4 )

#define	MCSS_CONST(x)	( x << MCSS_DEFAULT_SHIFT )

#define	MCSS_TEX_ADRS	(0x8000)
#define	MCSS_TEX_SIZE	(0x4000)
#define	MCSS_PAL_ADRS	(0x0100)
#define	MCSS_PAL_SIZE	(0x0020)

//--------------------------------------------------------------------------
/**
 * 構造体宣言
 */
//--------------------------------------------------------------------------
typedef struct	
{
	NNSG2dCharacterData*	pCharData;			//テクスチャキャラ
	NNSG2dPaletteData*		pPlttData;			//テクスチャパレット
	void					*pBufChar;			//テクスチャキャラバッファ
	void					*pBufPltt;			//テクスチャパレットバッファ
	int						chr_ofs;
	int						pal_ofs;
	MCSS_WORK				*mcss;
}TCB_LOADRESOURCE_WORK;

//--------------------------------------------------------------------------
/**
 * プロトタイプ宣言
 */
//--------------------------------------------------------------------------
MCSS_SYS_WORK*	MCSS_Init( int max, GFL_TCBSYS *mcss_tcb_sys, HEAPID heapID );
void			MCSS_Exit( MCSS_SYS_WORK *mcss_sys );
void			MCSS_Main( MCSS_SYS_WORK *mcss_sys );
void			MCSS_Draw( MCSS_SYS_WORK *mcss_sys );
MCSS_WORK*		MCSS_Add( MCSS_SYS_WORK *mcss_sys, fx32	pos_x, fx32	pos_y, fx32	pos_z, MCSS_ADD_WORK *maw );
void			MCSS_Del( MCSS_SYS_WORK *mcss_sys, MCSS_WORK *mcss );

void			MCSS_GetPosition( MCSS_WORK *mcss, VecFx32 *pos );
void			MCSS_SetPosition( MCSS_WORK *mcss, VecFx32 *pos );
void			MCSS_GetScale( MCSS_WORK *mcss, VecFx32 *scale );
void			MCSS_SetScale( MCSS_WORK *mcss, VecFx32 *scale );
void			MCSS_SetMepachiFlag( MCSS_WORK *mcss );
void			MCSS_ResetMepachiFlag( MCSS_WORK *mcss );
void			MCSS_SetAnmStopFlag( MCSS_WORK *mcss );
void			MCSS_ResetAnmStopFlag( MCSS_WORK *mcss );
void			MCSS_SetVanishFlag( MCSS_WORK *mcss );
void			MCSS_ResetVanishFlag( MCSS_WORK *mcss );

static	void	MCSS_DrawAct( MCSS_WORK *mcss, 
							  MtxFx44 inv_camera,
							  MtxFx44 trans,
							  MtxFx44 mc_mtx,
							  fx32 pos_x,
							  fx32 pos_y,
							  fx32 scale_x,
							  fx32 scale_y,
							  fx32 tex_s,
							  fx32 tex_t,
							  NNSG2dAnimDataSRT *anim_SRT_c,
							  NNSG2dAnimDataSRT *anim_SRT_mc,
							  int cell,
							  fx32 *pos_z_default );

static	void	MCSS_LoadResource( MCSS_SYS_WORK *mcss_sys, int count, MCSS_ADD_WORK *maw );
static	void	MCSS_GetNewMultiCellAnimation(MCSS_WORK *mcss, NNSG2dMCType	mcType );
static	void	MCSS_MaterialSetup( void );
static NNSG2dMultiCellAnimation*     GetNewMultiCellAnim_( u16 num );

static	void	TCB_LoadResource( GFL_TCB *tcb, void *work );

#ifdef USE_RENDER
static	void	MCSS_InitRenderer( MCSS_SYS_WORK *mcss_sys );
#endif //USE_RENDER

//--------------------------------------------------------------------------
/**
 * システム初期化
 */
//--------------------------------------------------------------------------
MCSS_SYS_WORK*	MCSS_Init( int max, GFL_TCBSYS *mcss_tcb_sys,HEAPID heapID )
{
	MCSS_SYS_WORK	*mcss_sys;

	mcss_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(MCSS_SYS_WORK) );

	mcss_sys->mcss_max		= max;
	mcss_sys->mcss_tcb_sys	= mcss_tcb_sys;
	mcss_sys->heapID		= heapID;

	mcss_sys->mcss =GFL_HEAP_AllocClearMemory( heapID, sizeof(MCSS_WORK *) * max );

#ifdef USE_RENDER
	MCSS_InitRenderer( mcss_sys );
#endif //USE_RENDER

	//テクスチャ系の転送先アドレスをセット（可変に出来る予定）
#warning MCSS TEX PAL ADRS Not Changeability
	mcss_sys->texAdrs = MCSS_TEX_ADRS;
	mcss_sys->palAdrs = MCSS_PAL_ADRS;
	
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

	for( i = 0 ; i < mcss_sys->mcss_max ; i++ ){
		if( mcss_sys->mcss[i] != NULL ){
			MCSS_Del( mcss_sys, mcss_sys->mcss[i] );
		}
	}
	GFL_HEAP_FreeMemory( mcss_sys->mcss );
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

	for( index = 0 ; index < mcss_sys->mcss_max ; index++ ){
		if( ( mcss_sys->mcss[index] != NULL ) && ( mcss_sys->mcss[index]->anm_stop_flag == 0 ) ){
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
	NNSG2dAnimDataSRT			anim_SRT_mc;
	NNSG2dAnimController		*anim_ctrl_c;
	NNSG2dAnimDataT				*anim_T_p;
	NNSG2dAnimDataSRT			anim_SRT;
	NNSG2dMCNodeCellAnimArray	*MC_Array;
	VecFx32						camPos,target;
	VecFx32						camPos_p,target_p;
	MtxFx44						inv_camera,trans,mc_mtx;

	G3_PushMtx();

	G3_MtxMode( GX_MTXMODE_TEXTURE );
	G3_Identity();
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

	G3_LookAt( NNS_G3dGlbGetCameraPos(), NNS_G3dGlbGetCameraUp(), NNS_G3dGlbGetCameraTarget(), NULL );


	//ビルボード回転行列を求める
	{
		MtxFx43						camera;
		MtxFx33						rotate;

		MTX_Inverse43( NNS_G3dGlbGetCameraMtx(), &camera );	//カメラ逆行列取得
		MTX_Copy43To33( &camera, &rotate );		//カメラ逆行列から回転行列を取り出す
		MTX_Copy33To44( &rotate, &inv_camera );
	}

	for( index = 0 ; index < mcss_sys->mcss_max ; index++ ){
		if( ( mcss_sys->mcss[index] != NULL ) && ( mcss_sys->mcss[index]->vanish_flag == 0 ) ){

			G3_PushMtx();

			mcss		= mcss_sys->mcss[index];
			image_p		= &mcss->mcss_image_proxy;
			palette_p	= &mcss->mcss_palette_proxy;
			anim_ctrl_mc= NNS_G2dGetMCAnimAnimCtrl(&mcss->mcss_mcanim);
			MC_Array = (NNSG2dMCNodeCellAnimArray*)&mcss->mcss_mcanim.multiCellInstance.pCellAnimInstasnces;
			pos_z_default = 0;

			//平行移動行列を作成しておく
			MTX_Identity44( &trans );
			MTX_TransApply44( &trans, &trans, mcss->pos.x, mcss->pos.y, mcss->pos.z );


			switch( anim_ctrl_mc->pAnimSequence->animType & 0xffff ){
			case NNS_G2D_ANIMELEMENT_INDEX:		// Index のみ
				anim_SRT_mc.index	= *( (u16 *)anim_ctrl_mc->pCurrent->pContent );
				anim_SRT_mc.rotZ	= 0;
				anim_SRT_mc.sx		= 0x1000;
				anim_SRT_mc.sy		= 0x1000;
				anim_SRT_mc.px		= 0;
				anim_SRT_mc.py		= 0;
				break;
			case NNS_G2D_ANIMELEMENT_INDEX_SRT:	// Index + SRT 
				anim_SRT_mc = *( (NNSG2dAnimDataSRT *)anim_ctrl_mc->pCurrent->pContent );
				break;
			case NNS_G2D_ANIMELEMENT_INDEX_T:	// Index + T 
				anim_T_p			= anim_ctrl_mc->pCurrent->pContent;
				anim_SRT_mc.index	= anim_T_p->index;
				anim_SRT_mc.rotZ	= 0;
				anim_SRT_mc.sx		= 0x1000;
				anim_SRT_mc.sy		= 0x1000;
				anim_SRT_mc.px		= anim_T_p->px;
				anim_SRT_mc.py		= anim_T_p->py;
				break;
			default:
				GF_ASSERT(0);
				break;
			}

			{
				MtxFx44	scale,trans;

				//マルチセルのアニメーションデータから行列を作成しておく
				MTX_Scale44( &scale, FX_Mul( anim_SRT_mc.sx, mcss->scale.x ), FX_Mul( anim_SRT_mc.sy, mcss->scale.y ), FX32_ONE );
				MTX_RotZ44( &mc_mtx, -FX_SinIdx( anim_SRT_mc.rotZ ), FX_CosIdx( anim_SRT_mc.rotZ ) );
				MTX_Concat44( &scale, &mc_mtx, &mc_mtx );
				pos.x = MCSS_CONST( anim_SRT_mc.px );
				pos.y = MCSS_CONST( -anim_SRT_mc.py );
				MTX_Identity44( &trans );
				MTX_TransApply44( &trans, &trans, pos.x, pos.y, 0 );
				MTX_Concat44( &mc_mtx, &trans, &mc_mtx );
			}

			//前もって、普遍なマルチセルデータをカレント行列にかけておく
			G3_MultMtx44( &trans );
			//カメラの逆行列を掛け合わせる（ビルボード処理）
			G3_MultMtx44( &inv_camera );
			G3_MultMtx44( &mc_mtx );

			MCSS_MaterialSetup();

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
			for( cell = 0 ; cell < mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc.index].numNodes ; cell++ ){
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
				ncec = &mcss->mcss_ncec->ncec[anim_SRT.index];

				//メパチ処理
				if( ncec->mepachi_size_x ){
					if( mcss->mepachi_flag ){
						MCSS_DrawAct( mcss, 
									  inv_camera,
									  trans,
									  mc_mtx,
									  ncec->mepachi_pos_x,
									  ncec->mepachi_pos_y,
									  ncec->mepachi_size_x,
									  ncec->mepachi_size_y,
									  ncec->mepachi_tex_s,
									  ncec->mepachi_tex_t + ncec->mepachi_size_y,
									  &anim_SRT, &anim_SRT_mc, cell, &pos_z_default );
					}
					else{
						MCSS_DrawAct( mcss, 
									  inv_camera,
									  trans,
									  mc_mtx,
									  ncec->mepachi_pos_x,
									  ncec->mepachi_pos_y,
									  ncec->mepachi_size_x,
									  ncec->mepachi_size_y,
									  ncec->mepachi_tex_s,
									  ncec->mepachi_tex_t,
									  &anim_SRT, &anim_SRT_mc, cell, &pos_z_default );
					}
				}
				MCSS_DrawAct( mcss, 
							  inv_camera,
							  trans,
							  mc_mtx,
							  ncec->pos_x,
							  ncec->pos_y,
							  ncec->size_x,
							  ncec->size_y,
							  ncec->tex_s,
							  ncec->tex_t,
							  &anim_SRT, &anim_SRT_mc, cell, &pos_z_default );
			}
			G3_PopMtx(1);
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

	for( index = 0 ; index < mcss_sys->mcss_max ; index++ ){
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
 * マルチセルを描画
 */
//--------------------------------------------------------------------------
static	void	MCSS_DrawAct( MCSS_WORK *mcss,
							  MtxFx44 inv_camera,
							  MtxFx44 trans,
							  MtxFx44 mc_mtx,
							  fx32 pos_x,
							  fx32 pos_y,
							  fx32 scale_x,
							  fx32 scale_y,
							  fx32 tex_s,
							  fx32 tex_t,
							  NNSG2dAnimDataSRT *anim_SRT_c,
							  NNSG2dAnimDataSRT *anim_SRT_mc,
							  int cell,
							  fx32 *pos_z_default )
{
	VecFx32						pos;
	MtxFx44						mtx_44,mc_mtx_44,rot_44,scale_44;
	NNSG2dAnimDataSRT			anim_SRT;
	NNSG2dAnimDataT				*anim_T_p;
	static	const	MtxFx44		base_mtx = { FX32_ONE, 0, 0, 0,
											 0,	FX32_ONE, 0, 0,
											 0,	0, FX32_ONE, 0,
											 0,	0, 0, FX32_ONE };

	G3_PushMtx();

	//マルチセルデータから取得した位置で書き出し
	pos.x = MCSS_CONST( mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc->index].pHierDataArray[cell].posX );
	pos.y = MCSS_CONST( -mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc->index].pHierDataArray[cell].posY );
	G3_Translate( pos.x, pos.y, 0 );

	pos.x = MCSS_CONST( anim_SRT_c->px );
	pos.y = MCSS_CONST( -anim_SRT_c->py );
	G3_Translate( pos.x, pos.y, *pos_z_default );

	G3_RotZ( -FX_SinIdx( anim_SRT_c->rotZ ), FX_CosIdx( anim_SRT_c->rotZ ) );

	G3_Scale( anim_SRT_c->sx, anim_SRT_c->sy, FX32_ONE );

	G3_Translate( pos_x, pos_y, 0 );

	G3_Scale( scale_x, scale_y, FX32_ONE );

	G3_Begin(GX_BEGIN_QUADS);
	G3_TexCoord( tex_s,				tex_t );
	G3_Vtx( 0, 0, 0 );
	G3_TexCoord( tex_s + scale_x - MCSS_TEX_OFS,	tex_t );
	G3_Vtx( MCSS_DEFAULT_LINE, 0, 0 );
	G3_TexCoord( tex_s + scale_x - MCSS_TEX_OFS,	tex_t + scale_y - MCSS_TEX_OFS );
	G3_Vtx( MCSS_DEFAULT_LINE, -MCSS_DEFAULT_LINE, 0 );
	G3_TexCoord( tex_s,				tex_t + scale_y - MCSS_TEX_OFS );
	G3_Vtx( 0, -MCSS_DEFAULT_LINE, 0 );
	G3_End();
	*pos_z_default -= MCSS_DEFAULT_Z;

	G3_PopMtx(1);
}

//--------------------------------------------------------------------------
/**
 * マルチセル登録
 */
//--------------------------------------------------------------------------
MCSS_WORK*	MCSS_Add( MCSS_SYS_WORK *mcss_sys, fx32	pos_x, fx32	pos_y, fx32	pos_z, MCSS_ADD_WORK *maw )
{
	int			count;

	for( count = 0 ; count < mcss_sys->mcss_max ; count++ ){
		if( mcss_sys->mcss[ count ] == NULL ){
			mcss_sys->mcss[ count ] = GFL_HEAP_AllocClearMemory( mcss_sys->heapID, sizeof(MCSS_WORK) );
			mcss_sys->mcss[ count ]->index = count;
			mcss_sys->mcss[ count ]->heapID = mcss_sys->heapID;
			mcss_sys->mcss[ count ]->pos.x = pos_x;
			mcss_sys->mcss[ count ]->pos.y = pos_y;
			mcss_sys->mcss[ count ]->pos.z = pos_z;
			mcss_sys->mcss[ count ]->scale.x = FX32_ONE;
			mcss_sys->mcss[ count ]->scale.y = FX32_ONE;
			mcss_sys->mcss[ count ]->scale.z = FX32_ONE;
			MCSS_LoadResource( mcss_sys, count, maw );
			break;
		}
	}
	//登録MAX値をオーバー
	GF_ASSERT( count < mcss_sys->mcss_max );

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
	GFL_HEAP_FreeMemory( mcss->mcss_ncec );

	mcss_sys->mcss[ mcss->index ] = NULL;
	GFL_HEAP_FreeMemory( mcss );
}

//--------------------------------------------------------------------------
/**
 * ポジションゲット
 */
//--------------------------------------------------------------------------
void	MCSS_GetPosition( MCSS_WORK *mcss, VecFx32 *pos )
{
	pos->x = mcss->pos.x;
	pos->y = mcss->pos.y;
	pos->z = mcss->pos.z;
}

//--------------------------------------------------------------------------
/**
 * ポジションセット
 */
//--------------------------------------------------------------------------
void	MCSS_SetPosition( MCSS_WORK *mcss, VecFx32 *pos )
{
	mcss->pos.x = pos->x;
	mcss->pos.y = pos->y;
	mcss->pos.z = pos->z;
}

//--------------------------------------------------------------------------
/**
 * スケールゲット
 */
//--------------------------------------------------------------------------
void	MCSS_GetScale( MCSS_WORK *mcss, VecFx32 *scale )
{
	scale->x = mcss->scale.x;
	scale->y = mcss->scale.y;
	scale->z = mcss->scale.z;
}

//--------------------------------------------------------------------------
/**
 * スケールセット
 */
//--------------------------------------------------------------------------
void	MCSS_SetScale( MCSS_WORK *mcss, VecFx32 *scale )
{
	mcss->scale.x = scale->x;
	mcss->scale.y = scale->y;
	mcss->scale.z = scale->z;
}

//--------------------------------------------------------------------------
/**
 * メパチフラグセット
 */
//--------------------------------------------------------------------------
void	MCSS_SetMepachiFlag( MCSS_WORK *mcss )
{
	mcss->mepachi_flag = MCSS_MEPACHI_ON;
}

//--------------------------------------------------------------------------
/**
 * メパチフラグリセット
 */
//--------------------------------------------------------------------------
void	MCSS_ResetMepachiFlag( MCSS_WORK *mcss )
{
	mcss->mepachi_flag = MCSS_MEPACHI_OFF;
}

//--------------------------------------------------------------------------
/**
 * アニメストップフラグセット
 */
//--------------------------------------------------------------------------
void	MCSS_SetAnmStopFlag( MCSS_WORK *mcss )
{
	mcss->anm_stop_flag = MCSS_ANM_STOP_ON;
}

//--------------------------------------------------------------------------
/**
 * アニメストップフラグリセット
 */
//--------------------------------------------------------------------------
void	MCSS_ResetAnmStopFlag( MCSS_WORK *mcss )
{
	mcss->anm_stop_flag = MCSS_ANM_STOP_OFF;
}

//--------------------------------------------------------------------------
/**
 * バニッシュフラグセット
 */
//--------------------------------------------------------------------------
void	MCSS_SetVanishFlag( MCSS_WORK *mcss )
{
	mcss->vanish_flag = MCSS_VANISH_ON;
}

//--------------------------------------------------------------------------
/**
 * バニッシュフラグリセット
 */
//--------------------------------------------------------------------------
void	MCSS_ResetVanishFlag( MCSS_WORK *mcss )
{
	mcss->vanish_flag = MCSS_VANISH_OFF;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
/**
 * リソースロード
 */
//--------------------------------------------------------------------------
static	void	MCSS_LoadResource( MCSS_SYS_WORK *mcss_sys, int count, MCSS_ADD_WORK *maw )
{
	MCSS_WORK	*mcss = mcss_sys->mcss[ count ];

	mcss->vanish_flag = 1;

	//プロキシ初期化
	NNS_G2dInitImageProxy( &mcss->mcss_image_proxy );
	NNS_G2dInitImagePaletteProxy( &mcss->mcss_palette_proxy );

	// セルデータ、セルアニメーション、マルチセルデータ、
	// マルチセルアニメーションをロード。
	mcss->mcss_ncer_buf = GFL_ARC_UTIL_LoadCellBank(		maw->arcID, maw->ncer, FALSE, &mcss->mcss_ncer, mcss->heapID );
	GF_ASSERT( mcss->mcss_ncer_buf != NULL );
	mcss->mcss_nanr_buf = GFL_ARC_UTIL_LoadAnimeBank(		maw->arcID, maw->nanr, FALSE, &mcss->mcss_nanr, mcss->heapID );
	GF_ASSERT( mcss->mcss_nanr_buf != NULL );
	mcss->mcss_nmcr_buf = GFL_ARC_UTIL_LoadMultiCellBank(	maw->arcID, maw->nmcr, FALSE, &mcss->mcss_nmcr, mcss->heapID );
	GF_ASSERT( mcss->mcss_nmcr_buf != NULL );
	mcss->mcss_nmar_buf = GFL_ARC_UTIL_LoadMultiAnimeBank(	maw->arcID, maw->nmar, FALSE, &mcss->mcss_nmar, mcss->heapID );
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
		MCSS_GetNewMultiCellAnimation( mcss, NNS_G2D_MCTYPE_SHARE_CELLANIM );

		// マルチセルアニメーションに再生するシーケンスをセット
		NNS_G2dSetAnimSequenceToMCAnimation( &mcss->mcss_mcanim, pSequence );
	}

	//1枚の板ポリで表示するための情報の読み込み（独自フォーマット）
	mcss->mcss_ncec = GFL_ARC_LoadDataAlloc( maw->arcID, maw->ncec, mcss->heapID );

    //
    // VRAM 関連の初期化
    //
    {
		TCB_LOADRESOURCE_WORK *tlw = GFL_HEAP_AllocClearMemory( mcss->heapID, sizeof( TCB_LOADRESOURCE_WORK ) );
		tlw->chr_ofs = mcss_sys->texAdrs + MCSS_TEX_SIZE * count;
		tlw->pal_ofs = mcss_sys->palAdrs + MCSS_PAL_SIZE * count;
		tlw->mcss	 = mcss;
		// load character data for 3D (software sprite)
		{
			tlw->pBufChar = GFL_ARC_UTIL_LoadBGCharacter( maw->arcID, maw->ncbr, FALSE, &tlw->pCharData, mcss->heapID );
			GF_ASSERT( tlw->pBufChar != NULL);
        }

		// load palette data
		{
			tlw->pBufPltt = GFL_ARC_UTIL_LoadPalette( maw->arcID, maw->nclr, &tlw->pPlttData, mcss->heapID );
			GF_ASSERT( tlw->pBufPltt != NULL);

        }
		GFL_TCB_AddTask( mcss_sys->mcss_tcb_sys, TCB_LoadResource, tlw, 0 );
    }
}

//--------------------------------------------------------------------------
/**
 * リソースをVRAMに転送
 */
//--------------------------------------------------------------------------
static	void	TCB_LoadResource( GFL_TCB *tcb, void *work )
{
	TCB_LOADRESOURCE_WORK *tlw = ( TCB_LOADRESOURCE_WORK *)work;

	tlw->mcss->vanish_flag = 0;

	// Loading For 3D Graphics Engine.（本来は、VRAMマネージャを使用したい）
	NNS_G2dLoadImage2DMapping(
		tlw->pCharData,
		tlw->chr_ofs,
		NNS_G2D_VRAM_TYPE_3DMAIN,
		&tlw->mcss->mcss_image_proxy );

	GFL_HEAP_FreeMemory( tlw->pBufChar );

	// Loading For 3D Graphics Engine.
	NNS_G2dLoadPalette(
		tlw->pPlttData,
		tlw->pal_ofs,
		NNS_G2D_VRAM_TYPE_3DMAIN,
		&tlw->mcss->mcss_palette_proxy );

	GFL_HEAP_FreeMemory( tlw->pBufPltt );

	GFL_HEAP_FreeMemory( work );
	GFL_TCB_DeleteTask( tcb );
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

#ifdef USE_RENDER
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
#endif //USE_RENDER

