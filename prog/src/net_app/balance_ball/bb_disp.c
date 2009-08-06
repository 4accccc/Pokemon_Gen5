//==============================================================================
/**
 * @file	bb_disp.c
 * @brief	簡単な説明を書く
 * @author	goto
 * @date	2007.10.01(月)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================

#include <gflib.h>
#include "system/main.h"
#include "bb_common.h"
#include "print\gf_font.h"
#include "font/font.naix"

#include "wlmngm_tool.naix"
#include "system/bmp_winframe.h"
#include "system/palanm.h"

#include "net_old/comm_def.h"
#include "net_old\comm_system.h"
#include "net_old\comm_state.h"
#include "net_old\comm_info.h"
#include "net_old\comm_tool.h"
#include "net_old\comm_command.h"

#define BB_PRINT_COL		( GF_PRINTCOLOR_MAKE( 1,  2, 15 ) )
#define BB_PRINT_COL_VIP	( GF_PRINTCOLOR_MAKE( 5,  6, 15 ) )

//==============================================================
// Prototype
//==============================================================
static inline void BB_inline_ModelDraw( BB_3D_MODEL* wk );
static inline void BB_inline_LightSet( void );
static void BB_disp_COAP_SimpleInit( GFL_CLWK_DATA* coap, s16 x, s16 y, int pal, int id );


//--------------------------------------------------------------
/**
 * @brief	モデル描画
 *
 * @param	obj	
 * @param	mat43	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_3DModelDraw( GFL_G3D_OBJSTATUS* obj, BOOL *draw_flag, MtxFx43* mat43, VecFx32* pos, GFL_G3D_RND *g3drnd )
{
	if( *draw_flag ){
		NNS_G3dGlbFlush();
		NNS_G3dGeTranslateVec( pos );
		NNS_G3dGeMultMtx43( mat43 );
		NNS_G3dGlbSetBaseScale( &obj->scale );	///< 拡大を反映させるため
#if WB_FIX
		NNS_G3dDraw( &obj->render );
#else
    NNS_G3dDraw( GFL_G3D_RENDER_GetRenderObj(g3drnd) );
#endif
	}
}


//--------------------------------------------------------------
/**
 * @brief	model の 描画
 *
 * @param	obj	
 *
 * @retval	static inline	
 *
 */
//--------------------------------------------------------------
static inline void BB_inline_ModelDraw( BB_3D_MODEL* wk )
{    
	NNS_G3dGePushMtx();
	
	BB_disp_3DModelDraw( &wk->obj, &wk->draw_flag, &wk->tmp43, &wk->pos, wk->g3drnd );
	
	NNS_G3dGePopMtx( 1 );
}

static inline void BB_inline_ModelDrawEx( BB_3D_MODEL* wk, BB_3D_MODEL* wk2 )
{    
	NNS_G3dGePushMtx();
	
	BB_disp_3DModelDraw( &wk2->obj, &wk2->draw_flag, &wk->tmp43, &wk->pos, wk2->g3drnd );
	
	NNS_G3dGePopMtx( 1 );
}


//--------------------------------------------------------------
/**
 * @brief	Light の せってい
 *
 * @param	none	
 *
 * @retval	static inline	
 *
 */
//--------------------------------------------------------------
static inline void BB_inline_LightSet( void )
{
	VecFx32 vec = { BB_LIGHT_VX, BB_LIGHT_VY, BB_LIGHT_VZ };
	
	VEC_Normalize( &vec, &vec );
	NNS_G3dGlbLightVector( BB_LIGHT_NO, vec.x, vec.y, vec.z );

	NNS_G3dGlbLightColor( BB_LIGHT_NO, GX_RGB( 31, 31, 31 ) );

	NNS_G3dGlbMaterialColorDiffAmb( GX_RGB( 31,31,31 ), GX_RGB( 31,31,31 ), FALSE );
	NNS_G3dGlbMaterialColorSpecEmi( GX_RGB( 31,31,31 ), GX_RGB( 31,31,31 ), FALSE );	
}



//--------------------------------------------------------------
/**
 * @brief	モデル ロード
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Model_Load( BB_3D_MODEL* wk, ARCHANDLE* p_handle, int id )
{
#if WB_FIX
	D3DOBJ_MdlLoadH( &wk->mdl, p_handle, id, HEAPID_BB );
#else
  wk->p_mdlres = GFL_G3D_CreateResourceHandle( p_handle, id );
  GFL_G3D_TransVramTexture(wk->p_mdlres);

	wk->g3drnd = GFL_G3D_RENDER_Create( wk->p_mdlres, 0, wk->p_mdlres );
	wk->g3dobj = GFL_G3D_OBJECT_Create( wk->g3drnd, NULL, 10 );
#endif
}


//--------------------------------------------------------------
/**
 * @brief	モデル 登録
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Model_Init( BB_3D_MODEL* wk, int flag )
{
#if WB_FIX  //BB_disp_Model_Loadでやるようにした
	D3DOBJ_Init( &wk->obj, &wk->mdl );
#endif
#if WB_FIX
	D3DOBJ_SetMatrix( &wk->obj, BB_MODEL_OFS_X, BB_MODEL_OFS_Y, BB_MODEL_OFS_Z );
#else
	VEC_Set(&wk->obj.trans, BB_MODEL_OFS_X, BB_MODEL_OFS_Y, BB_MODEL_OFS_Z );
#endif

#if WB_FIX
	if ( flag == 0 ){
		D3DOBJ_SetScale( &wk->obj, FX32_CONST(1.00f), FX32_CONST(1.00f), FX32_CONST(1.00f) );
	}
	else {
		D3DOBJ_SetScale( &wk->obj, FX32_CONST(1.50f), FX32_CONST(1.50f), FX32_CONST(1.50f) );
	}
  D3DOBJ_SetDraw( &wk->obj, TRUE );
#else
	if ( flag == 0 ){
		VEC_Set( &wk->obj.scale, FX32_CONST(1.00f), FX32_CONST(1.00f), FX32_CONST(1.00f) );
	}
	else {
		VEC_Set( &wk->obj.scale, FX32_CONST(1.50f), FX32_CONST(1.50f), FX32_CONST(1.50f) );
	}
	MTX_Identity33(&wk->obj.rotate);
	VEC_Set(&wk->obj_rotate, 0, 0, 0);
  wk->draw_flag = TRUE;
#endif

  wk->pos.x = BB_MODEL_OFS_X;
  wk->pos.y = BB_MODEL_OFS_Y;
  wk->pos.z = BB_MODEL_OFS_Z;
  
  wk->get_pow_x = 0.0f;
  wk->get_pow_y = 0.0f;
  
  wk->bAnime = FALSE;
  wk->bInit  = TRUE;
}


//--------------------------------------------------------------
/**
 * @brief	モデル 破棄
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Model_Delete( BB_3D_MODEL* wk )
{
	wk->bInit = FALSE;
#if WB_FIX
	D3DOBJ_MdlDelete( &wk->mdl );
#else
  if(wk->g3drnd != NULL){
  	GFL_G3D_RENDER_Delete( wk->g3drnd );
    wk->g3drnd = NULL;
  }
  if(wk->g3dobj != NULL){
  	GFL_G3D_OBJECT_Delete( wk->g3dobj );
  	wk->g3dobj = NULL;
  }
  if(wk->p_mdlres != NULL){
    GFL_G3D_DeleteResource(wk->p_mdlres);
    wk->p_mdlres = NULL;
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief	描画
 *
 * @param	wk	
 *
 * @retval	none	
 */
//--------------------------------------------------------------
void BB_disp_Draw( BB_WORK* wk )
{
	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	
	BB_inline_LightSet();
	
	{
		int i;		
		for ( i = 0; i < BB_KAMI_HUBUKI_MAX; i++ ){
			BB_inline_ModelDraw( &wk->p_client->bb3d_kami[ i ] );
		#if WB_FIX
			D3DOBJ_AnmLoop( &wk->p_client->bb3d_kami[ i ].anm[ 0 ], FX32_CONST( 1 ) );
		#else
      GFL_G3D_OBJECT_LoopAnimeFrame(wk->p_client->bb3d_kami[ i ].g3dobj, 0, FX32_CONST(1));
		#endif
		}
	}
	
	switch ( wk->p_client->anime_type ){
	case eANM_CODE_DEFAULT:
#if WB_FIX
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 0 ].obj, TRUE );
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 1 ].obj, FALSE );
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 2 ].obj, FALSE );
		D3DOBJ_AnmLoop( &wk->p_client->bb3d_mane[ 0 ].anm[ wk->p_client->bb3d_mane[ 0 ].anime_no ], FX32_CONST( 1 ) );
#else
		wk->p_client->bb3d_mane[ 0 ].draw_flag = TRUE;
		wk->p_client->bb3d_mane[ 1 ].draw_flag = FALSE;
		wk->p_client->bb3d_mane[ 2 ].draw_flag = FALSE;
    GFL_G3D_OBJECT_LoopAnimeFrame(wk->p_client->bb3d_mane[ 0 ].g3dobj, wk->p_client->bb3d_mane[ 0 ].anime_no, FX32_CONST(1));
#endif
		break;

	case eANM_CODE_FALL:
#if WB_FIX
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 0 ].obj, FALSE );
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 1 ].obj, TRUE );
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 2 ].obj, FALSE );
		D3DOBJ_AnmNoLoop( &wk->p_client->bb3d_mane[ 1 ].anm[ 0 ], FX32_CONST( 1 ) );
#else
    wk->p_client->bb3d_mane[ 0 ].draw_flag = FALSE;
    wk->p_client->bb3d_mane[ 1 ].draw_flag = TRUE;
    wk->p_client->bb3d_mane[ 2 ].draw_flag = FALSE;
		GFL_G3D_OBJECT_IncAnimeFrame( wk->p_client->bb3d_mane[ 1 ].g3dobj, 0, FX32_CONST( 1 ) );
#endif
		break;

	case eANM_CODE_RECOVER:
#if WB_FIX
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 0 ].obj, FALSE );
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 1 ].obj, FALSE );
		D3DOBJ_SetDraw( &wk->p_client->bb3d_mane[ 2 ].obj, TRUE );
		D3DOBJ_AnmNoLoop( &wk->p_client->bb3d_mane[ 2 ].anm[ 0 ], FX32_CONST( 1 ) );
#else
    wk->p_client->bb3d_mane[ 0 ].draw_flag = FALSE;
    wk->p_client->bb3d_mane[ 1 ].draw_flag = FALSE;
    wk->p_client->bb3d_mane[ 2 ].draw_flag = TRUE;
		GFL_G3D_OBJECT_IncAnimeFrame( wk->p_client->bb3d_mane[ 2 ].g3dobj, 0, FX32_CONST( 1 ) );
#endif
		break;
	}		
	OS_TPrintf("aaa anime_type = %d, rnder=%x, anime_no = %d\n", wk->p_client->anime_type, wk->p_client->bb3d_mane[ wk->p_client->anime_type ].g3drnd, wk->p_client->bb3d_mane[ 0 ].anime_no);
	BB_inline_ModelDrawEx( &wk->p_client->bb3d_mane[ 0 ], &wk->p_client->bb3d_mane[ wk->p_client->anime_type ] );
	BB_inline_ModelDraw( &wk->p_client->bb3d_ball );
	BB_inline_ModelDraw( &wk->p_client->bb3d_spot );

	///< lv up　上昇
	if ( wk->p_client->bb3d_lvup.bInit ){
		BB_inline_ModelDraw( &wk->p_client->bb3d_lvup );		
		if ( wk->p_client->bb3d_lvup.bAnime ){
			BOOL bEnd[ 2 ];
		#if WB_FIX
			bEnd[ 0 ] = D3DOBJ_AnmNoLoop( &wk->p_client->bb3d_lvup.anm[ 0 ], FX32_CONST( 1 ) );
			bEnd[ 1 ] = D3DOBJ_AnmNoLoop( &wk->p_client->bb3d_lvup.anm[ 1 ], FX32_CONST( 1 ) );
			if ( bEnd[ 0 ] && bEnd[ 1 ] ){
				 wk->p_client->bb3d_lvup.bAnime = FALSE;
			}
		#else
			bEnd[ 0 ] = GFL_G3D_OBJECT_IncAnimeFrame( wk->p_client->bb3d_lvup.g3dobj, 0, FX32_CONST( 1 ) );
			bEnd[ 1 ] = GFL_G3D_OBJECT_IncAnimeFrame( wk->p_client->bb3d_lvup.g3dobj, 1, FX32_CONST( 1 ) );
			if ( bEnd[ 0 ] == FALSE && bEnd[ 1 ] == FALSE){
				 wk->p_client->bb3d_lvup.bAnime = FALSE;
			}
		#endif
		}
	}
	
	///< lv up 通常
	if ( wk->p_client->bb3d_lvbs.bInit ){
		BB_inline_ModelDraw( &wk->p_client->bb3d_lvbs );		
		if ( wk->p_client->bb3d_lvbs.bAnime ){
		//	D3DOBJ_AnmLoop( &wk->p_client->bb3d_lvbs.anm[ 0 ], FX32_CONST( 1 ) );
	#if WB_FIX
			D3DOBJ_AnmLoop( &wk->p_client->bb3d_lvbs.anm[ 1 ], FX32_CONST( 1 ) );
	#else
      GFL_G3D_OBJECT_LoopAnimeFrame(wk->p_client->bb3d_lvbs.g3dobj, 1, FX32_CONST(1));
	#endif
		}
	}
	
	GFL_G3D_DRAW_End();

    GFL_CLACT_SYS_Main();
}


//--------------------------------------------------------------
/**
 * @brief	まねねのアニメ設定
 * @param	model	
 * @param	anime_no	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void ManeneAnime_Set( BB_CLIENT* wk, int anime_no )
{
	int old = wk->bb3d_mane[ 0 ].anime_no;
	int now = anime_no;

	if ( now == old ){ return; }

#if WB_FIX
	D3DOBJ_DelAnm( &wk->bb3d_mane[ 0 ].obj, &wk->bb3d_mane[ 0 ].anm[ old ] );
#else
	GFL_G3D_OBJECT_DisableAnime( wk->bb3d_mane[0].g3dobj, old );
#endif

	wk->bb3d_mane[ 0 ].anime_no = anime_no;	///< anime の変更

#if WB_FIX
	D3DOBJ_AddAnm( &wk->bb3d_mane[ 0 ].obj, &wk->bb3d_mane[ 0 ].anm[ now ] );
#else
  GFL_G3D_OBJECT_EnableAnime( wk->bb3d_mane[0].g3dobj, now);
#endif
}

//--------------------------------------------------------------
/**
 *
 * @param	model	
 *
 * @retval	int	
 *
 */
//--------------------------------------------------------------
int ManeneAnime_Get( BB_WORK* wk )
{
	return wk->p_client->bb3d_mane[ 0 ].anime_no;
}


//--------------------------------------------------------------
/**
 * @brief	まねねのアニメフラグ設定
 * @param	model	
 * @param	flag	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void ManeneAnime_FlagSet( BB_WORK* wk, BOOL flag )
{
	wk->p_client->bb3d_mane[ 0 ].bAnime = flag;
}


//--------------------------------------------------------------
/**
 * @brief	カメラせってい
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
GFL_G3D_CAMERA * BB_disp_CameraSet( BB_WORK* wk )
{
	VecFx32	target = { BB_CAMERA_TX, BB_CAMERA_TY, BB_CAMERA_TZ };
	VecFx32	pos	   = { BB_CAMERA_PX, BB_CAMERA_PY, BB_CAMERA_PZ };

#if WB_FIX
	GFC_InitCameraTC( &target, &pos, BB_CAMERA_PERSPWAY, GF_CAMERA_PERSPECTIV, FALSE, wk->sys.camera_p );
//	GFC_InitCameraTC( &target, &pos, BB_CAMERA_PERSPWAY, GF_CAMERA_PERSPECTIV, FALSE, wk->sys.camera_p );

	GFC_SetCameraClip( BB_CAMERA_NEAR_CLIP, BB_CAMERA_NEAR_FAR, wk->sys.camera_p );
	
	GFC_AttachCamera( wk->sys.camera_p );
#else
	VecFx32 camUp = {0, FX32_ONE, 0};
  GFL_G3D_CAMERA *p_camera;
  
	p_camera = GFL_G3D_CAMERA_Create(	GFL_G3D_PRJPERS, 
		FX_SinIdx( BB_CAMERA_PERSPWAY ), FX_CosIdx( BB_CAMERA_PERSPWAY ),
		defaultCameraAspect, 0, BB_CAMERA_NEAR_CLIP, BB_CAMERA_NEAR_FAR, 0,
		&pos, &camUp, &target, HEAPID_BB );

	GFL_G3D_CAMERA_Switching(p_camera);
  
  return p_camera;
#endif
}

//--------------------------------------------------------------
/**
 * @brief	簡単に初期化
 *
 * @param	x	
 * @param	y	
 * @param	d_area	
 * @param	pal	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void BB_disp_COAP_SimpleInit( GFL_CLWK_DATA* coap, s16 x, s16 y, int pal, int id )
{
#if WB_FIX
	coap->x			 = x;
	coap->y			 = y;
	coap->z			 = 0;		
	coap->anm		 = 0;
	coap->pri		 = 0;
	coap->d_area	 = d_area;
	coap->bg_pri	 = 2;
	coap->vram_trans = 0;
	coap->pal		 = pal;
	coap->id[0]		 = id;
	coap->id[1]		 = id;
	coap->id[2]		 = id;
	coap->id[3]		 = id;
	coap->id[4]		 = CLACT_U_HEADER_DATA_NONE;
	coap->id[5]		 = CLACT_U_HEADER_DATA_NONE;	
#else
  coap->pos_x = x;
  coap->pos_y = y;
  coap->anmseq = 0;
  coap->softpri = 0;
  coap->bgpri = 2;
#endif
}

static const s16 manene_pos[][ 3 ] = {
	{	///< １人プレイ
		0,0,0,
	},
	{	///< ２人プレイ
		128,0,0
	},
	{	///< ３人プレイ
		85, 170,0,
	},
	{	///< ４人プレイ
		64, 128, 192,
	},
};


//--------------------------------------------------------------
/**
 * @brief	花火の登録
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Hanabi_OAM_Add( BB_CLIENT* wk )
{
	GFL_CLWK_DATA coap;
#if WB_FIX
	CATS_SYS_PTR			 csp = wk->sys->csp;
	CATS_RES_PTR			 crp = wk->sys->crp;
#endif
	PALETTE_FADE_PTR		 pfd = wk->sys->pfd;
	
	int i;
	
	for ( i = 0; i < BB_KAMI_MAX; i++ ){
		
		BB_disp_COAP_SimpleInit( &coap, i * 8, 20, eBB_OAM_PAL_BD_HANABI, eBB_ID_HANABI );
    wk->cap_hanabi[ i ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_HANABI], wk->pltt_id[eBB_ID_HANABI], wk->cell_id[eBB_ID_HANABI], 
	    &coap,  CLSYS_DEFREND_MAIN, HEAPID_BB );
//		GFL_CLACT_WK_SetPlttOffs( wk->cap_hanabi[ i ], eBB_OAM_PAL_BD_HANABI, CLWK_PLTTOFFS_MODE_OAM_COLOR );		
		GFL_CLACT_WK_SetAnmSeq( wk->cap_hanabi[ i ], ( i % 11 ) + 1 );
	}

	BB_disp_Hanabi_OAM_Enable( wk, FALSE, 0 );
}


//--------------------------------------------------------------
/**
 * @brief	手の登録
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Hand_Add( BB_CLIENT* wk )
{
	GFL_CLWK_DATA coap;
#if WB_FIX
	CATS_SYS_PTR			 csp = wk->sys->csp;
	CATS_RES_PTR			 crp = wk->sys->crp;
#endif
	PALETTE_FADE_PTR		 pfd = wk->sys->pfd;
	
	int i;
	s16 pos_x[] = {  72, 180,  24, 232 };
	
	for ( i = 0; i < BB_HAND_MAX; i++ ){
		BB_disp_COAP_SimpleInit( &coap, pos_x[ i ], BB_SURFACE_LIMIT, eBB_OAM_PAL_BD_HAND, eBB_ID_HAND );
		coap.bgpri = 0;
		coap.softpri = 1;
    wk->cap_hand[ i ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_HAND], wk->pltt_id[eBB_ID_HAND], wk->cell_id[eBB_ID_HAND], 
	    &coap,  CLSYS_DEFREND_MAIN, HEAPID_BB );
		GFL_CLACT_WK_AddAnmFrame( wk->cap_hand[ i ], FX32_ONE*2 );
//		GFL_CLACT_WK_SetPlttOffs( wk->cap_hand[ i ], eBB_OAM_PAL_BD_HAND, CLWK_PLTTOFFS_MODE_OAM_COLOR );
	}
}


//--------------------------------------------------------------
/**
 * @brief	手の破棄
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Hand_Del( BB_CLIENT* wk )
{
	int i;
	
	for ( i = 0; i < BB_HAND_MAX; i++ ){
		GFL_CLACT_WK_Remove( wk->cap_hand[ i ] );
	}
}

void BB_disp_Hanabi_OAM_Enable( BB_CLIENT* wk, BOOL flag, int mode )
{
	int i;
	
	for ( i = 0; i < BB_KAMI_MAX; i++ ){		
		GFL_CLACT_WK_SetDrawEnable( wk->cap_hanabi[ i ], flag );
	}	
}

void BB_disp_Hanabi_OAM_Update( BB_CLIENT* wk )
{
	int i;
	
	for ( i = 0; i < BB_KAMI_MAX; i++ ){
		
		GFL_CLACT_WK_AddAnmFrame( wk->cap_hanabi[ i ], FX32_ONE*2 );
	}
}

void BB_disp_Hanabi_OAM_Del( BB_CLIENT* wk )
{
	int i;
	
	for ( i = 0; i < BB_KAMI_MAX; i++ ){
		
		GFL_CLACT_WK_Remove( wk->cap_hanabi[ i ] );
	}
}

void BB_disp_Manene_OAM_AnimeChangeCap( GFL_CLWK* cap, int type, int anime )
{
	int anime_seq[][2] = {
		{ eANM_MANENE_ARUKU,  eANM_BALL_STOP },
		{ eANM_MANENE_ARUKU,  eANM_BALL_ARUKU },
		{ eANM_MANENE_ARUKU,  eANM_BALL_ARUKU2 },
		{ eANM_MANENE_OCHIRU, eANM_BALL_STOP },
	};
		
	GFL_CLACT_WK_SetAnmSeq( cap, anime_seq[ anime ][ type ] );
}

void BB_disp_Manene_OAM_AnimeChange( BB_CLIENT* wk, int netid, int anime )
{
	int anime_seq[][2] = {
		{ eANM_MANENE_ARUKU,  eANM_BALL_STOP },
		{ eANM_MANENE_ARUKU,  eANM_BALL_ARUKU },
		{ eANM_MANENE_ARUKU,  eANM_BALL_ARUKU2 },
		{ eANM_MANENE_OCHIRU, eANM_BALL_STOP },
	};
	
	int no = wk->netid_to_capid[ netid ];
	
	GFL_CLACT_WK_SetAnmSeq( wk->cap_mane[ no ], anime_seq[ anime ][ 0 ] );
	GFL_CLACT_WK_SetAnmSeq( wk->cap_ball[ no ], anime_seq[ anime ][ 1 ] );
}


int BB_disp_Manene_OAM_AnimeCheck( BB_CLIENT* wk, int netid )
{
	int anime_seq[][2] = {
		{ eANM_MANENE_ARUKU,  eANM_BALL_STOP },
		{ eANM_MANENE_ARUKU,  eANM_BALL_ARUKU },
		{ eANM_MANENE_ARUKU,  eANM_BALL_ARUKU2 },
		{ eANM_MANENE_OCHIRU, eANM_BALL_STOP },
	};	
	int no;
	int anime[ 2 ];
	
	no = wk->netid_to_capid[ netid ];
	
	anime[ 0 ] = GFL_CLACT_WK_GetAnmSeq( wk->cap_mane[ no ] );
	anime[ 1 ] = GFL_CLACT_WK_GetAnmSeq( wk->cap_ball[ no ] );
	
	{
		int i;
		
		for ( i = 0; i < 4; i++ ){
			
			if ( anime[ 0 ] == anime_seq[ i ][ 0 ]
			&& 	 anime[ 1 ] == anime_seq[ i ][ 1 ] ){ return i; }
		}
	}

	GF_ASSERT( 0 );
	
	return 0;
}


void BB_disp_Manene_OAM_Update( BB_CLIENT* wk )
{
	int i;
	int no = 0;		
	
	for ( i = 0; i < wk->comm_num; i++ ){		
		if ( i == wk->netid ){ continue; }		
		GFL_CLACT_WK_AddAnmFrame( wk->cap_mane[ no ], FX32_ONE*2 );
		GFL_CLACT_WK_AddAnmFrame( wk->cap_ball[ no ], FX32_ONE*2 );
		GFL_CLACT_WK_AddAnmFrame( wk->cap_kage[ no ], FX32_ONE*2 );
		no++;
	}	
}

void BB_disp_Manene_OAM_Del( BB_CLIENT* wk )
{
	int i;
	int no = 0;		
	
	for ( i = 0; i < wk->comm_num; i++ ){
		
		if ( i == wk->netid ){ continue; }
		
		GFL_CLACT_WK_Remove( wk->cap_mane[ no ] );
		GFL_CLACT_WK_Remove( wk->cap_ball[ no ] );
		GFL_CLACT_WK_Remove( wk->cap_kage[ no ] );
		
		no++;
	}
}

//--------------------------------------------------------------
/**
 * @brief	メッセージウィンドウ作成
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_InfoWinAdd( BB_WORK *bb_wk, BB_CLIENT* wk )
{
#if WB_FIX
	GF_BGL_BmpWinInit( &wk->win );
#endif
	wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME3_S, 2, 19, 28, 4, 14, GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_MakeScreen(wk->win);
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win), 0xFF );	
	PRINT_UTIL_Setup(&bb_wk->print_util_talk, wk->win);
	{
		GFL_MSGDATA* man;
		STRBUF* str;
			
		man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_minigame_bb_dat, HEAPID_BB );
		str	= GFL_MSG_CreateString( man, 0 );
		
#if WB_FIX
		GF_STR_PrintSimple( &wk->win, NET_FONT_SYSTEM, str, 0, 0, 0, NULL );
#else
  	PRINT_UTIL_Print( &bb_wk->print_util_talk, bb_wk->printQue, 0, 0, str, bb_wk->font_handle );
#endif
		GFL_STR_DeleteBuffer( str );
		GFL_MSG_Delete( man );	
	}	
	BmpWinFrame_Write( wk->win, WINDOW_TRANS_ON, BB_BG_CGX_OFS, 13 );
#if WB_FIX
	GF_BGL_BmpWinOn( &wk->win );
#else
	GFL_BMPWIN_MakeTransWindow(wk->win);
#endif
}


//--------------------------------------------------------------
/**
 * @brief	メッセージウィンドウ破棄
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_InfoWinDel( BB_CLIENT* wk )
{
#if WB_FIX
	if( GF_BGL_BmpWinAddCheck( &wk->win ) == TRUE ){
#else
	if(wk->win != NULL){
#endif
		BmpWinFrame_Clear( wk->win, WINDOW_TRANS_OFF );
		GFL_BMPWIN_ClearTransWindow( wk->win );
		GFL_BMPWIN_Delete( wk->win );
		wk->win = NULL;
	}
}

//--------------------------------------------------------------
/**
 * @brief	いろがえ
 *
 * @param	wk	
 * @param	id	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BR_ColorChange( BB_WORK* wk, int id )
{
	PALETTE_FADE_PTR pfd	= wk->sys.pfd;

	/// 1 = 青 2 = 赤 = 3 = 黄 4 = 緑
	static int pal_pos[][ 4 ][ 4 + 1 ] = {
		/// comm_num = 1
		{
			{ 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0 },
		},
		/// comm_num = 2 
		{
			{ 1, 2, 0, 0, 0 },
			{ 2, 1, 0, 0, 0 },
			{ 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0 },
		},
		/// comm_num = 3
		{
			{ 1, 2, 3, 0, 0 },
			{ 2, 1, 3, 0, 0 },
			{ 3, 1, 2, 0, 0 },
			{ 0, 0, 0, 0, 0 },
		},
		/// comm_num = 4
		{
			{ 1, 2, 3, 4, 0 },
			{ 2, 1, 3, 4, 0 },
			{ 3, 1, 2, 4, 0 },
			{ 4, 1, 2, 3, 0 },
		},
	};
	
	{
		int i;
		
		for ( i = 0; i < 4; i++ ){
			PaletteWorkSetEx_Arc( pfd, ARCID_BB_RES, NARC_balance_ball_upper_bg_NCLR, HEAPID_BB, FADE_SUB_BG, 0x20,
								  ( i + 1 ) * 16, pal_pos[ wk->p_client->comm_num - 1 ][ id ][ i + 1 ] * 16 );
		}
	}
}

static int BR_print_x_Get( GFL_BMPWIN* win, STRBUF* str, GFL_FONT *font_handle )
{
	int len = PRINTSYS_GetStrWidth( str, font_handle, 0 );
	int x	= ( GFL_BMPWIN_GetSizeX( win ) * 8 - len ) / 2;
	
	return x;
}
void BB_disp_NameWinAdd( BB_WORK* bwk, BB_CLIENT* wk )
{
	int i;
	int no = 0;
	int ofs = BB_CGX_OFS;
	
	s16 win_dat[][ 3 ][ 4 ] = {
		{ 
			{ 12, 8, 8, 2 },
			{ 0,0,0,0 },
			{ 0,0,0,0 },
		},
		{
			{  4+1, 8, 8, 2 },
			{ 18+1, 8, 8, 2 },
			{ 0,0,0,0 },
		},
		{
			{  0+1, 8, 8, 2 },
			{ 11+1, 8, 8, 2 },
			{ 22+1, 8, 8, 2 },
		},
	};
	int x,y,w,h;
	MYSTATUS* ms;
	BOOL bVip;
	
	switch ( wk->comm_num ){
	case 2:
		GFL_ARCHDL_UTIL_TransVramScreen( wk->sys->p_handle_bb, NARC_balance_ball_upper_bg_maku3_1_NSCR, GFL_BG_FRAME3_S, 0, 0, 0, HEAPID_BB );
		break;
	case 3:
		GFL_ARCHDL_UTIL_TransVramScreen( wk->sys->p_handle_bb, NARC_balance_ball_upper_bg_maku3_2_NSCR, GFL_BG_FRAME3_S, 0, 0, 0, HEAPID_BB );
		break;
	case 4:
		GFL_ARCHDL_UTIL_TransVramScreen( wk->sys->p_handle_bb, NARC_balance_ball_upper_bg_maku3_3_NSCR, GFL_BG_FRAME3_S, 0, 0, 0, HEAPID_BB );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	for ( i = 0; i < wk->comm_num; i++ ){
		
		if ( i == wk->netid ){ continue; }
		
		bVip = MNGM_ENRES_PARAM_GetVipFlag( &bwk->entry_param, i );
		
		//ms  = CommInfoGetMyStatus( i );
		ms = MNGM_ENRES_PARAM_GetMystatus( &bwk->entry_param, i );
	#if WB_FIX
		GF_BGL_BmpWinInit( wk->win_name[ no ] );
	#endif
		x = win_dat[ wk->comm_num - 2 ][ no ][ 0 ];
		y = win_dat[ wk->comm_num - 2 ][ no ][ 1 ];
		w = win_dat[ wk->comm_num - 2 ][ no ][ 2 ];
		h = win_dat[ wk->comm_num - 2 ][ no ][ 3 ];
		wk->win_name[ no ] = GFL_BMPWIN_Create( GFL_BG_FRAME3_S, x, y, w, h, 14, GFL_BMP_CHRAREA_GET_B );
		GFL_BMPWIN_MakeScreen(wk->win_name[no]);
		ofs += ( w * h );

		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win_name[ no ]), 0xFF );
		
		{
			STRBUF* str;
			int px;

			str	= MyStatus_CreateNameString( ms, HEAPID_BB );
			
			px = BR_print_x_Get( wk->win_name[ no ], str, bwk->font_handle );
			
		//	GF_STR_PrintSimple( wk->win_name[ no ], NET_FONT_SYSTEM, str, 0, 0, 0, NULL );
	#if WB_FIX
			if ( bVip ){
				GF_STR_PrintExpand( wk->win_name[ no ], NET_FONT_SYSTEM, str, px, 0, 0, BB_PRINT_COL_VIP, 0, 0, NULL );
			}
			else {
				GF_STR_PrintExpand( wk->win_name[ no ], NET_FONT_SYSTEM, str, px, 0, 0, BB_PRINT_COL, 0, 0, NULL );
			}
	#else
			if ( bVip ){
				PRINTSYS_PrintQueColor( bwk->printQue, GFL_BMPWIN_GetBmp(wk->win_name[ no ]), 
				  px, 0, str, bwk->font_handle, BB_PRINT_COL_VIP );
			}
			else {
				PRINTSYS_PrintQueColor( bwk->printQue, GFL_BMPWIN_GetBmp(wk->win_name[ no ]), 
				  px, 0, str, bwk->font_handle, BB_PRINT_COL );
			}
	#endif

			GFL_STR_DeleteBuffer( str );
		}

#if WB_FIX
		GF_BGL_BmpWinOn( wk->win_name[ no ] );
#else
	  GFL_BMPWIN_MakeTransWindow( wk->win_name[ no ] );
#endif

		no++;
	}
}

void BB_disp_NameWinDel( BB_CLIENT* wk )
{
	int i;
	int no = 0;
	
	for ( i = 0; i < wk->comm_num; i++ ){
		
		if ( i == wk->netid ){ continue; }		
		

	#if WB_FIX
		if( GF_BGL_BmpWinAddCheck( &wk->win_name[ no ] ) == TRUE ){
	#else
		if(wk->win_name[no] != NULL){
	#endif
			BmpWinFrame_Clear( wk->win_name[ no ], WINDOW_TRANS_OFF );
		    GFL_BMPWIN_ClearTransWindow( wk->win_name[ no ] );
			GFL_BMPWIN_Delete( wk->win_name[ no ] );
			wk->win_name[no] = NULL;
		}
		
		no++;
	}	
	GFL_ARCHDL_UTIL_TransVramScreen( wk->sys->p_handle_bb, NARC_balance_ball_upper_bg_maku3_NSCR, GFL_BG_FRAME3_S, 0, 0, 0, HEAPID_BB );
}


//--------------------------------------------------------------
/**
 * @brief	リソースの読み込み
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_ResourceLoad( BB_WORK* bb_wk, BB_CLIENT *wk )
{	
	ARCHANDLE*		 hdl_bb = bb_wk->sys.p_handle_bb;
	ARCHANDLE*		 hdl_cr = bb_wk->sys.p_handle_cr;
#if WB_FIX
	CATS_SYS_PTR	 csp	= bb_wk->sys.csp;
	CATS_RES_PTR	 crp	= bb_wk->sys.crp;
#endif
	PALETTE_FADE_PTR pfd	= bb_wk->sys.pfd;
	
	ARCHANDLE* hdl;
	int nca;
	int nce;
	int ncg;
	int ncl;
	int id;
	int palnum;
	
	int dest_palno;
	
	// ----- 上画面 -----
	
	hdl = hdl_bb;
    nca = NARC_balance_ball_upper_oam_NANR;
	nce = NARC_balance_ball_upper_oam_NCER;
	ncg = NARC_balance_ball_upper_oam_NCGR;
	ncl = NARC_balance_ball_upper_oam_NCLR;
	id  = eBB_ID_OAM_S;
	palnum = eBB_OAM_PAL_TD_MAX;

	wk->cgr_id[id] = GFL_CLGRP_CGR_Register(hdl, ncg, FALSE, CLSYS_DRAW_SUB, HEAPID_BB);
	wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(hdl, nce, nca, HEAPID_BB);
	wk->pltt_id[id] = PLTTSLOT_ResourceSet(
		bb_wk->sys.plttslot, hdl, ncl, CLSYS_DRAW_SUB, palnum, HEAPID_BB);
	dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_SUB) / 0x20;
	PaletteWorkSet_VramCopy(pfd, FADE_SUB_OBJ, dest_palno*16, palnum * 0x20);
	
	// ----- 下画面 -----
	// 星
    nca = NARC_balance_ball_eff_star_NANR;
	nce = NARC_balance_ball_eff_star_NCER;
	ncg = NARC_balance_ball_eff_star_NCGR;
	ncl = NARC_balance_ball_eff_star_NCLR;
	id  = eBB_ID_STAR;
	palnum = 1;

	wk->cgr_id[id] = GFL_CLGRP_CGR_Register(hdl, ncg, FALSE, CLSYS_DRAW_MAIN, HEAPID_BB);
	wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(hdl, nce, nca, HEAPID_BB);
	wk->pltt_id[id] = PLTTSLOT_ResourceSet(
		bb_wk->sys.plttslot, hdl, ncl, CLSYS_DRAW_MAIN, palnum, HEAPID_BB);
	dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_MAIN) / 0x20;
	PaletteWorkSet_VramCopy(pfd, FADE_MAIN_OBJ, dest_palno*16, palnum * 0x20);
	
	// 拍手
    nca = NARC_balance_ball_bottom_hakusyu_NANR;
	nce = NARC_balance_ball_bottom_hakusyu_NCER;
	ncg = NARC_balance_ball_bottom_hakusyu_NCGR ;
	ncl = NARC_balance_ball_bottom_hakusyu_NCLR;
	id  = eBB_ID_HAND;
	palnum = 1;

	wk->cgr_id[id] = GFL_CLGRP_CGR_Register(hdl, ncg, FALSE, CLSYS_DRAW_MAIN, HEAPID_BB);
	wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(hdl, nce, nca, HEAPID_BB);
	wk->pltt_id[id] = PLTTSLOT_ResourceSet(
		bb_wk->sys.plttslot, hdl, ncl, CLSYS_DRAW_MAIN, palnum, HEAPID_BB);
	dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_MAIN) / 0x20;
	PaletteWorkSet_VramCopy(pfd, FADE_MAIN_OBJ, dest_palno*16, palnum * 0x20);
	
	// 紙
    nca = NARC_balance_ball_bottom_kami_NANR;
	nce = NARC_balance_ball_bottom_kami_NCER;
	ncg = NARC_balance_ball_bottom_kami_NCGR ;
	ncl = NARC_balance_ball_bottom_kami_NCLR;
	id  = eBB_ID_KAMI;
	palnum = 1;

	wk->cgr_id[id] = GFL_CLGRP_CGR_Register(hdl, ncg, FALSE, CLSYS_DRAW_MAIN, HEAPID_BB);
	wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(hdl, nce, nca, HEAPID_BB);
	wk->pltt_id[id] = PLTTSLOT_ResourceSet(
		bb_wk->sys.plttslot, hdl, ncl, CLSYS_DRAW_MAIN, palnum, HEAPID_BB);
	dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_MAIN) / 0x20;
	PaletteWorkSet_VramCopy(pfd, FADE_MAIN_OBJ, dest_palno*16, palnum * 0x20);
	
	
	// 花火
    nca = NARC_balance_ball_bottom_hanabi_NANR;
	nce = NARC_balance_ball_bottom_hanabi_NCER;
	ncg = NARC_balance_ball_bottom_hanabi_NCGR ;
	ncl = NARC_balance_ball_bottom_hanabi_NCLR;
	id  = eBB_ID_HANABI;
	palnum = 1;

	wk->cgr_id[id] = GFL_CLGRP_CGR_Register(hdl, ncg, FALSE, CLSYS_DRAW_MAIN, HEAPID_BB);
	wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(hdl, nce, nca, HEAPID_BB);
	wk->pltt_id[id] = PLTTSLOT_ResourceSet(
		bb_wk->sys.plttslot, hdl, ncl, CLSYS_DRAW_MAIN, palnum, HEAPID_BB);
	dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_MAIN) / 0x20;
	PaletteWorkSet_VramCopy(pfd, FADE_MAIN_OBJ, dest_palno*16, palnum * 0x20);
	
	// ライト
    nca = NARC_balance_ball_bottom_oam_NANR;
	nce = NARC_balance_ball_bottom_oam_NCER;
	ncg = NARC_balance_ball_bottom_oam_NCGR;
	ncl = NARC_balance_ball_bottom_oam_NCLR;
	id  = eBB_ID_LIGHT_M;
	palnum = 3;

	wk->cgr_id[id] = GFL_CLGRP_CGR_Register(hdl, ncg, FALSE, CLSYS_DRAW_MAIN, HEAPID_BB);
	wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(hdl, nce, nca, HEAPID_BB);
	wk->pltt_id[id] = PLTTSLOT_ResourceSet(
		bb_wk->sys.plttslot, hdl, ncl, CLSYS_DRAW_MAIN, palnum, HEAPID_BB);
	dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_MAIN) / 0x20;
	PaletteWorkSet_VramCopy(pfd, FADE_MAIN_OBJ, dest_palno*16, palnum * 0x20);
	
	// ペン
	{
		ARCHANDLE* res_hdl = GFL_ARC_OpenDataHandle( ARCID_WLMNGM_TOOL_GRA, HEAPID_BB );

	    nca = NARC_wlmngm_tool_touchpen_NANR;
		nce = NARC_wlmngm_tool_touchpen_NCER;
		ncg = NARC_wlmngm_tool_touchpen_NCGR;
		ncl = NARC_wlmngm_tool_touchpen_NCLR;
		id  = eBB_ID_PEN;
		palnum = 2;
		wk->cgr_id[id] = GFL_CLGRP_CGR_Register(res_hdl, ncg, FALSE, CLSYS_DRAW_MAIN, HEAPID_BB);
		wk->cell_id[id] = GFL_CLGRP_CELLANIM_Register(res_hdl, nce, nca, HEAPID_BB);
		wk->pltt_id[id] = PLTTSLOT_ResourceSet(
			bb_wk->sys.plttslot, res_hdl, ncl, CLSYS_DRAW_MAIN, palnum, HEAPID_BB);
		dest_palno = GFL_CLGRP_PLTT_GetAddr(wk->pltt_id[id], CLSYS_DRAW_MAIN) / 0x20;
		PaletteWorkSet_VramCopy(pfd, FADE_MAIN_OBJ, dest_palno*16, palnum * 0x20);

		GFL_ARC_CloseDataHandle( res_hdl );
	}
}


void BB_disp_Manene_Add( BB_WORK* bb_wk, BB_CLIENT* wk )
{	
	GFL_CLWK_DATA coap;
#if WB_FIX
	CATS_SYS_PTR			 csp = wk->sys->csp;
	CATS_RES_PTR			 crp = wk->sys->crp;
#endif
	PALETTE_FADE_PTR		 pfd = wk->sys->pfd;
	
	int i;
	int no = 0;		
	
	for ( i = 0; i < wk->comm_num; i++ ){
		
		wk->netid_to_capid[ i ] = 0xFF;
		
		if ( i == wk->netid ){ continue; }
		
		wk->netid_to_capid[ i ] = no;
		
		BB_disp_COAP_SimpleInit( &coap, manene_pos[ wk->comm_num - 1 ][ no ] + BB_OAM_OX, BB_MANE_PY,// + Action_MoveValue_2Y(),
							 	 eBB_OAM_PAL_TD_MANENE, eBB_ID_OAM_S );
		coap.softpri = 1;
		wk->cap_mane[ no ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_OAM_S], wk->pltt_id[eBB_ID_OAM_S], wk->cell_id[eBB_ID_OAM_S], 
	    &coap,  CLSYS_DEFREND_SUB, HEAPID_BB );

		
		BB_disp_COAP_SimpleInit( &coap, manene_pos[ wk->comm_num - 1 ][ no ] + BB_OAM_OX, BB_BALL_PY, eBB_OAM_PAL_TD_MANENE, eBB_ID_OAM_S );
		coap.softpri = 2;
    wk->cap_ball[ no ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_OAM_S], wk->pltt_id[eBB_ID_OAM_S], wk->cell_id[eBB_ID_OAM_S], 
	    &coap,  CLSYS_DEFREND_SUB, HEAPID_BB );
		GFL_CLACT_WK_SetPlttOffs( wk->cap_ball[ no ], eBB_OAM_PAL_TD_MANENE + NetID_To_PlayerNo( bb_wk, i ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
		
		
#if WB_FIX
  		BB_disp_COAP_SimpleInit( &coap, manene_pos[ wk->comm_num - 1 ][ no ] + BB_OAM_OX, BB_BALL_PY, eBB_OAM_PAL_TD_MANENE, eBB_ID_OAM_S );
  		coap.softpri = 3;
      wk->cap_kage[ no ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
        wk->cgr_id[eBB_ID_OAM_S], wk->pltt_id[eBB_ID_OAM_S], wk->cell_id[eBB_ID_OAM_S], 
  	    &coap,  CLSYS_DEFREND_SUB, HEAPID_BB );
  		CATS_ObjectPaletteOffsetSetCap( wk->cap_kage[ no ], eBB_OAM_PAL_TD_MANENE + NetID_To_PlayerNo( bb_wk, i ) );
  		GFL_CLACT_WK_SetAnmSeq( wk->cap_kage[ no ], eANM_KAGE );

  		GFL_CLACT_WK_AddAnmFrame( wk->cap_mane[ no ], FX32_ONE*2 );
  		GFL_CLACT_WK_AddAnmFrame( wk->cap_ball[ no ], FX32_ONE*2 );
  		GFL_CLACT_WK_AddAnmFrame( wk->cap_kage[ no ], FX32_ONE*2 );
#else
      {
        GFL_CLWK_AFFINEDATA aff_coap = {
          {
            0, 0, 0, 3, 2,
          },
          0, 0, FX32_ONE, FX32_ONE, 0, CLSYS_AFFINETYPE_NONE,
        };
        aff_coap.clwkdata.pos_x = manene_pos[ wk->comm_num - 1 ][ no ] + BB_OAM_OX;
        aff_coap.clwkdata.pos_y = BB_BALL_PY;
    		
        wk->cap_kage[ no ] = GFL_CLACT_WK_CreateAffine( wk->sys->clunit, 
          wk->cgr_id[eBB_ID_OAM_S], wk->pltt_id[eBB_ID_OAM_S], wk->cell_id[eBB_ID_OAM_S], 
    	    &aff_coap,  CLSYS_DEFREND_SUB, HEAPID_BB );
        GFL_CLACT_WK_SetPlttOffs( wk->cap_kage[ no ], 
          eBB_OAM_PAL_TD_MANENE + NetID_To_PlayerNo( bb_wk, i ), CLWK_PLTTOFFS_MODE_OAM_COLOR );
    		GFL_CLACT_WK_SetAnmSeq( wk->cap_kage[ no ], eANM_KAGE );

    		GFL_CLACT_WK_AddAnmFrame( wk->cap_mane[ no ], FX32_ONE*2 );
    		GFL_CLACT_WK_AddAnmFrame( wk->cap_ball[ no ], FX32_ONE*2 );
    		GFL_CLACT_WK_AddAnmFrame( wk->cap_kage[ no ], FX32_ONE*2 );
      }
#endif
  		BB_disp_Manene_OAM_AnimeChange( wk, i, eANM_CODE_STOP );		
  		no++;
	}
}


//--------------------------------------------------------------
/**
 * @brief	スターダスト
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
GFL_CLWK* BB_disp_Stardust_Add( BB_CLIENT* wk, s16 x, s16 y )
{	
	GFL_CLWK*			 cap;
	GFL_CLWK_DATA coap;
#if WB_FIX
	CATS_SYS_PTR			 csp = wk->sys->csp;
	CATS_RES_PTR			 crp = wk->sys->crp;
#endif
	PALETTE_FADE_PTR		 pfd = wk->sys->pfd;
	
	BB_disp_COAP_SimpleInit( &coap, x, y, eBB_OAM_PAL_BD_STAR, eBB_ID_STAR );
	coap.bgpri = 0;
	coap.softpri	= 0;

  cap = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_STAR], wk->pltt_id[eBB_ID_STAR], wk->cell_id[eBB_ID_STAR], 
	    &coap,  CLSYS_DEFREND_MAIN, HEAPID_BB );

	GFL_CLACT_WK_AddAnmFrame( cap, FX32_ONE*2 );
	
	return cap;
}


//--------------------------------------------------------------
/**
 * @brief	ライト
 *
 * @param	wk
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Light_Add( BB_CLIENT* wk )
{
	int i;
	GFL_CLWK*			 cap;
	GFL_CLWK_DATA coap;
#if WB_FIX
	CATS_SYS_PTR			 csp = wk->sys->csp;
	CATS_RES_PTR			 crp = wk->sys->crp;
#endif
	PALETTE_FADE_PTR		 pfd = wk->sys->pfd;
	
	for ( i = 0; i < BB_LIGHT_MAX; i++ ){
		BB_disp_COAP_SimpleInit( &coap, 0, 0, eBB_OAM_PAL_BD_LIGHT, eBB_ID_LIGHT_M );
		coap.bgpri = 2;
		coap.softpri	= 20;
    wk->cap_light_m[ i ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_LIGHT_M], wk->pltt_id[eBB_ID_LIGHT_M], wk->cell_id[eBB_ID_LIGHT_M], 
	    &coap,  CLSYS_DEFREND_MAIN, HEAPID_BB );
		GFL_CLACT_WK_SetAnmSeq( wk->cap_light_m[ i ], 0 );	
		GFL_CLACT_WK_AddAnmFrame( wk->cap_light_m[ i ] , FX32_ONE);
//		GFL_CLACT_WK_SetPlttOffs( wk->cap_light_m[ i ], eBB_OAM_PAL_BD_LIGHT, CLWK_PLTTOFFS_MODE_OAM_COLOR );	
		GFL_CLACT_WK_SetObjMode( wk->cap_light_m[ i ], GX_OAM_MODE_XLU );
		GFL_CLACT_WK_SetDrawEnable( wk->cap_light_m[ i ], FALSE );

		BB_disp_COAP_SimpleInit( &coap, 0, 0, eBB_OAM_PAL_TD_MANENE, eBB_ID_OAM_S );
		coap.bgpri = 2;
		coap.softpri	= 20;
    wk->cap_light_s[ i ] = GFL_CLACT_WK_Create( wk->sys->clunit, 
      wk->cgr_id[eBB_ID_OAM_S], wk->pltt_id[eBB_ID_OAM_S], wk->cell_id[eBB_ID_OAM_S], 
	    &coap,  CLSYS_DEFREND_SUB, HEAPID_BB );
		GFL_CLACT_WK_SetAnmSeq( wk->cap_light_s[ i ], eANM_SPOT_LIGHT );	
		GFL_CLACT_WK_AddAnmFrame( wk->cap_light_s[ i ] , FX32_ONE);
//		GFL_CLACT_WK_SetPlttOffs( wk->cap_light_s[ i ], eBB_OAM_PAL_TD_MANENE, CLWK_PLTTOFFS_MODE_OAM_COLOR );
		GFL_CLACT_WK_SetObjMode( wk->cap_light_s[ i ], GX_OAM_MODE_XLU );
		GFL_CLACT_WK_SetDrawEnable( wk->cap_light_s[ i ], FALSE );
	}
}


//--------------------------------------------------------------
/**
 * @brief	ライト削除
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Light_Del( BB_CLIENT* wk )
{
	int i;

	for ( i = 0; i < BB_LIGHT_MAX; i++ ){
		GFL_CLACT_WK_Remove( wk->cap_light_m[ i ] );
		GFL_CLACT_WK_Remove( wk->cap_light_s[ i ] );
	}
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Pen_Add( BB_CLIENT* wk )
{
	int i;
	GFL_CLWK*			 cap;
	GFL_CLWK_DATA coap;
#if WB_FIX
	CATS_SYS_PTR			 csp = wk->sys->csp;
	CATS_RES_PTR			 crp = wk->sys->crp;
#endif
	PALETTE_FADE_PTR		 pfd = wk->sys->pfd;
	
	BB_disp_COAP_SimpleInit( &coap, 128, 128, eBB_OAM_PAL_BD_PEN, eBB_ID_PEN );
	coap.bgpri = 0;
	coap.softpri	= 0;
  wk->cap_pen = GFL_CLACT_WK_Create( wk->sys->clunit, 
    wk->cgr_id[eBB_ID_PEN], wk->pltt_id[eBB_ID_PEN], wk->cell_id[eBB_ID_PEN], 
    &coap,  CLSYS_DEFREND_MAIN, HEAPID_BB );
	GFL_CLACT_WK_SetAnmSeq( wk->cap_pen, 1 );
	GFL_CLACT_WK_SetDrawEnable( wk->cap_pen, TRUE );	
	GFL_CLACT_WK_AddAnmFrame( wk->cap_pen , FX32_ONE);
	GFL_CLACT_WK_SetPlttOffs( wk->cap_pen, eBB_OAM_PAL_BD_PEN, CLWK_PLTTOFFS_MODE_OAM_COLOR );
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_Pen_Del( BB_CLIENT* wk )
{
	GFL_CLACT_WK_Remove( wk->cap_pen );
}

//--------------------------------------------------------------
/**
 * @brief	BG の 展開
 *
 * @param	wk	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BB_disp_BG_Load( BB_WORK* wk )
{
	ARCHANDLE*		 hdl_bb = wk->sys.p_handle_bb;
	ARCHANDLE*		 hdl_cr = wk->sys.p_handle_cr;
#if WB_FIX
	CATS_SYS_PTR	 csp	= wk->sys.csp;
	CATS_RES_PTR	 crp	= wk->sys.crp;
#endif
	PALETTE_FADE_PTR pfd	= wk->sys.pfd;
	
	ARCHANDLE* hdl = hdl_bb;
	
	// ----- 上画面 -----	
	GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl, NARC_balance_ball_upper_bg_NCGR,   	GFL_BG_FRAME0_S, 0, 0, 0, HEAPID_BB );		///< 背景
	GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl, NARC_balance_ball_upper_bg_maku_NCGR, GFL_BG_FRAME1_S, 0, 0, 0, HEAPID_BB );		///< 幕
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_upper_bg_00_NSCR,  	GFL_BG_FRAME0_S, 0, 0, 0, HEAPID_BB );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_upper_bg_maku1_NSCR, 	GFL_BG_FRAME1_S, 0, 0, 0, HEAPID_BB );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_upper_bg_maku2_NSCR, 	GFL_BG_FRAME2_S, 0, 0, 0, HEAPID_BB );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_upper_bg_maku3_NSCR, 	GFL_BG_FRAME3_S, 0, 0, 0, HEAPID_BB );
	PaletteWorkSet_Arc( pfd, ARCID_BB_RES, NARC_balance_ball_upper_bg_NCLR, 	HEAPID_BB, FADE_SUB_BG, 0x20 * 5, 0 );	///< 4本分

	// ----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl, NARC_balance_ball_bottom_bg_NCGR,   	GFL_BG_FRAME3_M, 0, 0, 0, HEAPID_BB );		///< 背景
	GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl, NARC_balance_ball_bottom_bg_maku_NCGR,GFL_BG_FRAME1_M, 0, 0, 0, HEAPID_BB );		///< 幕
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_bottom_bg_NSCR,  		GFL_BG_FRAME3_M, 0, 0, 0, HEAPID_BB );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_bottom_bg_maku1_NSCR, GFL_BG_FRAME1_M, 0, 0, 0, HEAPID_BB );
	GFL_ARCHDL_UTIL_TransVramScreen( hdl, NARC_balance_ball_bottom_bg_maku2_NSCR, GFL_BG_FRAME2_M, 0, 0, 0, HEAPID_BB );
	PaletteWorkSet_Arc( pfd, ARCID_BB_RES, NARC_balance_ball_bottom_bg_NCLR, 	HEAPID_BB, FADE_MAIN_BG, 0x20, 0 );

	// ----- フォント -----
	PaletteWorkSet_Arc( pfd, ARCID_FONT, NARC_font_default_nclr, HEAPID_BB, FADE_SUB_BG, 0x20, 14 * 16);
	
	// ----- ウィンドウ -----
#if WB_FIX
	PaletteWorkSet_Arc( pfd, ARC_WINFRAME, MenuWinPalArcGet(), HEAPID_BB, FADE_SUB_BG, 0x20, 13 * 16);	
#else
	PaletteWorkSet_Arc( pfd, ARCID_FLDMAP_WINFRAME, BmpWinFrame_WinPalArcGet(), HEAPID_BB, FADE_SUB_BG, 0x20, 13 * 16);	
#endif
	BmpWinFrame_GraphicSet( GFL_BG_FRAME3_S, BB_BG_CGX_OFS, 13, 0, HEAPID_BB );
}
