#define MAP_BLOCK_COUNT		(9)

#define	MAPDATA_SIZE	(0x0f000)	//モデルデータ用メモリ確保サイズ 
#define	MAPTEX_SIZE		(0x4800) 	//テクスチャデータ用ＶＲＡＭ＆メモリ確保サイズ 
#define	MAPPLTT_SIZE	(0x200) 	//テクスチャパレット用ＶＲＡＭ確保サイズ 
#define	MAPATTR_SIZE	(0x6000) 	//アトリビュート（高さ等）用メモリ確保サイズ 
#define	MAPOBJ_SIZE		(0x18000) 	//ブロック内オブジェクトモデルデータ用メモリ確保サイズ 
#define	MAPOBJTEX_SIZE	(0x4000) 	//ブロック内オブジェクトテクスチャデータ用ＶＲＡＭ確保サイズ 

#define	MAPLOAD_SIZE	(0x800)		//分割データロードサイズ(ROM→RAM) 
#define	MAPTRANS_SIZE	(0x2000) 	//分割データ転送サイズ(RAM→VRAM) 

#define MAP_GRIDCOUNT	(32)		//マップ内縦横グリッド数

typedef struct _G3D_MAPPER G3D_MAPPER;

typedef struct {
	fx16	vecN1_x;
	fx16	vecN1_y;
	fx16	vecN1_z;

	fx16	vecN2_x;
	fx16	vecN2_y;
	fx16	vecN2_z;

	fx32	vecN1_D;
	fx32	vecN2_D;

	u32		attr:31;
	u32		tryangleType:1;
}NormalVtxSt;

typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}G3D_MAPPER_INFODATA;

typedef struct {
	G3D_MAPPER_INFODATA		gridData[MAP_BLOCK_COUNT];	//グリッドデータ取得ワーク
	u16						count;
}G3D_MAPPER_GRIDINFO;

#define	NON_ATTR	(0xffff)
typedef struct {
	u16 datID;
	u16 texID;
	u16 attrID;
}G3D_MAPPER_DATA;

typedef enum {
	G3D_MAPPER_MODE_SCROLL_NONE = 0,
	G3D_MAPPER_MODE_SCROLL_XZ,
	G3D_MAPPER_MODE_SCROLL_Y,
}G3D_MAPPER_MODE;

typedef struct {
	u16						sizex;		//横ブロック数
	u16						sizez;		//縦ブロック数
	u32						totalSize;	//配列サイズ
	fx32					width;		//ブロック１辺の幅
	fx32					height;		//ブロック高さ
	G3D_MAPPER_MODE			mode;		//動作モード
	u32						arcID;		//グラフィックアーカイブＩＤ
	const G3D_MAPPER_DATA*	data;		//実マップデータ

}G3D_MAPPER_RESIST;

#define	NON_LOWQ	(0xffff)
typedef struct {
	u16 highQ_ID;
	u16 lowQ_ID;
}G3D_MAPPEROBJ_DATA;

typedef struct {
	u32							arcID;	//アーカイブＩＤ
	const G3D_MAPPEROBJ_DATA*	data;	//実マップデータ
	u32							count;		//モデル数

}G3D_MAPPEROBJ_RESIST;

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
//------------------------------------------------------------------
/**
 * @brief	オブジェクトリソースセット
 */
//------------------------------------------------------------------
extern void ResistObjRes3Dmapper( G3D_MAPPER* g3Dmapper, const G3D_MAPPEROBJ_RESIST* resistData );
extern void ReleaseObjRes3Dmapper( G3D_MAPPER* g3Dmapper );

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


