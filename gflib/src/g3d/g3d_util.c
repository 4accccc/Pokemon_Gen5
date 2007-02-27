//=============================================================================================
/**
 * @file	g3d_util.c                                                  
 * @brief	３Ｄデータ管理ユーティリティープログラム
 * @date	
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================
struct _GFL_G3D_UTIL {
	GFL_G3D_RES**		g3DresTbl;
	u8*					g3DresReference;
	u16					g3DresCount;

	GFL_G3D_OBJ**		g3DobjTbl;
	u8*					g3DobjExResourceRef;
	u16					g3DobjCount;

	HEAPID				heapID;
};

enum {
	RES_NO_REFERENCE	= 0x00,
	RES_ON_REFERENCE	= 0x01,
};

enum {
	EXRES_NULL			= 0x00,
	EXRES_MDL			= 0x01,
	EXRES_TEX			= 0x02,
};

//=============================================================================================
/**
 *
 *
 * ３Ｄ関連ハンドルの管理ユーティリティー
 *
 *
 */
//=============================================================================================
#define pHANDLE_SIZE (4)

#define RES_MDL_CHECK( res ) ( GFL_G3D_ResTypeCheck( res, GFL_G3D_RES_CHKTYPE_MDL ) )
#define RES_TEX_CHECK( res ) ( GFL_G3D_ResTypeCheck( res, GFL_G3D_RES_CHKTYPE_TEX ) )
			
static inline BOOL resourceLoad( const GFL_G3D_UTIL_RES* resTbl, GFL_G3D_RES** p_g3Dres );
static inline BOOL resourceHandleGet( GFL_G3D_UTIL* g3Dutil, const GFL_G3D_UTIL_SETUP* setup,
										GFL_G3D_RES** p_g3Dres, u16 idx );
//--------------------------------------------------------------------------------------------
/**
 * セットアップ
 *
 * @param	setup				設定データ
 * @param	heapID				ヒープＩＤ
 *
 * @return	GFL_G3D_UTIL*		データセットハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_UTIL*
	GFL_G3D_UtilsysCreate
		( const GFL_G3D_UTIL_SETUP* setup, HEAPID heapID )
{
	GFL_G3D_UTIL*			g3Dutil;
	const GFL_G3D_UTIL_OBJ*	objTbl;
	GFL_G3D_RES				*g3DresMdl, *g3DresTex, *g3DresAnm;
	GFL_G3D_RND*			g3Drnd;
	GFL_G3D_ANM*			g3Danm;
	GFL_G3D_ANM**			g3DanmTbl;
	int						i,j;

	//管理領域確保
	g3Dutil = GFL_HEAP_AllocMemoryClear( heapID, sizeof(GFL_G3D_UTIL) );

	//リソース管理配列作成
	g3Dutil->g3DresTbl = GFL_HEAP_AllocMemoryClear( heapID, pHANDLE_SIZE * setup->resCount );
	g3Dutil->g3DresReference = GFL_HEAP_AllocMemoryClear( heapID, setup->resCount );
	g3Dutil->g3DresCount = setup->resCount;
	//リソース読み込み
	for( i=0; i<setup->resCount; i++ ){
		resourceLoad( &setup->resTbl[i], &g3Dutil->g3DresTbl[i] );
	}
	//リソース参照フラグ初期化
	for( i=0; i<setup->objCount; i++ ){
		g3Dutil->g3DresReference[i] = RES_NO_REFERENCE;
	}
	//テクスチャデータ連続転送（試しに効率がいいかもしれない方をやってみる）
	{
		GX_BeginLoadTex();
		for( i=0; i<setup->resCount; i++ ){
			if( RES_TEX_CHECK( g3Dutil->g3DresTbl[i] ) == TRUE ){
				GFL_G3D_VramLoadTexDataOnly( g3Dutil->g3DresTbl[i] );
			}
		}
		GX_EndLoadTex();
	}
	//テクスチャパレット連続転送（試しに効率がいいかもしれない方をやってみる）
	{
		GX_BeginLoadTexPltt();
		for( i=0; i<setup->resCount; i++ ){
			if( RES_TEX_CHECK( g3Dutil->g3DresTbl[i] ) == TRUE ){
				GFL_G3D_VramLoadTexPlttOnly( g3Dutil->g3DresTbl[i] );
			}
		}
		GX_EndLoadTexPltt();
	}
	//オブジェクト管理配列作成
	g3Dutil->g3DobjTbl = GFL_HEAP_AllocMemoryClear( heapID, pHANDLE_SIZE * setup->objCount );
	g3Dutil->g3DobjExResourceRef = GFL_HEAP_AllocMemoryClear( heapID, setup->objCount );
	g3Dutil->g3DobjCount = setup->objCount;
	//オブジェクト追加リソース作成フラグ初期化
	for( i=0; i<setup->objCount; i++ ){
		g3Dutil->g3DobjExResourceRef[i] = EXRES_NULL;
	}

	//オブジェクト作成
	for( i=0; i<setup->objCount; i++ ){
		objTbl = &setup->objTbl[i];

		//モデルリソースの参照指定(リソースを共有出来ないので追加読み込みチェックを行う)
		if( resourceHandleGet(g3Dutil, setup, &g3DresMdl, objTbl->mdlresID ) == TRUE ){
			g3Dutil->g3DobjExResourceRef[i] |= EXRES_MDL;	//追加リソースフラグセット
		}
		//テクスチャリソースの参照指定
		if( objTbl->texresID != objTbl->mdlresID ){
			//モデル内包リソースの場合は共有チェックを行う
			if( RES_MDL_CHECK( g3Dutil->g3DresTbl[ objTbl->texresID ] ) == TRUE ){
				if( resourceHandleGet(g3Dutil, setup, &g3DresTex, objTbl->texresID ) == TRUE){
					g3Dutil->g3DobjExResourceRef[i] |= EXRES_TEX;	//追加リソースフラグセット
					GFL_G3D_VramLoadTex( g3DresTex );
				}
			} else {
				g3DresTex = g3Dutil->g3DresTbl[ objTbl->texresID ];
			}
		} else {
			//モデルリソースと同じ指定の場合はモデル依存で共有
			g3DresTex = g3DresMdl;
			//テクスチャVRAM未転送の場合は転送する
			if( GFL_G3D_VramTexkeyLiveCheck( g3DresTex ) == FALSE ){
				GFL_G3D_VramLoadTex( g3DresTex );
			}
		}
		//レンダー作成
		g3Drnd = GFL_G3D_RndCreate( g3DresMdl, objTbl->mdldatID, g3DresTex ); 

		//アニメーションハンドルテンポラリ作成
		g3DanmTbl = GFL_HEAP_AllocMemoryLowClear( heapID, objTbl->anmCount );

		//アニメーション設定
		for( j=0; j<objTbl->anmCount; j++ ){
			//アニメーションリソースの参照指定
			GF_ASSERT( objTbl->anmTbl[j].anmresID < setup->resCount );
			g3DresAnm = g3Dutil->g3DresTbl[ objTbl->anmTbl[j].anmresID ];
			//アニメーション作成
			g3Danm = GFL_G3D_AnmCreate( g3Drnd, g3DresAnm, objTbl->anmTbl[j].anmdatID );
			g3DanmTbl[j] = g3Danm;
		}
		//オブジェクト作成
		g3Dutil->g3DobjTbl[i] = GFL_G3D_ObjCreate( g3Drnd, g3DanmTbl, objTbl->anmCount );

		GFL_HEAP_FreeMemory( g3DanmTbl );
	}
	return g3Dutil;
}

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 *
 * @param	GFL_G3D_UTIL*		データセットハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilsysDelete
		( GFL_G3D_UTIL* g3Dutil )
{
	GFL_G3D_RES			*g3DresMdl,	*g3DresTex, *g3DresAnm;
	GFL_G3D_RND*		g3Drnd;
	GFL_G3D_ANM*		g3Danm;
	GFL_G3D_OBJ*		g3Dobj;
	int					i,j;
	u16					anmCount;

	//オブジェクト破棄
	for( i=0; i<g3Dutil->g3DobjCount; i++ ){
		g3Dobj = g3Dutil->g3DobjTbl[i];
		g3Drnd = GFL_G3D_ObjG3DrndGet( g3Dobj );

		//アニメーション解放
		anmCount = GFL_G3D_ObjAnmCountGet( g3Dobj );
		for( j=0; j<anmCount; j++ ){
			GFL_G3D_AnmDelete( GFL_G3D_ObjG3DanmGet( g3Dobj, j ) );
		}
		//追加テクスチャリソース作成フラグ確認
		if( g3Dutil->g3DobjExResourceRef[i] & (EXRES_TEX^0xff) ){
			//テクスチャリソース解放
			g3DresTex = GFL_G3D_RndG3DresTexGet( g3Drnd );
			GFL_G3D_VramUnloadTex( g3DresTex );
			GFL_G3D_ResDelete( g3DresTex );
		}
		//追加モデルリソース作成フラグ確認
		if( g3Dutil->g3DobjExResourceRef[i] & (EXRES_MDL^0xff) ){
			//モデルリソース解放
			g3DresMdl = GFL_G3D_RndG3DresMdlGet( g3Drnd );
			GFL_G3D_ResDelete( g3DresMdl );
		}	
		//レンダー解放
		GFL_G3D_RndDelete( g3Drnd ); 

		//オブジェクト解放
		GFL_G3D_ObjDelete( g3Dutil->g3DobjTbl[i] );
	}
	//オブジェクト管理配列解放
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjExResourceRef );
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjTbl );

	//リソース破棄
	for( i=0; i<g3Dutil->g3DresCount; i++ ){
		if( g3Dutil->g3DresTbl[i] ){
			if( RES_TEX_CHECK( g3Dutil->g3DresTbl[i] ) == TRUE ){
				//テクスチャリソースの場合ＶＲＡＭ解放（内部で転送したかのチェックは行っている）
				GFL_G3D_VramUnloadTex( g3Dutil->g3DresTbl[i] ); 
			}
			GFL_G3D_ResDelete( g3Dutil->g3DresTbl[i] ); 
		}
	}
	//リソース管理配列作成
	GFL_HEAP_FreeMemory( g3Dutil->g3DresReference );
	GFL_HEAP_FreeMemory( g3Dutil->g3DresTbl );
	
	//管理領域解放
	GFL_HEAP_FreeMemory( g3Dutil );
}

//--------------------------------------------------------------------------------------------
/**
 * リソース読み込み
 */
//--------------------------------------------------------------------------------------------
static inline BOOL resourceLoad( const GFL_G3D_UTIL_RES* resTbl, GFL_G3D_RES** p_g3Dres )
{
	if( resTbl->arcType == GFL_G3D_UTIL_RESARC ){
		//アーカイブＩＤより
		*p_g3Dres = GFL_G3D_ResCreateArc( (int)resTbl->arcive, resTbl->datID ); 
		return TRUE;
	} else {			
		//アーカイブパスより
		*p_g3Dres = GFL_G3D_ResCreatePath( (const char*)resTbl->arcive, resTbl->datID ); 
		return TRUE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * リソースハンドルを取得（参照済みの場合は追加読み込み）
 *	共有できないリソース設定の際に使用
 */
//--------------------------------------------------------------------------------------------
static inline BOOL resourceHandleGet( GFL_G3D_UTIL* g3Dutil, const GFL_G3D_UTIL_SETUP* setup,
										GFL_G3D_RES** p_g3Dres, u16 idx )
{
	GF_ASSERT( idx < setup->resCount );

	if( g3Dutil->g3DresReference[ idx ] == RES_ON_REFERENCE ){
		resourceLoad( &setup->resTbl[ idx ], p_g3Dres );	//追加読み込み
		return TRUE;
	} else {
		g3Dutil->g3DresReference[ idx ] = RES_ON_REFERENCE;//参照済フラグセット
		*p_g3Dres = g3Dutil->g3DresTbl[ idx ];
		return FALSE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドル取得
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_UtilsysObjHandleGet
		( GFL_G3D_UTIL* g3Dutil, u16 idx )
{
	return g3Dutil->g3DobjTbl[idx];
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクト数取得
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UtilsysObjCountGet
		( GFL_G3D_UTIL* g3Dutil )
{
	return g3Dutil->g3DobjCount;
}



