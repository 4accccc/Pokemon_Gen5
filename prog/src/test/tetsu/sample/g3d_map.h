#define	MAPPLTT_SIZE	(0x200) 	//テクスチャパレット用ＶＲＡＭ確保サイズ 
//#define	MAPOBJ_SIZE		(0x18000) 	//ブロック内オブジェクトモデルデータ用メモリ確保サイズ 
//#define	MAPOBJTEX_SIZE	(0x4000) 	//ブロック内オブジェクトテクスチャデータ用ＶＲＡＭ確保サイズ 
#define	MAPLOAD_SIZE	(0x800)		//分割データロードサイズ(ROM→RAM) 
#define	MAPTRANS_SIZE	(0x2000) 	//分割データ転送サイズ(RAM→VRAM) 

#define MAP_GRIDCOUNT	(32)		//マップ内縦横グリッド数

#define DRAW_LIMIT	(1024 * FX32_ONE)		//描画範囲
#define LOD_LIMIT	(512 * FX32_ONE)		//HighQuallity(LOD)用描画範囲

#define	NON_TEX		(0xffff)
#define	NON_ATTR	(0xffff)

#define GFL_G3D_MAP_ATTR_GETMAX	(16)	//アトリビュート取得最大数
//------------------------------------------------------------------
//システムハンドル定義
typedef struct _GFL_G3D_MAP GFL_G3D_MAP;

//------------------------------------------------------------------
//ファイルタイプ定義
typedef enum {
	FILE_MAPEDITER_DATA = 0,
	FILE_CUSTOM_DATA = 1,
}GFL_G3D_MAPDATA_FILETYPE;
//------------------------------------------------------------------
//描画タイプ定義
enum {
	DRAW_NORMAL = 0,
	DRAW_YBILLBOARD,
};
//------------------------------------------------------------------
//ライト使用定義
enum {
	LIGHT_NONE = 0,
	LIGHT0_ON = 1,
	LIGHT1_ON = 2,
	LIGHT2_ON = 4,
	LIGHT3_ON = 8,
};
//------------------------------------------------------------------
//読み込みステータス定義
typedef struct {
	u16			seq;
	u16			loadCount;
	u32			mOffs;
	u32			fOffs;
	u32			fSize;
	u32			tOffs;
	u32			tSize;

	BOOL		mdlLoaded;
	BOOL		texLoaded;
	BOOL		attrLoaded;
}GFL_G3D_MAP_LOAD_STATUS;
//------------------------------------------------------------------
//読み込みシーケンス定義
enum {
	LOAD_IDLING = 0,
	LOAD_START,
};
//------------------------------------------------------------------
//ジオメトリ直接書き込みオブジェクト関数定義
typedef void (GFL_G3D_MAP_DDOBJ_FUNC)
					( u32 texDataAdrs, u32 texPlttAdrs, VecFx16* vecView, BOOL lodSt );
//------------------------------------------------------------------
//ジオメトリ直接書き込みオブジェクトパラメータ定義
typedef struct {
	u8						lightMask;
	GXRgb					diffuse;
    GXRgb					ambient;
	GXRgb					specular;
    GXRgb					emission;
	u8						polID;
	u8						alpha;
	u8						drawType;	
	int						scaleVal;
	GFL_G3D_MAP_DDOBJ_FUNC*	func;
}GFL_G3D_MAP_DDOBJ_DATA;
//------------------------------------------------------------------
//グローバルオブジェクト設定定義
typedef struct {
	NNSG3dRenderObj*	NNSrnd_H;	//レンダー(High Q)
	GFL_G3D_RES*		g3Dres_H;	//リソース(High Q)
	NNSG3dRenderObj*	NNSrnd_L;	//レンダー(Low Q)
	GFL_G3D_RES*		g3Dres_L;	//リソース(High Q)
}GFL_G3D_MAP_OBJ;
//------------------------------------------------------------------
//ジオメトリ直接書き込みオブジェクト設定定義
typedef struct {
	GFL_G3D_RES*			g3Dres;
	u32						texDataAdrs;
	u32						texPlttAdrs;
	const GFL_G3D_MAP_DDOBJ_DATA*	data;
}GFL_G3D_MAP_DDOBJ;
//------------------------------------------------------------------
//アトリビュート定義
typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}GFL_G3D_MAP_ATTR;

//アトリビュート情報定義
typedef struct {
	GFL_G3D_MAP_ATTR	mapAttr[GFL_G3D_MAP_ATTR_GETMAX];
	u32					mapAttrCount;
}GFL_G3D_MAP_ATTRINFO;

//------------------------------------------------------------------
//マップデータファイル処理関数定義
typedef BOOL (GFL_G3D_MAP_FILELOAD_FUNC)( GFL_G3D_MAP* g3Dmap );
typedef void (GFL_G3D_MAP_FILEATTR_FUNC)( GFL_G3D_MAP_ATTRINFO* attrInfo,
										const void* mapdata, const VecFx32* posInBlock, 
										const fx32 map_width, const fx32 map_height );
//------------------------------------------------------------------
//マップデータファイル処理関数テーブル定義
typedef struct {
	GFL_G3D_MAP_FILELOAD_FUNC*	loadFunc; 
	GFL_G3D_MAP_FILEATTR_FUNC*	attrFunc; 
}MAPFILE_FUNC;
//------------------------------------------------------------------
//セットアップ定義
typedef struct {
	u32					mapDataHeapSize;
	u32					texVramSize;
	const MAPFILE_FUNC*	mapFileFunc;

}GFL_G3D_MAP_SETUP;

//============================================================================================
/**
 * @brief	基本システム
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップシステム作成
 */
//------------------------------------------------------------------
extern GFL_G3D_MAP*	GFL_G3D_MAP_Create( GFL_G3D_MAP_SETUP* setup, HEAPID heapID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップシステム破棄
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_Delete( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップメイン
 *				※データ読み込み
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_Main( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ描画
 *
 *		ex)
 *		{
 *			GFL_G3D_MAP_StartDraw();	//描画開始
 *
 *			for( i=0; i<g3Dmap_count; i++ ){
 *				GFL_G3D_MAP_Draw( ...... );		//各マップ描画関数
 *			
 *			GFL_G3D_MAP_EndDraw();		//描画終了
 *		}
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_StartDraw( void );
extern void	GFL_G3D_MAP_Draw( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera,
								const GFL_G3D_MAP_OBJ* obj, const GFL_G3D_MAP_DDOBJ* ddobj );
extern void	GFL_G3D_MAP_EndDraw( void );


//============================================================================================
/**
 * @brief	マップコントロール関数
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップアーカイブデータ登録
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_ResistArc( GFL_G3D_MAP* g3Dmap, const u32 arcID, HEAPID heapID );
extern void	GFL_G3D_MAP_ReleaseArc( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルテクスチャリソース登録
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_ResistGrobalTex( GFL_G3D_MAP* g3Dmap, GFL_G3D_RES* grobalResTex );
extern void	GFL_G3D_MAP_ReleaseGrobalTex( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_SetLoadReq( GFL_G3D_MAP* g3Dmap, const u32 datID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ描画ＯＮ／ＯＦＦ＆取得
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_SetDrawSw( GFL_G3D_MAP* g3Dmap, BOOL drawSw );
extern BOOL	GFL_G3D_MAP_GetDrawSw( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ位置設定＆取得
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_SetTrans( GFL_G3D_MAP* g3Dmap, const VecFx32* trans );
extern void	GFL_G3D_MAP_GetTrans( GFL_G3D_MAP* g3Dmap, VecFx32* trans );
//------------------------------------------------------------------
/**
 * @brief	アトリビュート取得
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_InitAttr( GFL_G3D_MAP_ATTRINFO* attrInfo );
extern void GFL_G3D_MAP_GetAttr( GFL_G3D_MAP_ATTRINFO* attrInfo,
								GFL_G3D_MAP* g3Dmap, const VecFx32* pos, const fx32 map_width );
//------------------------------------------------------------------
/**
 * @brief	ファイル識別設定（仮）※いずれはデータファイルの中に識別を埋め込む
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_ResistFileType( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAPDATA_FILETYPE fileType );


//============================================================================================
/**
 * @brief	マップデータファイル処理用関数
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ロードステータスポインタ取得
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_GetLoadStatusPointer( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_LOAD_STATUS** ldst );
//------------------------------------------------------------------
/**
 * @brief	ロードステータス初期化
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_ResetLoadStatus( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	ロードメモリポインタ取得
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_GetLoadMemoryPointer( GFL_G3D_MAP* g3Dmap, void** mem );
//------------------------------------------------------------------
/**
 * @brief	アーカイブハンドル取得
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_GetLoadArcHandle( GFL_G3D_MAP* g3Dmap, ARCHANDLE** handle );
//------------------------------------------------------------------
/**
 * @brief	アーカイブ＆ロードデータＩＤ取得
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_GetLoadDatID( GFL_G3D_MAP* g3Dmap, u32* ID );
//------------------------------------------------------------------
/**
 * @brief	モデルリソース設定
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_CreateResourceMdl( GFL_G3D_MAP* g3Dmap, void* mem );
extern void GFL_G3D_MAP_DeleteResourceMdl( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	テクスチャリソース設定
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_CreateResourceTex( GFL_G3D_MAP* g3Dmap, void* mem );
extern void GFL_G3D_MAP_DeleteResourceTex( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	レンダー作成
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_MakeRenderObj( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	ファイル読み込み関数
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_StartFileLoad( GFL_G3D_MAP* g3Dmap, u32 datID );
extern BOOL GFL_G3D_MAP_ContinueFileLoad( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	ローカルテクスチャVRAM転送関数(テクスチャ読み込み終了後に行う)
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_SetTransVramParam( GFL_G3D_MAP* g3Dmap );
extern BOOL GFL_G3D_MAP_TransVram( GFL_G3D_MAP* g3Dmap );


//============================================================================================
/**
 * @brief	テスト関数
 */
//============================================================================================
extern void GFL_G3D_MAP_MakeTestPos( GFL_G3D_MAP* g3Dmap );

