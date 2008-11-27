
//============================================================================================
/**
 * @file	btl_stage.c
 * @brief	戦闘画面描画用関数（お盆）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================
#include <gflib.h>

#include "btl_stage.h"
#include "btl_stage_def.h"

#include "arc_def.h"
#include "battgra/battgra_wb.naix"

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTL_STAGE_WORK
{
	GFL_G3D_RES			*stage_resource;
	GFL_G3D_RES			*stage_anm_resource;
	GFL_G3D_ANM			*stage_anm;
	GFL_G3D_RND			*stage_render;
	GFL_G3D_OBJ			*stage_obj;
	GFL_G3D_OBJSTATUS	stage_status[ BTL_STAGE_MAX ];
	HEAPID				heapID;
};

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

BTL_STAGE_WORK	*BTL_STAGE_Init( int index, HEAPID heapID );
void			BTL_STAGE_Exit( BTL_STAGE_WORK *bsw );
void			BTL_STAGE_Main( BTL_STAGE_WORK *bsw );
void			BTL_STAGE_Draw( BTL_STAGE_WORK *bsw );

//============================================================================================
/**
 *	お盆の位置テーブル
 */
//============================================================================================
static	const	VecFx32	stage_pos_table[]={
	{ FX_F32_TO_FX32( -3.845f ), 0, FX_F32_TO_FX32(   4.702f ) },
	{ FX_F32_TO_FX32(  4.964f ), 0, FX_F32_TO_FX32( -12.540f ) },
};

//============================================================================================
/**
 *	お盆のリソーステーブル
 */
//============================================================================================
//モデルデータ
static	const	int	stage_resource_table[]={
	NARC_battgra_wb_batt_stage01_nsbmd,
};

//アニメデータ
static	const	int	stage_anm_resource_table[]={
	NARC_battgra_wb_batt_stage01_nsbca,
};

//エッジマーキングカラー
static	const	GXRgb stage_edge_color_table[][8]={
	{ GX_RGB( 0, 0, 0 ), GX_RGB( 6, 6, 6 ), 0, 0, 0, 0, 0, 0 },
};

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	index	読み込むリソースのINDEX
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
BTL_STAGE_WORK	*BTL_STAGE_Init( int index, HEAPID heapID )
{
	BTL_STAGE_WORK *bsw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTL_STAGE_WORK ) );
	BOOL	ret;

	GF_ASSERT( index < NELEMS( stage_resource_table ) );
	GF_ASSERT( index < NELEMS( stage_anm_resource_table ) );

	bsw->heapID = heapID;

	//リソース読み込み
	bsw->stage_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, stage_resource_table[ index ] );
	bsw->stage_anm_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, stage_anm_resource_table[ index ] );
	ret = GFL_G3D_TransVramTexture( bsw->stage_resource );
	GF_ASSERT( ret == TRUE );

	//RENDER生成
	bsw->stage_render = GFL_G3D_RENDER_Create( bsw->stage_resource, 0, bsw->stage_resource );

	//ANIME生成
	bsw->stage_anm = GFL_G3D_ANIME_Create( bsw->stage_render, bsw->stage_anm_resource, BTL_STAGE_ANM_MAX ); 

	//OBJ生成
	bsw->stage_obj = GFL_G3D_OBJECT_Create( bsw->stage_render, &bsw->stage_anm, BTL_STAGE_ANMTBL_MAX );

	//ANIME起動
	GFL_G3D_OBJECT_EnableAnime( bsw->stage_obj, BTL_STAGE_ANM_NO );

	//自分側お盆
	bsw->stage_status[ BTL_STAGE_MINE ].trans.x = stage_pos_table[ BTL_STAGE_MINE ].x;
	bsw->stage_status[ BTL_STAGE_MINE ].trans.y = stage_pos_table[ BTL_STAGE_MINE ].y;
	bsw->stage_status[ BTL_STAGE_MINE ].trans.z = stage_pos_table[ BTL_STAGE_MINE ].z;
	bsw->stage_status[ BTL_STAGE_MINE ].scale.x = BTL_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTL_STAGE_MINE ].scale.y = BTL_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTL_STAGE_MINE ].scale.z = BTL_STAGE_DEFAULT_SCALE;
	MTX_Identity33( &bsw->stage_status[ BTL_STAGE_MINE ].rotate );

	//相手側お盆
	bsw->stage_status[ BTL_STAGE_ENEMY ].trans.x = stage_pos_table[ BTL_STAGE_ENEMY ].x;
	bsw->stage_status[ BTL_STAGE_ENEMY ].trans.y = stage_pos_table[ BTL_STAGE_ENEMY ].y;
	bsw->stage_status[ BTL_STAGE_ENEMY ].trans.z = stage_pos_table[ BTL_STAGE_ENEMY ].z;
	bsw->stage_status[ BTL_STAGE_ENEMY ].scale.x = BTL_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTL_STAGE_ENEMY ].scale.y = BTL_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTL_STAGE_ENEMY ].scale.z = BTL_STAGE_DEFAULT_SCALE;
	MTX_Identity33( &bsw->stage_status[ BTL_STAGE_ENEMY ].rotate );

	//エッジマーキングカラーセット
	G3X_SetEdgeColorTable( &stage_edge_color_table[ index ][ 0 ] );

	return bsw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bsw	BTL_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTL_STAGE_Exit( BTL_STAGE_WORK *bsw )
{
	GFL_G3D_DeleteResource( bsw->stage_resource );
	GFL_G3D_DeleteResource( bsw->stage_anm_resource );
	GFL_G3D_RENDER_Delete( bsw->stage_render );
	GFL_G3D_ANIME_Delete( bsw->stage_anm );
	GFL_G3D_OBJECT_Delete( bsw->stage_obj );

	GFL_HEAP_FreeMemory( bsw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bsw	BTL_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTL_STAGE_Main( BTL_STAGE_WORK *bsw )
{
	GFL_G3D_OBJECT_LoopAnimeFrame( bsw->stage_obj, BTL_STAGE_ANM_NO, BTL_STAGE_ANM_WAIT ); 
}

//============================================================================================
/**
 *	描画
 *
 * @param[in]	bsw	BTL_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTL_STAGE_Draw( BTL_STAGE_WORK *bsw )
{
	int	i;

	for( i = 0 ; i < BTL_STAGE_MAX ; i++ ){
		GFL_G3D_DRAW_DrawObject( bsw->stage_obj, &bsw->stage_status[ i ] );
	}
}

