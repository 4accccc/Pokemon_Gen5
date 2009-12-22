#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//#define	MAPOBJ_SIZE		(0x18000) 	//ブロック内オブジェクトモデルデータ用メモリ確保サイズ 
//#define	MAPOBJTEX_SIZE	(0x4000) 	//ブロック内オブジェクトテクスチャデータ用ＶＲＡＭ確保サイズ 
#define	GFL_G3D_MAP_MAPPLTT_SIZE	(0x200) 	//テクスチャパレット用ＶＲＡＭ確保サイズ(外部からサイズ指定できるように修正予定...) 
#define	GFL_G3D_MAP_MAPLOAD_SIZE	(0x800)		//分割データロードサイズ(ROM→RAM) 
#define	GFL_G3D_MAP_MAPTRANS_SIZE	(0x2000) 	//分割データ転送サイズ(RAM→VRAM) 


#define GFL_G3D_MAP_ATTR_GETMAX	(16)	//アトリビュート取得最大数
//------------------------------------------------------------------
//システムハンドル定義
typedef struct _GFL_G3D_MAP GFL_G3D_MAP;

//------------------------------------------------------------------
//描画タイプ定義
enum {
	GFL_G3D_MAP_DRAW_NORMAL = 0,
	GFL_G3D_MAP_DRAW_YBILLBOARD,
};
//------------------------------------------------------------------
//ライト使用定義
enum {
	GFL_G3D_MAP_LIGHT_NONE = 0,
	GFL_G3D_MAP_LIGHT0_ON = 1,
	GFL_G3D_MAP_LIGHT1_ON = 2,
	GFL_G3D_MAP_LIGHT2_ON = 4,
	GFL_G3D_MAP_LIGHT3_ON = 8,
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
	GFL_G3D_MAP_LOAD_IDLING = 0,
	GFL_G3D_MAP_LOAD_START,
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
	GFL_G3D_OBJ*	g3DobjHQ;	//オブジェクト(High Q)
	GFL_G3D_OBJ*	g3DobjLQ;	//オブジェクト(Low Q)
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
//グローバルオブジェクト管理設定定義
#define	GLOBALOBJ_SET_ERROR		(0xffffffff)
typedef struct {
	GFL_G3D_MAP_OBJ*	gobj;
	u32					gobjCount;
	u16*				gobjIDexchange;
	GFL_G3D_MAP_DDOBJ*	gddobj;
	u32					gddobjCount;
	u16*				gddobjIDexchange;
}GFL_G3D_MAP_GLOBALOBJ;

//------------------------------------------------------------------
//グローバルオブジェクト配置設定定義
typedef struct {
	u32			id;
	VecFx32		trans;
	u16			rotate;
}GFL_G3D_MAP_GLOBALOBJ_ST;

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
typedef BOOL (GFL_G3D_MAP_FILELOAD_FUNC)( GFL_G3D_MAP* g3Dmap, void * exWork );
typedef void (GFL_G3D_MAP_FILEATTR_FUNC)( GFL_G3D_MAP_ATTRINFO* attrInfo,
										const void* mapdata, const VecFx32* posInBlock, 
										const fx32 map_width, const fx32 map_height );
//------------------------------------------------------------------
//マップデータファイル処理関数テーブル定義
#define MAPFILE_FUNC_DEFAULT	(0xffffffff)	//テーブルの最後に記述設定しておくこと
typedef struct {
	u32							signature;
	GFL_G3D_MAP_FILELOAD_FUNC*	loadFunc; 
	GFL_G3D_MAP_FILEATTR_FUNC*	attrFunc; 
}GFL_G3D_MAP_FILE_FUNC;
//------------------------------------------------------------------
//セットアップ定義
typedef struct {
	u32								mapDataHeapSize;
	u32								texVramSize;
	const GFL_G3D_MAP_FILE_FUNC*	mapFileFunc;
	void *						externalWork;
  u32               mapLoadSize;

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
extern GFL_G3D_MAP*	GFL_G3D_MAP_Create( const GFL_G3D_MAP_SETUP* setup, HEAPID heapID );
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
extern void	GFL_G3D_MAP_Draw( GFL_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera );
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
extern void	GFL_G3D_MAP_ResistGlobalTexResource
					( GFL_G3D_MAP* g3Dmap, GFL_G3D_RES* globalResTex );
extern void	GFL_G3D_MAP_ReleaseGlobalTexResource( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルオブジェクトリソース登録
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_ResistGlobalObjResource
					( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ* globalObj );
extern void	GFL_G3D_MAP_ReleaseGlobalObjResource( GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルオブジェクト配置登録
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_ResistGlobalObj
					( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status, u32 idx );
extern u32	GFL_G3D_MAP_ResistAutoGlobalObj
					( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status );
extern void	GFL_G3D_MAP_ReleaseGlobalObj( GFL_G3D_MAP* g3Dmap, u32 idx );
extern GFL_G3D_MAP_GLOBALOBJ_ST * GFL_G3D_MAP_GetGlobalObj
          ( GFL_G3D_MAP* g3Dmap, u32 idx );

//------------------------------------------------------------------
extern void	GFL_G3D_MAP_ResistGlobalDDobj
					( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status, u32 idx );
extern u32	GFL_G3D_MAP_ResistAutoGlobalDDobj
					( GFL_G3D_MAP* g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST* status );
extern void	GFL_G3D_MAP_ReleaseGlobalDDobj( GFL_G3D_MAP* g3Dmap, u32 idx );
//------------------------------------------------------------------
/**
 * @brief	３ＤマップグローバルオブジェクトＩＤ変換
 */
//------------------------------------------------------------------
extern BOOL	GFL_G3D_MAP_GetGlobalObjectID
					( GFL_G3D_MAP* g3Dmap, const u32 datID, u32* exchangeID );
extern BOOL	GFL_G3D_MAP_GetGlobalDDobjectID
					( GFL_G3D_MAP* g3Dmap, const u32 datID, u32* exchangeID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
extern void	GFL_G3D_MAP_SetLoadReq( GFL_G3D_MAP* g3Dmap, const u32 datID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロード停止
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_ResetLoadData( GFL_G3D_MAP* g3Dmap );


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
extern void GFL_G3D_MAP_ResistFileType( GFL_G3D_MAP* g3Dmap, u32 fileType );


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
extern const GFL_G3D_RES* GFL_G3D_MAP_GetResourceMdl( const GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	テクスチャリソース設定
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_CreateResourceTex( GFL_G3D_MAP* g3Dmap, void* mem );
extern void GFL_G3D_MAP_DeleteResourceTex( GFL_G3D_MAP* g3Dmap );
extern const GFL_G3D_RES* GFL_G3D_MAP_GetResourceTex( const GFL_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	レンダー作成
 */
//------------------------------------------------------------------
extern void GFL_G3D_MAP_MakeRenderObj( GFL_G3D_MAP* g3Dmap );
extern NNSG3dRenderObj* GFL_G3D_MAP_GetRenderObj( GFL_G3D_MAP* g3Dmap );
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

#ifdef __cplusplus
}/* extern "C" */
#endif

