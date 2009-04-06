
//============================================================================================
/**
 * @file	btlv_stage.c
 * @brief	戦闘画面描画用関数（お盆）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================
#include <gflib.h>

#include "btlv_stage.h"

#include "arc_def.h"
#include "battgra/battgra_wb.naix"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	BTLV_STAGE_DEFAULT_SCALE		( FX32_ONE )	//お盆のデフォルトスケール値
#define	BTLV_STAGE_ANM_MAX			( 0 )			//お盆のアニメーション数
													//（初期化関数から１の時は０を要求されているので実際は１）
#define	BTLV_STAGE_ANMTBL_MAX		( 1 )			//お盆のアニメーションテーブル数

#define	BTLV_STAGE_ANM_NO			( 0 )			//お盆のアニメーションナンバー
#define	BTLV_STAGE_ANM_WAIT			( FX32_ONE )	//お盆のアニメーションウェイト

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTLV_STAGE_WORK
{
	GFL_G3D_RES			*stage_resource;
	GFL_G3D_RES			*stage_anm_resource;
	GFL_G3D_ANM			*stage_anm;
	GFL_G3D_RND			*stage_render;
	GFL_G3D_OBJ			*stage_obj;
	GFL_G3D_OBJSTATUS	stage_status[ BTLV_STAGE_MAX ];
	HEAPID				heapID;
};

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

BTLV_STAGE_WORK	*BTLV_STAGE_Init( int index, HEAPID heapID );
void			BTLV_STAGE_Exit( BTLV_STAGE_WORK *bsw );
void			BTLV_STAGE_Main( BTLV_STAGE_WORK *bsw );
void			BTLV_STAGE_Draw( BTLV_STAGE_WORK *bsw );

//============================================================================================
/**
 *	お盆の位置テーブル
 */
//============================================================================================
static	const	VecFx32	stage_pos_table[]={
//	{ FX_F32_TO_FX32( -3.845f ), 0, FX_F32_TO_FX32(   4.702f ) },
//	{ FX_F32_TO_FX32(  4.964f ), 0, FX_F32_TO_FX32( -12.540f ) },
	{ 0, 0, FX_F32_TO_FX32( 7.345f - 0.5f ) },
	{ 0, 0, FX_F32_TO_FX32( -12.0f ) },
};

//============================================================================================
/**
 *	お盆のリソーステーブル
 */
//============================================================================================
//モデルデータ
static	const	int	stage_resource_table[]={
//	NARC_battgra_wb_batt_stage01_nsbmd,
	NARC_battgra_wb_batt_stage02_nsbmd,
//	NARC_battgra_wb_batt_stage03_nsbmd,
};

//アニメデータ
static	const	int	stage_anm_resource_table[]={
//	NARC_battgra_wb_batt_stage01_nsbca,
	NARC_battgra_wb_batt_stage02_nsbca,
	NULL,
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
BTLV_STAGE_WORK	*BTLV_STAGE_Init( int index, HEAPID heapID )
{
	BTLV_STAGE_WORK *bsw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_STAGE_WORK ) );
	BOOL	ret;

	GF_ASSERT( index < NELEMS( stage_resource_table ) );
	GF_ASSERT( index < NELEMS( stage_anm_resource_table ) );

	bsw->heapID = heapID;

	//リソース読み込み
	bsw->stage_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, stage_resource_table[ index ] );
	if( stage_anm_resource_table[ index ] ){
		bsw->stage_anm_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, stage_anm_resource_table[ index ] );
	}
	ret = GFL_G3D_TransVramTexture( bsw->stage_resource );
	GF_ASSERT( ret == TRUE );

	//RENDER生成
	bsw->stage_render = GFL_G3D_RENDER_Create( bsw->stage_resource, 0, bsw->stage_resource );

	if(	bsw->stage_anm_resource ){
		//ANIME生成
		bsw->stage_anm = GFL_G3D_ANIME_Create( bsw->stage_render, bsw->stage_anm_resource, BTLV_STAGE_ANM_MAX ); 

		//OBJ生成
		bsw->stage_obj = GFL_G3D_OBJECT_Create( bsw->stage_render, &bsw->stage_anm, BTLV_STAGE_ANMTBL_MAX );

		//ANIME起動
		GFL_G3D_OBJECT_EnableAnime( bsw->stage_obj, BTLV_STAGE_ANM_NO );
	}
	else{
		bsw->stage_obj = GFL_G3D_OBJECT_Create( bsw->stage_render, NULL, 0 );
	}

	//自分側お盆
	bsw->stage_status[ BTLV_STAGE_MINE ].trans.x = stage_pos_table[ BTLV_STAGE_MINE ].x;
	bsw->stage_status[ BTLV_STAGE_MINE ].trans.y = stage_pos_table[ BTLV_STAGE_MINE ].y;
	bsw->stage_status[ BTLV_STAGE_MINE ].trans.z = stage_pos_table[ BTLV_STAGE_MINE ].z;
	bsw->stage_status[ BTLV_STAGE_MINE ].scale.x = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_MINE ].scale.y = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_MINE ].scale.z = BTLV_STAGE_DEFAULT_SCALE;
	MTX_Identity33( &bsw->stage_status[ BTLV_STAGE_MINE ].rotate );

	//相手側お盆
	bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.x = stage_pos_table[ BTLV_STAGE_ENEMY ].x;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.y = stage_pos_table[ BTLV_STAGE_ENEMY ].y;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.z = stage_pos_table[ BTLV_STAGE_ENEMY ].z;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].scale.x = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].scale.y = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].scale.z = BTLV_STAGE_DEFAULT_SCALE;
	MTX_Identity33( &bsw->stage_status[ BTLV_STAGE_ENEMY ].rotate );

	//エッジマーキングカラーセット
	G3X_SetEdgeColorTable( &stage_edge_color_table[ index ][ 0 ] );

	return bsw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bsw	BTLV_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_STAGE_Exit( BTLV_STAGE_WORK *bsw )
{
	GFL_G3D_OBJECT_Delete( bsw->stage_obj );
	if(	bsw->stage_anm_resource ){
		GFL_G3D_ANIME_Delete( bsw->stage_anm );
	}
	GFL_G3D_RENDER_Delete( bsw->stage_render );
	GFL_G3D_DeleteResource( bsw->stage_resource );
	if(	bsw->stage_anm_resource ){
		GFL_G3D_DeleteResource( bsw->stage_anm_resource );
	}

	GFL_HEAP_FreeMemory( bsw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bsw	BTLV_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_STAGE_Main( BTLV_STAGE_WORK *bsw )
{
	if(	bsw->stage_anm_resource ){
		GFL_G3D_OBJECT_LoopAnimeFrame( bsw->stage_obj, BTLV_STAGE_ANM_NO, BTLV_STAGE_ANM_WAIT ); 
	}
}

//============================================================================================
/**
 *	描画
 *
 * @param[in]	bsw	BTLV_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_STAGE_Draw( BTLV_STAGE_WORK *bsw )
{
	int	i;

	for( i = 0 ; i < BTLV_STAGE_MAX ; i++ ){
		GFL_G3D_DRAW_DrawObject( bsw->stage_obj, &bsw->stage_status[ i ] );
	}
}

