//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		billboard.h
 *	@brief		ビルボードアクターシステム
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __BILLBOARD_H__
#define __BILLBOARD_H__

#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	テクスチャフォーマット
//-------------------------------------
typedef enum {
	GFL_BBD_TEXFMT_PAL16 = 0,	//16色パレット
	GFL_BBD_TEXFMT_PAL256,		//256色パレット
	GFL_BBD_TEXFMT_PAL4,		//4色パレット
	GFL_BBD_TEXFMT_A3I5,		//半透明テクスチャ(α3bit
	GFL_BBD_TEXFMT_A5I3,		//半透明テクスチャ(α5bit
	GFL_BBD_TEXFMT_ERROR,
}GFL_BBD_TEXFMT;

//-------------------------------------
///	ビルボードテクスチャサイズ
//-------------------------------------
enum {
	// 2の(3 + n)累乗値
	GFL_BBD_TEXSIZ_8 = 0,
	GFL_BBD_TEXSIZ_16 = 1,
	GFL_BBD_TEXSIZ_32 = 2,
	GFL_BBD_TEXSIZ_64 = 3,
	GFL_BBD_TEXSIZ_128 = 4,
	GFL_BBD_TEXSIZ_256 = 5,
	GFL_BBD_TEXSIZ_512 = 6,
	GFL_BBD_TEXSIZ_1024 = 7,
};

typedef enum {
	GFL_BBD_TEXSIZ_8x8				= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_8x16				= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_8x32				= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_8x64				= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_8x128			= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_8x256			= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_8x512			= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_8x1024			= (GFL_BBD_TEXSIZ_8 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_16x8				= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_16x16			= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_16x32			= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_16x64			= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_16x128			= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_16x256			= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_16x512			= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_16x1024		= (GFL_BBD_TEXSIZ_16 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_32x8				= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_32x16			= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_32x32			= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_32x64			= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_32x128			= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_32x256			= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_32x512			= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_32x1024		= (GFL_BBD_TEXSIZ_32 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_64x8				= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_64x16			= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_64x32			= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_64x64			= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_64x128			= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_64x256			= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_64x512			= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_64x1024		= (GFL_BBD_TEXSIZ_64 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_128x8			= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_128x16			= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_128x32			= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_128x64			= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_128x128		= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_128x256		= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_128x512		= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_128x1024		= (GFL_BBD_TEXSIZ_128 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_256x8			= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_256x16			= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_256x32			= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_256x64			= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_256x128		= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_256x256		= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_256x512		= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_256x1024		= (GFL_BBD_TEXSIZ_256 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_512x8			= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_512x16			= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_512x32			= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_512x64			= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_512x128		= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_512x256		= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_512x512		= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_512x1024		= (GFL_BBD_TEXSIZ_512 << 4) + GFL_BBD_TEXSIZ_1024,
	GFL_BBD_TEXSIZ_1024x8			= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_8,
	GFL_BBD_TEXSIZ_1024x16		= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_16,
	GFL_BBD_TEXSIZ_1024x32		= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_32,
	GFL_BBD_TEXSIZ_1024x64		= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_64,
	GFL_BBD_TEXSIZ_1024x128		= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_128,
	GFL_BBD_TEXSIZ_1024x256		= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_256,
	GFL_BBD_TEXSIZ_1024x512		= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_512,
	GFL_BBD_TEXSIZ_1024x1024	= (GFL_BBD_TEXSIZ_1024 << 4) + GFL_BBD_TEXSIZ_1024,
}GFL_BBD_TEXSIZ;

//-------------------------------------
///	ライトマスク
//-------------------------------------
typedef enum {
	GFL_BBD_LIGHT_NONE = 0,
	GFL_BBD_LIGHTMASK_0,	// = 1
	GFL_BBD_LIGHTMASK_1,	// = 2
	GFL_BBD_LIGHTMASK_01,
	GFL_BBD_LIGHTMASK_2,	// = 4
	GFL_BBD_LIGHTMASK_02,
	GFL_BBD_LIGHTMASK_12,
	GFL_BBD_LIGHTMASK_012,
	GFL_BBD_LIGHTMASK_3,	// = 8
	GFL_BBD_LIGHTMASK_03,
	GFL_BBD_LIGHTMASK_13,
	GFL_BBD_LIGHTMASK_013,
	GFL_BBD_LIGHTMASK_23,
	GFL_BBD_LIGHTMASK_023,
	GFL_BBD_LIGHTMASK_123,
	GFL_BBD_LIGHTMASK_0123,
}GFL_BBD_LIGHTMASK;

//-------------------------------------
///	アルファなし定義
//-------------------------------------
#define GFL_BBD_NON_ALPHA	(31)

//-------------------------------------
///	原点タイプ
//-------------------------------------
typedef enum {
	GFL_BBD_ORIGIN_CENTER,
	GFL_BBD_ORIGIN_TOP,
	GFL_BBD_ORIGIN_BOTTOM,
	GFL_BBD_ORIGIN_LEFT,
	GFL_BBD_ORIGIN_RIGHT,

	GFL_BBD_ORIGIN_TOP_LEFT,
	GFL_BBD_ORIGIN_TOP_RIGHT,
	GFL_BBD_ORIGIN_BOTTOM_LEFT,
	GFL_BBD_ORIGIN_BOTTOM_RIGHT,

	GFL_BBD_ORIGIN_MAX,	// モジュール内で使用
}GFL_BBD_ORIGIN;

//-------------------------------------
///	描画タイプ
//-------------------------------------
typedef enum {
	GFL_BBD_DRAWMODE_BILLBORD = 0,
	GFL_BBD_DRAWMODE_Y_BILLBORD,
	GFL_BBD_DRAWMODE_NON_BILLBORD,
	GFL_BBD_DRAWMODE_XZ_FLAT_BILLBORD,
}GFL_BBD_DRAWMODE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ビルボードシステム不完全型
//-------------------------------------
typedef struct _GFL_BBD_SYS		GFL_BBD_SYS;

//-------------------------------------
///	ビルボードシステム設定用構造体
//-------------------------------------
typedef struct {
	u16							resCountMax;
	u16							objCountMax;
	VecFx32						scale;
	GXRgb						diffuse;
    GXRgb						ambient;
	GXRgb						specular;
    GXRgb						emission;
	u8							polID;
	u8							origin;
}GFL_BBD_SETUP;

//-------------------------------------
///	デフォルト設定用構造体
//=====================================
extern const GFL_BBD_SETUP defaultSetup;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *
 * @brief	セットアップ
 *
 */
//------------------------------------------------------------------
//	ビルボードシステム作成 (return billboardSys)
extern GFL_BBD_SYS*	GFL_BBD_CreateSys( const GFL_BBD_SETUP* setup, HEAPID heapID );

//	ビルボードシステム破棄
extern void	GFL_BBD_DeleteSys( GFL_BBD_SYS* billboardSys );

//	ビルボードシステム各種パラメータの取得と変更
extern void	GFL_BBD_SetUseCustomVecN( GFL_BBD_SYS* billboardSys, VecFx16* pVecN );
extern void	GFL_BBD_ResetUseCustomVecN( GFL_BBD_SYS* billboardSys );
extern void	GFL_BBD_SetFlushType( GFL_BBD_SYS* billboardSys, GFL_G3D_DRAWFLUSH_TYPE flushType );

extern void	GFL_BBD_GetScale( GFL_BBD_SYS* billboardSys, VecFx32* scale );
extern void	GFL_BBD_SetScale( GFL_BBD_SYS* billboardSys, VecFx32* scale );
extern void	GFL_BBD_GetDiffuse( GFL_BBD_SYS* billboardSys, GXRgb* diffuse );
extern void	GFL_BBD_SetDiffuse( GFL_BBD_SYS* billboardSys, GXRgb* diffuse );
extern void	GFL_BBD_GetAmbient( GFL_BBD_SYS* billboardSys, GXRgb* ambient );
extern void	GFL_BBD_SetAmbient( GFL_BBD_SYS* billboardSys, GXRgb* ambient );
extern void	GFL_BBD_GetSpecular( GFL_BBD_SYS* billboardSys, GXRgb* specular );
extern void	GFL_BBD_SetSpecular( GFL_BBD_SYS* billboardSys, GXRgb* specular );
extern void	GFL_BBD_GetEmission( GFL_BBD_SYS* billboardSys, GXRgb* emission );
extern void	GFL_BBD_SetEmission( GFL_BBD_SYS* billboardSys, GXRgb* emission );
extern void	GFL_BBD_GetPolID( GFL_BBD_SYS* billboardSys, u8* polID );
extern void	GFL_BBD_SetPolID( GFL_BBD_SYS* billboardSys, u8* polID );
extern void	GFL_BBD_GetOrigin( GFL_BBD_SYS* billboardSys, u8* origin );
extern void	GFL_BBD_SetOrigin( GFL_BBD_SYS* billboardSys, u8 origin  );
extern void GFL_BBD_GetFlipTexOffs( GFL_BBD_SYS* billboardSys, fx16* flip_tex_offs_s, fx16* flip_tex_offs_t );
extern void GFL_BBD_SetFlipTexOffs( GFL_BBD_SYS* billboardSys, fx16 flip_tex_offs_s, fx16 flip_tex_offs_t );

//GFL_BBD_TEXSIZからSとTの実数値を取得
extern void GFL_BBD_GetTexSize( GFL_BBD_TEXSIZ texSiz, u16 * s, u16 * t );

//------------------------------------------------------------------
/**
 * @brief	ビルボードリソース
 */
//------------------------------------------------------------------
//	ビルボードリソース追加
//		１）読み込み済みのリソースを登録(return resIdx)
extern int GFL_BBD_AddResource( GFL_BBD_SYS* billboardSys, 
																GFL_G3D_RES* g3DresTex,
																GFL_BBD_TEXFMT texFmt, GFL_BBD_TEXSIZ texSiz, 
																u8 celSizX, u8 celSizY ); 
//		２）アーカイブＩＤによる読み込み(return resIdx)
extern int GFL_BBD_AddResourceArc(	GFL_BBD_SYS* billboardSys, 
																		int arcID, int datID,
																		GFL_BBD_TEXFMT texFmt, GFL_BBD_TEXSIZ texSiz, 
																		u8 celSizX, u8 celSizY ); 
//		３）アーカイブファイルパスによる読み込み(return resIdx)
extern int GFL_BBD_AddResourcePath( GFL_BBD_SYS* billboardSys, 
																		const char* path, int datID,
																		GFL_BBD_TEXFMT texFmt, GFL_BBD_TEXSIZ texSiz, 
																		u8 celSizX, u8 cellSizY ); 
//		４）テクスチャキー・パレットキーから作成
extern int GFL_BBD_AddResourceKey(	GFL_BBD_SYS* billboardSys, 
																		NNSGfdTexKey texKey ,NNSGfdPlttKey plttKey ,
																		GFL_BBD_TEXFMT texFmt, GFL_BBD_TEXSIZ texSiz, 
																		u8 celSizX, u8 cellSizY ); 
//	ビルボードリソース破棄
//		１）リソースＩＤＸによる破棄
extern void	GFL_BBD_RemoveResource( GFL_BBD_SYS* billboardSys, int resIdx );
//		２）全破棄
extern void	GFL_BBD_RemoveResourceAll( GFL_BBD_SYS* billboardSys );

//	ビルボードリソースデータポインタ取得
extern u32	GFL_BBD_GetResourceData( GFL_BBD_SYS* billboardSys, int resIdx );
//	ビルボードリソースデータカット
extern void	GFL_BBD_CutResourceData( GFL_BBD_SYS* billboardSys, int resIdx );

//------------------------------------------------------------------
/**
 * @brief	ビルボードリソース各種パラメータの取得
 */
//------------------------------------------------------------------
//texDataAdrs
extern void	GFL_BBD_GetResourceTexDataAdrs
				( GFL_BBD_SYS* billboardSys, int resIdx, u32* texDataAdrs );
//texPlttAdrs
extern void	GFL_BBD_GetResourceTexPlttAdrs
				( GFL_BBD_SYS* billboardSys, int resIdx, u32* texPlttAdrs );
//NNSGfdTexKey
extern void	GFL_BBD_GetResourceTexKey
        ( GFL_BBD_SYS* billboardSys, int resIdx, NNSGfdTexKey* texKey );
extern void	GFL_BBD_SetResourceTexKey
        ( GFL_BBD_SYS* billboardSys, int resIdx, NNSGfdTexKey texKey );
//NNSGfdPlttKey
extern void	GFL_BBD_GetResourcePlttKey
        ( GFL_BBD_SYS* billboardSys, int resIdx, NNSGfdPlttKey* plttKey );
extern void	GFL_BBD_SetResourcePlttKey
        ( GFL_BBD_SYS* billboardSys, int resIdx, NNSGfdPlttKey plttKey );
//cel計算
enum {
	CEL_OFFS_1D = 0,
	CEL_OFFS_2D,
};
extern void	GFL_BBD_GetResourceCelOffset( GFL_BBD_SYS* billboardSys, int resIdx, 
								const u16 celIdx, u32* dataOffs, u32* celDataSiz, u8 type );

//------------------------------------------------------------------
/**
 * @brief	ビルボードオブジェクト
 */
//------------------------------------------------------------------
//	ビルボードオブジェクト追加  (return objIdx)
//extern int	GFL_BBD_AddObject
//		( GFL_BBD_SYS* billboardSys, int resIdx, const fx16 sizX, const fx16 sizY, 
//			const VecFx32* trans, const u8 alpha, const GFL_BBD_LIGHTMASK lightMask );
extern int	GFL_BBD_AddObjectEx(	GFL_BBD_SYS* billboardSys, 
																	int resIdx, 
																	const fx16 sizX, const fx16 sizY, 
																	const VecFx32* trans, 
																	const u8 alpha, 
																	const GFL_BBD_LIGHTMASK lightMask,
																	const GFL_BBD_DRAWMODE drawMode );
inline int	GFL_BBD_AddObject(		GFL_BBD_SYS* billboardSys, 
																	int resIdx, 
																	const fx16 sizX, const fx16 sizY, 
																	const VecFx32* trans, 
																	const u8 alpha, 
																	const GFL_BBD_LIGHTMASK lightMask )
{
	return GFL_BBD_AddObjectEx
		(billboardSys, resIdx, sizX, sizY, trans, alpha, lightMask, GFL_BBD_DRAWMODE_BILLBORD);
}

//	ビルボードオブジェクト破棄
extern void	GFL_BBD_RemoveObject( GFL_BBD_SYS* billboardSys, int objIdx );
//	ビルボードオブジェクト破棄 Vramのみ
extern void	GFL_BBD_RemoveResourceVram( GFL_BBD_SYS* billboardSys, int resIdx );

//------------------------------------------------------------------
/**
 * @brief	ビルボードオブジェクト各種パラメータの取得と変更
 */
//------------------------------------------------------------------
extern void	GFL_BBD_GetObjectResIdx
				( GFL_BBD_SYS* billboardSys, int objIdx, u16* resIdx );
extern void	GFL_BBD_SetObjectResIdx
				( GFL_BBD_SYS* billboardSys, int objIdx, const u16* resIdx );
extern void	GFL_BBD_GetObjectTrans
				( GFL_BBD_SYS* billboardSys, int objIdx, VecFx32* trans );
extern void	GFL_BBD_SetObjectTrans
				( GFL_BBD_SYS* billboardSys, int objIdx, const VecFx32* trans );
extern void	GFL_BBD_GetObjectCelIdx
				( GFL_BBD_SYS* billboardSys, int objIdx, u16* celIdx );
extern void	GFL_BBD_SetObjectCelIdx
				( GFL_BBD_SYS* billboardSys, int objIdx, const u16* celIdx );
extern void	GFL_BBD_GetObjectSiz
				( GFL_BBD_SYS* billboardSys, int objIdx, fx16* sizX, fx16* sizY );
extern void	GFL_BBD_SetObjectSiz
				( GFL_BBD_SYS* billboardSys, int objIdx, const fx16* sizX, const fx16* sizY );
extern void	GFL_BBD_GetObjectAlpha
				( GFL_BBD_SYS* billboardSys, int objIdx, u8* alpha );
extern void	GFL_BBD_SetObjectAlpha
				( GFL_BBD_SYS* billboardSys, int objIdx, const u8* alpha );
extern BOOL	GFL_BBD_GetObjectDrawEnable
				( const GFL_BBD_SYS* billboardSys, int objIdx );
extern void	GFL_BBD_SetObjectDrawEnable
				( GFL_BBD_SYS* billboardSys, int objIdx, const BOOL* drawEnable );
extern GFL_BBD_LIGHTMASK	GFL_BBD_GetObjectLightMask
				( const GFL_BBD_SYS* billboardSys, int objIdx );
extern void	GFL_BBD_SetObjectLightMask
				( GFL_BBD_SYS* billboardSys, int objIdx, const GFL_BBD_LIGHTMASK* lightMask );
extern void	GFL_BBD_OnObjectLightMask
				( GFL_BBD_SYS* billboardSys, int objIdx, GFL_BBD_LIGHTMASK lightMask );
extern void	GFL_BBD_OffObjectLightMask
				( GFL_BBD_SYS* billboardSys, int objIdx, GFL_BBD_LIGHTMASK lightMask );
extern BOOL	GFL_BBD_GetObjectFlipS
				( const GFL_BBD_SYS* billboardSys, int objIdx );
extern void	GFL_BBD_SetObjectFlipS
				( GFL_BBD_SYS* billboardSys, int objIdx, const BOOL* flipS );
extern BOOL	GFL_BBD_GetObjectFlipT
				( const GFL_BBD_SYS* billboardSys, int objIdx );
extern void	GFL_BBD_SetObjectFlipT
				( GFL_BBD_SYS* billboardSys, int objIdx, const BOOL* flipT );
extern void	GFL_BBD_GetObjectRotate
				( GFL_BBD_SYS* billboardSys, int objIdx, u16 *rotZ );
extern void	GFL_BBD_SetObjectRotate
				( GFL_BBD_SYS* billboardSys, int objIdx, const u16* rotZ );
extern void	GFL_BBD_GetObjectPolID
				( GFL_BBD_SYS* billboardSys, int objIdx, u8* polID );
extern void	GFL_BBD_SetObjectPolID
				( GFL_BBD_SYS* billboardSys, int objIdx, const u8* polID );

//------------------------------------------------------------------
/**
 *
 * @brief	ビルボード描画
 *
 */
//------------------------------------------------------------------
extern void	GFL_BBD_Draw
		( GFL_BBD_SYS* billboardSys, GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif		// __BILLBOARD_H__
