//============================================================================================
/**
 */
//============================================================================================
#pragma once

#include "field_hit_check.h"
//------------------------------------------------------------------
/**
 * @brief	マッパー制御構造体の不完全型定義
 */
//------------------------------------------------------------------
typedef struct _FLD_G3D_MAPPER FLDMAPPER;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}FLDMAPPER_GRIDINFODATA;

#define FLDMAPPER_GRIDINFO_MAX	(16)

typedef struct {
	FLDMAPPER_GRIDINFODATA	gridData[FLDMAPPER_GRIDINFO_MAX];	//グリッドデータ取得ワーク
	u16						count;
}FLDMAPPER_GRIDINFO;



//-------------------------------------
///	1ブロックメモリサイズの基準
//=====================================
#define	FLD_MAPPER_MAPMDL_SIZE		(0xf000)	//モデルデータ用メモリ確保サイズ 
#define	FLD_MAPPER_MAPTEX_SIZE		(0) 	    //テクスチャデータ用ＶＲＡＭ＆メモリ確保サイズ 
#define	FLD_MAPPER_MAPATTR_SIZE	(0x6004) 	//アトリビュート（高さ等）用メモリ確保サイズ 
#define FLD_MAPPER_BLOCK_MEMSIZE	( FLD_MAPPER_MAPMDL_SIZE + FLD_MAPPER_MAPTEX_SIZE + FLD_MAPPER_MAPATTR_SIZE )


//============================================================================================
//
//
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム作成
 */
//------------------------------------------------------------------
extern FLDMAPPER*	FLDMAPPER_Create( HEAPID heapID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムメイン
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_Main( FLDMAPPER* g3Dmapper );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_Draw( const FLDMAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム破棄
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_Delete( FLDMAPPER* g3Dmapper );

extern void	FLDMAPPER_ReleaseData( FLDMAPPER* g3Dmapper );

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ転送待ち
 */
//------------------------------------------------------------------
extern BOOL FLDMAPPER_CheckTrans( const FLDMAPPER* g3Dmapper );


//------------------------------------------------------------------
/**
 * @brief	マップ位置セット
 */
//------------------------------------------------------------------
extern void FLDMAPPER_SetPos( FLDMAPPER* g3Dmapper, const VecFx32* pos );


//------------------------------------------------------------------
/**
 * @brief	グリッド情報ワーク初期化
 */
//------------------------------------------------------------------
extern void FLDMAPPER_GRIDINFODATA_Init( FLDMAPPER_GRIDINFODATA* gridInfoData );
extern void FLDMAPPER_GRIDINFO_Init( FLDMAPPER_GRIDINFO* gridInfo );
//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得
 */
//------------------------------------------------------------------
extern BOOL FLDMAPPER_GetGridInfo
	( const FLDMAPPER* g3Dmapper, const VecFx32* pos, FLDMAPPER_GRIDINFO* gridInfo );


//------------------------------------------------------------------
/**
 * @brief	範囲外チェック
 */
//------------------------------------------------------------------
extern BOOL FLDMAPPER_CheckOutRange( const FLDMAPPER* g3Dmapper, const VecFx32* pos );
//------------------------------------------------------------------
/**
 * @brief	サイズ取得
 */
//------------------------------------------------------------------
extern void FLDMAPPER_GetSize( const FLDMAPPER* g3Dmapper, fx32* x, fx32* z );

extern void FLDMAPPER_SetDrawOffset( FLDMAPPER *g3Dmapper, const VecFx32 *offs );
extern void FLDMAPPER_GetDrawOffset( const FLDMAPPER *g3Dmapper, VecFx32 *offs );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const GFL_G3D_MAP_GLOBALOBJ_ST * FLDMAPPER_CreateObjStatusList
( const FLDMAPPER* g3Dmapper, const FLDHIT_RECT * rect, HEAPID heapID, u32 * num );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern BOOL DEBUG_Field_Grayscale(GFL_G3D_RES *g3Dres);

