#pragma once

#define MAP_BLOCK_COUNT		(9)

#define	MAPMDL_SIZE		(0x0f000)	//モデルデータ用メモリ確保サイズ 
#define	MAPTEX_SIZE		(0x4800) 	//テクスチャデータ用ＶＲＡＭ＆メモリ確保サイズ 
#define	MAPATTR_SIZE	(0x6000) 	//アトリビュート（高さ等）用メモリ確保サイズ 

#define FLD_G3D_MAPPER_ATTR_MAX	(16)

typedef struct _FLD_G3D_MAPPER FLD_G3D_MAPPER;

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

#define FLD_G3D_MAPPER_NOMAP	(0xffffffff)
typedef struct {
	u32 datID;
}FLD_G3D_MAPPER_DATA;

typedef enum {
	FLD_G3D_MAPPER_MODE_SCROLL_NONE = 0,
	FLD_G3D_MAPPER_MODE_SCROLL_XZ,
	FLD_G3D_MAPPER_MODE_SCROLL_Y,
}FLD_G3D_MAPPER_MODE;

#define	NON_LOWQ	(0xffff)
#define	NON_TEX		(0xffff)
typedef struct {
	u16 highQ_ID;
	u16 lowQ_ID;
}FLD_G3D_MAPPEROBJ_DATA;

typedef struct {
	u32							arcID;	//アーカイブＩＤ
	const FLD_G3D_MAPPEROBJ_DATA*	data;	//実マップデータ
	u32							count;		//モデル数

}FLD_G3D_MAPPER_RESIST_OBJ;

typedef struct {
	u32			arcID;	//アーカイブＩＤ
	const u16*	data;	//実マップデータ
	u32			count;	//テクスチャ数

}FLD_G3D_MAPPER_RESIST_DDOBJ;

#define NON_GLOBAL_OBJ	(0xffffffff)
#define USE_GLOBAL_OBJSET_TBL	(0)
#define USE_GLOBAL_OBJSET_BIN	(1)
typedef struct {
	u32							objArcID;	//アーカイブＩＤ
	const FLD_G3D_MAPPEROBJ_DATA*	objData;	//実マップデータ
	u32							objCount;	//モデル数
	u32							ddobjArcID;	//アーカイブＩＤ
	const u16*					ddobjData;	//実マップデータ
	u32							ddobjCount;	//モデル数
}FLD_G3D_MAPPER_GLOBAL_OBJSET_TBL;

typedef struct {
	u32							areaObjArcID;		//配置種類アーカイブＩＤ
	u32							areaObjAnmTblArcID;	//配置種類アニメ付与データアーカイブＩＤ
	u32							areaObjDatID;		//配置種類データＩＤ
	u32							objArcID;			//モデルアーカイブＩＤ
	u32							objtexArcID;		//テクスチャアーカイブＩＤ
	u32							objanmArcID;		//アニメアーカイブＩＤ
}FLD_G3D_MAPPER_GLOBAL_OBJSET_BIN;

#define	NON_GLOBAL_TEX	(0xffffffff)
#define	USE_GLOBAL_TEX	(0)
typedef struct {
	u32 arcID;
	u32 datID;
}FLD_G3D_MAPPER_GLOBAL_TEXTURE;

typedef struct {
	u32						g3DmapFileType;	//g3Dmapファイル識別タイプ（仮）
	fx32					width;			//ブロック１辺の幅
	fx32					height;			//ブロック高さ
	FLD_G3D_MAPPER_MODE			mode;			//動作モード
	u32						arcID;			//グラフィックアーカイブＩＤ
	u32						gtexType;		//グローバルテクスチャタイプ
	void*					gtexData;		//グローバルテクスチャ
	u32						gobjType;		//グローバルオブジェクトタイプ
	void*					gobjData;		//グローバルオブジェクト

	u16						sizex;			//横ブロック数
	u16						sizez;			//縦ブロック数
	u32						totalSize;		//配列サイズ
	const FLD_G3D_MAPPER_DATA*	data;			//実マップデータ

}FLD_G3D_MAPPER_RESIST;

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

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ転送待ち
 */
//------------------------------------------------------------------
extern BOOL CheckTransFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper );

//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
extern void	ResistDataFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper, const FLD_G3D_MAPPER_RESIST* resistData );
extern void	ReleaseDataFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper );

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
extern void GetFieldG3DmapperSize( FLD_G3D_MAPPER* g3Dmapper, fx32* x, fx32* z );

extern void SetFieldG3DmapperDrawOffset( FLD_G3D_MAPPER *g3Dmapper, const VecFx32 *offs );
extern void GetFieldG3DmapperDrawOffset( FLD_G3D_MAPPER *g3Dmapper, VecFx32 *offs );
