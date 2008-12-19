//============================================================================================
/**
 */
//============================================================================================
#pragma once

//------------------------------------------------------------------
//------------------------------------------------------------------

#define MAP_BLOCK_COUNT		(9)

#define	MAPMDL_SIZE		(0x0f000)	//モデルデータ用メモリ確保サイズ 
#define	MAPTEX_SIZE		(0x4800) 	//テクスチャデータ用ＶＲＡＭ＆メモリ確保サイズ 
#define	MAPATTR_SIZE	(0x6000) 	//アトリビュート（高さ等）用メモリ確保サイズ 


//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _FLD_G3D_MAPPER FLD_G3D_MAPPER;

//------------------------------------------------------------------
//------------------------------------------------------------------
#define FLD_G3D_MAPPER_ATTR_MAX	(16)
enum {
	FILE_MAPEDITER_DATA = 0,
	FILE_CUSTOM_DATA = 1,
};

typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}FLD_G3D_MAPPER_INFODATA;

typedef struct {
	FLD_G3D_MAPPER_INFODATA		gridData[FLD_G3D_MAPPER_ATTR_MAX];	//グリッドデータ取得ワーク
	u16						count;
}FLD_G3D_MAPPER_GRIDINFO;

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム作成
 */
//------------------------------------------------------------------
extern FLD_G3D_MAPPER*	CreateFieldG3Dmapper( HEAPID heapID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムメイン
 */
//------------------------------------------------------------------
extern void	MainFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
//------------------------------------------------------------------
extern void	DrawFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム破棄
 */
//------------------------------------------------------------------
extern void	DeleteFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper );

extern void	ReleaseDataFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper );

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ転送待ち
 */
//------------------------------------------------------------------
extern BOOL CheckTransFieldG3Dmapper( const FLD_G3D_MAPPER* g3Dmapper );


//------------------------------------------------------------------
/**
 * @brief	マップ位置セット
 */
//------------------------------------------------------------------
extern void SetPosFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos );


//------------------------------------------------------------------
/**
 * @brief	グリッド情報ワーク初期化
 */
//------------------------------------------------------------------
extern void InitGetFieldG3DmapperGridInfoData( FLD_G3D_MAPPER_INFODATA* gridInfoData );
extern void InitGetFieldG3DmapperGridInfo( FLD_G3D_MAPPER_GRIDINFO* gridInfo );
//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得
 */
//------------------------------------------------------------------
extern BOOL GetFieldG3DmapperGridInfoData
	( FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos, FLD_G3D_MAPPER_INFODATA* gridInfoData );
extern BOOL GetFieldG3DmapperGridInfo
	( const FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos, FLD_G3D_MAPPER_GRIDINFO* gridInfo );

extern u32 GetFieldG3DmapperFileType( const FLD_G3D_MAPPER *g3Dmapper );
extern BOOL GetFieldG3DmapperGridAttr(
	const FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos, u16 *attr );

//------------------------------------------------------------------
/**
 * @brief	範囲外チェック
 */
//------------------------------------------------------------------
extern BOOL CheckFieldG3DmapperOutRange( const FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos );
//------------------------------------------------------------------
/**
 * @brief	サイズ取得
 */
//------------------------------------------------------------------
extern void GetFieldG3DmapperSize( const FLD_G3D_MAPPER* g3Dmapper, fx32* x, fx32* z );

extern void SetFieldG3DmapperDrawOffset( FLD_G3D_MAPPER *g3Dmapper, const VecFx32 *offs );
extern void GetFieldG3DmapperDrawOffset( const FLD_G3D_MAPPER *g3Dmapper, VecFx32 *offs );

