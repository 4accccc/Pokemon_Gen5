//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		clact.c
 *	@brief		セルアクターシステム
 *	@author		tomoya takahashi
 *	@data		2006.11.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "include/gflib.h"

#define __CLACT_H_GLOBAL
#include "include/clact.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	GFL_CLSYS_REND関係
//=====================================
#define CLSYS_DEFFREND_ZOFFS	(1)		// デフォルトで設定する3D用のZオフセット設定


//-------------------------------------
///	CLWK_ANMDATA関係
//=====================================
typedef enum{
	CLWK_ANM_CELL,		// セルアニメ
	CLWK_ANM_TRCELL,	// セル転送アニメ
	CLWK_ANM_MULTICEL,	// マルチセルアニメ
	CLWK_ANM_MAX,		// アニメーションタイプ最大数
} CLWK_ANMTYPE;


//-------------------------------------
///	GFL_CLUNIT	レンダラータイプ
//=====================================
#define CLUNIT_RENDTYPE_DEFF	(0)	// デフォルト設定レンダラー
#define CLUNIT_RENDTYPE_USER	(1)	// ユーザー定義レンダラー


//-------------------------------------
///	フリップマスク
//=====================================
#define CLSYS_FLIPNONE	(0)	// フリップなし
#define CLSYS_VFLIP		(1)	// Vフリップ
#define CLSYS_HFLIP		(2)	// Hフリップ
#define CLSYS_VHFLIP	(CLSYS_VFLIP|CLSYS_HFLIP)	// VHフリップ

//-------------------------------------
///	オートアニメデフォルトスピード
//=====================================
#define CLWK_AUTOANM_DEFF_SPEED	( FX32_ONE )


//-----------------------------------------------------------------------------
/**
 *					オブジェクト実体構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	OAMマネージャオブジェクト
//====================================
typedef struct {
	NNSG2dOamManagerInstance man;
	BOOL init;	// 初期化したかのフラグ
} OAMMAN_DATA;

//-------------------------------------
///	OAMMAN OAMマネージャ管理構造体
//=====================================
typedef struct {
	OAMMAN_DATA man[ CLSYS_DRAW_MAX ];	// メイン画面用OAMマネージャ
} CLSYS_OAMMAN;

//-------------------------------------
///	REND　レンダラー管理構造体
//=====================================
typedef struct _CLSYS_REND{
	NNSG2dRendererInstance rend;
	NNSG2dRenderSurface*   p_surface;
	u32	surface_num;
} GFL_CLSYS_REND;

//-------------------------------------
///	TRMAN　転送データ格納領域
//=====================================
typedef struct {
	void*	p_src;			// 転送データ		データが設定されているかのチェックにも使用
	u32		addr;			// 転送先アドレス
	u32		size;			// サイズ
	CLSYS_DRAW_TYPE	type;	// 転送先タイプ	
} TRMAN_DATA;

//-------------------------------------
///	TRMAN	セルVram転送マネージャ関係
//=====================================
typedef struct {
	NNSG2dCellTransferState* p_cell;	// セル転送管理バッファ
	TRMAN_DATA* p_trdata;				// 転送キャラクタ管理バッファ
	u32 data_num;						// 両バッファの要素数
	BOOL init;							// 初期化有無
} CLSYS_TRMAN;



//-------------------------------------
///	CLSYS構造体
//=====================================
typedef struct {
	CLSYS_OAMMAN	oamman;
	GFL_CLSYS_REND		rend;
	CLSYS_TRMAN		trman;
	u16 pltt_no[CLSYS_DRAW_MAX];	// 上書きするパレットナンバー
	BOOL transflag;					// 転送許可フラグ
	BOOL oammanclear;				// OAMMANclear許可フラグ
} CLSYS;


//-------------------------------------
///	アニメーションデータ構造体
//=====================================
typedef struct {
	CLWK_ANMTYPE	type;	// 格納されているアニメタイプ
	union{
		struct{
			const NNSG2dCellDataBank*       cp_cell;	// セルデータ
			const NNSG2dCellAnimBankData*   cp_canm;	// セルアニメーション
			NNSG2dCellAnimation				anmctrl;	// アニメーションコントローラ
		} cell;
		struct{
			NNSG2dCellDataBank*				p_cell;		// セルデータ
			const NNSG2dCellAnimBankData*   cp_canm;	// セルアニメーション
			NNSG2dCellAnimation				anmctrl;	// アニメーションコントローラ
			u32								trhandle;	// セルアニメーションのVram転送を管理する
														// セル転送状態管理オブジェクトのハンドル
		} trcell;
		struct{
			const NNSG2dCellDataBank*       cp_cell;	// セルデータ
			const NNSG2dCellAnimBankData*	cp_canm;	// セルアニメーション
			NNSG2dMultiCellAnimation		anmctrl;	// アニメーションコントローラ
			const NNSG2dMultiCellDataBank*        cp_mcell;    // マルチセルデータ
			const NNSG2dMultiCellAnimBankData*    cp_mcanm;   // マルチセルアニメーション
			void* p_wk;									// アニメーションワーク
		} multicell;
	} data;
} CLWK_ANMDATA;


//-------------------------------------
///	GFL_CLWK構造体
//=====================================
typedef struct _CLWK{
	GFL_CLWK*				p_next;			// 次のデータ
	GFL_CLWK*				p_last;			// 前のデータ
	GFL_CLUNIT*				p_unit;			// 自分の親セルアクターユニット
	GFL_CLACTPOS			pos;			// 座標
	GFL_CLACTPOS			affinepos;		// アフィン座標
	GFL_CLSCALE				scale;			// 拡大
	NNSG2dImageProxy		img_proxy;		// キャラクタ/テクスチャプロキシ
	NNSG2dImagePaletteProxy pltt_proxy;		// パレットプロキシ
	fx32					auto_anm_speed;	// オートアニメスピード
	u32						setsf;	// サーフェースインデックス
	u16						rotation;		// 回転
	u16						anmseq;			// アニメーションシーケンス

	u32						soft_pri:8;		// ソフト優先順位(u8)
	u32						over_write:4;	// オーバーライトフラグ(0～8)
	u32						pal_offs:4;		// パレットオフセット(0～15)
	u32						bg_pri:2;		// BG優先順位(0～3)
	u32						affine:2;		// アフィン変換タイプ(0～2)
	u32						vhflip:2;		// フリップ変換マスク(0～3)
	u32						objmode:2;		// オブジェクトモード(0～3)
	u32						mosaic:1;		// モザイクフラグ(0～1)
	u32						auto_anm:1;		// オートアニメーションフラグ(0:1)
	u32						draw_flag:1;	// 描画On-Off
	u32						playmode:4;		// アニメーションプレイモード
	u32						dummy:1;		// あまりbit

	CLWK_ANMDATA			anmdata;		// セルアニメデータ
} GFL_CLWK;


//-------------------------------------
///	CLUNT構造体
//=====================================
typedef struct _CLUNIT{
	GFL_CLWK*	p_wk;		// セルワーク
	GFL_CLWK*	p_drawlist;	// 描画リスト
	GFL_CLSYS_REND* p_rend;	// 関連付けられているレンダラー
	u16		wk_num;		// セルワーク数
	u8		rend_type;// レンダラータイプ	（ユーザー/デフォルト）
	u8		draw_flag;// 描画ON_OFFフラグ
} GFL_CLUNIT;



//-----------------------------------------------------------------------------
/**
 *			セルアクター初期化データ構造体宣言
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	サーフェースコールバック構造体
//=====================================
typedef struct {
    NNSG2dOamRegisterFunction         pFuncOamRegister;         // Oam 登録関数
    NNSG2dAffineRegisterFunction      pFuncOamAffineRegister;   // Affine 登録関数
    NNSG2dRndCellCullingFunction      pFuncVisibilityCulling;   // 可視カリング関数
} REND_SURFACE_CALLBACK;



//-----------------------------------------------------------------------------
/**
 *			セルアクターシステム実体
 */
//-----------------------------------------------------------------------------
static CLSYS* pClsys = NULL;


//-----------------------------------------------------------------------------
/**
 *			セルアクターシステム　基本初期化データ
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	スクリーンを分割したバージョン
//=====================================
const GFL_CLSYS_INIT GFL_CLSYSINIT_DEF_DIVSCREEN = {
	0, 0,
 	0, 512,
	0, 128,
	0, 128,
	0
};
//-------------------------------------
///	スクリーンをつなげるバージョン
//=====================================
const GFL_CLSYS_INIT GFL_CLSYSINIT_DEF_CONSCREEN = {
	0, 0,
	0, 192,
	0, 128,
	0, 128,
	0
};



//-----------------------------------------------------------------------------
/**
 *			データ類
 */
//-----------------------------------------------------------------------------
static const u16 CLSYS_DRAW_TYPEtoNNS_G2D_VRAM_TYPE[ CLSYS_DRAW_MAX ] = {
	NNS_G2D_VRAM_TYPE_2DMAIN,
	NNS_G2D_VRAM_TYPE_2DSUB,
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS関係
//=====================================
static void CLSYS_DefaultRendInit( GFL_CLSYS_REND* p_rend, const GFL_CLSYS_INIT* cp_data, HEAPID heapID );
static void CLSYS_SysSetPaletteProxy( const NNSG2dImagePaletteProxy* cp_pltt, u8 pal_offs );
static void CLSYS_SetTransFlag( BOOL flag );
static BOOL CLSYS_GetTransFlag( void );
static void CLSYS_SetOamManClearFlag( BOOL flag );
static BOOL CLSYS_GetOamManClearFlag( void );


//-------------------------------------
///	CLSYS_OAMMAN関係
//=====================================
static void OAMMAN_SysInit( CLSYS_OAMMAN* p_oamman, u8 oamst_main, u8 oamnum_main, u8 oamst_sub, u8 oamnum_sub );
static void OAMMAN_SysExit( CLSYS_OAMMAN* p_oamman );
static void OAMMAN_SysTrans( CLSYS_OAMMAN* p_oamman );
static void OAMMAN_SysClean( CLSYS_OAMMAN* p_oamman );
static void OAMMAN_ObjCreate( OAMMAN_DATA* p_obj, u8 oamst, u8 oamnum, NNSG2dOamType oam_type );
static BOOL OAMMAN_ObjEntryOamAttr( OAMMAN_DATA* p_obj, const GXOamAttr* pOam, u16 affineIndex );
static u16 OAMMAN_ObjEntryAffine( OAMMAN_DATA* p_obj, const MtxFx22* mtx );
static void OAMMAN_ObjTrans( OAMMAN_DATA* p_obj );
static void OAMMAN_ObjClearBuff( OAMMAN_DATA* p_obj );

//-------------------------------------
///	GFL_CLSYS_REND関係
//=====================================
static void REND_SysInit( GFL_CLSYS_REND* p_rend, const GFL_REND_SURFACE_INIT* cp_data, u32 data_num, HEAPID heapID );
static void REND_SysExit( GFL_CLSYS_REND* p_rend );
static void REND_SysBeginDraw( GFL_CLSYS_REND* p_rend, const NNSG2dImageProxy* pImgProxy, const NNSG2dImagePaletteProxy* pPltProxy );
static void REND_SysEndDraw( GFL_CLSYS_REND* p_rend );
static void REND_SysSetAffine( GFL_CLSYS_REND* p_rend, CLSYS_AFFINETYPE mode );
static void REND_SysSetFlip( GFL_CLSYS_REND* p_rend, BOOL vflip, BOOL hflip );
static void REND_SysSetOverwrite( GFL_CLSYS_REND* p_rend, u8 over_write );
static void REND_SysSetMosaicFlag( GFL_CLSYS_REND* p_rend, BOOL on_off );
static void REND_SysSetOBJMode( GFL_CLSYS_REND* p_rend, GXOamMode objmode );
static void REND_SysSetBGPriority( GFL_CLSYS_REND* p_rend, u8 pri );
static void REND_SurfaceObjCreate( NNSG2dRenderSurface* p_surface, const GFL_REND_SURFACE_INIT* cp_data );
static void REND_SurfaceObjSetPos( NNSG2dRenderSurface* p_surface, s16 x, s16 y );
static void REND_SurfaceObjSetSize( NNSG2dRenderSurface* p_surface, s16 width, s16 height );
static void REND_SurfaceObjSetType( NNSG2dRenderSurface* p_surface, CLSYS_DRAW_TYPE type );
static void REND_SurfaceObjSetCallBack( NNSG2dRenderSurface* p_surface, const REND_SURFACE_CALLBACK* cp_callback );
static void REND_SurfaceObjGetPos( const NNSG2dRenderSurface* cp_surface, s16* p_x, s16* p_y );
static void REND_SurfaceObjGetSize( const NNSG2dRenderSurface* cp_surface, s16* p_width, s16* p_height );
static CLSYS_DRAW_TYPE REND_SurfaceObjGetType( const NNSG2dRenderSurface* cp_surface );
// レンダラーDefaultコールバック
static BOOL REND_CallBackAddOam( OAMMAN_DATA* p_obj, const GXOamAttr* pOam, u16 affineIndex, u32 pltt_no );
static BOOL REND_CallBackMainAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine );
static BOOL REND_CallBackSubAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine );
static u16 REND_CallBackMainAddAffine( const MtxFx22* mtx );
static u16 REND_CallBackSubAddAffine( const MtxFx22* mtx );
static BOOL REND_CallBackCulling( const NNSG2dCellData* pCell, const MtxFx32* pMtx, const NNSG2dViewRect* pViewRect );


//-------------------------------------
///	TRMAN	関係
//=====================================
static void TRMAN_SysInit( CLSYS_TRMAN* p_trman, u8 tr_cell, HEAPID heapID );
static void TRMAN_SysExit( CLSYS_TRMAN* p_trman );
static void TRMAN_SysMain( CLSYS_TRMAN* p_trman );
static void TRMAN_SysVBlank( CLSYS_TRMAN* p_trman );
static u8 TRMAN_SysGetBuffNum( const CLSYS_TRMAN* cp_trman );
static void TRMAN_CellTransManCreate( CLSYS_TRMAN* p_trman, u8 tr_cell, HEAPID heapID );
static void TRMAN_CellTransManDelete( CLSYS_TRMAN* p_trman );
static u32 TRMAN_CellTransManAddHandle( CLSYS_TRMAN* p_trman );
static void TRMAN_CellTransManDelHandle( CLSYS_TRMAN* p_trman, u32 handle );
static void TRMAN_TrSysCreate( CLSYS_TRMAN* p_trman, u8 tr_cell, HEAPID heapID );
static void TRMAN_TrSysDelete( CLSYS_TRMAN* p_trman );
static void TRMAN_TrSysUpdata( CLSYS_TRMAN* p_trman );
static void TRMAN_TrSysTrans( CLSYS_TRMAN* p_trman );
static TRMAN_DATA* TRMAN_TrSysGetCleanBuff( CLSYS_TRMAN* p_trman );
static BOOL TRMAN_TrSysCallBackAddTransData( NNS_GFD_DST_TYPE type, u32 dstAddr, void* pSrc, u32 szByte );
static void TRMAN_TrDataClean( TRMAN_DATA* p_data );
static void TRMAN_TrDataAddData( TRMAN_DATA* p_data, CLSYS_DRAW_TYPE type, u32 dstAddr, void* pSrc, u32 szByte );
static void TRMAN_TrDataTrans( TRMAN_DATA* p_data );
static BOOL TRMAN_TrDataCheckClean( const TRMAN_DATA* cp_data );


//-------------------------------------
///	GFL_CLUNIT	関係
//=====================================
static GFL_CLWK* CLUNIT_SysGetCleanWk( GFL_CLUNIT* p_unit );
static void CLUNIT_DrawListAdd( GFL_CLUNIT* p_unit, GFL_CLWK* p_wk );
static void CLUNIT_DrawListDel( GFL_CLUNIT* p_unit, GFL_CLWK* p_wk );
static GFL_CLWK* CLUNIT_DrawListSarchTop( GFL_CLWK* p_top, u8 pri );
static GFL_CLWK* CLUNIT_DrawListSarchBottom( GFL_CLWK* p_bottom, u8 pri );


//-------------------------------------
///	GFL_CLWK	関係
//=====================================
static void CLWK_SysClean( GFL_CLWK* p_wk );
static void CLWK_SysDraw( GFL_CLWK* p_wk, GFL_CLSYS_REND* p_rend );
static void CLWK_SysDrawSetRend( GFL_CLWK* p_wk, GFL_CLSYS_REND* p_rend );
static void CLWK_SysDrawCell( GFL_CLWK* p_wk );
static void CLWK_SysAutoAnm( GFL_CLWK* p_wk );
static void CLWK_SysSetClwkData( GFL_CLWK* p_wk, const GFL_CLWK_DATA* cp_data, u16 setsf );
static void CLWK_SysSetClwkRes( GFL_CLWK* p_wk, const GFL_CLWK_RES* cp_res );
static void CLWK_SysGetSetSfPos( const GFL_CLWK* cp_wk, u16 setsf, GFL_CLACTPOS* p_pos );

//-------------------------------------
///	CLWK_ANMDATA	関係
//=====================================
static void CLWK_AnmDataInit( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID );
static void CLWK_AnmDataExit( CLWK_ANMDATA* p_anmdata );
static CLWK_ANMTYPE CLWK_AnmDataGetType( const GFL_CLWK_RES* cp_res );
static void CLWK_AnmDataCreateCellData( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID );
static void CLWK_AnmDataCreateTRCellData( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID );
static void CLWK_AnmDataCreateMCellData( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID );
static void CLWK_AnmDataDeleteCellData( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataDeleteTRCellData( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataDeleteMCellData( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataSetAnmSeq( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqTRCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqMCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqNoReset( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqNoResetCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqNoResetTRCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmSeqNoResetMCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmFrame( CLWK_ANMDATA* p_anmdata, u16 idx );
static void CLWK_AnmDataSetAnmFrameCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmFrameTRCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataSetAnmFrameMCell( CLWK_ANMDATA* p_anmdata, u16 anmseq );
static void CLWK_AnmDataAddAnmFrame( CLWK_ANMDATA* p_anmdata, fx32 speed );
static void CLWK_AnmDataAddAnmFrameCell( CLWK_ANMDATA* p_anmdata, fx32 speed );
static void CLWK_AnmDataAddAnmFrameTRCell( CLWK_ANMDATA* p_anmdata, fx32 speed );
static void CLWK_AnmDataAddAnmFrameMCell( CLWK_ANMDATA* p_anmdata, fx32 speed );
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrl( CLWK_ANMDATA* p_anmdata );
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrlCell( CLWK_ANMDATA* p_anmdata );
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrlTRCell( CLWK_ANMDATA* p_anmdata );
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrlMCell( CLWK_ANMDATA* p_anmdata );
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrl( const CLWK_ANMDATA* cp_anmdata );
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrlCell( const CLWK_ANMDATA* cp_anmdata );
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrlTRCell( const CLWK_ANMDATA* cp_anmdata );
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrlMCell( const CLWK_ANMDATA* cp_anmdata );
static u16 CLWK_AnmDataGetAnmFrame( const CLWK_ANMDATA* cp_anmdata );
static void CLWK_AnmDataSetAnmSpeed( CLWK_ANMDATA* p_anmdata, fx32 speed );
static void CLWK_AnmDataSetAnmSpeedCell( CLWK_ANMDATA* p_anmdata, fx32 speed );
static void CLWK_AnmDataSetAnmSpeedTRCell( CLWK_ANMDATA* p_anmdata, fx32 speed );
static void CLWK_AnmDataSetAnmSpeedMCell( CLWK_ANMDATA* p_anmdata, fx32 speed );
static BOOL CLWK_AnmDataCheckAnmActive( const CLWK_ANMDATA* cp_anmdata );
static void CLWK_AnmDataResetAnm( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataResetAnmCell( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataResetAnmMCell( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataSetAnmMode( CLWK_ANMDATA* p_anmdata, CLSYS_ANM_PLAYMODE playmode );
static void CLWK_AnmDataStartAnm( CLWK_ANMDATA* p_anmdata );
static void CLWK_AnmDataStopAnm( CLWK_ANMDATA* p_anmdata );
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttr( const CLWK_ANMDATA* cp_anmdata, u32 seq );
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttrCell( const CLWK_ANMDATA* cp_anmdata, u32 seq );
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttrTRCell( const CLWK_ANMDATA* cp_anmdata, u32 seq );
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttrMCell( const CLWK_ANMDATA* cp_anmdata, u32 seq );
static u32 CLWK_AnmDataGetUserCellAttr( const CLWK_ANMDATA* cp_anmdata, u32 idx );
static const NNSG2dUserExCellAttrBank* CLWK_AnmDataGetUserCellAttrCell( const CLWK_ANMDATA* cp_anmdata );
static const NNSG2dUserExCellAttrBank* CLWK_AnmDataGetUserCellAttrTRCell( const CLWK_ANMDATA* cp_anmdata );
static const NNSG2dUserExCellAttrBank* CLWK_AnmDataGetUserCellAttrMCell( const CLWK_ANMDATA* cp_anmdata );
static u32 CLWK_AnmDataGetAnmSeqNum( const CLWK_ANMDATA* cp_anmdata );
static u32 CLWK_AnmDataGetAnmSeqNumCell( const CLWK_ANMDATA* cp_anmdata );
static u32 CLWK_AnmDataGetAnmSeqNumTRCell( const CLWK_ANMDATA* cp_anmdata );
static u32 CLWK_AnmDataGetAnmSeqNumMCell( const CLWK_ANMDATA* cp_anmdata );






//-----------------------------------------------------------------------------
/**
 *				外部公開関数郡
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　初期化
 *
 *	@param	cp_data		初期化データ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_Init( const GFL_CLSYS_INIT* cp_data, HEAPID heapID )
{
	int i;

	GF_ASSERT( pClsys == NULL );

	//OS_Printf( "[%d]\n", __LINE__ );

	pClsys = GFL_HEAP_AllocMemory( heapID, sizeof(CLSYS) );
	GFL_STD_MemFill( pClsys, 0, sizeof(CLSYS) );
	
	// OAMマネジャー初期化
	OAMMAN_SysInit( &pClsys->oamman, 
			cp_data->oamst_main, cp_data->oamnum_main,
			cp_data->oamst_sub, cp_data->oamnum_sub
			);

	// レンダラー初期化
	CLSYS_DefaultRendInit( &pClsys->rend, cp_data, heapID );

	// セル転送アニメシステム初期化
	TRMAN_SysInit( &pClsys->trman, cp_data->tr_cell, heapID );

	// パレットナンバー
	for( i=0; i<CLSYS_DRAW_MAX; i++ ){
		pClsys->pltt_no[i] = 0;
	}

	// 転送許可
	CLSYS_SetTransFlag( TRUE );

	// OAMMANクリア許可設定
	CLSYS_SetOamManClearFlag( TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム	破棄
 *
 *	@param	none
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_Exit( void )
{
	GF_ASSERT( pClsys );
	//OS_Printf( "[%d]\n", __LINE__ );

	// 転送不可設定
	CLSYS_SetTransFlag( FALSE );
	
	// 各システム破棄
	OAMMAN_SysExit( &pClsys->oamman );
	REND_SysExit( &pClsys->rend );
	TRMAN_SysExit( &pClsys->trman );

	GFL_HEAP_FreeMemory( pClsys );
	pClsys = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　メイン処理
 *
 *	*全セルアクターユニットの描画が終わった後に呼ぶ必要があります。
 *	*メインループの最後に呼ぶようにしておくと確実だと思います。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_Main( void )
{
	//OS_Printf( "[%d]\n", __LINE__ );
	if( pClsys ){
		TRMAN_SysMain( &pClsys->trman );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　Vブランク処理
 *
 * [セルアクターシステム　Vブランク期間内での転送処理　説明]
 *	＞用途に合わせて２つのタイプを選べる
 *　　・転送処理方法には２つのタイプがあります。
 *		1.	GFL_CLACT_VBlankFunc
 *		2.	GFL_CLACT_VBlankFuncTransOnly ＋　GFL_CLACT_ClearOamBuff
 *	　・1　GFL_CLACT_VBlankFunc
 *		OAMデータ転送後、バッファをクリーンします。
 *	　　処理落ちなどで、１ループ中に２回割り込みが入ると何も表示されなく
 *	　　なってしまいますので、割り込み内で使用する事が出来ません。
 *
 *	  ・2　GFL_CLACT_VBlankFuncTransOnly ＋　GFL_CLACT_ClearOamBuff
 *		GFL_CLACT_VBlankFuncTransOnlyはOAMバッファのデータの転送のみ行いますので、
 *		この関数をVBlank割り込み内で呼んでください。
 *		CLACT_UNITの描画前にGFL_CLACT_ClearOamBuffを呼ぶことでOAMバッファをクリアする
 *		事が出来ます。
 *
 *	　＞GFL_CLACT_ClearOamBuffについて
 *		・セルアクターシステム内にはoammanclearフラグというものを持っています。
 *		　これは、OamBuffをクリアしてよいかを判別するフラグです。
 *		　フラグが立っている状態のときのみGFL_CLACT_ClearOamBuffを
 *		　実行することが出来ます。
 *		　『oammanclearフラグの動作』
 *			GFL_CLACT_VBlankFuncTransOnlyが呼ばれると立ちます。
 *			GFL_CLACT_ClearOamBuffが呼ばれると落ちます。
 *		　これはCLACT_UNITが自由にGFL_CLACT_ClearOamBuffを呼べるようにするため
 *		　行っています。
 *			
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_VBlankFunc( void )
{
	//OS_Printf( "[%d]\n", __LINE__ );
	if( pClsys ){
		if( CLSYS_GetTransFlag() ){
			OAMMAN_SysTrans( &pClsys->oamman );
			OAMMAN_SysClean( &pClsys->oamman );
			TRMAN_SysVBlank( &pClsys->trman );

			CLSYS_SetOamManClearFlag( FALSE );	// 常にOAMMANバッファクリア不可設定
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターOAMバッファのクリーン処理
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_ClearOamBuff( void )
{
	if( pClsys ){
		if( CLSYS_GetOamManClearFlag() ){
			CLSYS_SetTransFlag( FALSE );	// 転送不可
			OAMMAN_SysClean( &pClsys->oamman );
			CLSYS_SetOamManClearFlag( FALSE );	// １回クリアしたので不可設定
			CLSYS_SetTransFlag( TRUE );	// 転送許可
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム	Vブランク処理	転送のみ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_VBlankFuncTransOnly( void )
{
	if( pClsys ){
		if( CLSYS_GetTransFlag() ){
			OAMMAN_SysTrans( &pClsys->oamman );
			TRMAN_SysVBlank( &pClsys->trman );
			CLSYS_SetOamManClearFlag( TRUE );	// バッファクリア許可
		}
	}
}



//-------------------------------------
///	USER定義レンダラー関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムを作成する
 *
 *	@param	cp_data			ユーザー定義サーフェースデータ
 *	@param	data_num		データ数
 *	@param	heapID			ヒープID
 *
 *	@return	作成したレンダラーシステム
 */
//-----------------------------------------------------------------------------
GFL_CLSYS_REND* GFL_CLACT_USERREND_Create( const GFL_REND_SURFACE_INIT* cp_data, u32 data_num, HEAPID heapID )
{
	GFL_CLSYS_REND* p_rend;
	//OS_Printf( "[%d]\n", __LINE__ );
	GF_ASSERT( cp_data );
	
	p_rend = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_CLSYS_REND) );
	REND_SysInit( p_rend, cp_data, data_num, heapID );
	return p_rend;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムを破棄する
 *
 *	@param	p_rend			ユーザー定義サーフェースデータ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_USERREND_Delete( GFL_CLSYS_REND* p_rend )
{
	GF_ASSERT( p_rend );
	//OS_Printf( "[%d]\n", __LINE__ );

	REND_SysExit( p_rend );
	GFL_HEAP_FreeMemory( p_rend );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェース左上座標を設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	cp_pos			設定する座標
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_USERREND_SetSurfacePos( GFL_CLSYS_REND* p_rend, u32 idx, const GFL_CLACTPOS* cp_pos )
{
	GF_ASSERT( p_rend );
	// サーフェース数を超えたidxでバッファを参照しようとしています
	GF_ASSERT( p_rend->surface_num > idx );
	//OS_Printf( "[%d]\n", __LINE__ );
	REND_SurfaceObjSetPos( &p_rend->p_surface[ idx ], cp_pos->x, cp_pos->y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェース左上座標を取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	p_pos			座標格納先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_USERREND_GetSurfacePos( const GFL_CLSYS_REND* cp_rend, u32 idx, GFL_CLACTPOS* p_pos )
{
	GF_ASSERT( cp_rend );
	// サーフェース数を超えたidxでバッファを参照しようとしています
	GF_ASSERT( cp_rend->surface_num > idx );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	REND_SurfaceObjGetPos( &cp_rend->p_surface[ idx ], &p_pos->x, &p_pos->y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェースサイズを設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	cp_size			サイズ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_USERREND_SetSurfaceSize( GFL_CLSYS_REND* p_rend, u32 idx, const GFL_CLACTPOS* cp_size )
{
	GF_ASSERT( p_rend );
	// サーフェース数を超えたidxでバッファを参照しようとしています
	GF_ASSERT( p_rend->surface_num > idx );
	//OS_Printf( "[%d]\n", __LINE__ );
	REND_SurfaceObjSetSize( &p_rend->p_surface[ idx ], cp_size->x, cp_size->y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェースサイズを取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	p_size			サイズ取得先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_USERREND_GetSurfaceSize( const GFL_CLSYS_REND* cp_rend, u32 idx, GFL_CLACTPOS* p_size )
{
	GF_ASSERT( cp_rend );
	// サーフェース数を超えたidxでバッファを参照しようとしています
	GF_ASSERT( cp_rend->surface_num > idx );
	//OS_Printf( "[%d]\n", __LINE__ );
	REND_SurfaceObjGetSize( &cp_rend->p_surface[ idx ], &p_size->x, &p_size->y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェースタイプを設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	type			サーフェースタイプ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_USERREND_SetSurfaceType( GFL_CLSYS_REND* p_rend, u32 idx, CLSYS_DRAW_TYPE type )
{
	GF_ASSERT( p_rend );
	// サーフェース数を超えたidxでバッファを参照しようとしています
	GF_ASSERT( p_rend->surface_num > idx );
	//OS_Printf( "[%d]\n", __LINE__ );
	REND_SurfaceObjSetType( &p_rend->p_surface[ idx ], type );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェースタイプを取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *
 *	@return	サーフェースタイプ	（CLSYS_DRAW_TYPE）
 */
//-----------------------------------------------------------------------------
CLSYS_DRAW_TYPE GFL_CLACT_USERREND_GetSurfaceType( const GFL_CLSYS_REND* cp_rend, u32 idx )
{
	GF_ASSERT( cp_rend );
	// サーフェース数を超えたidxでバッファを参照しようとしています
	GF_ASSERT( cp_rend->surface_num > idx );
	//OS_Printf( "[%d]\n", __LINE__ );
	return REND_SurfaceObjGetType( &cp_rend->p_surface[ idx ] );
}


//-------------------------------------
///	GFL_CLUNIT関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットを生成
 *
 *	@param	wknum		ワーク数
 *	@param	heapID		ヒープID
 *
 *	@return	ユニットポインタ
 */
//-----------------------------------------------------------------------------
GFL_CLUNIT* GFL_CLACT_UNIT_Create( u16 wknum, HEAPID heapID )
{
	GFL_CLUNIT* p_unit;
	int i;
	//OS_Printf( "[%d]\n", __LINE__ );

	// 実体を生成
	p_unit = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_CLUNIT) );
	GFL_STD_MemFill( p_unit, 0, sizeof(GFL_CLUNIT) );

	// ワーク作成
	p_unit->p_wk = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_CLWK)*wknum );
	p_unit->wk_num = wknum;
	for( i=0; i<wknum; i++ ){
		CLWK_SysClean( &p_unit->p_wk[i] );
	}

	// デフォルトレンダラー設定
	GFL_CLACT_UNIT_SetDefaultRend( p_unit );
	
	// 表示ON
	GFL_CLACT_UNIT_SetDrawFlag( p_unit, TRUE );

	return p_unit;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットを破棄
 *
 *	@param	p_unit			セルアクターユニット
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_UNIT_Delete( GFL_CLUNIT* p_unit )
{
	int i;
	
	GF_ASSERT( p_unit );
	//OS_Printf( "[%d]\n", __LINE__ );

	// 全ワーク破棄
	for( i=0; i<p_unit->wk_num; i++ ){
		if( p_unit->p_wk[i].p_next != NULL ){
			GFL_CLACT_WK_Remove( &p_unit->p_wk[i] );
		}
	}
	// ワーク破棄
	GFL_HEAP_FreeMemory( p_unit->p_wk );
	// 実体を破棄
	GFL_HEAP_FreeMemory( p_unit );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニット　描画処理
 *
 *	@param	p_unit			セルアクターユニット
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_UNIT_Draw( GFL_CLUNIT* p_unit )
{
	GFL_CLWK* p_wk;

	GF_ASSERT( p_unit );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	// 表示フラグが立っているか
	if( p_unit->draw_flag == FALSE ){
		return;
	}
	// 表示するワークがあるかチェック
	if( p_unit->p_drawlist == NULL ){
		return;
	}

	// 転送不可設定
	CLSYS_SetTransFlag( FALSE );
 
	// 先頭のデータを設定
	p_wk = p_unit->p_drawlist;

	do{
		// 表示
		CLWK_SysDraw( p_wk, p_unit->p_rend );

		// オートアニメ処理
		CLWK_SysAutoAnm( p_wk );

		// 次へ
		p_wk = p_wk->p_next;
	}while( p_wk != p_unit->p_drawlist );

	// 転送許可設定
	CLSYS_SetTransFlag( TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットに描画フラグを設定
 *	
 *	@param	p_unit		セルアクターユニット
 *	@param	on_off		描画ON-OFF
 *
 *	TRUE	表示
 *	FALSE	非表示
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_UNIT_SetDrawFlag( GFL_CLUNIT* p_unit, BOOL on_off )
{
	GF_ASSERT( p_unit );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_unit->draw_flag = on_off;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画フラグを取得
 *
 *	@param	cp_unit		セルアクターユニット
 *
 *	@retval	TRUE	表示
 *	@retval	FALSE	非表示
 */
//-----------------------------------------------------------------------------
BOOL GFL_CLACT_UNIT_GetDrawFlag( const GFL_CLUNIT* cp_unit )
{
	GF_ASSERT( cp_unit );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_unit->draw_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットにユーザー独自のレンダラーシステムを設定
 *
 *	@param	p_unit		セルアクターユニット
 *	@param	p_rend		ユーザー独自のレンダラー
 *
 *	この関数で、好きな設定をしたレンダラーを使用することが出来るようになります。
 *	＞ユーザー独自のレンダラーを作成する
 *		GFL_CLACT_USERREND_～関数で作成できます。
 *
 *  ＞注意事項
 *	　　座標設定するときに渡すsetsfの値は、ユーザー独自レンダラー
 *	　　に登録したサーフェースのインデックス番号となります。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_UNIT_SetUserRend( GFL_CLUNIT* p_unit, GFL_CLSYS_REND* p_rend )
{
	GF_ASSERT( p_unit );
	GF_ASSERT( p_rend );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_unit->p_rend = p_rend;
	p_unit->rend_type = CLUNIT_RENDTYPE_USER;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットにデフォルトレンダラーシステムを設定
 *
 *	@param	p_unit		セルアクターユニット
 *
 *	この関数で、独自レンダラーシステムからデフォルトレンダラーシステムを
 *	使用するように戻すことが出来ます。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_UNIT_SetDefaultRend( GFL_CLUNIT* p_unit )
{
	GF_ASSERT( pClsys );
	GF_ASSERT( p_unit );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_unit->p_rend = &pClsys->rend;
	p_unit->rend_type = CLUNIT_RENDTYPE_DEFF;
}


//-------------------------------------
///	GFL_CLWK関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメ用リソースデータ作成
 *
 *	@param	p_res			リソースデータ格納先
 *	@param	cp_img			イメージプロクシ
 *	@param	cp_pltt			パレットプロクシ
 *	@param	p_cell			セルデータバンク
 *	@param	cp_canm			セルアニメーションデータバンク
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetCellResData( GFL_CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm )
{
	p_res->cp_img	= cp_img;
	p_res->cp_pltt	= cp_pltt;
	p_res->p_cell	= p_cell;
	p_res->cp_canm	= cp_canm;
	p_res->cp_mcell = NULL;
	p_res->cp_mcanm = NULL;
	p_res->cp_char	= NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション用リソースデータ作成
 *
 *	@param	p_res		リソースデータ格納先
 *	@param	cp_img		イメージプロクシ
 *	@param	cp_pltt     パレットプロクシ
 *	@param	p_cell      セルデータバンク
 *	@param	cp_canm     セルアニメーションデータバンク
 *	@param	cp_char     キャラクタデータ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetTrCellResData( GFL_CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm, const NNSG2dCharacterData* cp_char )
{
	GFL_CLACT_WK_SetCellResData( p_res, cp_img, cp_pltt, p_cell, cp_canm );
	p_res->cp_char = cp_char;
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション　リソースデータ作成
 *
 *	@param	p_res			リソースデータ格納先
 *	@param	cp_img			イメージプロクシ
 *	@param	cp_pltt         パレットプロクシ
 *	@param	p_cell          セルデータバンク
 *	@param	cp_canm         セルアニメーションデータバンク
 *	@param	cp_mcell		マルチセルデータバンク
 *	@param	cp_mcanm		マルチセルアニメーションデータバンク
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetMCellResData( GFL_CLWK_RES* p_res, const NNSG2dImageProxy* cp_img, const NNSG2dImagePaletteProxy* cp_pltt, NNSG2dCellDataBank* p_cell, const NNSG2dCellAnimBankData* cp_canm, const NNSG2dMultiCellDataBank* cp_mcell, const NNSG2dMultiCellAnimBankData* cp_mcanm )
{
	GFL_CLACT_WK_SetCellResData( p_res, cp_img, cp_pltt, p_cell, cp_canm );
	p_res->cp_mcell = cp_mcell;
	p_res->cp_mcanm = cp_mcanm;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターの登録
 *
 *	@param	p_unit			セルアクターユニット
 *	@param	cp_data			セルアクターデータ
 *	@param	cp_res			セルアクターリソース
 *	@param	setsf			設定サーフェース
 *	@param	heapID			ヒープID
 *
 *	@return	登録したセルアクターワーク
 *
 *	【setsfの説明】
 *		GFL_CLUNITの使用するレンダラーシステムを変更していないときは
 *		CLSYS_DEFREND_TYPEの値を指定する
 *		・CLSYS_DEFREND_MAIN指定時	pos_x/yがメイン画面左上座標からの相対座標になる。
 *		・CLSYS_DEFREND_SUB指定時	pos_x/yがサブ画面左上座標からの相対座標になる。
 *		
 *		独自のレンダラーシステムをGFL_CLUNITに設定しているときは、
 *		サーフェースの要素数を指定することで、
 *		指定されたサーフェース左上座標からの相対座標になる。
 *
 *		通常/独自レンダラーシステム共通で、
 *		CLWK_SETSF_NONEを指定すると絶対座標設定になる
 */
//-----------------------------------------------------------------------------
GFL_CLWK* GFL_CLACT_WK_Add( GFL_CLUNIT* p_unit, const GFL_CLWK_DATA* cp_data, const GFL_CLWK_RES* cp_res, u16 setsf, HEAPID heapID )
{
	GFL_CLWK* p_wk;
	//OS_Printf( "[%d]\n", __LINE__ );
	
	// 空いているワークを取得
	p_wk = CLUNIT_SysGetCleanWk( p_unit );

	// このした順番大事です！
	
	// 親セルアクターユニットを設定
	p_wk->p_unit = p_unit;

	// セルアニメーションデータ初期化
	CLWK_AnmDataInit( &p_wk->anmdata, cp_res, heapID );

	// ワークにパラメータ設定
	CLWK_SysSetClwkData( p_wk, cp_data, setsf );
	CLWK_SysSetClwkRes( p_wk, cp_res );

	// 描画リストに登録
	CLUNIT_DrawListAdd( p_unit, p_wk );
	GFL_CLACT_WK_SetDrawFlag( p_wk, TRUE );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターの登録	アフィン変換バージョン
 *
 *	@param	p_unit			セルアクターユニット
 *	@param	cp_data			セルアクターデータ　アフィンバージョン
 *	@param	cp_res			セルアクターリソース
 *	@param	setsf			サーフェースインデックス
 *	@param	heapID			ヒープID
 *
 *	@return	登録したセルアクターワーク
 */
//-----------------------------------------------------------------------------
GFL_CLWK* GFL_CLACT_WK_AddAffine( GFL_CLUNIT* p_unit, const GFL_CLWK_AFFINEDATA* cp_data, const GFL_CLWK_RES* cp_res, u16 setsf, HEAPID heapID )
{
	GFL_CLWK* p_wk;
	GF_ASSERT( cp_data );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	// 登録
	p_wk = GFL_CLACT_WK_Add( p_unit, &cp_data->clwkdata, cp_res, setsf, heapID );

	// アフィン設定
	GFL_CLACT_WK_SetTypeAffinePos( p_wk, cp_data->affinepos_x, CLSYS_MAT_X );
	GFL_CLACT_WK_SetTypeAffinePos( p_wk, cp_data->affinepos_x, CLSYS_MAT_Y );
	GFL_CLACT_WK_SetTypeScale( p_wk, cp_data->scale_x, CLSYS_MAT_X );
	GFL_CLACT_WK_SetTypeScale( p_wk, cp_data->scale_y, CLSYS_MAT_Y );
	GFL_CLACT_WK_SetRotation( p_wk, cp_data->rotation );
	GFL_CLACT_WK_SetAffineParam( p_wk, cp_data->affine_type );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄処理
 *
 *	@param	p_wk	破棄するセルアクターワーク
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_Remove( GFL_CLWK* p_wk )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	// 描画リストからはずす
	CLUNIT_DrawListDel( p_wk->p_unit, p_wk );

	// アニメデータ破棄
	CLWK_AnmDataExit( &p_wk->anmdata );

	// ワークをクリア
	CLWK_SysClean( p_wk );
}



//----------------------------------------------------------------------------
/**
 *	@brief	表示フラグ設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	on_off			表示ON-OFF
 *	TRUE	表示
 *	FALSE	非表示
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetDrawFlag( GFL_CLWK* p_wk, BOOL on_off )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->draw_flag = on_off;
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示フラグ取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@retval	TRUE	表示
 *	@retval	FALSE	非表示
 */
//-----------------------------------------------------------------------------
BOOL GFL_CLACT_WK_GetDrawFlag( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->draw_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_pos			座標構造体
 *	@param	setsf			サーフェースインデックス
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetPos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos, u16 setsf )
{
	GFL_CLACTPOS sf_pos;
	GFL_CLACTPOS wk_pos;

	GF_ASSERT( cp_pos );
	//OS_Printf( "[%d]\n", __LINE__ );

	// 設定サーフェースの座標を取得する
	CLWK_SysGetSetSfPos( p_wk, setsf, &sf_pos );

	// 絶対座標にして設定
	wk_pos.x = cp_pos->x + sf_pos.x;
	wk_pos.y = cp_pos->y + sf_pos.y;
	GFL_CLACT_WK_SetWldPos( p_wk, &wk_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pos			座標格納先
 *	@param	setsf			サーフェースインデックス
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_GetPos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos, u16 setsf )
{
	GFL_CLACTPOS sf_pos;

	GF_ASSERT( p_pos );
	//OS_Printf( "[%d]\n", __LINE__ );

	// 設定サーフェースの座標を取得する
	CLWK_SysGetSetSfPos( cp_wk, setsf, &sf_pos );

	// 自分の座標を取得する
	GFL_CLACT_WK_GetWldPos( cp_wk, p_pos );

	// 相対座標に変更する
	p_pos->x -= sf_pos.x;
	p_pos->y -= sf_pos.y;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標個別設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pos			設定値
 *	@param	setsf		設定サーフェース
 *	@param	type		座標タイプ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetTypePos( GFL_CLWK* p_wk, s16 pos, u16 setsf, CLSYS_MAT_TYPE type )
{
	GFL_CLACTPOS sf_pos;
	//OS_Printf( "[%d]\n", __LINE__ );
	// 設定サーフェースの座標を取得する
	CLWK_SysGetSetSfPos( p_wk, setsf, &sf_pos );
	// 絶対座標にして設定
	if( type == CLSYS_MAT_X ){
		pos += sf_pos.x;
	}else{
		pos += sf_pos.y;
	}
	GFL_CLACT_WK_SetWldTypePos( p_wk, pos, type );
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標個別取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	setsf		設定サーフェース
 *	@param	type		座標タイプ
 *
 *	@return	座標タイプのサーフェース内相対座標
 */
//-----------------------------------------------------------------------------
s16 GFL_CLACT_WK_GetTypePos( const GFL_CLWK* cp_wk, u16 setsf, CLSYS_MAT_TYPE type )
{
	GFL_CLACTPOS sf_pos;
	s16 pos;

	//OS_Printf( "[%d]\n", __LINE__ );
	// 設定サーフェースの座標を取得する
	CLWK_SysGetSetSfPos( cp_wk, setsf, &sf_pos );

	// 自分の座標を取得する
	pos = GFL_CLACT_WK_GetWldTypePos( cp_wk, type );

	// 相対座標に変更する
	if( type == CLSYS_MAT_X ){
		pos -= sf_pos.x;
	}else{
		pos -= sf_pos.y;
	}
	return pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標設定
 *
 *	@param	p_wk			セルアクターワーク	
 *	@param	cp_pos			座標
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetWldPos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos )
{
	GF_ASSERT( cp_pos );
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->pos = *cp_pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pos			座標格納先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_GetWldPos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( p_pos );
	//OS_Printf( "[%d]\n", __LINE__ );
	*p_pos = cp_wk->pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標個別設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	pos				設定値
 *	@param	type			座標タイプ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetWldTypePos( GFL_CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type )
{
	GF_ASSERT( p_wk );
	// 対応していないtypeです
	GF_ASSERT( CLSYS_MAT_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );
	if( type == CLSYS_MAT_X ){
		p_wk->pos.x = pos;
	}else{
		p_wk->pos.y = pos;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標個別取得
 *	
 *	@param	cp_wk			セルアクターワーク
 *	@param	type			座標タイプ
 *
 *	@return	座標タイプの絶対座標
 */
//-----------------------------------------------------------------------------
s16 GFL_CLACT_WK_GetWldTypePos( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type )
{
	s16 pos;
	GF_ASSERT( cp_wk );
	// 対応していないtypeです
	GF_ASSERT( CLSYS_MAT_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );
	if( type == CLSYS_MAT_X ){
		pos = cp_wk->pos.x;
	}else{
		pos = cp_wk->pos.y;
	}
	return pos;
}


//----------------------------------------------------------------------------
/**
 *	@brief	アフィンパラメータ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	affine		アフィンパラメータ
 *	affine
 *		CLSYS_AFFINETYPE_NONE	上書きしない
 *		CLSYS_AFFINETYPE_NORMAL	通常のアフィン変換方式に設定
 *		CLSYS_AFFINETYPE_DOUBLE	倍角アフィン変換方式に設定
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAffineParam( GFL_CLWK* p_wk, CLSYS_AFFINETYPE affine )
{
	GF_ASSERT( p_wk );
	// 対応していないアフィンタイプです
	GF_ASSERT( CLSYS_AFFINETYPE_NUM > affine );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->affine = affine;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アフィンパラメータ取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLSYS_AFFINETYPE_NONE	上書きしない
 *	@retval	CLSYS_AFFINETYPE_NORMAL	通常のアフィン変換方式に設定
 *	@retval	CLSYS_AFFINETYPE_DOUBLE	倍角アフィン変換方式に設定
 */
//-----------------------------------------------------------------------------
CLSYS_AFFINETYPE GFL_CLACTWkGetAffineParam( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->affine;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標一括設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	cp_pos		アフィン変換相対座標
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAffinePos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_pos );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->affinepos = *cp_pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標一括取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	p_pos		アフィン変換相対座標取得先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_GetAffinePos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( p_pos );
	//OS_Printf( "[%d]\n", __LINE__ );
	*p_pos = cp_wk->affinepos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標個別設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pos			設定値
 *	@param	type		座標タイプ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetTypeAffinePos( GFL_CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type )
{
	GF_ASSERT( p_wk );
	// 対応していない座標タイプです
	GF_ASSERT( CLSYS_MAT_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );
	if( type == CLSYS_MAT_X ){
		p_wk->affinepos.x = pos;
	}else{
		p_wk->affinepos.y = pos;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標個別取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		座標タイプ
 *
 *	@return	座標タイプのアフィン変換相対座標
 */
//-----------------------------------------------------------------------------
s16 GFL_CLACT_WK_GetTypeAffinePos( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type )
{
	s16 pos;

	GF_ASSERT( cp_wk );
	// 対応していない座標タイプです
	GF_ASSERT( CLSYS_MAT_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );

	if( type == CLSYS_MAT_X ){
		pos = cp_wk->affinepos.x;
	}else{
		pos = cp_wk->affinepos.y;
	}
	return pos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値一括設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_sca			拡大縮小値
 *
 *	*AffineParamに
 *		CLSYS_AFFINETYPE_NORMAL		通常
 *		CLSYS_AFFINETYPE_DOUBLE		倍角
 *	が設定されていないと描画に反映されません。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetScale( GFL_CLWK* p_wk, const GFL_CLSCALE* cp_sca )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_sca );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->scale = *cp_sca;
}

//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値一括取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_sca			拡大縮小値取得先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_GetScale( const GFL_CLWK* cp_wk, GFL_CLSCALE* p_sca )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( p_sca );
	//OS_Printf( "[%d]\n", __LINE__ );
	*p_sca = cp_wk->scale;
}

//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値個別設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	scale			設定値
 *	@param	type			座標タイプ
 *
 *	*AffineParamに
 *		CLSYS_AFFINETYPE_NORMAL		通常
 *		CLSYS_AFFINETYPE_DOUBLE		倍角
 *	が設定されていないと描画に反映されません。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetTypeScale( GFL_CLWK* p_wk, fx32 scale, CLSYS_MAT_TYPE type )
{
	GF_ASSERT( p_wk );
	// 対応していない座標タイプです
	GF_ASSERT( CLSYS_MAT_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );
	if( type == CLSYS_MAT_X ){
		p_wk->scale.x = scale;
	}else{
		p_wk->scale.y = scale;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値個別取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	type			座標タイプ
 *
 *	@return	座標タイプの拡大縮小値
 */
//-----------------------------------------------------------------------------
fx32 GFL_CLACT_WK_GetTypeScale( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type )
{
	fx32 scale;

	GF_ASSERT( cp_wk );
	// 対応していない座標タイプです
	GF_ASSERT( CLSYS_MAT_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	if( type == CLSYS_MAT_X ){
		scale = cp_wk->scale.x;
	}else{
		scale = cp_wk->scale.y;
	}
	return scale;
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転角度設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	rotation		回転角度(0～0xffff 0xffffが360度)
 *
 *	*AffineParamに
 *		CLSYS_AFFINETYPE_NORMAL		通常
 *		CLSYS_AFFINETYPE_DOUBLE		倍角
 *	が設定されていないと描画に反映されません。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetRotation( GFL_CLWK* p_wk, u16 rotation )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->rotation = rotation;
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転角度を取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@return	回転角度(0～0xffff 0xffffが360度)
 */
//-----------------------------------------------------------------------------
u16  GFL_CLACT_WK_GetRotation( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->rotation;
}

//----------------------------------------------------------------------------
/**
 *	@brief		フリップ設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	flip_type		設定フリップタイプ
 *	@param	on_off			ONOFFフラグ	TRUE:On	FALSE:Off
 *
 *	*AffineParamがNNS_G2D_RND_AFFINE_OVERWRITE_NONEになっていないと反映されません
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetFlip( GFL_CLWK* p_wk, CLWK_FLIP_TYPE flip_type, BOOL on_off )
{
	u8 msk;

	GF_ASSERT( p_wk );
	// 対応していないフリップタイプです
	GF_ASSERT( CLWK_FLIP_MAX > flip_type );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	if( flip_type == CLWK_FLIP_V ){
		msk = CLSYS_VFLIP;
	}else{
		msk = CLSYS_HFLIP;
	}
	if( on_off == FALSE ){
		p_wk->vhflip &= ~msk;
	}else{
		p_wk->vhflip |= msk;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フリップ設定を取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	flip_type		フリップタイプ
 *
 *	@retval	TRUE	フリップ設定
 *	@retval	FALSE	フリップ未設定
 */
//-----------------------------------------------------------------------------
BOOL GFL_CLACT_WK_GetFlip( const GFL_CLWK* cp_wk, CLWK_FLIP_TYPE flip_type )
{
	BOOL ret = FALSE;

	//OS_Printf( "[%d]\n", __LINE__ );
	GF_ASSERT( cp_wk );
	// 対応していないフリップタイプです
	GF_ASSERT( CLWK_FLIP_MAX > flip_type );
	
	if( flip_type == CLWK_FLIP_V ){
		if( cp_wk->vhflip & CLSYS_VFLIP ){
			ret = TRUE;
		}
	}else{
		if( cp_wk->vhflip & CLSYS_HFLIP ){
			ret = TRUE;
		}
	}
	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief		オブジェモード設定
 *	
 *	@param	p_wk		セルアクターワーク
 *	@param	mode		オブジェモード
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetObjMode( GFL_CLWK* p_wk, GXOamMode mode )
{
	GF_ASSERT( p_wk );
	// 対応していないオブジェモードです
	GF_ASSERT( GX_OAM_MODE_BITMAPOBJ >= mode );

	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->objmode = mode;
	
	if(mode == GX_OAM_MODE_NORMAL){
		p_wk->over_write ^= NNS_G2D_RND_OVERWRITE_OBJMODE;
	}else{
		p_wk->over_write |= NNS_G2D_RND_OVERWRITE_OBJMODE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクトモード取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	GX_OAM_MODE_NORMAL		ノーマルOBJ
 *	@retval	GX_OAM_MODE_XLU			半透明OBJ 
 *	@retval	GX_OAM_MODE_OBJWND		OBJウィンドウ
 *	@retval	GX_OAM_MODE_BITMAPOBJ	ビットマップOBJ
 */
//-----------------------------------------------------------------------------
GXOamMode GFL_CLACT_WK_GetObjMode( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->objmode;
}

//----------------------------------------------------------------------------
/**
 *	@brief	モザイクフラグ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	on_off		モザイクON－OFF	TRUE:On	FALSE:Off
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetMosaic( GFL_CLWK* p_wk, BOOL on_off )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );

	p_wk->mosaic = on_off;
	if( on_off == TRUE ){
		p_wk->over_write |= NNS_G2D_RND_OVERWRITE_MOSAIC;
	}else{
		p_wk->over_write ^= NNS_G2D_RND_OVERWRITE_MOSAIC;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	モザイクフラグ取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	TRUE	モザイク設定
 *	@retval	FALSE	モザイク未設定
 */
//-----------------------------------------------------------------------------
BOOL GFL_CLACT_WK_GetMosaic( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->mosaic;
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pal_offs	パレットオフセット
 *
 *	パレットプロクシアドレス+pal_offs+OamAttr.c_paramの
 *	パレット番号を使用するようになります。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetPlttOffs( GFL_CLWK* p_wk, u8 pal_offs )
{
	GF_ASSERT( p_wk );
	// パレットのリミットをオーバーします
	GF_ASSERT( 16 > pal_offs );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->pal_offs = pal_offs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	pal_offs	パレットオフセット
 */
//-----------------------------------------------------------------------------
u8 GFL_CLACT_WK_GetPlttOffs( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->pal_offs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットプロクシのアドレス取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		描画タイプ
 *
 *	@retval	CLWK_VRAM_ADDR_NONE以外	パレットVramアドレス
 *	@retval	CLWK_VRAM_ADDR_NONE		読み込まれていない
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetPlttAddr( const GFL_CLWK* cp_wk, CLSYS_DRAW_TYPE type )
{
	BOOL result;

	//OS_Printf( "[%d]\n", __LINE__ );
	GF_ASSERT( cp_wk );
	// ありえない描画面タイプです
	GF_ASSERT( CLSYS_DRAW_MAX > type );

	// 描画タイプのVRAMにパレットが読み込まれているかチェック
	result = NNS_G2dIsImagePaletteReadyToUse( &cp_wk->pltt_proxy, CLSYS_DRAW_TYPEtoNNS_G2D_VRAM_TYPE[ type ] );
	if( result == FALSE ){
		return CLWK_VRAM_ADDR_NONE;
	}
	return NNS_G2dGetImagePaletteLocation( &cp_wk->pltt_proxy, CLSYS_DRAW_TYPEtoNNS_G2D_VRAM_TYPE[ type ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	新しいパレットプロクシを設定する
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_pltt			設定するパレットプロクシ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetPlttProxy( GFL_CLWK* p_wk, const NNSG2dImagePaletteProxy* cp_pltt )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_pltt );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	// 値が入っているかチェック
	{
		BOOL main_plttproxy;
		BOOL sub_plttproxy;
		main_plttproxy	= NNS_G2dIsImagePaletteReadyToUse( cp_pltt, NNS_G2D_VRAM_TYPE_2DMAIN );
		sub_plttproxy	= NNS_G2dIsImagePaletteReadyToUse( cp_pltt, NNS_G2D_VRAM_TYPE_2DSUB );
		GF_ASSERT( ((main_plttproxy == TRUE) || (sub_plttproxy == TRUE)) );
	}
	
	p_wk->pltt_proxy = *cp_pltt;
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットプロクシデータの取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pltt			パレットプロクシ格納先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_GetPlttProxy( const GFL_CLWK* cp_wk,  NNSG2dImagePaletteProxy* p_pltt )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( p_pltt );
	*p_pltt = cp_wk->pltt_proxy;
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータVramアドレスを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		描画タイプ
 *
 *	@retval	CLWK_VRAM_ADDR_NONE以外	キャラクタVramアドレス
 *	@retval	CLWK_VRAM_ADDR_NONE		読み込まれていない
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetCharAddr( const GFL_CLWK* cp_wk, CLSYS_DRAW_TYPE type )
{
	BOOL result;

	GF_ASSERT( cp_wk );
	// ありえない描画面タイプです
	GF_ASSERT( CLSYS_DRAW_MAX > type );
	//OS_Printf( "[%d]\n", __LINE__ );

	// 描画タイプのVRAMにキャラクタが読み込まれているかチェック
	result = NNS_G2dIsImageReadyToUse( &cp_wk->img_proxy, CLSYS_DRAW_TYPEtoNNS_G2D_VRAM_TYPE[ type ] );
	if( result == FALSE ){
		return CLWK_VRAM_ADDR_NONE;
	}
	return NNS_G2dGetImageLocation( &cp_wk->img_proxy, CLSYS_DRAW_TYPEtoNNS_G2D_VRAM_TYPE[ type ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	イメージプロクシを設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_img			設定イメージプロクシ
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetImgProxy( GFL_CLWK* p_wk, const NNSG2dImageProxy* cp_img )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_img );
	//OS_Printf( "[%d]\n", __LINE__ );
	

	// 値が入っているかチェック
	{
		BOOL main_imgproxy;
		BOOL sub_imgproxy;
		main_imgproxy	= NNS_G2dIsImageReadyToUse( cp_img, NNS_G2D_VRAM_TYPE_2DMAIN );
		sub_imgproxy	= NNS_G2dIsImageReadyToUse( cp_img, NNS_G2D_VRAM_TYPE_2DSUB );
		GF_ASSERT( ((main_imgproxy == TRUE) || (sub_imgproxy == TRUE)) );
	}
	
	p_wk->img_proxy = *cp_img;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イメージプロクシを取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_img			イメージプロクシ格納先
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_GetImgProxy( const GFL_CLWK* cp_wk,  NNSG2dImageProxy* p_img )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( p_img );
	*p_img = cp_wk->img_proxy;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ソフト優先順位の設定
 *	
 *	@param	p_wk			セルアクターワーク
 *	@param	pri				ソフト優先順位	0>0xff
 *	
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetSoftPri( GFL_CLWK* p_wk, u8 pri )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );

	p_wk->soft_pri = pri;
	CLUNIT_DrawListDel( p_wk->p_unit, p_wk );
	CLUNIT_DrawListAdd( p_wk->p_unit, p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ソフト優先順位の取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@return	ソフト優先順位
 */
//-----------------------------------------------------------------------------
u8 GFL_CLACT_WK_GetSoftPri( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	
	return cp_wk->soft_pri;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG優先順位の設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	pri				BG優先順位
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetBgPri( GFL_CLWK* p_wk, u8 pri )
{
	GF_ASSERT( p_wk );
	// ありえないBGプライオリティプです
	GF_ASSERT( 4 > pri );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->bg_pri = pri;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG優先順位を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	BG優先順位
 */
//-----------------------------------------------------------------------------
u8 GFL_CLACT_WK_GetBgPri( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->bg_pri;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAnmSeq( GFL_CLWK* p_wk, u16 anmseq )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->anmseq = anmseq;
	CLWK_AnmDataSetAnmSeq( &p_wk->anmdata, anmseq );

	// アニメーション開始
	GFL_CLACT_WK_StartAnm( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
u16 GFL_CLACT_WK_GetAnmSeq( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->anmseq;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスの数を取得
 *	
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションシーケンスの数
 */
//-----------------------------------------------------------------------------
u16 GFL_CLACT_WK_GetAnmSeqNum( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return CLWK_AnmDataGetAnmSeqNum( &cp_wk->anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスが変わっていたら変更する
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAnmSeqDiff( GFL_CLWK* p_wk, u16 anmseq )
{
	//OS_Printf( "[%d]\n", __LINE__ );
	GF_ASSERT( p_wk );
	if( anmseq != p_wk->anmseq ){
		GFL_CLACT_WK_SetAnmSeq( p_wk, anmseq );
		// アニメーション開始
		GFL_CLACT_WK_StartAnm( p_wk );
	}
}

#if 0	// 使用用途がわからないので排除
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更する　
 *			その際フレーム時間などのリセットを行いません。　MultiCell非対応
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAnmSeqNoReset( GFL_CLWK* p_wk, u16 anmseq )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->anmseq = anmseq;
	CLWK_AnmDataSetAnmSeqNoReset( &p_wk->anmdata, anmseq );
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	idx			フレーム数
 *	NitroCharacterのアニメーションシーケンス内のコマ番号を指定してください
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAnmFrame( GFL_CLWK* p_wk, u16 idx )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	CLWK_AnmDataSetAnmFrame( &p_wk->anmdata, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを進める
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	speed		進めるアニメーションスピード
 *
 *	*逆再生も可能です
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_AddAnmFrame( GFL_CLWK* p_wk, fx32 speed )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	CLWK_AnmDataSetAnmSpeed( &p_wk->anmdata, speed );
	CLWK_AnmDataAddAnmFrame( &p_wk->anmdata, FX32_ONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレーム数を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
u16 GFL_CLACT_WK_GetAnmFrame( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return CLWK_AnmDataGetAnmFrame( &cp_wk->anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	on_off		オートアニメーションOn-Off	TRUE:On	FALSE:Off
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAutoAnmFlag( GFL_CLWK* p_wk, BOOL on_off )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->auto_anm = on_off;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	TRUE	オートアニメON
 *	@retval	FALSE	オートアニメOFF
 */
//-----------------------------------------------------------------------------
BOOL GFL_CLACT_WK_GetAutoAnmFlag( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->auto_anm;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションスピード設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	speed		アニメーションスピード		
 *
 *	*逆再生も可能です
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAutoAnmSpeed( GFL_CLWK* p_wk, fx32 speed )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->auto_anm_speed = speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションスピードを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	オートアニメーションスピード
 */
//-----------------------------------------------------------------------------
fx32 GFL_CLACT_WK_GetAutoAnmSpeed( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return cp_wk->auto_anm_speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション開始
 *
 *	@param	p_wk		セルアクターワーク
 *	GFL_CLACT_WK_StopAnmでとめたアニメーションを開始させます。
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_StartAnm( GFL_CLWK* p_wk )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	CLWK_AnmDataStartAnm( &p_wk->anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション停止
 *
 *	@param	p_wk		セルアクターワーク
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_StopAnm( GFL_CLWK* p_wk )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	CLWK_AnmDataStopAnm( &p_wk->anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターワーク	アニメーション動作チェック
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	TRUE	再生中
 *	@retval	FALSE	停止中
 */
//-----------------------------------------------------------------------------
BOOL GFL_CLACT_WK_CheckAnmActive( const GFL_CLWK* cp_wk )
{
	GF_ASSERT( cp_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	return CLWK_AnmDataCheckAnmActive( &cp_wk->anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションリスタート
 *
 *	@param	p_wk	セルアクターワーク
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_ResetAnm( GFL_CLWK* p_wk )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	CLWK_AnmDataResetAnm( &p_wk->anmdata );
	CLWK_AnmDataStartAnm( &p_wk->anmdata );	// アクティブフラグを初期化しないので、手動で再開させる
}

//----------------------------------------------------------------------------
/**
 *	@brief	上書きアニメーション動作モード設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	playmode	動作モード
 *
 *	playmode
 *	CLSYS_ANMPM_INVALID		無効（これを設定するとNitroCharacterで設定したのアニメ方式になります）
 *	CLSYS_ANMPM_FORWARD		ワンタイム再生(順方向)
 *	CLSYS_ANMPM_FORWARD_L	リピート再生(順方向ループ)
 *	CLSYS_ANMPM_REVERSE		往復再生(リバース（順＋逆方向）
 *	CLSYS_ANMPM_REVERSE_L	往復再生リピート（リバース（順＋逆順方向） ループ）
 *		
 */
//-----------------------------------------------------------------------------
void GFL_CLACT_WK_SetAnmMode( GFL_CLWK* p_wk, CLSYS_ANM_PLAYMODE playmode )
{
	GF_ASSERT( p_wk );
	//OS_Printf( "[%d]\n", __LINE__ );
	p_wk->playmode = playmode;
	CLWK_AnmDataSetAnmMode( &p_wk->anmdata, playmode );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	上書きアニメーション方式を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLSYS_ANMPM_INVALID		無効(NitroCharacterで設定したアニメ方式でアニメしています)
 *	@retval	CLSYS_ANMPM_FORWARD		ワンタイム再生(順方向)
 *	@retval	CLSYS_ANMPM_FORWARD_L	リピート再生(順方向ループ)
 *	@retval	CLSYS_ANMPM_REVERSE		往復再生(リバース（順＋逆方向）
 *	@retval	CLSYS_ANMPM_REVERSE_L	往復再生リピート（リバース（順＋逆順方向） ループ）
 */
//-----------------------------------------------------------------------------
CLSYS_ANM_PLAYMODE GFL_CLACT_WK_GetAnmMode( const GFL_CLWK* cp_wk )
{
	//OS_Printf( "[%d]\n", __LINE__ );
	GF_ASSERT( cp_wk );
	return cp_wk->playmode;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	seq			シーケンスナンバー
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetUserAttrAnmSeq( const GFL_CLWK* cp_wk, u32 seq )
{
	const NNSG2dUserExAnimSequenceAttr* p_seqattr;
	GF_ASSERT( cp_wk );
	// シーケンスの拡張アトリビュートデータ取得
	p_seqattr = CLWK_AnmDataGetUserAnimSeqAttr( &cp_wk->anmdata, seq );
	if( p_seqattr ){
		return NNS_G2dGetUserExAnimSeqAttrValue( p_seqattr );
	}
	return CLWK_USERATTR_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今のアニメーションシーケンスのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetUserAttrAnmSeqNow( const GFL_CLWK* cp_wk )
{
	u32 seq;
	seq = GFL_CLACT_WK_GetAnmSeq( cp_wk );
	return GFL_CLACT_WK_GetUserAttrAnmSeq( cp_wk, seq );
}
	
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	seq			シーケンスナンバー
 *	@param	frame		フレームナンバー
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetUserAttrAnmFrame( const GFL_CLWK* cp_wk, u32 seq, u32 frame )
{
	const NNSG2dUserExAnimSequenceAttr* p_seqattr;
	const NNSG2dUserExAnimFrameAttr* p_frameattr;
	GF_ASSERT( cp_wk );
	// シーケンスの拡張アトリビュートデータ取得
	p_seqattr = CLWK_AnmDataGetUserAnimSeqAttr( &cp_wk->anmdata, seq );
	if( p_seqattr ){
		p_frameattr =  NNS_G2dGetUserExAnimFrameAttr( p_seqattr, frame );
		if( p_frameattr ){
			return NNS_G2dGetUserExAnimFrmAttrValue( p_frameattr );
		}
	}
	return CLWK_USERATTR_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今のアニメーションフレームのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetUserAttrAnmFrameNow( const GFL_CLWK* cp_wk )
{
	u32 seq;
	u32 frame;

	seq = GFL_CLACT_WK_GetAnmSeq( cp_wk );
	frame = GFL_CLACT_WK_GetAnmFrame( cp_wk );
	return GFL_CLACT_WK_GetUserAttrAnmFrame( cp_wk, seq, frame );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルのユーザー拡張アトリビュートデータの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	cellidx		セルインデックス
 *
 *	@retval	CLWK_USERATTR_NONE	拡張アトリビュートデータなし
 *	@retval	それ以外			拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
u32 GFL_CLACT_WK_GetUserAttrCell( const GFL_CLWK* cp_wk, u32 cellidx )
{
	return CLWK_AnmDataGetUserCellAttr( &cp_wk->anmdata, cellidx );
}





//-----------------------------------------------------------------------------
/**
 *				プライベート関数郡
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	CLSYS関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	デフォルト設定レンダラー作成
 *
 *	@param	p_rend		レンダラーシステム
 *	@param	cp_data		初期化データ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void CLSYS_DefaultRendInit( GFL_CLSYS_REND* p_rend, const GFL_CLSYS_INIT* cp_data, HEAPID heapID )
{
	// デフォルトレンダラー　サーフェース設定
	static GFL_REND_SURFACE_INIT c_defsurface[ CLSYS_DRAW_MAX ] = {
		{
			0, 0,
			256, 192,
			CLSYS_DRAW_MAIN
		},
		{
			0, 0,
			256, 192,
			CLSYS_DRAW_SUB
		}
	};

	// 左上座標だけ設定
	c_defsurface[ CLSYS_DEFREND_MAIN ].lefttop_x = cp_data->surface_main_left;
	c_defsurface[ CLSYS_DEFREND_MAIN ].lefttop_y = cp_data->surface_main_top;
	c_defsurface[ CLSYS_DEFREND_SUB ].lefttop_x = cp_data->surface_sub_left;
	c_defsurface[ CLSYS_DEFREND_SUB ].lefttop_y = cp_data->surface_sub_top;
	
	REND_SysInit( p_rend, c_defsurface, CLSYS_DRAW_MAX, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステムにパレットプロクシを設定
 *	
 *	@param	cp_pltt		パレットプロクシ
 *	@param	pal_offs	パレットオフセット
 */	
//-----------------------------------------------------------------------------
static void CLSYS_SysSetPaletteProxy( const NNSG2dImagePaletteProxy* cp_pltt, u8 pal_offs )
{
	GF_ASSERT( pClsys );
	GF_ASSERT( cp_pltt );
	// ありえないパレット番号です
	GF_ASSERT( 16 > pal_offs );
	
	// パレットプロクシからアドレスを取得する
	pClsys->pltt_no[ CLSYS_DRAW_MAIN ] = NNS_G2dGetImagePaletteLocation( cp_pltt, NNS_G2D_VRAM_TYPE_2DMAIN ) / 32;
	pClsys->pltt_no[ CLSYS_DRAW_SUB ] = NNS_G2dGetImagePaletteLocation( cp_pltt, NNS_G2D_VRAM_TYPE_2DSUB ) / 32;

	pClsys->pltt_no[ CLSYS_DRAW_MAIN ] += pal_offs;
	pClsys->pltt_no[ CLSYS_DRAW_SUB ] += pal_offs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送許可フラグの設定
 *
 *	@param	flag	フラグ
 */
//-----------------------------------------------------------------------------
static void CLSYS_SetTransFlag( BOOL flag )
{
	if( pClsys ){
		pClsys->transflag = flag;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送許可フラグの取得
 *
 *	@retval	TRUE	転送可
 *	@retval	FALSE	転送不可
 */
//-----------------------------------------------------------------------------
static BOOL CLSYS_GetTransFlag( void )
{
	if( pClsys ){
		return pClsys->transflag;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMバッファのクリア許可フラグ設定
 *
 *	@param	flag	設定フラグ
 */
//-----------------------------------------------------------------------------
static void CLSYS_SetOamManClearFlag( BOOL flag )
{
	if( pClsys ){
		pClsys->oammanclear = flag;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMバッファのクリア許可フラグ取得
 *
 *	@retval	TRUE	クリア可
 *	@retval	FALSE	クリア不可
 */
//-----------------------------------------------------------------------------
static BOOL CLSYS_GetOamManClearFlag( void )
{
	if( pClsys ){
		return pClsys->oammanclear;
	}
	return FALSE;
}




//-------------------------------------
///	CLSYS_OamMan関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャシステムの初期化
 *
 *	@param	p_oamman		OAMマネージャシステム
 *	@param	oamst_main		メイン画面OAM管理開始位置
 *	@param	oamnum_main		メイン画面OAM管理数
 *	@param	oamst_sub		サブ画面OAM管理開始位置
 *	@param	oamnum_sub		サブ画面OAM管理数
 */
//-----------------------------------------------------------------------------
static void OAMMAN_SysInit( CLSYS_OAMMAN* p_oamman, u8 oamst_main, u8 oamnum_main, u8 oamst_sub, u8 oamnum_sub )
{
	GF_ASSERT( p_oamman );
	
	// NitroSystem OamManagerSystemの初期化
	NNS_G2dInitOamManagerModule();

	// メインとサブのOAMマネージャインスタンスを作成
	OAMMAN_ObjCreate( &p_oamman->man[NNS_G2D_OAMTYPE_MAIN], oamst_main, oamnum_main, NNS_G2D_OAMTYPE_MAIN );
	OAMMAN_ObjCreate( &p_oamman->man[NNS_G2D_OAMTYPE_SUB], oamst_sub, oamnum_sub, NNS_G2D_OAMTYPE_SUB );
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャシステムの破棄
 *
 *	@param	p_oamman		OAMマネージャシステム
 */
//-----------------------------------------------------------------------------
static void OAMMAN_SysExit( CLSYS_OAMMAN* p_oamman )
{
	GF_ASSERT( p_oamman );	// OAMMANを持っていないならこの処理を行うことが出来ないようにするため

	// 描画していたOAMRAM状態をクリアする
	OAMMAN_SysClean( p_oamman );
	OAMMAN_SysTrans( p_oamman );
	
	// NitroSystem OamManagerSystemの初期化
	NNS_G2dInitOamManagerModule();
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャシステム	転送処理
 *
 *	@param	p_oamman		OAMマネージャシステム
 */
//-----------------------------------------------------------------------------
static void OAMMAN_SysTrans( CLSYS_OAMMAN* p_oamman )
{
	int i;
	GF_ASSERT( p_oamman );
	for( i=0; i<CLSYS_DRAW_MAX; i++ ){
		OAMMAN_ObjTrans( &p_oamman->man[ i ] );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャシステム　バッファクリーン処理
 *
 *	@param	p_oamman		OAMマネージャシステム
 */
//-----------------------------------------------------------------------------
static void OAMMAN_SysClean( CLSYS_OAMMAN* p_oamman )
{
	int i;
	GF_ASSERT( p_oamman );
	for( i=0; i<CLSYS_DRAW_MAX; i++ ){
		OAMMAN_ObjClearBuff( &p_oamman->man[ i ] );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャオブジェクトの初期化
 *
 *	@param	p_obj			OAMマネージャオブジェクト
 *	@param	oamst			OAM管理開始位置
 *	@param	oamnum			OAM管理数
 *	@param	oam_type		OAMタイプ
 */
//-----------------------------------------------------------------------------
static void OAMMAN_ObjCreate( OAMMAN_DATA* p_obj, u8 oamst, u8 oamnum, NNSG2dOamType oam_type )
{
	BOOL result;

	GF_ASSERT( p_obj );

	if( oamnum == 0 ){
		p_obj->init = FALSE;
	}else{
		
		// 高速転送ﾓｰﾄﾞでOAMマネージャを初期化する
		result = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode(
				&p_obj->man,
				oamst,
				oamnum,
				oam_type
				);
		// OAMマネージャを作成できません。
		// GFL_CLSYS_INITのOAM管理情報を変更する必要があります。
		GF_ASSERT( result == TRUE );

		p_obj->init = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャオブジェクトにOAMAttrを登録
 *
 *	@param	p_obj			OAMマネージャオブジェクト
 *	@param	pOam			OAMAttr
 *	@param	affineIndex		アフィンIndex
 *
 *	@retval	TRUE	登録成功
 *	@retval	FALSE	登録失敗
 */
//-----------------------------------------------------------------------------
static BOOL OAMMAN_ObjEntryOamAttr( OAMMAN_DATA* p_obj, const GXOamAttr* pOam, u16 affineIndex )
{
	GF_ASSERT( p_obj );
	GF_ASSERT( p_obj->init );	// 初期化されていない
	GF_ASSERT( pOam );
	
	return NNS_G2dEntryOamManagerOamWithAffineIdx(
			&p_obj->man,
			pOam,
			affineIndex
			);
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャオブジェクトにアフィン行列を登録
 *
 *	@param	p_obj			OAMマネージャオブジェクト
 *	@param	mtx				アフィンマトリクス
 *
 *	@return	登録したアフィンIndex
 */
//-----------------------------------------------------------------------------
static u16 OAMMAN_ObjEntryAffine( OAMMAN_DATA* p_obj, const MtxFx22* mtx )
{
	GF_ASSERT( p_obj );
	GF_ASSERT( p_obj->init );	// 初期化されていない
	GF_ASSERT( mtx );
	return NNS_G2dEntryOamManagerAffine(
			&p_obj->man,
			mtx
			);
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャオブジェクト　RAM転送処理
 *
 *	@param	 p_obj			OAMマネージャオブジェクト
 */
//-----------------------------------------------------------------------------
static void OAMMAN_ObjTrans( OAMMAN_DATA* p_obj )
{
	GF_ASSERT( p_obj );
	if( p_obj->init ){
		NNS_G2dApplyOamManagerToHW( &p_obj->man );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャオブジェクト　バッファクリア
 *
 *	@param	p_obj			OAMマネージャオブジェクト
 */
//-----------------------------------------------------------------------------
static void OAMMAN_ObjClearBuff( OAMMAN_DATA* p_obj )
{
	GF_ASSERT( p_obj );
	if( p_obj->init ){
		NNS_G2dResetOamManagerBuffer( &p_obj->man );
	}
}


//-------------------------------------
///	GFL_CLSYS_REND関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステムの初期化
 *
 *	@param	p_rend		レンダラーシステム
 *	@param	cp_data		登録サーフェースデータ
 *	@param	data_num	データ数
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void REND_SysInit( GFL_CLSYS_REND* p_rend, const GFL_REND_SURFACE_INIT* cp_data, u32 data_num, HEAPID heapID )
{
	int i;

	GF_ASSERT( p_rend );
	GF_ASSERT( cp_data );
	
	// レンダラー初期化
	NNS_G2dInitRenderer( &p_rend->rend );
	NNS_G2dSetRendererSpriteZoffset( &p_rend->rend, CLSYS_DEFFREND_ZOFFS );

	
	// サーフェースオブジェクトメモリ確保
	p_rend->p_surface = GFL_HEAP_AllocMemory( heapID, sizeof(NNSG2dRenderSurface)*data_num );
	p_rend->surface_num = data_num;

	// サーフェースオブジェクト初期化
	// レンダラーに登録
	for( i=0; i<data_num; i++ ){
		REND_SurfaceObjCreate( &p_rend->p_surface[i], &cp_data[i] );
		NNS_G2dAddRendererTargetSurface( &p_rend->rend, &p_rend->p_surface[i] );
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステムの破棄
 *
 *	@param	p_rend		レンダラーシステム
 */
//-----------------------------------------------------------------------------
static void REND_SysExit( GFL_CLSYS_REND* p_rend )
{
	GF_ASSERT( p_rend );
	// サーフェースオブジェクトメモリ破棄
	GFL_HEAP_FreeMemory( p_rend->p_surface );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステム　描画前処理
 *
 *	@param	p_rend			レンダラーシステム
 *	@param	pImgProxy		イメージプロクシ
 *	@param	pPltProxy		パレットプロクシ
 */
//-----------------------------------------------------------------------------
static void REND_SysBeginDraw( GFL_CLSYS_REND* p_rend, const NNSG2dImageProxy* pImgProxy, const NNSG2dImagePaletteProxy* pPltProxy )
{
	GF_ASSERT( p_rend );
	GF_ASSERT( pImgProxy );
	GF_ASSERT( pPltProxy );
	NNS_G2dSetRendererImageProxy( &p_rend->rend, pImgProxy, pPltProxy );
	NNS_G2dBeginRendering( &p_rend->rend );

}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステム	描画後処理
 *
 *	@param	p_rend			レンダラーシステム
 */
//-----------------------------------------------------------------------------
static void REND_SysEndDraw( GFL_CLSYS_REND* p_rend )
{
	GF_ASSERT( p_rend );
	NNS_G2dEndRendering();
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーにアフィン変換上書き設定を行う
 *
 *	@param	p_rend			レンダラーシステム
 *	@param	mode			アフィン変換モード
 */
//-----------------------------------------------------------------------------
static void REND_SysSetAffine( GFL_CLSYS_REND* p_rend, CLSYS_AFFINETYPE mode )
{
	GF_ASSERT( p_rend );
	// 不正なアフィンタイプ
	GF_ASSERT( CLSYS_AFFINETYPE_NUM > mode );
	NNS_G2dSetRndCoreAffineOverwriteMode(
			&(p_rend->rend.rendererCore),
			mode );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステム　フリップ設定を行う
 *
 *	@param	p_rend		レンダラーシステム
 *	@param	vflip		Vフリップ有無
 *	@param	hflip		Hフリップ有無
 */
//-----------------------------------------------------------------------------
static void REND_SysSetFlip( GFL_CLSYS_REND* p_rend, BOOL vflip, BOOL hflip )
{
	GF_ASSERT( p_rend );
	
	NNS_G2dSetRndCoreFlipMode(
			&(p_rend->rend.rendererCore),
			vflip,
			hflip );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステムにオーバーライドフラグを設定
 *
 *	@param	p_rend			レンダラーシステム
 *	@param	over_write		オーバーライドフラグ
 */
//-----------------------------------------------------------------------------
static void REND_SysSetOverwrite( GFL_CLSYS_REND* p_rend, u8 over_write )
{
	GF_ASSERT( p_rend );
	// オーバーライト有効フラグ設定
	NNS_G2dSetRendererOverwriteEnable( &p_rend->rend, over_write );
	// オーバーライト無効フラグ設定
	NNS_G2dSetRendererOverwriteDisable( &p_rend->rend, ~over_write );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステムにモザイクの有無を設定
 *
 *	@param	p_rend			レンダラーシステム
 *	@param	on_off			モザイクの有無
 */
//-----------------------------------------------------------------------------
static void REND_SysSetMosaicFlag( GFL_CLSYS_REND* p_rend, BOOL on_off )
{
	GF_ASSERT( p_rend );
	// オーバーライトにモザイクの値を設定
	NNS_G2dSetRendererOverwriteMosaicFlag( &p_rend->rend, on_off );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステムに上書きするオブジェモードを設定
 *
 *	@param	p_rend			レンダラーシステム
 *	@param	objmode			オブジェモード
 */
//-----------------------------------------------------------------------------
static void REND_SysSetOBJMode( GFL_CLSYS_REND* p_rend, GXOamMode objmode )
{
	GF_ASSERT( p_rend );
	// 不正なオブジェモードです
	GF_ASSERT( GX_OAM_MODE_BITMAPOBJ > objmode );
	// オーバーライトにOBJモードの値を設定
	NNS_G2dSetRendererOverwriteOBJMode( &p_rend->rend, objmode );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーシステムに上書きするBG優先順位を設定
 *
 *	@param	p_rend			レンダラーシステム
 *	@param	pri				BG優先順位
 */
//-----------------------------------------------------------------------------
static void REND_SysSetBGPriority( GFL_CLSYS_REND* p_rend, u8 pri )
{
	GF_ASSERT( p_rend );
	// 不正なプライオリティです
	GF_ASSERT( 4 > pri );
	// BG優先順位を設定
	NNS_G2dSetRendererOverwritePriority( &p_rend->rend, pri );
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクト初期化
 *
 *	@param	p_surface		サーフェースオブジェクト
 *	@param	cp_data			サーフェースデータ
 */
//-----------------------------------------------------------------------------
static void REND_SurfaceObjCreate( NNSG2dRenderSurface* p_surface, const GFL_REND_SURFACE_INIT* cp_data )
{
	// コールバックデータ
	static const REND_SURFACE_CALLBACK callback[ CLSYS_DRAW_MAX ] = {
		{
			REND_CallBackMainAddOam,
			REND_CallBackMainAddAffine,
			REND_CallBackCulling
		},
		{
			REND_CallBackSubAddOam,
			REND_CallBackSubAddAffine,
			REND_CallBackCulling
		},
	};

	GF_ASSERT( cp_data );
	// 不正な描画面です
	GF_ASSERT( CLSYS_DRAW_MAX > cp_data->type );

	NNS_G2dInitRenderSurface(p_surface);
	
	// サーフェースデータ設定
	REND_SurfaceObjSetPos( p_surface, cp_data->lefttop_x, cp_data->lefttop_y );
	REND_SurfaceObjSetSize( p_surface, cp_data->width, cp_data->height );
	REND_SurfaceObjSetType( p_surface, cp_data->type );
	REND_SurfaceObjSetCallBack( p_surface, &callback[ cp_data->type ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクトに左上座標を設定する
 *
 *	@param	p_surface		サーフェースオブジェクト
 *	@param	x				左上ｘ座標
 *	@param	y				左上ｙ座標
 */
//-----------------------------------------------------------------------------
static void REND_SurfaceObjSetPos( NNSG2dRenderSurface* p_surface, s16 x, s16 y )
{
	GF_ASSERT( p_surface );
	p_surface->viewRect.posTopLeft.x = x << FX32_SHIFT;
	p_surface->viewRect.posTopLeft.y = y << FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクトの大きさを設定
 *
 *	@param	p_surface		サーフェースオブジェクト
 *	@param	width			幅
 *	@param	height			高さ
 */
//-----------------------------------------------------------------------------
static void REND_SurfaceObjSetSize( NNSG2dRenderSurface* p_surface, s16 width, s16 height )
{
	GF_ASSERT( p_surface );
	p_surface->viewRect.sizeView.x = width << FX32_SHIFT;
	p_surface->viewRect.sizeView.y = height << FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクトのタイプを設定
 *
 *	@param	p_surface		サーフェースオブジェクト
 *	@param	type			サーフェースタイプ
 */	
//-----------------------------------------------------------------------------
static void REND_SurfaceObjSetType( NNSG2dRenderSurface* p_surface, CLSYS_DRAW_TYPE type )
{
	static const u16 type_dict[ CLSYS_DRAW_MAX ] = {
		NNS_G2D_SURFACETYPE_MAIN2D,
		NNS_G2D_SURFACETYPE_SUB2D,
	};

	GF_ASSERT( p_surface );
	// 不正な描画面です
	GF_ASSERT( CLSYS_DRAW_MAX > type );

	p_surface->type = type_dict[ type ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクトにデフォルトのコールバック関数を設定
 *
 *	@param	p_surface		サーフェースオブジェクト
 *	@param	cp_callback		コールバックデータ
 */
//-----------------------------------------------------------------------------
static void REND_SurfaceObjSetCallBack( NNSG2dRenderSurface* p_surface, const REND_SURFACE_CALLBACK* cp_callback )
{
	GF_ASSERT( p_surface );
	GF_ASSERT( cp_callback );
	p_surface->pFuncOamRegister			= cp_callback->pFuncOamRegister;
	p_surface->pFuncOamAffineRegister	= cp_callback->pFuncOamAffineRegister;
	p_surface->pFuncVisibilityCulling	= cp_callback->pFuncVisibilityCulling;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクト左上座標を取得
 *	
 *	@param	cp_surface		サーフェースオブジェクト
 *	@param	p_x				左上ｘ座標格納先
 *	@param	p_y				左上ｙ座標格納先
 */
//-----------------------------------------------------------------------------
static void REND_SurfaceObjGetPos( const NNSG2dRenderSurface* cp_surface, s16* p_x, s16* p_y )
{
	GF_ASSERT( cp_surface );
	GF_ASSERT( p_x );
	GF_ASSERT( p_y );
	*p_x = cp_surface->viewRect.posTopLeft.x >> FX32_SHIFT;
	*p_y = cp_surface->viewRect.posTopLeft.y >> FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクトの大きさを取得
 *
 *	@param	cp_surface		サーフェースオブジェクト
 *	@param	p_width			幅格納先
 *	@param	p_height		高さ格納先
 */
//-----------------------------------------------------------------------------
static void REND_SurfaceObjGetSize( const NNSG2dRenderSurface* cp_surface, s16* p_width, s16* p_height )
{
	GF_ASSERT( cp_surface );
	GF_ASSERT( p_width );
	GF_ASSERT( p_height );
	*p_width	= cp_surface->viewRect.sizeView.x >> FX32_SHIFT;
	*p_height	= cp_surface->viewRect.sizeView.y >> FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェースオブジェクトタイプ取得
 *
 *	@param	cp_surface				サーフェースオブジェクト
 *
 *	@retval	CLSYS_DRAW_MAIN		メイン画面用サーフェース
 *	@retval	CLSYS_DRAW_SUB		サブ画面用サーフェース
 */
//-----------------------------------------------------------------------------
static CLSYS_DRAW_TYPE REND_SurfaceObjGetType( const NNSG2dRenderSurface* cp_surface )
{
	GF_ASSERT( cp_surface );
	if( cp_surface->type == NNS_G2D_SURFACETYPE_MAIN2D ){
		return CLSYS_DRAW_MAIN;
	}else if( cp_surface->type == NNS_G2D_SURFACETYPE_SUB2D ){
		return CLSYS_DRAW_SUB;
	}
	// 不正なサーフェースタイプ
	GF_ASSERT( 0 );
	return CLSYS_DRAW_MAX;
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャにOAMAttrを登録する処理
 *
 *	@param	p_obj			OAMマネージャオブジェクト
 *	@param	pOam			登録するOamAttr	
 *	@param	affineIndex		アフィンIndex
 *	@param	pltt_no			上書きパレットナンバー
 *
 *	@retval	TRUE	登録成功
 *	@retval	FALSE	登録失敗
 */
//-----------------------------------------------------------------------------
static BOOL REND_CallBackAddOam( OAMMAN_DATA* p_obj, const GXOamAttr* pOam, u16 affineIndex, u32 pltt_no )
{
	BOOL result;
	GXOamAttr oamattr;

	GF_ASSERT( pOam );

	oamattr = *pOam;

	// パレットナンバーをOamAttrに上書きする
	if( (oamattr.cParam + pltt_no) <= 0xf ){
		oamattr.cParam += pltt_no;
	}else{
		// 表示パレットNoが15以上になりました。
		// そのパレット位置だと色が出ないです。
		//
		// また、描画可能では無いサーフェースの描画エリアに座標が入ってしまってる
		// 可能性があります。
		GF_ASSERT( 0 );
	}
	
	// OAMマネージャに登録
	result = OAMMAN_ObjEntryOamAttr( p_obj, &oamattr, affineIndex );

	// OAM登録失敗　
	// OAM登録可能数より多く描画しようとしました。
	GF_ASSERT( result );
	return result;

}
// メイン画面用
static BOOL REND_CallBackMainAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
	GF_ASSERT( pClsys );

	return REND_CallBackAddOam( &pClsys->oamman.man[ CLSYS_DRAW_MAIN ], pOam, 
			affineIndex, 
			pClsys->pltt_no[ CLSYS_DRAW_MAIN ] );
}
// サブ画面用
static BOOL REND_CallBackSubAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
	GF_ASSERT( pClsys );
	return REND_CallBackAddOam( &pClsys->oamman.man[ CLSYS_DRAW_SUB ], pOam, 
			affineIndex,
			pClsys->pltt_no[ CLSYS_DRAW_SUB ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMマネージャにアフィン行列を設定する
 *
 *	@param	mtx		アフィン行列
 *
 *	@return	アフィンIndex
 */
//-----------------------------------------------------------------------------
// メイン画面用
static u16 REND_CallBackMainAddAffine( const MtxFx22* mtx )
{
	u16 result;
	GF_ASSERT( pClsys );
	result = OAMMAN_ObjEntryAffine( &pClsys->oamman.man[ CLSYS_DRAW_MAIN ], mtx );
	// アフィンインデックス登録失敗
	// 登録可能最大数を超えています
	GF_ASSERT( result != NNS_G2D_OAM_AFFINE_IDX_NONE );
	return result;
}
// サブ画面用
static u16 REND_CallBackSubAddAffine( const MtxFx22* mtx )
{
	u16 result;
	GF_ASSERT( pClsys );
	result = OAMMAN_ObjEntryAffine( &pClsys->oamman.man[ CLSYS_DRAW_SUB ], mtx );
	// アフィンインデックス登録失敗
	// 登録可能最大数を超えています
	GF_ASSERT( result != NNS_G2D_OAM_AFFINE_IDX_NONE );
	return result;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース領域内にセルデータがいるかをチェックする処理
 *
 *	@param	pCell			表示するセルデータ
 *	@param	pMtx			表示位置行列
 *	@param	pViewRect		サーフェース領域
 *
 *	@retval	TRUE	表示する
 *	@retval	FALSE	表示しない
 */
//-----------------------------------------------------------------------------
static BOOL REND_CallBackCulling( const NNSG2dCellData* pCell, const MtxFx32* pMtx, const NNSG2dViewRect* pViewRect )
{
	// コンバータにより計算された境界球半径を取得する
	const NNSG2dCellBoundingRectS16* Rect = NNS_G2dGetCellBoundingRect( pCell );
    const fx32  R = NNS_G2dGetCellBoundingSphereR( pCell );
    // セルの表示位置原点を求める
    const fx32  px = pMtx->_20 - pViewRect->posTopLeft.x;
    const fx32  py = pMtx->_21 - pViewRect->posTopLeft.y;
	// セル矩形
	fx32  minY; 
	fx32  maxY; 
	fx32  minX; 
	fx32  maxX; 
	fx32  work;

	// セルの境界球を内包する矩形を求める
	// セル定義位置を中心位置にしたとき
	if( NNS_G2dCellHasBR( pCell ) == TRUE ){
		minY = Rect->minY << FX32_SHIFT;
		maxY = Rect->maxY << FX32_SHIFT;
		minX = Rect->minX << FX32_SHIFT;
		maxX = Rect->maxX << FX32_SHIFT;
	}else{
		minY = -R << FX32_SHIFT;
		maxY = R << FX32_SHIFT;
		minX = -R << FX32_SHIFT;
		maxX = R << FX32_SHIFT;
	}
	
	// 座標計算で行列計算後の座標に変換
	minY = FX_Mul(minY, pMtx->_01) + FX_Mul(minY, pMtx->_11) + py;
	maxY = FX_Mul(maxY, pMtx->_01) + FX_Mul(maxY, pMtx->_11) + py;
	minX = FX_Mul(minX, pMtx->_00) + FX_Mul(minX, pMtx->_10) + px;
	maxX = FX_Mul(maxX, pMtx->_00) + FX_Mul(maxX, pMtx->_10) + px;

	// 座標が逆転している可能性があるのでチェック
	if( maxY < minY ){
		work = maxY;
		maxY = minY;
		minY = work;
	}

	if( maxX < minX ){
		work = maxX;
		maxX = minX;
		minX = work;
	}

    if( (maxY > 0) && (minY < pViewRect->sizeView.y) ){
        if( (maxX > 0) && (minX < pViewRect->sizeView.x) ){
			return TRUE;
        }
    }
	
	return FALSE;
}


//-------------------------------------
///	TRMAN	関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメシステム　初期化
 *
 *	@param	p_trman		セル転送アニメシステム
 *	@param	tr_cell		セル転送バッファ数
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void TRMAN_SysInit( CLSYS_TRMAN* p_trman, u8 tr_cell, HEAPID heapID )
{
	GF_ASSERT( p_trman );

	if( tr_cell == 0 ){
		p_trman->init = FALSE;
	}else{
		p_trman->data_num = tr_cell;
		TRMAN_CellTransManCreate( p_trman, tr_cell, heapID );
		TRMAN_TrSysCreate( p_trman, tr_cell, heapID );
		p_trman->init = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメシステム　破棄
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_SysExit( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );
	if( p_trman->init ){
		TRMAN_CellTransManDelete( p_trman );
		TRMAN_TrSysDelete( p_trman );
		p_trman->init = FALSE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメシステム　メイン処理
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_SysMain( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );
	if( p_trman->init ){
		TRMAN_TrSysUpdata( p_trman );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメシステム　Vブランク処理
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_SysVBlank( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );
	if( p_trman->init ){
		TRMAN_TrSysTrans( p_trman );
	}	
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメシステム　バッファ数を取得
 *
 *	@param	cp_trman	セル転送アニメシステム
 *
 *	@return	バッファ数
 */
//-----------------------------------------------------------------------------
static u8 TRMAN_SysGetBuffNum( const CLSYS_TRMAN* cp_trman )
{
	GF_ASSERT( cp_trman );
	return cp_trman->data_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメマネージャ生成
 *
 *	@param	p_trman		セル転送アニメシステム
 *	@param	tr_cell		バッファ作成数
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void TRMAN_CellTransManCreate( CLSYS_TRMAN* p_trman, u8 tr_cell, HEAPID heapID )
{
	GF_ASSERT( p_trman );

	p_trman->p_cell = GFL_HEAP_AllocMemory(heapID, sizeof(NNSG2dCellTransferState)*tr_cell);
	
	NNS_G2dInitCellTransferStateManager(
				p_trman->p_cell,
				tr_cell,
				TRMAN_TrSysCallBackAddTransData
			);
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送アニメマネージャ破棄
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_CellTransManDelete( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );
	GFL_HEAP_FreeMemory( p_trman->p_cell );
	p_trman->p_cell = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セル転送状態オブジェクトハンドルを取得する
 *
 *	@param	p_trman		セル転送アニメシステム
 *
 *	@return	セル転送状態オブジェクトのハンドル
 */
//-----------------------------------------------------------------------------
static u32 TRMAN_CellTransManAddHandle( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );	// CLSYS_TRMANを持っているシステムしか呼んではいけない
	GF_ASSERT( p_trman->init );// CLSYS_TRMANを作成していない	
	return NNS_G2dGetNewCellTransferStateHandle();
}

//----------------------------------------------------------------------------
/**
 	@brief	セル転送状態オブジェクトハンドルを破棄する
 *
 *	@param	p_trman		セル転送アニメシステム
 *	@param	handle		ハンドル
 */
//-----------------------------------------------------------------------------
static void TRMAN_CellTransManDelHandle( CLSYS_TRMAN* p_trman, u32 handle )
{
	GF_ASSERT( p_trman );	// CLSYS_TRMANを持っているシステムしか呼んではいけない
	GF_ASSERT( p_trman->init );// CLSYS_TRMANを作成していない
	NNS_G2dFreeCellTransferStateHandle( handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送システム生成
 *
 *	@param	p_trman		セル転送アニメシステム
 *	@param	tr_cell		作成するバッファ数
 *	@param	heapID		ヒープ
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrSysCreate( CLSYS_TRMAN* p_trman, u8 tr_cell, HEAPID heapID )
{
	int i;

	GF_ASSERT( p_trman );
	p_trman->p_trdata = GFL_HEAP_AllocMemory( heapID, sizeof(TRMAN_DATA)*tr_cell );
	for( i=0; i<p_trman->data_num; i++ ){
		TRMAN_TrDataClean( &p_trman->p_trdata[i] );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送システム破棄
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrSysDelete( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );
	GFL_HEAP_FreeMemory( p_trman->p_trdata );
	p_trman->p_trdata = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送システム	セルアニメ更新処理
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrSysUpdata( CLSYS_TRMAN* p_trman )
{
	GF_ASSERT( p_trman );	// CLSYS_TRMANを持っているシステムしか呼んではいけない
	GF_ASSERT( p_trman->init );// CLSYS_TRMANを作成していない
	NNS_G2dUpdateCellTransferStateManager();
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送システム　転送処理
 *
 *	@param	p_trman		セル転送アニメシステム
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrSysTrans( CLSYS_TRMAN* p_trman )
{
	int i;
	
	GF_ASSERT( p_trman );

	for( i=0; i<p_trman->data_num; i++ ){
		// データ設定されているかチェック
		if( TRMAN_TrDataCheckClean( &p_trman->p_trdata[i] ) == FALSE ){
			// 転送実行
			TRMAN_TrDataTrans( &p_trman->p_trdata[i] );
			TRMAN_TrDataClean( &p_trman->p_trdata[i] );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送システム　空いているバッファを取得
 *
 *	@param	p_trman		セル転送アニメシステム
 *
 *	@return	空いているバッファポインタ
 */
//-----------------------------------------------------------------------------
static TRMAN_DATA* TRMAN_TrSysGetCleanBuff( CLSYS_TRMAN* p_trman )
{
	int i;

	GF_ASSERT( p_trman );

	for( i=0; i<p_trman->data_num; i++ ){
		// データ設定されているかチェック
		if( TRMAN_TrDataCheckClean( &p_trman->p_trdata[i] ) ){
			return &p_trman->p_trdata[i];
		}
	}
	// もう空きがない
	// GF_CLACT_SysVBlank関数を呼んでいない可能性があります。
	GF_ASSERT( 0 );
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送データをバッファに設定
 *
 *	@param	type		転送タイプ
 *	@param	dstAddr		転送先アドレス
 *	@param	pSrc		転送データ
 *	@param	szByte		転送データサイズ
 *
 *	@retval	TRUE	転送データ設定成功
 */
//-----------------------------------------------------------------------------
static BOOL TRMAN_TrSysCallBackAddTransData( NNS_GFD_DST_TYPE type, u32 dstAddr, void* pSrc, u32 szByte )
{
	TRMAN_DATA* p_data;
	CLSYS_DRAW_TYPE tr_type;
	
	GF_ASSERT( pClsys );

	// 空いているバッファ取得
	p_data = TRMAN_TrSysGetCleanBuff( &pClsys->trman );

	if( type == NNS_GFD_DST_2D_OBJ_CHAR_MAIN ){
		tr_type = CLSYS_DRAW_MAIN;
	}else if( type == NNS_GFD_DST_2D_OBJ_CHAR_SUB ){
		tr_type = CLSYS_DRAW_SUB;
	}else{
		// ここに来ることはありえない
		GF_ASSERT(0);
	}

	GF_ASSERT( pSrc );
//	GF_ASSERT( szByte != 0 );	0のときもあったので変更
	if( szByte != 0 ){	// サイズが0のときは転送する必要もない
		TRMAN_TrDataAddData( p_data, tr_type, dstAddr, pSrc, szByte );
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送データバッファを初期化
 *
 *	@param	p_data	ワーク
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrDataClean( TRMAN_DATA* p_data )
{
	GF_ASSERT( p_data );
	GFL_STD_MemFill( p_data, 0, sizeof(TRMAN_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送データを設定
 *
 *	@param	p_data		転送データ格納先
 *	@param	type		転送タイプ
 *	@param	dstAddr		Vramアドレス
 *	@param	pSrc		転送キャラクタデータ
 *	@param	szByte		転送データサイズ
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrDataAddData( TRMAN_DATA* p_data, CLSYS_DRAW_TYPE type, u32 dstAddr, void* pSrc, u32 szByte )
{
	GF_ASSERT( p_data );
	p_data->type	= type;
	p_data->addr	= dstAddr;
	p_data->p_src	= pSrc;
	p_data->size	= szByte;
}

//----------------------------------------------------------------------------
/**
 *	@brief	データをVramに転送
 *
 *	@param	p_data	転送データ
 */
//-----------------------------------------------------------------------------
static void TRMAN_TrDataTrans( TRMAN_DATA* p_data )
{
	static void (* const load_func[ CLSYS_DRAW_MAX ])(const void*, u32, u32) = {
		GX_LoadOBJ,
		GXS_LoadOBJ,
	};
	GF_ASSERT( p_data );
	DC_FlushRange( p_data->p_src, p_data->size );
	load_func[ p_data->type ]( p_data->p_src, p_data->addr, p_data->size );
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送データバッファにデータが入っていないかチェック
 *
 *	@param	cp_data		チェックするバッファ
 *
 *	@retval	TRUE	空いている
 *	@retval	FALSE	空いていない
 */
//-----------------------------------------------------------------------------
static BOOL TRMAN_TrDataCheckClean( const TRMAN_DATA* cp_data )
{
	GF_ASSERT( cp_data );
	if( cp_data->p_src == NULL ){
		return TRUE;
	}
	return FALSE;
}


//-------------------------------------
///	GFL_CLUNIT	関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットから空のGFL_CLWKを取得する
 *
 *	@param	p_unit		セルアクターユニット
 *	
 *	@return	空のGFL_CLWKオブジェクト
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* CLUNIT_SysGetCleanWk( GFL_CLUNIT* p_unit )
{
	int i;

	GF_ASSERT( p_unit );

	for( i=0; i<p_unit->wk_num; i++ ){
		if( p_unit->p_wk[i].p_next == NULL ){
			return &p_unit->p_wk[i];
		}
	}
	GF_ASSERT( 0 );	// 登録数オーバー
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画リストに新規登録
 *
 *	@param	p_unit		セルアクターユニット
 *	@param	p_wk		登録するセルアクターワーク
 */
//-----------------------------------------------------------------------------
static void CLUNIT_DrawListAdd( GFL_CLUNIT* p_unit, GFL_CLWK* p_wk )
{
	u8 top_pri;		// 先頭の優先順位
	u8 bottom_pri;	// 最終の優先順位
	u8 ave_pri;		// 優先順位の平均
	u8 wk_pri;		// 登録する優先順位
	GFL_CLWK* p_last;	// 登録する前ワーク

	GF_ASSERT( p_unit );
	GF_ASSERT( p_wk );
	
	// 先頭チェック
	if( p_unit->p_drawlist == NULL ){
		p_unit->p_drawlist = p_wk;
		p_unit->p_drawlist->p_next = p_wk;
		p_unit->p_drawlist->p_last = p_wk;
		return ;
	}

	// 登録する優先順位を取得
	wk_pri = GFL_CLACT_WK_GetSoftPri( p_wk );

	// 先頭に登録するかチェック
	top_pri = GFL_CLACT_WK_GetSoftPri( p_unit->p_drawlist );
	if( wk_pri < top_pri ){
		// 先頭に登録
		p_last = p_unit->p_drawlist->p_last;
		p_unit->p_drawlist = p_wk;

	}else{

		// 先頭と最終要素の優先順位を取得
		bottom_pri = GFL_CLACT_WK_GetSoftPri( p_unit->p_drawlist->p_last );
		ave_pri = (top_pri + bottom_pri)/2;	// 平均を求める


		if( ave_pri >= wk_pri ){
			// 先頭から検索
			p_last = CLUNIT_DrawListSarchTop( p_unit->p_drawlist, wk_pri );
		}else{
			// 後方から検索
			p_last = CLUNIT_DrawListSarchBottom( p_unit->p_drawlist->p_last, wk_pri );
		}
	}

	// p_lastの次に登録
	p_wk->p_last			= p_last;
	p_wk->p_next			= p_last->p_next;
	p_last->p_next->p_last	= p_wk;
	p_last->p_next			= p_wk;
}


//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画リストからワークをはずす
 *
 *	@param	p_unit		セルアクターユニット
 *	@param	p_wk		はずすワーク
 */
//-----------------------------------------------------------------------------
static void CLUNIT_DrawListDel( GFL_CLUNIT* p_unit, GFL_CLWK* p_wk )
{
	GF_ASSERT( p_unit );
	GF_ASSERT( p_wk );

	// 自分が先頭かチェック
	if( p_unit->p_drawlist == p_wk ){
		// 自分の次も自分だったら終わり
		if( p_wk->p_next == p_wk ){
			p_unit->p_drawlist = NULL;
		}else{
			// 先頭を自分の次のやつに譲る
			p_unit->p_drawlist = p_wk->p_next;
		}
	}
	// はずす
	p_wk->p_next->p_last = p_wk->p_last;
	p_wk->p_last->p_next = p_wk->p_next;
	// 自分のリストポインタをクリア
	p_wk->p_next = NULL;
	p_wk->p_last = NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画リスト先頭から登録する前ワークを取得
 *
 *	@param	p_top		描画リスト先頭データ
 *	@param	pri			優先順位
 *
 *	@return	登録する前ワーク
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* CLUNIT_DrawListSarchTop( GFL_CLWK* p_top, u8 pri )
{
	GFL_CLWK* p_wk = p_top;
	u8 ck_pri;

	GF_ASSERT( p_top );
	
	do{
		ck_pri = GFL_CLACT_WK_GetSoftPri( p_wk );
		if( ck_pri > pri ){
			return p_wk->p_last;	// 優先順位が大きいワークの前のワークの次に追加する
		}
		// 次へ
		p_wk = p_wk->p_next;
	} while( p_wk != p_top );

	// 最後尾に登録してもらう
	return p_top->p_last;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画リスト後ろから登録する前ワークを取得
 *
 *	@param	p_top		描画リスト最後尾データ
 *	@param	pri			優先順位
 *
 *	@return	登録する前ワーク
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* CLUNIT_DrawListSarchBottom( GFL_CLWK* p_bottom, u8 pri )
{
	GFL_CLWK* p_wk = p_bottom;
	u8 ck_pri;

	GF_ASSERT( p_bottom );

	do{
		ck_pri = GFL_CLACT_WK_GetSoftPri( p_wk );
		if( ck_pri <= pri ){
			return p_wk;	// 優先順位が等しいか小さいワークの次に追加する
		}
		// 前へ
		p_wk = p_wk->p_last;
	} while( p_wk != p_bottom );

	// 先頭に登録してもらう
	return p_bottom->p_next;
}


//-------------------------------------
///	GFL_CLWK	関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターワークのクリア
 *
 *	@param	p_wk		セルアクターワーク
 */
//-----------------------------------------------------------------------------
static void CLWK_SysClean( GFL_CLWK* p_wk )
{
	GF_ASSERT( p_wk );
	GFL_STD_MemFill( p_wk, 0, sizeof(GFL_CLWK) );
	p_wk->auto_anm_speed = CLWK_AUTOANM_DEFF_SPEED;
	NNS_G2dInitImageProxy( &p_wk->img_proxy );
	NNS_G2dInitImagePaletteProxy( &p_wk->pltt_proxy );
	p_wk->over_write |= NNS_G2D_RND_OVERWRITE_PRIORITY;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターワーク描画処理
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	p_rend		レンダラーシステム
 */
//-----------------------------------------------------------------------------
static void CLWK_SysDraw( GFL_CLWK* p_wk, GFL_CLSYS_REND* p_rend )
{
	GF_ASSERT( p_wk );

	// 非表示
	if( p_wk->draw_flag == 0 ){
		return;
	}
	
	// レンダラー描画前設定
	REND_SysBeginDraw( p_rend, &p_wk->img_proxy, &p_wk->pltt_proxy );

	// レンダラーに描画パラメータを設定する
	CLWK_SysDrawSetRend( p_wk, p_rend );

	// パレットナンバー設定
	CLSYS_SysSetPaletteProxy( &p_wk->pltt_proxy, p_wk->pal_offs );
	
	// 描画処理
	NNS_G2dPushMtx();
	{
		CLWK_SysDrawCell( p_wk );
	}
	NNS_G2dPopMtx();

	// レンダラー描画終了
	REND_SysEndDraw( p_rend );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラーにセルアクターワーク
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	p_rend		レンダラーシステム
 */
//-----------------------------------------------------------------------------
static void CLWK_SysDrawSetRend( GFL_CLWK* p_wk, GFL_CLSYS_REND* p_rend )
{
	GF_ASSERT( p_wk );
	// アフィン設定
	REND_SysSetAffine( p_rend, p_wk->affine );
	if( p_wk->affine == NNS_G2D_RND_AFFINE_OVERWRITE_NONE ){
		BOOL vflip, hflip;
		vflip = GFL_CLACT_WK_GetFlip( p_wk, CLWK_FLIP_V );
		hflip = GFL_CLACT_WK_GetFlip( p_wk, CLWK_FLIP_H );
		REND_SysSetFlip( p_rend, vflip, hflip );
	}
	// オバーライトフラグ設定
	REND_SysSetOverwrite( p_rend, p_wk->over_write );
	// オバーライトパラメータ設定
	REND_SysSetMosaicFlag( p_rend, p_wk->mosaic );
	REND_SysSetOBJMode( p_rend, p_wk->objmode );
	REND_SysSetBGPriority( p_rend, p_wk->bg_pri );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルデータの描画
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void CLWK_SysDrawCell( GFL_CLWK* p_wk )
{
	GF_ASSERT( p_wk );

	// 座標移動
	NNS_G2dTranslate( FX32_CONST(p_wk->pos.x), FX32_CONST(p_wk->pos.y), 0 );
	
	if(p_wk->affine != NNS_G2D_RND_AFFINE_OVERWRITE_NONE){
		NNS_G2dTranslate( FX32_CONST(p_wk->affinepos.x), FX32_CONST(p_wk->affinepos.y), 0 );
		NNS_G2dScale( p_wk->scale.x, p_wk->scale.y, FX32_ONE );
		NNS_G2dRotZ( FX_SinIdx(p_wk->rotation), FX_CosIdx(p_wk->rotation) );
		NNS_G2dTranslate( -FX32_CONST(p_wk->affinepos.x), -FX32_CONST(p_wk->affinepos.y), 0 );
	}

	// それぞれの方法で描画
	switch( p_wk->anmdata.type ){
	case CLWK_ANM_CELL:
		NNS_G2dDrawCellAnimation( &p_wk->anmdata.data.cell.anmctrl );
		break;
	case CLWK_ANM_TRCELL:
		NNS_G2dDrawCellAnimation( &p_wk->anmdata.data.trcell.anmctrl );
		break;
	case CLWK_ANM_MULTICEL:
		NNS_G2dDrawMultiCellAnimation( &p_wk->anmdata.data.multicell.anmctrl );
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターオートアニメーションを行う
 *
 *	@param	p_wk	セルアクターワーク
 */
//-----------------------------------------------------------------------------
static void CLWK_SysAutoAnm( GFL_CLWK* p_wk )
{
	if( p_wk->auto_anm == TRUE ){
		GFL_CLACT_WK_AddAnmFrame( p_wk, p_wk->auto_anm_speed );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターワークに初期化データを設定する
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	cp_data		初期化データ
 *	@param	setsf		設定サーフェース
 */
//-----------------------------------------------------------------------------
static void CLWK_SysSetClwkData( GFL_CLWK* p_wk, const GFL_CLWK_DATA* cp_data, u16 setsf )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_data );
	
	GFL_CLACT_WK_SetTypePos( p_wk, cp_data->pos_x, setsf, CLSYS_MAT_X );
	GFL_CLACT_WK_SetTypePos( p_wk, cp_data->pos_y, setsf, CLSYS_MAT_Y );
	p_wk->soft_pri = cp_data->softpri;
	GFL_CLACT_WK_SetBgPri( p_wk, cp_data->bgpri );
	GFL_CLACT_WK_SetAnmSeq( p_wk, cp_data->anmseq );
	GFL_CLACT_WK_StartAnm( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	リソースデータをワークに登録
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	cp_res		リソースデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_SysSetClwkRes( GFL_CLWK* p_wk, const GFL_CLWK_RES* cp_res )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_res );
	GFL_CLACT_WK_SetImgProxy( p_wk, cp_res->cp_img );
	GFL_CLACT_WK_SetPlttProxy( p_wk, cp_res->cp_pltt );
}

//----------------------------------------------------------------------------
/**
 *	@brief	設定サーフェースの座標を取得する
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	setsf		設定サーフェース
 *	@param	p_pos		設定サーフェース座標格納先
 */
//-----------------------------------------------------------------------------
static void CLWK_SysGetSetSfPos( const GFL_CLWK* cp_wk, u16 setsf, GFL_CLACTPOS* p_pos )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( p_pos );
	// サーフェース座標を取得
	if( setsf != CLWK_SETSF_NONE ){
		GFL_CLACT_USERREND_GetSurfacePos( cp_wk->p_unit->p_rend, setsf, p_pos );
	}else{
		p_pos->x = 0;
		p_pos->y = 0;
	}
}


//-------------------------------------
///	CLWK_ANMDATA	関係
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief		アニメーションデータ初期化
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	cp_res			リソースワーク
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataInit( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID )
{
	static void (* const p_create[ CLWK_ANM_MAX ])( CLWK_ANMDATA* , const GFL_CLWK_RES* ,  HEAPID ) = {
		CLWK_AnmDataCreateCellData,
		CLWK_AnmDataCreateTRCellData,
		CLWK_AnmDataCreateMCellData
	};

	GF_ASSERT( p_anmdata );
	
	// アニメーションタイプを取得
	p_anmdata->type = CLWK_AnmDataGetType( cp_res );

	// それぞれの初期化へ
	p_create[ p_anmdata->type ]( p_anmdata, cp_res, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータの破棄処理
 *
 *	@param	p_anmdata	アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataExit( CLWK_ANMDATA* p_anmdata )
{
	static void (* const p_delete[ CLWK_ANM_MAX ])( CLWK_ANMDATA*  ) = {
		CLWK_AnmDataDeleteCellData,
		CLWK_AnmDataDeleteTRCellData,
		CLWK_AnmDataDeleteMCellData,
	};

	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );
	
	p_delete[ p_anmdata->type ]( p_anmdata );
	GFL_STD_MemFill( p_anmdata, 0, sizeof(CLWK_ANMDATA) );
}


//----------------------------------------------------------------------------
/**
 *	@brief	リソースデータからアニメーションタイプを取得する
 *
 *	@param	cp_res	リソースデータ
 *
 *	@return	アニメーションタイプ
 */
//-----------------------------------------------------------------------------
static CLWK_ANMTYPE CLWK_AnmDataGetType( const GFL_CLWK_RES* cp_res )
{
	GF_ASSERT( cp_res );

	// Vram転送セルアニメ
	if( cp_res->cp_char != NULL ){
		return CLWK_ANM_TRCELL;
	}
	// マルチセル
	if( cp_res->cp_mcell != NULL ){
		return CLWK_ANM_MULTICEL;
	}
	return CLWK_ANM_CELL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーションデータ　初期化
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	cp_res			リソースデータ
 *	@param	heapID
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataCreateCellData( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID )
{
	GF_ASSERT( p_anmdata );
	GF_ASSERT( cp_res );

	p_anmdata->data.cell.cp_cell	= cp_res->p_cell;
	p_anmdata->data.cell.cp_canm	= cp_res->cp_canm;

	// アニメーションコントローラ初期化
	NNS_G2dInitCellAnimation( 
			&p_anmdata->data.cell.anmctrl,
			NNS_G2dGetAnimSequenceByIdx(p_anmdata->data.cell.cp_canm, 0),
			p_anmdata->data.cell.cp_cell
			);
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション　初期化
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	cp_res			リソースデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataCreateTRCellData( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID )
{
	GF_ASSERT( pClsys );
	GF_ASSERT( p_anmdata );
	GF_ASSERT( cp_res );

	p_anmdata->data.trcell.p_cell	= cp_res->p_cell;
	p_anmdata->data.trcell.cp_canm	= cp_res->cp_canm;
	
	// Vram転送セル管理ハンドル
	p_anmdata->data.trcell.trhandle = TRMAN_CellTransManAddHandle( &pClsys->trman ); 

	// セルVram転送アニメの初期化
	NNS_G2dInitCellAnimationVramTransfered(
                &p_anmdata->data.trcell.anmctrl,
                NNS_G2dGetAnimSequenceByIdx(p_anmdata->data.trcell.cp_canm, 0),
                p_anmdata->data.trcell.p_cell,

                p_anmdata->data.trcell.trhandle,
                NNS_G2D_VRAM_ADDR_NONE ,
                NNS_G2dGetImageLocation(cp_res->cp_img, NNS_G2D_VRAM_TYPE_2DMAIN),
                NNS_G2dGetImageLocation(cp_res->cp_img, NNS_G2D_VRAM_TYPE_2DSUB),
                cp_res->cp_char->pRawData, 
                NULL,
                cp_res->cp_char->szByte 
            );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーションを初期化
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	cp_res			リソースデータ
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataCreateMCellData( CLWK_ANMDATA* p_anmdata, const GFL_CLWK_RES* cp_res, HEAPID heapID )
{
	u32 wk_size;

	GF_ASSERT( p_anmdata );
	GF_ASSERT( cp_res );
	
	p_anmdata->data.multicell.cp_cell	= cp_res->p_cell;
	p_anmdata->data.multicell.cp_canm	= cp_res->cp_canm;
	p_anmdata->data.multicell.cp_mcell	= cp_res->cp_mcell;
	p_anmdata->data.multicell.cp_mcanm	= cp_res->cp_mcanm;
	
	// ワーク作成
	wk_size =  NNS_G2dGetMCWorkAreaSize( p_anmdata->data.multicell.cp_mcell, NNS_G2D_MCTYPE_SHARE_CELLANIM );
	p_anmdata->data.multicell.p_wk = GFL_HEAP_AllocMemory( heapID, wk_size );

	// マルチセルアニメーション初期化
	NNS_G2dInitMCAnimationInstance( 
		&p_anmdata->data.multicell.anmctrl,  
		p_anmdata->data.multicell.p_wk, 
		p_anmdata->data.multicell.cp_canm,  
		p_anmdata->data.multicell.cp_cell,
		p_anmdata->data.multicell.cp_mcell,
		NNS_G2D_MCTYPE_SHARE_CELLANIM
	);

	// アニメーションシーケンス設定
	NNS_G2dSetAnimSequenceToMCAnimation( &p_anmdata->data.multicell.anmctrl, 
			NNS_G2dGetAnimSequenceByIdx( p_anmdata->data.multicell.cp_mcanm, 0 ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーション　破棄処理
 *
 *	@param	p_anmdata		アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataDeleteCellData( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( p_anmdata );
	// 何もしなくて良い
	GFL_STD_MemFill( p_anmdata, 0, sizeof(CLWK_ANMDATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション　破棄処理
 *
 *	@param	p_anmdata		アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataDeleteTRCellData( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( pClsys );
	GF_ASSERT( p_anmdata );

	// Vram転送管理ハンドル解放
	TRMAN_CellTransManDelHandle( &pClsys->trman, p_anmdata->data.trcell.trhandle );
	GFL_STD_MemFill( p_anmdata, 0, sizeof(CLWK_ANMDATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション　破棄処理
 *
 *	@param	p_anmdata		アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataDeleteMCellData( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( p_anmdata );

	// ワーク破棄
	GFL_HEAP_FreeMemory( p_anmdata->data.multicell.p_wk );
	GFL_STD_MemFill( p_anmdata, 0, sizeof(CLWK_ANMDATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータのアニメーションシーケンスを変更
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	anmseq			アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeq( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	static void (* const p_SetAnmSeq[ CLWK_ANM_MAX ])( CLWK_ANMDATA*, u16 ) = {
		CLWK_AnmDataSetAnmSeqCell,
		CLWK_AnmDataSetAnmSeqTRCell,
		CLWK_AnmDataSetAnmSeqMCell,
	};
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );
	p_SetAnmSeq[ p_anmdata->type ]( p_anmdata, anmseq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーション　アニメーションシーケンス変更
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	anmseq			アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	const NNSG2dAnimSequence* p_seq;

	GF_ASSERT( p_anmdata );
	
	// シーケンスを取得
	p_seq = NNS_G2dGetAnimSequenceByIdx( p_anmdata->data.cell.cp_canm, anmseq );

	// シーケンスを適用
	NNS_G2dSetCellAnimationSequence( &p_anmdata->data.cell.anmctrl, p_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション　アニメーションシーケンス変更
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqTRCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	const NNSG2dAnimSequence* p_seq;

	GF_ASSERT( p_anmdata );
	
	// シーケンスを取得
	p_seq = NNS_G2dGetAnimSequenceByIdx( p_anmdata->data.trcell.cp_canm, anmseq );

	// シーケンスを適用
	NNS_G2dSetCellAnimationSequence( &p_anmdata->data.trcell.anmctrl, p_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション　アニメーションシーケンス変更
 *	
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqMCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	const NNSG2dMultiCellAnimSequence* p_seq;

	GF_ASSERT( p_anmdata );

	p_seq = NNS_G2dGetAnimSequenceByIdx( p_anmdata->data.multicell.cp_mcanm, anmseq );
	NNS_G2dSetAnimSequenceToMCAnimation( 
			&p_anmdata->data.multicell.anmctrl, 
			p_seq );
}

#if 0		// 使用用途がわからないので排除
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更する　
 *			その際フレーム時間などのリセットを行いません。　MultiCell非対応
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	anmseq			アニメーションシーケンス
 */	
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqNoReset( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	static void (* const p_SetAnmSeq[ CLWK_ANM_MAX ])( CLWK_ANMDATA*, u16 ) = {
		CLWK_AnmDataSetAnmSeqNoResetCell,
		CLWK_AnmDataSetAnmSeqNoResetTRCell,
		CLWK_AnmDataSetAnmSeqNoResetMCell,
	};
	GF_ASSERT( p_anmdata );
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );

	p_SetAnmSeq[ p_anmdata->type ]( p_anmdata, anmseq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーション　リセットを行わないシーケンス変更処理
 *
 *	@param	p_anmdata		アニメーションデータ
 *	@param	anmseq			アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqNoResetCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	const NNSG2dAnimSequence* cp_seq;

	GF_ASSERT( p_anmdata );
	
	// シーケンスを取得
	cp_seq = NNS_G2dGetAnimSequenceByIdx( p_anmdata->data.cell.cp_canm, anmseq );

	// シーケンスを適用
	NNS_G2dSetCellAnimationSequenceNoReset( &p_anmdata->data.cell.anmctrl, cp_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション　リセットを行わないシーケンス変更処理
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqNoResetTRCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	const NNSG2dAnimSequence* cp_seq;

	GF_ASSERT( p_anmdata );
	
	// シーケンスを取得
	cp_seq = NNS_G2dGetAnimSequenceByIdx( p_anmdata->data.trcell.cp_canm, anmseq );

	// シーケンスを適用
	NNS_G2dSetCellAnimationSequenceNoReset( &p_anmdata->data.trcell.anmctrl, cp_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション	非対応
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSeqNoResetMCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	GF_ASSERT( p_anmdata );
	// マルチセルアニメーションには非対応です
	GF_ASSERT( 0 );	
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータのアニメーションフレーム数を設定
 *	
 *	@param	p_anmdata			アニメーションデータ
 *	@param	idx					アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmFrame( CLWK_ANMDATA* p_anmdata, u16 idx )
{
	static void (* const p_SetFrame[ CLWK_ANM_MAX ])( CLWK_ANMDATA*, u16 ) = {
		CLWK_AnmDataSetAnmFrameCell,
		CLWK_AnmDataSetAnmFrameTRCell,
		CLWK_AnmDataSetAnmFrameMCell,
	};
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );

	p_SetFrame[ p_anmdata->type ]( p_anmdata, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーション　アニメーションフレーム数設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmFrameCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dSetCellAnimationCurrentFrame( &p_anmdata->data.cell.anmctrl, anmseq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション　アニメーションフレーム数設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmFrameTRCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dSetCellAnimationCurrentFrame( &p_anmdata->data.trcell.anmctrl, anmseq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション　アニメーションフレーム数設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	anmseq				アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmFrameMCell( CLWK_ANMDATA* p_anmdata, u16 anmseq )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dSetMCAnimationCurrentFrame( &p_anmdata->data.multicell.anmctrl, anmseq );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ　アニメーションフレームを進める
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				アニメーションスピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataAddAnmFrame( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	static void (* const p_add[ CLWK_ANM_MAX ])( CLWK_ANMDATA*, fx32 ) = {
		CLWK_AnmDataAddAnmFrameCell,
		CLWK_AnmDataAddAnmFrameTRCell,
		CLWK_AnmDataAddAnmFrameMCell,
	};
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );

	p_add[ p_anmdata->type ]( p_anmdata, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーション　アニメーションを進める
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				アニメーションスピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataAddAnmFrameCell( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dTickCellAnimation( &p_anmdata->data.cell.anmctrl, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーション　アニメーションを進める
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				アニメーションスピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataAddAnmFrameTRCell( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dTickCellAnimation( &p_anmdata->data.trcell.anmctrl, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション　アニメーションを進める
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				アニメーションスピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataAddAnmFrameMCell( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dTickMCAnimation( &p_anmdata->data.multicell.anmctrl, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータからアニメーションコントローラを取得する
 *
 *	@param	p_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrl( CLWK_ANMDATA* p_anmdata )
{
	static NNSG2dAnimController* (* const p_get[ CLWK_ANM_MAX ])( CLWK_ANMDATA* ) = {
		CLWK_AnmDataGetAnmCtrlCell,
		CLWK_AnmDataGetAnmCtrlTRCell,
		CLWK_AnmDataGetAnmCtrlMCell,
	};
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );
		
	return p_get[ p_anmdata->type ]( p_anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーションからアニメーションコントローラを取得
 *
 *	@param	p_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrlCell( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( p_anmdata );
	return NNS_G2dGetCellAnimationAnimCtrl( &p_anmdata->data.cell.anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーションからアニメーションコントローラを取得
 *
 *	@param	p_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrlTRCell( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( p_anmdata );
	return NNS_G2dGetCellAnimationAnimCtrl( &p_anmdata->data.trcell.anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーションからアニメーションコントローラを取得
 *
 *	@param	p_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static NNSG2dAnimController* CLWK_AnmDataGetAnmCtrlMCell( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( p_anmdata );
	return NNS_G2dGetMCAnimAnimCtrl( &p_anmdata->data.multicell.anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータからアニメーションコントローラを取得する
 *
 *	@param	p_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrl( const CLWK_ANMDATA* cp_anmdata )
{
	static const NNSG2dAnimController* (* const p_get[ CLWK_ANM_MAX ])( const CLWK_ANMDATA* ) = {
		CLWK_AnmDataGetCAnmCtrlCell,
		CLWK_AnmDataGetCAnmCtrlTRCell,
		CLWK_AnmDataGetCAnmCtrlMCell,
	};
	GF_ASSERT( cp_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > cp_anmdata->type );

	return p_get[ cp_anmdata->type ]( cp_anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーションからアニメーションコントローラを取得
 *
 *	@param	cp_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrlCell( const CLWK_ANMDATA* cp_anmdata )
{
	GF_ASSERT( cp_anmdata );
	return &cp_anmdata->data.cell.anmctrl.animCtrl;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーションからアニメーションコントローラを取得
 *
 *	@param	cp_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrlTRCell( const CLWK_ANMDATA* cp_anmdata )
{
	GF_ASSERT( cp_anmdata );
	return &cp_anmdata->data.trcell.anmctrl.animCtrl;
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーションからアニメーションコントローラを取得
 *
 *	@param	p_anmdata			アニメーションデータ
 *
 *	@return	アニメーションコントローラ
 */
//-----------------------------------------------------------------------------
static const NNSG2dAnimController* CLWK_AnmDataGetCAnmCtrlMCell( const CLWK_ANMDATA* cp_anmdata )
{
	GF_ASSERT( cp_anmdata );
	return &cp_anmdata->data.multicell.anmctrl.animCtrl;
}
		

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータのアニメーションフレーム数を取得
 *
 *	@param	cp_anmdata			アニメーションデータ
 *
 *	@return	アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
static u16 CLWK_AnmDataGetAnmFrame( const CLWK_ANMDATA* cp_anmdata )
{
	const NNSG2dAnimController* cp_anmctrl = CLWK_AnmDataGetCAnmCtrl( cp_anmdata );
	GF_ASSERT( cp_anmdata );
	return NNS_G2dGetAnimCtrlCurrentFrame( cp_anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータにオートアニメーションスピードを設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				スピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSpeed( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	static void (* p_set[ CLWK_ANM_MAX ])( CLWK_ANMDATA* , fx32 )={
		CLWK_AnmDataSetAnmSpeedCell,
		CLWK_AnmDataSetAnmSpeedTRCell,
		CLWK_AnmDataSetAnmSpeedMCell,
	};
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );

	p_set[ p_anmdata->type ]( p_anmdata, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーションデータにオートアニメーションスピード設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				スピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSpeedCell( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dSetCellAnimationSpeed( &p_anmdata->data.cell.anmctrl, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vram転送セルアニメーションデータにオートアニメーションスピード設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				スピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSpeedTRCell( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dSetCellAnimationSpeed( &p_anmdata->data.trcell.anmctrl, speed );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーションデータにオートアニメーションスピード設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	speed				スピード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmSpeedMCell( CLWK_ANMDATA* p_anmdata, fx32 speed )
{
	GF_ASSERT( p_anmdata );
	NNS_G2dSetMCAnimationSpeed( &p_anmdata->data.multicell.anmctrl, speed );
}


//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションが動いているかチェックする
 *
 *	@param	cp_anmdata			アニメーションデータ
 *
 *	@retval	TRUE	再生中
 *	@retval	FALSE	停止中
 */
//-----------------------------------------------------------------------------
static BOOL CLWK_AnmDataCheckAnmActive( const CLWK_ANMDATA* cp_anmdata )
{
	const NNSG2dAnimController* cp_anmctrl = CLWK_AnmDataGetCAnmCtrl( cp_anmdata );
	GF_ASSERT( cp_anmdata );
	return NNS_G2dIsAnimCtrlActive( cp_anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ	アニメーションリセット
 *
 *	@param	p_anmdata			アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataResetAnm( CLWK_ANMDATA* p_anmdata )
{
	static void (* p_reset[ CLWK_ANM_MAX ])( CLWK_ANMDATA* )={
		CLWK_AnmDataResetAnmCell,
		CLWK_AnmDataResetAnmCell,
		CLWK_AnmDataResetAnmMCell,
	};
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > p_anmdata->type );

	p_reset[p_anmdata->type]( p_anmdata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメーション		アニメーションリセット
 *
 *	@param	p_anmdata			アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataResetAnmCell( CLWK_ANMDATA* p_anmdata )
{
	NNSG2dAnimController* p_anmctrl = CLWK_AnmDataGetAnmCtrl( p_anmdata );
	GF_ASSERT( p_anmdata );
	NNS_G2dResetAnimCtrlState( p_anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マルチセルアニメーション	アニメーションリセット
 *
 *	@param	p_anmdata			アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataResetAnmMCell( CLWK_ANMDATA* p_anmdata )
{
	GF_ASSERT( p_anmdata );
	CLWK_AnmDataResetAnmCell( p_anmdata );
	NNS_G2dResetMCCellAnimationAll( &p_anmdata->data.multicell.anmctrl.multiCellInstance );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータにアニメーションプレイモードを設定
 *
 *	@param	p_anmdata			アニメーションデータ
 *	@param	playmode			アニメーションプレイモード
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataSetAnmMode( CLWK_ANMDATA* p_anmdata, CLSYS_ANM_PLAYMODE playmode )
{
	NNSG2dAnimController* p_anmctrl = CLWK_AnmDataGetAnmCtrl( p_anmdata );
	
	GF_ASSERT( p_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLSYS_ANMPM_MAX > playmode );
	
	if( playmode == CLSYS_ANMPM_INVALID ){
		// 無効データの時下に戻す
		p_anmctrl->bReverse = FALSE;		// 下の関数の中でリバースフラグを初期化しないので、ここで初期化する
		NNS_G2dResetAnimCtrlPlayModeOverridden( p_anmctrl );
	}else{	
		NNS_G2dSetAnimCtrlPlayModeOverridden( p_anmctrl, playmode );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ	アニメーションの再生
 *	
 *	@param	p_anmdata		アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataStartAnm( CLWK_ANMDATA* p_anmdata )
{
	NNSG2dAnimController* p_anmctrl = CLWK_AnmDataGetAnmCtrl( p_anmdata );
	NNS_G2dStartAnimCtrl( p_anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ	アニメーションの停止
 *
 *	@param	p_anmdata		アニメーションデータ
 */
//-----------------------------------------------------------------------------
static void CLWK_AnmDataStopAnm( CLWK_ANMDATA* p_anmdata )
{
	NNSG2dAnimController* p_anmctrl = CLWK_AnmDataGetAnmCtrl( p_anmdata );
	NNS_G2dStopAnimCtrl( p_anmctrl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ　アニメーションシーケンスの拡張アトリビュートデータ取得
 *
 *	@param	cp_anmdata		アニメーションデータ
 *	@param	seq				シーケンス
 *
 *	@return	アニメーションシーケンスの拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttr( const CLWK_ANMDATA* cp_anmdata, u32 seq )
{
	static const NNSG2dUserExAnimSequenceAttr* (* const p_getattr[ CLWK_ANM_MAX ])( const CLWK_ANMDATA* cp_anmdata, u32 seq ) = {
		CLWK_AnmDataGetUserAnimSeqAttrCell,
		CLWK_AnmDataGetUserAnimSeqAttrTRCell,
		CLWK_AnmDataGetUserAnimSeqAttrMCell,
	};
	GF_ASSERT( cp_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > cp_anmdata->type );
	return p_getattr[cp_anmdata->type]( cp_anmdata, seq );
}
// セル
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttrCell( const CLWK_ANMDATA* cp_anmdata, u32 seq )
{
	const NNSG2dUserExAnimAttrBank* p_attr_anmbank;
	p_attr_anmbank = NNS_G2dGetUserExAnimAttrBank( cp_anmdata->data.cell.cp_canm );
	if( p_attr_anmbank ){
		return NNS_G2dGetUserExAnimSequenceAttr( p_attr_anmbank, seq );
	}
	return NULL;
}
// 転送セル
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttrTRCell( const CLWK_ANMDATA* cp_anmdata, u32 seq )
{
	const NNSG2dUserExAnimAttrBank* p_attr_anmbank;
	p_attr_anmbank = NNS_G2dGetUserExAnimAttrBank( cp_anmdata->data.trcell.cp_canm );
	if( p_attr_anmbank ){
		return NNS_G2dGetUserExAnimSequenceAttr( p_attr_anmbank, seq );
	}
	return NULL;
}
// マルチセル
static const NNSG2dUserExAnimSequenceAttr* CLWK_AnmDataGetUserAnimSeqAttrMCell( const CLWK_ANMDATA* cp_anmdata, u32 seq )
{
	const NNSG2dUserExAnimAttrBank* p_attr_anmbank;
	p_attr_anmbank = NNS_G2dGetUserExAnimAttrBank( cp_anmdata->data.multicell.cp_mcanm );
	if( p_attr_anmbank ){
		return NNS_G2dGetUserExAnimSequenceAttr( p_attr_anmbank, seq );
	}
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ　セルデータの拡張アトリビュートデータ取得
 *
 *	@param	cp_anmdata		アニメーションデータ
 *	@param	idx				セルデータインデックス
 *
 *	@return	セルデータの拡張アトリビュートデータ
 */
//-----------------------------------------------------------------------------
static u32 CLWK_AnmDataGetUserCellAttr( const CLWK_ANMDATA* cp_anmdata, u32 idx )
{
	static const NNSG2dUserExCellAttrBank* (* const p_getattr[ CLWK_ANM_MAX ])( const CLWK_ANMDATA* cp_anmdata ) = {
		CLWK_AnmDataGetUserCellAttrCell,
		CLWK_AnmDataGetUserCellAttrTRCell,
		CLWK_AnmDataGetUserCellAttrMCell,
	};
	const NNSG2dUserExCellAttrBank* p_cellattrbank;
	const NNSG2dUserExCellAttr* p_cellattr;
	GF_ASSERT( cp_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > cp_anmdata->type );
	p_cellattrbank = p_getattr[cp_anmdata->type]( cp_anmdata );
	if( p_cellattrbank ){
		p_cellattr = NNS_G2dGetUserExCellAttr( p_cellattrbank, idx ); 
		if( p_cellattr ){
			return NNS_G2dGetUserExCellAttrValue( p_cellattr );
		}
	}
	return CLWK_USERATTR_NONE;
}
// セル
static const NNSG2dUserExCellAttrBank* CLWK_AnmDataGetUserCellAttrCell( const CLWK_ANMDATA* cp_anmdata )
{
	return NNS_G2dGetUserExCellAttrBankFromCellBank( cp_anmdata->data.cell.cp_cell );
}
// 転送セル
static const NNSG2dUserExCellAttrBank* CLWK_AnmDataGetUserCellAttrTRCell( const CLWK_ANMDATA* cp_anmdata )
{
	return NNS_G2dGetUserExCellAttrBankFromCellBank( cp_anmdata->data.trcell.p_cell );
}
// マルチセル
static const NNSG2dUserExCellAttrBank* CLWK_AnmDataGetUserCellAttrMCell( const CLWK_ANMDATA* cp_anmdata )
{
	return NNS_G2dGetUserExCellAttrBankFromMCBank( cp_anmdata->data.multicell.cp_mcell );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションデータ　アニメーションシーケンスの数を取得
 *
 *	@param	cp_anmdata	アニメーションデータ
 *
 *	@return	アニメーションシーケンス数
 */
//-----------------------------------------------------------------------------
static u32 CLWK_AnmDataGetAnmSeqNum( const CLWK_ANMDATA* cp_anmdata )
{
	static u32 (* const p_func[ CLWK_ANM_MAX ])( const CLWK_ANMDATA* cp_anmdata ) ={
		CLWK_AnmDataGetAnmSeqNumCell,
		CLWK_AnmDataGetAnmSeqNumTRCell,
		CLWK_AnmDataGetAnmSeqNumMCell,
	};
	GF_ASSERT( cp_anmdata );
	// ありえないアニメタイプ
	GF_ASSERT( CLWK_ANM_MAX > cp_anmdata->type );
	return p_func[cp_anmdata->type]( cp_anmdata );
}
// セル
static u32 CLWK_AnmDataGetAnmSeqNumCell( const CLWK_ANMDATA* cp_anmdata )
{
	return cp_anmdata->data.cell.cp_canm->numSequences;
}
// 転送セル
static u32 CLWK_AnmDataGetAnmSeqNumTRCell( const CLWK_ANMDATA* cp_anmdata )
{
	return cp_anmdata->data.trcell.cp_canm->numSequences;
}
// マルチセル
static u32 CLWK_AnmDataGetAnmSeqNumMCell( const CLWK_ANMDATA* cp_anmdata )
{
	return cp_anmdata->data.multicell.cp_mcanm->numSequences;
}

