//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wflby_3dmapobj.c
 *	@brief		マップ表示物管理
 *	@author		tomoya takahashi
 *	@data		2007.11.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>


#include "wifi_lobby.naix"

#include "wflby_3dmapobj.h"
#include "wflby_3dmatrix.h"
#include "wflby_3dmapobj_data.h"
#include "wflby_maparc.h"

#include "arc_def.h"
#include "system/gfl_use.h"


//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------

#ifdef PM_DEBUG
//#define WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME	// 処理速度を表示
#endif

#ifdef WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME

static OSTick	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_Tick;
#define WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_INIT	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_Tick = OS_GetTick();
#define WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT(line)	OS_TPrintf( "	od line[%d] time %d micro\n", (line), OS_TicksToMicroSeconds(OS_GetTick() - WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_Tick) );

#else		// WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME

#define		WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_INIT			((void)0);
#define		WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT(line)	((void)0);

#endif		// WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------




//-------------------------------------
///	地面ワーク
//=====================================
typedef struct _WFLBY_3DMAPOBJ_MAP{
	BOOL		on;
	GFL_G3D_OBJSTATUS		objst[ WFLBY_3DMAPOBJ_MAPOBJ_NUM ];
	VecFx32 objrotate[ WFLBY_3DMAPOBJ_MAPOBJ_NUM ];
	GFL_G3D_OBJ *g3dobj[ WFLBY_3DMAPOBJ_MAPOBJ_NUM ];
	BOOL draw_flag[WFLBY_3DMAPOBJ_MAPOBJ_NUM];
	u32			anm_on[ WFLBY_3DMAPOBJ_MAP_ANM_NUM ];
	fx32		anm_frame[ WFLBY_3DMAPOBJ_MAP_ANM_NUM ];
}WFLBY_3DMAPOBJ_MAP;

//-------------------------------------
///	地面リソース
//=====================================
typedef struct {
	GFL_G3D_RND *rnder[ WFLBY_3DMAPOBJ_MAPOBJ_NUM ];
	GFL_G3D_RES *p_mdlres[ WFLBY_3DMAPOBJ_MAPOBJ_NUM ];
	GFL_G3D_RES	*p_anmres[ WFLBY_3DMAPOBJ_MAP_ANM_NUM ];
	GFL_G3D_ANM	*anm[ WFLBY_3DMAPOBJ_MAP_ANM_NUM ];
	BOOL		anm_load[ WFLBY_3DMAPOBJ_MAP_ANM_NUM ];
} WFLBY_3DMAPOBJ_MAPRES;


//-------------------------------------
///	フロートワーク
//=====================================
struct _WFLBY_3DMAPOBJ_FLOAT{
	u8			on;	
	u8			col;	// 色
	u8			mdlno;	// モデルナンバー
	u8			pad;
	GFL_G3D_OBJSTATUS		objst;
	VecFx32		objrotate;
	GFL_G3D_OBJ	*g3dobj;
	u32			anm_on[ WFLBY_3DMAPOBJ_FLOAT_ANM_NUM ];
	fx32		anm_frame[ WFLBY_3DMAPOBJ_FLOAT_ANM_NUM ];
	VecFx32		mat;
	VecFx32		ofs;
	u8			draw_flag;
	u8			padding[3];
};

//-------------------------------------
///	フロートリソース
//=====================================
typedef struct {
	GFL_G3D_RND	*rnder[ WFLBY_3DMAPOBJ_FLOAT_NUM ];
	GFL_G3D_RES *p_texres[ WFLBY_3DMAPOBJ_FLOAT_NUM ][ WFLBY_3DMAPOBJ_FLOAT_COL_NUM ];
	GFL_G3D_RES *p_mdlres[ WFLBY_3DMAPOBJ_FLOAT_NUM ];
	GFL_G3D_RES *p_anmres[ WFLBY_3DMAPOBJ_FLOAT_NUM ][ WFLBY_3DMAPOBJ_FLOAT_ANM_NUM ];
	GFL_G3D_ANM	*anm[ WFLBY_3DMAPOBJ_FLOAT_NUM ][ WFLBY_3DMAPOBJ_FLOAT_ANM_NUM ];
} WFLBY_3DMAPOBJ_FLOATRES;

//-------------------------------------
///	その他の物ワーク
//	(表示非表示くらいしか出来ない)
//=====================================
struct _WFLBY_3DMAPOBJ_WK{
	u16			on;
	u16			mdlid;
	GFL_G3D_OBJSTATUS		objst;
	VecFx32		objrotate;
	GFL_G3D_OBJ	*g3dobj;
	GFL_G3D_ANM	*anm[ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
	u8			anm_on[ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
	u8			anm_on_pad;
	u8			play[ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
	u8			play_pad;
	fx32		anm_frame[ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
	u8			wait_def;							// ランダムウエイト定数	
	u8			wait[ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];	// ウエイト数
	pWFLBY_3DMAPOBJ_WK_AnmCallBack	p_anmcallback[ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
	fx32		speed;		// アニメスピード
	u8			alpha_flag;	// アルファ値設定フラグ
	u8			alpha;		// アルファ値
	u8			def_alpha;	// 基本アルファ値
	u8			pad;	// 基本アルファ値
	
	u8			draw_flag;
	u8			padding[3];
};

//-------------------------------------
///	その他の配置オブジェリソース
//=====================================
typedef struct {
	GFL_G3D_RND *rnder[ WFLBY_3DMAPOBJ_WK_NUM ];
	GFL_G3D_RES *p_mdlres[ WFLBY_3DMAPOBJ_WK_NUM ];
#if WB_FIX
	void*		p_anm[ WFLBY_3DMAPOBJ_WK_NUM ][ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
#else
	GFL_G3D_RES* p_anmres[ WFLBY_3DMAPOBJ_WK_NUM ][ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
	GFL_G3D_ANM* anm[ WFLBY_3DMAPOBJ_WK_NUM ][ WFLBY_3DMAPOBJ_ALL_ANM_MAX ];
#endif
} WFLBY_3DMAPOBJ_WKRES;

//-------------------------------------
///	マップ表示物管理システム
//=====================================
struct _WFLBY_3DMAPOBJ{
	WFLBY_3DMAPOBJ_MAP		map;		// マップワーク	
	WFLBY_3DMAPOBJ_FLOAT*	p_float;	// フロート
	WFLBY_3DMAPOBJ_WK*		p_obj;		// 配置オブジェ
	u8						floatnum;	// フロート数
	u8						objnum;		// 配置オブジェ数
	u8						room;		// 部屋タイプ
	u8						season;		// シーズンタイプ

	BOOL						res_load;								// リソースを読み込んだか
	WFLBY_3DMAPOBJ_MAPRES		mapres;									// マップワーク	
	WFLBY_3DMAPOBJ_FLOATRES		floatres;								// フロート
	WFLBY_3DMAPOBJ_WKRES		objres;									// 配置オブジェ
#if WB_FIX
	NNSFndAllocator				allocator;								// アロケータ
#endif
};



//-----------------------------------------------------------------------------
/**
 *				配置オブジェクトデータ
 */
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// シーズンの部屋の読み込みデータ作成
static WFLBY_3DMAPOBJ_MDL_DATA* WFLBY_3DMAPOBJ_MDLRES_DATA_Init( WFLBY_ROOM_TYPE room, WFLBY_SEASON_TYPE season, u32 heapID );
static void WFLBY_3DMAPOBJ_MDLRES_DATA_Exit( WFLBY_3DMAPOBJ_MDL_DATA* p_data );


// 広場用モデル読み込みシステム
static void WFLBY_3DMAPOBJ_MDL_Load( GFL_G3D_RES** p_res, ARCHANDLE* p_handle, u32 data_idx, u32 gheapID );
static void WFLBY_3DMAPOBJ_MDL_Delete( GFL_G3D_RES* p_mdl );

// アニメフレーム管理
static void WFLBY_3DMAPOBJ_ANM_Loop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed );
static BOOL WFLBY_3DMAPOBJ_ANM_NoLoop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed );
static void WFLBY_3DMAPOBJ_ANM_BackLoop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed );
static BOOL WFLBY_3DMAPOBJ_ANM_BackNoLoop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed );

// マップワーク
static void WFLBY_3DMAPOBJ_MAP_Load( WFLBY_3DMAPOBJ_MAPRES* p_wk, ARCHANDLE* p_handle, const WFLBY_3DMAPOBJ_MDL_DATA* cp_data, u32 gheapID );
static void WFLBY_3DMAPOBJ_MAP_Release( WFLBY_3DMAPOBJ_MAPRES* p_wk );
static void WFLBY_3DMAPOBJ_MAP_Add( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res );
static void WFLBY_3DMAPOBJ_MAP_Delete( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res );
static void WFLBY_3DMAPOBJ_MAP_Main( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res );
static void WFLBY_3DMAPOBJ_MAP_Draw( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res );

// フロートワーク
static WFLBY_3DMAPOBJ_FLOAT* WFLBY_3DMAPOBJ_FLOAT_GetCleanWk( WFLBY_3DMAPOBJ* p_sys );
static void WFLBY_3DMAPOBJ_FLOAT_Load( WFLBY_3DMAPOBJ_FLOATRES* p_wk, ARCHANDLE* p_handle, const WFLBY_3DMAPOBJ_MDL_DATA* cp_data, u32 gheapID );
static void WFLBY_3DMAPOBJ_FLOAT_Release( WFLBY_3DMAPOBJ_FLOATRES* p_wk );
static void WFLBY_3DMAPOBJ_FLOAT_Main( WFLBY_3DMAPOBJ_FLOAT* p_wk, WFLBY_3DMAPOBJ_FLOATRES* p_res );
static void WFLBY_3DMAPOBJ_FLOAT_Draw( WFLBY_3DMAPOBJ_FLOAT* p_wk, WFLBY_3DMAPOBJ_FLOATRES* p_res );

// 配置オブジェクトワーク
static WFLBY_3DMAPOBJ_WK* WFLBY_3DMAPOBJ_WK_GetCleanWk( WFLBY_3DMAPOBJ* p_sys );
static void WFLBY_3DMAPOBJ_WK_Load( WFLBY_3DMAPOBJ_WKRES* p_wk, ARCHANDLE* p_handle, const WFLBY_3DMAPOBJ_MDL_DATA* cp_data, u32 gheapID );
static void WFLBY_3DMAPOBJ_WK_Release( WFLBY_3DMAPOBJ_WKRES* p_wk );
static void WFLBY_3DMAPOBJ_WK_Draw( WFLBY_3DMAPOBJ_WKRES* p_res, WFLBY_3DMAPOBJ_WK* p_wk );
static void WFLBY_3DMAPOBJ_WK_Anm( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk );

static void WFLBY_G3DOBJSTATUS_Init(GFL_G3D_OBJSTATUS *status, VecFx32 *rotate);
static void WFLBY_RotateMtx(VecFx32 *rotate, MtxFx33 *dst_mtx);


// システム管理
//----------------------------------------------------------------------------
/**
 *	@brief	マップオブジェシステム	初期化
 *
 *	@param	float_num		フロート数
 *	@param	objwk_num		そのたの表示物数
 *	@param	heapID			ヒープID
 *	@param	gheapID			グラフィックヒープID
 *
 *	@return	システムワーク
 */
//-----------------------------------------------------------------------------
WFLBY_3DMAPOBJ* WFLBY_3DMAPOBJ_Init( u32 float_num, u32 objwk_num, u32 heapID, u32 gheapID )
{
	WFLBY_3DMAPOBJ* p_sys;

	p_sys = GFL_HEAP_AllocMemory( heapID, sizeof(WFLBY_3DMAPOBJ) );
	GFL_STD_MemFill( p_sys,  0, sizeof(WFLBY_3DMAPOBJ) );

	// 各ワークを作成
	p_sys->p_float	= GFL_HEAP_AllocMemory( heapID, sizeof(WFLBY_3DMAPOBJ_FLOAT)*float_num );
	p_sys->p_obj	= GFL_HEAP_AllocMemory( heapID, sizeof(WFLBY_3DMAPOBJ_WK)*objwk_num );
	GFL_STD_MemFill( p_sys->p_float, 0, sizeof(WFLBY_3DMAPOBJ_FLOAT)*float_num );
	GFL_STD_MemFill( p_sys->p_obj, 0, sizeof(WFLBY_3DMAPOBJ_WK)*objwk_num );
	p_sys->floatnum	= float_num;
	p_sys->objnum	= objwk_num;
	
	
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップオブジェシステム	破棄
 *
 *	@param	p_sys		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_Exit( WFLBY_3DMAPOBJ* p_sys )
{
	// データを読み込み済みなら破棄する
	if( p_sys->res_load ){
		WFLBY_3DMAPOBJ_ResRelease( p_sys );
	}

	// 全メモリを破棄
	GFL_HEAP_FreeMemory( p_sys->p_float );
	GFL_HEAP_FreeMemory( p_sys->p_obj );
	GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップオブジェシステム	メイン処理
 *
 *	@param	p_sys		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_Main( WFLBY_3DMAPOBJ* p_sys )
{
	// アニメメイン
	
	//  マップメイン
	WFLBY_3DMAPOBJ_MAP_Main( &p_sys->map, &p_sys->mapres );
	
	// フロートメイン
	{
		int i;

		for( i=0; i<p_sys->floatnum; i++ ){
//			OS_TPrintf( "float idx=%d ",  i );
			WFLBY_3DMAPOBJ_FLOAT_Main( &p_sys->p_float[i], &p_sys->floatres );
		}
	}

	// 配置オブジェメイン
	{
		int i;

		for( i=0; i<p_sys->objnum; i++ ){
			WFLBY_3DMAPOBJ_WK_Anm( p_sys, &p_sys->p_obj[i] );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップオブジェシステム	描画処理
 *
 *	@param	p_sys		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_Draw( WFLBY_3DMAPOBJ* p_sys )
{
	int i;
	GF_ASSERT( p_sys );
	GF_ASSERT( p_sys->p_obj );
	GF_ASSERT( p_sys->p_float );

//	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_INIT;
	
	// マップ描画
	WFLBY_3DMAPOBJ_MAP_Draw( &p_sys->map, &p_sys->mapres );
//	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT( __LINE__ );
	
	// 配置オブジェクト描画
	for( i=0; i<p_sys->objnum; i++ ){
		WFLBY_3DMAPOBJ_WK_Draw( &p_sys->objres, &p_sys->p_obj[i] );
	}
//	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT( __LINE__ );
	
	// フロート描画
	for( i=0; i<p_sys->floatnum; i++ ){
//		OS_TPrintf( "float idx=%d ",  i );
		WFLBY_3DMAPOBJ_FLOAT_Draw( &p_sys->p_float[i], &p_sys->floatres );
	}
//	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT( __LINE__ );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップオブジェシステム	VBLANK処理
 *
 *	@param	p_sys		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_VBlank( WFLBY_3DMAPOBJ* p_sys )
{
	// VBlank関数
}

// 部屋のリソース
//----------------------------------------------------------------------------
/**
 *	@brief	部屋のリソースを読み込む
 *
 *	@param	p_sys		システムワーク
 *	@param	room		部屋タイプ
 *	@param	season		シーズンタイプ
 *	@param	heapID		ヒープID
 *	@param	gheapID		グラフィックヒープID
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_ResLoad( WFLBY_3DMAPOBJ* p_sys, WFLBY_ROOM_TYPE room, WFLBY_SEASON_TYPE season, u32 heapID, u32 gheapID )
{
	ARCHANDLE* p_handle;
	WFLBY_3DMAPOBJ_MDL_DATA* p_data;

	// シーズンと部屋を保存
	p_sys->season	= season;
	p_sys->room		= room;

	// モデル読み込みデータを取得する
	p_data = WFLBY_3DMAPOBJ_MDLRES_DATA_Init( room, season, heapID );
	
	// シーズンの各リソースを読み込む
	p_handle = GFL_ARC_OpenDataHandle( ARCID_WFLBY_GRA, heapID );	// ハンドルオープン

#if WB_FIX
	// アロケータを作成
	sys_InitAllocator( &p_sys->allocator, gheapID, 4 );
#endif

	// マップ
	WFLBY_3DMAPOBJ_MAP_Load( &p_sys->mapres, p_handle, p_data, gheapID );

	// 配置オブジェクト
	WFLBY_3DMAPOBJ_WK_Load( &p_sys->objres, p_handle, p_data, gheapID );


	// フロートオブジェクト
	WFLBY_3DMAPOBJ_FLOAT_Load( &p_sys->floatres, p_handle, p_data, gheapID );

	GFL_ARC_CloseDataHandle( p_handle );
	
	// 破棄
	WFLBY_3DMAPOBJ_MDLRES_DATA_Exit( p_data );	

	// マップはすでに登録する
	WFLBY_3DMAPOBJ_MAP_Add( &p_sys->map, &p_sys->mapres );

	p_sys->res_load = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋リソースを破棄する
 *
 *	@param	p_sys	システムワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_ResRelease( WFLBY_3DMAPOBJ* p_sys )
{
	// マップを破棄する
	WFLBY_3DMAPOBJ_MAP_Delete( &p_sys->map, &p_sys->mapres );
	
	// マップワーク
	WFLBY_3DMAPOBJ_MAP_Release( &p_sys->mapres );
	
	// 配置オブジェワーク
	WFLBY_3DMAPOBJ_WK_Release( &p_sys->objres );
	
	// フロートワーク
	WFLBY_3DMAPOBJ_FLOAT_Release( &p_sys->floatres );

	p_sys->res_load = FALSE;
}


// マップワーク操作
//----------------------------------------------------------------------------
/**
 *	@brief	マップのポールを表示
 *
 *	@param	p_sys	システムワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_MAP_OnPoll( WFLBY_3DMAPOBJ* p_sys )
{
	if( p_sys->mapres.anm_load[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] == TRUE ){
		if( p_sys->map.anm_on[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] == FALSE ){
		#if WB_FIX
			p_sys->map.anm_on[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] = TRUE;
			D3DOBJ_AddAnm( &p_sys->map.obj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], &p_sys->mapres.anm[WFLBY_3DMAPOBJ_MAP_ANM_POLL] );
		#else
			u16 anm_idx;
			p_sys->map.anm_on[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] = TRUE;
			//常に1つしかアニメを持たせないようにしているため、必ずindexは0のはず
			GFL_G3D_OBJECT_EnableAnime( p_sys->map.g3dobj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], WFLBY_3DMAPOBJ_MAP_ANM_POLL );
		#endif
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップのポールを表示しなくする
 *
 *	@param	p_sys	システムワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_MAP_OffPoll( WFLBY_3DMAPOBJ* p_sys )
{
	if( p_sys->mapres.anm_load[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] == TRUE ){
		if( p_sys->map.anm_on[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] == TRUE ){
		#if WB_FIX
			D3DOBJ_DelAnm( &p_sys->map.obj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], &p_sys->mapres.anm[WFLBY_3DMAPOBJ_MAP_ANM_POLL] );
		#else
			GFL_G3D_OBJECT_DisableAnime( p_sys->map.g3dobj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], WFLBY_3DMAPOBJ_MAP_ANM_POLL );
		#endif
			p_sys->map.anm_on[ WFLBY_3DMAPOBJ_MAP_ANM_POLL ] = FALSE;
		}
	}
}


// フロートワーク操作
//----------------------------------------------------------------------------
/**
 *	@brief	フロートワーク読み込み
 *	
 *	@param	p_sys		ワーク
 *	@param	floattype	フロートナンバー
 *	@param	floatcol	フロートカラー
 *	@param	cp_mat		座標
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
WFLBY_3DMAPOBJ_FLOAT* WFLBY_3DMAPOBJ_FLOAT_Add( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_FLOAT_TYPE floattype, WFLBY_3DMAPOBJ_FLOAT_COL floatcol, const VecFx32* cp_mat )
{
	WFLBY_3DMAPOBJ_FLOAT* p_wk;

	GF_ASSERT( floattype <= WFLBY_3DMAPOBJ_FLOAT_NUM );
	GF_ASSERT( floatcol <= WFLBY_3DMAPOBJ_FLOAT_COL_NUM );
	
	// 空いているフロートワークを取得する	
	p_wk = WFLBY_3DMAPOBJ_FLOAT_GetCleanWk( p_sys );


#if WB_FIX
	// 絵をくっつけてあげる
	D3DOBJ_Init( &p_wk->obj, &p_sys->floatres.mdl[ floattype ] );

	// 描画開始
	D3DOBJ_SetDraw( &p_wk->obj, TRUE );
#else
	// 絵をくっつけてあげる
	GF_ASSERT(p_wk->g3dobj == NULL);
	p_wk->g3dobj = GFL_G3D_OBJECT_Create( p_sys->floatres.rnder[ floattype ], p_sys->floatres.anm[ floattype ], WFLBY_3DMAPOBJ_FLOAT_ANM_NUM );
	WFLBY_G3DOBJSTATUS_Init(&p_wk->objst, &p_wk->objrotate);

	// 描画開始
	p_wk->draw_flag = TRUE;
#endif

	// 座標を設定
	WFLBY_3DMAPOBJ_FLOAT_SetPos( p_wk, cp_mat );
	{
		VecFx32 mat = {0,0,0};
		WFLBY_3DMAPOBJ_FLOAT_SetOfsPos( p_wk, &mat );
	}

	// モデルナンバーとカラーナンバーを保存
	p_wk->col	= floatcol;
	p_wk->mdlno	= floattype;

	p_wk->on = TRUE;


	// 常にアニメするものをアニメさせておく
	{
		p_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_00] = TRUE;
		p_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_02] = TRUE;
	}
	
	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_FLOAT_Del( WFLBY_3DMAPOBJ_FLOAT* p_wk )
{
	//  描画フラグを落として、クリアする
#if WB_FIX
	D3DOBJ_SetDraw( &p_wk->obj, FALSE );
#else
	p_wk->draw_flag = FALSE;
	GFL_G3D_OBJECT_Delete(p_wk->g3dobj);
	p_wk->g3dobj = NULL;
#endif
	
	p_wk->on = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート座標を設定
 *
 *	@param	p_wk		ワーク
 *	@param	pos			座標
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_FLOAT_SetPos( WFLBY_3DMAPOBJ_FLOAT* p_wk, const VecFx32* cp_mat )
{
	p_wk->mat	= *cp_mat;
#if WB_FIX
	D3DOBJ_SetMatrix( &p_wk->obj, p_wk->mat.x+p_wk->ofs.x, p_wk->mat.y+p_wk->ofs.y, p_wk->mat.z+p_wk->ofs.z );
#else
	VEC_Set(&p_wk->objst.trans, p_wk->mat.x+p_wk->ofs.x, p_wk->mat.y+p_wk->ofs.y, p_wk->mat.z+p_wk->ofs.z );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画オフセット値を設定する
 *
 *	@param	p_wk		ワーク
 *	@param	cp_mat		座標
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_FLOAT_SetOfsPos( WFLBY_3DMAPOBJ_FLOAT* p_wk, const VecFx32* cp_mat )
{
	p_wk->ofs	= *cp_mat;
#if WB_FIX
	D3DOBJ_SetMatrix( &p_wk->obj, p_wk->mat.x+p_wk->ofs.x, p_wk->mat.y+p_wk->ofs.y, p_wk->mat.z+p_wk->ofs.z );
#else
	VEC_Set(&p_wk->objst.trans, p_wk->mat.x+p_wk->ofs.x, p_wk->mat.y+p_wk->ofs.y, p_wk->mat.z+p_wk->ofs.z );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート座標を取得
 *
 *	@param	p_wk	ワーク
 *	@param	p_mat	マトリックス
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_FLOAT_GetPos( const WFLBY_3DMAPOBJ_FLOAT* cp_wk, VecFx32* p_mat )
{
#if WB_FIX
	D3DOBJ_GetMatrix( &cp_wk->obj, &p_mat->x, &p_mat->y, &p_mat->z );
#else
	*p_mat = cp_wk->objst.trans;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート表示のＯＮＯＦＦ
 *
 *	@param	p_wk	ワーク
 *	@param	flag	フラグ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_FLOAT_SetDraw( WFLBY_3DMAPOBJ_FLOAT* p_wk, BOOL flag )
{
#if WB_FIX
	D3DOBJ_SetDraw( &p_wk->obj, flag );
#else
	p_wk->draw_flag = flag;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート描画チェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	描画中
 *	@retval	FALSE	描画してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_FLOAT_GetDraw( const WFLBY_3DMAPOBJ_FLOAT* cp_wk )
{
#if WB_FIX
	return D3DOBJ_GetDraw( &cp_wk->obj );
#else
	return cp_wk->draw_flag;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	Soundアニメ
 *
 *	@param	p_wk	ワーク
 *
 *	@retval	TRUE	再生した
 *	@retval	FALSE	再生中なので再生しない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_FLOAT_SetAnmSound( WFLBY_3DMAPOBJ_FLOAT* p_wk )
{
	if( p_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_01] == FALSE ){
		p_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_01] = TRUE;
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	bodyアニメ
 *
 *	@param	p_wk	ワーク
 *
 *	@retval	TRUE	再生した
 *	@retval	FALSE	再生中なので再生しない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_FLOAT_SetAnmBody( WFLBY_3DMAPOBJ_FLOAT* p_wk )
{
	if( p_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_03] == FALSE ){
		p_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_03] = TRUE;
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転角度を設定する
 *
 *	@param	p_wk		ワーク
 *	@param	x			ｘ角度
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_FLOAT_SetRotaX( WFLBY_3DMAPOBJ_FLOAT* p_wk, u16 x )
{
#if WB_FIX
	D3DOBJ_SetRota( &p_wk->obj, x, D3DOBJ_ROTA_WAY_X );
#else
	p_wk->objrotate.x = x;
	WFLBY_RotateMtx(&p_wk->objrotate, &p_wk->objst.rotate);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート音性アニメ再生チェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	音声アニメ	再生中
 *	@retval	FALSE	音声アニメ	再生してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_FLOAT_CheckAnmSound( const WFLBY_3DMAPOBJ_FLOAT* cp_wk )
{
	return cp_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_01];
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート胴体アニメ	再生中
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	胴体アニメ	再生中
 *	@retval	FALSE	胴体アニメ	停止中
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_FLOAT_CheckAnmBody( const WFLBY_3DMAPOBJ_FLOAT* cp_wk )
{
	return cp_wk->anm_on[WFLBY_3DMAPOBJ_FLOAT_ANM_03];
}



// 配置オブジェ操作
//----------------------------------------------------------------------------
/**
 *	@brief	配置オブジェ作成
 *
 *	@param	p_sys		システム
 *	@param	objtype		オブジェクトNO
 *	@param	gridx		グリッドX
 *	@param	gridy		グリッドY
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
WFLBY_3DMAPOBJ_WK* WFLBY_3DMAPOBJ_WK_Add( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK_TYPE objtype, u32 gridx, u32 gridy )
{
	WFLBY_3DMAPOBJ_WK* p_wk;
	int i;
	
	// 空いているワークを取得
	p_wk = WFLBY_3DMAPOBJ_WK_GetCleanWk( p_sys );


	// 選択したグラフィックを設定
#if WB_FIX
	D3DOBJ_Init( &p_wk->obj, &p_sys->objres.mdl[objtype] );
#else
	GF_ASSERT(p_wk->g3dobj == NULL);
	p_wk->g3dobj = GFL_G3D_OBJECT_Create( p_sys->objres.rnder[ objtype ], p_sys->objres.anm[ objtype ], WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	WFLBY_G3DOBJSTATUS_Init(&p_wk->objst, &p_wk->objrotate);
#endif

	// アニメオブジェの初期化
	for( i=0; i<WFLBY_3DMAPOBJ_ALL_ANM_MAX; i++ ){
	#if WB_FIX
		if( p_sys->objres.p_anm[objtype][i] != NULL ){
			// アニメションオブジェの初期化
			D3DOBJ_AnmLoad_Data( &p_wk->anm[i], &p_sys->objres.mdl[objtype],
					p_sys->objres.p_anm[objtype][i] );
		}
	#else
		if( p_sys->objres.p_anmres[objtype][i] != NULL ){
			// アニメションオブジェの初期化
			GF_ASSERT(p_wk->anm[i] == NULL);
			p_wk->anm[i] = GFL_G3D_ANIME_Create( 
				p_sys->objres.rnder[ objtype ], p_sys->objres.p_anmres[objtype][i], 0 ); 
		}
	#endif
	}
	
	// 描画開始
#if WB_FIX
	D3DOBJ_SetDraw( &p_wk->obj, TRUE );
#else
	p_wk->draw_flag = TRUE;
#endif

	// グリッド座標を設定
	{
		WF2DMAP_POS	pos;
		pos.x	= WF2DMAP_GRID2POS(gridx);
		pos.y	= WF2DMAP_GRID2POS(gridy);
		WFLBY_3DMAPOBJ_WK_SetPos( p_wk, pos );
	}

	p_wk->on	= TRUE;
	p_wk->mdlid	= objtype;

	// アニメ設定
	p_wk->wait_def = WFLBY_3DMAPOBJ_WK_ANM_RANDWAIT_WAITDEF;

	// スピード
	p_wk->speed = FX32_ONE;

	// アルファ設定
	p_wk->alpha_flag	= FALSE;
	p_wk->alpha			= 31;
	p_wk->def_alpha		= 31;

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	配置オブジェ破棄
 *
 *	@param	p_sys		システム
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_Del( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk )
{
	int i;
	
	//  描画フラグを落として、クリアする
#if WB_FIX
	D3DOBJ_SetDraw( &p_wk->obj, FALSE );
#else
	p_wk->draw_flag = FALSE;
	GFL_G3D_OBJECT_Delete( p_wk->g3dobj );
	p_wk->g3dobj = NULL;
#endif

	// アニメオブジェの破棄
	for( i=0; i<WFLBY_3DMAPOBJ_ALL_ANM_MAX; i++ ){
	#if WB_FIX
		if( p_sys->objres.p_anm[p_wk->mdlid][i] != NULL ){
			// アニメションオブジェの初期化
			D3DOBJ_AnmLoad_Data( &p_wk->anm[i], &p_sys->objres.mdl[p_wk->mdlid],
					p_sys->objres.p_anm[p_wk->mdlid][i] );
		}
	#else
		if( p_sys->objres.p_anmres[p_wk->mdlid][i] != NULL ){
			// アニメションオブジェの初期化
			GFL_G3D_ANIME_Delete( p_wk->anm[i] );
			p_wk->anm[i] = NULL;
		}
	#endif
	}
	
	p_wk->on = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェタイプを取得する
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	オブジェタイプ
 */
//-----------------------------------------------------------------------------
WFLBY_3DMAPOBJ_WK_TYPE WFLBY_3DMAPOBJ_WK_GetType( const WFLBY_3DMAPOBJ_WK* cp_wk )
{
	return cp_wk->mdlid;
}

//----------------------------------------------------------------------------
/**
 *	@brief	座標設定
 *	
 *	@param	p_wk	ワーク
 *	@param	pos		座標
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetPos( WFLBY_3DMAPOBJ_WK* p_wk, WF2DMAP_POS pos )
{
	VecFx32 matrix;

	WFLBY_3DMATRIX_GetPosVec( &pos, &matrix );
#if WB_FIX
	D3DOBJ_SetMatrix( &p_wk->obj, matrix.x, matrix.y, matrix.z );
#else
	VEC_Set( &p_wk->objst.trans, matrix.x, matrix.y, matrix.z );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	座標を取得する
 *
 *	@param	p_wk	ワーク
 *
 *	@return	座標
 */
//-----------------------------------------------------------------------------
WF2DMAP_POS WFLBY_3DMAPOBJ_WK_GetPos( const WFLBY_3DMAPOBJ_WK* cp_wk )
{
	VecFx32		matrix;
	WF2DMAP_POS	pos;

#if WB_FIX
	D3DOBJ_GetMatrix( &cp_wk->obj, &matrix.x, &matrix.y, &matrix.z );
#else
	matrix = cp_wk->objst.trans;
#endif
	WFLBY_3DMATRIX_GetVecPos( &matrix, &pos );

	return pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ登録
 *
 *	@param	p_sys		オブジェシステム
 *	@param	p_wk		ワーク
 *	@param	anm			アニメ
 *	@param	play		プレイ方式
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_AddAnm( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm, WFLBY_3DMAPOBJ_WK_ANM_PLAY play )
{
	WFLBY_3DMAPOBJ_WK_AddAnmAnmCallBack( p_sys, p_wk, anm, play, NULL );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ再生	終了コールバックつき
 *
 *	@param	p_sys		オブジェシステム
 *	@param	p_wk		ワーク
 *	@param	anm			アニメ
 *	@param	play		プレイ方式
 *	@param	p_callback	アニメ終了コールバック
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_AddAnmAnmCallBack( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm, WFLBY_3DMAPOBJ_WK_ANM_PLAY play, pWFLBY_3DMAPOBJ_WK_AnmCallBack p_callback )
{
	int get_anmframe, set_anmframe, index;
	int test, test2;
	
	// そのアニメがあるかチェック
	GF_ASSERT( anm < WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	GF_ASSERT( play < WFLBY_3DMAPOBJ_WK_ANM_PLAYNUM );
	GF_ASSERT( p_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );

	index = 0;
	
#if WB_FIX
	if( (p_sys->objres.p_anm[p_wk->mdlid][anm] != NULL) ){
#else
	if( (p_sys->objres.p_anmres[p_wk->mdlid][anm] != NULL) ){
#endif
		if( p_wk->anm_on[ anm ] == FALSE ){
			//  アニメ設定
		#if WB_FIX
			D3DOBJ_AddAnm( &p_wk->obj, &p_wk->anm[anm] );
		#else
			GFL_G3D_OBJECT_RemoveAnime( p_wk->g3dobj, index);
			test = GFL_G3D_OBJECT_AddAnime( p_wk->g3dobj, p_wk->anm[anm] );
			test2 = GFL_G3D_OBJECT_EnableAnime( p_wk->g3dobj, index );
			GF_ASSERT(index == test);
			GF_ASSERT(test == 0);
			GF_ASSERT(test2 == TRUE);
		#endif
		}

		p_wk->anm_on[ anm ]		= TRUE;
		p_wk->play[anm]			= play;
		p_wk->p_anmcallback[anm]= p_callback;
		p_wk->wait[ anm ]		= 0;

		switch( play ){
		case WFLBY_3DMAPOBJ_WK_ANM_LOOP:
		case WFLBY_3DMAPOBJ_WK_ANM_NOLOOP:
		case WFLBY_3DMAPOBJ_WK_ANM_NOAUTO:
			p_wk->anm_frame[ anm ]	= 0;
			break;
		case WFLBY_3DMAPOBJ_WK_ANM_BACKLOOP:
		case WFLBY_3DMAPOBJ_WK_ANM_BACKNOLOOP:
		#if WB_FIX
			p_wk->anm_frame[ anm ]	= D3DOBJ_AnmGetFrameNum( &p_wk->anm[anm] );
		#else
			p_wk->anm_frame[ anm ]	= NNS_G3dAnmObjGetNumFrame(GFL_G3D_ANIME_GetAnmObj(p_wk->anm[anm]));
		#endif
			break;

		case WFLBY_3DMAPOBJ_WK_ANM_RANDWAIT:
			p_wk->anm_frame[ anm ]	= 0;
			p_wk->wait[ anm ] = GFUser_GetPublicRand(p_wk->wait_def);
			break;

		case WFLBY_3DMAPOBJ_WK_ANM_RANDSTART:
			p_wk->anm_frame[ anm ]	= 0;
			p_wk->wait[ anm ] = GFUser_GetPublicRand(p_wk->wait_def);
			break;
		}

	#if WB_FIX
		D3DOBJ_AnmSet( &p_wk->anm[anm], p_wk->anm_frame[anm] );
	#else
		set_anmframe = p_wk->anm_frame[anm];
		GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, index, &set_anmframe );
		OS_TPrintf("aaa SetAnimeFrame frame=%d, index=%d\n", set_anmframe, index);
	#endif
	}	
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ破棄
 *
 *	@param	p_sys		オブジェシステム
 *	@param	p_wk		ワーク
 *	@param	anm			アニメ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_DelAnm( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm )
{
  int index;
  BOOL ret;
  
  index = 0;
  
	// そのアニメがあるかチェック
	GF_ASSERT( anm < WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	GF_ASSERT( p_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );

#if WB_FIX
	if( (p_sys->objres.p_anm[p_wk->mdlid][anm] != NULL) && (p_wk->anm_on[anm] == TRUE) ){
#else
	if( (p_sys->objres.p_anmres[p_wk->mdlid][anm] != NULL) && (p_wk->anm_on[anm] == TRUE) ){
#endif
		//  アニメをはがす
	#if WB_FIX
		D3DOBJ_DelAnm( &p_wk->obj, &p_wk->anm[ anm ] );
	#else
		ret = GFL_G3D_OBJECT_DisableAnime( p_wk->g3dobj, index ); //anm );
		GF_ASSERT(ret == TRUE);
	#endif
	
		p_wk->anm_on[ anm ]		= FALSE;
		p_wk->anm_frame[ anm ]	= 0;
		p_wk->play[ anm ]		= 0;
		p_wk->wait[ anm ]		= 0;
		p_wk->p_anmcallback[anm]= NULL;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	全アニメをOFFする
 *
 *	@param	p_sys		システムワーク
 *	@param	p_wk		配置オブジェワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_DelAnmAll( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk )
{
	int i;

	for( i=0; i<WFLBY_3DMAPOBJ_ALL_ANM_MAX; i++ ){
		if( WFLBY_3DMAPOBJ_WK_GetAnmFlag( p_wk, i ) == TRUE ){
			WFLBY_3DMAPOBJ_WK_DelAnm( p_sys,  p_wk, i );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ再生中かチェック
 *
 *	@param	cp_sys		システム
 *	@param	cp_wk		ワーク
 *	@param	anm			アニメ
 *
 *	@retval	TRUE	再生中
 *	@retval	FALSE	再生してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_WK_CheckAnm( const WFLBY_3DMAPOBJ* cp_sys, const WFLBY_3DMAPOBJ_WK* cp_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm )
{
	// そのアニメがあるかチェック
	GF_ASSERT( anm < WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	GF_ASSERT( cp_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );
	
#if WB_FIX
	if( cp_sys->objres.p_anm[cp_wk->mdlid][anm] != NULL ){
#else
	if( cp_sys->objres.p_anmres[cp_wk->mdlid][anm] != NULL ){
#endif
		return cp_wk->anm_on[ anm ];
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを設定する
 *
 *	@param	p_sys		システム
 *	@param	p_wk		ワーク
 *	@param	anm			アニメタイプ
 *	@param	frame		フレーム数
 *
 *	アニメーションタイプがWFLBY_3DMAPOBJ_WK_ANM_NOAUTOのとき専用
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetAnmFrame( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm, fx32 frame )
{
	int set_anmframe;
	
	// そのアニメがあるかチェック
	GF_ASSERT( anm < WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	GF_ASSERT( p_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );
#if WB_FIX
	GF_ASSERT( p_sys->objres.p_anm[p_wk->mdlid][anm] != NULL );
#else
	GF_ASSERT( p_sys->objres.p_anmres[p_wk->mdlid][anm] != NULL );
#endif

	if( p_wk->play[ anm ] != WFLBY_3DMAPOBJ_WK_ANM_NOAUTO ){
		return ; 	// オートアニメ中なので操作しない
	}
	p_wk->anm_frame[ anm ]	= frame;
#if WB_FIX
	D3DOBJ_AnmSet( &p_wk->anm[anm], p_wk->anm_frame[anm] );
#else
	set_anmframe = p_wk->anm_frame[anm];
	GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, anm, &set_anmframe );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ設定しているかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	anm			アニメタイプ
 *
 *	@retval	TRUE	アニメ中
 *	@retval	FALSE	アニメしてない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_3DMAPOBJ_WK_GetAnmFlag( const WFLBY_3DMAPOBJ_WK* cp_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm )
{
	// そのアニメがあるかチェック
	GF_ASSERT( anm < WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	GF_ASSERT( cp_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );
	return cp_wk->anm_on[ anm ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメフレームを取得する
 *
 *	@param	cp_wk		ワーク
 *	@param	anm			アニメタイプ
 *
 *	@return	フレーム数
 */
//-----------------------------------------------------------------------------
fx32 WFLBY_3DMAPOBJ_WK_GetAnmFrame( const WFLBY_3DMAPOBJ_WK* cp_wk, WFLBY_3DMAPOBJ_WK_ANM_TYPE anm )
{
	// そのアニメがあるかチェック
	GF_ASSERT( anm < WFLBY_3DMAPOBJ_ALL_ANM_MAX );
	GF_ASSERT( cp_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );
	return cp_wk->anm_frame[ anm ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	ランダムウエイト再生用	ウエイト時間設定
 *
 *	@param	p_sys		システム
 *	@param	p_wk		ワーク
 *	@param	wait		ウエイト定数
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetRandWait( WFLBY_3DMAPOBJ_WK* p_wk, u32 wait )
{
	p_wk->wait_def = wait;
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画フラグを設定
 *
 *	@param	p_wk	ワーク
 *	@param	flag	描画フラグ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetDraw( WFLBY_3DMAPOBJ_WK* p_wk, BOOL flag )
{
#if WB_FIX
	D3DOBJ_SetDraw( &p_wk->obj, flag );
#else
	p_wk->draw_flag = flag;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライトフラグを設定する
 *			【そのモデルと同じ形のモデルはすべてこのライト設定になってしまうので
 *			注意が必要です。】
 *
 *	@param	p_sys			システム
 *	@param	p_wk			ワーク
 *	@param	light_flag		ライトフラグ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetLightFlag( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk, u32 light_flag )
{
	NNSG3dResMdlSet*		pMdlset;
	NNSG3dResMdl*			pMdl;
	
	// そのアニメがあるかチェック
	GF_ASSERT( p_wk->mdlid < WFLBY_3DMAPOBJ_WK_NUM );

	// フィールドの設定に変更
#if WB_FIX
	NNS_G3dMdlSetMdlLightEnableFlagAll( p_sys->objres.mdl[ p_wk->mdlid ].pModel, light_flag );
#else
	pMdlset = NNS_G3dGetMdlSet(GFL_G3D_GetResourceFileHeader(p_sys->objres.p_mdlres[p_wk->mdlid]));
	pMdl = NNS_G3dGetMdlByIdx( pMdlset, 0 );
	NNS_G3dMdlSetMdlLightEnableFlagAll( pMdl, light_flag );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメスピード設定
 *
 *	@param	p_wk	ワーク
 *	@param	speed	スピード
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetAnmSpeed( WFLBY_3DMAPOBJ_WK* p_wk, fx32 speed )
{
	p_wk->speed = speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメスピードを取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	スピード
 */
//-----------------------------------------------------------------------------
fx32 WFLBY_3DMAPOBJ_WK_GetAnmSpeed( const WFLBY_3DMAPOBJ_WK* cp_wk )
{
	return cp_wk->speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アルファ値設定
 *	
 *	@param	p_wk	ワーク
 *	@param	alpha	アルファ
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetAlpha( WFLBY_3DMAPOBJ_WK* p_wk, u8 alpha, u8 def_alpha )
{
	p_wk->alpha_flag	= TRUE;
	p_wk->alpha			= alpha;
	p_wk->def_alpha		= def_alpha;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アルファ値のリセット
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_ResetAlpha( WFLBY_3DMAPOBJ_WK* p_wk )
{
	p_wk->alpha_flag	= FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	拡大値を設定
 *
 *	@param	p_wk	ワーク
 *	@param	x		ｘ方向
 *	@param	y		ｙ方向
 *	@param	z		ｚ方向
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_WK_SetScale( WFLBY_3DMAPOBJ_WK* p_wk, fx32 x, fx32 y, fx32 z )
{
#if WB_FIX
	D3DOBJ_SetScale( &p_wk->obj, x, y, z );
#else
	VEC_Set(&p_wk->objst.scale, x, y, z);
#endif
}




//-----------------------------------------------------------------------------
/**
 *				プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	部屋ナンバーSeasonから読み込みデータを取得する
 *
 *	@param	room		部屋ナンバー
 *	@param	season		Seasonナンバー
 *	@param	heapID		ヒープID
 *
 *	@return	読み込みグラフィックデータ
 */	
//-----------------------------------------------------------------------------
static WFLBY_3DMAPOBJ_MDL_DATA* WFLBY_3DMAPOBJ_MDLRES_DATA_Init( WFLBY_ROOM_TYPE room, WFLBY_SEASON_TYPE season, u32 heapID )
{
	void* p_data;
	u32 idx;

	// 部屋インデックス
	idx = (season * WFLBY_ROOM_NUM) + room;

	GF_ASSERT( idx < (WFLBY_SEASON_NUM*WFLBY_ROOM_NUM) );
	
	p_data = GFL_ARC_UTIL_Load( ARCID_WFLBY_MAP, 
			NARC_wflby_map_wflby_3dmapobj_data_0_dat+idx,
			FALSE, GFL_HEAP_LOWID(heapID));

	return p_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief	読み込みグラフィックデータ取得
 *
 *	@param	p_data	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MDLRES_DATA_Exit( WFLBY_3DMAPOBJ_MDL_DATA* p_data )
{
	GFL_HEAP_FreeMemory( p_data );
}


//----------------------------------------------------------------------------
/**
 *	@brief	広場用モデル読み込み
 *
 *	@param	p_mdl		モデルワーク
 *	@param	p_handle	ハンドル
 *	@param	data_idx	データインデックス
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MDL_Load( GFL_G3D_RES** p_res, ARCHANDLE* p_handle, u32 data_idx, u32 gheapID )
{
	// テクスチャ実部分を破棄したテクスチャリソースを読み込む
	WFLBY_3DMAPOBJ_TEX_LoatCutTex( p_res, p_handle, data_idx, gheapID );
	
	// 本データの各ポインタを取得する
#if WB_FIX	//GFL_G3D_RENDER_Createの内部で行うのでやる必要なし
	{
		NNSG3dResFileHeader* nnsg3d_head = GFL_G3D_GetResourceFileHeader(*p_res);

		p_mdl->pModelSet	= NNS_G3dGetMdlSet( nnsg3d_head );
		p_mdl->pModel		= NNS_G3dGetMdlByIdx( p_mdl->pModelSet, 0 );
		p_mdl->pMdlTex		= NNS_G3dGetTex( nnsg3d_head );
	}

	// テクスチャキーを設定して、モデルにバインドする
	{
		BindTexture( p_mdl->pResMdl, p_mdl->pMdlTex );
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	広場用モデル破棄
 *
 *	@param	p_mdl		モデルワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MDL_Delete( GFL_G3D_RES* p_mdl )
{
#if WB_FIX
	D3DOBJ_MdlDelete( p_mdl );
#else
	GFL_G3D_FreeVramTexture(p_mdl);
	GFL_G3D_DeleteResource(p_mdl);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	フレーム管理
 *
 *	@param	p_frame		フレームポインタ
 *	@param	cp_anm		対象アニメーションオブジェ
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_ANM_Loop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed )
{
	fx32 end_frame;

#if WB_FIX
	end_frame = D3DOBJ_AnmGetFrameNum( cp_anm );
#else
	end_frame = NNS_G3dAnmObjGetNumFrame(GFL_G3D_ANIME_GetAnmObj(cp_anm));
#endif

	// アニメを進める
	if( ((*p_frame) + speed) < end_frame ){
		(*p_frame) += speed;
	}else{
		(*p_frame) = ((*p_frame) + speed) % end_frame;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フレーム管理
 *
 *	@param	p_frame		フレームポインタ
 *	@param	cp_anm		対象アニメーションオブジェ
 *
 *	@retval	TRUE	アニメ終了
 *	@retval	FALSE	アニメ途中
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_3DMAPOBJ_ANM_NoLoop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed )
{
	fx32 end_frame;
	BOOL ret;

#if WB_FIX
	end_frame = D3DOBJ_AnmGetFrameNum( cp_anm );
#else
	end_frame = NNS_G3dAnmObjGetNumFrame(GFL_G3D_ANIME_GetAnmObj(cp_anm));
#endif

	// アニメを進める
	if( ((*p_frame) + speed) < end_frame ){
		(*p_frame) += speed;
		ret = FALSE;
	}else{
		(*p_frame) =  end_frame-FX32_HALF;
		ret = TRUE;
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	逆再生アニメ
 *	
 *	@param	p_frame		フレーム
 *	@param	cp_anm		アニメオブジェ
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_ANM_BackLoop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed )
{
	fx32 end_frame;

#if WB_FIX
	end_frame = D3DOBJ_AnmGetFrameNum( cp_anm );
#else
	end_frame = NNS_G3dAnmObjGetNumFrame(GFL_G3D_ANIME_GetAnmObj(cp_anm));

#endif

	// アニメを進める
	if( ((*p_frame) - speed) >= 0 ){
		(*p_frame) -= speed;
	}else{
		(*p_frame) = ((*p_frame) - speed) + end_frame;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	逆再生ループなしアニメ
 *
 *	@param	p_frame		フレーム
 *	@param	cp_anm		アニメ
 *
 *	@retval	TRUE	アニメ終了
 *	@retval	FALSE	アニメ途中
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_3DMAPOBJ_ANM_BackNoLoop( fx32* p_frame, const GFL_G3D_ANM* cp_anm, fx32 speed )
{
	BOOL ret;

	// アニメを進める
	if( ((*p_frame) - speed) > 0 ){
		(*p_frame) -= speed;
		ret = FALSE;
	}else{
		(*p_frame) =  0;
		ret = TRUE;
	}

	return ret;
}



// マップワーク
//----------------------------------------------------------------------------
/**
 *	@brief	マップ読み込み
 *
 *	@param	p_wk		ワーク
 *	@param	p_handle	ハンドル
 *	@param	cp_data		データ
 *	@param	gheapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MAP_Load( WFLBY_3DMAPOBJ_MAPRES* p_wk, ARCHANDLE* p_handle, const WFLBY_3DMAPOBJ_MDL_DATA* cp_data, u32 gheapID )
{
	int i, j;
	// モデルリソース読み込み
	for( i=0; i<WFLBY_3DMAPOBJ_MAPOBJ_NUM; i++ ){
//		OS_TPrintf( "map load mdl idx = %d\n",  cp_data->map_idx[i] );
		WFLBY_3DMAPOBJ_MDL_Load( &p_wk->p_mdlres[ i ],
				p_handle, cp_data->map_idx[ i ], gheapID );
		p_wk->rnder[i] = GFL_G3D_RENDER_Create( p_wk->p_mdlres[i], 0, p_wk->p_mdlres[i] );


		// ライト設定
	#if WB_FIX
		WFLBY_LIGHT_SetUpMdl( p_wk->mdl[ i ].pResMdl );	// ライト反映
	#else
		WFLBY_LIGHT_SetUpMdl( GFL_G3D_GetResourceFileHeader(p_wk->p_mdlres[i]) );	// ライト反映
	#endif
	}

	// アニメリソースを読み込む
	for( i=0; i<WFLBY_3DMAPOBJ_MAP_ANM_NUM; i++ ){

		// アニメIDXがモデルIDXと違ったら読み込む
		if( cp_data->map_anmidx[i] == cp_data->map_idx[WFLBY_3DMAPOBJ_MAPOBJ_MAP] ){
			p_wk->anm_load[i] = FALSE;
		}else{

			p_wk->anm_load[i] = TRUE;

//			OS_TPrintf( "map load anm idx = %d\n",  cp_data->map_anmidx[i] );

			if( i != WFLBY_3DMAPOBJ_MAP_ANM_MAT ){
			#if WB_FIX
				D3DOBJ_AnmLoadH( &p_wk->anm[i], &p_wk->mdl[ WFLBY_3DMAPOBJ_MAPOBJ_MAP ],
						p_handle, 
						cp_data->map_anmidx[i], gheapID, p_allocator );
			#else
				p_wk->p_anmres[i] = GFL_G3D_CreateResourceHandle( 
					p_handle, cp_data->map_anmidx[i] );
				p_wk->anm[i] = GFL_G3D_ANIME_Create( 
					p_wk->rnder[WFLBY_3DMAPOBJ_MAPOBJ_MAP], p_wk->p_anmres[i], 0 ); 
			#endif
			}else{

			#if WB_FIX
				D3DOBJ_AnmLoadH( &p_wk->anm[i], &p_wk->mdl[ WFLBY_3DMAPOBJ_MAPOBJ_MAT ],
						p_handle, 
						cp_data->map_anmidx[i], gheapID, p_allocator );
			#else
				p_wk->p_anmres[i] = GFL_G3D_CreateResourceHandle( 
					p_handle, cp_data->map_anmidx[i] );
				p_wk->anm[i] = GFL_G3D_ANIME_Create( 
					p_wk->rnder[WFLBY_3DMAPOBJ_MAPOBJ_MAT], p_wk->p_anmres[i], 0 ); 
			#endif
			}

#if 0
			// 動作させるNODEIDXを指定
			for( j=0; j<cp_data->map_nodenum; j++ ){
				if( cp_data->map_anmnodeidx[i] != j ){
					NNS_G3dAnmObjDisableID( p_wk->anm[i].pAnmObj, j );
				}
			}
#endif
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	モデルリソースの破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MAP_Release( WFLBY_3DMAPOBJ_MAPRES* p_wk )
{
	int i;

	// アニメ破棄
	for( i=0; i<WFLBY_3DMAPOBJ_MAP_ANM_NUM; i++ ){
		if( p_wk->anm_load[i] ){
		#if WB_FIX
			D3DOBJ_AnmDelete( &p_wk->anm[i], p_allocator );
		#else
			GFL_G3D_ANIME_Delete( p_wk->anm[i] );
			GFL_G3D_DeleteResource( p_wk->p_anmres[i] );
		#endif
			p_wk->anm_load[i] = FALSE;
		}
	}

	//  モデル破棄
	for( i=0; i<WFLBY_3DMAPOBJ_MAPOBJ_NUM; i++ ){
		WFLBY_3DMAPOBJ_MDL_Delete( p_wk->p_mdlres[i] );	
		GFL_G3D_RENDER_Delete(p_wk->rnder[i]);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面オブジェの作成
 *
 *	@param	p_wk		ワーク
 *	@param	p_res		リソース 
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MAP_Add( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res )
{
	int i;

	p_wk->on = TRUE;
	
	for( i=0; i<WFLBY_3DMAPOBJ_MAPOBJ_NUM; i++ ){
	#if WB_FIX
		D3DOBJ_Init( &p_wk->obj[i],  &p_res->mdl[i] );
		// 描画開始
		D3DOBJ_SetDraw( &p_wk->obj[i], TRUE );
	#else
		p_wk->g3dobj[i] = GFL_G3D_OBJECT_Create( p_res->rnder[ i ], p_res->anm, WFLBY_3DMAPOBJ_MAP_ANM_NUM );
		WFLBY_G3DOBJSTATUS_Init(&p_wk->objst[i], &p_wk->objrotate[i]);
		// 描画開始
		p_wk->draw_flag[i] = TRUE;
	#endif
	
		// 半グリッドずらす	
//		D3DOBJ_SetMatrix( &p_wk->obj[i], -WFLBY_3DMATRIX_GRID_GRIDSIZ_HALF, 0, 0 );	
	}

	// POLL以外のアニメを開始
	for( i=0; i<WFLBY_3DMAPOBJ_MAP_ANM_NUM-1; i++ ){

		if( p_res->anm_load[i] == TRUE ){
			p_wk->anm_on[i] = TRUE;

			// シート用マットアニメ以外はMAPにアニメ追加
			if( i != WFLBY_3DMAPOBJ_MAP_ANM_MAT ){

			#if WB_FIX
				D3DOBJ_AddAnm( &p_wk->obj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], 
						&p_res->anm[i] );
			#else
				GFL_G3D_OBJECT_EnableAnime( p_wk->g3dobj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], i );
			#endif
			}else{
			#if WB_FIX
				D3DOBJ_AddAnm( &p_wk->obj[WFLBY_3DMAPOBJ_MAPOBJ_MAT], 
						&p_res->anm[i] );
			#else
				GFL_G3D_OBJECT_EnableAnime( p_wk->g3dobj[WFLBY_3DMAPOBJ_MAPOBJ_MAT], i );
			#endif
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップオブジェ	破棄
 *
 *	@param	p_wk	ワーク
 *	@param	p_res	リソースワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MAP_Delete( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res )
{
	int i, k;

	// 全アニメをはがす
	for( k=0; k<WFLBY_3DMAPOBJ_MAPOBJ_NUM; k++){
  	for( i=0; i<WFLBY_3DMAPOBJ_MAP_ANM_NUM-1; i++ ){

  		if( p_wk->anm_on[i] == TRUE ){
  			p_wk->anm_on[i] = FALSE;

  			// シート用マットアニメ以外はMAPにアニメ追加
  			if( i != WFLBY_3DMAPOBJ_MAP_ANM_MAT ){

  			#if WB_FIX
  				D3DOBJ_DelAnm( &p_wk->obj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], 
  						&p_res->anm[i] );
  			#else
  				GFL_G3D_OBJECT_DisableAnime( p_wk->g3dobj[WFLBY_3DMAPOBJ_MAPOBJ_MAP], i );
  			#endif
  			}else{
  			#if WB_FIX
  				D3DOBJ_DelAnm( &p_wk->obj[WFLBY_3DMAPOBJ_MAPOBJ_MAT], 
  						&p_res->anm[i] );
  			#else
  				GFL_G3D_OBJECT_DisableAnime( p_wk->g3dobj[WFLBY_3DMAPOBJ_MAPOBJ_MAT], i );
  			#endif
  			}
  		}
  	}
		GFL_G3D_OBJECT_Delete( p_wk->g3dobj[k] );
		p_wk->g3dobj[k] = NULL;
	}
	
	p_wk->on = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップメイン処理
 *
 *	@param	p_wk	ワーク
 *	@param	p_res	リソース
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MAP_Main( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res )
{
	int i, set_anmframe;
	int k;
	
	// アニメが開始していたらフレームを設定する
	for( i=0; i<WFLBY_3DMAPOBJ_MAP_ANM_NUM; i++ ){
		if( p_wk->anm_on[i] ){
			WFLBY_3DMAPOBJ_ANM_Loop( &p_wk->anm_frame[i], p_res->anm[i], FX32_ONE );
			// フレームをあわせる
		#if WB_FIX
			D3DOBJ_AnmSet( &p_res->anm[i], p_wk->anm_frame[i] );
		#else
			for(k = 0; k < WFLBY_3DMAPOBJ_MAPOBJ_NUM; k++){
				set_anmframe = p_wk->anm_frame[i];
				GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj[k], i, &set_anmframe );
			}
		#endif
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップ描画
 *
 *	@param	p_wk	ワーク
 *	@param	p_res	リソース
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_MAP_Draw( WFLBY_3DMAPOBJ_MAP* p_wk, WFLBY_3DMAPOBJ_MAPRES* p_res )
{
	int i, j;
	if( p_wk->on ){
		for( i=0; i<WFLBY_3DMAPOBJ_MAPOBJ_NUM; i++ ){

		#if WB_FIX
			D3DOBJ_Draw( &p_wk->obj[i] );
		#else
			if(p_wk->draw_flag[i] == TRUE){
				GFL_G3D_DRAW_DrawObjectCullingON(p_wk->g3dobj[i], &p_wk->objst[i]);
			}
		#endif
		
		}
	}
}


// フロートワーク
//----------------------------------------------------------------------------
/**
 *	@brief	空いているワークを取得する
 *
 *	@param	p_sys		ワーク
 *
 *	@return	空いているワーク
 */
//-----------------------------------------------------------------------------
static WFLBY_3DMAPOBJ_FLOAT* WFLBY_3DMAPOBJ_FLOAT_GetCleanWk( WFLBY_3DMAPOBJ* p_sys )
{
	int i;

	for( i=0; i<p_sys->floatnum; i++ ){
		if( p_sys->p_float[i].on == FALSE ){
			return &p_sys->p_float[i];
		}
	}

	GF_ASSERT(0);
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートの読み込み
 *	
 *	@param	p_wk		ワーク
 *	@param	p_handle	ハンドル
 *	@param	cp_data		データ
 *	@param	gheapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_FLOAT_Load( WFLBY_3DMAPOBJ_FLOATRES* p_wk, ARCHANDLE* p_handle, const WFLBY_3DMAPOBJ_MDL_DATA* cp_data, u32 gheapID )
{
	int i, j, k;

	// テクスチャ読み込み
	{
		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_NUM; i++ ){
			for( j=0; j<WFLBY_3DMAPOBJ_FLOAT_COL_NUM; j++ ){
				WFLBY_3DMAPOBJ_TEX_LoatCutTex( &p_wk->p_texres[i][j], p_handle,
						cp_data->float_texidx[i][j], gheapID );
			}
		}
	}

	// モデル読み込み
	{
		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_NUM; i++ ){

		#if WB_FIX	//GFL_G3D_RENDER_Createの内部で行うのでやる必要なし

			// モデルを読み込み
			p_wk->mdl[i].pResMdl	= GFL_ARCHDL_UTIL_Load( p_handle, 
						cp_data->float_idx[i], FALSE, gheapID );
			p_wk->mdl[i].pModelSet	= NNS_G3dGetMdlSet( p_wk->mdl[i].pResMdl );
			p_wk->mdl[i].pModel		= NNS_G3dGetMdlByIdx( p_wk->mdl[i].pModelSet, 0 );

			// テクスチャは後でバインドする
			// とりあえず今はアニメ初期用に０のものを入れる
			p_wk->mdl[i].pMdlTex = NNS_G3dGetTex( p_wk->p_texres[i][0] );
		#else
			p_wk->p_mdlres[i] = GFL_G3D_CreateResourceHandle( p_handle, cp_data->float_idx[i] ) ;
		#endif
		}
	}

	//レンダー作成
	{
		for(i = 0; i < WFLBY_3DMAPOBJ_FLOAT_NUM; i++){
			// テクスチャは後でバインドする
			// とりあえず今はアニメ初期用に０のものを入れる
			p_wk->rnder[i] = GFL_G3D_RENDER_Create( p_wk->p_mdlres[i], 0, p_wk->p_texres[i][0] );
		}
	}
	
	// アニメの読み込み
	{
		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_NUM; i++ ){
			for( j=0; j<WFLBY_3DMAPOBJ_FLOAT_ANM_NUM; j++ ){
	
				// モデルIDXとアニメIDXが一致したら読み込まない
				if( cp_data->float_idx[0] != cp_data->float_anmidx[i][j] ){

				#if WB_FIX
					D3DOBJ_AnmLoadH( &p_wk->anm[i][j], &p_wk->mdl[i], p_handle, 
							cp_data->float_anmidx[i][j], gheapID );
				#else
					p_wk->p_anmres[i][j] = GFL_G3D_CreateResourceHandle( p_handle, 
						cp_data->float_anmidx[i][j] ) ;
					p_wk->anm[i][j] = GFL_G3D_ANIME_Create( 
						p_wk->rnder[i], p_wk->p_anmres[i][j], 0 ); 
				#endif
				
					// 頭と尻尾（か翼）のアニメなら反映先ノードIDを設定
					if( j >= WFLBY_3DMAPOBJ_FLOAT_ANM_01 ){

						// モデルノード数分アニメ設定をOFF
						for( k=0; k<cp_data->float_nodenum[i]; k++ ){
							if( cp_data->float_anm_node[i][j-WFLBY_3DMAPOBJ_FLOAT_ANM_01] != k ){
							#if WB_FIX
								NNS_G3dAnmObjDisableID( p_wk->anm[i][j].pAnmObj, k );
							#else
								NNS_G3dAnmObjDisableID(
									GFL_G3D_ANIME_GetAnmObj(p_wk->anm[i][j]), k);
							#endif
							}
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートリソース破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_FLOAT_Release( WFLBY_3DMAPOBJ_FLOATRES* p_wk )
{
	int i, j;

  // レンダーの破棄
  for(i = 0; i < WFLBY_3DMAPOBJ_FLOAT_NUM; i++){
    GFL_G3D_RENDER_Delete(p_wk->rnder[i]);
  }
  
	// アニメの破棄
	{
		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_NUM; i++ ){
			for( j=0; j<WFLBY_3DMAPOBJ_FLOAT_ANM_NUM; j++ ){
			#if WB_FIX
				D3DOBJ_AnmDelete( &p_wk->anm[i][j], p_allocator );
			#else
				GFL_G3D_ANIME_Delete( p_wk->anm[i][j] );
				GFL_G3D_DeleteResource( p_wk->p_anmres[i][j] );
			#endif
			}
		}
	}

	// モデル破棄
	{
		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_NUM; i++ ){
		#if WB_FIX
			GFL_HEAP_FreeMemory( p_wk->mdl[i].pResMdl );
		#else
			GFL_G3D_DeleteResource(p_wk->p_mdlres[i]);
		#endif
		}
	}

	// テクスチャ破棄
	{
		NNSG3dTexKey texKey;
		NNSG3dTexKey tex4x4Key;
		NNSG3dPlttKey plttKey;
		NNSG3dResTex* p_tex;


		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_NUM; i++ ){
			for( j=0; j<WFLBY_3DMAPOBJ_FLOAT_COL_NUM; j++ ){
			#if WB_FIX
				p_tex = NNS_G3dGetTex( p_wk->p_texres[i][j] );
				//VramKey破棄
				NNS_G3dTexReleaseTexKey( p_tex, &texKey, &tex4x4Key );
				NNS_GfdFreeTexVram( texKey );	
				NNS_GfdFreeTexVram( tex4x4Key );	

				plttKey = NNS_G3dPlttReleasePlttKey( p_tex );
				NNS_GfdFreePlttVram( plttKey );

				// メモリ破棄
				GFL_HEAP_FreeMemory( p_wk->p_texres[i][j] );
			#else
				//ここで吹っ飛ぶようならWFLBY_3DMAPOBJ_TEX_LoatCutTexでテクスチャヒープを
				//実体部分の解放をしているからかも 2009.03.25(水) matsuda
				GFL_G3D_FreeVramTexture(p_wk->p_texres[i][j]);
				GFL_G3D_DeleteResource(p_wk->p_texres[i][j]);
			#endif
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート　メイン処理
 *
 *	@param	p_wk		ワーク
 *	@param	p_res		リソース
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_FLOAT_Main( WFLBY_3DMAPOBJ_FLOAT* p_wk, WFLBY_3DMAPOBJ_FLOATRES* p_res )
{
	int i;

	// アニメさせる
	{
		fx32 end_frame;
		for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_ANM_NUM; i++ ){
			if( p_wk->anm_on[i] ){
				switch(i){
				case WFLBY_3DMAPOBJ_FLOAT_ANM_00:
				case WFLBY_3DMAPOBJ_FLOAT_ANM_02:		// 頭
//					OS_TPrintf( "anm roop %d=%d  ",  i, p_wk->anm_frame[i] );
					WFLBY_3DMAPOBJ_ANM_Loop( &p_wk->anm_frame[i], p_res->anm[ p_wk->mdlno ][ i ], FX32_ONE );
					break;
				case WFLBY_3DMAPOBJ_FLOAT_ANM_01:		// 泣き声用
				case WFLBY_3DMAPOBJ_FLOAT_ANM_03:		// 尻尾
					{
						BOOL result;
//						OS_TPrintf( "anm noroop %d=%d  ",  i, p_wk->anm_frame[i] );
						result = WFLBY_3DMAPOBJ_ANM_NoLoop( &p_wk->anm_frame[i], p_res->anm[ p_wk->mdlno ][ i ], FX32_ONE );
						if( result == TRUE ){
							// アニメ停止
							p_wk->anm_on[i] = FALSE;
							p_wk->anm_frame[i] = 0;
						}
					}
					break;
				default:
					GF_ASSERT(0);
					break;
				}
			}
		}
	}			
//	OS_TPrintf( "\n",  i, p_wk->anm_frame[i] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートの描画処理
 *
 *	@param	p_wk	ワーク
 *	@param	p_res	リソース
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_FLOAT_Draw( WFLBY_3DMAPOBJ_FLOAT* p_wk, WFLBY_3DMAPOBJ_FLOATRES* p_res )
{
	BOOL result;
	int i, set_anmframe;
	GFL_G3D_RND *g3dRnd;
	
	if( p_wk->on == FALSE ){
		return ;
	}

	g3dRnd = GFL_G3D_OBJECT_GetG3Drnd(p_wk->g3dobj);
	
	// FLOAT描画チェック
#if WB_FIX	//描画の方でカリングONでする為、ここの処理はなし
	if( WFLBY_3DMAPOBJ_MDL_BOXCheck( &p_res->mdl[ p_wk->mdlno ], &p_wk->obj ) == FALSE ){
		return ;
	}
#endif

	GF_ASSERT( p_wk->col < WFLBY_3DMAPOBJ_FLOAT_COL_NUM );
	GF_ASSERT( p_wk->mdlno < WFLBY_3DMAPOBJ_FLOAT_NUM );


	// カラーのテクスチャをバインド
#if WB_FIX
	p_res->mdl[ p_wk->mdlno ].pMdlTex = NNS_G3dGetTex( p_res->p_texres[ p_wk->mdlno ][ p_wk->col ] );
	result = NNS_G3dBindMdlSet(p_res->mdl[ p_wk->mdlno ].pModelSet, p_res->mdl[ p_wk->mdlno ].pMdlTex );
	GF_ASSERT( result );
#else
	GFL_G3D_RENDER_SetTexture( g3dRnd, 0, p_res->p_texres[ p_wk->mdlno ][ p_wk->col ] );
#endif

	// アニメの適用
	for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_ANM_NUM; i++ ){
		if( p_wk->anm_on[i] ){
		#if WB_FIX
			D3DOBJ_AddAnm( &p_wk->obj, &p_res->anm[ p_wk->mdlno ][ i ] );
			// フレームをあわせる
			D3DOBJ_AnmSet( &p_res->anm[ p_wk->mdlno ][ i ], p_wk->anm_frame[i] );
		#else
			GFL_G3D_OBJECT_EnableAnime( p_wk->g3dobj, i );
			// フレームをあわせる
			set_anmframe = p_wk->anm_frame[i];
			GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
		#endif
		}
	}

#if WB_FIX
	D3DOBJ_Draw( &p_wk->obj );
#else
	if(p_wk->draw_flag == TRUE){
		GFL_G3D_DRAW_DrawObjectCullingON(p_wk->g3dobj, &p_wk->objst);
	}
#endif

	// アニメを破棄
	for( i=0; i<WFLBY_3DMAPOBJ_FLOAT_ANM_NUM; i++ ){
		if( p_wk->anm_on[i] ){
		#if WB_FIX
			D3DOBJ_DelAnm( &p_wk->obj, &p_res->anm[ p_wk->mdlno ][ i ] );
		#else
			GFL_G3D_OBJECT_DisableAnime(p_wk->g3dobj, i);
		#endif
		}
	}

	// バインド解除
#if WB_FIX
	NNS_G3dReleaseMdlSet(p_res->mdl[ p_wk->mdlno ].pModelSet );
#else
	{
		NNSG3dResFileHeader *file_head;
		file_head = GFL_G3D_GetResourceFileHeader(GFL_G3D_RENDER_GetG3DresMdl(g3dRnd));
		NNS_G3dReleaseMdlSet( NNS_G3dGetMdlSet( file_head ) );
	}
#endif
}

// 配置オブジェクトワーク
//----------------------------------------------------------------------------
/**
 *	@brief	空いているワークを取得する
 *
 *	@param	p_sys	システムワーク
 *	
 *	@return	空いているワーク
 */
//-----------------------------------------------------------------------------
static WFLBY_3DMAPOBJ_WK* WFLBY_3DMAPOBJ_WK_GetCleanWk( WFLBY_3DMAPOBJ* p_sys )
{
	int i;

	for( i=0; i<p_sys->objnum; i++ ){
		if( p_sys->p_obj[i].on == FALSE ){
			return &p_sys->p_obj[i];
		}
	}

	GF_ASSERT(0);
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	配置オブジェリソース読み込み
 *
 *	@param	p_wk		ワーク
 *	@param	p_handle	ハンドル
 *	@param	cp_data		データ
 *	@param	gheapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_WK_Load( WFLBY_3DMAPOBJ_WKRES* p_wk, ARCHANDLE* p_handle, const WFLBY_3DMAPOBJ_MDL_DATA* cp_data, u32 gheapID )
{
	int i, j;

	for( i=0; i<WFLBY_3DMAPOBJ_WK_NUM; i++ ){
//		OS_TPrintf( "wk load mdl idx = %d\n",  cp_data->obj_idx[i] );
		WFLBY_3DMAPOBJ_MDL_Load( &p_wk->p_mdlres[i], p_handle, cp_data->obj_idx[i], gheapID );
		p_wk->rnder[i] = GFL_G3D_RENDER_Create( p_wk->p_mdlres[i], 0, p_wk->p_mdlres[i] );

		// ライト設定
		if( (i != WFLBY_3DMAPOBJ_LAMP00) && 
			(i != WFLBY_3DMAPOBJ_LAMP01) ){
		#if WB_FIX
			WFLBY_LIGHT_SetUpMdl( p_wk->mdl[ i ].pResMdl );	// ライト反映
		#else
			WFLBY_LIGHT_SetUpMdl( GFL_G3D_GetResourceFileHeader(p_wk->p_mdlres[ i ]) );	// ライト反映
		#endif
		}

		// モデル分のアニメを読み込む
		for( j=0; j<WFLBY_3DMAPOBJ_ALL_ANM_MAX; j++ ){
			
			// オブジェのIDと同じなら読み込まない
			if( cp_data->obj_anm[i][j] != cp_data->obj_idx[i] ){
			#if WB_FIX
				p_wk->p_anm[i][j] = GFL_ARCHDL_UTIL_Load( p_handle, cp_data->obj_anm[i][j], FALSE, gheapID );
			#else
				p_wk->p_anmres[i][j] = GFL_G3D_CreateResourceHandle(p_handle, cp_data->obj_anm[i][j]);
			#endif
			}else{
				p_wk->p_anmres[i][j] = NULL;
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	配置オブジェリソース破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_WK_Release( WFLBY_3DMAPOBJ_WKRES* p_wk )
{
	int i, j;

	for( i=0; i<WFLBY_3DMAPOBJ_WK_NUM; i++ ){

		// アニメリソースを破棄
		for( j=0; j<WFLBY_3DMAPOBJ_ALL_ANM_MAX; j++ ){
/*			if( p_wk->anm[i][j].pResAnm != NULL ){
				D3DOBJ_AnmDelete( &p_wk->anm[i][j], p_allocator );
				p_wk->anm[i][j].pResAnm = NULL;
			}//*/

			if( p_wk->p_anmres[i][j] != NULL ){
			#if WB_FIX
				GFL_HEAP_FreeMemory( p_wk->p_anm[i][j] );
			#else
				GFL_G3D_DeleteResource( p_wk->p_anmres[i][j] );
			#endif
				p_wk->p_anmres[i][j] = NULL;
			}

		}
		
		WFLBY_3DMAPOBJ_MDL_Delete( p_wk->p_mdlres[i] );
		GFL_G3D_RENDER_Delete(p_wk->rnder[i]);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクト描画
 *
 *	@param	p_res	リソースオブジェ
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_WK_Draw( WFLBY_3DMAPOBJ_WKRES* p_res, WFLBY_3DMAPOBJ_WK* p_wk )
{
	int i;
	NNSG3dResMdlSet*		pMdlset;
	NNSG3dResMdl*			pMdl;

	if( p_wk->on == FALSE ){
		return ;
	}

	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_INIT;
	
	// 描画チェック
#if WB_FIX	//描画の方でカリングONでする為、ここの処理はなし
	if( WFLBY_3DMAPOBJ_MDL_BOXCheck( &p_res->mdl[ p_wk->mdlid ], &p_wk->obj ) == FALSE ){
		return ;
	}
#endif
	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT( __LINE__ );


	/*
	// アニメiのフレーム設定
	for( i=0; i<WFLBY_3DMAPOBJ_ALL_ANM_MAX; i++ ){
		// アニメがONになっていてRANDWAITのwaitが終わってたらアニメフレーム設定
		if( p_wk->anm_on[i] && (p_wk->wait[i] == 0) ){
			// フレームをあわせる
			D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
		}
	}
	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT( __LINE__ );
	//*/

	pMdlset = NNS_G3dGetMdlSet(GFL_G3D_GetResourceFileHeader(p_res->p_mdlres[p_wk->mdlid]));
	pMdl = NNS_G3dGetMdlByIdx( pMdlset, 0 );

	// アルファ値の適用
	if( p_wk->alpha_flag == TRUE ){
		NNS_G3dMdlSetMdlAlphaAll( pMdl, p_wk->alpha );
	}

#if WB_FIX
	D3DOBJ_Draw( &p_wk->obj );
#else
	if(p_wk->draw_flag == TRUE){
		GFL_G3D_DRAW_DrawObjectCullingON(p_wk->g3dobj, &p_wk->objst);
	}
#endif
	WFLBY_DEBUG_3DMAPOBJ_PRINT_TIME_TICK_PRINT( __LINE__ );

	// アルファ値を戻す
	if( p_wk->alpha_flag == TRUE ){
		NNS_G3dMdlSetMdlAlphaAll( pMdl, p_wk->def_alpha );
	}

}


//----------------------------------------------------------------------------
/**
 *	@brief	起動中のアニメをループ再生
 *
 *	@param	p_sys	システムワーク
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_3DMAPOBJ_WK_Anm( WFLBY_3DMAPOBJ* p_sys, WFLBY_3DMAPOBJ_WK* p_wk )
{
	int i, set_anmframe;
	BOOL result;
	pWFLBY_3DMAPOBJ_WK_AnmCallBack p_callback;

	for( i=0; i<WFLBY_3DMAPOBJ_ALL_ANM_MAX; i++ ){
		if( p_wk->anm_on[i] ){
			switch( p_wk->play[i] ){
			// ループ
			case WFLBY_3DMAPOBJ_WK_ANM_LOOP:
				WFLBY_3DMAPOBJ_ANM_Loop( &p_wk->anm_frame[i], p_wk->anm[i], p_wk->speed );
			#if WB_FIX
				D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
			#else
				set_anmframe = p_wk->anm_frame[i];
				GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
			#endif
				break;

			// ループなし
			case WFLBY_3DMAPOBJ_WK_ANM_NOLOOP:
				result = WFLBY_3DMAPOBJ_ANM_NoLoop( &p_wk->anm_frame[i], p_wk->anm[i], p_wk->speed );
			#if WB_FIX
				D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
			#else
				set_anmframe = p_wk->anm_frame[i];
				GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
			#endif
				if( result == TRUE ){
					
					// 保存
					p_callback = p_wk->p_anmcallback[i];

					//  アニメを破棄
					WFLBY_3DMAPOBJ_WK_DelAnm( p_sys, p_wk, i );

					// コールバックがあったら呼ぶ
					if( p_callback ){
						p_callback( p_sys, p_wk );
					}

				}
				break;

			// 逆再生ループ
			case WFLBY_3DMAPOBJ_WK_ANM_BACKLOOP:
				WFLBY_3DMAPOBJ_ANM_BackLoop( &p_wk->anm_frame[i], p_wk->anm[i], p_wk->speed );
			#if WB_FIX
				D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
			#else
				set_anmframe = p_wk->anm_frame[i];
				GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
			#endif
				break;

			// 逆再生ループなし
			case WFLBY_3DMAPOBJ_WK_ANM_BACKNOLOOP:
				result = WFLBY_3DMAPOBJ_ANM_BackNoLoop( &p_wk->anm_frame[i], p_wk->anm[i], p_wk->speed );
			#if WB_FIX
				D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
			#else
				set_anmframe = p_wk->anm_frame[i];
				GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
			#endif
				if( result == TRUE ){
					// 保存
					p_callback = p_wk->p_anmcallback[i];

					//  アニメを破棄
					WFLBY_3DMAPOBJ_WK_DelAnm( p_sys, p_wk, i );

					// コールバックがあったら呼ぶ
					if( p_callback ){
						p_callback( p_sys, p_wk );
					}

				}
				break;

			// ランダムウエイト再生	ループ
			case WFLBY_3DMAPOBJ_WK_ANM_RANDWAIT:
				// ウエイトしてから再生
				if( p_wk->wait[i] > 0 ){
					p_wk->wait[i]--;
				}else{
					result = WFLBY_3DMAPOBJ_ANM_NoLoop( &p_wk->anm_frame[i], p_wk->anm[i], p_wk->speed );
					if( result == TRUE ){
						// ループ再生
						p_wk->wait[i] = GFUser_GetPublicRand(p_wk->wait_def);
						p_wk->anm_frame[i] = 0;
					}
				#if WB_FIX
					D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
				#else
					set_anmframe = p_wk->anm_frame[i];
					GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
				#endif
				}
				break;

			// ランダムスタート再生	ループ
			case WFLBY_3DMAPOBJ_WK_ANM_RANDSTART:
				// ウエイトしてから再生
				if( p_wk->wait[i] > 0 ){
					p_wk->wait[i]--;
				}else{
					WFLBY_3DMAPOBJ_ANM_Loop( &p_wk->anm_frame[i], p_wk->anm[i], p_wk->speed );
				#if WB_FIX
					D3DOBJ_AnmSet( &p_wk->anm[i], p_wk->anm_frame[i] );
				#else
					set_anmframe = p_wk->anm_frame[i];
					GFL_G3D_OBJECT_SetAnimeFrame( p_wk->g3dobj, i, &set_anmframe );
				#endif
				}
				break;

			// なにもしない
			case WFLBY_3DMAPOBJ_WK_ANM_NOAUTO:
			default:
				break;

			}

		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   GFL_G3D_OBJSTATUSの初期パラメータセット
 *
 * @param   status		
 */
//--------------------------------------------------------------
static void WFLBY_G3DOBJSTATUS_Init(GFL_G3D_OBJSTATUS *status, VecFx32 *rotate)
{
	VEC_Set(&status->trans, 0, 0, 0);
	VEC_Set(&status->scale, FX32_ONE, FX32_ONE, FX32_ONE);
	MTX_Identity33(&status->rotate);
	
	VEC_Set(rotate, 0, 0, 0);
}

//--------------------------------------------------------------
/**
 * @brief   回転行列の計算
 *
 * @param   status		
 * @param   rotate		
 */
//--------------------------------------------------------------
static void WFLBY_RotateMtx(VecFx32 *rotate, MtxFx33 *dst_mtx)
{
	MtxFx33 mtx, calc_mtx;
	
	MTX_Identity33( &mtx );
	MTX_RotX33( &calc_mtx, FX_SinIdx( rotate->x ), FX_CosIdx( rotate->x ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	MTX_RotZ33( &calc_mtx, FX_SinIdx( rotate->z ), FX_CosIdx( rotate->z ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	MTX_RotY33( &calc_mtx, FX_SinIdx( rotate->y ), FX_CosIdx( rotate->y ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	
	*dst_mtx = mtx;
}

