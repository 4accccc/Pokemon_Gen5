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
	GFL_G3D_UTIL*		g3Dutil;
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
 * @param	g3Dutil		依存するg3Dutil
 * @param	actMax		設定可能なアクター数
 * @param	wksiz		１アクターに割り当てるワークのサイズ
 * @param	heapID		ヒープＩＤ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_ACTSYS*
	GFL_G3D_ActorSysCreate
		( GFL_G3D_UTIL* g3Dutil, const u16 actMax, const u32 wksiz, const HEAPID heapID )
{
	GFL_G3D_ACTSYS*	g3DactSys;
	GFL_TCBL*		g3DactTCBL;
	u32	TCBLworkSize = sizeof(GFL_G3D_ACTOR) + wksiz;

	GF_ASSERT( g3Dutil );
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
	g3DactSys->g3Dutil = g3Dutil;
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
		g3Dobj = GFL_G3D_UtilsysObjHandleGet( g3DactSys->g3Dutil, g3Dact->actData.objID );

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
