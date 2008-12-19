#pragma once
#ifdef __cplusplus
extern "C" {
#endif
//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _GFL_BBD_SYS		GFL_BBD_SYS;

typedef enum {
	GFL_BBD_TEXFMT_PAL16 = 0,
	GFL_BBD_TEXFMT_PAL256,
	GFL_BBD_TEXFMT_PAL4,
	GFL_BBD_TEXFMT_ERROR,
}GFL_BBD_TEXFMT;

typedef enum {
	GFL_BBD_TEXSIZ_8x8 = 0,
	GFL_BBD_TEXSIZ_16x8,
	GFL_BBD_TEXSIZ_16x16,
	GFL_BBD_TEXSIZ_32x16,
	GFL_BBD_TEXSIZ_32x32,
	GFL_BBD_TEXSIZ_64x32,
	GFL_BBD_TEXSIZ_64x64,
	GFL_BBD_TEXSIZ_128x64,
	GFL_BBD_TEXSIZ_128x128,
	GFL_BBD_TEXSIZ_256x128,
	GFL_BBD_TEXSIZ_256x256,
	GFL_BBD_TEXSIZ_512x256,
	GFL_BBD_TEXSIZ_512x512,
	GFL_BBD_TEXSIZ_1024x512,
	GFL_BBD_TEXSIZ_1024x1024,
	GFL_BBD_TEXSIZ_32x512,
	GFL_BBD_TEXSIZ_32x1024,
	GFL_BBD_TEXSIZ_ERROR,
}GFL_BBD_TEXSIZ;

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

typedef struct {
	u16							resCountMax;
	u16							objCountMax;
	VecFx32						scale;
	GXRgb						diffuse;
    GXRgb						ambient;
	GXRgb						specular;
    GXRgb						emission;
	u8							polID;
}GFL_BBD_SETUP;

extern const GFL_BBD_SETUP defaultSetup;

#define GFL_BBD_NON_ALPHA	(31)
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

//------------------------------------------------------------------
/**
 * @brief	ビルボードリソース
 */
//------------------------------------------------------------------
//	ビルボードリソース追加
//		１）読み込み済みのリソースを登録(return resIdx)
extern int GFL_BBD_AddResource( GFL_BBD_SYS* billboardSys, GFL_G3D_RES* g3DresTex,
									u8 texFmt, u8 texSiz, u8 celSizX, u8 celSizY ); 
//		２）アーカイブＩＤによる読み込み(return resIdx)
extern int GFL_BBD_AddResourceArc( GFL_BBD_SYS* billboardSys, int arcID, int datID,
									u8 texFmt, u8 texSiz, u8 celSizX, u8 celSizY ); 
//		３）アーカイブファイルパスによる読み込み(return resIdx)
extern int GFL_BBD_AddResourcePath( GFL_BBD_SYS* billboardSys, const char* path, int datID,
									u8 texFmt, u8 texSiz, u8 celSizX, u8 cellSizY ); 
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
extern int	GFL_BBD_AddObject
		( GFL_BBD_SYS* billboardSys, int resIdx, const fx16 sizX, const fx16 sizY, 
			const VecFx32* trans, const u8 alpha, const GFL_BBD_LIGHTMASK lightMask );
//	ビルボードオブジェクト破棄
extern void	GFL_BBD_RemoveObject( GFL_BBD_SYS* billboardSys, int objIdx );

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
extern void	GFL_BBD_SetObjectDrawEnable
				( GFL_BBD_SYS* billboardSys, int objIdx, const BOOL* drawEnable );
extern void	GFL_BBD_SetObjectLightMask
				( GFL_BBD_SYS* billboardSys, int objIdx, const GFL_BBD_LIGHTMASK* lightMask );
extern void	GFL_BBD_SetObjectFlipS
				( GFL_BBD_SYS* billboardSys, int objIdx, const BOOL* flipS );
extern void	GFL_BBD_SetObjectFlipT
				( GFL_BBD_SYS* billboardSys, int objIdx, const BOOL* flipT );

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
