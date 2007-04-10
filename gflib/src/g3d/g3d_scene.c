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
	GFL_G3D_SCENEOBJ_DATA	sceneObjData;
	GFL_G3D_OBJ*			g3Dobj;
	void*					sceneObjWorkEx;
};

struct _GFL_G3D_SCENE {
	GFL_G3D_UTIL*		g3Dutil;
	GFL_TCBLSYS*		g3DsceneObjTCBLsys;
	GFL_TCBL**			g3DsceneObjTCBLtbl;
	GFL_AREAMAN*		g3DsceneObjAreaman;
	u16					g3DsceneObjMax;	
	u32					g3DsceneObjWorkSize;
	u16*				g3DsceneObjPriTbl;
	HEAPID				heapID;
};


static void sceneObjfunc( GFL_TCBL* tcbl, void* work );
static void objDrawSort( GFL_G3D_SCENE* g3Dscene );

//static void getTranslucent( NNSG3dRS* rs );

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
 * @param	g3Dutil			依存するg3Dutil
 * @param	sceneObjMax		配置可能なオブジェクト数
 * @param	sceneObjWkSiz	１オブジェクトに割り当てるワークのサイズ
 * @param	heapID			ヒープＩＤ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_SCENE*
	GFL_G3D_SCENE_Create
		( GFL_G3D_UTIL* g3Dutil, const u16 sceneObjMax, const u32 sceneObjWkSiz, 
			const HEAPID heapID )
{
	GFL_G3D_SCENE*	g3Dscene;
	GFL_TCBL*		g3DsceneObjTCBL;
	u32	TCBLworkSize = sizeof(GFL_G3D_SCENEOBJ) + sceneObjWkSiz;

	GF_ASSERT( g3Dutil );
	//管理領域確保
	g3Dscene = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_SCENE) );
	//TCBL起動
	g3Dscene->g3DsceneObjTCBLsys = GFL_TCBL_SysInit(heapID,heapID,sceneObjMax,TCBLworkSize); 
	//管理配列作成
	g3Dscene->g3DsceneObjTCBLtbl = GFL_HEAP_AllocClearMemory(heapID,TCBL_POINTER_SIZ*sceneObjMax);
	//配列領域マネージャ作成
	g3Dscene->g3DsceneObjAreaman = GFL_AREAMAN_Create( sceneObjMax, heapID );
	//プライオリティー管理配列作成
	g3Dscene->g3DsceneObjPriTbl = GFL_HEAP_AllocClearMemory( heapID, 2*sceneObjMax );

	g3Dscene->heapID = heapID;
	g3Dscene->g3Dutil = g3Dutil;
	g3Dscene->g3DsceneObjMax = sceneObjMax;
	g3Dscene->g3DsceneObjWorkSize = TCBLworkSize;

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
	GFL_TCBL_SysMain( g3Dscene->g3DsceneObjTCBLsys );
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
	GFL_G3D_OBJ*		g3Dobj;
	GFL_G3D_RND*		g3Drnd;
	NNSG3dResMdl*		pMdl;
	int	i = 0;

	//描画開始
	GFL_G3D_DRAW_Start();
	//カメラグローバルステート設定		
 	GFL_G3D_DRAW_SetLookAt();
	//描画プライオリティーによるソート
	objDrawSort( g3Dscene );

	//各アクターの描画
	while( ( i<g3Dscene->g3DsceneObjMax )&&( g3Dscene->g3DsceneObjPriTbl[i] != 0xffff) ){
		g3DsceneObj = GFL_TCBL_GetWork
						( g3Dscene->g3DsceneObjTCBLtbl[ g3Dscene->g3DsceneObjPriTbl[i] ] );
		g3Dobj = g3DsceneObj->g3Dobj;

		//半透明設定
		g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( g3DsceneObj->g3Dobj );
		pMdl = NNS_G3dRenderObjGetResMdl( GFL_G3D_RENDER_GetRenderObj( g3Drnd ) );
#if 0
		{
			u32 MatID;

			NNS_G3dMdlUseMdlAlpha( pMdl );
			for ( MatID = 0; MatID < pMdl->info.numMat; ++MatID ){
				NNS_G3dMdlSetMdlAlpha( pMdl, MatID, g3DsceneObj->sceneObjData.blendAlpha );
			}
		}
#else
		if( g3DsceneObj->sceneObjData.blendAlpha == GFL_G3D_SCENEOBJ_ALPHA_OFF ){
			NNS_G3dMdlUseMdlPolygonID( pMdl );
			NNS_G3dMdlUseMdlAlpha( pMdl );
		} else {
			NNS_G3dGlbPolygonAttr(	GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE,
									GX_CULL_NONE, i, g3DsceneObj->sceneObjData.blendAlpha, 0 );
			NNS_G3dMdlUseGlbPolygonID( pMdl );	//一時的にＩＤを変更（半透明のため）
			NNS_G3dMdlUseGlbAlpha( pMdl );		//反映しているのはα設定だけ
		}
#endif
		GFL_G3D_DRAW_DrawObjectCullingON( g3Dobj, &g3DsceneObj->sceneObjData.status );
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
	GFL_HEAP_FreeMemory( g3Dscene->g3DsceneObjPriTbl );
	GFL_AREAMAN_Delete( g3Dscene->g3DsceneObjAreaman );
	GFL_HEAP_FreeMemory( g3Dscene->g3DsceneObjTCBLtbl );
	GFL_TCBL_SysExit( g3Dscene->g3DsceneObjTCBLsys );
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
	GF_ASSERT( idx < g3Dscene->g3DsceneObjMax );
	return ( GFL_G3D_SCENEOBJ* )GFL_TCBL_GetWork( g3Dscene->g3DsceneObjTCBLtbl[ idx ] );
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
		g3DsceneObj->sceneObjData	= sceneObjTbl[i];
		g3DsceneObj->g3Dobj	= GFL_G3D_UTIL_GetObjHandle
								( g3Dscene->g3Dutil, g3DsceneObj->sceneObjData.objID );
		g3DsceneObj->sceneObjWorkEx	= NULL;
#if 0
		// 半透明処理コールバック設定
		{
			GFL_G3D_RND* g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( g3DsceneObj->g3Dobj );
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
	GFL_TCBL* g3DsceneObjTCBL;
	int	i;

	GF_ASSERT( g3Dscene );
	for( i=0; i<sceneObjCount; i++ ){
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
	return g3DsceneObj->g3Dobj;
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
