//======================================================================
/**
 * @file	sta_act_bg.c
 * @brief	ステージ用　背景(bbdでは大きいサイズが扱えないので独自実装
 * @author	ariizumi
 * @data	09/03/16
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "stage_gra.naix"

#include "sta_acting.h"
#include "sta_local_def.h"
#include "sta_act_bg.h"
#include "musical/musical_camera_def.h"

#include "test/ariizumi/ari_debug.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define ACT_BG_TEX_FMT (GX_TEXFMT_PLTT256)

#define ACT_BG_SIZE_X (FX32_CONST(512.0f))
#define ACT_BG_SIZE_Y (FX32_CONST(192.0f))

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _STA_BG_SYS
{
	HEAPID heapId;
	ACTING_WORK* actWork;

	u16	scrollOffset;
	
	GFL_G3D_RES	*texRes;

	u32				texDataAdrs;		//テクスチャデータ格納VRAMアドレス
	u32				texPlttAdrs;		//テクスチャパレット格納VRAMアドレス
	NNSGfdTexKey	texDataKey;
	NNSGfdPlttKey	texPlttKey;

	GFL_G3D_RES	*mdlRes;
  GFL_G3D_RND *render;
  GFL_G3D_OBJ *g3dobj;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
STA_BG_SYS* STA_BG_InitSystem( HEAPID heapId , ACTING_WORK* actWork )
{
	u8 i;
	STA_BG_SYS *work = GFL_HEAP_AllocMemory( heapId , sizeof(STA_BG_SYS) );
	
	work->heapId = heapId;
	work->actWork = actWork;
	work->scrollOffset = 0;
	work->texRes = NULL;
	
	return work;
}

void	STA_BG_ExitSystem( STA_BG_SYS *work )
{
	if( work->texRes != NULL )
	{
		STA_BG_DeleteBg( work );
	}
	
	GFL_HEAP_FreeMemory( work );
}

void	STA_BG_UpdateSystem( STA_BG_SYS *work )
{
}

void	STA_BG_DrawSystem( STA_BG_SYS *work )
{
  if( work->texRes != NULL )
  {
    static GFL_G3D_OBJSTATUS objState = {
                      {FX32_CONST(16.0f),FX32_CONST(3.0f),FX32_CONST(-6.0f)},
                      {FX32_CONST(0.05f),FX32_CONST(0.05f),FX32_CONST(0.05f)},
                      {0}};
    
  	MTX_Identity33(&objState.rotate);
    GFL_G3D_DRAW_DrawObject( work->g3dobj , &objState );
    
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    {
      OS_Printf("[%.0f][%.0f][%.0f]:[%.2f]\n",FX_FX32_TO_F32(objState.trans.x),FX_FX32_TO_F32(objState.trans.y),FX_FX32_TO_F32(objState.trans.z),FX_FX32_TO_F32(objState.scale.x));
    }

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
      {
        objState.trans.z += FX32_CONST(0.25f);
      }
      else
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
      {
        objState.trans.z -= FX32_CONST(0.25f);
      }
      else
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
      {
        objState.scale.x += FX32_CONST(0.01);
        objState.scale.y += FX32_CONST(0.01);
        objState.scale.z += FX32_CONST(0.01);
      }
      else
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
      {
        objState.scale.x -= FX32_CONST(0.01);
        objState.scale.y -= FX32_CONST(0.01);
        objState.scale.z -= FX32_CONST(0.01);
      }
    }
    else
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
      {
        objState.trans.y -= FX32_CONST(0.25f);
      }
      else
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
      {
        objState.trans.y += FX32_CONST(0.25f);
      }
      else
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
      {
        objState.trans.x += FX32_CONST(0.25f);
      }
      else
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
      {
        objState.trans.x -= FX32_CONST(0.25f);
      }
    }
  }
}

void	STA_BG_CreateBg( STA_BG_SYS* work , const int arcId , const int datId )
{
	work->texRes = GFL_G3D_CreateResourceArc( ARCID_STAGE_GRA, NARC_stage_gra_musical_test_nsbtx ); 
	work->mdlRes = GFL_G3D_CreateResourceArc( ARCID_STAGE_GRA, NARC_stage_gra_musical_test_nsbmd ); 
	if( GFL_G3D_TransVramTexture( work->mdlRes ) == FALSE )
	{
		GF_ASSERT_MSG( NULL , "Stage Bg System Can't Create Bg!\n" );
		return;
	}
  
  work->render = GFL_G3D_RENDER_Create( work->mdlRes , 0 , work->mdlRes );
//  GFL_G3D_RENDER_SetTexture( work->render , 0 , work->texRes );
  work->g3dobj = GFL_G3D_OBJECT_Create( work->render , NULL , 0 );
}

void	STA_BG_DeleteBg( STA_BG_SYS* work )
{
  GFL_G3D_OBJECT_Delete( work->g3dobj );
  GFL_G3D_RENDER_Delete( work->render );
  GFL_G3D_DeleteResource( work->mdlRes );
  GFL_G3D_DeleteResource( work->texRes );
}

#if 0
void	STA_BG_DrawSystem( STA_BG_SYS *work )
{
	MtxFx33				mtxBillboard;
	VecFx16				vecN;
	fx32				scale, s0, t0, s1, t1, tmp;
	int					objIdx, resIdx;

	G3X_Reset();

	//カメラ設定取得
	{
		VecFx32		vecNtmp;
		MtxFx43		mtxCamera, mtxCameraInv;
		static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
		static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
		static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
	
		G3_LookAt( &cam_pos, &cam_up, &cam_target, &mtxCamera );	//mtxCameraには行列計算結果が返る
		MTX_Inverse43( &mtxCamera, &mtxCameraInv );			//カメラ逆行列取得
		MTX_Copy43To33( &mtxCameraInv, &mtxBillboard );		//カメラ逆行列から回転行列を取り出す

		VEC_Subtract( &cam_pos, &cam_target, &vecNtmp );
		VEC_Normalize( &vecNtmp, &vecNtmp );
		VEC_Fx16Set( &vecN, vecNtmp.x, vecNtmp.y, vecNtmp.z );
	}
/*
	if( g3Dlightset ){
		GFL_G3D_LIGHT_Switching( g3Dlightset );
	}
	G3_Scale( scale, scale, scale );
*/

	G3_PushMtx();
	{
		VecFx32 trans={0,0,0};
		//trans.x = FX_Div( obj->trans.x, p_setup->scale.x );
		//trans.y = FX_Div( obj->trans.y, p_setup->scale.y );
		//trans.z = FX_Div( obj->trans.z, p_setup->scale.z );
		//trans.x = FX_Div( obj->trans.x, scale );
		//trans.y = FX_Div( obj->trans.y, scale );
		//trans.z = FX_Div( obj->trans.z, scale );

		//回転、平行移動パラメータ設定
		G3_MultTransMtx33( &mtxBillboard, &trans );
	}

	G3_TexImageParam(	ACT_BG_TEX_FMT, GX_TEXGEN_TEXCOORD, GX_TEXSIZE_S512, GX_TEXSIZE_T256,
						GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
						GX_TEXPLTTCOLOR0_TRNS, work->texDataAdrs );
	G3_TexPlttBase( work->texPlttAdrs, ACT_BG_TEX_FMT );

	//マテリアル設定
//	G3_MaterialColorDiffAmb( p_setup->diffuse, p_setup->ambient, TRUE );
//	G3_MaterialColorSpecEmi( p_setup->specular, p_setup->emission, FALSE );
	G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),		// diffuse
							GX_RGB(16, 16, 16),		// ambient
							TRUE					// use diffuse as vtx color if TRUE
							);

	G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),		// specular
							GX_RGB( 0,	0,	0),		// emission
							FALSE					// use shininess table if TRUE
							);
	//ポリゴンIDはクリアカラーと一緒
	//そうしないと画面境界にエッジが入る
	G3_PolygonAttr(		GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 
						63, 31 , 0 );
//						0, 0 , 0 );

	{
		VecFx32 trans = {0,0,0};
//		G3_MultTransMtx33( &mtxBillboard, &trans );
	}
	G3_Translate( ACT_POS_X(-work->scrollOffset), ACT_POS_Y(0.0f), 0);
	G3_Scale( FX32_ONE*32, FX32_ONE*12, FX32_ONE );

	//平面ポリゴンなので法線ベクトルは4頂点で共用
//	if( obj->lightMask ){
//		G3_Normal( vecN.x, vecN.y, vecN.z );
//	} else {
		G3_Color( GX_RGB( 31, 31, 31 ) );
//	}

	G3_Begin( GX_BEGIN_QUADS );


	G3_TexCoord( 0, 0 );
	G3_Vtx( 0, 0 , 0 );

	G3_TexCoord( ACT_BG_SIZE_X, 0 );
	G3_Vtx( FX16_ONE, 0, 0 );

	G3_TexCoord( ACT_BG_SIZE_X, ACT_BG_SIZE_Y );
	G3_Vtx( FX16_ONE, -FX16_ONE, 0 );

	G3_TexCoord( 0, ACT_BG_SIZE_Y );
	G3_Vtx( 0 , -FX16_ONE, 0 );

	G3_End();
	G3_PopMtx(1);

	//↓後にG3D_Systemで行うので、ここではやらない
	//G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}


void	STA_BG_CreateBg( STA_BG_SYS* work , const int arcId , const int datId )
{
	if( work->texRes != NULL )
	{
		STA_BG_DeleteBg( work );
	}

	work->texRes = GFL_G3D_CreateResourceArc( arcId, datId ); 
	if( GFL_G3D_TransVramTexture( work->texRes ) == FALSE )
	{
		GF_ASSERT_MSG( NULL , "Stage Bg System Can't Create Bg!\n" );
		return;
	}

	work->texDataKey = GFL_G3D_GetTextureDataVramKey( work->texRes );
	work->texPlttKey = GFL_G3D_GetTexturePlttVramKey( work->texRes );

	work->texDataAdrs = NNS_GfdGetTexKeyAddr( work->texDataKey );
	work->texPlttAdrs = NNS_GfdGetPlttKeyAddr( work->texPlttKey );

}

void	STA_BG_DeleteBg( STA_BG_SYS* work )
{
	GFL_G3D_FreeVramTexture( work->texRes );
	GFL_G3D_DeleteResource( work->texRes );
	work->texRes = NULL;
}
#endif //0


void	STA_BG_SetScrollOffset( STA_BG_SYS* work , const u16 bgOfs )
{
	work->scrollOffset = bgOfs;
}

