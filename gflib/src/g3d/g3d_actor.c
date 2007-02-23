//=============================================================================================
/**
 * @file	g3d_actor.c                                                  
 * @brief	３Ｄアクタープログラム
 * @date	
 */
//=============================================================================================
#include "gflib.h"
#include "tcbl.h"

#include "g3d_actor.h"

//=============================================================================================
//	型宣言
//=============================================================================================
struct _GFL_G3D_ACTOR {
	GFL_G3D_ACTOR_DATA	actData;
	void*				actworkEx;
};

struct _GFL_G3D_ACTSYS {
	GFL_G3D_UTIL_SCENE*	g3Dscene;
	GFL_TCBLSYS*		g3DactTCBLsys;
	GFL_TCBL**			g3DactTCBLtbl;
	GFL_AREAMAN*		g3DactAreaman;
	u16					g3DactorMax;	
	u32					g3DactWorkSize;
	u16*				g3DactPriTbl;
	HEAPID				heapID;
};


static void actfunc( GFL_TCBL* tcbl, void* work );
static void objDrawSort( GFL_G3D_ACTSYS* g3DactSys );
#define TCBL_POINTER_SIZ	(4)
//=============================================================================================
/**
 *
 *
 * 管理システム
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * システム作成
 *
 * @param	g3Dscene		依存するg3DutilScene
 * @param	actMax			設定可能なアクター数
 * @param	wksiz			１アクターに割り当てるワークのサイズ
 * @param	heapID			ヒープＩＤ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_ACTSYS*
	GFL_G3D_ActorSysCreate
		( GFL_G3D_UTIL_SCENE* g3Dscene, const u16 actMax, const u32 wksiz, const HEAPID heapID )
{
	GFL_G3D_ACTSYS*	g3DactSys;
	GFL_TCBL*		g3DactTCBL;
	u32	TCBLworkSize = sizeof(GFL_G3D_ACTOR) + wksiz;

	GF_ASSERT( g3Dscene );
	//管理領域確保
	g3DactSys = GFL_HEAP_AllocMemoryClear( heapID, sizeof(GFL_G3D_ACTSYS) );
	//TCBL起動
	g3DactSys->g3DactTCBLsys = GFL_TCBL_SysInit(heapID, heapID, actMax, TCBLworkSize); 
	//管理配列作成
	g3DactSys->g3DactTCBLtbl = GFL_HEAP_AllocMemoryClear( heapID, TCBL_POINTER_SIZ*actMax );
	//配列領域マネージャ作成
	g3DactSys->g3DactAreaman = GFL_AREAMAN_Create( actMax, heapID );
	//プライオリティー管理配列作成
	g3DactSys->g3DactPriTbl = GFL_HEAP_AllocMemoryClear( heapID, 2*actMax );

	g3DactSys->heapID = heapID;
	g3DactSys->g3Dscene = g3Dscene;
	g3DactSys->g3DactorMax = actMax;
	g3DactSys->g3DactWorkSize = TCBLworkSize;

	return g3DactSys;
}

//--------------------------------------------------------------------------------------------
/**
 * システムメイン（動作）
 *
 * @param	g3DactSys		システムポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorSysMain
		( GFL_G3D_ACTSYS* g3DactSys )  
{
	GFL_TCBL_SysMain( g3DactSys->g3DactTCBLsys );
}

//--------------------------------------------------------------------------------------------
/**
 * システムメイン（描画）
 *
 * @param	g3DactSys		システムポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorSysDraw
		( GFL_G3D_ACTSYS* g3DactSys )  
{
	GFL_G3D_ACTOR*		g3Dact;
	GFL_G3D_OBJ*		g3Dobj;
	int	i = 0;

	//描画開始
	GFL_G3D_DrawStart();
	//カメラグローバルステート設定		
 	GFL_G3D_DrawLookAt();
	//描画プライオリティーによるソート
	objDrawSort( g3DactSys );

	//各アクターの描画
	while( ( i<g3DactSys->g3DactorMax )&&( g3DactSys->g3DactPriTbl[i] != 0xffff) ){
		g3Dact = GFL_TCBL_GetWork( g3DactSys->g3DactTCBLtbl[ g3DactSys->g3DactPriTbl[i] ] );
		g3Dobj = GFL_G3D_UtilsysObjHandleGet( g3DactSys->g3Dscene, g3Dact->actData.objID );

		GFL_G3D_ObjDraw( g3Dobj, &g3Dact->actData.status );
		i++;
	}
	//描画終了（バッファスワップ）
	GFL_G3D_DrawEnd();							
}

//--------------------------------------------------------------------------------------------
/**
 * システム破棄
 *
 * @param	g3DactSys		システムポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorSysDelete
		( GFL_G3D_ACTSYS* g3DactSys )  
{
	GFL_HEAP_FreeMemory( g3DactSys->g3DactPriTbl );
	GFL_AREAMAN_Delete( g3DactSys->g3DactAreaman );
	GFL_HEAP_FreeMemory( g3DactSys->g3DactTCBLtbl );
	GFL_TCBL_SysExit( g3DactSys->g3DactTCBLsys );
	GFL_HEAP_FreeMemory( g3DactSys );
}

//--------------------------------------------------------------------------------------------
/**
 * アクターポインタをＩＮＤＥＸより取得
 *
 * @param	g3DactSys		システムポインタ
 * @param	idx				アクター配置ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_ACTOR*
	GFL_G3D_G3DActorGet
		( GFL_G3D_ACTSYS* g3DactSys, u32 idx )
{
	GF_ASSERT( idx < g3DactSys->g3DactorMax );
	return ( GFL_G3D_ACTOR* )GFL_TCBL_GetWork( g3DactSys->g3DactTCBLtbl[ idx ] );
}

//--------------------------------------------------------------------------------------------
/**
 * 動作関数（ローカル）
 * 
 * この関数を経由して各アクター動作関数を呼び出し、戻ってくる
 */
//--------------------------------------------------------------------------------------------
static void actfunc( GFL_TCBL* tcbl, void* work )
{
	GFL_G3D_ACTOR* g3Dact = (GFL_G3D_ACTOR*)work;

	if( g3Dact->actData.func != NULL ){
		g3Dact->actData.func( g3Dact, (void*)((u32)work+sizeof(GFL_G3D_ACTOR)));
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ソート関数（ローカル）
 */
//--------------------------------------------------------------------------------------------
static void objDrawSort( GFL_G3D_ACTSYS* g3DactSys )
{
	int		i;
	u16		idx1,idx2;
	u8		pri1,pri2;
	BOOL	sw;
	int		count = 0;

	for( i=0; i<g3DactSys->g3DactorMax; i++ ){
		if( g3DactSys->g3DactTCBLtbl[i] != NULL ){
			GFL_G3D_ActorDrawSWGet( GFL_TCBL_GetWork(g3DactSys->g3DactTCBLtbl[i]), &sw );
			if( sw == TRUE ){
				g3DactSys->g3DactPriTbl[ count ] = i;
				count++;
			}
		}
	}
	for( i=count; i<g3DactSys->g3DactorMax; i++ ){
		g3DactSys->g3DactPriTbl[i] = 0xffff;
	}
	while( count ){
		for( i=0; i<count-1; i++ ){
			idx1 = g3DactSys->g3DactPriTbl[i];
			idx2 = g3DactSys->g3DactPriTbl[i+1];

			GFL_G3D_ActorDrawPriGet( GFL_TCBL_GetWork(g3DactSys->g3DactTCBLtbl[idx1]), &pri1 );
			GFL_G3D_ActorDrawPriGet( GFL_TCBL_GetWork(g3DactSys->g3DactTCBLtbl[idx2]), &pri2 );

			if( pri1 > pri2 ){
				g3DactSys->g3DactPriTbl[i] = idx2;
				g3DactSys->g3DactPriTbl[i+1] = idx1;
			}
		}
		count--;
	}
}





//=============================================================================================
/**
 *
 *
 * アクター
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * 作成
 *
 * @param	g3DactSys		システムポインタ
 * @param	actTbl			アクター設定データ
 * @param	actCount		アクター数
 *
 * @return	idx				アクター配置先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
u32
	GFL_G3D_ActorAdd
		( GFL_G3D_ACTSYS* g3DactSys, const GFL_G3D_ACTOR_DATA* actTbl, const u16 actCount )
{
	GFL_TCBL*		g3DactTCBL;
	GFL_G3D_ACTOR*	g3Dact;
	u32 pos;
	int	i;

	GF_ASSERT( g3DactSys );
	//作成INDEX確保
	pos = GFL_AREAMAN_ReserveAuto( g3DactSys->g3DactAreaman, actCount );
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannot reserve resource block" );

	for( i=0; i<actCount; i++ ){
		g3DactTCBL = GFL_TCBL_Create( g3DactSys->g3DactTCBLsys, actfunc,
										g3DactSys->g3DactWorkSize,
										actTbl[i].movePriority );	
		GF_ASSERT( g3DactTCBL );
		g3DactSys->g3DactTCBLtbl[ pos+i ] = g3DactTCBL;

		g3Dact = GFL_TCBL_GetWork( g3DactTCBL );
		g3Dact->actData		= actTbl[i];
		g3Dact->actworkEx	= NULL;
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 *
 * @param	g3DactSys		システムポインタ
 * @param	idx				アクター配置先頭ＩＮＤＥＸ
 * @param	resCount		アクター数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorDel
		( GFL_G3D_ACTSYS* g3DactSys, u32 idx, const u16 actCount )
{
	GFL_TCBL* g3DactTCBL;
	int	i;

	GF_ASSERT( g3DactSys );
	for( i=0; i<actCount; i++ ){
		g3DactTCBL = g3DactSys->g3DactTCBLtbl[ idx+i ];
		if( g3DactTCBL ){
			GFL_TCBL_Delete( g3DactTCBL );
			//ＩＮＤＥＸテーブルから削除
			g3DactSys->g3DactTCBLtbl[ idx+i ] = NULL;
		}
	}
	GFL_AREAMAN_Release( g3DactSys->g3DactAreaman, idx, actCount );
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトＩＤの取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	objID			オブジェクトＩＤの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorObjIDGet
		( GFL_G3D_ACTOR* g3Dact, u16* objID )
{
	*objID = g3Dact->actData.objID;
}

void
	GFL_G3D_ActorObjIDSet
		( GFL_G3D_ACTOR* g3Dact, u16* objID )
{
	g3Dact->actData.objID = *objID;
}

//--------------------------------------------------------------------------------------------
/**
 * 描画プライオリティーの取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	drawPri			描画プライオリティーの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorDrawPriGet
		( GFL_G3D_ACTOR* g3Dact, u8* drawPri )
{
	*drawPri = g3Dact->actData.drawPriority;
}

void
	GFL_G3D_ActorDrawPriSet
		( GFL_G3D_ACTOR* g3Dact, u8* drawPri )
{
	g3Dact->actData.drawPriority = *drawPri;
}

//--------------------------------------------------------------------------------------------
/**
 * 描画スイッチの取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	drawSW			描画スイッチの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorDrawSWGet
		( GFL_G3D_ACTOR* g3Dact, BOOL* drawSW )
{
	*drawSW = g3Dact->actData.drawSW;
}

void
	GFL_G3D_ActorDrawSWSet
		( GFL_G3D_ACTOR* g3Dact, BOOL* drawSW )
{
	g3Dact->actData.drawSW = *drawSW;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（位置情報）の取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	trans			位置情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorStatusTransGet
		( GFL_G3D_ACTOR* g3Dact, VecFx32* trans )
{
	*trans = g3Dact->actData.status.trans;
}

void
	GFL_G3D_ActorStatusTransSet
		( GFL_G3D_ACTOR* g3Dact, VecFx32* trans )
{
	g3Dact->actData.status.trans = *trans;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（スケール情報）の取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorStatusScaleGet
		( GFL_G3D_ACTOR* g3Dact, VecFx32* scale )
{
	*scale = g3Dact->actData.status.scale;
}

void
	GFL_G3D_ActorStatusScaleSet
		( GFL_G3D_ACTOR* g3Dact, VecFx32* scale )
{
	g3Dact->actData.status.scale = *scale;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（回転情報）の取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	rotate			回転情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorStatusRotateGet
		( GFL_G3D_ACTOR* g3Dact, MtxFx33* rotate )
{
	*rotate = g3Dact->actData.status.rotate;
}

void
	GFL_G3D_ActorStatusRotateSet
		( GFL_G3D_ACTOR* g3Dact, MtxFx33* rotate )
{
	g3Dact->actData.status.rotate = *rotate;
}

//--------------------------------------------------------------------------------------------
/**
 * 動作関数の取得と変更
 *
 * @param	g3Dact			アクターポインタ
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ActorFuncGet
		( GFL_G3D_ACTOR* g3Dact, GFL_G3D_ACTFUNC** func )
{
	*func = g3Dact->actData.func;
}

void
	GFL_G3D_ActorFuncSet
		( GFL_G3D_ACTOR* g3Dact, GFL_G3D_ACTFUNC** func )
{
	g3Dact->actData.func = *func;
}

	








#if 0
//=============================================================================================
/**
 *
 *
 * ３Ｄリソース管理
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * リソースを配列に追加
 *
 * @param	resTable		リソース配列ポインタ
 * @param	resCount		リソース数
 *
 * @return	idx				リソース配置先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UtilResLoad
		( const GFL_G3D_UTIL_RES* resTable, u16 resCount )  
{
	int	i;
	u32 pos;

	GF_ASSERT( g3Dutil );

	pos = GFL_AREAMAN_ReserveAuto( g3Dutil->g3DresAreaman, resCount );
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannot reserve resource block" );

	//配列にセットアップ
	for( i=0; i<resCount; i++ ){
		if( resTable[i].arcType == GFL_G3D_UTIL_RESARC ){
			//アーカイブＩＤより
			g3Dutil->g3DresTbl[ pos+i ] = GFL_G3D_ResourceCreateArc
										( (int)resTable[i].arcive, resTable[i].datID ); 
		} else {
			//アーカイブパスより
			g3Dutil->g3DresTbl[ pos+i ] = GFL_G3D_ResourceCreatePath
										( (const char*)resTable[i].arcive, resTable[i].datID ); 
		}
	}

	//テクスチャの一括転送（試しに効率がいいかもしれない方をやってみる）
	{
		int	i;
		BOOL resTypeFlg;

		//テクスチャデータ連続転送
		GX_BeginLoadTex();

		for( i=0; i<resCount; i++ ){
			//テクスチャリソースかどうか確認
			resTypeFlg = GFL_G3D_ResourceTypeCheck
							( g3Dutil->g3DresTbl[ pos+i ], GFL_G3D_RES_CHKTYPE_TEX );

			if(( resTable[i].trans == TRUE )&&( resTypeFlg == TRUE )){
				GFL_G3D_VramLoadTexDataOnly( g3Dutil->g3DresTbl[ pos+i ] );
			}
		}
		GX_EndLoadTex();
	
		//テクスチャパレット連続転送
		GX_BeginLoadTexPltt();
	
		for( i=0; i<resCount; i++ ){
			//テクスチャリソースかどうか確認
			resTypeFlg = GFL_G3D_ResourceTypeCheck
							( g3Dutil->g3DresTbl[ pos+i ], GFL_G3D_RES_CHKTYPE_TEX );
	
			if(( resTable[i].trans == TRUE )&&( resTypeFlg == TRUE )){
				GFL_G3D_VramLoadTexPlttOnly( g3Dutil->g3DresTbl[ pos+i ] );
			}
		}
		GX_EndLoadTexPltt();
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * リソースを配列から削除
 *
 * @param	idx				リソース配置先頭ＩＮＤＥＸ
 * @param	resCount		リソース数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilResUnload
		( u16 idx, u16 resCount ) 
{
	int	i;

	for( i=0; i<resCount; i++ ){
		if( g3Dutil->g3DresTbl[ i+idx ] ){
			GFL_G3D_RES* g3Dres = g3Dutil->g3DresTbl[ i+idx ];
			//テクスチャリソースかどうか確認
			if( GFL_G3D_ResourceTypeCheck( g3Dres, GFL_G3D_RES_CHKTYPE_TEX ) == TRUE ){
#if 0
				//ＶＲＡＭ転送済みかどうか確認
				if( GFL_G3D_ObjTexkeyLiveCheck( g3Dres ) == TRUE ){
					//ＶＲＡＭ解放
					GFL_G3D_VramUnloadTex( g3Dres );
				}
#else
				//ＶＲＡＭ解放
				GFL_G3D_VramUnloadTex( g3Dres ); 
#endif
			}
			//リソース破棄
			GFL_G3D_ResourceDelete( g3Dres ); 
			//ＩＮＤＥＸテーブルから削除
			g3Dutil->g3DresTbl[ i+idx ] = NULL;
		}
	}
	GFL_AREAMAN_Release( g3Dutil->g3DresAreaman, idx, resCount );
}

//--------------------------------------------------------------------------------------------
/**
 * リソースハンドルを配列から取得
 *
 * @param	idx				リソース格納ＩＮＤＥＸ
 *
 * @return	GFL_G3D_RES*	リソースポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RES*
	GFL_G3D_UtilResGet
		( u16 idx ) 
{
	if( idx >= g3Dutil->g3DresCount ){
		OS_Panic( "ID over" );
	}
	return g3Dutil->g3DresTbl[idx];
}
	
//--------------------------------------------------------------------------------------------
/**
 * テクスチャリソースを配列から転送(→ＶＲＡＭ)
 *
 * @param	idx		リソース格納ＩＮＤＥＸ
 *
 * @return	BOOL	結果(成功=TRUE)
 */
//--------------------------------------------------------------------------------------------
BOOL 
	GFL_G3D_UtilVramLoadTex
		( u16 idx ) 
{
	if( idx >= g3Dutil->g3DresCount ){
		OS_Panic( "ID over" );
	}
	return GFL_G3D_VramLoadTex( g3Dutil->g3DresTbl[idx] );
}





//=============================================================================================
/**
 *
 *
 * ３Ｄオブジェクト管理
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * オブジェクトを配列に追加
 *
 * @param	objTable		オブジェクト配列ポインタ
 * @param	objCount		オブジェクト数
 *
 * @return	idx				オブジェクト配置先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UtilObjLoad
		( const GFL_G3D_UTIL_OBJ* objTable, u16 objCount )  
{
	int	i;
	u32 pos;
	GFL_G3D_RES	*g3DresMdl,*g3DresTex;

	GF_ASSERT( g3Dutil );

	pos = GFL_AREAMAN_ReserveAuto( g3Dutil->g3DobjAreaman, objCount );
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannot reserve object block" );

	//配列にセットアップ
	for( i=0; i<objCount; i++ ){
		g3DresMdl = GFL_G3D_UtilResGet( objTable[i].mdlresID );
		GF_ASSERT( g3DresMdl );

		if( objTable[i].texresID != GFL_G3D_UTIL_RESNULL ){
			g3DresTex = GFL_G3D_UtilResGet( objTable[i].texresID );
		} else {
			g3DresTex = NULL;
		}
		OS_Printf("mdl = %x, mdlID = %x, tex = %x\n",g3DresMdl, objTable[i].mdldatID, g3DresTex );
		g3Dutil->g3DobjTbl[ pos+i ] = GFL_G3D_ObjCreate(	g3DresMdl, 
															objTable[i].mdldatID,
															g3DresTex );
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトを配列から削除
 *
 * @param	idx				オブジェクト配置先頭ＩＮＤＥＸ
 * @param	objCount		オブジェクト数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilObjUnload
		( u16 idx, u16 objCount ) 
{
	int	i;

	for( i=0; i<objCount; i++ ){
		if( g3Dutil->g3DobjTbl[ i+idx ] ){
			//オブジェクト破棄
			GFL_G3D_ObjDelete( g3Dutil->g3DobjTbl[ i+idx ] ); 
			//ＩＮＤＥＸテーブルから削除
			g3Dutil->g3DobjTbl[ i+idx ] = NULL;
		}
	}
	GFL_AREAMAN_Release( g3Dutil->g3DobjAreaman, idx, objCount );
}
	
//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドルを配列から取得
 *
 * @param	idx				オブジェクト格納ＩＮＤＥＸ
 *
 * @return	GFL_G3D_OBJ*	オブジェクトポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_UtilObjGet
		( u16 idx ) 
{
	if( idx >= g3Dutil->g3DobjCount ){
		OS_Panic( "ID over" );
	}
	return g3Dutil->g3DobjTbl[idx];
}





//=============================================================================================
/**
 *
 *
 * ３Ｄアニメーション管理
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * アニメーションを配列に追加
 *	参照オブジェクトが必要とされるので、オブジェクトの追加後に行うこと
 *
 * @param	anmTable		アニメーション配列ポインタ
 * @param	anmCount		アニメーション数
 *
 * @return	idx				アニメーション配置先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UtilAnmLoad
		( const GFL_G3D_UTIL_ANM* anmTable, u16 anmCount )  
{
	int	i;
	u32 pos;
	GFL_G3D_RES*	g3DresAnm;
	GFL_G3D_OBJ*	g3Dobj;

	GF_ASSERT( g3Dutil );

	pos = GFL_AREAMAN_ReserveAuto( g3Dutil->g3DanmAreaman, anmCount );
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannot reserve animetion block" );

	//配列にセットアップ
	for( i=0; i<anmCount; i++ ){
		g3DresAnm = GFL_G3D_UtilResGet( anmTable[i].anmresID );
		GF_ASSERT( g3DresAnm );
		g3Dobj = GFL_G3D_UtilObjGet( anmTable[i].forMdl );
		GF_ASSERT( g3Dobj );

		g3Dutil->g3DanmTbl[ pos+i ] = GFL_G3D_AnmCreate(	g3Dobj, 
															g3DresAnm, 
															anmTable[i].anmdatID,
															anmTable[i].bind );
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * アニメーションを配列から削除
 *
 * @param	idx				アニメーション配置先頭ＩＮＤＥＸ
 * @param	anmCount		アニメーション数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilAnmUnload
		( u16 idx, u16 anmCount ) 
{
	int	i;

	for( i=0; i<anmCount; i++ ){
		if( g3Dutil->g3DanmTbl[ i+idx ] ){
			//アニメーション破棄
			GFL_G3D_AnmDelete( g3Dutil->g3DanmTbl[ i+idx ] ); 
			//ＩＮＤＥＸテーブルから削除
			g3Dutil->g3DanmTbl[ i+idx ] = NULL;
		}
	}
	GFL_AREAMAN_Release( g3Dutil->g3DanmAreaman, idx, anmCount );
}
	
//--------------------------------------------------------------------------------------------
/**
 * アニメーションハンドルを配列から取得
 *
 * @param	idx				アニメーション格納ＩＮＤＥＸ
 *
 * @return	GFL_G3D_ANM*	アニメーションポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_ANM*
	GFL_G3D_UtilAnmGet
		( u16 idx ) 
{
	if( idx >= g3Dutil->g3DanmCount ){
		OS_Panic( "ID over" );
	}
	return g3Dutil->g3DanmTbl[idx];
}





#if 0
//=============================================================================================
/**
 *
 *
 * ３Ｄアクター管理
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * アクターを配列に追加
 *
 * @param	actTable		アクター配列ポインタ
 * @param	actCount		アクター数
 *
 * @return	idx				アクター配置先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_UtilActLoad
		( const GFL_G3D_UTIL_ACT* actTable, u16 actCount )  
{
	int	i;
	u32 pos;
	GFL_G3D_RES	*g3DresMdl,*g3DresTex;

	GF_ASSERT( g3Dutil );

	pos = GFL_AREAMAN_ReserveAuto( g3Dutil->g3DactAreaman, actCount );
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannot reserve  actor block" );

	//配列にセットアップ
	for( i=0; i<actCount; i++ ){
		g3DresMdl = GFL_G3D_UtilResGet( actTable[i].mdlresID );
		GF_ASSERT( g3DresMdl );

		if( objTable[i].texresID != GFL_G3D_UTIL_RESNULL ){
			g3DresTex = GFL_G3D_UtilResGet( objTable[i].texresID );
		} else {
			g3DresTex = NULL;
		}
		OS_Printf("mdl = %x, mdlID = %x, tex = %x\n",g3DresMdl, objTable[i].mdldatID, g3DresTex );
		g3Dutil->g3DobjTbl[ pos+i ] = GFL_G3D_ObjCreate( g3DresMdl, objTable[i].mdldatID,
															g3DresTex );

		GFL_G3D_ObjContSetTrans( (g3Dutil->g3DobjTbl)[ pos+i ], &objTable[i].trans );
		GFL_G3D_ObjContSetScale( (g3Dutil->g3DobjTbl)[ pos+i ], &objTable[i].scale );
		GFL_G3D_ObjContSetRotate( (g3Dutil->g3DobjTbl)[ pos+i ], &objTable[i].rotate );

		g3Dutil->g3DobjStat[ pos+i ].priority = objTable[i].priority;
		g3Dutil->g3DobjStat[ pos+i ].drawSW = objTable[i].drawSW;
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトを配列から削除
 *
 * @param	idx				オブジェクト配置先頭ＩＮＤＥＸ
 * @param	objCount		オブジェクト数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilObjUnload
		( u16 idx, u16 objCount ) 
{
	int	i;

	for( i=0; i<objCount; i++ ){
		if( g3Dutil->g3DobjTbl[ i+idx ] ){
			//オブジェクト破棄
			GFL_G3D_ObjDelete( g3Dutil->g3DobjTbl[ i+idx ] ); 
			//ＩＮＤＥＸテーブルから削除
			g3Dutil->g3DobjTbl[ i+idx ] = NULL;
		}
	}
	GFL_AREAMAN_Release( g3Dutil->g3DobjAreaman, idx, objCount );
}
	
//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドルを配列から取得
 *
 * @param	idx				オブジェクト格納ＩＮＤＥＸ
 *
 * @return	GFL_G3D_OBJ*	オブジェクトポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_UtilObjGet
		( u16 idx ) 
{
	if( idx >= g3Dutil->g3DobjCount ){
		OS_Panic( "ID over" );
	}
	return g3Dutil->g3DobjTbl[idx];
}
#endif





//=============================================================================================
/**
 *
 *
 * 描画管理
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトリストのソート
 */
//--------------------------------------------------------------------------------------------
#if 0
static void objDrawSort( void )
{
	int	i;
	u16	idx1,idx2;
	u8	pri1,pri2;
	u32 count = g3Dutil->g3DobjCount;

	for( i=0; i<count; i++ ){
		g3Dutil->g3DobjPriTbl[i] = i;
	}
	while( count ){
		for( i=0; i<count-1; i++ ){
			idx1 = g3Dutil->g3DobjPriTbl[i];
			idx2 = g3Dutil->g3DobjPriTbl[i+1];

			pri1 = g3Dutil->g3DobjStat[idx1].priority;
			pri2 = g3Dutil->g3DobjStat[idx2].priority;

			if( pri1 > pri2 ){
				g3Dutil->g3DobjPriTbl[i] = idx2;
				g3Dutil->g3DobjPriTbl[i+1] = idx1;
			}
		}
		count--;
	}
}
#endif

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの描画
 *
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilDraw
		( void )
{
#if 0
	//描画開始
	GFL_G3D_DrawStart();
	//カメラグローバルステート設定		
 	GFL_G3D_DrawLookAt();

	objDrawSort();
	{
		int	i;
		u16 idx;
		GFL_G3D_OBJSTAT*	drawStatus;
		GFL_G3D_OBJ*		drawObj;

		for( i=0; i<g3Dutil->g3DobjCount; i++ ){
			idx = g3Dutil->g3DobjPriTbl[i];
			drawStatus = &g3Dutil->g3DobjStat[ idx ];
			drawObj = g3Dutil->g3DobjTbl[ idx ];

			if(( drawObj )&&( drawStatus->drawSW == TRUE )){
				GFL_G3D_ObjDraw( drawObj );
			}
		}
	}
	//描画終了（バッファスワップ）
	GFL_G3D_DrawEnd();							
#endif
}





//=============================================================================================
/**
 *
 *
 * その他
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの回転行列の作成
 *
 * @param	rotSrc	計算前の回転ベクトルポインタ
 * @param	rotDst	計算後の回転行列格納ポインタ
 *
 * この関数等を使用し、オブジェクト毎に適切な回転行列を作成したものを、描画に流す。
 */
//--------------------------------------------------------------------------------------------
// Ｘ→Ｙ→Ｚの順番で計算
void
	GFL_G3D_UtilObjDrawRotateCalcXY
		( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotX33(	rotDst, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );

	MTX_RotY33(	&tmp, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)rotSrc->z), FX_CosIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

// Ｘ→Ｙ→Ｚの順番で計算（相対）
void
	GFL_G3D_UtilObjDrawRotateCalcXY_Rev
		( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotX33(	rotDst, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );

	MTX_RotY33(	&tmp, FX_SinIdx((u16)-rotSrc->x), FX_CosIdx((u16)-rotSrc->x) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_CosIdx((u16)rotSrc->z), FX_SinIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

// Ｙ→Ｘ→Ｚの順番で計算
void
	GFL_G3D_UtilObjDrawRotateCalcYX
		( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotY33(	rotDst, FX_SinIdx((u16)rotSrc->y), FX_CosIdx((u16)rotSrc->y) );

	MTX_RotX33(	&tmp, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)rotSrc->z), FX_CosIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst,&tmp, rotDst );
}

// Ｙ→Ｘ→Ｚの順番で計算（相対）
void
	GFL_G3D_UtilObjDrawRotateCalcYX_Rev
		( VecFx32* rotSrc, MtxFx33* rotDst )
{
	MtxFx33 tmp;

	MTX_RotY33(	rotDst, FX_SinIdx((u16)rotSrc->x), FX_CosIdx((u16)rotSrc->x) );

	MTX_RotX33(	&tmp, FX_SinIdx((u16)-rotSrc->y), FX_CosIdx((u16)-rotSrc->y) );
	MTX_Concat33( rotDst, &tmp, rotDst );

	MTX_RotZ33(	&tmp,FX_CosIdx((u16)rotSrc->z), FX_SinIdx((u16)rotSrc->z) );
	MTX_Concat33( rotDst, &tmp, rotDst );
}

#endif
