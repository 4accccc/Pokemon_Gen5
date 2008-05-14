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
#define UNIT_COUNT_MAX	(64)
#define UNIT_NULL		(0xffff)

typedef struct {
	u16	resIdx;
	u16	resCount;
	u16	objIdx;
	u16	objCount;

}GFL_G3D_UTIL_UNIT;

struct _GFL_G3D_UTIL {
	GFL_G3D_RES**		g3DresTbl;
	u8*					g3DresReference;
	u16					g3DresCount;

	GFL_G3D_OBJ**		g3DobjTbl;
	u8*					g3DobjExResourceRef;
	u16					g3DobjCount;

	GFL_G3D_UTIL_UNIT	unit[ UNIT_COUNT_MAX ];

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
#define RES_MDL_CHECK( res ) ( GFL_G3D_CheckResourceType( res, GFL_G3D_RES_CHKTYPE_MDL ) )
#define RES_TEX_CHECK( res ) ( GFL_G3D_CheckResourceType( res, GFL_G3D_RES_CHKTYPE_TEX ) )
			
static void addResource
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx, const GFL_G3D_UTIL_SETUP* setup, HEAPID heapID );
static void delResource
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx );
static void addObject
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx, const GFL_G3D_UTIL_SETUP* setup, HEAPID heapID );
static void delObject
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx );
//--------------------------------------------------------------------------------------------
/**
 * セットアップ
 *
 * @param	resCountMax			設定リソース最大数定
 * @param	objCountMax			設定オブジェクト最大数
 * @param	heapID				ヒープＩＤ
 *
 * @return	GFL_G3D_UTIL*		データセットハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_UTIL*
	GFL_G3D_UTIL_Create
		( u16 resCountMax, u16 objCountMax, HEAPID heapID )
{
	GFL_G3D_UTIL*			g3Dutil;
	int						i;

	//管理領域確保
	g3Dutil = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_UTIL) );

	//リソース管理配列作成
	g3Dutil->g3DresTbl = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_RES*) * resCountMax );
	g3Dutil->g3DresReference = GFL_HEAP_AllocClearMemory( heapID, resCountMax );
	g3Dutil->g3DresCount = resCountMax;
	//リソース参照フラグ初期化
	for( i=0; i<resCountMax; i++ ){
		g3Dutil->g3DresReference[i] = RES_NO_REFERENCE;
	}
	//オブジェクト管理配列作成
	g3Dutil->g3DobjTbl = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_OBJ*) * objCountMax );
	g3Dutil->g3DobjExResourceRef = GFL_HEAP_AllocClearMemory( heapID, objCountMax );
	g3Dutil->g3DobjCount = objCountMax;
	//オブジェクト追加リソース作成フラグ初期化
	for( i=0; i<objCountMax; i++ ){
		g3Dutil->g3DobjExResourceRef[i] = EXRES_NULL;
	}
#if 0
	addResource( g3Dutil, setup, heapID );
	addObject( g3Dutil, setup, heapID );
#else
	for( i=0; i<UNIT_COUNT_MAX; i++ ){
		g3Dutil->unit[i].resIdx = UNIT_NULL;
		g3Dutil->unit[i].resCount = UNIT_NULL;
		g3Dutil->unit[i].objIdx = UNIT_NULL;
		g3Dutil->unit[i].objCount = UNIT_NULL;
	}
#endif
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
	GFL_G3D_UTIL_Delete
		( GFL_G3D_UTIL* g3Dutil )
{
#if 0
	delObject( g3Dutil );
	delResource( g3Dutil );
#else
	 int i;

	for( i=0; i<UNIT_COUNT_MAX; i++ ){
		if( g3Dutil->unit[i].resIdx != UNIT_NULL ){
			GF_ASSERT(0);
		}
	}
#endif
	//オブジェクト管理配列解放
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjExResourceRef );
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjTbl );
	//リソース管理配列開放
	GFL_HEAP_FreeMemory( g3Dutil->g3DresReference );
	GFL_HEAP_FreeMemory( g3Dutil->g3DresTbl );
	
	//管理領域解放
	GFL_HEAP_FreeMemory( g3Dutil );
}

//--------------------------------------------------------------------------------------------
/**
 * ユニットセットアップ
 *
 * @param	GFL_G3D_UTIL*		データセットハンドル
 * @param	setup				設定データ
 *
 * @return	unitIdx				ユニットインデックス
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UTIL_AddUnit
		( GFL_G3D_UTIL* g3Dutil, const GFL_G3D_UTIL_SETUP* setup )
{
	int unitIdx;
	u16 resIdx, objIdx;

	resIdx = 0;
	objIdx = 0;

	for( unitIdx=0; unitIdx<UNIT_COUNT_MAX; unitIdx++ ){
		if( g3Dutil->unit[unitIdx].resIdx == UNIT_NULL ){
			if( resIdx + setup->resCount > g3Dutil->g3DresCount ){
				GF_ASSERT(0);
			}
			if( objIdx + setup->objCount > g3Dutil->g3DobjCount ){
				GF_ASSERT(0);
			}
			g3Dutil->unit[unitIdx].resIdx =	resIdx; 
			g3Dutil->unit[unitIdx].resCount = setup->resCount;
			g3Dutil->unit[unitIdx].objIdx = objIdx;
			g3Dutil->unit[unitIdx].objCount = setup->objCount;

			addResource( g3Dutil, unitIdx, setup, g3Dutil->heapID );
			addObject( g3Dutil, unitIdx, setup, g3Dutil->heapID );
			return unitIdx;
		}
		resIdx = g3Dutil->unit[unitIdx].resIdx + g3Dutil->unit[unitIdx].resCount;
		objIdx = g3Dutil->unit[unitIdx].objIdx + g3Dutil->unit[unitIdx].objCount;
	}
	GF_ASSERT(0);
	return UNIT_NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * ユニット破棄
 *
 * @param	GFL_G3D_UTIL*		データセットハンドル
 * @param	unitIdx				ユニットインデックス
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UTIL_DelUnit
		( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	if( g3Dutil->unit[unitIdx].resIdx == UNIT_NULL ){
		GF_ASSERT(0);
	}
	delObject( g3Dutil, unitIdx );
	delResource( g3Dutil, unitIdx );

	g3Dutil->unit[unitIdx].resIdx =	UNIT_NULL;
	g3Dutil->unit[unitIdx].resCount = UNIT_NULL;
	g3Dutil->unit[unitIdx].objIdx = UNIT_NULL;
	g3Dutil->unit[unitIdx].objCount = UNIT_NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * リソース読み込み
 */
//--------------------------------------------------------------------------------------------
static inline BOOL loadResource( const GFL_G3D_UTIL_RES* resTbl, GFL_G3D_RES** p_g3Dres )
{
	if( resTbl->arcType == GFL_G3D_UTIL_RESARC ){
		//アーカイブＩＤより
		*p_g3Dres = GFL_G3D_CreateResourceArc( (int)resTbl->arcive, resTbl->datID ); 
		return TRUE;
	} else {			
		//アーカイブパスより
		*p_g3Dres = GFL_G3D_CreateResourcePath( (const char*)resTbl->arcive, resTbl->datID ); 
		return TRUE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * リソースハンドルを取得（参照済みの場合は追加読み込み）
 *	共有できないリソース設定の際に使用
 */
//--------------------------------------------------------------------------------------------
static inline BOOL getResourceHandle( GFL_G3D_UTIL* g3Dutil, const GFL_G3D_UTIL_SETUP* setup,
										GFL_G3D_RES** p_g3Dres, u16 idxoffs, u16 idx )
{
//	GF_ASSERT( idx < setup->resCount );

	if( g3Dutil->g3DresReference[ idxoffs+idx ] == RES_ON_REFERENCE ){
		loadResource( &setup->resTbl[ idx ], p_g3Dres );	//追加読み込み
		return TRUE;
	} else {
		g3Dutil->g3DresReference[ idxoffs+idx ] = RES_ON_REFERENCE;//参照済フラグセット
		*p_g3Dres = g3Dutil->g3DresTbl[ idxoffs+idx ];
		return FALSE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * テクスチャ転送
 */
//--------------------------------------------------------------------------------------------
static inline void transTexture( GFL_G3D_RES* g3DresTex )
{
	//テクスチャVRAM未転送の場合は転送する
	if( GFL_G3D_CheckTextureKeyLive( g3DresTex ) == FALSE ){
		GFL_G3D_TransVramTexture( g3DresTex );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * リソースセットアップと削除
 */
//--------------------------------------------------------------------------------------------
static void addResource
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx, const GFL_G3D_UTIL_SETUP* setup, HEAPID heapID )
{
	int i;
	u16 resIdx = g3Dutil->unit[unitIdx].resIdx;
	u16 resCount = g3Dutil->unit[unitIdx].resCount;

	for( i=0; i<resCount; i++ ){
		//リソース読み込み
		loadResource( &setup->resTbl[i], &g3Dutil->g3DresTbl[resIdx+i] );
	}
}

static void delResource
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	int i;
	u16 resIdx = g3Dutil->unit[unitIdx].resIdx;
	u16 resCount = g3Dutil->unit[unitIdx].resCount;
	GFL_G3D_RES* g3Dres;

	//リソース破棄
	for( i=0; i<resCount; i++ ){
		g3Dres = g3Dutil->g3DresTbl[resIdx+i];
		if( g3Dres ){
			if( RES_TEX_CHECK( g3Dres ) == TRUE ){
				//テクスチャリソースの場合ＶＲＡＭ解放（内部で転送したかのチェックは行っている）
				GFL_G3D_FreeVramTexture( g3Dres ); 
			}
			GFL_G3D_DeleteResource( g3Dres ); 
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトセットアップと削除
 */
//--------------------------------------------------------------------------------------------
static void addObject
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx, const GFL_G3D_UTIL_SETUP* setup, HEAPID heapID )
{
	GFL_G3D_RES				*g3DresMdl, *g3DresTex, *g3DresAnm;
	GFL_G3D_RND*			g3Drnd;
	GFL_G3D_ANM*			g3Danm;
	GFL_G3D_ANM**			g3DanmTbl;
	const GFL_G3D_UTIL_OBJ*	objTbl;
	int						i,j;
	u16 resIdx = g3Dutil->unit[unitIdx].resIdx;
	u16 resCount = g3Dutil->unit[unitIdx].resCount;
	u16 objIdx = g3Dutil->unit[unitIdx].objIdx;
	u16 objCount = g3Dutil->unit[unitIdx].objCount;

	//オブジェクト作成
	for( i=0; i<objCount; i++ ){
		objTbl = &setup->objTbl[i];

		//モデルリソースの参照指定(リソースを共有出来ないので追加読み込みチェックを行う)
		if( getResourceHandle(g3Dutil, setup, &g3DresMdl, resIdx, objTbl->mdlresID ) == TRUE ){
			g3Dutil->g3DobjExResourceRef[resIdx+i] |= EXRES_MDL;	//追加リソースフラグセット
		}
		//テクスチャリソースの参照指定
		if( objTbl->texresID != objTbl->mdlresID ){
			//モデル内包リソースの場合は共有チェックを行う
			if( RES_MDL_CHECK( g3Dutil->g3DresTbl[ resIdx+objTbl->texresID ] ) == TRUE ){
				if( getResourceHandle(g3Dutil,setup,&g3DresTex,resIdx,objTbl->texresID) == TRUE){
					g3Dutil->g3DobjExResourceRef[resIdx+i] |= EXRES_TEX;//追加リソースフラグセット
					GFL_G3D_TransVramTexture( g3DresTex );
					//transTexture( g3DresTex );
				}
			} else {
				g3DresTex = g3Dutil->g3DresTbl[ resIdx+objTbl->texresID ];
				GFL_G3D_TransVramTexture( g3DresTex );
				//transTexture( g3DresTex );
			}
		} else {
			//モデルリソースと同じ指定の場合はモデル依存で共有
			g3DresTex = g3DresMdl;
			//テクスチャVRAM未転送の場合は転送する
			transTexture( g3DresTex );
		}
		//レンダー作成
		g3Drnd = GFL_G3D_RENDER_Create( g3DresMdl, objTbl->mdldatID, g3DresTex ); 

		//アニメーションハンドルテンポラリ作成
		g3DanmTbl = GFL_HEAP_AllocClearMemoryLo( heapID, objTbl->anmCount * sizeof(GFL_G3D_ANM*) );

		//アニメーション設定
		for( j=0; j<objTbl->anmCount; j++ ){
			//アニメーションリソースの参照指定
			u16 anmresID = objTbl->anmTbl[j].anmresID;

			GF_ASSERT( (anmresID < resCount)||(anmresID == GFL_G3D_UTIL_NORESOURCE_ID) );
			if( anmresID == GFL_G3D_UTIL_NORESOURCE_ID ){
				g3Danm = NULL;	//ダミーポインタ
			} else {
				g3DresAnm = g3Dutil->g3DresTbl[ resIdx + anmresID ];
				//アニメーション作成
				g3Danm = GFL_G3D_ANIME_Create( g3Drnd, g3DresAnm, objTbl->anmTbl[j].anmdatID );
			}
			g3DanmTbl[j] = g3Danm;
		}
		//オブジェクト作成
		g3Dutil->g3DobjTbl[objIdx+i] = GFL_G3D_OBJECT_Create( g3Drnd, g3DanmTbl, objTbl->anmCount );

		GFL_HEAP_FreeMemory( g3DanmTbl );
	}
}

static void delObject
	( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	GFL_G3D_RES			*g3DresMdl,	*g3DresTex, *g3DresAnm;
	GFL_G3D_RND*		g3Drnd;
	GFL_G3D_ANM*		g3Danm;
	GFL_G3D_OBJ*		g3Dobj;
	u16					anmCount;
	int					i,j;
	u16 resIdx = g3Dutil->unit[unitIdx].resIdx;
	u16 resCount = g3Dutil->unit[unitIdx].resCount;
	u16 objIdx = g3Dutil->unit[unitIdx].objIdx;
	u16 objCount = g3Dutil->unit[unitIdx].objCount;

	//オブジェクト破棄
	for( i=0; i<objCount; i++ ){
		g3Dobj = g3Dutil->g3DobjTbl[objIdx+i];
		g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( g3Dobj );

		//アニメーション解放
		anmCount = GFL_G3D_OBJECT_GetAnimeCount( g3Dobj );
		for( j=0; j<anmCount; j++ ){
			GFL_G3D_ANIME_Delete( GFL_G3D_OBJECT_GetG3Danm( g3Dobj, j ) );
		}
		//追加テクスチャリソース作成フラグ確認
		if( g3Dutil->g3DobjExResourceRef[resIdx+i] & (EXRES_TEX^0xff) ){
			//テクスチャリソース解放
			g3DresTex = GFL_G3D_RENDER_GetG3DresTex( g3Drnd );
			GFL_G3D_FreeVramTexture( g3DresTex );
			GFL_G3D_DeleteResource( g3DresTex );
		}
		//追加モデルリソース作成フラグ確認
		if( g3Dutil->g3DobjExResourceRef[resIdx+i] & (EXRES_MDL^0xff) ){
			//モデルリソース解放
			g3DresMdl = GFL_G3D_RENDER_GetG3DresMdl( g3Drnd );
			GFL_G3D_DeleteResource( g3DresMdl );
		}	
		//レンダー解放
		GFL_G3D_RENDER_Delete( g3Drnd ); 

		//オブジェクト解放
		GFL_G3D_OBJECT_Delete( g3Dobj );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ユニットリソース先頭インデックス取得
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UTIL_GetUnitResIdx
		( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	return g3Dutil->unit[unitIdx].resIdx;
}

//--------------------------------------------------------------------------------------------
/**
 * ユニットリソース数取得
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UTIL_GetUnitResCount
		( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	return g3Dutil->unit[unitIdx].resCount;
}

//--------------------------------------------------------------------------------------------
/**
 * ユニットオブジェクト先頭インデックス取得
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UTIL_GetUnitObjIdx
		( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	return g3Dutil->unit[unitIdx].objIdx;
}

//--------------------------------------------------------------------------------------------
/**
 * ユニットオブジェクト数取得
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UTIL_GetUnitObjCount
		( GFL_G3D_UTIL* g3Dutil, u16 unitIdx )
{
	return g3Dutil->unit[unitIdx].objCount;
}

//--------------------------------------------------------------------------------------------
/**
 * リソースハンドル取得
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RES*
	GFL_G3D_UTIL_GetResHandle
		( GFL_G3D_UTIL* g3Dutil, u16 idx )
{
	return g3Dutil->g3DresTbl[idx];
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドル取得
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_UTIL_GetObjHandle
		( GFL_G3D_UTIL* g3Dutil, u16 idx )
{
	return g3Dutil->g3DobjTbl[idx];
}

//--------------------------------------------------------------------------------------------
/**
 * 総オブジェクト数取得
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UTIL_GetObjCount
		( GFL_G3D_UTIL* g3Dutil )
{
	int i;
	u16 count = 0;

	for( i=0; i<UNIT_COUNT_MAX; i++ ){
		if( g3Dutil->unit[i].resIdx != UNIT_NULL ){
			count += g3Dutil->unit[i].objCount;
		}
	}
	return count;
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトにアニメーションを追加設定する（別ユニットのアニメを設定するなど）
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_UTIL_SetObjAnotherUnitAnime
		( GFL_G3D_UTIL* g3Dutil, u16 objUnitIdx, u16 objIdx, 
			u16 anmUnitIdx, const GFL_G3D_UTIL_ANM* anmTbl, u16 anmCount )
{
	GFL_G3D_OBJ*	g3Dobj;
	GFL_G3D_RND*	g3Drnd;
	GFL_G3D_ANM*	g3Danm;
	u16				anmresID;
	int i;
	
	GF_ASSERT( objUnitIdx + objIdx <  g3Dutil->g3DobjCount );
	g3Dobj = g3Dutil->g3DobjTbl[ objUnitIdx + objIdx ];
	g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( g3Dobj ); 
 
	//アニメーション設定
	for( i=0; i<anmCount; i++ ){
		//アニメーションリソースの参照指定
		anmresID = anmTbl[i].anmresID;

		GF_ASSERT( (anmresID < g3Dutil->unit[anmUnitIdx].resCount)
					||(anmresID == GFL_G3D_UTIL_NORESOURCE_ID) );

		if( anmresID == GFL_G3D_UTIL_NORESOURCE_ID ){
			g3Danm = NULL;	//ダミーポインタ
		} else {
			//アニメーション作成
			g3Danm = GFL_G3D_ANIME_Create(	g3Drnd, 
											g3Dutil->g3DresTbl[ anmUnitIdx + anmresID ], 
											anmTbl[i].anmdatID );
		}
		GFL_G3D_OBJECT_AddAnime( g3Dobj, g3Danm );
	}

	return TRUE;
}

