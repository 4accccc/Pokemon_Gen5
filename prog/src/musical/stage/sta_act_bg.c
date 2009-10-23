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
enum
{
  MUS_BG_ANM_ICA,
  MUS_BG_ANM_ITA,
  MUS_BG_ANM_ITP,
  
  MUS_BG_ANM_MAX,
};

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _STA_BG_SYS
{
	HEAPID heapId;
	ACTING_WORK* actWork;

	u16	scrollOffset;
	
	u32				texDataAdrs;		//テクスチャデータ格納VRAMアドレス
	u32				texPlttAdrs;		//テクスチャパレット格納VRAMアドレス
	NNSGfdTexKey	texDataKey;
	NNSGfdPlttKey	texPlttKey;

	GFL_G3D_RES	*mdlRes;
	GFL_G3D_RES	*anmRes[MUS_BG_ANM_MAX];
  GFL_G3D_RND *render;
  GFL_G3D_ANM *anmObj[MUS_BG_ANM_MAX];
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
	work->mdlRes = NULL;
	
	return work;
}

void	STA_BG_ExitSystem( STA_BG_SYS *work )
{
	if( work->mdlRes != NULL )
	{
		STA_BG_DeleteBg( work );
	}
	
	GFL_HEAP_FreeMemory( work );
}

void	STA_BG_UpdateSystem( STA_BG_SYS *work )
{
  //ミュージカルショットではアニメさせないために呼ばないので注意！
  u8 i;
	if( work->mdlRes != NULL )
	{
    for(i=0;i<MUS_BG_ANM_MAX;i++ )
    {
      GFL_G3D_OBJECT_LoopAnimeFrame( work->g3dobj , i , FX32_ONE );
    }
  }
}

void	STA_BG_DrawSystem( STA_BG_SYS *work )
{
  if( work->mdlRes != NULL )
  {
    static GFL_G3D_OBJSTATUS objState = {
                      {FX32_CONST(16.0f),FX32_CONST(3.0f),FX32_CONST(-6.0f)},
                      {FX32_CONST(0.2f),FX32_CONST(0.2f),FX32_CONST(0.2f)},
//                      {FX32_ONE,FX32_ONE,FX32_ONE},
                      {0}};
    
  	MTX_Identity33(&objState.rotate);
    GFL_G3D_DRAW_DrawObject( work->g3dobj , &objState );
    /*
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    {
      OS_Printf("[%.2f][%.2f][%.2f]:[%.2f]\n",FX_FX32_TO_F32(objState.trans.x),FX_FX32_TO_F32(objState.trans.y),FX_FX32_TO_F32(objState.trans.z),FX_FX32_TO_F32(objState.scale.x));
    }

    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
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
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
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
    */
  }
}

void	STA_BG_CreateBg( STA_BG_SYS* work , const u8 bgNo )
{
  u8 i;

	if( work->mdlRes != NULL )
	{
		STA_BG_DeleteBg( work );
	}

	work->mdlRes = GFL_G3D_CreateResourceArc( ARCID_STAGE_GRA, NARC_stage_gra_mus_stage01_nsbmd + bgNo ); 

	work->anmRes[MUS_BG_ANM_ICA] = GFL_G3D_CreateResourceArc( ARCID_STAGE_GRA, NARC_stage_gra_mus_stage01_nsbta + bgNo ); 
	work->anmRes[MUS_BG_ANM_ITA] = GFL_G3D_CreateResourceArc( ARCID_STAGE_GRA, NARC_stage_gra_mus_stage01_nsbca + bgNo ); 
	work->anmRes[MUS_BG_ANM_ITP] = GFL_G3D_CreateResourceArc( ARCID_STAGE_GRA, NARC_stage_gra_mus_stage01_nsbtp + bgNo ); 
	if( GFL_G3D_TransVramTexture( work->mdlRes ) == FALSE )
	{
		GF_ASSERT_MSG( NULL , "Stage Bg System Can't Create Bg!\n" );
		return;
	}
  
  work->render = GFL_G3D_RENDER_Create( work->mdlRes , 0 , work->mdlRes );

  for(i=0;i<MUS_BG_ANM_MAX;i++ )
  {
    work->anmObj[i] = GFL_G3D_ANIME_Create( work->render , work->anmRes[i], 0 );
  }
  work->g3dobj = GFL_G3D_OBJECT_Create( work->render , work->anmObj , MUS_BG_ANM_MAX );

  for(i=0;i<MUS_BG_ANM_MAX;i++ )
  {
    GFL_G3D_OBJECT_EnableAnime( work->g3dobj , i );
  }
}

void	STA_BG_DeleteBg( STA_BG_SYS* work )
{
  u8 i;
  GFL_G3D_OBJECT_Delete( work->g3dobj );

  for(i=0;i<MUS_BG_ANM_MAX;i++ )
  {
    GFL_G3D_ANIME_Delete( work->anmObj[i] );
  }
  GFL_G3D_RENDER_Delete( work->render );

  for(i=0;i<MUS_BG_ANM_MAX;i++ )
  {
    GFL_G3D_DeleteResource( work->anmRes[i] );
  }
  GFL_G3D_FreeVramTexture( work->mdlRes );
  GFL_G3D_DeleteResource( work->mdlRes );
  
  work->mdlRes = NULL;
}


void	STA_BG_SetScrollOffset( STA_BG_SYS* work , const u16 bgOfs )
{
	work->scrollOffset = bgOfs;
}

