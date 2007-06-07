//=============================================================================================
/**
 * @file	g3d_scene.c                                                  
 * @brief	３Ｄシーン管理プログラム
 * @date	
 */
//=============================================================================================
#include "gflib.h"
#include "tcbl.h"

#include "g3d_scene.h"

//=============================================================================================
//	型宣言
//=============================================================================================
struct _GFL_G3D_SCENEOBJ {
	GFL_G3D_SCENEOBJ_DATA		sceneObjData;
	GFL_G3D_SCENE*				g3Dscene;
	void*						sceneObjWorkEx;
	GFL_G3D_SCENEACCESORY_DATA*	accesory;
	u16							accesoryCount;
};

struct _GFL_G3D_SCENE {
	GFL_G3D_UTIL*			g3Dutil;
	GFL_TCBLSYS*			g3DsceneObjTCBLsys;
	GFL_TCBL**				g3DsceneObjTCBLtbl;
	GFL_AREAMAN*			g3DsceneObjAreaman;
	u16						g3DsceneObjMax;	
	u32						g3DsceneObjWorkSize;
	u16*					g3DsceneObjPriTbl;
	BOOL					GFL_PTC_Enable;
	BOOL					GFL_PTC_DrawSW;
	HEAPID					heapID;
};


static void sceneObjfunc( GFL_TCBL* tcbl, void* work );
static void objDrawSort( GFL_G3D_SCENE* g3Dscene );

//static void getTranslucent( NNSG3dRS* rs );

#define TCBL_POINTER_SIZ		(4)
//=============================================================================================
/**
 *
 *
 * 管理システム
 *
 *	g3Dutilリソース配列を利用し、配列内をＩＮＤＥＸ参照する形で処理を行う。
 *	なので描画元リソースはすべてそちらでセットアップされている必要がある。
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * システム作成
 *
 * @param	g3Dutil			依存するg3Dutil
 * @param	sceneObjMax		配置可能なオブジェクト数
 * @param	sceneObjWkSiz	１オブジェクトに割り当てるワークのサイズ
 * @param	GFL_PTC_Enable	パーティクルシステムの起動フラグ
 * @param	heapID			ヒープＩＤ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_SCENE*
	GFL_G3D_SCENE_Create
		( GFL_G3D_UTIL* g3Dutil, const u16 sceneObjMax, const u32 sceneObjWkSiz, 
			const u16 sceneAccesoryMax, BOOL GFL_PTC_Enable, const HEAPID heapID )
{
	GFL_G3D_SCENE*	g3Dscene;
	GFL_TCBL*		g3DsceneObjTCBL;
	u32	TCBLworkSize = sizeof(GFL_G3D_SCENEOBJ) + sceneObjWkSiz;

	GF_ASSERT( g3Dutil );
	//シーン管理領域確保
	g3Dscene = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_SCENE) );

	//配置オブジェクトTCBL起動（動作プライオリティーなどをTCBL管理する）
	g3Dscene->g3DsceneObjTCBLsys = GFL_TCBL_Init(heapID,heapID,sceneObjMax,TCBLworkSize); 
	//配置オブジェクト管理配列作成
	g3Dscene->g3DsceneObjTCBLtbl = GFL_HEAP_AllocClearMemory(heapID,TCBL_POINTER_SIZ*sceneObjMax);
	//配置オブジェクト配列領域マネージャ作成
	g3Dscene->g3DsceneObjAreaman = GFL_AREAMAN_Create( sceneObjMax, heapID );
	//配置オブジェクトプライオリティー管理配列作成
	g3Dscene->g3DsceneObjPriTbl = GFL_HEAP_AllocClearMemory( heapID, 2*sceneObjMax );

	g3Dscene->heapID = heapID;
	g3Dscene->g3Dutil = g3Dutil;
	g3Dscene->g3DsceneObjMax = sceneObjMax;
	g3Dscene->g3DsceneObjWorkSize = TCBLworkSize;
	g3Dscene->GFL_PTC_Enable = GFL_PTC_Enable;
	
	if( g3Dscene->GFL_PTC_Enable == TRUE ){
		GFL_PTC_Init(heapID);
		g3Dscene->GFL_PTC_DrawSW = TRUE;
	}
	return g3Dscene;
}

//--------------------------------------------------------------------------------------------
/**
 * システムメイン（動作）
 *
 * @param	g3Dscene		システムポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENE_Main
		( GFL_G3D_SCENE* g3Dscene )  
{
	GFL_TCBL_Main( g3Dscene->g3DsceneObjTCBLsys );
}

//--------------------------------------------------------------------------------------------
/**
 * システムメイン（描画）
 *
 * @param	g3Dscene		システムポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENE_Draw
		( GFL_G3D_SCENE* g3Dscene )  
{
	GFL_G3D_SCENEOBJ*	g3DsceneObj;
	GFL_G3D_OBJ			*g3Dobj, *g3Dobj_Accesory;
	GFL_G3D_RND*		g3Drnd;
	NNSG3dResMdl*		pMdl;
	int	i = 0;
	int	j;
	BOOL cullResult; 

	//描画開始
	GFL_G3D_DRAW_Start();

	if(( g3Dscene->GFL_PTC_Enable == TRUE )&&( g3Dscene->GFL_PTC_DrawSW == TRUE )){
		GFL_PTC_Main();
	}
	//カメラグローバルステート設定		
 	GFL_G3D_DRAW_SetLookAt();
	//描画プライオリティーによるソート
	objDrawSort( g3Dscene );

	//各アクターの描画
	while( ( i<g3Dscene->g3DsceneObjMax )&&( g3Dscene->g3DsceneObjPriTbl[i] != 0xffff) ){
		//主要構造体ポインタ取得
		g3DsceneObj = GFL_TCBL_GetWork
						( g3Dscene->g3DsceneObjTCBLtbl[ g3Dscene->g3DsceneObjPriTbl[i] ] );
		g3Dobj = GFL_G3D_UTIL_GetObjHandle( g3Dscene->g3Dutil, g3DsceneObj->sceneObjData.objID );

		//半透明設定
		g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( g3Dobj );
		pMdl = NNS_G3dRenderObjGetResMdl( GFL_G3D_RENDER_GetRenderObj( g3Drnd ) );
		if( g3DsceneObj->sceneObjData.blendAlpha == GFL_G3D_SCENEOBJ_ALPHA_OFF ){
			NNS_G3dMdlUseMdlPolygonID( pMdl );
			NNS_G3dMdlUseMdlAlpha( pMdl );
		} else {
			NNS_G3dGlbPolygonAttr(	GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE,
									GX_CULL_NONE, i, g3DsceneObj->sceneObjData.blendAlpha, 0 );
			NNS_G3dMdlUseGlbPolygonID( pMdl );	//一時的にＩＤを変更（半透明のため）
			NNS_G3dMdlUseGlbAlpha( pMdl );		//反映しているのはα設定だけ
		}

		if( g3DsceneObj->sceneObjData.cullingFlag == TRUE ){
			//本体の描画
			cullResult = GFL_G3D_DRAW_DrawObjectCullingON
							( g3Dobj, &g3DsceneObj->sceneObjData.status );
		} else {
			//本体の描画
			GFL_G3D_DRAW_DrawObject( g3Dobj, &g3DsceneObj->sceneObjData.status );
			cullResult = TRUE;
		}
		//アクセサリーの描画（一部パラメーターは本体のものを引継ぎ）
		if( cullResult == TRUE ){
			for( j=0; j<g3DsceneObj->accesoryCount; j++ ){
				g3Dobj_Accesory = GFL_G3D_UTIL_GetObjHandle
								( g3Dscene->g3Dutil, g3DsceneObj->accesory[j].objID );
				GFL_G3D_DRAW_DrawAccesory( g3Dobj, g3Dobj_Accesory,
						&g3DsceneObj->accesory[j].status, g3DsceneObj->accesory[j].jntID );
			}
		}
		i++;
	}
	//描画終了（バッファスワップ）
	GFL_G3D_DRAW_End();							
}

//--------------------------------------------------------------------------------------------
/**
 * システム破棄
 *
 * @param	g3Dscene		システムポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENE_Delete
		( GFL_G3D_SCENE* g3Dscene )  
{
	GF_ASSERT( g3Dscene );

	if( g3Dscene->GFL_PTC_Enable == TRUE ){
		GFL_PTC_Exit();
	}
	GFL_HEAP_FreeMemory( g3Dscene->g3DsceneObjPriTbl );
	GFL_AREAMAN_Delete( g3Dscene->g3DsceneObjAreaman );
	GFL_HEAP_FreeMemory( g3Dscene->g3DsceneObjTCBLtbl );
	GFL_TCBL_Exit( g3Dscene->g3DsceneObjTCBLsys );
	GFL_HEAP_FreeMemory( g3Dscene );
}

//--------------------------------------------------------------------------------------------
/**
 * アクターポインタをＩＮＤＥＸより取得
 *
 * @param	g3Dscene		システムポインタ
 * @param	idx				アクター配置ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_SCENEOBJ*
	GFL_G3D_SCENEOBJ_Get
		( GFL_G3D_SCENE* g3Dscene, u32 idx )
{
	GF_ASSERT( g3Dscene );
	GF_ASSERT( idx < g3Dscene->g3DsceneObjMax );
	return ( GFL_G3D_SCENEOBJ* )GFL_TCBL_GetWork( g3Dscene->g3DsceneObjTCBLtbl[ idx ] );
}

//--------------------------------------------------------------------------------------------
/**
 * パーティクル描画のＯＮ/ＯＦＦ(パーティクルシステムが起動されている場合のみ有効)
 *
 * @param	BOOL			パーティクル描画スイッチ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENE_SetDrawParticleSW
		( GFL_G3D_SCENE* g3Dscene, BOOL sw )
{
	GF_ASSERT( g3Dscene );
	g3Dscene->GFL_PTC_DrawSW = sw;
}

//--------------------------------------------------------------------------------------------
/**
 * 動作関数（ローカル）
 * 
 * この関数を経由して各配置オブジェクト動作関数を呼び出し、戻ってくる
 */
//--------------------------------------------------------------------------------------------
static void sceneObjfunc( GFL_TCBL* tcbl, void* work )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = (GFL_G3D_SCENEOBJ*)work;

	if( g3DsceneObj->sceneObjData.func != NULL ){
		g3DsceneObj->sceneObjData.func( g3DsceneObj, (void*)((u32)work+sizeof(GFL_G3D_SCENEOBJ)));
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ソート関数【昇順】（ローカル）
 */
//--------------------------------------------------------------------------------------------
static void objDrawSort( GFL_G3D_SCENE* g3Dscene )
{
	int		i;
	u16		idx1,idx2;
	u8		pri1,pri2;
	BOOL	sw;
	int		count = 0;

	for( i=0; i<g3Dscene->g3DsceneObjMax; i++ ){
		if( g3Dscene->g3DsceneObjTCBLtbl[i] != NULL ){
			GFL_G3D_SCENEOBJ_GetDrawSW( GFL_TCBL_GetWork(g3Dscene->g3DsceneObjTCBLtbl[i]), &sw );
			if( sw == TRUE ){
				g3Dscene->g3DsceneObjPriTbl[ count ] = i;
				count++;
			}
		}
	}
	for( i=count; i<g3Dscene->g3DsceneObjMax; i++ ){
		g3Dscene->g3DsceneObjPriTbl[i] = 0xffff;
	}
	while( count ){
		for( i=0; i<count-1; i++ ){
			idx1 = g3Dscene->g3DsceneObjPriTbl[i];
			idx2 = g3Dscene->g3DsceneObjPriTbl[i+1];

			GFL_G3D_SCENEOBJ_GetDrawPri(GFL_TCBL_GetWork(g3Dscene->g3DsceneObjTCBLtbl[idx1]),&pri1);
			GFL_G3D_SCENEOBJ_GetDrawPri(GFL_TCBL_GetWork(g3Dscene->g3DsceneObjTCBLtbl[idx2]),&pri2);

			if( pri1 > pri2 ){
				g3Dscene->g3DsceneObjPriTbl[i] = idx2;
				g3Dscene->g3DsceneObjPriTbl[i+1] = idx1;
			}
		}
		count--;
	}
}





//=============================================================================================
/**
 *
 *
 * 配置オブジェクト
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * 作成
 *
 * @param	g3Dscene		システムポインタ
 * @param	sceneObjTbl		配置オブジェクト設定データ
 * @param	sceneObjCount	配置オブジェクト数
 *
 * @return	idx				配置オブジェクト先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
u32
	GFL_G3D_SCENEOBJ_Add
		( GFL_G3D_SCENE* g3Dscene, const GFL_G3D_SCENEOBJ_DATA* sceneObjTbl, 
			const u16 sceneObjCount )
{
	GFL_TCBL*			g3DsceneObjTCBL;
	GFL_G3D_SCENEOBJ*	g3DsceneObj;
	u32 pos;
	int	i;

	GF_ASSERT( g3Dscene );
	//作成INDEX確保
	pos = GFL_AREAMAN_ReserveAuto( g3Dscene->g3DsceneObjAreaman, sceneObjCount );
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannot reserve resource block" );

	for( i=0; i<sceneObjCount; i++ ){
		g3DsceneObjTCBL = GFL_TCBL_Create( g3Dscene->g3DsceneObjTCBLsys, sceneObjfunc,
										g3Dscene->g3DsceneObjWorkSize,
										sceneObjTbl[i].movePriority );	
		GF_ASSERT( g3DsceneObjTCBL );
		g3Dscene->g3DsceneObjTCBLtbl[ pos+i ] = g3DsceneObjTCBL;

		g3DsceneObj = GFL_TCBL_GetWork( g3DsceneObjTCBL );
		g3DsceneObj->g3Dscene = g3Dscene;
		g3DsceneObj->sceneObjData = sceneObjTbl[i];
		g3DsceneObj->sceneObjWorkEx	= NULL;
		g3DsceneObj->accesoryCount = 0;
		g3DsceneObj->accesory = NULL;
#if 0
		// 半透明処理コールバック設定
		{
			GFL_G3D_OBJ* g3Dobj = GFL_G3D_UTIL_GetObjHandle( g3Dscene->g3Dutil, 
																g3DsceneObj->sceneObjData.objID );
			GFL_G3D_RND* g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( g3Dobj );

			NNS_G3dRenderObjSetCallBack( GFL_G3D_RENDER_GetRenderObj( g3Drnd ),
						&getTranslucent, NULL, NNS_G3D_SBC_MAT, NNS_G3D_SBC_CALLBACK_TIMING_B);
		}
#endif
		GFL_STD_MemClear( (void*)((u32)g3DsceneObj + sizeof(GFL_G3D_SCENEOBJ)), 
							g3Dscene->g3DsceneObjWorkSize - sizeof(GFL_G3D_SCENEOBJ) );
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 *
 * @param	g3Dscene		システムポインタ
 * @param	idx				配置オブジェクト先頭ＩＮＤＥＸ
 * @param	sceneObjCount	配置オブジェクト数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_Remove
		( GFL_G3D_SCENE* g3Dscene, u32 idx, const u16 sceneObjCount )
{
	GFL_TCBL*			g3DsceneObjTCBL;
	GFL_G3D_SCENEOBJ*	g3DsceneObj;
	int	i;

	GF_ASSERT( g3Dscene );
	for( i=0; i<sceneObjCount; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( g3Dscene, idx+i );

		GFL_G3D_SCENEOBJ_ACCESORY_Remove( g3DsceneObj );	//アクセサリー破棄

		g3DsceneObjTCBL = g3Dscene->g3DsceneObjTCBLtbl[ idx+i ];
		if( g3DsceneObjTCBL ){
			GFL_TCBL_Delete( g3DsceneObjTCBL );
			//ＩＮＤＥＸテーブルから削除
			g3Dscene->g3DsceneObjTCBLtbl[ idx+i ] = NULL;
		}
	}
	GFL_AREAMAN_Release( g3Dscene->g3DsceneObjAreaman, idx, sceneObjCount );
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドル(G3D_OBJ)の取得
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_SCENEOBJ_GetG3DobjHandle
		( GFL_G3D_SCENEOBJ* g3DsceneObj )
{
	return GFL_G3D_UTIL_GetObjHandle(	g3DsceneObj->g3Dscene->g3Dutil, 
										g3DsceneObj->sceneObjData.objID );
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトＩＤの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	objID			オブジェクトＩＤの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetObjID
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16* objID )
{
	*objID = g3DsceneObj->sceneObjData.objID;
}

void
	GFL_G3D_SCENEOBJ_SetObjID
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16* objID )
{
	g3DsceneObj->sceneObjData.objID = *objID;
}

//--------------------------------------------------------------------------------------------
/**
 * 描画プライオリティーの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	drawPri			描画プライオリティーの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetDrawPri
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* drawPri )
{
	*drawPri = g3DsceneObj->sceneObjData.drawPriority;
}

void
	GFL_G3D_SCENEOBJ_SetDrawPri
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* drawPri )
{
	g3DsceneObj->sceneObjData.drawPriority = *drawPri;
}

//--------------------------------------------------------------------------------------------
/**
 * αブレンド値の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	blendAlpha		αブレンドの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetBlendAlpha
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* blendAlpha )
{
	*blendAlpha = g3DsceneObj->sceneObjData.blendAlpha;
}

void
	GFL_G3D_SCENEOBJ_SetBlendAlpha
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* blendAlpha )
{
	g3DsceneObj->sceneObjData.blendAlpha = *blendAlpha;
}

//--------------------------------------------------------------------------------------------
/**
 * 描画スイッチの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	drawSW			描画スイッチの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetDrawSW
		( GFL_G3D_SCENEOBJ* g3DsceneObj, BOOL* drawSW )
{
	*drawSW = g3DsceneObj->sceneObjData.drawSW;
}

void
	GFL_G3D_SCENEOBJ_SetDrawSW
		( GFL_G3D_SCENEOBJ* g3DsceneObj, BOOL* drawSW )
{
	g3DsceneObj->sceneObjData.drawSW = *drawSW;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（位置情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	trans			位置情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetPos
		( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* trans )
{
	*trans = g3DsceneObj->sceneObjData.status.trans;
}

void
	GFL_G3D_SCENEOBJ_SetPos
		( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* trans )
{
	g3DsceneObj->sceneObjData.status.trans = *trans;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（スケール情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetScale
		( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* scale )
{
	*scale = g3DsceneObj->sceneObjData.status.scale;
}

void
	GFL_G3D_SCENEOBJ_SetScale
		( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* scale )
{
	g3DsceneObj->sceneObjData.status.scale = *scale;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（回転情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	rotate			回転情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetRotate
		( GFL_G3D_SCENEOBJ* g3DsceneObj, MtxFx33* rotate )
{
	*rotate = g3DsceneObj->sceneObjData.status.rotate;
}

void
	GFL_G3D_SCENEOBJ_SetRotate
		( GFL_G3D_SCENEOBJ* g3DsceneObj, MtxFx33* rotate )
{
	g3DsceneObj->sceneObjData.status.rotate = *rotate;
}

//--------------------------------------------------------------------------------------------
/**
 * 動作関数の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_GetFunc
		( GFL_G3D_SCENEOBJ* g3DsceneObj, GFL_G3D_SCENEOBJFUNC** func )
{
	*func = g3DsceneObj->sceneObjData.func;
}

void
	GFL_G3D_SCENEOBJ_SetFunc
		( GFL_G3D_SCENEOBJ* g3DsceneObj, GFL_G3D_SCENEOBJFUNC** func )
{
	g3DsceneObj->sceneObjData.func = *func;
}

//--------------------------------------------------------------------------------------------
/**
 *
 * アクセサリー関連
 *　破棄に関しては配置オブジェクトを削除した際にも自動で消える。
 */
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * アクセサリー登録
 *
 * @param	g3DsceneObj			配置オブジェクトポインタ
 * @param	accesoryTbl			アクセサリー設定データテーブルポインタ
 * @param	accesoryCount		アクセサリー数
 *
 * @return	BOOL
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_SCENEOBJ_ACCESORY_Add
		( GFL_G3D_SCENEOBJ* g3DsceneObj, const GFL_G3D_SCENEACCESORY_DATA* accesoryTbl, 
			const u16 accesoryCount )
{
	int	i;

	if( g3DsceneObj->accesoryCount ) return FALSE;	//二重登録不可

	g3DsceneObj->accesory = GFL_HEAP_AllocClearMemory( g3DsceneObj->g3Dscene->heapID, 
								sizeof(GFL_G3D_SCENEACCESORY_DATA) * accesoryCount );
	g3DsceneObj->accesoryCount = accesoryCount;

	//アクセサリー設定配列をコピー
	for( i=0; i<accesoryCount; i++ ){
		g3DsceneObj->accesory[i] = accesoryTbl[i];
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 *
 * @param	g3DsceneObj			配置オブジェクトポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_ACCESORY_Remove
		( GFL_G3D_SCENEOBJ* g3DsceneObj )
{
	if( !g3DsceneObj->accesory ) return;

	GFL_HEAP_FreeMemory( g3DsceneObj->accesory );
	g3DsceneObj->accesory = NULL;
	g3DsceneObj->accesoryCount = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトＩＤの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	accesoryIdx		アクセサリーインデックス
 * @param	objID			オブジェクトＩＤの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_ACCESORY_GetObjID
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, u16* objID )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	*objID = g3DsceneObj->accesory[accesoryIdx].objID;
}

void
	GFL_G3D_SCENEOBJ_ACCESORY_SetObjID
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, u16* objID )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	g3DsceneObj->accesory[accesoryIdx].objID = *objID;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（位置情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	accesoryIdx		アクセサリーインデックス
 * @param	trans			位置情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_ACCESORY_GetPos
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, VecFx32* trans )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	*trans = g3DsceneObj->accesory[accesoryIdx].status.trans;
}

void
	GFL_G3D_SCENEOBJ_ACCESORY_SetPos
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, VecFx32* trans )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	g3DsceneObj->accesory[accesoryIdx].status.trans = *trans;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（スケール情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	accesoryIdx		アクセサリーインデックス
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_ACCESORY_GetScale
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, VecFx32* scale )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	*scale = g3DsceneObj->accesory[accesoryIdx].status.scale;
}

void
	GFL_G3D_SCENEOBJ_ACCESORY_SetScale
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, VecFx32* scale )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	g3DsceneObj->accesory[accesoryIdx].status.scale = *scale;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス（回転情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	accesoryIdx		アクセサリーインデックス
 * @param	rotate			回転情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SCENEOBJ_ACCESORY_GetRotate
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, MtxFx33* rotate )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	*rotate = g3DsceneObj->accesory[accesoryIdx].status.rotate;
}

void
	GFL_G3D_SCENEOBJ_ACCESORY_SetRotate
		( GFL_G3D_SCENEOBJ* g3DsceneObj, u16 accesoryIdx, MtxFx33* rotate )
{
	GF_ASSERT( g3DsceneObj->accesoryCount );
	GF_ASSERT( accesoryIdx < g3DsceneObj->accesoryCount );
	g3DsceneObj->accesory[accesoryIdx].status.rotate = *rotate;
}





//=============================================================================================
/**
 *
 *
 * コールバック
 *
 *
 */
//=============================================================================================
#if 0
static void getTranslucent( NNSG3dRS* rs )
{
#if 0
    int matID;
    NNSG3dMatAnmResult* matResult;
    NNS_G3D_GET_MATID(NNS_G3dRenderObjGetResMdl(NNS_G3dRSGetRenderObj(rs)),
                      &matID,
                      "lambert2");
    SDK_ASSERT(matID >= 0);

    matResult = NNS_G3dRSGetMatAnmResult(rs);

    if (NNS_G3dRSGetCurrentMatID(rs) == matID)
    {
        matResult->prmPolygonAttr =
            (matResult->prmPolygonAttr & ~REG_G3_POLYGON_ATTR_ALPHA_MASK) |
            (16 << REG_G3_POLYGON_ATTR_ALPHA_SHIFT);
    }
    else
    {
        matResult->flag |= NNS_G3D_MATANM_RESULTFLAG_WIREFRAME;
    }
#else
	NNSG3dMatAnmResult* matResult = NNS_G3dRSGetMatAnmResult(rs);

	matResult->prmPolygonAttr = (matResult->prmPolygonAttr & ~REG_G3_POLYGON_ATTR_ALPHA_MASK) |
								(8 << REG_G3_POLYGON_ATTR_ALPHA_SHIFT);
#endif
}
#endif



#if 0
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
