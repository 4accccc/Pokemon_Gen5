//=============================================================================================
/**
 * @file	g3d_util.c                                                  
 * @brief	３Ｄ描画管理システム使用プログラム
 * @date	2006/4/26
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================
typedef struct {
	u8			priority;				
	BOOL		drawSW;
}GFL_G3D_OBJSTAT;

typedef struct {
	GFL_G3D_RES**		g3DresTbl;
	GFL_AREAMAN*		g3DresAreaman;
	u16					g3DresCount;

	GFL_G3D_OBJ**		g3DobjTbl;
	GFL_AREAMAN*		g3DobjAreaman;
	u16					g3DobjCount;
	GFL_G3D_OBJSTAT*	g3DobjStat;

	GFL_G3D_ANM**		g3DanmTbl;
	GFL_AREAMAN*		g3DanmAreaman;
	u16					g3DanmCount;

	u16*				g3DobjPriTbl;

	HEAPID				heapID;
}GFL_G3D_UTIL;

GFL_G3D_UTIL* g3Dutil = NULL;

//=============================================================================================
/**
 *
 *
 * ３Ｄ関連ハンドルの配列管理ユーティリティー
 *
 *
 */
//=============================================================================================
#define HANDLE_POINTER_SIZE (4)
//--------------------------------------------------------------------------------------------
/**
 * セットアップ
 *
 * @param	resCount		管理リソース最大数	
 * @param	objCount		管理オブジェクト最大数
 * @param	anmCount		管理アニメーション最大数
 * @param	heapID			ヒープＩＤ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilsysInit
		( u16 resCount, u16 objCount, u16 anmCount, HEAPID heapID )  
{
	int	i;

	GF_ASSERT( !g3Dutil );

	//管理領域確保
	g3Dutil = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_UTIL) );

	//リソース管理配列作成
	g3Dutil->g3DresTbl = GFL_HEAP_AllocMemory( heapID, HANDLE_POINTER_SIZE * resCount );
	//リソース配列領域マネージャ作成
	g3Dutil->g3DresAreaman = GFL_AREAMAN_Create( resCount, heapID );

	//オブジェクト管理配列作成
	g3Dutil->g3DobjTbl = GFL_HEAP_AllocMemory( heapID, HANDLE_POINTER_SIZE * objCount );
	//リソース配列領域マネージャ作成
	g3Dutil->g3DobjAreaman = GFL_AREAMAN_Create( objCount, heapID );
	//オブジェクト描画ステータス管理配列作成（オブジェクト管理配列作成と並び順は同じ）
	g3Dutil->g3DobjStat = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_OBJSTAT)*objCount );
	//オブジェクト描画プライオリティー管理配列作成
	g3Dutil->g3DobjPriTbl = GFL_HEAP_AllocMemory( heapID, sizeof(u16)*objCount );

	//アニメーション管理配列作成
	g3Dutil->g3DanmTbl = GFL_HEAP_AllocMemory( heapID, HANDLE_POINTER_SIZE * anmCount );
	//アニメーション配列領域マネージャ作成
	g3Dutil->g3DanmAreaman = GFL_AREAMAN_Create( anmCount, heapID );

	//初期化
	for( i=0; i<resCount; i++ ){
		g3Dutil->g3DresTbl[i] = NULL;
	}
	for( i=0; i<objCount; i++ ){
		g3Dutil->g3DobjTbl[i] = NULL;

		g3Dutil->g3DobjStat[i].priority = 0;
		g3Dutil->g3DobjStat[i].drawSW = FALSE;
	}
	for( i=0; i<anmCount; i++ ){
		g3Dutil->g3DresTbl[i] = NULL;
	}
	g3Dutil->g3DresCount = resCount;
	g3Dutil->g3DobjCount = objCount;
	g3Dutil->g3DanmCount = anmCount;
	g3Dutil->heapID = heapID;
}

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_UtilsysExit
		( void )  
{
	int	i;

	GF_ASSERT( g3Dutil );

	//リソース配列領域マネージャ破棄
	GFL_AREAMAN_Delete( g3Dutil->g3DanmAreaman );
	//解放忘れチェック
	for( i=0; i<g3Dutil->g3DanmCount; i++ ){
		GF_ASSERT_MSG( !(g3Dutil->g3DanmTbl[i]),"not unload animetion exist" );
	}
	//リソース管理配列破棄
	GFL_HEAP_FreeMemory( g3Dutil->g3DanmTbl );

	//オブジェクト描画プライオリティー管理配列破棄
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjPriTbl );
	//オブジェクト描画ステータス管理配列破棄
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjStat );
	//オブジェクト配列領域マネージャ破棄
	GFL_AREAMAN_Delete( g3Dutil->g3DobjAreaman );
	//解放忘れチェック
	for( i=0; i<g3Dutil->g3DobjCount; i++ ){
		GF_ASSERT_MSG( !(g3Dutil->g3DobjTbl[i]),"not unload object exist" );
	}
	//オブジェクト管理配列破棄
	GFL_HEAP_FreeMemory( g3Dutil->g3DobjTbl );

	//リソース配列領域マネージャ破棄
	GFL_AREAMAN_Delete( g3Dutil->g3DresAreaman );
	//解放忘れチェック
	for( i=0; i<g3Dutil->g3DresCount; i++ ){
		GF_ASSERT_MSG( !(g3Dutil->g3DresTbl[i]),"not unload resouce exist" );
	}
	//リソース管理配列破棄
	GFL_HEAP_FreeMemory( g3Dutil->g3DresTbl );

	//管理領域解放
	GFL_HEAP_FreeMemory( g3Dutil );
	g3Dutil = NULL;
}





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
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannnot reserve resouce block" );

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
				//ＶＲＡＭ転送済みかどうか確認
				if( GFL_G3D_ObjTexkeyLiveCheck( g3Dres ) == TRUE ){
					//ＶＲＡＭ解放
					GFL_G3D_VramUnloadTex( g3Dres );
				}
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
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannnot reserve object block" );

	//配列にセットアップ
	for( i=0; i<objCount; i++ ){
		g3DresMdl = GFL_G3D_UtilResGet( objTable[i].mdlresID );
		GF_ASSERT( g3DresMdl );

		if( objTable[i].texresID != GFL_G3D_UTIL_RESNULL ){
			g3DresTex = GFL_G3D_UtilResGet( objTable[i].texresID );
		} else {
			g3DresTex = NULL;
		}
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
	GF_ASSERT_MSG(( pos != AREAMAN_POS_NOTFOUND ), "cannnot reserve animetion block" );

	//配列にセットアップ
	for( i=0; i<anmCount; i++ ){
		g3DresAnm = GFL_G3D_UtilResGet( anmTable[i].anmresID );
		GF_ASSERT( g3DresAnm );
		g3Dobj = GFL_G3D_UtilObjGet( anmTable[i].forMdl );
		GF_ASSERT( g3Dobj );

		g3Dutil->g3DanmTbl[ pos+i ] = GFL_G3D_AnmCreate( g3Dobj, g3DresAnm, anmTable[i].anmdatID,
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







