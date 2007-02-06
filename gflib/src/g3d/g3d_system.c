//=============================================================================================
/**
 * @file	g3d_system.c                                                  
 * @brief	３Ｄ描画管理システムプログラム
 * @date	2006/4/26
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================
#define TEX_BLOCKNUM	(128)
#define PLT_BLOCKNUM	(256)

#define TEX_SLOTSIZ		(0x20000)
#define PLT_SLOTSIZ		(0x2000)

//=============================================================================================
//	型宣言
//=============================================================================================
//	グローバルステート構造体（射影）
typedef struct {
	GFL_G3D_PROJECTION_TYPE	type;	///<射影行列タイプ
	fx32		param1;				///<パラメータ１（タイプによって使用法は異なる）
	fx32		param2;				///<パラメータ２（タイプによって使用法は異なる）
	fx32		param3;				///<パラメータ３（タイプによって使用法は異なる）
	fx32		param4;				///<パラメータ４（タイプによって使用法は異なる）
	fx32		near;				///<視点からnearクリップ面までの距離
	fx32		far;				///<視点からfarクリップ面までの距離
	fx32		scaleW;				///<ビューボリュームの精度調整パラメータ（使用しないときは0）
}GFL_G3D_PROJECTION;

//	グローバルステート構造体（ライト）
typedef struct {
	VecFx16		vec;				///<ライト方向
	u16			color;				///<色
}GFL_G3D_LIGHT;

//	グローバルステート構造体（カメラ）
typedef struct {
	VecFx32		camPos;		///<カメラの位置(＝視点)
	VecFx32		camUp;		///<カメラの上方向
	VecFx32		target;		///<カメラの焦点(＝注視点)
} GFL_G3D_LOOKAT;

//	グローバルステート構造体（レンダリングバッファ）
typedef struct {
	GXSortMode		aw;
	GXBufferMode	zw;
} GFL_G3D_SWAPBUFMODE;

/**
 * @brief	３Ｄシステムマネージャ構造体
 */
typedef struct GFL_G3D_MAN_tag
{
	NNSFndAllocator		allocater;			///<メモリアロケータ(NNSで使用)
	void*				plt_memory;			///<パレットマネージャ用メモリ
	void*				tex_memory;			///<テクスチャマネージャ用メモリ
	HEAPID				heapID;				///<ヒープＩＤ

	GFL_G3D_PROJECTION	projection;			///<グローバルステート（射影）
	GFL_G3D_LIGHT		light[4];			///<グローバルステート（ライト）
	GFL_G3D_LOOKAT		lookAt;				///<グローバルステート（カメラ）
	GFL_G3D_SWAPBUFMODE	swapBufMode;		///<グローバルステート（レンダリングバッファ）

}GFL_G3D_MAN;

static GFL_G3D_MAN*  g3Dman = NULL;

static inline BOOL G3DMAN_CHECK( void ){
	if( g3Dman == NULL ){
		return FALSE;
	}
	return TRUE;
}

//=============================================================================================
/**
 *
 *
 * システムマネージャ関数
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * システムマネージャ設定
 *
 * @param	texmanMode		テクスチャマネージャ使用モード
 * @param	texmanSize		テクスチャマネージャ使用領域サイズ
 * @param	palmanMode		パレットマネージャ使用モード
 * @param	palmanSize		パレットマネージャ使用領域サイズ
 * @param	GeBufEnable		ジオメトリバッファの使用サイズ(0:使用しない)
 * @param	heapID			メモリ取得用ヒープエリア
 * @param	setUp			セットアップ関数(NULLの時はDefaultSetUp)
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysInit
		( GFL_G3D_VMAN_MODE texmanMode, GFL_G3D_VMAN_TEXSIZE texmanSize, 
			GFL_G3D_VMAN_MODE pltmanMode, GFL_G3D_VMAN_PLTSIZE pltmanSize,
				u16 GeBufSize, HEAPID heapID, GFL_G3D_SETUP_FUNC setup )
{
	int	tex_size,plt_size;
	g3Dman = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_MAN) );

	g3Dman->heapID = heapID;

	// NitroSystem:３Ｄエンジンの初期化
	NNS_G3dInit();
	// マトリクススタックの初期化
    G3X_InitMtxStack();
	// グローバルステート初期化
	NNS_G3dGlbInit();

	// ジオメトリコマンドバッファの設定
	if( GeBufSize ){
		NNSG3dGeBuffer* geBuf;

		if( GeBufSize > GEBUF_SIZE_MAX ){
			GeBufSize = GEBUF_SIZE_MAX;
		}
		geBuf = ( NNSG3dGeBuffer* )GFL_HEAP_DTCM_AllocMemory( GeBufSize );
		NNS_G3dGeSetBuffer( geBuf );
	}
	// ジオメトリエンジン起動後必ず呼ばれなければならない
    G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );

	//ＮＮＳ関数で使用するアロケータのセットアップ
	GFL_HEAP_InitAllocator( &g3Dman->allocater, g3Dman->heapID, 4 ); 

	//ＶＲＡＭテクスチャマネージャの設定
	if( texmanMode == GFL_G3D_VMANLNK ){
		//リンクドモードの設定
		tex_size = NNS_GfdGetLnkTexVramManagerWorkSize( TEX_BLOCKNUM * texmanSize );
		g3Dman->tex_memory = GFL_HEAP_AllocMemory( g3Dman->heapID, tex_size );

		// マネージャがテクスチャイメージスロットを指定スロット分管理できるようにする
		NNS_GfdInitLnkTexVramManager
			( TEX_SLOTSIZ*texmanSize, 0, g3Dman->tex_memory, tex_size, TRUE);
	} else {
		//フレームモードの設定
		NNS_GfdInitFrmTexVramManager( texmanSize, TRUE);
		g3Dman->tex_memory = NULL;
	}

	//ＶＲＡＭパレットマネージャの設定
	if( pltmanMode == GFL_G3D_VMANLNK ){
		//リンクドモードの設定
		plt_size = NNS_GfdGetLnkPlttVramManagerWorkSize( PLT_BLOCKNUM * pltmanSize );
		g3Dman->plt_memory = GFL_HEAP_AllocMemory( g3Dman->heapID, plt_size );

		// マネージャがテクスチャイメージスロットを指定サイズ分管理できるようにする
		NNS_GfdInitLnkPlttVramManager
			( PLT_SLOTSIZ * pltmanSize, g3Dman->plt_memory, plt_size, TRUE);
	} else {
		//フレームモードの設定
		NNS_GfdInitFrmTexVramManager( PLT_SLOTSIZ * pltmanSize, TRUE);
		g3Dman->tex_memory = NULL;
	}

	//グローバルステート内部情報初期化
	{
		VecFx32 initVec32 = { 0, 0, 0 };
		VecFx16 initVec16 = { FX32_ONE-1, FX32_ONE-1, FX32_ONE-1 };

		//射影
		GFL_G3D_sysProjectionSet( GFL_G3D_PRJPERS,
						FX_SinIdx( 0x1000 ), FX_CosIdx( 0x1000 ), ( FX32_ONE * 4 / 3 ), 0, 
						( 1 << FX32_SHIFT ), ( 1024 << FX32_SHIFT ), 0 );
		//ライト
		GFL_G3D_sysLightSet( 0, &initVec16, 0x7fff );
		GFL_G3D_sysLightSet( 1, &initVec16, 0x7fff );
		GFL_G3D_sysLightSet( 2, &initVec16, 0x7fff );
		GFL_G3D_sysLightSet( 3, &initVec16, 0x7fff );
		//カメラ
		GFL_G3D_sysLookAtSet( &initVec32, &initVec32, &initVec32 );
		g3Dman->lookAt.camPos.z	= ( 256 << FX32_SHIFT );
		g3Dman->lookAt.camUp.y	= FX32_ONE;
		//レンダリングスワップバッファ
		GFL_G3D_sysSwapBufferModeSet( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
	}

	if( setup != NULL ){
		setup();
	} else {	//各種モードデフォルトセットアップ
		// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
		G3X_SetShading( GX_SHADING_TOON );
		G3X_AntiAlias( FALSE );
		G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
		G3X_AlphaBlend( FALSE );		// アルファブレンド　オン
		G3X_EdgeMarking( FALSE );
		G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

		// クリアカラーの設定
		G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

		// ビューポートの設定
		G3_ViewPort(0, 0, 255, 191);
	}
}

//--------------------------------------------------------------------------------------------
/**
 *
 * マネージャ終了
 *
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysExit
		( void )
{
	if( G3DMAN_CHECK() == FALSE ){
		OS_Printf("g3D_system_maneger is nothing(GFL_G3D_sysExit)\n");
	} else {
		//ＶＲＡＭパレットマネージャの解放
		if( g3Dman->plt_memory != NULL ){
			GFL_HEAP_FreeMemory( g3Dman->plt_memory );
		}
		//ＶＲＡＭテクスチャマネージャの解放
		if( g3Dman->tex_memory != NULL ){
			GFL_HEAP_FreeMemory( g3Dman->tex_memory );
		}
		// ジオメトリコマンドバッファの解放
		if( NNS_G3dGeIsBufferExist() == TRUE ){
			NNSG3dGeBuffer* geBuf = NNS_G3dGeReleaseBuffer();
			GFL_HEAP_DTCM_FreeMemory( geBuf );
		}
		GFL_HEAP_FreeMemory( g3Dman );
	}
}

//--------------------------------------------------------------------------------------------
/**
 *
 * 内部保持情報（グローバルステート）へのアクセス
 *
 */
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * 射影行列の設定
 *	保存しておく必要があるかわわからないが、とりあえず。
 *	直接射影行列を直接設定する場合、その行列は外部で持つ
 *
 * @param	type		射影タイプ
 * @param	param1		PRJPERS			→fovySin :縦(Y)方向の視界角度(画角)/2の正弦をとった値
 *						PRJPERS,PRJORTH	→top	  :nearクリップ面上辺のY座標
 * @param	param2		PRJPERS			→fovyCos :縦(Y)方向の視界角度(画角)/2の余弦をとった値	
 *						PRJPERS,PRJORTH	→bottom  :nearクリップ面下辺のY座標
 * @param	param3		PRJPERS			→aspect  :縦に対する視界の割合(縦横比：視界での幅／高さ)
 *						PRJPERS,PRJORTH	→left	  :nearクリップ面左辺のX座標
 * @param	param4		PRJPERS			→未使用 
 *						PRJPERS,PRJORTH	→right	  :nearクリップ面右辺のX座標
 * @param	near		視点からnearクリップ面までの距離	
 * @param	far			視点からfarクリップ面までの距離	
 * @param	scaleW		ビューボリュームの精度調整パラメータ（使用しないときは0）
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysProjectionSet
		( GFL_G3D_PROJECTION_TYPE type, fx32 param1, fx32 param2, fx32 param3, fx32 param4, 
			fx32 near, fx32 far, fx32 scaleW )
{
	g3Dman->projection.type		= type;
	g3Dman->projection.param1	= param1;
	g3Dman->projection.param2	= param2;
	g3Dman->projection.param3	= param3;
	g3Dman->projection.param4	= param4;
	g3Dman->projection.near		= near;
	g3Dman->projection.far		= far;
	g3Dman->projection.scaleW	= scaleW;

	switch( type ){
		case GFL_G3D_PRJPERS:	// 透視射影を設定
			if( !scaleW ){
				NNS_G3dGlbPerspective( param1, param2, param3, near, far );
			} else {
				NNS_G3dGlbPerspectiveW( param1, param2, param3, near, far, scaleW );
			}
			break;
		case GFL_G3D_PRJFRST:	// 透視射影を設定
			if( !scaleW ){
				NNS_G3dGlbFrustum( param1, param2, param3, param4, near, far );
			} else {
				NNS_G3dGlbFrustumW( param1, param2, param3, param4, near, far, scaleW );
			}
			break;
		case GFL_G3D_PRJORTH:	// 正射影を設定
			if( !scaleW ){
				NNS_G3dGlbOrtho( param1, param2, param3, param4, near, far );
			} else {
				NNS_G3dGlbOrthoW( param1, param2, param3, param4, near, far, scaleW );
			}
			break;
	}
}

void
	GFL_G3D_sysProjectionSetDirect
		( MtxFx44* param )
{
	g3Dman->projection.type		= GFL_G3D_PRJMTX;
	g3Dman->projection.param1	= 0;
	g3Dman->projection.param2	= 0;
	g3Dman->projection.param3	= 0;
	g3Dman->projection.param4	= 0;
	g3Dman->projection.near		= 0;
	g3Dman->projection.far		= 0;

	NNS_G3dGlbSetProjectionMtx( param );
}

//--------------------------------------------------------------------------------------------
/**
 * ライトの設定
 *
 * @param	lightID			ライトＩＤ
 * @param	vec				ライトのベクトルポインタ
 * @param	color			色
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysLightSet
		( u8 lightID, VecFx16* vec, u16 color )
{
	g3Dman->light[ lightID ].vec.x = vec->x;
	g3Dman->light[ lightID ].vec.y = vec->y;
	g3Dman->light[ lightID ].vec.z = vec->z;
	g3Dman->light[ lightID ].color = color;

	NNS_G3dGlbLightVector( lightID, vec->x, vec->y, vec->z );
	NNS_G3dGlbLightColor( lightID, color );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ行列の設定
 *
 * @param	camPos			カメラ位置ベクトルポインタ
 * @param	camUp			カメラの上方向のベクトルへのポインタ
 * @param	target			カメラ焦点へのポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysLookAtSet
		( VecFx32* camPos, VecFx32* camUp, VecFx32* target )
{
	g3Dman->lookAt.camPos	= *camPos;
	g3Dman->lookAt.camUp	= *camUp;
	g3Dman->lookAt.target	= *target;
}

//--------------------------------------------------------------------------------------------
/**
 * レンダリングスワップバッファの設定
 *
 * @param	sortMode		ソートモード
 * @param	bufferMode		デプスバッファモード
 * 設定値についてはNitroSDK内GX_SwapBuffer関数を参照
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysSwapBufferModeSet
		( GXSortMode sortMode, GXBufferMode bufferMode )
{
	g3Dman->swapBufMode.aw = sortMode;
	g3Dman->swapBufMode.zw = bufferMode;
}





//=============================================================================================
/**
 *
 *
 * ３Ｄリソース管理関数
 *
 *
 */
//=============================================================================================
#define G3DRES_MAGICNUM	(0x48BF)

enum {
	GFL_G3D_RES_TYPE_UNKNOWN = 0,	//不明なデータ
	GFL_G3D_RES_TYPE_MDLTEX,		//ファイル内にモデリングおよびテクスチャデータ内包
	GFL_G3D_RES_TYPE_MDL,			//ファイル内にモデリングデータ内包
	GFL_G3D_RES_TYPE_TEX,			//ファイル内にテクスチャデータ内包
	GFL_G3D_RES_TYPE_ANM,			//ファイル内にアニメーションデータ内包
};

///	３Ｄリソース構造体
struct _GFL_G3D_RES {
	u16		magicnum;
	u16		type;
	void* 	file;
};

static BOOL
	GFL_G3D_GetTexDataVramkey( NNSG3dResTex* res, NNSGfdTexKey* tex, NNSGfdTexKey* tex4x4 );
static BOOL
	GFL_G3D_GetTexPlttVramkey( NNSG3dResTex* res, NNSGfdPlttKey* pltt );

static inline BOOL G3DRES_FILE_CHECK( GFL_G3D_RES* g3Dres )
{
	if( g3Dres->magicnum == G3DRES_MAGICNUM ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄリソースの読み込み
 *
 * @param	arcID or path	アーカイブＩＤ or ファイルパス
 * @param	datID			データＩＤ
 *
 * @return	g3Dres	３Ｄリソースポインタ(失敗=NULL)
 */
//--------------------------------------------------------------------------------------------
static GFL_G3D_RES*
	GFL_G3D_ResourceCreate
		( NNSG3dResFileHeader* header )
{
	//リソース管理ハンドル作成
	GFL_G3D_RES* g3Dres = GFL_HEAP_AllocMemory( g3Dman->heapID, sizeof(GFL_G3D_RES) );

	//OS_Printf("3D_resource check now...\n");
	//ファイルタイプの判別
	switch( *(u32*)&header[0] )
	{
		case NNS_G3D_SIGNATURE_NSBMD:
			//OS_Printf("nsbmd file check...\n");
			if( NNS_G3dGetTex( header ) == NULL ){
				//OS_Printf("this 3D_resource is model_data\n");
				g3Dres->type = GFL_G3D_RES_TYPE_MDL;	//モデリングデータ内包
			} else {
				//OS_Printf("this 3D_resource is model_data & texture_data\n");
				g3Dres->type = GFL_G3D_RES_TYPE_MDLTEX;	//モデリングおよびテクスチャデータ内包
			}
			break;
		case NNS_G3D_SIGNATURE_NSBTX:
			//OS_Printf("this 3D_resource is texture_data\n");
			g3Dres->type = GFL_G3D_RES_TYPE_TEX;		//テクスチャデータ内包
			break;
		case NNS_G3D_SIGNATURE_NSBCA:
		case NNS_G3D_SIGNATURE_NSBVA:
		case NNS_G3D_SIGNATURE_NSBMA:
		case NNS_G3D_SIGNATURE_NSBTP:
		case NNS_G3D_SIGNATURE_NSBTA:
			//OS_Printf("this 3D_resource is animetion_data\n");
			g3Dres->type = GFL_G3D_RES_TYPE_ANM;		//ファイル内にアニメーションデータ内包
			break;
		default:
			//OS_Printf("this 3D_resource is unknown\n");
			g3Dres->type = GFL_G3D_RES_TYPE_UNKNOWN;	//不明なデータ
			break;
	}
	g3Dres->magicnum = G3DRES_MAGICNUM;
	//ファイルポインタの設定
	g3Dres->file = ( void* )header;
	//OS_Printf("3D_resource is loaded\n");

	return g3Dres;
}

//-------------------------------
// アーカイブＩＤによる読み込み
GFL_G3D_RES*
	GFL_G3D_ResourceCreateArc
		( int arcID, int datID ) 
{
	NNSG3dResFileHeader* header;

	//システムチェック
	if( G3DMAN_CHECK() == FALSE ){
		OS_Panic("please setup 3D_system_manager (GFL_G3D_ResourceCreateArc)\n");
		return NULL;
	}
	//OS_Printf("3D_resource loading...\n");
	//対象アーカイブＩＮＤＥＸからヘッダデータを読み込み
	header = GFL_ARC_DataLoadMalloc( arcID, datID, g3Dman->heapID );

	return GFL_G3D_ResourceCreate( header );
}

//-------------------------------
// アーカイブファイルパスによる読み込み
GFL_G3D_RES*
	GFL_G3D_ResourceCreatePath
		( const char* path, int datID ) 
{
	NNSG3dResFileHeader* header;

	//システムチェック
	if( G3DMAN_CHECK() == FALSE ){
		OS_Panic("please setup 3D_system_manager (GFL_G3D_ResourceCreateArc)\n");
		return NULL;
	}
	//OS_Printf("3D_resource loading...\n");
	//対象アーカイブファイルからヘッダデータを読み込み
	header = GFL_ARC_DataLoadFilePathMalloc( path, datID, g3Dman->heapID );

	return GFL_G3D_ResourceCreate( header );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄリソースの破棄
 *
 * @param	g3Dres	３Ｄリソースポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ResourceDelete
		( GFL_G3D_RES* g3Dres ) 
{
	if( G3DRES_FILE_CHECK( g3Dres ) == FALSE ){
		OS_Printf("file is not 3D_resource (GFL_G3D_ResourceDelete)\n");
		return;
	}
	GFL_HEAP_FreeMemory( g3Dres->file );
	GFL_HEAP_FreeMemory( g3Dres );
}
	
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄリソースタイプの確認
 *
 * @param	g3Dres		３Ｄリソースポインタ
 * @param	checkType	チェックするタイプ
 *
 * @return	BOOL		TRUEで一致
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_ResourceTypeCheck
		( GFL_G3D_RES* g3Dres, GFL_G3D_RES_CHKTYPE checkType ) 
{
	u16 resType;

	if( G3DRES_FILE_CHECK( g3Dres ) == FALSE ){
		OS_Printf("file is not 3D_resource (GFL_G3D_ResourceTypeCheck)\n");
		return FALSE;
	}
	resType = g3Dres->type;

	switch( checkType ){
		case GFL_G3D_RES_CHKTYPE_UKN:
			if( resType == GFL_G3D_RES_TYPE_UNKNOWN ){
				return TRUE;
			} else {
				return FALSE;
			}
			break;

		case GFL_G3D_RES_CHKTYPE_MDL:
			if(( resType == GFL_G3D_RES_TYPE_MDLTEX )||( resType == GFL_G3D_RES_TYPE_MDL )){
				return TRUE;
			} else {
				return FALSE;
			}
			break;

		case GFL_G3D_RES_CHKTYPE_TEX:
			if(( resType == GFL_G3D_RES_TYPE_MDLTEX )||( resType == GFL_G3D_RES_TYPE_TEX )){
				return TRUE;
			} else {
				return FALSE;
			}
			break;

		case GFL_G3D_RES_CHKTYPE_ANM:
			if( resType == GFL_G3D_RES_TYPE_ANM ){
				return TRUE;
			} else {
				return FALSE;
			}
			break;
	}
	return FALSE;
}
	




//=============================================================================================
/**
 *
 *
 * ３Ｄリソース転送関数（事実上テクスチャデータ、テクスチャパレットのみ）
 *
 *
 */
//=============================================================================================
static inline BOOL G3DRES_FILE_CHECK_EQU_TEX( GFL_G3D_RES* g3Dres )
{
	if( g3Dres->magicnum == G3DRES_MAGICNUM ){
		if(( g3Dres->type == GFL_G3D_RES_TYPE_MDLTEX )||( g3Dres->type == GFL_G3D_RES_TYPE_TEX )){
			return TRUE;
		}
	}
	OS_Panic("file is not texture_resource (GFL_G3D_VramLoadTex)\n");
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * テクスチャリソースのＶＲＡＭ領域確保と転送
 *
 * @param	g3Dres	３Ｄリソースポインタ
 *
 * @return	BOOL	結果(成功=TRUE)
 */
//--------------------------------------------------------------------------------------------
BOOL 
	GFL_G3D_VramLoadTex
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;
	NNSGfdPlttKey			plttKey;

	G3DRES_FILE_CHECK_EQU_TEX( g3Dres );

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexDataVramkey( texture, &texKey, &tex4x4Key ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadTex)\n");
			return FALSE;
		}
		if( GFL_G3D_GetTexPlttVramkey( texture, &plttKey ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadTex)\n");
			return FALSE;
		}
		//リソースへのＶＲＡＭキーの設定
		NNS_G3dTexSetTexKey( texture, texKey, tex4x4Key );
		NNS_G3dPlttSetPlttKey( texture, plttKey );

		//内部でＤＭＡ転送するので転送元をフラッシュする
		DC_FlushRange( header, header->fileSize );
		//キーを参照して、リソースをＶＲＡＭ転送
		NNS_G3dTexLoad( texture, TRUE );
		NNS_G3dPlttLoad( texture, TRUE );
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * テクスチャリソースをＶＲＡＭから解放
 *
 * @param	g3Dres	３Ｄリソースポインタ
 *
 * @return	BOOL	結果(成功=TRUE)
 */
//--------------------------------------------------------------------------------------------
BOOL 
	GFL_G3D_VramUnloadTex
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;
	NNSGfdPlttKey			plttKey;

	G3DRES_FILE_CHECK_EQU_TEX( g3Dres );

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	//ＶＲＡＭキーの放棄フラグをリソースに設定
	NNS_G3dTexReleaseTexKey( texture, &texKey, &tex4x4Key );
	plttKey = NNS_G3dPlttReleasePlttKey( texture );

	if( NNS_GfdFreePlttVram( plttKey ) ){
		OS_Printf("Vramkey cannot free (GFL_G3D_VramUnloadTex)\n");
	}
	if( NNS_GfdFreeLnkTexVram( tex4x4Key )){
		OS_Printf("Vramkey cannot free (GFL_G3D_VramUnloadTex)\n");
	}
	if( NNS_GfdFreeLnkTexVram( texKey )){
		OS_Printf("Vramkey cannot free (GFL_G3D_VramUnloadTex)\n");
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * テクスチャデータリソースのＶＲＡＭ領域確保と転送
 *
 * @param	g3Dres	３Ｄリソースポインタ
 *
 * @return	BOOL	結果(成功=TRUE)
 *
 * 連続してテクスチャを読み込む場合に効率的に使用するための関数
 * 連続読み込みの際は、外部で
 *    開始時にGX_BeginLoadTex関数
 *    終了時にGX_EndLoadTex関数
 * を呼ぶ必要がある
 */
//--------------------------------------------------------------------------------------------
BOOL 
	GFL_G3D_VramLoadTexDataOnly
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;

	G3DRES_FILE_CHECK_EQU_TEX( g3Dres );

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexDataVramkey( texture, &texKey, &tex4x4Key ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadTexDataOnly)\n");
			return FALSE;
		}
		//リソースへのＶＲＡＭキーの設定
		NNS_G3dTexSetTexKey( texture, texKey, tex4x4Key );

		//内部でＤＭＡ転送するので転送元をフラッシュする
		DC_FlushRange( header, header->fileSize );
		//キーを参照して、リソースをＶＲＡＭ転送
		NNS_G3dTexLoad( texture, FALSE );
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * テクスチャパレットリソースのＶＲＡＭ領域確保と転送
 *
 * @param	g3Dres	３Ｄリソースポインタ
 *
 * @return	BOOL	結果(成功=TRUE)
 *
 * 連続してパレットを読み込む場合に効率的に使用するための関数
 * 連続読み込みの際は、外部で
 *    開始時にGX_BeginLoadPltt関数
 *    終了時にGX_EndLoadPltt関数
 * を呼ぶ必要がある
 */
//--------------------------------------------------------------------------------------------
BOOL 
	GFL_G3D_VramLoadTexPlttOnly
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdPlttKey			plttKey;

	G3DRES_FILE_CHECK_EQU_TEX( g3Dres );

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexPlttVramkey( texture, &plttKey ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadPlttOnly)\n");
			return FALSE;
		}
		//リソースへのＶＲＡＭキーの設定
		NNS_G3dPlttSetPlttKey( texture, plttKey );

		//内部でＤＭＡ転送するので転送元をフラッシュする
		DC_FlushRange( header, header->fileSize );
		//キーを参照して、リソースをＶＲＡＭ転送
		NNS_G3dPlttLoad( texture, FALSE );
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	テクスチャキーの取得
 *
 *	@param	res		テクスチャリソース参照ポインタ
 *	@param	tex		テクスチャキー保存ポインタ
 *	@param	tex4x4	4x4圧縮テクスチャ保存ポインタ
 *	@param	pltt	パレットキー保存ポインタ
 */
//-----------------------------------------------------------------------------
static BOOL
	GFL_G3D_GetTexDataVramkey
		( NNSG3dResTex* res, NNSGfdTexKey* tex, NNSGfdTexKey* tex4x4 )
{
	//各リソースサイズ取得
	u32 tex_size		= NNS_G3dTexGetRequiredSize( res );
	u32 tex_4x4_size	= NNS_G3dTex4x4GetRequiredSize( res );

	//キー取得(ＶＲＡＭ確保)
	if( tex_size != 0 ){
		*tex = NNS_GfdAllocTexVram( tex_size, FALSE, 0 );
		if( *tex == NNS_GFD_ALLOC_ERROR_TEXKEY ){
			return FALSE;
		}
	}
	if( tex_4x4_size != 0 ){
		*tex4x4 = NNS_GfdAllocTexVram( tex_size, TRUE, 0 );
		if( *tex4x4 == NNS_GFD_ALLOC_ERROR_TEXKEY ){
			return FALSE;
		}
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	パレットキーの取得
 *
 *	@param	res		テクスチャリソース参照ポインタ
 *	@param	pltt	パレットキー保存ポインタ
 */
//-----------------------------------------------------------------------------
static BOOL
	GFL_G3D_GetTexPlttVramkey
		( NNSG3dResTex* res, NNSGfdPlttKey* pltt )
{
	//リソースサイズ取得
	u32 pltt_size		= NNS_G3dPlttGetRequiredSize( res );

	//キー取得(ＶＲＡＭ確保)
	if( pltt_size != 0 ){
		BOOL b4Pltt = res->tex4x4Info.flag & NNS_G3D_RESPLTT_USEPLTT4;
		*pltt = NNS_GfdAllocPlttVram( pltt_size, b4Pltt, 0 );
		if( *pltt == NNS_GFD_ALLOC_ERROR_PLTTKEY ){
			return FALSE;
		}
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	テクスチャキーが割り振られているかチェック
 *
 *@param	g3Dobj		３Ｄオブジェクトハンドル
 *	
 *@return	BOOL		TRUE:割り振られてる、FALSE:割り振られていない
 */
//-----------------------------------------------------------------------------
BOOL
	GFL_G3D_ObjTexkeyLiveCheck
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			restex;

	G3DRES_FILE_CHECK_EQU_TEX( g3Dres );

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	restex = NNS_G3dGetTex( header ); 

	// 普通のテクスチャか4*4テクセル圧縮テクスチャ
	// でVramに展開されているかをチェック
	if(( restex->texInfo.flag & NNS_G3D_RESTEX_LOADED ) ||
	   ( restex->tex4x4Info.flag & NNS_G3D_RESTEX4x4_LOADED )||
	   ( restex->plttInfo.flag & NNS_G3D_RESPLTT_LOADED )){
		return TRUE;
	}
	return FALSE;
}





//=============================================================================================
/**
 *
 *
 * 各オブジェクト管理関数
 *
 *
 */
//=============================================================================================
#define G3DOBJ_MAGICNUM	(0x7A14)
///	３Ｄモデル制御用構造体
struct _GFL_G3D_OBJ
{
	u16						magicnum;
	u16						dummy;

	NNSG3dRenderObj*		rndobj;
	NNSG3dAnmObj*			anmobj;

	NNSG3dResMdl*			pMdl;
	NNSG3dResTex*			pTex;
	void*					pAnm;

	VecFx32					trans;
	VecFx32					scale;
	MtxFx33					rotate;
};

static inline BOOL G3DOBJ_HANDLE_CHECK( GFL_G3D_OBJ* g3Dobj )
{
	if( g3Dobj->magicnum == G3DOBJ_MAGICNUM ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの作成
 *
 * @param	mdl			参照モデルセットリソース構造体ポインタ
 * @param	mdlidx		mdl内データインデックス(複数登録されている場合。１つの場合は0)
 * @param	tex			参照テクスチャリソース構造体ポインタ(使用しない場合はNULL)
 * @param	anm			参照アニメーションリソース構造体ポインタ(使用しない場合はNULL)
 * @param	anmidx		anm内データインデックス(複数登録されている場合。１つの場合は0)
 *
 * @return	GFL_3D_OBJ	３Ｄオブジェクトハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_ObjCreate
		( GFL_G3D_RES* mdl, int mdlidx, GFL_G3D_RES* tex, GFL_G3D_RES* anm, int anmidx )  
{
	NNSG3dResFileHeader*	header;
	NNSG3dResMdlSet*		pMdlset;
	NNSG3dResMdl*			pMdl;
	NNSG3dResTex*			pTex;
	void*					pAnm;
	GFL_G3D_OBJ*			g3Dobj;

	//整合性チェックおよび各種リソースポインタ取得
	if(( G3DRES_FILE_CHECK( mdl ) == TRUE )&&
	   (( mdl->type == GFL_G3D_RES_TYPE_MDLTEX )||( mdl->type == GFL_G3D_RES_TYPE_MDL )) ){
		//モデルデータリソースポインタ取得
		header = (NNSG3dResFileHeader*)mdl->file;
		pMdlset = NNS_G3dGetMdlSet( header );
		pMdl = NNS_G3dGetMdlByIdx( pMdlset, mdlidx );
	} else {
		pMdl = NULL;
	}
	if(( G3DRES_FILE_CHECK( tex ) == TRUE )&&
	   (( tex->type == GFL_G3D_RES_TYPE_MDLTEX )||( tex->type == GFL_G3D_RES_TYPE_TEX )) ){
		//テクスチャリソースポインタ取得
		header = (NNSG3dResFileHeader*)tex->file;
		pTex = NNS_G3dGetTex( header );
	} else {
		pTex = NULL;
	}
	if(( G3DRES_FILE_CHECK( anm ) == TRUE )&&( anm->type == GFL_G3D_RES_TYPE_ANM )){
		//アニメーションリソースポインタ取得
		pAnm = NNS_G3dGetAnmByIdx( anm->file, anmidx );
	} else {
		pAnm = NULL;
	}
	if(( pMdl == NULL )&&( pAnm == NULL )){
		OS_Panic("cannot cleate 3D_object (GFL_G3D_ObjCreate)\n");
		return NULL;
	}

	//オブジェクトハンドルの作成
	g3Dobj = GFL_HEAP_AllocMemory( g3Dman->heapID, sizeof(GFL_G3D_OBJ) );
	g3Dobj->magicnum = G3DOBJ_MAGICNUM;
	g3Dobj->pMdl = pMdl;
	g3Dobj->pTex = pTex;
	g3Dobj->pAnm = pAnm;

	//レンダリングオブジェクト設定
	if( pMdl ){
		//レンダリングオブジェクト領域の確保
		g3Dobj->rndobj = NNS_G3dAllocRenderObj( &g3Dman->allocater );
		if( g3Dobj->rndobj == NULL ){
			OS_Panic("cannot alloc rndobj (GFL_G3D_ObjCreate)\n");
			return NULL;
		}
		//テクスチャリソースとの関連付け
		NNS_G3dBindMdlSet( pMdlset, pTex );
		//レンダリングオブジェクト初期化
		NNS_G3dRenderObjInit( g3Dobj->rndobj, pMdl);
	} else {
		g3Dobj->rndobj = NULL;	//レンダリングなし※アニメコントロールオブジェクトになる
	}

	//アニメーションオブジェクト領域の確保と設定
	if( pAnm ){
		//アニメーションオブジェクト領域の確保
		g3Dobj->anmobj = NNS_G3dAllocAnmObj( &g3Dman->allocater, pAnm, pMdl );
		if( g3Dobj->anmobj == NULL ){
			OS_Panic("cannot alloc anmobj (GFL_G3D_ObjCreate)\n");
			return NULL;
		}
		//アニメーションオブジェクト初期化
		NNS_G3dAnmObjInit( g3Dobj->anmobj, pAnm, pMdl, pTex );
		//レンダリングオブジェクトとの関連付け
		NNS_G3dRenderObjAddAnmObj( g3Dobj->rndobj, g3Dobj->anmobj );
	} else {
		g3Dobj->anmobj = NULL;
	}

	//オブジェクトステータスワーク初期化
	{
		VecFx32 init_trans	= { 0, 0, 0 };
		VecFx32 init_scale	= { FX32_ONE, FX32_ONE, FX32_ONE };
		MtxFx33 init_rotate = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };

		g3Dobj->trans	= init_trans;
		g3Dobj->scale	= init_scale;
		g3Dobj->rotate	= init_rotate;
	}
	return g3Dobj;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの破棄
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjDelete
		( GFL_G3D_OBJ* g3Dobj ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjDelete)\n");
		return;
	}
	if( g3Dobj->anmobj != NULL ){
		NNS_G3dFreeAnmObj( &g3Dman->allocater, g3Dobj->anmobj );
	}
	if( g3Dobj->rndobj != NULL ){
		NNS_G3dFreeRenderObj( &g3Dman->allocater, g3Dobj->rndobj );
	}
	GFL_HEAP_FreeMemory( g3Dobj );
}
	




//=============================================================================================
/**
 *
 *
 * 各オブジェクト描画関数
 *
 *	SAMPLE
 *	{
 *		GFL_G3D_DrawStart();							//描画開始
 *		GFL_G3D_DrawLookAt();							//カメラグローバルステート設定		
 *		{
 *			GFL_G3D_ObjDraw( g3Dobj );					//各オブジェクト描画
 *		}
 *		GFL_G3D_DrawEnd();								//描画終了（バッファスワップ）
 *	}
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄ描画の開始
 *
 * 全体描画関数内、描画開始時に呼び出される
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_DrawStart
		( void )
{
	G3X_Reset();
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄ描画の終了
 *
 * 全体描画関数内、描画を終了時に呼び出される
 * 内部でレンダリングバッファのスワップを行う。
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_DrawEnd
		( void )
{
	G3_SwapBuffers( g3Dman->swapBufMode.aw, g3Dman->swapBufMode.zw );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラグローバルステートの設定
 *
 * 全体描画関数内に呼び出される
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_DrawLookAt
		( void )
{
	NNS_G3dGlbLookAt( &g3Dman->lookAt.camPos, &g3Dman->lookAt.camUp, &g3Dman->lookAt.target );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの描画
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 *
 * ～Draw	:カレント射影行列に射影変換行列が、
 *			 カレント位置座標行列と方向ベクトル行列にカメラ行列とモデリング行列が合成された行列
 *			 が設定されます。
 *
 * ～DrawVP	:カレント射影行列に射影変換行列とカメラ行列が合成された行列が、
 *			 カレント位置座標行列と方向ベクトル行列にモデリング行列
 *			 が設定されます。
 *
 * ～DrawWVP:カレント射影行列に射影変換行列とカメラ行列とモデリング行列が合成された行列が、
 *			 カレント位置座標行列と方向ベクトル行列に単位行列
 *			 が設定されます。
 */
//--------------------------------------------------------------------------------------------
// ３Ｄオブジェクトの情報設定
static inline void statusSet( VecFx32* pTrans, MtxFx33* pRotate, VecFx32* pScale )
{
	// 位置設定
	NNS_G3dGlbSetBaseTrans( pTrans );
	// 角度設定
	NNS_G3dGlbSetBaseRot( pRotate );
	// スケール設定
	NNS_G3dGlbSetBaseScale( pScale );
}

//--------------------------------------------------------------------------------
// 通常描画
//--------------------------------------------------------------------------------
void
	GFL_G3D_ObjDraw
		( GFL_G3D_OBJ* g3Dobj )
{
	statusSet( &g3Dobj->trans, &g3Dobj->rotate, &g3Dobj->scale );
	NNS_G3dGlbFlush();
	NNS_G3dDraw( g3Dobj->rndobj );
	NNS_G3dGeFlushBuffer();
}

void
	GFL_G3D_ObjDrawVP
		( GFL_G3D_OBJ* g3Dobj )
{
	statusSet( &g3Dobj->trans, &g3Dobj->rotate, &g3Dobj->scale );
	NNS_G3dGlbFlushVP();
	NNS_G3dDraw( g3Dobj->rndobj );
	NNS_G3dGeFlushBuffer();
}

void
	GFL_G3D_ObjDrawWVP
		( GFL_G3D_OBJ* g3Dobj )
{
	statusSet( &g3Dobj->trans, &g3Dobj->rotate, &g3Dobj->scale );
	NNS_G3dGlbFlushWVP();
	NNS_G3dDraw( g3Dobj->rndobj );
	NNS_G3dGeFlushBuffer();
}

//--------------------------------------------------------------------------------
// １つのモデルに１つのマテリアルのみ設定されているときに高速描画するための関数
//		matID	描画するマテリアルへのインデックス 
//		shpID	描画するシェイプへのインデックス 
//		sendMat	マテリアル情報をジオメトリエンジンに送信するかどうか 
//--------------------------------------------------------------------------------
void
	GFL_G3D_ObjDraw1mat1shp
		( GFL_G3D_OBJ* g3Dobj, u32 matID, u32 shpID, BOOL sendMat )

{
	statusSet( &g3Dobj->trans, &g3Dobj->rotate, &g3Dobj->scale );
	NNS_G3dGlbFlush();
	NNS_G3dDraw1Mat1Shp( g3Dobj->pMdl, matID, shpID, sendMat );
	NNS_G3dGeFlushBuffer();
}

void
	GFL_G3D_ObjDrawVP1mat1shp
		( GFL_G3D_OBJ* g3Dobj, u32 matID, u32 shpID, BOOL sendMat )
{
	statusSet( &g3Dobj->trans, &g3Dobj->rotate, &g3Dobj->scale );
	NNS_G3dGlbFlushVP();
	NNS_G3dDraw1Mat1Shp( g3Dobj->pMdl, matID, shpID, sendMat );
	NNS_G3dGeFlushBuffer();
}

void
	GFL_G3D_ObjDrawWVP1mat1shp
		( GFL_G3D_OBJ* g3Dobj, u32 matID, u32 shpID, BOOL sendMat )
{
	statusSet( &g3Dobj->trans, &g3Dobj->rotate, &g3Dobj->scale );
	NNS_G3dGlbFlushWVP();
	NNS_G3dDraw1Mat1Shp( g3Dobj->pMdl, matID, shpID, sendMat );
	NNS_G3dGeFlushBuffer();
}





//=============================================================================================
/**
 *
 *
 * 各オブジェクトコントロール関数
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * 位置情報をセット
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	trans	セットする位置情報
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjContSetTrans
		( GFL_G3D_OBJ* g3Dobj, const VecFx32* trans ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjContSetTrans)\n");
		return;
	}
	g3Dobj->trans = *trans;
}

//--------------------------------------------------------------------------------------------
/**
 * スケール情報をセット
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	trans	セットするスケール情報
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjContSetScale
		( GFL_G3D_OBJ* g3Dobj, const VecFx32* scale ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjContSetScale)\n");
		return;
	}
	g3Dobj->scale = *scale;
}

//--------------------------------------------------------------------------------------------
/**
 * 回転情報をセット
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	rotate	セットする回転情報
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjContSetRotate
		( GFL_G3D_OBJ* g3Dobj, const MtxFx33* rotate ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjContSetRotate)\n");
		return;
	}
	g3Dobj->rotate = *rotate;
}

//--------------------------------------------------------------------------------------------
/**
 * アニメーションフレームをリセット
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjContAnmFrameReset
		( GFL_G3D_OBJ* g3Dobj ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjContAnmFrameInc)\n");
		return;
	}
	if( g3Dobj->anmobj == NULL ){
		OS_Printf("this handle is not have animetion (GFL_G3D_ObjContAnmFrameInc)\n");
		return;
	}
	g3Dobj->anmobj->frame = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * アニメーションフレームを進める
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	count	増加分（FX32_ONEで１フレーム進める）
 *
 * @return	BOOL	FALSEで終了検出
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_ObjContAnmFrameInc
		( GFL_G3D_OBJ* g3Dobj, fx32 count ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjContAnmFrameInc)\n");
		return FALSE;
	}
	if( g3Dobj->anmobj == NULL ){
		OS_Printf("this handle is not have animetion (GFL_G3D_ObjContAnmFrameInc)\n");
		return FALSE;
	}
	g3Dobj->anmobj->frame += count;
	if( g3Dobj->anmobj->frame >= NNS_G3dAnmObjGetNumFrame( g3Dobj->anmobj )){
		return FALSE;
	} else {
		return TRUE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * アニメーションオートループ
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	count	増加分（FX32_ONEで１フレーム進める）
 *
 * @return	BOOL	FALSEで１ループ終了検出
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_ObjContAnmFrameAutoLoop
		( GFL_G3D_OBJ* g3Dobj, fx32 count ) 
{
	if( G3DOBJ_HANDLE_CHECK( g3Dobj ) == FALSE ){
		OS_Printf("handle is not 3D_object (GFL_G3D_ObjContAnmFrameInc)\n");
		return FALSE;
	}
	if( g3Dobj->anmobj == NULL ){
		OS_Printf("this handle is not have animetion (GFL_G3D_ObjContAnmFrameInc)\n");
		return FALSE;
	}
	g3Dobj->anmobj->frame += count;
	if( g3Dobj->anmobj->frame >= NNS_G3dAnmObjGetNumFrame( g3Dobj->anmobj )){
		g3Dobj->anmobj->frame = 0;
		return FALSE;
	}
	return TRUE;
}





//=============================================================================================
/**
 *
 *
 * データ
 *
 *
 */
//=============================================================================================



