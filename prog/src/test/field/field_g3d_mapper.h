#define MAP_BLOCK_COUNT		(9)

#define	MAPMDL_SIZE		(0x0f000)	//モデルデータ用メモリ確保サイズ 
#define	MAPTEX_SIZE		(0x4800) 	//テクスチャデータ用ＶＲＡＭ＆メモリ確保サイズ 
#define	MAPATTR_SIZE	(0x6000) 	//アトリビュート（高さ等）用メモリ確保サイズ 
//#define	MAPOBJ_SIZE		(0x18000)//ブロック内オブジェクトモデルデータ用メモリ確保サイズ 
//#define	MAPOBJTEX_SIZE	(0x4000) //ブロック内オブジェクトテクスチャデータ用ＶＲＡＭ確保サイズ 

typedef struct _FLD_G3D_MAPPER FLD_G3D_MAPPER;

typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}FLD_G3D_MAPPER_INFODATA;

typedef struct {
	FLD_G3D_MAPPER_INFODATA		gridData[MAP_BLOCK_COUNT];	//グリッドデータ取得ワーク
	u16						count;
}FLD_G3D_MAPPER_GRIDINFO;

#define	NON_ATTR	(0xffff)
typedef struct {
	u16 datID;
	u16 texID;
	u16 attrID;
}FLD_G3D_MAPPER_DATA;

typedef enum {
	FLD_G3D_MAPPER_MODE_SCROLL_NONE = 0,
	FLD_G3D_MAPPER_MODE_SCROLL_XZ,
	FLD_G3D_MAPPER_MODE_SCROLL_Y,
}FLD_G3D_MAPPER_MODE;

typedef struct {
	u16						sizex;		//横ブロック数
	u16						sizez;		//縦ブロック数
	u32						totalSize;	//配列サイズ
	fx32					width;		//ブロック１辺の幅
	fx32					height;		//ブロック高さ
	FLD_G3D_MAPPER_MODE			mode;		//動作モード
	u32						arcID;		//グラフィックアーカイブＩＤ
	const FLD_G3D_MAPPER_DATA*	data;		//実マップデータ

}FLD_G3D_MAPPER_RESIST;

#define	NON_LOWQ	(0xffff)
typedef struct {
	u16 highQ_ID;
	u16 lowQ_ID;
}FLD_G3D_MAPPEROBJ_DATA;

typedef struct {
	u32							arcID;	//アーカイブＩＤ
	const FLD_G3D_MAPPEROBJ_DATA*	data;	//実マップデータ
	u32							count;		//モデル数

}FLD_G3D_MAPPEROBJ_RESIST;

typedef struct {
	u32			arcID;	//アーカイブＩＤ
	const u16*	data;	//実マップデータ
	u32			count;	//テクスチャ数

}FLD_G3D_MAPPERDDOBJ_RESIST;

//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム作成
 */
//------------------------------------------------------------------
extern FLD_G3D_MAPPER*	FieldCreate3Dmapper( HEAPID heapID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムメイン
 */
//------------------------------------------------------------------
extern void	FieldMain3Dmapper( FLD_G3D_MAPPER* g3Dmapper );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステムディスプレイ
 */
//------------------------------------------------------------------
extern void	FieldDraw3Dmapper( FLD_G3D_MAPPER* g3Dmapper, GFL_G3D_CAMERA* g3Dcamera );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップコントロールシステム破棄
 */
//------------------------------------------------------------------
extern void	FieldDelete3Dmapper( FLD_G3D_MAPPER* g3Dmapper );


//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
extern void	FieldResistData3Dmapper( FLD_G3D_MAPPER* g3Dmapper, const FLD_G3D_MAPPER_RESIST* resistData );
//------------------------------------------------------------------
/**
 * @brief	オブジェクトリソース登録
 */
//------------------------------------------------------------------
extern void FieldResistObjRes3Dmapper( FLD_G3D_MAPPER* g3Dmapper, const FLD_G3D_MAPPEROBJ_RESIST* resistData );
extern void FieldReleaseObjRes3Dmapper( FLD_G3D_MAPPER* g3Dmapper );
//------------------------------------------------------------------
extern void FieldResistDDobjRes3Dmapper
			( FLD_G3D_MAPPER* g3Dmapper, const FLD_G3D_MAPPERDDOBJ_RESIST* resistData );
extern void FieldReleaseDDobjRes3Dmapper( FLD_G3D_MAPPER* g3Dmapper );

//------------------------------------------------------------------
/**
 * @brief	マップ位置セット
 */
//------------------------------------------------------------------
extern void FieldSetPos3Dmapper( FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos );


//------------------------------------------------------------------
/**
 * @brief	グリッド情報ワーク初期化
 */
//------------------------------------------------------------------
extern void FieldInitGet3DmapperGridInfoData( FLD_G3D_MAPPER_INFODATA* gridInfoData );
extern void FieldInitGet3DmapperGridInfo( FLD_G3D_MAPPER_GRIDINFO* gridInfo );
//------------------------------------------------------------------
/**
 * @brief	グリッド情報取得
 */
//------------------------------------------------------------------
extern BOOL FieldGet3DmapperGridInfoData
	( FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos, FLD_G3D_MAPPER_INFODATA* gridInfoData );
extern BOOL FieldGet3DmapperGridInfo
	( FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos, FLD_G3D_MAPPER_GRIDINFO* gridInfo );

//------------------------------------------------------------------
/**
 * @brief	範囲外チェック
 */
//------------------------------------------------------------------
extern BOOL FieldCheck3DmapperOutRange( FLD_G3D_MAPPER* g3Dmapper, const VecFx32* pos );
//------------------------------------------------------------------
/**
 * @brief	サイズ取得
 */
//------------------------------------------------------------------
extern void FieldGet3DmapperSize( FLD_G3D_MAPPER* g3Dmapper, fx32* x, fx32* z );


