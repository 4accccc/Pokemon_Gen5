//=============================================================================================
/**
 * @file	g3d_system.c                                                  
 * @brief	３Ｄ管理システムプログラム
 * @date	2006/4/26
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	定数宣言
//=============================================================================================
#define TEX_BLOCKNUM	(128)
#define PLT_BLOCKNUM	(256)

#define TEX_SLOTSIZ		(0x20000)
#define PLT_SLOTSIZ		(0x2000)

//=============================================================================================
//	型宣言
//=============================================================================================
typedef void	(GFL_G3D_DRAW_FLASHFUNC)( void );

/**
 * @brief	３Ｄシステムマネージャ構造体
 */
typedef struct GFL_G3D_MAN_tag
{
	NNSFndAllocator			allocater;			///<メモリアロケータ(NNSで使用)
	void*					plt_memory;			///<パレットマネージャ用メモリ
	void*					tex_memory;			///<テクスチャマネージャ用メモリ
	HEAPID					heapID;				///<ヒープＩＤ

	GFL_G3D_PROJECTION		projection;			///<グローバルステート（射影）
	GFL_G3D_LIGHT			light[4];			///<グローバルステート（ライト）
	GFL_G3D_LOOKAT			lookAt;				///<グローバルステート（カメラ）
	GFL_G3D_SWAPBUFMODE		swapBufMode;		///<グローバルステート（レンダリングバッファ）

	GFL_G3D_DRAW_FLASHFUNC*	drawFlushFunc;		///<描画フラッシュタイプ
	GFL_G3D_VMAN_MODE		texmanMode;			///<テクスチャマネージャモード
	GFL_G3D_VMAN_MODE		pltmanMode;			///<パレットマネージャモード
}GFL_G3D_MAN;

static GFL_G3D_MAN*  g3Dman = NULL;

//=============================================================================================
//=============================================================================================
/**
 *
 *
 * システムマネージャ関数
 *
 *
 */
//=============================================================================================
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
	GFL_G3D_Init
		( GFL_G3D_VMAN_MODE texmanMode, GFL_G3D_VMAN_TEXSIZE texmanSize, 
			GFL_G3D_VMAN_MODE pltmanMode, GFL_G3D_VMAN_PLTSIZE pltmanSize,
				u16 GeBufSize, HEAPID heapID, GFL_G3D_SETUP_FUNC setup )
{
	int	tex_size,plt_size;

	GF_ASSERT( g3Dman == NULL );

	g3Dman = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_MAN) );

	g3Dman->heapID = heapID;
	g3Dman->texmanMode = texmanMode;
	g3Dman->pltmanMode = pltmanMode;

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
		{	//射影
			GFL_G3D_PROJECTION initProjection = { GFL_G3D_PRJPERS, 0, 0, ( FX32_ONE * 4 / 3 ), 0, 
												( 1 << FX32_SHIFT ), ( 1024 << FX32_SHIFT ), 0 };
			initProjection.param1 = FX_SinIdx( 40/2 *PERSPWAY_COEFFICIENT ); 
			initProjection.param2 = FX_CosIdx( 40/2 *PERSPWAY_COEFFICIENT ); 
			GFL_G3D_SetSystemProjection( &initProjection );
		}
		{ //ライト
			GFL_G3D_LIGHT initLight = { { -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, 0x7fff };
			GFL_G3D_SetSystemLight( 0, &initLight );
			GFL_G3D_SetSystemLight( 1, &initLight );
			GFL_G3D_SetSystemLight( 2, &initLight );
			GFL_G3D_SetSystemLight( 3, &initLight );
		}
		{	//カメラ
			GFL_G3D_LOOKAT initCamera = {{0,0,(256<<FX32_SHIFT)},{0,FX32_ONE,0},{0,0,0}};
			GFL_G3D_SetSystemLookAt( &initCamera );
		}
		//レンダリングスワップバッファ
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
	}
	GFL_G3D_SetDrawFlushMode( GFL_G3D_FLUSH_P );

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
	GFL_G3D_Exit
		( void )
{
	GF_ASSERT( g3Dman != NULL );

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
	g3Dman = NULL;
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
 *
 * システム起動チェック
 *
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_CheckBootSystem
		( void )
{
	if( g3Dman != NULL ){
		return FALSE;
	} else {
		return TRUE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 射影行列の取得と設定
 *	保存しておく必要があるかわわからないが、とりあえず。
 *	直接射影行列を直接設定する場合、その行列は外部で持つ
 *
 *	type		射影タイプ
 *	param1		PRJPERS			→fovySin :縦(Y)方向の視界角度(画角)/2の正弦をとった値
 *				PRJFRST,PRJORTH	→top	  :nearクリップ面上辺のY座標
 *	param2		PRJPERS			→fovyCos :縦(Y)方向の視界角度(画角)/2の余弦をとった値	
 *				PRJFRST,PRJORTH	→bottom  :nearクリップ面下辺のY座標
 *	param3		PRJPERS			→aspect  :縦に対する視界の割合(縦横比：視界での幅／高さ)
 *				PRJFRST,PRJORTH	→left	  :nearクリップ面左辺のX座標
 *	param4		PRJPERS			→未使用 
 *				PRJFRST,PRJORTH	→right	  :nearクリップ面右辺のX座標
 *	near		視点からnearクリップ面までの距離	
 *	far			視点からfarクリップ面までの距離	
 *	scaleW		ビューボリュームの精度調整パラメータ（使用しないときは0）
 *
 * @param	projection		取得or設定用射影パラメータポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_GetSystemProjection
		( GFL_G3D_PROJECTION* projection )
{
	GF_ASSERT( g3Dman != NULL );

	projection->type	= g3Dman->projection.type;
	projection->param1	= g3Dman->projection.param1;
	projection->param2	= g3Dman->projection.param2;
	projection->param3	= g3Dman->projection.param3;
	projection->param4	= g3Dman->projection.param4;
	projection->near	= g3Dman->projection.near;
	projection->far		= g3Dman->projection.far;
	projection->scaleW	= g3Dman->projection.scaleW;
}

void
	GFL_G3D_SetSystemProjection
		( const GFL_G3D_PROJECTION* projection )
{
	GF_ASSERT( g3Dman != NULL );

	g3Dman->projection.type		= projection->type;
	g3Dman->projection.param1	= projection->param1;
	g3Dman->projection.param2	= projection->param2;
	g3Dman->projection.param3	= projection->param3;
	g3Dman->projection.param4	= projection->param4;
	g3Dman->projection.near		= projection->near;
	g3Dman->projection.far		= projection->far;
	g3Dman->projection.scaleW	= projection->scaleW;

	switch( g3Dman->projection.type ){
		case GFL_G3D_PRJPERS:	// 透視射影を設定
			if( !g3Dman->projection.scaleW ){
				NNS_G3dGlbPerspective(	g3Dman->projection.param1, g3Dman->projection.param2, 
										g3Dman->projection.param3, 
										g3Dman->projection.near, g3Dman->projection.far );
			} else {
				NNS_G3dGlbPerspectiveW( g3Dman->projection.param1, g3Dman->projection.param2, 
										g3Dman->projection.param3, 
										g3Dman->projection.near, g3Dman->projection.far, 
										g3Dman->projection.scaleW );
			}
			break;
		case GFL_G3D_PRJFRST:	// 透視射影を設定
			if( !g3Dman->projection.scaleW ){
				NNS_G3dGlbFrustum(	g3Dman->projection.param1, g3Dman->projection.param2, 
									g3Dman->projection.param3, g3Dman->projection.param4, 
									g3Dman->projection.near, g3Dman->projection.far );
			} else {
				NNS_G3dGlbFrustumW( g3Dman->projection.param1, g3Dman->projection.param2, 
									g3Dman->projection.param3, g3Dman->projection.param4, 
									g3Dman->projection.near, g3Dman->projection.far, 
									g3Dman->projection.scaleW );
			}
			break;
		case GFL_G3D_PRJORTH:	// 正射影を設定
			if( !g3Dman->projection.scaleW ){
				NNS_G3dGlbOrtho(	g3Dman->projection.param1, g3Dman->projection.param2, 
									g3Dman->projection.param3, g3Dman->projection.param4, 
									g3Dman->projection.near, g3Dman->projection.far );
			} else {
				NNS_G3dGlbOrthoW(	g3Dman->projection.param1, g3Dman->projection.param2,
									g3Dman->projection.param3, g3Dman->projection.param4, 
									g3Dman->projection.near, g3Dman->projection.far, 
									g3Dman->projection.scaleW );
			}
			break;
	}
}

void
	GFL_G3D_GetSystemProjectionDirect
		( MtxFx44* param )
{
	*param = *(NNS_G3dGlbGetProjectionMtx());
}
void
	GFL_G3D_SetSystemProjectionDirect
		( const MtxFx44* param )
{
	NNS_G3dGlbSetProjectionMtx( param );
}

//--------------------------------------------------------------------------------------------
/**
 * ライトの取得と設定
 *
 * @param	lightID			ライトＩＤ
 * @param	light			取得or設定用ライトパラメータポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_GetSystemLight
		( const u8 lightID, GFL_G3D_LIGHT* light )
{
	GF_ASSERT( g3Dman != NULL );

	light->vec		= g3Dman->light[ lightID ].vec;
	light->color	= g3Dman->light[ lightID ].color;
}

void
	GFL_G3D_SetSystemLight
		( const u8 lightID, const GFL_G3D_LIGHT* light )
{
	GF_ASSERT( g3Dman != NULL );

	g3Dman->light[ lightID ].vec	= light->vec;
	g3Dman->light[ lightID ].color	= light->color;

	NNS_G3dGlbLightVector(	lightID, 
							g3Dman->light[ lightID ].vec.x,
							g3Dman->light[ lightID ].vec.y,
							g3Dman->light[ lightID ].vec.z );
	NNS_G3dGlbLightColor( lightID, g3Dman->light[ lightID ].color );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ行列の取得と設定
 *
 * @param	lookAt			取得or設定用カメラ行列パラメータポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_GetSystemLookAt
		( GFL_G3D_LOOKAT* lookAt )
{
	GF_ASSERT( g3Dman != NULL );

	lookAt->camPos	= g3Dman->lookAt.camPos;
	lookAt->camUp	= g3Dman->lookAt.camUp;
	lookAt->target	= g3Dman->lookAt.target;
}

void
	GFL_G3D_SetSystemLookAt
		( const GFL_G3D_LOOKAT* lookAt )
{
	GF_ASSERT( g3Dman != NULL );

	g3Dman->lookAt.camPos	= lookAt->camPos;
	g3Dman->lookAt.camUp	= lookAt->camUp;
	g3Dman->lookAt.target	= lookAt->target;

	NNS_G3dGlbLookAt( &g3Dman->lookAt.camPos, &g3Dman->lookAt.camUp, &g3Dman->lookAt.target );
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
	GFL_G3D_SetSystemSwapBufferMode
		( const GXSortMode sortMode, const GXBufferMode bufferMode )
{
	GF_ASSERT( g3Dman != NULL );

	g3Dman->swapBufMode.aw = sortMode;
	g3Dman->swapBufMode.zw = bufferMode;
}

//--------------------------------------------------------------------------------------------
/**
 * テクスチャおよびパレットマネージャ状態の取得
 *
 * return GFL_G3D_VMAN_MODE		マネージャモード
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_VMAN_MODE
	GFL_G3D_GetTextureManagerMode
		( void )
{
	return g3Dman->texmanMode;
}

GFL_G3D_VMAN_MODE
	GFL_G3D_GetPaletteManagerMode
		( void )
{
	return g3Dman->pltmanMode;
}

//--------------------------------------------------------------------------------------------
/**
 * オブジェクト描画フラッシュ（グローバル状態送信）タイプの設定
 *
 * @param	type	描画タイプ
 *
 * GFL_G3D_FLUSH_P:		カレント射影行列に射影変換行列が、
 *						カレント位置座標行列と方向ベクトル行列に
 *						カメラ行列とモデリング行列が合成された行列が設定されます。
 *
 * GFL_G3D_FLUSH_VP:	カレント射影行列に射影変換行列とカメラ行列が合成された行列が、
 *						カレント位置座標行列と方向ベクトル行列にモデリング行列
 *						が設定されます。
 *
 * GFL_G3D_FLUSH_WVP:	カレント射影行列に
 *						射影変換行列とカメラ行列とモデリング行列が合成された行列が、
 *						カレント位置座標行列と方向ベクトル行列に単位行列が設定されます。
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_SetDrawFlushMode
		( GFL_G3D_DRAWFLUSH_TYPE type )
{
	GF_ASSERT( g3Dman != NULL );

	switch( type ){
		default:
		case GFL_G3D_FLUSH_P:
			g3Dman->drawFlushFunc = NNS_G3dGlbFlush;
			break;
		case GFL_G3D_FLUSH_VP:
			g3Dman->drawFlushFunc = NNS_G3dGlbFlushVP;
			break;
		case GFL_G3D_FLUSH_WVP:
			g3Dman->drawFlushFunc = NNS_G3dGlbFlushWVP;
			break;
	}
}





//=============================================================================================
//=============================================================================================
/**
 *
 *
 * ３Ｄデータ管理関数
 *
 *	３Ｄデータは３階層に区分する。
 *	作成手順としては以下のとおり
 *
 *	１）ファイルで配置されるＲＯＭデータの読み込み		：→リソース管理及びリソース転送参照
 *	２）１のリソースより各構成要素の作成				：→レンダー及びアニメーション管理参照
 *	３）２の要素を組み合わせてデータオブジェクトを作成	：→オブジェクト管理参照
 *
 */
//=============================================================================================
//=============================================================================================
//=============================================================================================
/**
 *
 * ３Ｄリソース管理関数
 *
 * 　各種リソースファイル（bmd,btx,bca,bva,bma,btp,bta）を一つの型で一元管理する。
 * 　リソースタイプはハンドルに内包。
 *　 全体サイズはファイルに依存するので、もちろん不定。
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
	GFL_G3D_GetTextureDataKey( NNSG3dResTex* res, NNSGfdTexKey* tex, NNSGfdTexKey* tex4x4 );
static BOOL
	GFL_G3D_GetTexturePlttKey( NNSG3dResTex* res, NNSGfdPlttKey* pltt );

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
	GFL_G3D_CreateResource
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
	GFL_G3D_CreateResourceArc
		( int arcID, int datID ) 
{
	NNSG3dResFileHeader* header;

	GF_ASSERT( g3Dman != NULL );

	//OS_Printf("3D_resource loading...\n");
	//対象アーカイブＩＮＤＥＸからヘッダデータを読み込み
	header = GFL_ARC_LoadDataAlloc( arcID, datID, g3Dman->heapID );

	return GFL_G3D_CreateResource( header );
}

//-------------------------------
// アーカイブファイルパスによる読み込み
GFL_G3D_RES*
	GFL_G3D_CreateResourcePath
		( const char* path, int datID ) 
{
	NNSG3dResFileHeader* header;

	GF_ASSERT( g3Dman != NULL );

	//OS_Printf("3D_resource loading...\n");
	//対象アーカイブファイルからヘッダデータを読み込み
	header = GFL_ARC_LoadDataFilePathAlloc( path, datID, g3Dman->heapID );

	return GFL_G3D_CreateResource( header );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄリソースの破棄
 *
 * @param	g3Dres	３Ｄリソースポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_DeleteResource
		( GFL_G3D_RES* g3Dres ) 
{
	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );

	GFL_HEAP_FreeMemory( g3Dres->file );
	GFL_HEAP_FreeMemory( g3Dres );
}
	
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄリソースヘッダポインタの取得
 *
 * @param	g3Dres		３Ｄリソースポインタ
 *
 * @return	NNSG3dResFileHeader*
 *
 * 　ＮＮＳ関数を外部で使用したい場合などに使用する
 */
//--------------------------------------------------------------------------------------------
NNSG3dResFileHeader*
	GFL_G3D_GetResourceFileHeader
		( GFL_G3D_RES* g3Dres ) 
{
	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );

	return (NNSG3dResFileHeader*)g3Dres->file;
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
	GFL_G3D_CheckResourceType
		( GFL_G3D_RES* g3Dres, GFL_G3D_RES_CHKTYPE checkType ) 
{
	u16 resType;

	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );

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
	GFL_G3D_TransVramTexture
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;
	NNSGfdPlttKey			plttKey;

	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT(( g3Dres->type==GFL_G3D_RES_TYPE_MDLTEX )||( g3Dres->type==GFL_G3D_RES_TYPE_TEX ));

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTextureDataKey( texture, &texKey, &tex4x4Key ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadTex)\n");
			return FALSE;
		}
		if( GFL_G3D_GetTexturePlttKey( texture, &plttKey ) == FALSE ){
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
		return TRUE;
	}
	return FALSE;
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
	GFL_G3D_TransVramTextureDataOnly
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;

	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT(( g3Dres->type==GFL_G3D_RES_TYPE_MDLTEX )||( g3Dres->type==GFL_G3D_RES_TYPE_TEX ));

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTextureDataKey( texture, &texKey, &tex4x4Key ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadTexDataOnly)\n");
			return FALSE;
		}
		//リソースへのＶＲＡＭキーの設定
		NNS_G3dTexSetTexKey( texture, texKey, tex4x4Key );

		//内部でＤＭＡ転送するので転送元をフラッシュする
		DC_FlushRange( header, header->fileSize );
		//キーを参照して、リソースをＶＲＡＭ転送
		NNS_G3dTexLoad( texture, FALSE );
		return TRUE;
	}
	return FALSE;
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
	GFL_G3D_TransVramTexturePlttOnly
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdPlttKey			plttKey;

	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT(( g3Dres->type==GFL_G3D_RES_TYPE_MDLTEX )||( g3Dres->type==GFL_G3D_RES_TYPE_TEX ));

	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexturePlttKey( texture, &plttKey ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_VramLoadPlttOnly)\n");
			return FALSE;
		}
		//リソースへのＶＲＡＭキーの設定
		NNS_G3dPlttSetPlttKey( texture, plttKey );

		//内部でＤＭＡ転送するので転送元をフラッシュする
		DC_FlushRange( header, header->fileSize );
		//キーを参照して、リソースをＶＲＡＭ転送
		NNS_G3dPlttLoad( texture, FALSE );
		return TRUE;
	}
	return FALSE;
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
	GFL_G3D_FreeVramTexture
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;
	NNSGfdPlttKey			plttKey;

	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT(( g3Dres->type==GFL_G3D_RES_TYPE_MDLTEX )||( g3Dres->type==GFL_G3D_RES_TYPE_TEX ));

	//ＶＲＡＭ転送済みかどうか確認
	if( GFL_G3D_CheckTextureKeyLive( g3Dres ) == TRUE ){

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
	return FALSE;
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
	GFL_G3D_GetTextureDataKey
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
	GFL_G3D_GetTexturePlttKey
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
	GFL_G3D_CheckTextureKeyLive
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			restex;

	GF_ASSERT( g3Dres->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT(( g3Dres->type==GFL_G3D_RES_TYPE_MDLTEX )||( g3Dres->type==GFL_G3D_RES_TYPE_TEX )); 

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
 * ３Ｄレンダー管理関数
 *
 * 　モデリングデータとテクスチャーでデータを管理する。
 * 　モデルリソースに関しては、テクスチャ貼り付けの際、その内部を書き換えるため
 * 　１つのレンダーにつき１つのモデルリソースを持つ必要がある。
 *　 全体サイズは一定。
 *
 */
//=============================================================================================
#define G3DRND_MAGICNUM	(0x7A14)
///	３Ｄレンダー制御用構造体
struct _GFL_G3D_RND
{
	u16						magicnum;
	u16						dummy;

	NNSG3dRenderObj*		rndobj;
	GFL_G3D_RES*			mdl;
	GFL_G3D_RES*			tex;
};


//--------------------------------------------------------------------------------------------
/**
 * ３Ｄレンダーの作成
 *
 * @param	mdl			参照モデルセットリソース構造体ポインタ
 * @param	mdlidx		mdl内データインデックス(複数登録されている場合。１つの場合は0)
 * @param	tex			参照テクスチャリソース構造体ポインタ(使用しない場合はNULL)
 *
 * @return	GFL_3D_RND	３Ｄレンダーハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RND*
	GFL_G3D_RENDER_Create
		( GFL_G3D_RES* mdl, int mdlidx, GFL_G3D_RES* tex )
{
	NNSG3dResMdlSet*		pMdlset;
	NNSG3dResMdl*			pMdl = NULL;
	NNSG3dResTex*			pTex = NULL;
	GFL_G3D_RND*			g3Drnd;

	//モデルデータリソースポインタ取得
	GF_ASSERT( mdl->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT( GFL_G3D_CheckResourceType( mdl, GFL_G3D_RES_CHKTYPE_MDL ) == TRUE );
	pMdlset = NNS_G3dGetMdlSet( (NNSG3dResFileHeader*)mdl->file );
	pMdl = NNS_G3dGetMdlByIdx( pMdlset, mdlidx );
	GF_ASSERT( pMdl != NULL );

	//テクスチャリソースポインタ取得
	if( tex != NULL ){	//テクスチャーなしを指定することもできる
		GF_ASSERT( tex->magicnum == G3DRES_MAGICNUM );
		GF_ASSERT( GFL_G3D_CheckResourceType( tex, GFL_G3D_RES_CHKTYPE_TEX ) == TRUE );
		pTex = NNS_G3dGetTex( (NNSG3dResFileHeader*)tex->file );
		GF_ASSERT( pTex != NULL );
	}

	//レンダーハンドルの作成
	g3Drnd = GFL_HEAP_AllocMemory( g3Dman->heapID, sizeof(GFL_G3D_RND) );
	g3Drnd->magicnum = G3DRND_MAGICNUM;
	g3Drnd->mdl = mdl;
	g3Drnd->tex = tex;

	//レンダリングオブジェクト領域の確保
	g3Drnd->rndobj = NNS_G3dAllocRenderObj( &g3Dman->allocater );
	GF_ASSERT( g3Drnd->rndobj != NULL );

	//テクスチャリソースとの関連付け
	if( pTex ){
		BOOL result = TRUE;
		result &= NNS_G3dBindMdlTex( pMdl, pTex );
		result &= NNS_G3dBindMdlPltt( pMdl, pTex );
		if( result == FALSE ){
			OS_Printf("texture bind failed\n");
		}
	}
	//レンダリングオブジェクト初期化
	NNS_G3dRenderObjInit( g3Drnd->rndobj, pMdl );

	return g3Drnd;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄレンダーの破棄
 *
 * @param	g3Drnd	３Ｄレンダーハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_RENDER_Delete
		( GFL_G3D_RND* g3Drnd ) 
{
	GF_ASSERT( g3Drnd->magicnum == G3DRND_MAGICNUM );

	NNS_G3dFreeRenderObj( &g3Dman->allocater, g3Drnd->rndobj );
	GFL_HEAP_FreeMemory( g3Drnd );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄレンダーのモデルリソースポインタを取得
 *
 * @param	g3Drnd	３Ｄレンダーハンドル
 *
 * @return	GFL_G3D_RES*
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RES*
	GFL_G3D_RENDER_GetG3DresMdl
		( GFL_G3D_RND* g3Drnd ) 
{
	GF_ASSERT( g3Drnd->magicnum == G3DRND_MAGICNUM );

	return g3Drnd->mdl;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄレンダーのモデルリソースポインタを取得
 *
 * @param	g3Drnd	３Ｄレンダーハンドル
 *
 * @return	GFL_G3D_RES*
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RES*
	GFL_G3D_RENDER_GetG3DresTex
		( GFL_G3D_RND* g3Drnd ) 
{
	GF_ASSERT( g3Drnd->magicnum == G3DRND_MAGICNUM );

	return g3Drnd->tex;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄレンダーのレンダリングオブジェクトポインタを取得
 *
 * @param	g3Drnd	３Ｄレンダーハンドル
 *
 * @return	NNSG3dRenderObj*
 *
 * 　ＮＮＳ関数を外部で使用したい場合などに使用する
 */
//--------------------------------------------------------------------------------------------
NNSG3dRenderObj*
	GFL_G3D_RENDER_GetRenderObj
		( GFL_G3D_RND* g3Drnd ) 
{
	GF_ASSERT( g3Drnd->magicnum == G3DRND_MAGICNUM );

	return g3Drnd->rndobj;
}





//=============================================================================================
/**
 *
 * ３Ｄアニメーション管理関数
 *
 * 　テクスチャアニメーション＋ジョイントアニメーション等
 * 　各種アニメーションを管理する。
 * 　このアニメーションは一つのレンダーに一種類というわけではないので
 * 　レンダーとは別の構成要素として管理する。
 * 　作成の際、対象のレンダーハンドル（アロケートバッファサイズに関係する）を
 * 　必要とするので、作成手順に注意すること。
 *
 */
//=============================================================================================
#define G3DANM_MAGICNUM	(0x59d1)
///	３Ｄアニメーション制御用構造体
struct _GFL_G3D_ANM
{
	u16				magicnum;
	u16				dummy;

	NNSG3dAnmObj*	anmobj;
	GFL_G3D_RES*	anm;
};

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄアニメーションの作成
 *
 * @param	g3Drnd		付加される対象の３Ｄレンダーハンドル
 * @param	anm			参照アニメーションリソース構造体ポインタ
 * @param	anmidx		anm内データインデックス(複数登録されている場合。１つの場合は0)
 *
 * @return	GFL_3D_ANM	３Ｄアニメーションハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_ANM*
	GFL_G3D_ANIME_Create
		( GFL_G3D_RND* g3Drnd, GFL_G3D_RES* anm, int anmidx )  
{
	GFL_G3D_ANM*	g3Danm;
	NNSG3dResMdl*	pMdl;
	NNSG3dResTex*	pTex;
	void*			pAnm;

	GF_ASSERT( g3Drnd->magicnum == G3DRND_MAGICNUM );
	GF_ASSERT( anm->magicnum == G3DRES_MAGICNUM );
	GF_ASSERT( anm->type == GFL_G3D_RES_TYPE_ANM );

	//アニメーションハンドルの作成
	g3Danm = GFL_HEAP_AllocMemory( g3Dman->heapID, sizeof(GFL_G3D_ANM) );
	g3Danm->magicnum = G3DANM_MAGICNUM;
	g3Danm->anm = anm;

	//アニメーションリソースポインタ取得
	pAnm = NNS_G3dGetAnmByIdx( anm->file, anmidx );
	GF_ASSERT( pAnm != NULL );

	pMdl = NNS_G3dRenderObjGetResMdl( g3Drnd->rndobj );
	pTex = NNS_G3dGetTex( (NNSG3dResFileHeader*)g3Drnd->tex->file );

	//アニメーションオブジェクト領域の確保
	g3Danm->anmobj = NNS_G3dAllocAnmObj( &g3Dman->allocater, pAnm, pMdl );
	GF_ASSERT( g3Danm->anmobj != NULL );
	//アニメーションオブジェクト初期化
	NNS_G3dAnmObjInit( g3Danm->anmobj, pAnm, pMdl, pTex );

	return g3Danm;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄアニメーションの破棄
 *
 * @param	g3Danm	３Ｄアニメーションハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ANIME_Delete
		( GFL_G3D_ANM* g3Danm ) 
{
	GF_ASSERT( g3Danm->magicnum == G3DANM_MAGICNUM );

	NNS_G3dFreeAnmObj( &g3Dman->allocater, g3Danm->anmobj );
	GFL_HEAP_FreeMemory( g3Danm );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄアニメーションリソースポインタを取得
 *
 * @param	g3Danm	３Ｄアニメーションハンドル
 *
 * @return	GFL_G3D_RES*
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RES*
	GFL_G3D_ANIME_GetG3Dres
		( GFL_G3D_ANM* g3Danm ) 
{
	GF_ASSERT( g3Danm->magicnum == G3DANM_MAGICNUM );

	return g3Danm->anm;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄアニメーションのアニメオブジェクトポインタを取得
 *
 * @param	g3Danm	３Ｄアニメーションハンドル
 *
 * @return	NNSG3dAnmObj*
 *
 * 　ＮＮＳ関数を外部で使用したい場合などに使用する
 */
//--------------------------------------------------------------------------------------------
NNSG3dAnmObj*
	GFL_G3D_ANIME_GetAnmObj
		( GFL_G3D_ANM* g3Danm ) 
{
	GF_ASSERT( g3Danm->magicnum == G3DANM_MAGICNUM );

	return g3Danm->anmobj;
}





//=============================================================================================
/**
 *
 * ３Ｄオブジェクト管理関数
 *
 * 　レンダー及びアニメーションによって構成される。
 * 　ステータス（座標、スケール、回転）はオブジェクトハンドルには内包しない。
 * 　※一つのオブジェクトはあくまでも描画リソースデータ郡であり
 * 　　それが１物体とは限らないため。
 * 　　　例）森を描画する際、木のオブジェクトを一つ用意し、任意のステータス配置で複数描画する。
 * 　またアニメーションは複数登録できるように構成されている。（なしも可能）
 * 　　　例）テクスチャアニメーション＋ジョイントアニメーション
 *　 そのためハンドルサイズは不定になっていることに注意すること。
 *
 */
//=============================================================================================
#define G3DOBJ_MAGICNUM	(0x2185)
///	３Ｄオブジェクト制御用構造体
struct _GFL_G3D_OBJ
{
	u16				magicnum;

	GFL_G3D_RND*	g3Drnd;
	GFL_G3D_ANM**	anmTbl;
	u16				anmCount;
};

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの作成
 *
 * @param	g3Drnd		レンダーハンドル
 * @param	anmTbl		付加するアニメーションハンドルテーブル（複数設定を可能にするため）
 * @param	anmCount	付加する、もしくは付加予定のアニメーションの数
 *
 * @return	GFL_3D_OBJ	３Ｄオブジェクトハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ*
	GFL_G3D_OBJECT_Create
		( GFL_G3D_RND* g3Drnd, GFL_G3D_ANM** anmTbl, u16 anmCount )  
{
	GFL_G3D_OBJ*	g3Dobj;
	GFL_G3D_ANM*	g3Danm;
	int	i;

	GF_ASSERT( g3Drnd->magicnum == G3DRND_MAGICNUM );

	//オブジェクトハンドルの作成
	g3Dobj = GFL_HEAP_AllocMemory( g3Dman->heapID, sizeof(GFL_G3D_OBJ) );
	g3Dobj->magicnum	= G3DOBJ_MAGICNUM;
	g3Dobj->g3Drnd		= g3Drnd;
	g3Dobj->anmCount	= anmCount;

	//アニメーション配列作成
	g3Dobj->anmTbl = GFL_HEAP_AllocClearMemory( g3Dman->heapID, sizeof(GFL_G3D_ANM*) * anmCount );
	for( i=0; i<anmCount; i++ ){
		g3Danm = anmTbl[i];
		if( g3Danm != NULL ){
			GF_ASSERT( g3Danm->magicnum == G3DANM_MAGICNUM );

			//↓この時点ではＯＮにしない(icaが複数設定されているとマージされてしまうため)
			//NNS_G3dRenderObjAddAnmObj( g3Drnd->rndobj, g3Danm->anmobj );
		}
		g3Dobj->anmTbl[i] = g3Danm;
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
	GFL_G3D_OBJECT_Delete
		( GFL_G3D_OBJ* g3Dobj ) 
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );

	GFL_HEAP_FreeMemory( g3Dobj->anmTbl );
	GFL_HEAP_FreeMemory( g3Dobj );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのレンダーハンドルポインタを取得
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 *
 * @return	GFL_G3D_RND*
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_RND*
	GFL_G3D_OBJECT_GetG3Drnd
		( GFL_G3D_OBJ* g3Dobj ) 
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );

	return g3Dobj->g3Drnd;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーションハンドルポインタを取得
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	anmIdx	登録されているアニメーションインデックス
 *
 * @return	GFL_G3D_ANM*
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_ANM*
	GFL_G3D_OBJECT_GetG3Danm
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx ) 
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	return g3Dobj->anmTbl[ anmIdx ];
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーション設定数を取得
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 *
 * @return	u16
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_OBJECT_GetAnimeCount
		( GFL_G3D_OBJ* g3Dobj ) 
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );

	return g3Dobj->anmCount;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーションを有効にする
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	anmIdx	登録されているアニメーションインデックス
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_OBJECT_EnableAnime
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx ) 
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	//レンダリングオブジェクトとの関連付け
	NNS_G3dRenderObjAddAnmObj( g3Dobj->g3Drnd->rndobj, g3Dobj->anmTbl[ anmIdx ]->anmobj );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーションを無効にする
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	anmIdx	登録されているアニメーションインデックス
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_OBJECT_DisableAnime
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx ) 
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	//レンダリングオブジェクトとの関連付けを解除
	NNS_G3dRenderObjRemoveAnmObj( g3Dobj->g3Drnd->rndobj, g3Dobj->anmTbl[ anmIdx ]->anmobj );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトにアニメーションを追加する
 *
 * @param	g3Dobj		３Ｄオブジェクトハンドル
 * @param	g3Danm		３Ｄアニメーションハンドル
 *
 * @return	u16			登録位置
 */
//--------------------------------------------------------------------------------------------
u16
	GFL_G3D_OBJECT_AddAnime
		( GFL_G3D_OBJ* g3Dobj, GFL_G3D_ANM* g3Danm )
{
	u16	i;

	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( g3Danm->magicnum == G3DANM_MAGICNUM );

	for( i=0; i<g3Dobj->anmCount; i++ ){
		if( g3Dobj->anmTbl[i] == NULL ){
			//レンダリングオブジェクトとの関連付け
			//NNS_G3dRenderObjAddAnmObj( g3Dobj->g3Drnd->rndobj, g3Danm->anmobj );
			g3Dobj->anmTbl[ i ] = g3Danm;
			return i;
		}
	}
	GF_ASSERT(0);
	return 0xffff;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトからアニメーションを削除する
 *
 * @param	g3Dobj		３Ｄオブジェクトハンドル
 * @param	anmIdx		登録されているアニメーションインデックス
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_OBJECT_RemoveAnime
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx )
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	//レンダリングオブジェクトとの関連付けを解除
	//NNS_G3dRenderObjRemoveAnmObj( g3Dobj->g3Drnd->rndobj, g3Dobj->anmTbl[ anmIdx ]->anmobj );
	g3Dobj->anmTbl[ anmIdx ] = NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーションフレームをリセット
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	anmIdx	登録されているアニメーションインデックス
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_OBJECT_ResetAnimeFrame
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx )
{
	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	g3Dobj->anmTbl[ anmIdx ]->anmobj->frame = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーションフレームを進める
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	anmIdx	登録されているアニメーションインデックス
 * @param	count	増加分（FX32_ONEで１フレーム進める）
 *
 * @return	BOOL	FALSEで終了検出
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_OBJECT_IncAnimeFrame
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx, const fx32 count ) 
{
	NNSG3dAnmObj* anmobj;

	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	anmobj = g3Dobj->anmTbl[ anmIdx ]->anmobj;
	anmobj->frame += count;
	
	if( anmobj->frame >= NNS_G3dAnmObjGetNumFrame( anmobj )){
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトのアニメーションオートループ
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	anmIdx	登録されているアニメーションインデックス
 * @param	count	増加分（FX32_ONEで１フレーム進める）
 *
 * @return	BOOL	FALSEで１ループ終了検出
 */
//--------------------------------------------------------------------------------------------
BOOL
	GFL_G3D_OBJECT_LoopAnimeFrame
		( GFL_G3D_OBJ* g3Dobj, u16 anmIdx, const fx32 count ) 
{
	NNSG3dAnmObj* anmobj;

	GF_ASSERT( g3Dobj->magicnum == G3DOBJ_MAGICNUM );
	GF_ASSERT( anmIdx < g3Dobj->anmCount );

	anmobj = g3Dobj->anmTbl[ anmIdx ]->anmobj;
	anmobj->frame += count;
	
	if( anmobj->frame >= NNS_G3dAnmObjGetNumFrame( anmobj )){
		anmobj->frame = 0;
		return FALSE;
	}
	return TRUE;
}





//=============================================================================================
//=============================================================================================
/**
 *
 * 描画関数
 *
 * 　３Ｄオブジェクトハンドルとそれらのステータス（座標、スケール、回転）を引数として描画を行う。
 *
 *	SAMPLE
 *	{
 *		GFL_G3D_DRAW_Start();							//描画開始
 *		GFL_G3D_DRAW_SetLookAt();						//カメラグローバルステート設定		
 *		{
 *			GFL_G3D_DRAW_DrawObject( g3Dobj, status );	//各オブジェクト描画
 *		}
 *		GFL_G3D_DRAW_End();								//描画終了（バッファスワップ）
 *	}
 *
 */
//=============================================================================================
static BOOL GFL_G3D_DRAW_CheckCulling( GFL_G3D_OBJ* g3Dobj );
static s32	GFL_G3D_DRAW_TestBoundingBox( const GXBoxTestParam* boundingBox );
static void GFL_G3D_DRAW_GetTRSMatrix( const GFL_G3D_OBJSTATUS* status, MtxFx43* mtxTRS );
//u32 DebugCullingCount = 0;
//u32 DebugPrintCount = 0;
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄ描画の開始
 *
 * 全体描画関数内、描画開始時に呼び出される
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_DRAW_Start
		( void )
{
	G3X_Reset();
	//DebugPrintCount = 0;
	//DebugCullingCount = 0;
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
	GFL_G3D_DRAW_End
		( void )
{
	NNS_G3dGeFlushBuffer();
	G3_SwapBuffers( g3Dman->swapBufMode.aw, g3Dman->swapBufMode.zw );
	//OS_Printf(" CullingCount is %d PrintCount is %d\n", DebugCullingCount, DebugPrintCount );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラグローバルステートの設定
 *
 * 全体描画関数内に呼び出される
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_DRAW_SetLookAt
		( void )
{
	NNS_G3dGlbLookAt( &g3Dman->lookAt.camPos, &g3Dman->lookAt.camUp, &g3Dman->lookAt.target );
	g3Dman->drawFlushFunc();	//グローバルステート反映
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの描画
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 */
//--------------------------------------------------------------------------------------------
// ３Ｄオブジェクトの情報設定
static inline void 
	statusSet( const VecFx32* pTrans, const MtxFx33* pRotate, const VecFx32* pScale )
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
//これを置き換えれば高速化されるか？↓
/*
#if 0
	statusSet( &status->trans, &status->rotate, &status->scale );
	g3Dman->drawFlushFunc();
	NNS_G3dDraw( GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd( g3Dobj ) ) );
	NNS_G3dGeFlushBuffer();
#else
	MtxFx43 mtxTRS;

	NNS_G3dGeLoadMtx43( &NNS_G3dGlb.cameraMtx );
	GFL_G3D_DRAW_GetTRSMatrix( status, &mtxTRS );
	NNS_G3dGePushMtx();
	G3_MultMtx43( &mtxTRS );
	NNS_G3dDraw( GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd( g3Dobj ) ) );
	NNS_G3dGePopMtx(1);
	NNS_G3dGeFlushBuffer();
#endif
*/

//カリングなし
void
	GFL_G3D_DRAW_DrawObject
		( GFL_G3D_OBJ* g3Dobj, const GFL_G3D_OBJSTATUS* status )
{
    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );

	statusSet( &status->trans, &status->rotate, &status->scale );
	g3Dman->drawFlushFunc();
	NNS_G3dDraw( renderobj );
	//NNS_G3dGeFlushBuffer();
}

//カリングあり
BOOL
	GFL_G3D_DRAW_DrawObjectCullingON
		( GFL_G3D_OBJ* g3Dobj, const GFL_G3D_OBJSTATUS* status )
{
    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );
	BOOL result;

	statusSet( &status->trans, &status->rotate, &status->scale );
	g3Dman->drawFlushFunc();
	result = GFL_G3D_DRAW_CheckCulling( g3Dobj );
	if( result == TRUE ){
		NNS_G3dDraw( renderobj );
		//DebugPrintCount++;
	}
	//NNS_G3dGeFlushBuffer();
	return result;
}

//--------------------------------------------------------------------------------
//アクセサリーの描画
//	対象GFL_G3D_OBJのレンダリングリソース(imd)はg3dcvtr -s で出力されている必要がある。
//	※これにより各Node行列が行列スタックに格納された状態で描画を終えるため、
//	　本体直後にこの関数を呼ぶことにより、指定Node行列を参照した状態で描画することができる。
//	　Node数の最大は30まで。コンバートの際に制限があるのもこのためらしい。
//	  また、NNS_G3DGlbFlashを呼ぶ必要はなし。
//　　カリングＯＮについては本体に依存していいかなと思っているのでとりあえず作らない。
void
	GFL_G3D_DRAW_DrawAccesory
		( GFL_G3D_OBJ* g3Dobj, GFL_G3D_OBJ* g3Dobj_Accesory,
			const GFL_G3D_OBJSTATUS* status, int jntID )
{
    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );
    NNSG3dRenderObj* renderobj_Accesory = GFL_G3D_RENDER_GetRenderObj
											( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj_Accesory) );
	BOOL result;

	// レンダリングオブジェクトのjntIDノードに対応する行列をカレント行列に持ってくる
	// ２番目の引数にMtxFx43へのポインタを指定すると位置座標行列が、
	// ３番目の引数にMtxFx33へのポインタを指定すると方向ベクトル行列が得られる。
	result = NNS_G3dGetResultMtx( renderobj, NULL, NULL, (u32)jntID );

	// jntIDノードに対応する行列がなかった場合はFALSEが返ってくるので念のためチェック
	if( result != FALSE ){
		// 描画直前にしかposScaleでのスケーリングはかからないので、
		// 明示的にスケーリングしなくてはならない。
		NNS_G3dGeTranslateVec( &status->trans );
		NNS_G3dGeScaleVec( &status->scale );
		NNS_G3dGeMultMtx33( &status->rotate );
		NNS_G3dDraw( renderobj_Accesory );
	}
	//NNS_G3dGeFlushBuffer();
}

//ジョイント名による描画
void
	GFL_G3D_DRAW_DrawAccesoryByName
		( GFL_G3D_OBJ* g3Dobj, GFL_G3D_OBJ* g3Dobj_Accesory,
			const GFL_G3D_OBJSTATUS* status, const char* jntName )
{
	int jntID;
    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );

	// ジョイントIDを取得する。検索で見つからない場合は負の値が返る。
	//define NNS_G3D_GET_JNTID(pMdl, pJntID, literal)
	jntID = NNS_G3dGetNodeIdxByName( NNS_G3dGetNodeInfo( renderobj->resMdl ), 
									(NNSG3dResName*)jntName );
	GF_ASSERT(jntID > 0);

	GFL_G3D_DRAW_DrawAccesory( g3Dobj, g3Dobj_Accesory, status, jntID );
}

//--------------------------------------------------------------------------------
// １つのモデルに１つのマテリアルのみ設定されているときに高速描画するための関数
//		matID	描画するマテリアルへのインデックス 
//		shpID	描画するシェイプへのインデックス 
//		sendMat	マテリアル情報をジオメトリエンジンに送信するかどうか 
//--------------------------------------------------------------------------------
//カリングなし
void
	GFL_G3D_DRAW_DrawObject1mat1shp
		( GFL_G3D_OBJ* g3Dobj, u32 matID, u32 shpID, BOOL sendMat, 
			const GFL_G3D_OBJSTATUS* status )

{
    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );

	statusSet( &status->trans, &status->rotate, &status->scale );
	g3Dman->drawFlushFunc();
	NNS_G3dDraw1Mat1Shp( NNS_G3dRenderObjGetResMdl( renderobj ), matID, shpID, sendMat );
	//NNS_G3dGeFlushBuffer();
}

//カリングあり
BOOL
	GFL_G3D_DRAW_DrawObject1mat1shpCullingON
		( GFL_G3D_OBJ* g3Dobj, u32 matID, u32 shpID, BOOL sendMat, 
			const GFL_G3D_OBJSTATUS* status )

{
    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );
	BOOL result;

	statusSet( &status->trans, &status->rotate, &status->scale );
	g3Dman->drawFlushFunc();
	result = GFL_G3D_DRAW_CheckCulling( g3Dobj );
	if( result == TRUE ){
		NNS_G3dDraw1Mat1Shp( NNS_G3dRenderObjGetResMdl( renderobj ), matID, shpID, sendMat );
	}
	//NNS_G3dGeFlushBuffer();
	return result;
}

//--------------------------------------------------------------------------------------------
/**
 * カリングチェック
 *
 * オブジェクトのグローバルステータス反映後に呼ぶこと
 */
//--------------------------------------------------------------------------------------------
static BOOL
	GFL_G3D_DRAW_CheckCulling
		( GFL_G3D_OBJ* g3Dobj )
{
	NNSG3dResMdlInfo*	modelInfo;		// モデルデータからのボックステスト用パラメータ取得用
	BOOL				result = TRUE;
	GXBoxTestParam		boundingBox;

	//ボックステスト用パラメータ取得
	modelInfo = NNS_G3dGetMdlInfo
	( NNS_G3dRenderObjGetResMdl(GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(g3Dobj))));

	//バウンディングボックス作成
	boundingBox.x		= modelInfo->boxX;
	boundingBox.y		= modelInfo->boxY;
	boundingBox.z		= modelInfo->boxZ;
	boundingBox.width	= modelInfo->boxW;
	boundingBox.height	= modelInfo->boxH;
	boundingBox.depth	= modelInfo->boxD;

	//上のボックステストの箱の値をposScaleでかけることにより本当の値になる
	NNS_G3dGePushMtx();
	NNS_G3dGeScale( modelInfo->boxPosScale, modelInfo->boxPosScale, modelInfo->boxPosScale );
	
	//チェック
	if( !GFL_G3D_DRAW_TestBoundingBox( &boundingBox ) ){
		result = FALSE;
		//DebugCullingCount++;
	}
	NNS_G3dGePopMtx(1);

	return result;
}

//----------------------------------------------------------------------------
/**
 * ボックス視体積内外テスト
 *
 * @param	boundingBox		ボックステスト用パラメータへのポインタ
 *
 * @return	s32				0以外の値であれば、ボックスの一部または全部が視体積内
 *							0であれば、ボックスの全てが視体積外 
 */
//-----------------------------------------------------------------------------
static s32
	GFL_G3D_DRAW_TestBoundingBox
		( const GXBoxTestParam* boundingBox )
{
	s32 testResult = 1;

	//ポリゴン属性の「FAR面交差クリッピングフラグ」と「１ドットポリゴン表示フラグ」を1に設定
	//※ライブラリからの要求
	NNS_G3dGePolygonAttr( GX_LIGHTMASK_0, GX_POLYGONMODE_MODULATE, GX_CULL_NONE,           
					0, 0, GX_POLYGON_ATTR_MISC_FAR_CLIPPING | GX_POLYGON_ATTR_MISC_DISP_1DOT );

	//ポリゴンアトリビュート反映 ※ライブラリ指定手順
	NNS_G3dGeBegin( GX_BEGIN_TRIANGLES );							
	NNS_G3dGeEnd();	

	NNS_G3dGeBoxTest( boundingBox );	// ボックステスト
	NNS_G3dGeFlushBuffer();				// 同期を取る

	//チェック本体（返り値が0の場合テスト終了）。 0:視体積内、それ以外の値は視体積外
	while ( G3X_GetBoxTestResult( &testResult ) != 0 ) ;

	return testResult;
}

//--------------------------------------------------------------------------------------------
/**
 * TRSマトリクスを取得
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void 
	GFL_G3D_DRAW_GetTRSMatrix
		( const GFL_G3D_OBJSTATUS* status, MtxFx43* mtxTRS )
{
	MtxFx43	rot;
	MtxFx43 scl;

	MTX_Identity43( mtxTRS );
	MTX_TransApply43( mtxTRS, mtxTRS, status->trans.x, status->trans.y, status->trans.z);
	MTX_Copy33To43( &status->rotate, &rot );
	MTX_Scale43( &scl, status->scale.x, status->scale.y, status->scale.z);
	MTX_Concat43( &rot, mtxTRS, mtxTRS );
	MTX_Concat43( &scl, mtxTRS, mtxTRS );
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



