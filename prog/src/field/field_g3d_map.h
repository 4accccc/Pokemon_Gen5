//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_g3d_map.h
 *	@brief  g3d_map　WBバージョン
 *	@author	game freak inc.
 *	@date		2010.03.01
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//#define	MAPOBJ_SIZE		(0x18000) 	//ブロック内オブジェクトモデルデータ用メモリ確保サイズ 
//#define	MAPOBJTEX_SIZE	(0x4000) 	//ブロック内オブジェクトテクスチャデータ用ＶＲＡＭ確保サイズ 
#define	FLD_G3D_MAP_MAPPLTT_SIZE	(0x200) 	//テクスチャパレット用ＶＲＡＭ確保サイズ(外部からサイズ指定できるように修正予定...) 
#define	FLD_G3D_MAP_MAPLOAD_SIZE	(0x800)		//分割データロードサイズ(ROM→RAM) 
#define	FLD_G3D_MAP_MAPTRANS_SIZE	(0x2000) 	//分割データ転送サイズ(RAM→VRAM) 


#define FLD_G3D_MAP_ATTR_GETMAX	(16)	//アトリビュート取得最大数

//------------------------------------------------------------------
//システムハンドル定義
typedef struct _FLD_G3D_MAP FLD_G3D_MAP;

//------------------------------------------------------------------
//描画タイプ定義
enum {
	FLD_G3D_MAP_DRAW_NORMAL = 0,
	FLD_G3D_MAP_DRAW_YBILLBOARD,
};
//------------------------------------------------------------------
//ライト使用定義
enum {
	FLD_G3D_MAP_LIGHT_NONE = 0,
	FLD_G3D_MAP_LIGHT0_ON = 1,
	FLD_G3D_MAP_LIGHT1_ON = 2,
	FLD_G3D_MAP_LIGHT2_ON = 4,
	FLD_G3D_MAP_LIGHT3_ON = 8,
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
}FLD_G3D_MAP_LOAD_STATUS;
//------------------------------------------------------------------
//読み込みシーケンス定義
enum {
	FLD_G3D_MAP_LOAD_IDLING = 0,
	FLD_G3D_MAP_LOAD_START,
};
//------------------------------------------------------------------
//ジオメトリ直接書き込みオブジェクト関数定義
typedef void (FLD_G3D_MAP_DDOBJ_FUNC)
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
	FLD_G3D_MAP_DDOBJ_FUNC*	func;
}FLD_G3D_MAP_DDOBJ_DATA;
//------------------------------------------------------------------
//グローバルオブジェクト設定定義
typedef struct {
	GFL_G3D_OBJ*	g3DobjHQ;	//オブジェクト(High Q)
	GFL_G3D_OBJ*	g3DobjLQ;	//オブジェクト(Low Q)
}FLD_G3D_MAP_OBJ;
//------------------------------------------------------------------
//ジオメトリ直接書き込みオブジェクト設定定義
typedef struct {
	GFL_G3D_RES*			g3Dres;
	u32						texDataAdrs;
	u32						texPlttAdrs;
	const FLD_G3D_MAP_DDOBJ_DATA*	data;
}FLD_G3D_MAP_DDOBJ;
//------------------------------------------------------------------
//グローバルオブジェクト管理設定定義
#define	GLOBALOBJ_SET_ERROR		(0xffffffff)
typedef struct {
	FLD_G3D_MAP_OBJ*	gobj;
	u32					gobjCount;
	u16*				gobjIDexchange;
	FLD_G3D_MAP_DDOBJ*	gddobj;
	u32					gddobjCount;
	u16*				gddobjIDexchange;
}FLD_G3D_MAP_GLOBALOBJ;

//------------------------------------------------------------------
//グローバルオブジェクト配置設定定義
typedef struct {
	u32			id;
	VecFx32		trans;
	u16			rotate;
}FLD_G3D_MAP_GLOBALOBJ_ST;

//------------------------------------------------------------------
//アトリビュート定義
typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}FLD_G3D_MAP_ATTR;

//アトリビュート情報定義
typedef struct {
	FLD_G3D_MAP_ATTR	mapAttr[FLD_G3D_MAP_ATTR_GETMAX];
	u32					mapAttrCount;
}FLD_G3D_MAP_ATTRINFO;

//------------------------------------------------------------------
//マップデータファイル処理関数定義
typedef BOOL (FLD_G3D_MAP_FILELOAD_FUNC)( FLD_G3D_MAP* g3Dmap, void * exWork );
typedef void (FLD_G3D_MAP_FILEATTR_FUNC)( FLD_G3D_MAP_ATTRINFO* attrInfo,
										const void* mapdata, const VecFx32* posInBlock, 
										const fx32 map_width, const fx32 map_height );
//------------------------------------------------------------------
//マップデータファイル処理関数テーブル定義
#define MAPFILE_FUNC_DEFAULT	(0xffffffff)	//テーブルの最後に記述設定しておくこと
typedef struct {
	u32							signature;
	FLD_G3D_MAP_FILELOAD_FUNC*	loadFunc; 
	FLD_G3D_MAP_FILEATTR_FUNC*	attrFunc; 
}FLD_G3D_MAP_FILE_FUNC;
//------------------------------------------------------------------
//セットアップ定義
typedef struct {
	u32								mapDataHeapSize;
	u32								texVramSize;
	const FLD_G3D_MAP_FILE_FUNC*	mapFileFunc;
	void *						externalWork;
  u32               mapLoadSize;

  u16               obj_count;      // 配置オブジェ管理数
  u16               ddobj_count;    // DirectDraw配置オブジェ管理数
  fx32              obj_draw_limit; // 描画範囲
  fx32              obj_lod_limit;  // HighQuallity(LOD)用描画範囲
  
}FLD_G3D_MAP_SETUP;

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
extern FLD_G3D_MAP*	FLD_G3D_MAP_Create( const FLD_G3D_MAP_SETUP* setup, HEAPID heapID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップシステム破棄
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_Delete( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップメイン
 *				※データ読み込み
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_Main( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ描画
 *
 *		ex)
 *		{
 *			FLD_G3D_MAP_StartDraw();	//描画開始
 *
 *			for( i=0; i<g3Dmap_count; i++ ){
 *				FLD_G3D_MAP_Draw( ...... );		//各マップ描画関数
 *			
 *			FLD_G3D_MAP_EndDraw();		//描画終了
 *		}
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_StartDraw( void );
extern void	FLD_G3D_MAP_Draw( FLD_G3D_MAP* g3Dmap, GFL_G3D_CAMERA* g3Dcamera );
extern void	FLD_G3D_MAP_EndDraw( void );


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
extern void	FLD_G3D_MAP_ResistArc( FLD_G3D_MAP* g3Dmap, const u32 arcID, HEAPID heapID );
extern void	FLD_G3D_MAP_ReleaseArc( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルテクスチャリソース登録
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_ResistGlobalTexResource
					( FLD_G3D_MAP* g3Dmap, GFL_G3D_RES* globalResTex );
extern void	FLD_G3D_MAP_ReleaseGlobalTexResource( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルオブジェクトリソース登録
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_ResistGlobalObjResource
					( FLD_G3D_MAP* g3Dmap, FLD_G3D_MAP_GLOBALOBJ* globalObj );
extern void	FLD_G3D_MAP_ReleaseGlobalObjResource( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップグローバルオブジェクト配置登録
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_ResistGlobalObj
					( FLD_G3D_MAP* g3Dmap, FLD_G3D_MAP_GLOBALOBJ_ST* status, u32 idx );
extern u32	FLD_G3D_MAP_ResistAutoGlobalObj
					( FLD_G3D_MAP* g3Dmap, FLD_G3D_MAP_GLOBALOBJ_ST* status );
extern void	FLD_G3D_MAP_ReleaseGlobalObj( FLD_G3D_MAP* g3Dmap, u32 idx );
extern FLD_G3D_MAP_GLOBALOBJ_ST * FLD_G3D_MAP_GetGlobalObj
          ( FLD_G3D_MAP* g3Dmap, u32 idx );

//------------------------------------------------------------------
extern void	FLD_G3D_MAP_ResistGlobalDDobj
					( FLD_G3D_MAP* g3Dmap, FLD_G3D_MAP_GLOBALOBJ_ST* status, u32 idx );
extern u32	FLD_G3D_MAP_ResistAutoGlobalDDobj
					( FLD_G3D_MAP* g3Dmap, FLD_G3D_MAP_GLOBALOBJ_ST* status );
extern void	FLD_G3D_MAP_ReleaseGlobalDDobj( FLD_G3D_MAP* g3Dmap, u32 idx );
//------------------------------------------------------------------
/**
 * @brief	３ＤマップグローバルオブジェクトＩＤ変換
 */
//------------------------------------------------------------------
extern BOOL	FLD_G3D_MAP_GetGlobalObjectID
					( FLD_G3D_MAP* g3Dmap, const u32 datID, u32* exchangeID );
extern BOOL	FLD_G3D_MAP_GetGlobalDDobjectID
					( FLD_G3D_MAP* g3Dmap, const u32 datID, u32* exchangeID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロードリクエスト設定
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_SetLoadReq( FLD_G3D_MAP* g3Dmap, const u32 datID );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップロード停止
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_ResetLoadData( FLD_G3D_MAP* g3Dmap );


//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ描画ＯＮ／ＯＦＦ＆取得
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_SetDrawSw( FLD_G3D_MAP* g3Dmap, BOOL drawSw );
extern BOOL	FLD_G3D_MAP_GetDrawSw( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	３Ｄマップ位置設定＆取得
 */
//------------------------------------------------------------------
extern void	FLD_G3D_MAP_SetTrans( FLD_G3D_MAP* g3Dmap, const VecFx32* trans );
extern void	FLD_G3D_MAP_GetTrans( FLD_G3D_MAP* g3Dmap, VecFx32* trans );
//------------------------------------------------------------------
/**
 * @brief	アトリビュート取得
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_InitAttr( FLD_G3D_MAP_ATTRINFO* attrInfo );
extern void FLD_G3D_MAP_GetAttr( FLD_G3D_MAP_ATTRINFO* attrInfo,
								FLD_G3D_MAP* g3Dmap, const VecFx32* pos, const fx32 map_width );
//------------------------------------------------------------------
/**
 * @brief	ファイル識別設定（仮）※いずれはデータファイルの中に識別を埋め込む
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_ResistFileType( FLD_G3D_MAP* g3Dmap, u32 fileType );


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
extern void FLD_G3D_MAP_GetLoadStatusPointer( FLD_G3D_MAP* g3Dmap, FLD_G3D_MAP_LOAD_STATUS** ldst );
//------------------------------------------------------------------
/**
 * @brief	ロードステータス初期化
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_ResetLoadStatus( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	ロードメモリポインタ取得
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_GetLoadMemoryPointer( FLD_G3D_MAP* g3Dmap, void** mem );
//------------------------------------------------------------------
/**
 * @brief	アーカイブハンドル取得
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_GetLoadArcHandle( FLD_G3D_MAP* g3Dmap, ARCHANDLE** handle );
//------------------------------------------------------------------
/**
 * @brief	アーカイブ＆ロードデータＩＤ取得
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_GetLoadDatID( FLD_G3D_MAP* g3Dmap, u32* ID );
//------------------------------------------------------------------
/**
 * @brief	モデルリソース設定
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_CreateResourceMdl( FLD_G3D_MAP* g3Dmap, void* mem );
extern void FLD_G3D_MAP_DeleteResourceMdl( FLD_G3D_MAP* g3Dmap );
extern const GFL_G3D_RES* FLD_G3D_MAP_GetResourceMdl( const FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	テクスチャリソース設定
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_CreateResourceTex( FLD_G3D_MAP* g3Dmap, void* mem );
extern void FLD_G3D_MAP_DeleteResourceTex( FLD_G3D_MAP* g3Dmap );
extern const GFL_G3D_RES* FLD_G3D_MAP_GetResourceTex( const FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	レンダー作成
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_MakeRenderObj( FLD_G3D_MAP* g3Dmap );
extern NNSG3dRenderObj* FLD_G3D_MAP_GetRenderObj( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	ファイル読み込み関数
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_StartFileLoad( FLD_G3D_MAP* g3Dmap, u32 datID );
extern BOOL FLD_G3D_MAP_ContinueFileLoad( FLD_G3D_MAP* g3Dmap );
//------------------------------------------------------------------
/**
 * @brief	ローカルテクスチャVRAM転送関数(テクスチャ読み込み終了後に行う)
 */
//------------------------------------------------------------------
extern void FLD_G3D_MAP_SetTransVramParam( FLD_G3D_MAP* g3Dmap );
extern BOOL FLD_G3D_MAP_TransVram( FLD_G3D_MAP* g3Dmap );


//============================================================================================
/**
 * @brief	テスト関数
 */
//============================================================================================
extern void FLD_G3D_MAP_MakeTestPos( FLD_G3D_MAP* g3Dmap );

#ifdef __cplusplus
}/* extern "C" */
#endif

