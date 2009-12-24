//=============================================================================
/**
 * @file	dp3format.h
 * @brief   マップエディターの出力ファイル定義
 * @author	ohno_katsunmi@gamefreak.co.jp
 * @date    2008.09.02
 */
//=============================================================================

#ifndef _DP3FORMAT_H_
#define _DP3FORMAT_H_

#define DP3PACK_HEADER (0x4433)   //"3D"をひっくり返した
#define WBGRIDPACK_HEADER 'BW'  //"WB"の逆 標準グリッドマップ用
#define WBGCROSSPACK_HEADER 'CG' //"GC" GridCrossの逆 グリッド立体交差マップ用
#define WBRANDOMPACK_HEADER 'DR'  //"RD"の逆 randomマップ用

/// 高さデータ取得用
typedef struct {
	fx16	vecN1_x;        //１個目の三角形の法線ベクトル
	fx16	vecN1_y;
	fx16	vecN1_z;

	fx16	vecN2_x;       //２個目の三角形の法線ベクトル
	fx16	vecN2_y;
	fx16	vecN2_z;

	fx32	vecN1_D;       // ax+by+cz+d =0 のD値
	fx32	vecN2_D;

	u32		attr:31;          //アトリビュートビット    0-15がvalue 16-30がflg
	u32		tryangleType:1;   //三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
} NormalVtxSt;

typedef struct{
    u16			width;           // 高さデータの横の個数
    u16			height;          // 高さデータの縦の個数
//	NormalVtxSt* vtxData;     // 高さデータが配列で横×縦分ある 現在不要の為、コメント化 081113 kaga
} NormalVtxFormat;



///ポジションデータ取得

typedef struct{
    fx32	xpos;
    fx32	ypos;
    fx32	zpos;
    u16		rotate;
    u8		billboard;
    u8		resourceID;
} PositionSt;

// ポジションデータ全体
typedef struct{
    u32			count;       ///< ポジションデータ数
	PositionSt* posData;     ///< ポジションデータ
} LayoutFormat;

/// 全部をパックしたファイルのヘッダー
//サンプル版
typedef struct {
    u16 DataID;         ////< DP3PACK_HEADER
    u16 dummy1;
    u32 nsbmdOffset;    ///< ファイルの先頭からnsbmdの場所までのOFFSET
    u32 nsbtxOffset;    ///< ファイルの先頭からnsbtxの場所までのOFFSET
    u32 nsbtpOffset;    ///< ファイルの先頭からnsbtpの場所までのOFFSET
    u32 vertexOffset;   ///< ファイルの先頭から法線＋アトリビュートの場所までのOFFSET
    u32 positionOffset; ///< ファイルの先頭からポジションの場所までのOFFSET
    u32 endPos;         ///< ファイルの先頭からポジションの最後までのOFFSET
} Dp3packHeaderSt;


/// 全部をパックしたファイルのヘッダー
//WBグリッドマップ用(標準)
typedef struct {
    u16 DataID;         ////< DP3PACK_HEADER
    u16 dummy1;
    u32 nsbmdOffset;    ///< ファイルの先頭からnsbmdの場所までのOFFSET
  //  u32 nsbtxOffset;    ///< ファイルの先頭からnsbtxの場所までのOFFSET
  // u32 nsbtpOffset;    ///< ファイルの先頭からnsbtpの場所までのOFFSET
    u32 vertexOffset;   ///< ファイルの先頭から法線＋アトリビュートの場所までのOFFSET
    u32 positionOffset; ///< ファイルの先頭からポジションの場所までのOFFSET
    u32 endPos;         ///< ファイルの先頭からポジションの最後までのOFFSET
} WBGridMapPackHeaderSt;

//WBグリッド立体交差マップ用(グリッド拡張型)
typedef struct {
    u16 DataID;         ////< DP3PACK_HEADER
    u16 dummy1;
    u32 nsbmdOffset;    ///< ファイルの先頭からnsbmdの場所までのOFFSET
    u32 vertexOffset;   ///< ファイルの先頭から法線＋アトリビュートの場所までのOFFSET
    u32 exAttrOffset;   ///< ファイルの先頭からエキストラアトリビュートまでのOFFSET
    u32 positionOffset; ///< ファイルの先頭からポジションの場所までのOFFSET
    u32 endPos;         ///< ファイルの先頭からポジションの最後までのOFFSET
} WBGridCrossMapPackHeaderSt;





//-----------------------------------------------------------------------------
/**
 * @brief      index番目の法線ベクトル構造体ポインタを得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval     PositionSt
 */
//-----------------------------------------------------------------------------
extern NormalVtxSt* DP3MAP_GetNormalVtxSt(const int index,const NormalVtxFormat* pNormalVtx);

//-----------------------------------------------------------------------------
/**
 * @brief       index番目の法線ベクトルを得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  VecFx32* 返す VecFx32
 * @param[out]  VecFx32* 返す VecFx32
 * @retval      none
 */
//-----------------------------------------------------------------------------
extern void DP3MAP_GetNormalVtxPosition(const int index,const NormalVtxFormat* pNormalVtx,VecFx16* vc1 ,VecFx16* vc2);

//-----------------------------------------------------------------------------
/**
 * @brief       index番目の内積値を得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  内積１
 * @param[out]  内積２
 * @retval      none
 */
//-----------------------------------------------------------------------------
extern void DP3MAP_GetNormalVtxInnerProduct(const int index,const NormalVtxFormat* pNormalVtx,fx16* ip1 ,fx16* ip2);

//-----------------------------------------------------------------------------
/**
 * @brief       index番目のアトリビュートBITを得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  内積１
 * @param[out]  内積２
 * @retval      none
 */
//-----------------------------------------------------------------------------
extern u32 DP3MAP_GetNormalVtxAttrBit(const int index,const NormalVtxFormat* pNormalVtx);

//-----------------------------------------------------------------------------
/**
 * @brief       index番目のtryangleTypeを得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      三角形の形のタイプ  ＼ = 0  ／ = 1
 */
//-----------------------------------------------------------------------------
extern u32 DP3MAP_GetNormalVtxTriangleType(const int index,const NormalVtxFormat* pNormalVtx);


//-----------------------------------------------------------------------------
/**
 * @brief       与えられた位置の法線ベクトルと内積を返す
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[in]   posInBlock*   いる座標
 * @param[in]   map_width     ＭＡＰの長さ
 * @param[in]   map_height    ＭＡＰの高さ
 * @param[out]  nomalOut      法線ベクトル
 * @retval      内積
 */
//-----------------------------------------------------------------------------

extern fx32 DP3MAP_GetNormalVector( const NormalVtxFormat* pNormalVtx, const VecFx32* posInBlock,
                             const fx32 map_width, const fx32 map_height, VecFx32* normalOut );

//-----------------------------------------------------------------------------
/**
 * @brief      index番目の配置データ構造体ポインタを得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval     PositionSt
 */
//-----------------------------------------------------------------------------
extern PositionSt* DP3MAP_GetLayoutPositionSt(const int index,const LayoutFormat* pLayout);

//-----------------------------------------------------------------------------
/**
 * @brief      index番目の配置を得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  VecFx32* 返す VecFx32
 * @retval  none
 */
//-----------------------------------------------------------------------------

extern void DP3MAP_GetLayoutPosition(const int index,const LayoutFormat* pLayout,VecFx32* vc );

//-----------------------------------------------------------------------------
/**
 * @brief      index番目の角度を得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      角度
 */
//-----------------------------------------------------------------------------

extern u16 DP3MAP_GetLayoutRotate(const int index,const LayoutFormat* pLayout );

//-----------------------------------------------------------------------------
/**
 * @brief       index番目のビルボードフラグを得る
 * @param[in]   index
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      TRUEならビルボード
 */
//-----------------------------------------------------------------------------

extern BOOL DP3MAP_GetLayoutBillboard(const int index,const LayoutFormat* pLayout );

//-----------------------------------------------------------------------------
/**
 * @brief       index番目の配置物IDを得る
 * @param[in]   index
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      配置物ID
 */
//-----------------------------------------------------------------------------

extern u8 DP3MAP_GetLayoutResourceID(const int index,const LayoutFormat* pLayout );


#endif //_DP3FORMAT_H_
