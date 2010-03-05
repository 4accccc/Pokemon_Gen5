//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wb_attr_data.h
 *	@brief  WB専用　アトリビュート情報
 *	@author	GAME FREAK inc.
 *	@date		2010.03.02
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	アトリビュート・高さデータ　三角形数フラグ
//=====================================
typedef enum {
  WB_NORMALVTX_TRYANGLE_ONE=0,    // 三角形１つのデータ
  WB_NORMALVTX_TRYANGLE_TWO=1,    // 三角形２つのデータ
  WB_NORMALVTX_TRYANGLE_TWO_INDEX=2,// 三角形２つのデータのインデックス（サイズはWB_NORMALVTXST_TR1と一緒）

  WB_NORMALVTX_TRYANGLE_FLAG_MAX,

} WB_NORMALVTX_TRYANGLE_FLAG;

//-----------------------------------------------------------------------------
/**
 *  WB_NORMALVTXST_TR1  
 *  WB_NORMALVTXST_TR2  
*/
//-----------------------------------------------------------------------------
/// 高さデータ取得用
// 3角形　1つ
typedef struct {
  u16 tryangleFlag:2;     // WB_NORMALVTX_TRYANGLE_FLAG
	u16	vecN1_x:14;         //１個目の三角形の法線ベクトルインデックス（このインデックスから３つにx,y,zの順で格納）
	u16	vecN1_D;            // ax+by+cz+d =0 のD値
  //↑tryangleFlag == WB_NORMALVTX_TRYANGLE_TWO_INDEXのとき、vecN1_DはWB_NORMALVTXST_TR2配列のインデックスとして使用

	u32		attr:31;          //アトリビュートビット    0-15がvalue 16-30がflg
	u32		tryangleType:1;   //三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
} WB_NORMALVTXST_TR1;

// 3角形 2つ
typedef struct {
  u16 tryangleFlag:2;     // WB_NORMALVTX_TRYANGLE_FLAG  WB_NORMALVTX_TRYANGLE_TWO 固定
	u16	vecN1_x:14;         //１個目の三角形の法線ベクトル１個目の三角形の法線ベクトルインデックス（このインデックスから３つにx,y,zの順で格納）
	u16	vecN2_x;            //２個目の三角形の法線ベクトル１個目の三角形の法線ベクトルインデックス（このインデックスから３つにx,y,zの順で格納）
	u16	vecN1_D;            // ax+by+cz+d =0 のD値
	u16	vecN2_D;       

} WB_NORMALVTXST_TR2;


//-----------------------------------------------------------------------------
/**
 *					インデックス情報
*/
//-----------------------------------------------------------------------------
#if 0
#define WB_NORMAL_TBL_MAX ( 9 )
#define WB_PLANE_D_TBL_MAX ( 9 )
extern const fx16 WB_NORMAL_TBL[WB_NORMAL_TBL_MAX];  // fx16*3単位で法線が格納されています。
extern const fx32 WB_PLANE_D_TBL[WB_PLANE_D_TBL_MAX];
#endif



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  トライアングルフラグ取得
 *
 *	@param	cp_data    データ
 *
 *	@retval WB_NORMALVTX_TRYANGLE_FLAG
 */
//-----------------------------------------------------------------------------
static inline WB_NORMALVTX_TRYANGLE_FLAG WB_NORMALVTXST_GetTryangleFlag( const void* cp_data )
{
  const WB_NORMALVTXST_TR1* cp_tr = cp_data;
  return cp_tr->tryangleFlag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WB_NORMALVTX_TRYANGLE_TWO_INDEXの情報からTR2インデックスを取得
 *
 *	@param	cp_data    データ
 *
 *	@retval TR2インデックス
 */
//-----------------------------------------------------------------------------
static inline u16 WB_NORMALVTXST_GetTR2Index( const void* cp_data )
{
  const WB_NORMALVTXST_TR1* cp_tr = cp_data;
  GF_ASSERT( cp_tr->tryangleFlag == WB_NORMALVTX_TRYANGLE_TWO_INDEX );
  return cp_tr->vecN1_D;
}

#ifdef _cplusplus
}	// extern "C"{
#endif



