//============================================================================================
/**
 * @file	musical_mcss.c
 * @brief	ミュージカル特化マルチセルソフトウエアスプライト描画用関数
 * @author	ariizumi
 * @date	2008.02.27
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>

#include "arc_def.h"

#define		USE_SHADOW (0)
//#define	USE_RENDER		//有効にすることでNNSのレンダラを使用して描画する
#include "musical_mcss.h"	//内部でUSE_RENDERを参照しているのでここより上に移動は不可
#include "musical_mcss_def.h"
#include "musical_local.h"
#include "test/ariizumi/ari_debug.h"
#include "musical/musical_camera_def.h"

#define	MUS_MCSS_DEFAULT_SHIFT		( FX32_SHIFT - 4 )		//ポリゴン１辺の基準の長さにするシフト値
#define	MUS_MCSS_DEFAULT_LINE		( 1 << MUS_MCSS_DEFAULT_SHIFT )	//ポリゴン１辺の基準の長さ
#define	MUS_MCSS_DEFAULT_Z			( 1 << 8 )
#define	MUS_MCSS_DEFAULT_Z_ORTHO	( 1 << 10 )

#define	MUS_MCSS_CONST(x)	( x << MUS_MCSS_DEFAULT_SHIFT )

#define	MUS_MCSS_TEX_ADRS	(0x30000) //4体でVRAM2面使用時用 適宜変えること！！
#define	MUS_MCSS_TEX_SIZE	(0x4000)
#define	MUS_MCSS_PAL_ADRS	(0x1000)
#define	MUS_MCSS_PAL_SIZE	(0x0020)

#define	MUS_MCSS_NORMAL_MTX	( 29 )
#define	MUS_MCSS_SHADOW_MTX	( 30 )

#define	SHADOW_OFFSET	( -0x1000 )	//影位置のZ方向のオフセット値

#define MUS_MCSS_FLIP_NONE	(0)
#define MUS_MCSS_FLIP_X		(1)
#define MUS_MCSS_FLIP_Y		(2)

//--------------------------------------------------------------------------
/**
 * 構造体宣言
 */
//--------------------------------------------------------------------------
typedef struct	
{
	NNSG2dCharacterData*	pCharData;			//テクスチャキャラ
	NNSG2dPaletteData*		pPlttData;			//テクスチャパレット
    NNSG2dImageProxy		*image_p;
    NNSG2dImagePaletteProxy	*palette_p;
	void					*pBufChar;			//テクスチャキャラバッファ
	void					*pBufPltt;			//テクスチャパレットバッファ
	int						chr_ofs;
	int						pal_ofs;
	MUS_MCSS_WORK				*mcss;
}TCB_LOADRESOURCE_WORK;

//--------------------------------------------------------------------------
/**
 * プロトタイプ宣言
 */
//--------------------------------------------------------------------------
MUS_MCSS_SYS_WORK*	MUS_MCSS_Init( int max, HEAPID heapID );
void			MUS_MCSS_Exit( MUS_MCSS_SYS_WORK *mcss_sys );
void			MUS_MCSS_Main( MUS_MCSS_SYS_WORK *mcss_sys );
void			MUS_MCSS_Draw( MUS_MCSS_SYS_WORK *mcss_sys , MusicalCellCallBack musCellCb );
MUS_MCSS_WORK*		MUS_MCSS_Add( MUS_MCSS_SYS_WORK *mcss_sys, fx32	pos_x, fx32	pos_y, fx32	pos_z, MUS_MCSS_ADD_WORK *maw , void *work , const BOOL isVBlank );
void			MUS_MCSS_Del( MUS_MCSS_SYS_WORK *mcss_sys, MUS_MCSS_WORK *mcss );

void			MUS_MCSS_SetOrthoMode( MUS_MCSS_SYS_WORK *mcss_sys );
void			MUS_MCSS_ResetOrthoMode( MUS_MCSS_SYS_WORK *mcss_sys );

void			MUS_MCSS_GetPosition( MUS_MCSS_WORK *mcss, VecFx32 *pos );
void			MUS_MCSS_SetPosition( MUS_MCSS_WORK *mcss, VecFx32 *pos );
void			MUS_MCSS_GetScale( MUS_MCSS_WORK *mcss, VecFx32 *scale );
void			MUS_MCSS_SetScale( MUS_MCSS_WORK *mcss, VecFx32 *scale );
void			MUS_MCSS_GetRotation( MUS_MCSS_WORK *mcss, u16 *rot );
void			MUS_MCSS_SetRotation( MUS_MCSS_WORK *mcss, u16 rot );
void			MUS_MCSS_SetShadowScale( MUS_MCSS_WORK *mcss, VecFx32 *scale );
void			MUS_MCSS_SetMepachiFlag( MUS_MCSS_WORK *mcss );
void			MUS_MCSS_ResetMepachiFlag( MUS_MCSS_WORK *mcss );
void			MUS_MCSS_SetAnmStopFlag( MUS_MCSS_WORK *mcss );
void			MUS_MCSS_ResetAnmStopFlag( MUS_MCSS_WORK *mcss );
void			MUS_MCSS_ChangeAnm( MUS_MCSS_WORK *mcss , const u8 anmIdx );
int				MUS_MCSS_GetVanishFlag( MUS_MCSS_WORK *mcss );
void			MUS_MCSS_SetVanishFlag( MUS_MCSS_WORK *mcss );
void			MUS_MCSS_ResetVanishFlag( MUS_MCSS_WORK *mcss );

static	void	MUS_MCSS_DrawAct( MUS_MCSS_WORK *mcss, 
							  fx32 pos_x,
							  fx32 pos_y,
							  fx32 scale_x,
							  fx32 scale_y,
							  fx32 tex_s,
							  fx32 tex_t,
							  NNSG2dAnimDataSRT *anim_SRT_c,
							  NNSG2dAnimDataSRT *anim_SRT_mc,
							  int node,
							  u32 mcss_ortho_mode,
							  fx32 *pos_z_default ,
							  const u8 isFlip );

static	void	MUS_MCSS_LoadResource( MUS_MCSS_SYS_WORK *mcss_sys, int count, MUS_MCSS_ADD_WORK *maw , const BOOL isVBlank );
static	void	MUS_MCSS_GetNewMultiCellAnimation(MUS_MCSS_WORK *mcss, NNSG2dMCType	mcType );
static	void	MUS_MCSS_MaterialSetup( void );
static NNSG2dMultiCellAnimation*     GetNewMultiCellAnim_( u16 num );

static	void	TCB_LoadResource( GFL_TCB *tcb, void *work );

#ifdef USE_RENDER
static	void	MUS_MCSS_InitRenderer( MUS_MCSS_SYS_WORK *mcss_sys );
#endif //USE_RENDER

static	void MTX_MultVec44( const VecFx32 *cp_src, const MtxFx44 *cp_m, VecFx32 *p_dst, fx32 *p_w );

//影実験
#if USE_SHADOW
NNSG2dImagePaletteProxy		mus_shadow_palette;
#endif //USE_SHADOW

//--------------------------------------------------------------------------
/**
 * システム初期化
 */
//--------------------------------------------------------------------------
MUS_MCSS_SYS_WORK*	MUS_MCSS_Init( int max, HEAPID heapID )
{
	MUS_MCSS_SYS_WORK	*mcss_sys;

	mcss_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(MUS_MCSS_SYS_WORK) );

	mcss_sys->mcss_max		= max;
	mcss_sys->heapID		= heapID;

	mcss_sys->mcss =GFL_HEAP_AllocClearMemory( heapID, sizeof(MUS_MCSS_WORK *) * max );
  
  mcss_sys->befVCount = OS_GetVBlankCount();

#ifdef USE_RENDER
	MUS_MCSS_InitRenderer( mcss_sys );
#endif //USE_RENDER

	mcss_sys->texAdrs = MUS_MCSS_TEX_ADRS;
	mcss_sys->palAdrs = MUS_MCSS_PAL_ADRS;

#if USE_SHADOW
	//影リソースロード
	NNS_G2dInitImagePaletteProxy( &mus_shadow_palette );

	// load palette data
	{
		TCB_LOADRESOURCE_WORK *tlw = GFL_HEAP_AllocClearMemory( heapID, sizeof( TCB_LOADRESOURCE_WORK ) );

		tlw->palette_p = &mus_shadow_palette;
		tlw->pal_ofs = MUS_MCSS_PAL_ADRS + 0x100;
		tlw->pBufPltt = GFL_ARC_UTIL_LoadPalette( ARCID_BATTGRA, NARC_battgra_wb_shadow_NCLR, &tlw->pPlttData, heapID );
		GF_ASSERT( tlw->pBufPltt != NULL);

		GFUser_VIntr_CreateTCB( TCB_LoadResource, tlw, 0 );
	}
#endif //USE_SHADOW
	
	return mcss_sys;
}

//--------------------------------------------------------------------------
/**
 * システム終了
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_Exit( MUS_MCSS_SYS_WORK *mcss_sys )
{
	int	i;

	for( i = 0 ; i < mcss_sys->mcss_max ; i++ ){
		if( mcss_sys->mcss[i] != NULL ){
			MUS_MCSS_Del( mcss_sys, mcss_sys->mcss[i] );
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
void	MUS_MCSS_Main( MUS_MCSS_SYS_WORK *mcss_sys )
{
	int	index;

  const u32 nowVCount = OS_GetVBlankCount();
  u32 subVCount = nowVCount - mcss_sys->befVCount;

	for( index = 0 ; index < mcss_sys->mcss_max ; index++ ){
		if( ( mcss_sys->mcss[index] != NULL ) && ( mcss_sys->mcss[index]->anm_stop_flag == 0 ) ){
			// アニメーションを更新します
			NNS_G2dTickMCAnimation( &mcss_sys->mcss[index]->mcss_mcanim, FX32_ONE*subVCount );
		}
	}
	mcss_sys->befVCount = nowVCount;
}

#ifndef USE_RENDER
//独自描画
//--------------------------------------------------------------------------
/**
 * 描画システム
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_Draw( MUS_MCSS_SYS_WORK *mcss_sys , MusicalCellCallBack musCellCb )
{
	MUS_MCSS_WORK					*mcss;
	MUS_MCSS_NCEC					*ncec;
    NNSG2dImageProxy			*image_p;
    NNSG2dImagePaletteProxy		*palette_p;
	int							index;
	int							node,cell;
	fx32						pos_z_default;
	NNSG2dAnimController		*anim_ctrl_mc;
	NNSG2dAnimDataSRT			anim_SRT_mc;
	NNSG2dAnimController		*anim_ctrl_c;
	NNSG2dAnimDataT				*anim_T_p;
	NNSG2dAnimDataSRT			anim_SRT;
	NNSG2dMCNodeCellAnimArray	*MC_Array;
	NNSG2dAnimSequenceData		*anim;
	VecFx32						pos,anim_pos;
	MtxFx44						inv_camera;
	u8 							musCellArr;
	u8							flipFlg;
	
	G3_PushMtx();
	G3_MtxMode( GX_MTXMODE_PROJECTION );
	G3_StoreMtx( 0 );
	G3_MtxMode( GX_MTXMODE_TEXTURE );
	G3_Identity();
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

	if( mcss_sys->mcss_ortho_mode == 0 ){
		G3_LookAt( NNS_G3dGlbGetCameraPos(), NNS_G3dGlbGetCameraUp(), NNS_G3dGlbGetCameraTarget(), NULL );
	}
	else{
		// カメラ行列を設定します。
		// 単位行列と等価
/*
		VecFx32 Eye    = { 0, 0, 0 };          // Eye position
		VecFx32 vUp    = { 0, FX32_ONE, 0 };  // Up
		VecFx32 at     = { 0, 0, -FX32_ONE }; // Viewpoint

		G3_LookAt( &Eye, &vUp, &at, NULL );
*/

		static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
		static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
		static const VecFx32 cam_up = MUSICAL_CAMERA_UP;

		G3_LookAt( &cam_pos, &cam_up, &cam_target, NULL );

	}

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
			anim_ctrl_mc= NNS_G2dGetMCAnimAnimCtrl(&mcss->mcss_mcanim);
			MC_Array = (NNSG2dMCNodeCellAnimArray*)&mcss->mcss_mcanim.multiCellInstance.pCellAnimInstasnces;
			pos_z_default = 0;

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

			pos.x = mcss->pos.x;
			pos.y = mcss->pos.y;
			pos.z = mcss->pos.z;

			if( mcss_sys->mcss_ortho_mode == 0 ){
				anim_pos.x = MUS_MCSS_CONST( anim_SRT_mc.px );
				anim_pos.y = MUS_MCSS_CONST( -anim_SRT_mc.py );
			}
			else{
				
				
				MtxFx44	mtx,vp;
				fx32	w;

				vp._00 = FX32_CONST( 128 );
				vp._01 = 0;
				vp._02 = 0;
				vp._03 = FX32_CONST( 128 );
				vp._10 = 0;
				vp._11 = FX32_CONST( 96 );
				vp._12 = 0;
				vp._13 = -FX32_CONST( 96 );
				vp._20 = 0;
				vp._21 = 0;
				vp._22 = -FX32_CONST( 1 );
				vp._23 = FX32_CONST( 1 );
				vp._30 = 0;
				vp._31 = 0;
				vp._32 = 0;
				vp._33 = -FX32_CONST( 1 );

				MTX_Copy43To44( NNS_G3dGlbGetCameraMtx(), &mtx );
				MTX_Concat44( &mtx, NNS_G3dGlbGetProjectionMtx(), &mtx );
				MTX_MultVec44( &pos, &mtx, &pos, &w );

				pos.x = FX_Div( pos.x, w );
				pos.y = FX_Div( pos.y, w );

				MTX_MultVec44( &pos, &vp, &pos, &w );

				anim_pos.x = FX32_CONST( anim_SRT_mc.px );
				anim_pos.y = FX32_CONST( -anim_SRT_mc.py );
			}
			//Z値は戻す！
			pos.z = mcss->pos.z;

			//前もって、不変なマルチセルデータをカレント行列にかけておく
			G3_Translate( pos.x, pos.y, pos.z );
			//カメラの逆行列を掛け合わせる（ビルボード処理）
			if( mcss_sys->mcss_ortho_mode == 0 ){
				G3_MultMtx44( &inv_camera );
			}

			G3_Translate( anim_pos.x, anim_pos.y, 0 );
			G3_RotZ( -FX_SinIdx( anim_SRT_mc.rotZ ), FX_CosIdx( anim_SRT_mc.rotZ ) );
			G3_Scale( FX_Mul( anim_SRT_mc.sx, mcss->scale.x ), FX_Mul( anim_SRT_mc.sy, mcss->scale.y ), FX32_ONE );
			
			flipFlg = MUS_MCSS_FLIP_NONE;
			if( mcss->scale.x < 0 )
			{
				flipFlg += MUS_MCSS_FLIP_X;
			}
			if( mcss->scale.y < 0 )
			{
				//Y反転は取りあえず保留
			}
			
			//回転処理用のオフセットを出しておく
			{
				u16 rotZ;
				MtxFx33 rotWork;
				rotZ = mcss->rotZ;
				/*
				if( flipFlg == MUS_MCSS_FLIP_NONE )
				{
					rotZ = mcss->rotZ;
				}
				else
				if( isFlip & MUS_MCSS_FLIP_X )
				{
					rotZ = (u16)(0x10000-mcss->rotZ);
				}
				*/
				MTX_RotZ33( &rotWork , -FX_SinIdx( rotZ ) , FX_CosIdx( rotZ ) );
				MTX_MultVec33( &mcss->rotOfsBase , &rotWork , &mcss->rotOfs );
				VEC_Subtract( &mcss->rotOfsBase , &mcss->rotOfs , &mcss->rotOfs );
				mcss->rotOfs.x = FX_Div( mcss->rotOfs.x , mcss->scale.x );
				mcss->rotOfs.y = FX_Div( mcss->rotOfs.y , mcss->scale.y );
			}


			G3_StoreMtx( MUS_MCSS_NORMAL_MTX );
			
/*
			//影描画用の行列生成
			G3_LookAt( NNS_G3dGlbGetCameraPos(), NNS_G3dGlbGetCameraUp(), NNS_G3dGlbGetCameraTarget(), NULL );

			//前もって、不変なマルチセルデータをカレント行列にかけておく
			G3_Translate( mcss->pos.x, mcss->pos.y, mcss->pos.z + SHADOW_OFFSET );

			//影用の回転
			G3_RotX( FX_SinIdx( 65536 / 64 * 49 ), FX_CosIdx( 65536 / 64 * 49 ) );
			G3_Translate( MUS_MCSS_CONST( anim_SRT_mc.px ), MUS_MCSS_CONST( -anim_SRT_mc.py ), 0 );
			G3_RotZ( -FX_SinIdx( anim_SRT_mc.rotZ ), FX_CosIdx( anim_SRT_mc.rotZ ) );
			G3_Scale( FX_Mul( anim_SRT_mc.sx, mcss->shadow_scale.x ), FX_Mul( anim_SRT_mc.sy, mcss->shadow_scale.y ), FX32_ONE );

			G3_StoreMtx( MUS_MCSS_SHADOW_MTX );
*/
			MUS_MCSS_MaterialSetup();

			G3_TexImageParam(image_p->attr.fmt,
							 GX_TEXGEN_TEXCOORD,
							 image_p->attr.sizeS,
							 image_p->attr.sizeT,
							 GX_TEXREPEAT_ST,
							 GX_TEXFLIP_ST,
							 image_p->attr.plttUse,
							 image_p->vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN]);
			for( node = 0 ; node < mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc.index].numNodes ; node++ ){
				cell = (mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc.index].pHierDataArray[node].nodeAttr & 0xff00 ) >> 8;
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
						MUS_MCSS_DrawAct( mcss,
									  ncec->mepachi_pos_x,
									  ncec->mepachi_pos_y,
									  ncec->mepachi_size_x,
									  ncec->mepachi_size_y,
									  ncec->mepachi_tex_s,
									  ncec->mepachi_tex_t + ncec->mepachi_size_y,
									  &anim_SRT, &anim_SRT_mc, node,
									  mcss_sys->mcss_ortho_mode,
									  &pos_z_default ,
									  flipFlg );
					}
					else{
						MUS_MCSS_DrawAct( mcss,
									  ncec->mepachi_pos_x,
									  ncec->mepachi_pos_y,
									  ncec->mepachi_size_x,
									  ncec->mepachi_size_y,
									  ncec->mepachi_tex_s,
									  ncec->mepachi_tex_t,
									  &anim_SRT, &anim_SRT_mc, node,
									  mcss_sys->mcss_ortho_mode,
									  &pos_z_default ,
									  flipFlg );
					}
				}
				MUS_MCSS_DrawAct( mcss,
							  ncec->pos_x,
							  ncec->pos_y,
							  ncec->size_x,
							  ncec->size_y,
							  ncec->tex_s,
							  ncec->tex_t,
							  &anim_SRT, &anim_SRT_mc, node,
							  mcss_sys->mcss_ortho_mode,
							  &pos_z_default ,
							  flipFlg );
				
				//ミュージカルセル用処理
#if 1
				for( musCellArr=0;musCellArr<MUS_CELL_ARR_NUM;musCellArr++ )
				{
					if( ncec->musCellIdx[musCellArr] != 0xFF )
					{
						const u8 clIdx = ncec->musCellIdx[musCellArr];
						MUS_MCSS_CELL_DATA cellData;
						const s32 ofsx = mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc.index].pHierDataArray[node].posX;
						const s32 ofsy = mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc.index].pHierDataArray[node].posY;
						const fx32 itemOfsx = mcss->musInfo[clIdx].ofsX*FX_CosIdx(anim_SRT.rotZ) - mcss->musInfo[clIdx].ofsY*FX_SinIdx(anim_SRT.rotZ);
						const fx32 itemOfsy = mcss->musInfo[clIdx].ofsX*FX_SinIdx(anim_SRT.rotZ) + mcss->musInfo[clIdx].ofsY*FX_CosIdx(anim_SRT.rotZ);
						//ARI_TPrintf("mcss[%d:%d]\n",(int)F32_CONST(pos.x),(int)F32_CONST(pos.y));
						//ARI_TPrintf("mcss[%d:%d]:[%d:%d]:[%d:%d]\n",ofsx,ofsy,(int)F32_CONST(itemOfsx),(int)F32_CONST(itemOfsy),anim_SRT.px,anim_SRT.py);
						cellData.pos = pos;
						cellData.pos.y = -cellData.pos.y;
						cellData.ofs.x	= FX_Mul( FX32_CONST(ofsx + anim_SRT.px) + itemOfsx , mcss->scale.x/16 );
						cellData.ofs.y	= FX_Mul( FX32_CONST(ofsy + anim_SRT.py) + itemOfsy , mcss->scale.y/16 );
						cellData.ofs.z	= pos_z_default;
//						cellData.rotOfs.x = FX_Mul( mcss->rotOfs.x , mcss->scale.x );
//						cellData.rotOfs.y = FX_Mul( mcss->rotOfs.y , mcss->scale.y );
//						cellData.rotOfs.z = 0;
						cellData.rotOfs = mcss->rotOfsBase;
						cellData.rotZ	= mcss->rotZ;
						cellData.scale	= mcss->scale;
						cellData.itemRotZ = anim_SRT.rotZ + MUS_POKE_GRP_TO_ROT( mcss->musInfo[clIdx].grpNo );
						
						musCellCb(	mcss->musInfo[clIdx].pltNo ,
									&cellData , mcss->work );
						
						if( mcss->musInfo[clIdx].pltNo == MUS_POKE_PLT_ROTATE )
						{
							VEC_Set( &mcss->rotOfsBase , cellData.ofs.x , cellData.ofs.y , 0 );
						}
					}
					else
					{
						break;
					}
				}
#endif							  
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
void	MUS_MCSS_Draw( MUS_MCSS_SYS_WORK *mcss_sys )
{
	MUS_MCSS_WORK	*mcss;
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
static	void	MUS_MCSS_DrawAct( MUS_MCSS_WORK *mcss,
							  fx32 pos_x,
							  fx32 pos_y,
							  fx32 scale_x,
							  fx32 scale_y,
							  fx32 tex_s,
							  fx32 tex_t,
							  NNSG2dAnimDataSRT *anim_SRT_c,
							  NNSG2dAnimDataSRT *anim_SRT_mc,
							  int node,
							  u32 mcss_ortho_mode,
							  fx32 *pos_z_default ,
							  const u8 isFlip )
{
	VecFx32	pos;

	if( mcss_ortho_mode ){
		G3_OrthoW(	FX32_CONST( 96 ),
					-FX32_CONST( 96 ),
					-FX32_CONST( 128 ),
					FX32_CONST( 128 ),
					MUSICAL_CAMERA_NEAR,
					MUSICAL_CAMERA_FAR,
					FX32_ONE,
					NULL );
		G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );
	}

	G3_RestoreMtx( MUS_MCSS_NORMAL_MTX );

	G3_TexPlttBase(mcss->mcss_palette_proxy.vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN],
				   mcss->mcss_palette_proxy.fmt);
	G3_PolygonAttr(GX_LIGHTMASK_NONE,				// no lights
				   GX_POLYGONMODE_MODULATE,			// modulation mode
				   GX_CULL_NONE,					// cull back
				   0,								// polygon ID(0 - 63)
				   31,								// alpha(0 - 31)
				   0								// OR of GXPolygonAttrMisc's value
				   );

	//マルチセルデータから取得した位置で書き出し
	pos.x = MUS_MCSS_CONST( mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc->index].pHierDataArray[node].posX ) + MUS_MCSS_CONST( anim_SRT_c->px );
	pos.y = MUS_MCSS_CONST( -mcss->mcss_mcanim.pMultiCellDataBank->pMultiCellDataArray[anim_SRT_mc->index].pHierDataArray[node].posY ) + MUS_MCSS_CONST( -anim_SRT_c->py );
	{
		u16 rotZ;
		if( isFlip == MUS_MCSS_FLIP_NONE )
		{
			rotZ = mcss->rotZ;
		}
		else
		if( isFlip & MUS_MCSS_FLIP_X )
		{
			rotZ = (u16)(0x10000-mcss->rotZ);
		}
		
		
		G3_RotZ( -FX_SinIdx( rotZ ), FX_CosIdx( rotZ ) );
	}

	G3_Translate( pos.x, pos.y, *pos_z_default );

	G3_Translate( -mcss->rotOfs.x, mcss->rotOfs.y, 0 );
	G3_RotZ( -FX_SinIdx( anim_SRT_c->rotZ ), FX_CosIdx( anim_SRT_c->rotZ ) );

	if( isFlip == MUS_MCSS_FLIP_NONE )
	{
		G3_Scale( anim_SRT_c->sx, anim_SRT_c->sy, FX32_ONE );

		G3_Translate( pos_x, pos_y, 0 );

		G3_Scale( scale_x, scale_y, FX32_ONE );
	}
	else
	if( isFlip & MUS_MCSS_FLIP_X )
	{
		//反転による調整
		G3_Scale(-anim_SRT_c->sx, anim_SRT_c->sy, FX32_ONE );

		G3_Translate( -pos_x, pos_y, 0 );

		G3_Scale( scale_x, scale_y, FX32_ONE );
		G3_Translate( -MUS_MCSS_DEFAULT_LINE, 0, 0 );
		//テクスチャのリピートでフリップした所を使う
		tex_s = FX32_CONST(256.0f) + FX32_CONST(256.0f) - tex_s - scale_x;
	}
	if( isFlip & MUS_MCSS_FLIP_Y )
	{
		//Y反転は取りあえず保留
	}
	
	G3_Begin(GX_BEGIN_QUADS);
	G3_TexCoord( tex_s,				tex_t );
	G3_Vtx( 0, 0, 0 );
	G3_TexCoord( tex_s + scale_x,	tex_t );
	G3_Vtx( MUS_MCSS_DEFAULT_LINE, 0, 0 );
	G3_TexCoord( tex_s + scale_x,	tex_t + scale_y );
	G3_Vtx( MUS_MCSS_DEFAULT_LINE, -MUS_MCSS_DEFAULT_LINE, 0 );
	G3_TexCoord( tex_s,				tex_t + scale_y );
	G3_Vtx( 0, -MUS_MCSS_DEFAULT_LINE, 0 );
	G3_End();

	G3_MtxMode( GX_MTXMODE_PROJECTION );
	G3_RestoreMtx( 0 );
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );

#if USE_SHADOW
	G3_RestoreMtx( MUS_MCSS_SHADOW_MTX );

	G3_TexPlttBase(mus_shadow_palette.vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_3DMAIN],
				   mus_shadow_palette.fmt);
	G3_PolygonAttr(GX_LIGHTMASK_NONE,				// no lights
				   GX_POLYGONMODE_MODULATE,			// modulation mode
				   GX_CULL_NONE,					// cull back
				   1,								// polygon ID(0 - 63)
				   15,								// alpha(0 - 31)
				   0								// OR of GXPolygonAttrMisc's value
				   );

	G3_Translate( pos.x, pos.y, 0 );

	G3_RotZ( -FX_SinIdx( anim_SRT_c->rotZ ), FX_CosIdx( anim_SRT_c->rotZ ) );

	G3_Scale( anim_SRT_c->sx, anim_SRT_c->sy, FX32_ONE );

	G3_Translate( pos_x, pos_y, 0 );

	G3_Scale( scale_x, scale_y, FX32_ONE );

	G3_Begin(GX_BEGIN_QUADS);
	G3_TexCoord( tex_s,				tex_t );
	G3_Vtx( 0, 0, 0 );
	G3_TexCoord( tex_s + scale_x,	tex_t );
	G3_Vtx( MUS_MCSS_DEFAULT_LINE, 0, 0 );
	G3_TexCoord( tex_s + scale_x,	tex_t + scale_y );
	G3_Vtx( MUS_MCSS_DEFAULT_LINE, -MUS_MCSS_DEFAULT_LINE, 0 );
	G3_TexCoord( tex_s,				tex_t + scale_y );
	G3_Vtx( 0, -MUS_MCSS_DEFAULT_LINE, 0 );
	G3_End();
#endif //USE_SHADOW

	if( mcss_ortho_mode == 0 ){
		*pos_z_default -= MUS_MCSS_DEFAULT_Z;
	}
	else{
		*pos_z_default -= MUS_MCSS_DEFAULT_Z_ORTHO;
	}

}


//--------------------------------------------------------------------------
/**
 * マルチセル登録
 * 表示中に読み込むならisVBlankをTRUEにすること！！
 */
//--------------------------------------------------------------------------
MUS_MCSS_WORK*	MUS_MCSS_Add( MUS_MCSS_SYS_WORK *mcss_sys, fx32	pos_x, fx32	pos_y, fx32	pos_z, MUS_MCSS_ADD_WORK *maw , void *work , const BOOL isVBlank )
{
	int			count;

	for( count = 0 ; count < mcss_sys->mcss_max ; count++ ){
		if( mcss_sys->mcss[ count ] == NULL ){
			mcss_sys->mcss[ count ] = GFL_HEAP_AllocClearMemory( mcss_sys->heapID, sizeof(MUS_MCSS_WORK) );
			mcss_sys->mcss[ count ]->index = count;
			mcss_sys->mcss[ count ]->heapID = mcss_sys->heapID;
			mcss_sys->mcss[ count ]->pos.x = pos_x;
			mcss_sys->mcss[ count ]->pos.y = pos_y;
			mcss_sys->mcss[ count ]->pos.z = pos_z;
			mcss_sys->mcss[ count ]->rotZ = 0;
			mcss_sys->mcss[ count ]->scale.x = FX32_ONE;
			mcss_sys->mcss[ count ]->scale.y = FX32_ONE;
			mcss_sys->mcss[ count ]->scale.z = FX32_ONE;
			mcss_sys->mcss[ count ]->work = work;
			MUS_MCSS_LoadResource( mcss_sys, count, maw , isVBlank );
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
void	MUS_MCSS_Del( MUS_MCSS_SYS_WORK *mcss_sys, MUS_MCSS_WORK *mcss )
{
	GFL_HEAP_FreeMemory( mcss->mcss_ncer_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_nanr_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_nmcr_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_nmar_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_mcanim_buf );
	GFL_HEAP_FreeMemory( mcss->mcss_ncec );
//	GFL_HEAP_FreeMemory( mcss->musInfo );	//こいつはmcss_ncecからオフセットで取っているので開放無し

	mcss_sys->mcss[ mcss->index ] = NULL;
	GFL_HEAP_FreeMemory( mcss );
}

//--------------------------------------------------------------------------
/**
 * 正射影描画モードをセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_SetOrthoMode( MUS_MCSS_SYS_WORK *mcss_sys )
{
	mcss_sys->mcss_ortho_mode = 1;
}

//--------------------------------------------------------------------------
/**
 * 正射影描画モードをリセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_ResetOrthoMode( MUS_MCSS_SYS_WORK *mcss_sys )
{
	mcss_sys->mcss_ortho_mode = 0;
}

//--------------------------------------------------------------------------
/**
 * テクスチャVRAM開始アドレスの変更(何かを読み込んだ後に使うとマズイ
 */
//--------------------------------------------------------------------------
void			MUS_MCSS_SetTexAddres( MUS_MCSS_SYS_WORK *mcss_sys , u32 adr )
{
  mcss_sys->texAdrs = adr;
}
//--------------------------------------------------------------------------
/**
 * テクスチャVRAM開始アドレスの変更(何かを読み込んだ後に使うとマズイ
 */
//--------------------------------------------------------------------------
void			MUS_MCSS_SetPltAddres( MUS_MCSS_SYS_WORK *mcss_sys , u32 adr )
{
  mcss_sys->palAdrs = adr;
}
//--------------------------------------------------------------------------
/**
 * ポジションゲット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_GetPosition( MUS_MCSS_WORK *mcss, VecFx32 *pos )
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
void	MUS_MCSS_SetPosition( MUS_MCSS_WORK *mcss, VecFx32 *pos )
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
void	MUS_MCSS_GetScale( MUS_MCSS_WORK *mcss, VecFx32 *scale )
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
void	MUS_MCSS_SetScale( MUS_MCSS_WORK *mcss, VecFx32 *scale )
{
	mcss->scale.x = scale->x;
	mcss->scale.y = scale->y;
	mcss->scale.z = scale->z;
}

void			MUS_MCSS_GetRotation( MUS_MCSS_WORK *mcss, u16 *rotZ )
{
	*rotZ = mcss->rotZ;
}

void			MUS_MCSS_SetRotation( MUS_MCSS_WORK *mcss, u16 rotZ )
{
	mcss->rotZ = rotZ;
}

//--------------------------------------------------------------------------
/**
 * 影描画用スケールセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_SetShadowScale( MUS_MCSS_WORK *mcss, VecFx32 *scale )
{
	mcss->shadow_scale.x = scale->x;
	mcss->shadow_scale.y = scale->y;
	mcss->shadow_scale.z = scale->z;
}

//--------------------------------------------------------------------------
/**
 * メパチフラグセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_SetMepachiFlag( MUS_MCSS_WORK *mcss )
{
	mcss->mepachi_flag = MUS_MCSS_MEPACHI_ON;
}

//--------------------------------------------------------------------------
/**
 * メパチフラグリセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_ResetMepachiFlag( MUS_MCSS_WORK *mcss )
{
	mcss->mepachi_flag = MUS_MCSS_MEPACHI_OFF;
}

//--------------------------------------------------------------------------
/**
 * アニメストップフラグセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_SetAnmStopFlag( MUS_MCSS_WORK *mcss )
{
	mcss->anm_stop_flag = MUS_MCSS_ANM_STOP_ON;
}

//--------------------------------------------------------------------------
/**
 * アニメストップフラグリセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_ResetAnmStopFlag( MUS_MCSS_WORK *mcss )
{
	mcss->anm_stop_flag = MUS_MCSS_ANM_STOP_OFF;
}

//--------------------------------------------------------------------------
/**
 * アニメ変更
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_ChangeAnm( MUS_MCSS_WORK *mcss , const u8 anmIdx )
{
	const NNSG2dMultiCellAnimSequence* pSequence;

	// 再生するシーケンスを取得
	pSequence = NNS_G2dGetAnimSequenceByIdx( mcss->mcss_nmar, anmIdx );
	GF_ASSERT( pSequence != NULL );

	// マルチセルアニメーションを構築
	MUS_MCSS_GetNewMultiCellAnimation( mcss, NNS_G2D_MCTYPE_SHARE_CELLANIM );

	// マルチセルアニメーションに再生するシーケンスをセット
	NNS_G2dSetAnimSequenceToMCAnimation( &mcss->mcss_mcanim, pSequence );
}

//--------------------------------------------------------------------------
/**
 * バニッシュフラグゲット
 */
//--------------------------------------------------------------------------
int		MUS_MCSS_GetVanishFlag( MUS_MCSS_WORK *mcss )
{
	return mcss->vanish_flag;
}

//--------------------------------------------------------------------------
/**
 * バニッシュフラグセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_SetVanishFlag( MUS_MCSS_WORK *mcss )
{
	mcss->vanish_flag = MUS_MCSS_VANISH_ON;
}

//--------------------------------------------------------------------------
/**
 * バニッシュフラグリセット
 */
//--------------------------------------------------------------------------
void	MUS_MCSS_ResetVanishFlag( MUS_MCSS_WORK *mcss )
{
	mcss->vanish_flag = MUS_MCSS_VANISH_OFF;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

//MCSS_WORKでデータをコピーする(前後ろ切り替え時 コピー元mcss,コピー先mcss
void MUS_MCSS_CopyState( MUS_MCSS_WORK *srcMcss , MUS_MCSS_WORK *dstMcss )
{
  dstMcss->pos            = srcMcss->pos;
  dstMcss->scale          = srcMcss->scale;
  dstMcss->shadow_scale   = srcMcss->shadow_scale;
  dstMcss->mepachi_flag   = srcMcss->mepachi_flag;
  dstMcss->anm_stop_flag  = srcMcss->anm_stop_flag;
  dstMcss->vanish_flag    = srcMcss->vanish_flag;
  dstMcss->rotZ           = srcMcss->rotZ;
  dstMcss->rotOfsBase     = srcMcss->rotOfsBase;
  dstMcss->rotOfs         = srcMcss->rotOfs;
}

//--------------------------------------------------------------------------
/**
 * リソースロード
 */
//--------------------------------------------------------------------------
static	void	MUS_MCSS_LoadResource( MUS_MCSS_SYS_WORK *mcss_sys, int count, MUS_MCSS_ADD_WORK *maw , const BOOL isVBlank )
{
	MUS_MCSS_WORK	*mcss = mcss_sys->mcss[ count ];

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
		MUS_MCSS_GetNewMultiCellAnimation( mcss, NNS_G2D_MCTYPE_SHARE_CELLANIM );

		// マルチセルアニメーションに再生するシーケンスをセット
		NNS_G2dSetAnimSequenceToMCAnimation( &mcss->mcss_mcanim, pSequence );
	}

	//1枚の板ポリで表示するための情報の読み込み（独自フォーマット）
	mcss->mcss_ncec = GFL_ARC_LoadDataAlloc( maw->arcID, maw->ncec, mcss->heapID );
	mcss->musInfo  = (MUS_MCSS_NCEC_MUS*)&mcss->mcss_ncec->ncec[mcss->mcss_ncec->cells];
	{
#if 0 & DEB_ARI
		int i;
		for( i=0;i<12;i++ )
		{
			ARI_TPrintf("[%2d][%d][%d][%d][%d][%d]\n",i
								,mcss->musInfo[i].objNo
								,mcss->musInfo[i].pltNo
								,mcss->musInfo[i].ofsX
								,mcss->musInfo[i].ofsY
								,mcss->musInfo[i].grpNo);
		}
#endif
	}

    //
    // VRAM 関連の初期化
    //
    {
			TCB_LOADRESOURCE_WORK *tlw = GFL_HEAP_AllocClearMemory( mcss->heapID, sizeof( TCB_LOADRESOURCE_WORK ) );
			tlw->image_p = &mcss->mcss_image_proxy;
			tlw->palette_p = &mcss->mcss_palette_proxy;
			tlw->chr_ofs = mcss_sys->texAdrs + MUS_MCSS_TEX_SIZE * count;
			tlw->pal_ofs = mcss_sys->palAdrs + MUS_MCSS_PAL_SIZE * count;
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
			
			if( isVBlank == TRUE )
			{
				GFUser_VIntr_CreateTCB( TCB_LoadResource, tlw, 0 );
			}
			else
			{
				TCB_LoadResource( NULL , tlw );
			}
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

	if( tlw->mcss ){
		tlw->mcss->vanish_flag = 0;
	}

	if( tlw->pBufChar ){
		// Loading For 3D Graphics Engine.（本来は、VRAMマネージャを使用したい）
		NNS_G2dLoadImage2DMapping(
			tlw->pCharData,
			tlw->chr_ofs,
			NNS_G2D_VRAM_TYPE_3DMAIN,
			tlw->image_p );
	
		GFL_HEAP_FreeMemory( tlw->pBufChar );
	}

	if( tlw->pPlttData ){
		// Loading For 3D Graphics Engine.
		NNS_G2dLoadPalette(
			tlw->pPlttData,
			tlw->pal_ofs,
			NNS_G2D_VRAM_TYPE_3DMAIN,
			tlw->palette_p );

		GFL_HEAP_FreeMemory( tlw->pBufPltt );
	}

	GFL_HEAP_FreeMemory( work );
	if( tcb != NULL )
	{
		GFL_TCB_DeleteTask( tcb );
	}
}

/*---------------------------------------------------------------------------*
  Name:         MUS_MCSS_GetNewMultiCellAnimation

  Description:  新しい NNSG2dMultiCellAnimation を取得して初期化します。

  Arguments:    pAnimBank:          アニメーションデータバンク
                pCellDataBank:      セルデータバンク
                pMultiCellDataBank: マルチセルデータバンク
                mcType:             マルチセル実体の種類
                
  Returns:      プールから取得した初期化済みの NNSG2dMultiCellAnimation へ
                のポインタ
 *---------------------------------------------------------------------------*/
static	void	MUS_MCSS_GetNewMultiCellAnimation(MUS_MCSS_WORK *mcss, NNSG2dMCType	mcType )
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
static	void	MUS_MCSS_MaterialSetup(void)
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
  Name:         MUS_MCSS_InitRenderer

  Description:  Renderer と Surface を初期化します。

  Arguments:    pRenderer:  初期化する Renderer へのポインタ。
                pSurface:   初期化する Surface へのポインタ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static	void	MUS_MCSS_InitRenderer( MUS_MCSS_SYS_WORK *mcss_sys )
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

//神王蟲からいただき
//----------------------------------------------------------------------------
/**
 *	@brief	4x4行列に座標を掛け合わせる
 *			Vecをx,y,z,1として計算し、計算後のVecとwを返します。
 *
 *	@param	*cp_src	Vector座標
 *	@param	*cp_m	4*4行列
 *	@param	*p_dst	Vecror計算結果
 *	@param	*p_w	4つ目の要素の値
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static	void MTX_MultVec44( const VecFx32 *cp_src, const MtxFx44 *cp_m, VecFx32 *p_dst, fx32 *p_w )
{

	fx32 x = cp_src->x;
    fx32 y = cp_src->y;
    fx32 z = cp_src->z;
	fx32 w = FX32_ONE;

	GF_ASSERT( cp_src );
	GF_ASSERT( cp_m );
	GF_ASSERT( p_dst );
	GF_ASSERT( p_w );

    p_dst->x = (fx32)(((fx64)x * cp_m->_00 + (fx64)y * cp_m->_10 + (fx64)z * cp_m->_20) >> FX32_SHIFT);
    p_dst->x += cp_m->_30;	//	W=1なので足すだけ

    p_dst->y = (fx32)(((fx64)x * cp_m->_01 + (fx64)y * cp_m->_11 + (fx64)z * cp_m->_21) >> FX32_SHIFT);
    p_dst->y += cp_m->_31;//	W=1なので足すだけ

    p_dst->z = (fx32)(((fx64)x * cp_m->_02 + (fx64)y * cp_m->_12 + (fx64)z * cp_m->_22) >> FX32_SHIFT);
    p_dst->z += cp_m->_32;//	W=1なので足すだけ

	*p_w	= (fx32)(((fx64)x * cp_m->_03 + (fx64)y * cp_m->_13 + (fx64)z * cp_m->_23) >> FX32_SHIFT);
    *p_w	+= cp_m->_33;//	W=1なので足すだけ
}

