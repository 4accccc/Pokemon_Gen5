//============================================================================================
/**
 */
//============================================================================================
#pragma once

#include "field/map_matrix.h"

#include "height_ex.h"

#include "field/fieldmap_proc.h"


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
#define FLD_MAPPER_CROSSBLOCK_MEMSIZE	( FLD_MAPPER_MAPMDL_SIZE + FLD_MAPPER_MAPTEX_SIZE + (FLD_MAPPER_MAPATTR_SIZE*2) )


//-------------------------------------
///	描画ブロック指定
//=====================================
typedef enum{
  FLDMAPPER_DRAW_TOP,   // トップフレーム描画
  FLDMAPPER_DRAW_TAIL,  // テイルフレーム描画

  FLDMAPPER_DRAW_TYPE_MAX,
} FLDMAPPER_DRAW_TYPE;

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
extern BOOL	FLDMAPPER_Main( FLDMAPPER* g3Dmapper );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムメイン後半
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_MainTail( FLDMAPPER* g3Dmapper );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_Draw( const FLDMAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera, FLDMAPPER_DRAW_TYPE type );
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
 * @brief	マップ位置からカレントブロックアクセスindexを取得
 */
//------------------------------------------------------------------
extern int FLDMAPPER_GetCurrentBlockAccessIdx( const FLDMAPPER* g3Dmapper );


//------------------------------------------------------------------
/**
 * @brief	グリッド情報ワーク初期化
 */
//------------------------------------------------------------------
extern void FLDMAPPER_GRIDINFODATA_Init( FLDMAPPER_GRIDINFODATA* gridInfoData );
extern void FLDMAPPER_GRIDINFO_Init( FLDMAPPER_GRIDINFO* gridInfo );
//------------------------------------------------------------------
/**
 * @brief	グリッドの全階層アトリビュート情報取得
 */
//------------------------------------------------------------------
extern BOOL FLDMAPPER_GetGridInfo
	( const FLDMAPPER* g3Dmapper, const VecFx32* pos, FLDMAPPER_GRIDINFO* gridInfo );

//------------------------------------------------------------------
/**
 * @brief	グリッドの現在の高さに最も近い階層のアトリビュート情報取得
 */
//------------------------------------------------------------------
extern BOOL FLDMAPPER_GetGridData
	( const FLDMAPPER* g3Dmapper, const VecFx32* pos, FLDMAPPER_GRIDINFODATA* pGridData );

//------------------------------------------------------------------
/**
 * @brief	グリッドアトリビュート情報取得(エフェクトエンカウント専用！　他では使わないで！)
 *
 * @param g3Dmapper 
 * @param pos       x,zにデータを取得したいposの座標を格納
 * @param gridInfo  グリッドデータを取得するFLDMAPPER_GRIDINFO構造体型変数へのポインタ
 *
 * @li  複層データ及び拡張高さデータを無視しベース階層データのみ＆カレントブロックのみからデータを取得します
 */
//------------------------------------------------------------------
extern BOOL FLDMAPPER_GetGridDataForEffectEncount
	( const FLDMAPPER* g3Dmapper, int blockIdx, const VecFx32* pos, FLDMAPPER_GRIDINFODATA* pGridData );

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
/**
 * @brief ブロック位置取得
 */
//------------------------------------------------------------------
extern void FLDMAPPER_GetBlockXZPos( const FLDMAPPER * g3Dmapper, u32 * blockx, u32 * blockz );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern BOOL DEBUG_Field_Grayscale(GFL_G3D_RES *g3Dres);


//--------------------------------------------------------------
/**
 * @brief マップ接続
 *
 * @param fieldmap  フィールドマップ
 * @param g3Dmapper 接続対象マッパー
 * @param matrix    接続するマップのマップマトリックス
 *
 * @return 接続できたらTRUE
 */
//--------------------------------------------------------------
BOOL FLDMAPPER_Connect( FIELDMAP_WORK* fieldmap, FLDMAPPER* g3Dmapper, const MAP_MATRIX* matrix ); 

//--------------------------------------------------------------
/**
 * @brief デバッグ出力関数
 *
 * @param g3Dmapper 状態を出力したいマッパー
 */
//--------------------------------------------------------------
extern void FLDMAPPER_DebugPrint( const FLDMAPPER* g3Dmapper );

//--------------------------------------------------------------
/**
 * @brief 拡張高さデータリストポインタ取得
 *
 * @param g3Dmapper 状態を出力したいマッパー
 *
 * @return  拡張高さデータリストポインタ
 */
//--------------------------------------------------------------
extern EHL_PTR	FLDMAPPER_GetExHegihtPtr( FLDMAPPER* g3Dmapper );
