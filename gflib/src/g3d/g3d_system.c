//=============================================================================================
/**
 * @file	g3d_system.c                                                  
 * @brief	３Ｄ描画管理システムプログラム
 * @date	2006/4/26
 */
//=============================================================================================
#include "gflib.h"
#include "g3d_system.h"

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
/**
 * @brief	３Ｄシステムマネージャ構造体
 */
typedef struct GFL_G3D_MAN_tag
{
	NNSFndAllocator		allocater;	///<メモリアロケータ(NNSで使用)
	void*				plt_memory;	///<パレットマネージャ用メモリ
	void*				tex_memory;	///<テクスチャマネージャ用メモリ
	u16					heapID;		///<ヒープＩＤ
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
				u16 GeBufSize, u16 heapID, GFL_G3D_SETUP_FUNC setup )
{
	int	tex_size,plt_size;
	g3Dman = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_MAN) );

	g3Dman->heapID = heapID;

	// ジオメトリコマンドバッファの設定
	if( GeBufSize ){
		NNSG3dGeBuffer* geBuf;

		if( GeBufSize > GEBUF_SIZE_MAX ){
			GeBufSize = GEBUF_SIZE_MAX;
		}
		geBuf = ( NNSG3dGeBuffer* )GFL_HEAP_DTCM_AllocMemory( GeBufSize );
		NNS_G3dGeSetBuffer( geBuf );
	}

	// NitroSystem:３Ｄエンジンの初期化
	NNS_G3dInit();
	// マトリクススタックの初期化
    G3X_InitMtxStack();

	// ジオメトリエンジン起動後必ず呼ばれなければならない
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

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
	RES_TYPE_UNKNOWN = 0,	//不明なデータ
	RES_TYPE_MDLTEX,		//ファイル内にモデリングおよびテクスチャデータ内包
	RES_TYPE_MDL,			//ファイル内にモデリングデータ内包
	RES_TYPE_TEX,			//ファイル内にテクスチャデータ内包
	RES_TYPE_ANM,			//ファイル内にアニメーションデータ内包
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
static BOOL
	GFL_G3D_ObjTexkeyLiveCheck( NNSG3dResTex* restex );

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

	OS_Printf("3D_resource check now...\n");
	//ファイルタイプの判別
	switch( *(u32*)&header[0] )
	{
		case NNS_G3D_SIGNATURE_NSBMD:
			OS_Printf("nsbmd file check...\n");
			if( NNS_G3dGetTex( header ) == NULL ){
				OS_Printf("this 3D_resource is model_data\n");
				g3Dres->type = RES_TYPE_MDL;	//モデリングデータ内包
			} else {
				OS_Printf("this 3D_resource is model_data & texture_data\n");
				g3Dres->type = RES_TYPE_MDLTEX;	//モデリングおよびテクスチャデータ内包
			}
			break;
		case NNS_G3D_SIGNATURE_NSBTX:
			OS_Printf("this 3D_resource is texture_data\n");
			g3Dres->type = RES_TYPE_TEX;		//テクスチャデータ内包
			break;
		case NNS_G3D_SIGNATURE_NSBCA:
		case NNS_G3D_SIGNATURE_NSBVA:
		case NNS_G3D_SIGNATURE_NSBMA:
		case NNS_G3D_SIGNATURE_NSBTP:
		case NNS_G3D_SIGNATURE_NSBTA:
			OS_Printf("this 3D_resource is animetion_data\n");
			g3Dres->type = RES_TYPE_ANM;		//ファイル内にアニメーションデータ内包
			break;
		default:
			OS_Printf("this 3D_resource is unknown\n");
			g3Dres->type = RES_TYPE_UNKNOWN;	//不明なデータ
			break;
	}
	g3Dres->magicnum = G3DRES_MAGICNUM;
	//ファイルポインタの設定
	g3Dres->file = ( void* )header;
	OS_Printf("3D_resource is loaded\n");

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
	OS_Printf("3D_resource loading...\n");
	//対象アーカイブファイルからヘッダデータを読み込み
	header = GFL_ARC_DataLoadFilePathMalloc( path, datID, g3Dman->heapID );

	return GFL_G3D_ResourceCreate( header );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄリソースの破棄
 *
 * @param	g3Dres	３Ｄリソースポインタ
 *
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
 * テクスチャリソースの転送(→ＶＲＡＭ)
 *
 * @param	g3Dres	３Ｄリソースポインタ
 *
 * @return	BOOL	結果(成功=TRUE)
 */
//--------------------------------------------------------------------------------------------
BOOL 
	GFL_G3D_TransVramTex
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;
	NNSGfdPlttKey			plttKey;

	if( G3DRES_FILE_CHECK( g3Dres ) == FALSE ){
		OS_Printf("file is not 3D_resource (GFL_G3D_TransTex)\n");
		return FALSE;
	}
	if(( g3Dres->type != RES_TYPE_MDLTEX )&&( g3Dres->type != RES_TYPE_TEX )){
		OS_Printf("file is not texture_resource (GFL_G3D_TransTex)\n");
		return FALSE;
	}
	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexDataVramkey( texture, &texKey, &tex4x4Key ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_TransTex)\n");
			return FALSE;
		}
		if( GFL_G3D_GetTexPlttVramkey( texture, &plttKey ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_TransTex)\n");
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
 * テクスチャデータリソースの転送(→ＶＲＡＭ)
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
	GFL_G3D_TransVramTexDataOnly
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdTexKey			texKey, tex4x4Key;

	if( G3DRES_FILE_CHECK( g3Dres ) == FALSE ){
		OS_Printf("file is not 3D_resource (GFL_G3D_TransTexDataOnly)\n");
		return FALSE;
	}
	if(( g3Dres->type != RES_TYPE_MDLTEX )&&( g3Dres->type != RES_TYPE_TEX )){
		OS_Printf("file is not texture_resource (GFL_G3D_TransTexDataOnly)\n");
		return FALSE;
	}
	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexDataVramkey( texture, &texKey, &tex4x4Key ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_TransTexDataOnly)\n");
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
 * テクスチャパレットリソースの転送(→ＶＲＡＭ)
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
	GFL_G3D_TransVramTexPlttOnly
		( GFL_G3D_RES* g3Dres )
{
	NNSG3dResFileHeader*	header;
	NNSG3dResTex*			texture;
	NNSGfdPlttKey			plttKey;

	if( G3DRES_FILE_CHECK( g3Dres ) == FALSE ){
		OS_Printf("file is not 3D_resource (GFL_G3D_TransPlttOnly)\n");
		return FALSE;
	}
	if(( g3Dres->type != RES_TYPE_MDLTEX )&&( g3Dres->type != RES_TYPE_TEX )){
		OS_Printf("file is not texture_resource (GFL_G3D_TransPlttOnly)\n");
		return FALSE;
	}
	//テクスチャリソースポインタの取得
	header = (NNSG3dResFileHeader*)g3Dres->file;
	texture = NNS_G3dGetTex( header ); 

	if( texture ){
		//リソースを転送するためのＶＲＡＭキーの取得
		if( GFL_G3D_GetTexPlttVramkey( texture, &plttKey ) == FALSE ){
			OS_Printf("Vramkey cannot alloc (GFL_G3D_TransPlttOnly)\n");
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
static BOOL
	GFL_G3D_ObjTexkeyLiveCheck
		( NNSG3dResTex* restex )
{
	// 普通のテクスチャか4*4テクセル圧縮テクスチャ
	// でVramに展開されているかをチェック
	if((restex->texInfo.flag & NNS_G3D_RESTEX_LOADED) ||
	   (restex->tex4x4Info.flag & NNS_G3D_RESTEX4x4_LOADED)){
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
	u8						drawSW;
	u8						anmSW;

	NNSG3dRenderObj*		rndobj;
	NNSG3dAnmObj*			anmobj;

	NNSG3dResMdl*			pMdl;
	NNSG3dResTex*			pTex;
	void*					pAnm;

	VecFx32					trans;			//座標
	VecFx32					scale;			//スケール
	VecFx32					rotate;			//回転	
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
	   (( mdl->type == RES_TYPE_MDLTEX )||( mdl->type == RES_TYPE_MDL )) ){
		//モデルデータリソースポインタ取得
		header = (NNSG3dResFileHeader*)mdl->file;
		pMdlset = NNS_G3dGetMdlSet( header );
		pMdl = NNS_G3dGetMdlByIdx( pMdlset, mdlidx );
	} else {
		pMdl = NULL;
	}
	if(( G3DRES_FILE_CHECK( tex ) == TRUE )&&
	   (( tex->type == RES_TYPE_MDLTEX )||( tex->type == RES_TYPE_TEX )) ){
		//テクスチャリソースポインタ取得
		header = (NNSG3dResFileHeader*)tex->file;
		pTex = NNS_G3dGetTex( header );
	} else {
		pTex = NULL;
	}
	if(( G3DRES_FILE_CHECK( anm ) == TRUE )&&( anm->type == RES_TYPE_ANM )){
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

	//配置パラメータ初期化
	{
		VecFx32 init_trans	= { 0, 0, 0 };
		VecFx32 init_scale	= { FX32_ONE, FX32_ONE, FX32_ONE };
		VecFx32 init_rotate = { 0, 0, 0 };

		g3Dobj->trans	= init_trans;
		g3Dobj->scale	= init_scale;
		g3Dobj->rotate	= init_rotate;
	}

	//描画スイッチ初期化
	g3Dobj->drawSW = 0; 
	g3Dobj->anmSW = 0;

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
 *
 */
//=============================================================================================
static inline void G3DOBJ_DRAWCALC( VecFx32* pTrans, MtxFx33* pRotate, VecFx32* pScale )
{
	// 位置設定
	NNS_G3dGlbSetBaseTrans( pTrans );
	// 角度設定
	NNS_G3dGlbSetBaseRot( pRotate );
	// スケール設定
	NNS_G3dGlbSetBaseScale( pScale );
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの描画の開始
 *
 * グローバルステート
 * （射影変換行列、カメラ行列、モデリング行列、ビューポート、ライト設定、マテリアルカラー等）
 * をジオメトリエンジンに送信する。
 * 特にこのステートに外部加工をしていなければ、各オブジェクト描画関数(ObjDraw)直前に
 * いずれかの関数を毎回呼び出すことになる。
 */
//--------------------------------------------------------------------------------------------
// カレント射影行列に射影変換行列が、
// カレント位置座標行列と方向ベクトル行列にカメラ行列とモデリング行列が合成された行列
// が設定されます。
void
	GFL_G3D_ObjDrawStart
		( void ) 
{
	NNS_G3dGlbFlush();
}

// カレント射影行列に射影変換行列とカメラ行列が合成された行列が、
// カレント位置座標行列と方向ベクトル行列にモデリング行列
// が設定されます。
void
	GFL_G3D_ObjDrawStartVP
		( void ) 
{
	NNS_G3dGlbFlushVP();
}

// カレント射影行列に射影変換行列とカメラ行列とモデリング行列が合成された行列が、
// カレント位置座標行列と方向ベクトル行列に単位行列
// が設定されます。
void
	GFL_G3D_ObjDrawStartWVP
		( void ) 
{
	NNS_G3dGlbFlushWVP();
}

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの描画
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	rotate	回転行列
 *
 * 通常描画関数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjDraw
		( GFL_G3D_OBJ* g3Dobj, MtxFx33* rotate )
{
	G3DOBJ_DRAWCALC( &g3Dobj->trans, rotate, &g3Dobj->scale );

	NNS_G3dDraw( g3Dobj->rndobj );
	NNS_G3dGeFlushBuffer();
}
		
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの描画(1mat1shape)
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	rotate	回転行列
 *
 * １つのモデルに１つのマテリアルのみ設定されているときに高速描画するための関数
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_ObjDraw1mat1shape
		( GFL_G3D_OBJ* g3Dobj, MtxFx33* rotate )
{
	G3DOBJ_DRAWCALC( &g3Dobj->trans, rotate, &g3Dobj->scale );

    NNS_G3dDraw1Mat1Shp( g3Dobj->pMdl, 0, 0, TRUE );
	NNS_G3dGeFlushBuffer();
}
		
//--------------------------------------------------------------------------------------------
/**
 * ３Ｄオブジェクトの回転行列の作成
 *
 * @param	g3Dobj	３Ｄオブジェクトハンドル
 * @param	dst		計算後の回転行列格納ポインタ
 *
 * この関数等を使用し、オブジェクト毎に適切な回転行列を作成したものを、描画に流す。
 */
//--------------------------------------------------------------------------------------------
// Ｘ→Ｙ→Ｚの順番で計算
void
	GFL_G3D_ObjDrawRotateCalcXY
		( GFL_G3D_OBJ* g3Dobj, MtxFx33* rotate )
{
	VecFx32* src = &g3Dobj->rotate; 
	MtxFx33 tmp;

	MTX_RotY33(	rotate, FX_SinIdx((u16)src->x), FX_CosIdx((u16)src->x) );

	MTX_RotX33(	&tmp, FX_SinIdx((u16)src->y), FX_CosIdx((u16)src->y) );
	MTX_Concat33( rotate, &tmp, rotate );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)src->z), FX_CosIdx((u16)src->z) );
	MTX_Concat33( rotate, &tmp, rotate );
}

// Ｘ→Ｙ→Ｚの順番で計算（相対）
void
	GFL_G3D_ObjDrawRotateCalcXY_Rev
		( GFL_G3D_OBJ* g3Dobj, MtxFx33* rotate )
{
	VecFx32* src = &g3Dobj->rotate; 
	MtxFx33 tmp;

	MTX_RotY33(	rotate, FX_SinIdx((u16)src->y), FX_CosIdx((u16)src->y) );

	MTX_RotX33(	&tmp, FX_SinIdx((u16)-src->x), FX_CosIdx((u16)-src->x) );
	MTX_Concat33( rotate, &tmp, rotate );

	MTX_RotZ33(	&tmp, FX_CosIdx((u16)src->z), FX_SinIdx((u16)src->z) );
	MTX_Concat33( rotate, &tmp, rotate );
}

// Ｙ→Ｘ→Ｚの順番で計算
void
	GFL_G3D_ObjDrawRotateCalcYX
		( GFL_G3D_OBJ* g3Dobj, MtxFx33* rotate )
{
	VecFx32* src = &g3Dobj->rotate; 
	MtxFx33 tmp;

	MTX_RotY33(	rotate, FX_SinIdx((u16)src->y), FX_CosIdx((u16)src->y) );

	MTX_RotX33(	&tmp, FX_SinIdx((u16)src->x), FX_CosIdx((u16)src->x) );
	MTX_Concat33( rotate, &tmp, rotate );

	MTX_RotZ33(	&tmp, FX_SinIdx((u16)src->z), FX_CosIdx((u16)src->z) );
	MTX_Concat33( rotate,&tmp, rotate );
}

// Ｙ→Ｘ→Ｚの順番で計算（相対）
void
	GFL_G3D_ObjDrawRotateCalcYX_Rev
		( GFL_G3D_OBJ* g3Dobj, MtxFx33* rotate )
{
	VecFx32* src = &g3Dobj->rotate; 
	MtxFx33 tmp;

	MTX_RotY33(	rotate, FX_SinIdx((u16)src->x), FX_CosIdx((u16)src->x) );

	MTX_RotX33(	&tmp, FX_SinIdx((u16)-src->y), FX_CosIdx((u16)-src->y) );
	MTX_Concat33( rotate, &tmp, rotate );

	MTX_RotZ33(	&tmp,FX_CosIdx((u16)src->z), FX_SinIdx((u16)src->z) );
	MTX_Concat33( rotate, &tmp, rotate );
}








