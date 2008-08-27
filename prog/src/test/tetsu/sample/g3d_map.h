#define	MAPPLTT_SIZE	(0x200) 	//テクスチャパレット用ＶＲＡＭ確保サイズ 
#define	MAPOBJ_SIZE		(0x18000) 	//ブロック内オブジェクトモデルデータ用メモリ確保サイズ 
#define	MAPOBJTEX_SIZE	(0x4000) 	//ブロック内オブジェクトテクスチャデータ用ＶＲＡＭ確保サイズ 

#define	MAPLOAD_SIZE	(0x800)		//分割データロードサイズ(ROM→RAM) 
#define	MAPTRANS_SIZE	(0x2000) 	//分割データ転送サイズ(RAM→VRAM) 

#define MAP_GRIDCOUNT	(32)		//マップ内縦横グリッド数

#define DRAW_LIMIT	(1024 * FX32_ONE)		//描画範囲
#define LOD_LIMIT	(512 * FX32_ONE)		//HighQuallity(LOD)用描画範囲

typedef struct _GFL_G3D_MAP GFL_G3D_MAP;
typedef void (GFL_G3D_MAP_DDOBJ_FUNC)
					( u32 texDataAdrs, u32 texPlttAdrs, VecFx16* vecView, BOOL lodSt );

#define	NON_TEX		(0xffff)
#define	NON_ATTR	(0xffff)

enum {
	DRAW_NORMAL = 0,
	DRAW_YBILLBOARD,
};
enum {
	LIGHT_NONE = 0,
	LIGHT0_ON = 1,
	LIGHT1_ON = 2,
	LIGHT2_ON = 4,
	LIGHT3_ON = 8,
};

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


typedef struct {
	NNSG3dRenderObj*	NNSrnd_H;	//レンダー(High Q)
	GFL_G3D_RES*		g3Dres_H;	//リソース(High Q)
	NNSG3dRenderObj*	NNSrnd_L;	//レンダー(Low Q)
	GFL_G3D_RES*		g3Dres_L;	//リソース(High Q)
}GFL_G3D_MAP_OBJ;

typedef struct {
	GFL_G3D_RES*			g3Dres;
	u32						texDataAdrs;
	u32						texPlttAdrs;
	const GFL_G3D_MAP_DDOBJ_DATA*	data;
}GFL_G3D_MAP_DDOBJ;

typedef struct {
	VecFx16 vecN;
	u32		attr;
	fx32	height;
}GFL_G3D_MAP_ATTRINFO;

typedef struct {
	u32	mapDataHeapSize;
	u32	texVramSize;

}GFL_G3D_MAP_SETUP;

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
extern void	GFL_G3D_MAP_SetLoadReq( GFL_G3D_MAP* g3Dmap,
								const u32 datID, const u32 texID, const u32 attrID );
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
extern void GFL_G3D_MAP_GetAttr( GFL_G3D_MAP_ATTRINFO* attrInfo, const GFL_G3D_MAP* g3Dmap,
									const VecFx32* pos, const fx32 map_width );

