#define MAP_BLOCK_COUNT		(9)

#define	MAPMDL_SIZE		(0x0f000)	//モデルデータ用メモリ確保サイズ 
#define	MAPTEX_SIZE		(0x4800) 	//テクスチャデータ用ＶＲＡＭ＆メモリ確保サイズ 
#define	MAPATTR_SIZE	(0x6000) 	//アトリビュート（高さ等）用メモリ確保サイズ 

#define G3D_MAPPER_ATTR_MAX	(16)

typedef struct _G3D_MAPPER G3D_MAPPER;

enum {
	FILE_MAPEDITER_DATA = 0,
	FILE_CUSTOM_DATA = 1,
};

typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}G3D_MAPPER_INFODATA;

typedef struct {
	G3D_MAPPER_INFODATA		gridData[G3D_MAPPER_ATTR_MAX];	//グリッドデータ取得ワーク
	u16						count;
}G3D_MAPPER_GRIDINFO;

#define G3D_MAPPER_NOMAP	(0xffffffff)
typedef struct {
	u32 datID;
}G3D_MAPPER_DATA;

typedef enum {
	G3D_MAPPER_MODE_SCROLL_NONE = 0,
	G3D_MAPPER_MODE_SCROLL_XZ,
	G3D_MAPPER_MODE_SCROLL_Y,
}G3D_MAPPER_MODE;

#define	NON_LOWQ	(0xffff)
#define	NON_TEX		(0xffff)
typedef struct {
	u16 highQ_ID;
	u16 lowQ_ID;
}G3D_MAPPEROBJ_DATA;

typedef struct {
	u32							arcID;	//アーカイブＩＤ
	const G3D_MAPPEROBJ_DATA*	data;	//実マップデータ
	u32							count;		//モデル数

}G3D_MAPPER_RESIST_OBJ;

typedef struct {
	u32			arcID;	//アーカイブＩＤ
	const u16*	data;	//実マップデータ
	u32			count;	//テクスチャ数

}G3D_MAPPER_RESIST_DDOBJ;

#define NON_GLOBAL_OBJ	(0xffffffff)
#define USE_GLOBAL_OBJSET_TBL	(0)
#define USE_GLOBAL_OBJSET_BIN	(1)
typedef struct {
	u32							objArcID;	//アーカイブＩＤ
	const G3D_MAPPEROBJ_DATA*	objData;	//実マップデータ
	u32							objCount;	//モデル数
	u32							ddobjArcID;	//アーカイブＩＤ
	const u16*					ddobjData;	//実マップデータ
	u32							ddobjCount;	//モデル数
}G3D_MAPPER_GLOBAL_OBJSET_TBL;

typedef struct {
	u32							areaObjArcID;		//配置種類アーカイブＩＤ
	u32							areaObjAnmTblArcID;	//配置種類アニメ付与データアーカイブＩＤ
	u32							areaObjDatID;		//配置種類データＩＤ
	u32							objArcID;			//モデルアーカイブＩＤ
	u32							objtexArcID;		//テクスチャアーカイブＩＤ
	u32							objanmArcID;		//アニメアーカイブＩＤ
}G3D_MAPPER_GLOBAL_OBJSET_BIN;

#define	NON_GLOBAL_TEX	(0xffffffff)
#define	USE_GLOBAL_TEX	(0)
typedef struct {
	u32 arcID;
	u32 datID;
}G3D_MAPPER_GLOBAL_TEXTURE;

typedef struct {
	u32						g3DmapFileType;	//g3Dmapファイル識別タイプ（仮）
	u16						sizex;			//横ブロック数
	u16						sizez;			//縦ブロック数
	u32						totalSize;		//配列サイズ
	fx32					width;			//ブロック１辺の幅
	fx32					height;			//ブロック高さ
	G3D_MAPPER_MODE			mode;			//動作モード
	u32						arcID;			//グラフィックアーカイブＩＤ
	u32						gtexType;		//グローバルテクスチャタイプ
	void*					gtexData;		//グローバルテクスチャ
	u32						gobjType;		//グローバルオブジェクトタイプ
	void*					gobjData;		//グローバルオブジェクト

	const G3D_MAPPER_DATA*	data;			//実マップデータ

}G3D_MAPPER_RESIST;

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム作成
 */
//------------------------------------------------------------------
extern G3D_MAPPER*	Create3Dmapper( HEAPID heapID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムメイン
 */
//------------------------------------------------------------------
extern void	Main3Dmapper( G3D_MAPPER* g3Dmapper );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
//------------------------------------------------------------------
extern void	Draw3Dmapper( G3D_MAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム破棄
 */
//------------------------------------------------------------------
extern void	Delete3Dmapper( G3D_MAPPER* g3Dmapper );


//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
extern void	ResistData3Dmapper( G3D_MAPPER* g3Dmapper, const G3D_MAPPER_RESIST* resistData );
extern void	ReleaseData3Dmapper( G3D_MAPPER* g3Dmapper );

//------------------------------------------------------------------
/**
 * @brief	マップ位置セット
 */
//------------------------------------------------------------------
extern void SetPos3Dmapper( G3D_MAPPER* g3Dmapper, const VecFx32* pos );


//------------------------------------------------------------------
/**
 * @brief	グリッド情報ワーク初期化
 */
//------------------------------------------------------------------
extern void InitGet3DmapperGridInfoData( G3D_MAPPER_INFODATA* gridInfoData );
extern void InitGet3DmapperGridInfo( G3D_MAPPER_GRIDINFO* gridInfo );
//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得
 */
//------------------------------------------------------------------
extern BOOL Get3DmapperGridInfoData
	( G3D_MAPPER* g3Dmapper, const VecFx32* pos, G3D_MAPPER_INFODATA* gridInfoData );
extern BOOL Get3DmapperGridInfo
	( G3D_MAPPER* g3Dmapper, const VecFx32* pos, G3D_MAPPER_GRIDINFO* gridInfo );

//------------------------------------------------------------------
/**
 * @brief	範囲外チェック
 */
//------------------------------------------------------------------
extern BOOL Check3DmapperOutRange( G3D_MAPPER* g3Dmapper, const VecFx32* pos );
//------------------------------------------------------------------
/**
 * @brief	サイズ取得
 */
//------------------------------------------------------------------
extern void Get3DmapperSize( G3D_MAPPER* g3Dmapper, fx32* x, fx32* z );


