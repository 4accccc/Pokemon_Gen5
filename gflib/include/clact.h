//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		clact.h
 *	@brief		セルアクターシステム
 *	@author		tomoya takahashi
 *	@data		2006.11.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __CLACT_H__
#define __CLACT_H__

#undef GLOBAL
#ifdef	__CLACT_H_GLOBAL
#define	GLOBAL	/* */
#else
#define	GLOBAL	extern
#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	CLSYS描画面タイプ
//	レンダラーのサーフェース設定や、
//	Vramアドレス指定に使用します。
//=====================================
typedef enum {
	CLSYS_DRAW_MAIN,// メイン画面
	CLSYS_DRAW_SUB,	// サブ画面
	CLSYS_DRAW_MAX,	// タイプ最大数
} CLSYS_DRAW_TYPE;


//-------------------------------------
///	NNSG2dRendererAffineTypeOverwiteModeこの型名は長いので変更
//	セルアトリビュートのアフィン変換フラグを上書きするフラグ
//=====================================
typedef NNSG2dRendererAffineTypeOverwiteMode	CLSYS_AFFINETYPE;
enum{
	CLSYS_AFFINETYPE_NONE	= NNS_G2D_RND_AFFINE_OVERWRITE_NONE,  // 上書きしない
	CLSYS_AFFINETYPE_NORMAL = NNS_G2D_RND_AFFINE_OVERWRITE_NORMAL,// 通常のアフィン変換方式に設定
	CLSYS_AFFINETYPE_DOUBLE = NNS_G2D_RND_AFFINE_OVERWRITE_DOUBLE,// 倍角アフィン変換方式に設定
	CLSYS_AFFINETYPE_NUM
};

//-------------------------------------
///	NNSG2dAnimationPlayMode	この型名は長いので変更
//	セルアニメーションのアニメ方式を上書指定するフラグ
//=====================================
typedef NNSG2dAnimationPlayMode		CLSYS_ANM_PLAYMODE;
enum{
	CLSYS_ANMPM_INVALID		= NNS_G2D_ANIMATIONPLAYMODE_INVALID,		// 無効
	CLSYS_ANMPM_FORWARD		= NNS_G2D_ANIMATIONPLAYMODE_FORWARD,        // ワンタイム再生(順方向)
	CLSYS_ANMPM_FORWARD_L	= NNS_G2D_ANIMATIONPLAYMODE_FORWARD_LOOP,   // リピート再生(順方向ループ)
	CLSYS_ANMPM_REVERSE		= NNS_G2D_ANIMATIONPLAYMODE_REVERSE,        // 往復再生(リバース（順＋逆方向）
	CLSYS_ANMPM_REVERSE_L	= NNS_G2D_ANIMATIONPLAYMODE_REVERSE_LOOP,   // 往復再生リピート（リバース（順＋逆順方向） ループ）
	CLSYS_ANMPM_MAX
};


//-------------------------------------
///	フリップタイプ
//	CLWKへのフリップ設定に使用します。
//=====================================
typedef enum {
	CLWK_FLIP_V,		// Vフリップ状態
	CLWK_FLIP_H,		// Hフリップ状態
	CLWK_FLIP_MAX
} CLWK_FLIP_TYPE;

//-------------------------------------
///	座標タイプ
//	座標の個別設定を行うときに使用します。
//=====================================
typedef enum{
	CLSYS_MAT_X,			// X座標
	CLSYS_MAT_Y,			// Y座標
	CLSYS_MAT_MAX
} CLSYS_MAT_TYPE;

//-------------------------------------
///	設定サーフェース　絶対座標指定
//	CLWKの座標設定関数で絶対座標を設定するときに使用します。
//=====================================
#define CLWK_SETSF_NONE	(0xffff)

//-------------------------------------
///	Vramアドレス取得失敗
//	GFL_CLACT_WkGetPlttAddr
//	GFL_CLACT_WkGetCharAddr
//	の戻り値
//=====================================
#define CLWK_VRAM_ADDR_NONE	( 0xffffffff )




//-----------------------------------------------------------------------------
/**
 *					外部非公開システム構造体
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	ユーザー定義レンダラー
//=====================================
typedef struct _CLSYS_REND	CLSYS_REND;

//-------------------------------------
///	セルアクターユニット
//=====================================
typedef struct _CLUNIT		CLUNIT;

//-------------------------------------
///	セルアクターワーク
//=====================================
typedef struct _CLWK		CLWK;





//-----------------------------------------------------------------------------
/**
 *					初期化データ構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS初期化データ
//=====================================
typedef struct {
	s16 surface_main_left;	// メイン　サーフェースの左上座標
	s16 surface_main_top;	// メイン　サーフェースの左上座標
	s16 surface_sub_left;	// サブ　サーフェースの左上座標
	s16 surface_sub_top;	// サブ　サーフェースの左上座標
	u8	oamst_main;			// メイン画面OAM管理開始位置
	u8	oamnum_main;		// メイン画面OAM管理数
	u8	oamst_sub;			// サブ画面OAM管理開始位置
	u8	oamnum_sub;			// サブ画面OAM管理数
	u8	tr_cell;			// セルVram転送管理数
} CLSYS_INIT;


//-------------------------------------
///	独自レンダラー作成用
/// サーフェースデータ構造体
//=====================================
typedef struct {
	s16	lefttop_x;			// サーフェース左上ｘ座標
	s16	lefttop_y;			// サーフェース左上ｙ座標
	s16	width;				// サーフェース幅
	s16	height;				// サーフェース高さ
	CLSYS_DRAW_TYPE	type;	// サーフェースタイプ(CLSYS_DRAW_TYPE)
} REND_SURFACE_INIT;

//-------------------------------------
///	CLWK初期化リソースデータ
//	それぞれのアニメ方法のデータを設定する関数も用意されています。
//	・セルアニメ			GFL_CLACT_WkSetCellResData(...)
//	・Vram転送セルアニメ	GFL_CLACT_WkSetTrCellResData(...)
//	・マルチセルアニメ		GFL_CLACT_WkSetMCellResData(...)
//=====================================
typedef struct {
	const NNSG2dImageProxy*			cp_img;		// イメージプロクシ
	const NNSG2dImagePaletteProxy*	cp_pltt;	// パレットプロクシ
	NNSG2dCellDataBank*				p_cell;		// セルデータ
    const NNSG2dCellAnimBankData*   cp_canm;	// セルアニメーション

	// 以下は必要なときだけ値を入れる
	const NNSG2dMultiCellDataBank*  cp_mcell;	// マルチセルデータ		（無いときNULL）
    const NNSG2dMultiCellAnimBankData* cp_mcanm;// マルチセルアニメーション（無いときNULL）
	const NNSG2dCharacterData*		cp_char;	// Vram転送セルアニメ以外はNULL
} CLWK_RES;


//-------------------------------------
///	CLWK初期化基本データ
//=====================================
typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
} CLWK_DATA;

//-------------------------------------
///	CLWK初期化アフィン変換データ
//=====================================
typedef struct {
	CLWK_DATA clwkdata;		// 基本データ
	
	s16	affinepos_x;		// アフィンｘ座標
	s16 affinepos_y;		// アフィンｙ座標
	fx32 scale_x;			// 拡大ｘ値
	fx32 scale_y;			// 拡大ｙ値
	u16	rotation;			// 回転角度(0～0xffff 0xffffが360度)
	u16	affine_type;		// 上書きアフィンタイプ（CLSYS_AFFINETYPE）
} CLWK_AFFINEDATA;


//-------------------------------------
///	CLWK　座標構造体	
//=====================================
typedef struct {
	s16 x;
	s16 y;
} CLSYS_POS;

//-------------------------------------
///	CLWK　スケール構造体
//=====================================
typedef struct {
	fx32 x;
	fx32 y;
} CLSYS_SCALE;





//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS関係
//=====================================
GLOBAL void GFL_CLACT_SysInit( const CLSYS_INIT* cp_data, HEAPID heapID );
GLOBAL void GFL_CLACT_SysExit( void );
GLOBAL void GFL_CLACT_SysMain( void );
GLOBAL void GFL_CLACT_SysVblank( void );

//-------------------------------------
///	USER定義レンダラー関係
// 自分独自のサーフェース設定をした
// レンダラーを使用したいとき、この関数郡で作成して、
// セルアクターユニットに関連付けることが出来ます。
//=====================================
GLOBAL CLSYS_REND* GFL_CLACT_UserRendCreate( const REND_SURFACE_INIT* cp_data, u32 data_num, HEAPID heapID );
GLOBAL void GFL_CLACT_UserRendDelete( CLSYS_REND* p_rend );
GLOBAL void GFL_CLACT_UserRendSetSurfacePos( CLSYS_REND* p_rend, u32 idx, const CLSYS_POS* cp_pos );
GLOBAL void GFL_CLACT_UserRendGetSurfacePos( const CLSYS_REND* cp_rend, u32 idx, CLSYS_POS* p_pos );
GLOBAL void GFL_CLACT_UserRendSetSurfaceSize( CLSYS_REND* p_rend, u32 idx, const CLSYS_POS* cp_size );
GLOBAL void GFL_CLACT_UserRendGetSurfaceSize( const CLSYS_REND* cp_rend, u32 idx, CLSYS_POS* p_size );
GLOBAL void GFL_CLACT_UserRendSetSurfaceType( CLSYS_REND* p_rend, u32 idx, CLSYS_DRAW_TYPE type );
GLOBAL CLSYS_DRAW_TYPE GFL_CLACT_UserRendGetSurfaceType( const CLSYS_REND* cp_rend, u32 idx );

//-------------------------------------
///	CLUNIT関係
//=====================================
GLOBAL CLUNIT* GFL_CLACT_UnitCreate( u16 wknum, HEAPID heapID );
GLOBAL void GFL_CLACT_UnitDelete( CLUNIT* p_unit );
GLOBAL void GFL_CLACT_UnitDraw( CLUNIT* p_unit );
GLOBAL void GFL_CLACT_UnitSetDrawFlag( CLUNIT* p_unit, BOOL on_off );
GLOBAL BOOL GFL_CLACT_UnitGetDrawFlag( const CLUNIT* cp_unit );
GLOBAL void GFL_CLACT_UnitSetUserRend( CLUNIT* p_unit, CLSYS_REND* p_rend );
GLOBAL void GFL_CLACT_UnitSetDefaultRend( CLUNIT* p_unit );

//-------------------------------------
///	CLWK関係
//
/*	【setsfの説明】
 *		CLUNITの使用するレンダラーシステムを変更していないときは
 *		CLSYS_DRAW_TYPEの値を指定する
 *		・CLSYS_DRAW_MAIN指定時	pos_x/yがメイン画面左上座標からの相対座標になる。
 *		・CLSYS_DRAW_SUB指定時	pos_x/yがサブ画面左上座標からの相対座標になる。
 *		
 *		独自のレンダラーシステムをCLUNITに設定しているときは、
 *		サーフェースの要素数を指定することで、
 *		指定されたサーフェース左上座標からの相対座標になる。
 *
 *		通常/独自レンダラーシステム共通で、
 *		CLWK_SETSF_NONEを指定すると絶対座標設定になる
 */
//=====================================
// 初期化リソースデータ設定関数
GLOBAL void GFL_CLACT_WkSetCellResData( CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm );
GLOBAL void GFL_CLACT_WkSetTrCellResData( CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm, const NNSG2dCharacterData* cp_char );
GLOBAL void GFL_CLACT_WkSetMCellResData( CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm, const NNSG2dMultiCellDataBank* cp_mcell, const NNSG2dMultiCellAnimBankData* cp_mcanm );

// 登録破棄関係
GLOBAL CLWK* GFL_CLACT_WkAdd( CLUNIT* p_unit, const CLWK_DATA* cp_data, const CLWK_RES* cp_res, u16 setsf, HEAPID heapID );
GLOBAL CLWK* GFL_CLACT_WkAddAffine( CLUNIT* p_unit, const CLWK_AFFINEDATA* cp_data, const CLWK_RES* cp_res, u16 setsf, HEAPID heapID );
GLOBAL void GFL_CLACT_WkDel( CLWK* p_wk );

// 描画フラグ
GLOBAL void GFL_CLACT_WkSetDrawFlag( CLWK* p_wk, BOOL on_off );
GLOBAL BOOL GFL_CLACT_WkGetDrawFlag( const CLWK* cp_wk );
// サーフェース内相対座標
GLOBAL void GFL_CLACT_WkSetPos( CLWK* p_wk, const CLSYS_POS* cp_pos, u16 setsf );
GLOBAL void GFL_CLACT_WkGetPos( const CLWK* cp_wk, CLSYS_POS* p_pos, u16 setsf );
GLOBAL void GFL_CLACT_WkSetTypePos( CLWK* p_wk, s16 pos, u16 setsf, CLSYS_MAT_TYPE type );
GLOBAL s16 GFL_CLACT_WkGetTypePos( const CLWK* cp_wk, u16 setsf, CLSYS_MAT_TYPE type );
// 絶対座標
GLOBAL void GFL_CLACT_WkSetWldPos( CLWK* p_wk, const CLSYS_POS* cp_pos );
GLOBAL void GFL_CLACT_WkGetWldPos( const CLWK* cp_wk, CLSYS_POS* p_pos );
GLOBAL void GFL_CLACT_WkSetWldTypePos( CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type );
GLOBAL s16 GFL_CLACT_WkGetWldTypePos( const CLWK* cp_wk, CLSYS_MAT_TYPE type );
// アフィンパラメータ
GLOBAL void GFL_CLACT_WkSetAffineParam( CLWK* p_wk, CLSYS_AFFINETYPE affine );
GLOBAL CLSYS_AFFINETYPE GFL_CLACTWkGetAffineParam( const CLWK* cp_wk );
// アフィン座標
GLOBAL void GFL_CLACT_WkSetAffinePos( CLWK* p_wk, const CLSYS_POS* cp_pos );
GLOBAL void GFL_CLACT_WkGetAffinePos( const CLWK* cp_wk, CLSYS_POS* p_pos );
GLOBAL void GFL_CLACT_WkSetTypeAffinePos( CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type );
GLOBAL s16 GFL_CLACT_WkGetTypeAffinePos( const CLWK* cp_wk, CLSYS_MAT_TYPE type );
// 拡大縮小
GLOBAL void GFL_CLACT_WkSetScale( CLWK* p_wk, const CLSYS_SCALE* cp_sca );
GLOBAL void GFL_CLACT_WkGetScale( const CLWK* cp_wk, CLSYS_SCALE* p_sca );
GLOBAL void GFL_CLACT_WkSetTypeScale( CLWK* p_wk, fx32 scale, CLSYS_MAT_TYPE type );
GLOBAL fx32 GFL_CLACT_WkGetTypeScale( const CLWK* cp_wk, CLSYS_MAT_TYPE type );
// 回転
GLOBAL void GFL_CLACT_WkSetRotation( CLWK* p_wk, u16 rotation );
GLOBAL u16  GFL_CLACT_WkGetRotation( const CLWK* cp_wk );
// フリップ
GLOBAL void GFL_CLACT_WkSetFlip( CLWK* p_wk, CLWK_FLIP_TYPE flip_type, BOOL on_off );
GLOBAL BOOL GFL_CLACT_WkGetFlip( const CLWK* cp_wk, CLWK_FLIP_TYPE flip_type );
// オブジェクトモード
GLOBAL void GFL_CLACT_WkSetObjMode( CLWK* p_wk, GXOamMode mode );
GLOBAL GXOamMode GFL_CLACT_WkGetObjMode( const CLWK* cp_wk );
// モザイク
GLOBAL void GFL_CLACT_WkSetMosaic( CLWK* p_wk, BOOL on_off );
GLOBAL BOOL GFL_CLACT_WkGetMosaic( const CLWK* cp_wk );
// パレット
GLOBAL void GFL_CLACT_WkSetPlttOffs( CLWK* p_wk, u8 pal_offs );
GLOBAL u8 GFL_CLACT_WkGetPlttOffs( const CLWK* cp_wk );
GLOBAL u32 GFL_CLACT_WkGetPlttAddr( const CLWK* cp_wk, CLSYS_DRAW_TYPE type );
GLOBAL void GFL_CLACT_WkSetPlttProxy( CLWK* p_wk, const NNSG2dImagePaletteProxy* cp_pltt );
// キャラクタ
GLOBAL u32 GFL_CLACT_WkGetCharAddr( const CLWK* cp_wk, CLSYS_DRAW_TYPE type );
GLOBAL void GFL_CLACT_WkSetImgProxy( CLWK* p_wk, const NNSG2dImageProxy* cp_img );
// ソフト優先順位
GLOBAL void GFL_CLACT_WkSetSoftPri( CLWK* p_wk, u8 pri );
GLOBAL u8 GFL_CLACT_WkGetSoftPri( const CLWK* cp_wk );
// BG優先順位
GLOBAL void GFL_CLACT_WkSetBgPri( CLWK* p_wk, u8 pri );
GLOBAL u8 GFL_CLACT_WkGetBgPri( const CLWK* cp_wk );
// アニメーションシーケンス
GLOBAL void GFL_CLACT_WkSetAnmSeq( CLWK* p_wk, u16 anmseq );
GLOBAL u16 GFL_CLACT_WkGetAnmSeq( const CLWK* cp_wk );
GLOBAL void GFL_CLACT_WkSetAnmSeqDiff( CLWK* p_wk, u16 anmseq );
// アニメーションフレーム
GLOBAL void GFL_CLACT_WkSetAnmFrame( CLWK* p_wk, u16 idx );
GLOBAL void GFL_CLACT_WkAddAnmFrame( CLWK* p_wk, fx32 speed );
GLOBAL u16 GFL_CLACT_WkGetAnmFrame( const CLWK* cp_wk );
// オートアニメーション
GLOBAL void GFL_CLACT_WkSetAutoAnmFlag( CLWK* p_wk, BOOL on_off );
GLOBAL BOOL GFL_CLACT_WkGetAutoAnmFlag( const CLWK* cp_wk );
GLOBAL void GFL_CLACT_WkSetAutoAnmSpeed( CLWK* p_wk, fx32 speed );
GLOBAL fx32	GFL_CLACT_WkGetAutoAnmSpeed( const CLWK* cp_wk );
// アニメーション再生停止
GLOBAL void GFL_CLACT_WkStartAnm( CLWK* p_wk );
GLOBAL void GFL_CLACT_WkStopAnm( CLWK* p_wk );
GLOBAL BOOL GFL_CLACT_WkCheckAnmActive( const CLWK* cp_wk );
// アニメーション状態初期化
GLOBAL void GFL_CLACT_WkResetAnm( CLWK* p_wk );
// アニメーション再生方法の変更
GLOBAL void GFL_CLACT_WkSetAnmMode( CLWK* p_wk, CLSYS_ANM_PLAYMODE playmode );
GLOBAL CLSYS_ANM_PLAYMODE GFL_CLACT_WkGetAnmMode( const CLWK* cp_wk );

#undef	GLOBAL
#endif		// __CLACT_H__

