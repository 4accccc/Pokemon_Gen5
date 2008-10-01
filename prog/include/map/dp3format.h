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

	u32		attr:31;            //アトリビュートビット
	u32		tryangleType:1;   //三角形の形のタイプ  ＼ = 0  ／ = 1
} NormalVtxSt;

typedef struct{
    u16			width;           // 高さデータの横の個数
    u16			height;          // 高さデータの縦の個数
	NormalVtxSt* vtxData;       // 高さデータが配列で横×縦分ある
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


#endif //_DP3FORMAT_H_
