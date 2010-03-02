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

  WB_NORMALVTX_TRYANGLE_FLAG_MAX,

} WB_NORMALVTX_TRYANGLE_FLAG;

//-----------------------------------------------------------------------------
/**
 *  WB_NORMALVTXST_TR1  
 *  WB_NORMALVTXST_TR2  
 *
 *    そのブロック内に、WB_NORMALVTXST_TR1のグリッドしかない場合には、
 *    すべてのグリッドはWB_NORMALVTXST_TR1の形式で出力されます。
 *
 *    そのブロック内に、１つでも三角形が２つ必要なグリッド(WB_NORMALVTXST_TR2)が
 *    あった場合には、すべてのグリッドをWB_NORMALVTXST_TR2の形式で出力します。
 *
*/
//-----------------------------------------------------------------------------
/// 高さデータ取得用
// 3角形１
typedef struct {
  u16 tryangleFlag:1;
	u16	vecN1_x:15;         //１個目の三角形の法線ベクトル
	u16	vecN1_D;            // ax+by+cz+d =0 のD値

	u32		attr:31;          //アトリビュートビット    0-15がvalue 16-30がflg
	u32		tryangleType:1;   //三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
} WB_NORMALVTXST_TR1;

// 3角形１
typedef struct {
  u16 tryangleFlag:1;     // WB_NORMALVTX_TRYANGLE_FLAG
	u16	vecN1_x:15;         //１個目の三角形の法線ベクトル
	u16	vecN2_x;            //２個目の三角形の法線ベクトル
	u16	vecN1_D;            // ax+by+cz+d =0 のD値
	u16	vecN2_D;       

	u32		attr:31;          //アトリビュートビット    0-15がvalue 16-30がflg
	u32		tryangleType:1;   //三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
} WB_NORMALVTXST_TR2;

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

#ifdef _cplusplus
}	// extern "C"{
#endif



