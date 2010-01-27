//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		clact.h
 *	@brief		セルアクターシステム
 *	@author		tomoya takahashi
 *	@data		2006.11.28
 *	@data		2008.12.24		5点変更
 *								　1.大本のシステムの関数にSysを入れるように変更
 *								　2.Init ExitをCreate Deleteに変更
 *								　3.DrawFlagをDrawEnableに変更
 *								　4.CLUNITの描画関数を廃止し、GFL_CLACT_MainSys関数ないで、いっせいに描画するように変更
 *								　5.VBlankでの転送関数を１つにまとる。
 *								　**4.5.は、VBlank期間での処理をまとめるための変更*
 *
 *	@data		2009.01.07		obj_graphic_man.hの内容を統合
 *	@data		2009.08.31		・GFL_CLACT_WK_SetAnmFrameの意味を名前通り、
 *	                      　アニメーションフレーム数を設定する関数に変更
 *	                        ＊SetAnmFrameはマルチセル使用不可能です。
 *	                      ・アニメーションのインデックスを設定する関数
 *	                        GFL_CLACT_WK_SetAnmIndexを追加
 *	@data   2009.09.08    Element(アニメーション結果情報)を取得するインターフェースを追加
 *	@data   2009.09.30    キャラクタ、パレット再転送関数を作成
 *	                      
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __CLACT_H__
#define __CLACT_H__

#include "arc_tool.h"
#include "display.h"

#ifdef __cplusplus
extern "C" {
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

	CLSYS_DRAW_MAX,	// 両画面
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
//	GFL_CLWKへのフリップ設定に使用します。
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
///	デフォルト設定レンダラー内サーフェース指定定数
//=====================================
typedef enum{
	CLSYS_DEFREND_MAIN,
	CLSYS_DEFREND_SUB,
	CLSYS_DEFREND_NUM
} CLSYS_DEFREND_TYPE;

//-------------------------------------
///	パレットオフセット設定モード
//=====================================
typedef enum{
	CLWK_PLTTOFFS_MODE_OAM_COLOR,		// OamAttr.c_paramからのオフセット設定
	CLWK_PLTTOFFS_MODE_PLTT_TOP,		// パレット転送先の先頭からのオフセット設定

	CLWK_PLTTOFFS_MODE_NUM
} CLWK_PLTTOFFS_MODE;

//-------------------------------------
///	アニメーションコールバックタイプ
//=====================================
typedef enum{
  CLWK_ANM_CALLBACK_TYPE_LAST_FRM,    // アニメーションシーケンスの最終フレーム終了時によびだす
  CLWK_ANM_CALLBACK_TYPE_SPEC_FRM,    // 指定フレームの再生時に呼びだす
  CLWK_ANM_CALLBACK_TYPE_EVER_FRM,    // 毎フレーム呼び出す

  CLWK_ANM_CALLBACK_TYPE_MAX,    // システム内で使用
} CLWK_ANM_CALLBACK_TYPE;

//-------------------------------------
///	アニメーションコールバック関数
//=====================================
typedef void (*CLWK_ANM_CALLBACK_FUNC)( u32 param, fx32 currentFrame );


//-------------------------------------
///	設定サーフェース　絶対座標指定
//	GFL_CLWKの座標設定関数で絶対座標を設定するときに使用します。
//=====================================
#define CLWK_SETSF_NONE	(0xffff)

//-------------------------------------
///	Vramアドレス取得失敗
//	GFL_CLACT_WK_GetPlttAddr
//	GFL_CLACT_WK_GetCharAddr
//	の戻り値
//=====================================
#define CLWK_VRAM_ADDR_NONE	( 0xffffffff )


//-------------------------------------
///	ユーザー拡張アトリビュート取得
//	データがないときの戻り値
//	GFL_CLACT_WK_GetPlttAddr
//	GFL_CLACT_WK_GetCharAddr
//	の戻り値
//=====================================
#define CLWK_USERATTR_NONE	(0)	// ユーザー拡張アトリビュートなし


//-------------------------------------
///	グラフィック管理定数
//=====================================
#define GFL_CLGRP_REGISTER_FAILED	(0xffffffff)	// 登録失敗
#define GFL_CLGRP_EXPLTT_OFFSET		(0x200)			// 拡張パレット指定


//-------------------------------------
///	OAM管理数　ルール
//
//	OAMバッファを一括転送するために、以下２つのルールを守る必要があります。
//	１．管理開始位置を4の倍数にする
//	２．管理数を4の倍数にする	
//=====================================
#define GFL_CLSYS_OAMMAN_INTERVAL	( 4 )


//-----------------------------------------------------------------------------
/**
 *					外部非公開システム構造体
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	ユーザー定義レンダラー
//=====================================
typedef struct _CLSYS_REND	GFL_CLSYS_REND;

//-------------------------------------
///	セルアクターユニット
//=====================================
typedef struct _CLUNIT		GFL_CLUNIT;

//-------------------------------------
///	セルアクターワーク
//=====================================
typedef struct _CLWK		GFL_CLWK;





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
	u8	oamst_main;			// メイン画面OAM管理開始位置 4の倍数
	u8	oamnum_main;		// メイン画面OAM管理数		 4の倍数
	u8	oamst_sub;			// サブ画面OAM管理開始位置	 4の倍数
	u8	oamnum_sub;			// サブ画面OAM管理数		 4の倍数
	u32	tr_cell;			// セルVram転送管理数

	u16  CGR_RegisterMax;			///< 登録できるキャラデータ数
	u16  PLTT_RegisterMax;			///< 登録できるパレットデータ数
	u16  CELL_RegisterMax;			///< 登録できるセルアニメパターン数
	u16  MULTICELL_RegisterMax;		///< 登録できるマルチセルアニメパターン数（※現状未対応）

  u16 CGRVMan_AreaOffsMain; ///< メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
  u16 CGRVMan_AreaOffsSub;  ///< サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
} GFL_CLSYS_INIT;


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
} GFL_REND_SURFACE_INIT;


//-------------------------------------
///	GFL_CLWK初期化基本データ
//=====================================
typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
} GFL_CLWK_DATA;

//-------------------------------------
///	GFL_CLWK初期化アフィン変換データ
//=====================================
typedef struct {
	GFL_CLWK_DATA clwkdata;		// 基本データ
	
	s16	affinepos_x;		// アフィンｘ座標
	s16 affinepos_y;		// アフィンｙ座標
	fx32 scale_x;			// 拡大ｘ値
	fx32 scale_y;			// 拡大ｙ値
	u16	rotation;			// 回転角度(0～0xffff 0xffffが360度)
	u16	affine_type;		// 上書きアフィンタイプ（CLSYS_AFFINETYPE）
} GFL_CLWK_AFFINEDATA;


//-------------------------------------
///	GFL_CLWK　座標構造体	
//=====================================
typedef struct {
	s16 x;
	s16 y;
} GFL_CLACTPOS;

//-------------------------------------
///	GFL_CLWK　スケール構造体
//=====================================
typedef struct {
	fx32 x;
	fx32 y;
} GFL_CLSCALE;


//-------------------------------------
///	アニメーション要素構造体
//=====================================
typedef struct {
  u16   cell_index;       // セルインデックス
  u16   rotZ;             // 回転Z
  fx32  scale_x;          // スケールX
  fx32  scale_y;          // スケールY
  s16   trans_x;          // 移動X
  s16   trans_y;          // 移動Y
} GFL_CLWK_ANMELEMENT;


//-------------------------------------
///	アニメーション要素構造体
//=====================================
typedef struct {
  u16 callback_type;  // CLWK_ANM_CALLBACK_TYPE
  u16 frame_idx;      // CLWK_ANM_CALLBACK_TYPE_SPEC_FRM用フレームインデックス
  u32 param;          // コールバックワーク
  CLWK_ANM_CALLBACK_FUNC  p_func; // コールバック関数
} GFL_CLWK_ANM_CALLBACK;
   


//-----------------------------------------------------------------------------
/**
 *				基本的なCLSYS_INITデータ
 *				とりあえず出してみたいときなどに使用してください
 */
//-----------------------------------------------------------------------------
/*
 *	上下画面をつなげない場合
 *	const GFL_CLSYS_INIT GFL_CLSYSINIT_DEF_DIVSCREEN = {
 *		0, 0,
 *		0, 512,
 *		4, 124,
 *		4, 124,
 *		0,
 *		32,32,32,32,
 *    16, 16,
 *	};
 */
extern const GFL_CLSYS_INIT GFL_CLSYSINIT_DEF_DIVSCREEN;
/*
 *	上下画面をつなげる場合
 *	const GFL_CLSYS_INIT GFL_CLSYSINIT_DEF_CONSCREEN = {
 *		0, 0,
 *		0, 192,
 *		4, 124,
 *		4, 124,
 *		0,
 *		32,32,32,32,
 *    16, 16,
 *	};
 */
extern const GFL_CLSYS_INIT GFL_CLSYSINIT_DEF_CONSCREEN;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
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
 *	@param	cp_vrambank	VRAMバンク設定
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_SYS_Create( const GFL_CLSYS_INIT* cp_data, const GFL_DISP_VRAM* cp_vramBank, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム	破棄
 *
 *	@param	none
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_SYS_Delete( void );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　メイン処理
 *
 *	ユニット描画処理
 *	VRAM転送アニメの場合はキャラクタ情報を転送タスクへの登録します。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_SYS_Main( void );

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム　Vブランク処理
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_SYS_VBlankFunc( void );

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターシステム	OAMバッファのクリア関数
 *
 *	＊全CLUNITのOAMが消えますので注意が必要です。
 *	　反映されるのは次のVBlank期間です。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_SYS_ClearAllOamBuffer( void );


//-------------------------------------
///	GFL_CLGRP関係
//	リソース管理関係です。
//=====================================
//-----------------------------------------------------------------------------------------------------------
/**
 *	CGRデータ関連処理
 *
 *	・登録
 *		GFL_CLGRP_CGR_Register					VRAM常駐型データ用
 *		GFL_CLGRP_CGR_Register_VramTransfer		VRAM転送型データ用
 *		GFL_CLGRP_CGR_CreateAlies_VramTransfer	登録済みVRAM転送型データのエイリアスを生成
 *
 *	・解放
 *		GFL_CLGRP_CGR_Release
 *
 *	・VRAM転送元データの差し替え
 *		GFL_CLGRP_CGR_ReplaceSrc_VramTransfer
 *
 *	・VRAM転送元データのポインタ取得
 *		GFL_CLGRP_CGR_GetSrcPointer_VramTransfer
 *
 *	・プロキシ取得
 *		GFL_CLGRP_CGR_GetProxy
 *
 */
//-----------------------------------------------------------------------------------------------------------
//==============================================================================================
/**
 * CGRデータの登録（VRAM常駐型OBJ用）
 *
 * アーカイブからCGRデータをロードしてVRAM転送を行い、プロキシを作成して保持する。
 * CGRデータの実体は破棄する。
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   cgrDataID		[in] アーカイブ内のCGRデータID
 * @param   compressedFlag	[in] データが圧縮されているか
 * @param   targetVram		[in] 転送先VRAM指定
 * @param   heapID			[in] データロード用ヒープ（テンポラリ）
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_CLGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern u32 GFL_CLGRP_CGR_Register( ARCHANDLE* arcHandle, u32 cgrDataID, BOOL compressedFlag, CLSYS_DRAW_TYPE targetVram, HEAPID heapID );


//==============================================================================================
/**
 * CGRデータの登録（VRAM転送型OBJ用）
 *
 * アーカイブからCGRデータをロードし、プロキシを作成して保持する。
 * CGRデータの実体も保持し続ける。
 * CGRデータに関連付けられたセルアニメデータが先に登録されている必要がある。
 *
 * @param   arcHandle		アーカイブハンドル
 * @param   cgrDataID		CGRデータのアーカイブ内ID
 * @param   targetVram		転送先VRAM指定
 * @param   cellIndex		関連付けられたセルアニメデータの登録インデックス
 * @param   heapID			データロード用ヒープ
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_CLGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern u32 GFL_CLGRP_CGR_Register_VramTransfer( ARCHANDLE* arcHandle, u32 cgrDataID, BOOL compressedFlag, CLSYS_DRAW_TYPE targetVram, u32 cellIndex, HEAPID heapID );

//==============================================================================================
/**
 * 登録されているVRAM転送型CGRのエイリアスを生成し、新たに登録する
 *
 * すでに登録されているCGRデータと同一のを参照し、プロキシを作成・保持する。
 * 登録されているCGRデータと同一のものを参照するため、ヒープ使用量を抑えることができる。
 * CGRデータに関連付けられたセルアニメデータが先に登録されている必要がある。
 *
 * ※最初に登録したCGRデータとエイリアス（複数生成可）はどの順番で解放しても良い。
 *
 * @param   srcCgrIdx		[in] すでに登録されているCGRデータインデックス（VRAM転送型）
 * @param   cellAnimIdx		[in] 関連づけられたセルアニメデータの登録インデックス
 * @param   targetVram		[in] 転送先VRAM
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_CLGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern u32 GFL_CLGRP_CGR_CreateAliesVramTransfer( u32 srcCgrIdx, u32 cellAnimIdx, CLSYS_DRAW_TYPE targetVram );

//==============================================================================================
/**
 * 登録されたCGRデータの解放
 *
 * @param   index		登録インデックス
 */
//==============================================================================================
extern void GFL_CLGRP_CGR_Release( u32 index );

//==============================================================================================
/**
 * 登録済みCGRの転送アドレスに別のデータを再配置する(VRAM転送型は不可能)
 *
 * @param   index			CGRデータの登録インデックス
 * @param	src				読み込み済みキャラクタリソース
 *
 */
//==============================================================================================
extern void GFL_CLGRP_CGR_Replace( u32 index, const NNSG2dCharacterData* src );

//==============================================================================================
/**
 * 登録済みCGRの転送アドレスに別のデータを再配置する　細かく指定版(VRAM転送型は不可能)
 *
 * @param   index			CGRデータの登録インデックス
 * @param	  cp_buff   キャラクタ実データ
 * @param   size      転送サイズ  byte単位
 * @param   offs      転送オフセット（登録済みCGRが転送されているアドレスからのオフセット byte単位）
 * @param   vramType  転送先VramType  
 *
 * vramTypeにCLSYS_DRAW_MAXを渡すと、登録済みCGRで転送されている描画面すべてにリソースを転送します。
 *
 */
//==============================================================================================
extern void GFL_CLGRP_CGR_ReplaceEx( u32 index, const void* cp_buff, u32 size, u32 offs, CLSYS_DRAW_TYPE vramType );


//==============================================================================================
/**
 * 登録済みVRAM転送型CGRの転送元データ部を別のデータに差し替える
 *
 * @param   index			CGRデータ（VRAM転送型）の登録インデックス
 * @param   arc				差し替え後のCGRデータを持つアーカイブのハンドル
 * @param   cgrDatIdx		差し替え後のCGRデータのアーカイブ内インデックス
 * @param   compressedFlag	差し替え後のCGRデータが圧縮されているか
 * @param	heapID			作業用ヒープID
 *
 */
//==============================================================================================
extern void GFL_CLGRP_CGR_ReplaceSrc_VramTransfer( u32 index, ARCHANDLE* arc, u32 cgrDatIdx, BOOL compressedFlag, HEAPID heapID );

//==============================================================================================
/**
 * 登録済みVRAM転送型CGRの転送元データポインタを取得（取り扱いは慎重に！）
 *
 * @param   index			CGRデータ（VRAM転送型）の登録インデックス
 *
 * @retval  void*		データポインタ
 */
//==============================================================================================
extern void* GFL_CLGRP_CGR_GetSrcPointer_VramTransfer( u32 index );

//==============================================================================================
/**
 * CGRプロキシ取得
 *
 * @param   index		[in]  登録インデックス
 * @param   proxy		[out] プロキシデータ取得のための構造体アドレス
 *
 */
//==============================================================================================
extern void GFL_CLGRP_CGR_GetProxy( u32 index, NNSG2dImageProxy* proxy );

//--------------------------------------------------------------
/**
 * @brief   登録されたCGRデータから転送先アドレスを取得する
 * @param   index		登録インデックス
 * @param   vramType	転送先VRAM指定
 * @retval  転送先オフセットアドレス
 */
//--------------------------------------------------------------
extern u32 GFL_CLGRP_CGR_GetAddr( u32 index, CLSYS_DRAW_TYPE vramType );



//-----------------------------------------------------------------------------------------------------------
/**
 *	パレットデータ関連処理
 *
 *・登録
 *
 *		GFL_CLGRP_PLTT_Register		（非圧縮パレットデータ専用）
 *			転送先オフセットを指定可能。パレットVRAM容量を超えない範囲で全転送します。
 *
 *		GFL_CLGRP_PLTT_RegisterEx	（非圧縮パレットデータ専用）
 *			転送先オフセットに加え、転送元データの読み込み開始位置、転送本数を指定できます。
 *
 *		GFL_CLGRP_PLTT_RegisterComp	（圧縮パレットデータ専用）
 *			NitroCharacterで編集した通りの位置に転送します。
 *			転送先オフセットを指定してずらすことも出来ます。
 *
 *・解放
 *		GFL_CLGRP_PLTT_Release
 *
 *・プロキシ取得
 *		GFL_CLGRP_PLTT_GetProxy
 *
 */
//-----------------------------------------------------------------------------------------------------------
//==============================================================================================
/**
 * 非圧縮パレットデータの登録およびVRAMへの転送
 * 転送先オフセットに0以外の値を指定すると、VRAMサイズを超えないように転送サイズが適宜、調整される。
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   plttDataID		[in] パレットデータのアーカイブ内データID
 * @param   vramType		[in] 転送先VRAM指定
 * @param   byteOffs		[in] 転送先オフセット（バイト）
 * @param   heapID			[in] データロード用ヒープ（テンポラリ）
 *
 * ※OBJ拡張パレットを指定したい場合、 byteOffsの値に GFL_CLGRP_EXPLTT_OFFSET + オフセットバイト数を指定する
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_CLGRP_REGISTER_FAILED）
 *
 */
//==============================================================================================
extern u32 GFL_CLGRP_PLTT_Register( ARCHANDLE* arcHandle, u32 plttDataID, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID );

//==============================================================================================
/**
 * 非圧縮パレットデータの登録およびVRAMへの転送（拡張版）
 * 転送先オフセット指定の他に、データ読み出し開始位置と転送本数も指定できる。
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   plttDataID		[in] パレットデータのアーカイブ内データID
 * @param   vramType		[in] 転送先VRAM指定
 * @param   byteOffs		[in] 転送先オフセット（バイト）
 * @param   srcStartLine	[in] データ読み出し開始位置（パレット何本目から転送するか？ 1本=16色）
 * @param   numTransLines	[in] 転送本数（何本分転送するか？ 1本=16色／0だと全て転送）
 * @param   heapID			[in] データロード用ヒープ（テンポラリ）
 *
 * ※OBJ拡張パレットを指定したい場合、 byteOffsの値に GFL_CLGRP_EXPLTT_OFFSET を加算して指定する。
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_CLGRP_REGISTER_FAILED）
 *
 */
//==============================================================================================
extern u32 GFL_CLGRP_PLTT_RegisterEx( ARCHANDLE* arcHandle, u32 plttDataID, CLSYS_DRAW_TYPE vramType, u16 byteOffs, u16 srcStartLine, u16 numTransLines, HEAPID heapID );

//=============================================================================================
/**
 * 圧縮パレットデータ（-pcmオプションを指定して生成したデータ）の登録およびVRAMへの転送
 * NitroCharacterで編集した通りにVRAM配置される。
 *（一応、オフセットを指定することでズラすことも可能にしてある）
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   plttDataID		[in] パレットデータのアーカイブ内データID
 * @param   vramType		[in] 転送先VRAM指定
 * @param   byteOffs		[in] 転送先オフセット（バイト）
 * @param   heapID			[in] データロード用ヒープ（テンポラリ）
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_CLGRP_REGISTER_FAILED）
 */
//=============================================================================================
extern u32 GFL_CLGRP_PLTT_RegisterComp( ARCHANDLE* arcHandle, u32 plttDataID, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID );

//==============================================================================================
/**
 * 登録されたパレットデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
extern void GFL_CLGRP_PLTT_Release( u32 index );

//==============================================================================================
/**
 * 登録済み非圧縮型パレットの転送アドレスに別のデータを再配置する
 *
 * @param   index			PLTTデータの登録インデックス
 * @param	src				読み込み済みパレットリソース
 * @param	numTransLines	転送本数（何本分転送するか？ 1本=16色／0だと全て転送） 
 *
 */
//==============================================================================================
extern void GFL_CLGRP_PLTT_Replace( u32 index, const NNSG2dPaletteData* src, u16 numTransLines );

//==============================================================================================
/**
 * 登録されたパレットデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
extern void GFL_CLGRP_PLTT_GetProxy( u32 index, NNSG2dImagePaletteProxy* proxy );

//--------------------------------------------------------------
/**
 * @brief   登録されたパレットデータから転送先アドレスを取得する
 * @param   index		登録インデックス
 * @param   vramType	転送先VRAM指定
 * @retval  転送先オフセットアドレス
 */
//--------------------------------------------------------------
extern u32 GFL_CLGRP_PLTT_GetAddr( u32 index, CLSYS_DRAW_TYPE vramType );


//-----------------------------------------------------------------------------------------------------------
/**
 *	セルアニメデータ関連処理
 *
 *	・登録
 *		GFL_CLGRP_CELLANIM_Register
 *
 *	・解放
 *		GFL_CLGRP_CELLANIM_Release
 *
 *	・VRAM転送型かどうか判定
 *		GFL_CLGRP_CELLANIM_IsVramTransfer
 *
 */
//-----------------------------------------------------------------------------------------------------------
//==============================================================================================
/**
 * セルアニメデータ登録
 *
 * @param   arcHandle		アーカイブハンドル
 * @param   cellDataID		セルデータID
 * @param   animDataID		アニメデータID
 * @param   heapID			データ保持用ヒープID
 *
 * @retval  登録インデックス
 *
 */
//==============================================================================================
extern u32 GFL_CLGRP_CELLANIM_Register( ARCHANDLE* arcHandle, u32 cellDataID, u32 animDataID, HEAPID heapID );

//==============================================================================================
/**
 * 登録されたセルアニメデータの解放
 *
 * @param   index		登録インデックス
 */
//==============================================================================================
extern void GFL_CLGRP_CELLANIM_Release( u32 index );

//==============================================================================================
/**
 * 指定されたセルデータが、VRAM転送型かどうかをチェック
 *
 * @param   index		登録インデックス
 *
 * @retval  BOOL		TRUEならVRAM転送型データ
 */
//==============================================================================================
extern BOOL GFL_CLGRP_CELLANIM_IsVramTransfer( u32 index );



//-------------------------------------
///	GFL_CLUNIT関係
//
//	GFL_CLWKをまとめる１つのグループです。
//　ゲームによってグループの分け方は変わると思いますが、
//	例としては、
//	　フィール内
//	　　　天気グループ
//	　　　メニューリストグループ
//	　とわけ、それぞれにGFL_CLUNITを作成し、表示物の管理を行うことができます。
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットを生成
 *
 *	@param	wknum		ワーク数
 *	@param	unit_pri	ユニット描画優先順位　優先大 0,1,2,3,4,5....優先小
 *	@param	heapID		ヒープID
 *
 *	@return	ユニットポインタ
 */
//-----------------------------------------------------------------------------
extern GFL_CLUNIT* GFL_CLACT_UNIT_Create( u16 wknum, u8 unit_pri, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットを破棄
 *
 *	@param	p_unit			セルアクターユニット
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_Delete( GFL_CLUNIT* p_unit );
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
extern void GFL_CLACT_UNIT_SetDrawEnable( GFL_CLUNIT* p_unit, BOOL on_off );
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
extern BOOL GFL_CLACT_UNIT_GetDrawEnable( const GFL_CLUNIT* cp_unit );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画優先順位を変更
 *	
 *	@param	p_unit		セルアクターユニット
 *	@param	uni_pri		ユニット描画優先順位　優先大 0,1,2,3,4,5....優先小
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_UNIT_SetPri( GFL_CLUNIT* p_unit, u8 uni_pri );
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターユニットの描画優先順位を取得
 *
 *	@param	cp_unit		セルアクターユニット
 *
 *	@retval	描画優先順位　優先大 0,1,2,3,4,5....優先小
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_UNIT_GetPri( const GFL_CLUNIT* cp_unit );
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
extern void GFL_CLACT_UNIT_SetUserRend( GFL_CLUNIT* p_unit, GFL_CLSYS_REND* p_rend );
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
extern void GFL_CLACT_UNIT_SetDefaultRend( GFL_CLUNIT* p_unit );

//-------------------------------------
///	GFL_CLWK関係
//
/*	【setsfの説明】
 *		GFL_CLUNITの使用するレンダラーシステムを変更していないときは
 *		CLSYS_DEFREND_TYPEの値を指定する
 *		・CLSYS_DEFREND_MAIN指定時	pos_x/yがメイン画面左上座標からの相対座標になる。
 *		・CLSYS_DEFREND_SUB指定時	pos_x/yがサブ画面左上座標からの相対座標になる。
 *		
 *		独自のレンダラーシステム(GFL_CLSYS_REND)をGFL_CLUNITに設定しているときは、
 *		サーフェースの要素数を指定することで、
 *		指定されたサーフェース左上座標からの相対座標になる。
 *
 *		通常/独自レンダラーシステム共通で、
 *		CLWK_SETSF_NONEを指定すると絶対座標設定になる
 */
//=====================================

//	CLGRPのリソースを使用した登録
extern GFL_CLWK* GFL_CLACT_WK_Create( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_DATA* param, u16 setSerface, HEAPID heapID );

extern GFL_CLWK* GFL_CLACT_WK_CreateVT( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_DATA* param, u16 setSerface, HEAPID heapID );

extern GFL_CLWK* GFL_CLACT_WK_CreateAffine( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex,
	const GFL_CLWK_AFFINEDATA* param, u16 setSerface, HEAPID heapID );

extern GFL_CLWK* GFL_CLACT_WK_CreateVTAffine( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_AFFINEDATA* param, u16 setSerface, HEAPID heapID );


//----------------------------------------------------------------------------
/**
 *	@brief	破棄処理
 *
 *	@param	p_wk	破棄するセルアクターワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_Remove( GFL_CLWK* p_wk );

// ワークに対する操作
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
extern void GFL_CLACT_WK_SetDrawEnable( GFL_CLWK* p_wk, BOOL on_off );
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
extern BOOL GFL_CLACT_WK_GetDrawEnable( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_pos			座標構造体
 *	@param	setsf			サーフェースインデックス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetPos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos, u16 setsf );
//----------------------------------------------------------------------------
/**
 *	@brief	サーフェース内相対座標取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pos			座標格納先
 *	@param	setsf			サーフェースインデックス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetPos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos, u16 setsf );
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
extern void GFL_CLACT_WK_SetTypePos( GFL_CLWK* p_wk, s16 pos, u16 setsf, CLSYS_MAT_TYPE type );
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
extern s16 GFL_CLACT_WK_GetTypePos( const GFL_CLWK* cp_wk, u16 setsf, CLSYS_MAT_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標設定
 *
 *	@param	p_wk			セルアクターワーク	
 *	@param	cp_pos			座標
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetWldPos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pos			座標格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetWldPos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	絶対座標個別設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	pos				設定値
 *	@param	type			座標タイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetWldTypePos( GFL_CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type );
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
extern s16 GFL_CLACT_WK_GetWldTypePos( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type );
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
extern void GFL_CLACT_WK_SetAffineParam( GFL_CLWK* p_wk, CLSYS_AFFINETYPE affine );
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
extern CLSYS_AFFINETYPE GFL_CLACTWkGetAffineParam( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標一括設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	cp_pos		アフィン変換相対座標
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAffinePos( GFL_CLWK* p_wk, const GFL_CLACTPOS* cp_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標一括取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	p_pos		アフィン変換相対座標取得先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetAffinePos( const GFL_CLWK* cp_wk, GFL_CLACTPOS* p_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	アフィン変換相対座標個別設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pos			設定値
 *	@param	type		座標タイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetTypeAffinePos( GFL_CLWK* p_wk, s16 pos, CLSYS_MAT_TYPE type );
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
extern s16 GFL_CLACT_WK_GetTypeAffinePos( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type );
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
extern void GFL_CLACT_WK_SetScale( GFL_CLWK* p_wk, const GFL_CLSCALE* cp_sca );
//----------------------------------------------------------------------------
/**
 *	@brief	拡大縮小値一括取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_sca			拡大縮小値取得先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetScale( const GFL_CLWK* cp_wk, GFL_CLSCALE* p_sca );
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
extern void GFL_CLACT_WK_SetTypeScale( GFL_CLWK* p_wk, fx32 scale, CLSYS_MAT_TYPE type );
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
extern fx32 GFL_CLACT_WK_GetTypeScale( const GFL_CLWK* cp_wk, CLSYS_MAT_TYPE type );
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
extern void GFL_CLACT_WK_SetRotation( GFL_CLWK* p_wk, u16 rotation );
//----------------------------------------------------------------------------
/**
 *	@brief	回転角度を取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@return	回転角度(0～0xffff 0xffffが360度)
 */
//-----------------------------------------------------------------------------
extern u16  GFL_CLACT_WK_GetRotation( const GFL_CLWK* cp_wk );
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
extern void GFL_CLACT_WK_SetFlip( GFL_CLWK* p_wk, CLWK_FLIP_TYPE flip_type, BOOL on_off );
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
extern BOOL GFL_CLACT_WK_GetFlip( const GFL_CLWK* cp_wk, CLWK_FLIP_TYPE flip_type );
//----------------------------------------------------------------------------
/**
 *	@brief		オブジェモード設定
 *	
 *	@param	p_wk		セルアクターワーク
 *	@param	mode		オブジェモード
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetObjMode( GFL_CLWK* p_wk, GXOamMode mode );
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
extern GXOamMode GFL_CLACT_WK_GetObjMode( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	モザイクフラグ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	on_off		モザイクON－OFF	TRUE:On	FALSE:Off
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetMosaic( GFL_CLWK* p_wk, BOOL on_off );
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
extern BOOL GFL_CLACT_WK_GetMosaic( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	pal_offs	パレットオフセット
 *	@param	mode		オフセット設定モード
 *
 *	CLWK_PLTTOFFS_MODE_PLTT_TOP
 *		パレットプロクシアドレス+pal_offsの
 *		パレット番号を使用するようになります。
 * 
 *	CLWK_PLTTOFFS_MODE_OAM_COLOR
 *		パレットプロクシアドレス+pal_offs+OamAttr.c_paramの
 *		パレット番号を使用するようになります。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetPlttOffs( GFL_CLWK* p_wk, u8 pal_offs, CLWK_PLTTOFFS_MODE mode );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット設定モードの取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	パレットオフセット設定モード
 *
 *	CLWK_PLTTOFFS_MODE_PLTT_TOP
 *		パレットプロクシアドレス+pal_offsの
 *		パレット番号を使用するようになります。
 * 
 *	CLWK_PLTTOFFS_MODE_OAM_COLOR
 *		パレットプロクシアドレス+pal_offs+OamAttr.c_paramの
 *		パレット番号を使用するようになります。
 */
//-----------------------------------------------------------------------------
extern CLWK_PLTTOFFS_MODE GFL_CLACT_WK_GetPlttOffsMode( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセット取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	pal_offs	パレットオフセット
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_WK_GetPlttOffs( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットプロクシのVRAMオフセットアドレス取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		描画タイプ
 *
 *	@retval	CLWK_VRAM_ADDR_NONE以外	パレットVRAMオフセットアドレス
 *	@retval	CLWK_VRAM_ADDR_NONE		読み込まれていない
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetPlttAddr( const GFL_CLWK* cp_wk, CLSYS_DRAW_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	新しいパレットプロクシを設定する
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_pltt			設定するパレットプロクシ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetPlttProxy( GFL_CLWK* p_wk, const NNSG2dImagePaletteProxy* cp_pltt );
//----------------------------------------------------------------------------
/**
 *	@brief	パレットプロクシデータの取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_pltt			パレットプロクシ格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetPlttProxy( const GFL_CLWK* cp_wk,  NNSG2dImagePaletteProxy* p_pltt );
//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータVRAMオフセットアドレスを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *	@param	type		描画タイプ
 *
 *	@retval	CLWK_VRAM_ADDR_NONE以外	キャラクタのVRAMオフセットアドレス
 *	@retval	CLWK_VRAM_ADDR_NONE		読み込まれていない
 */
//-----------------------------------------------------------------------------
extern u32 GFL_CLACT_WK_GetCharAddr( const GFL_CLWK* cp_wk, CLSYS_DRAW_TYPE type );
//----------------------------------------------------------------------------
/**
 *	@brief	イメージプロクシを設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	cp_img			設定イメージプロクシ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetImgProxy( GFL_CLWK* p_wk, const NNSG2dImageProxy* cp_img );
//----------------------------------------------------------------------------
/**
 *	@brief	イメージプロクシを取得
 *
 *	@param	cp_wk			セルアクターワーク
 *	@param	p_img			イメージプロクシ格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetImgProxy( const GFL_CLWK* cp_wk,  NNSG2dImageProxy* p_img );
//----------------------------------------------------------------------------
/**
 *	@brief	CLGRP CGRインデックスを取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@retval CLGRP CGRインデックス
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetCgrIndex( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	CLGRP PLTTインデックスを取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@retval CLGRP PLTTインデックス
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetPlttIndex( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	ソフト優先順位の設定
 *	
 *	@param	p_wk			セルアクターワーク
 *	@param	pri				ソフト優先順位	0>0xff
 *	
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetSoftPri( GFL_CLWK* p_wk, u8 pri );
//----------------------------------------------------------------------------
/**
 *	@brief	ソフト優先順位の取得
 *
 *	@param	cp_wk			セルアクターワーク
 *
 *	@return	ソフト優先順位
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_WK_GetSoftPri( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	BG優先順位の設定
 *
 *	@param	p_wk			セルアクターワーク
 *	@param	pri				BG優先順位
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetBgPri( GFL_CLWK* p_wk, u8 pri );
//----------------------------------------------------------------------------
/**
 *	@brief	BG優先順位を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	BG優先順位
 */
//-----------------------------------------------------------------------------
extern u8 GFL_CLACT_WK_GetBgPri( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmSeq( GFL_CLWK* p_wk, u16 anmseq );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetAnmSeq( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスの数を取得
 *	
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションシーケンスの数
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetAnmSeqNum( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスが変わっていたら変更する
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmSeqDiff( GFL_CLWK* p_wk, u16 anmseq );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションシーケンスを変更する　
 *			その際フレーム時間などのリセットを行いません。　MultiCell非対応
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	anmseq		アニメーションシーケンス
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmSeqNoReset( GFL_CLWK* p_wk, u16 anmseq );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレームを設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	frame		フレーム数
 *
 *	マルチセルでは、使用不可能です。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmFrame( GFL_CLWK* p_wk, fx32 frame );
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
extern void GFL_CLACT_WK_AddAnmFrame( GFL_CLWK* p_wk, fx32 speed );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションフレーム数を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションフレーム数
 */
//-----------------------------------------------------------------------------
extern fx32 GFL_CLACT_WK_GetAnmFrame( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションインデックスを設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	idx		  インデックス数
 *	NitroCharacterのアニメーションシーケンス内のコマ番号を指定してください
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAnmIndex( GFL_CLWK* p_wk, u16 idx );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションインデックス数を取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	アニメーションインデックス数
 */
//-----------------------------------------------------------------------------
extern u16 GFL_CLACT_WK_GetAnmIndex( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグ設定
 *
 *	@param	p_wk		セルアクターワーク
 *	@param	on_off		オートアニメーションOn-Off	TRUE:On	FALSE:Off
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_SetAutoAnmFlag( GFL_CLWK* p_wk, BOOL on_off );
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
extern BOOL GFL_CLACT_WK_GetAutoAnmFlag( const GFL_CLWK* cp_wk );
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
extern void GFL_CLACT_WK_SetAutoAnmSpeed( GFL_CLWK* p_wk, fx32 speed );
//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションスピードを取得
 *
 *	@param	cp_wk		セルアクターワーク
 *
 *	@return	オートアニメーションスピード
 */
//-----------------------------------------------------------------------------
extern fx32 GFL_CLACT_WK_GetAutoAnmSpeed( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション開始
 *
 *	@param	p_wk		セルアクターワーク
 *	GFL_CLACT_WK_StopAnmでとめたアニメーションを開始させます。
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_StartAnm( GFL_CLWK* p_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション停止
 *
 *	@param	p_wk		セルアクターワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_StopAnm( GFL_CLWK* p_wk );
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
extern BOOL GFL_CLACT_WK_CheckAnmActive( const GFL_CLWK* cp_wk );
//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションリスタート
 *
 *	@param	p_wk	セルアクターワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_ResetAnm( GFL_CLWK* p_wk );
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
extern void GFL_CLACT_WK_SetAnmMode( GFL_CLWK* p_wk, CLSYS_ANM_PLAYMODE playmode );
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
extern CLSYS_ANM_PLAYMODE GFL_CLACT_WK_GetAnmMode( const GFL_CLWK* cp_wk );
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
extern u32 GFL_CLACT_WK_GetUserAttrAnmSeq( const GFL_CLWK* cp_wk, u32 seq );
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
extern u32 GFL_CLACT_WK_GetUserAttrAnmSeqNow( const GFL_CLWK* cp_wk );
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
extern u32 GFL_CLACT_WK_GetUserAttrAnmFrame( const GFL_CLWK* cp_wk, u32 seq, u32 frame );
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
extern u32 GFL_CLACT_WK_GetUserAttrAnmFrameNow( const GFL_CLWK* cp_wk );
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
extern u32 GFL_CLACT_WK_GetUserAttrCell( const GFL_CLWK* cp_wk, u32 cellidx );


//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションの、構成要素を取得する。
 *	  ＊Multi Cell　非対応
 *
 *	@param	cp_wk         ワーク
 *	@param	p_element     エレメント
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_GetAnmElementNow( const GFL_CLWK* cp_wk, GFL_CLWK_ANMELEMENT* p_element );

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションコールバックを設定
 *
 *	@param	p_wk         ワーク
 *	@param  cp_data       コールバック情報
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_StartAnmCallBack( GFL_CLWK* p_wk, const GFL_CLWK_ANM_CALLBACK* cp_data );


//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションコールバックをOFF
 *
 *	@param	p_wk         ワーク
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_WK_StopAnmCallBack( GFL_CLWK* p_wk );




//-------------------------------------
///	USER定義レンダラー関係
// 自分独自のサーフェース設定をした
// レンダラーを使用したいとき、この関数郡で作成して、
// セルアクターユニットに関連付けることが出来ます。
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
extern GFL_CLSYS_REND* GFL_CLACT_USERREND_Create( const GFL_REND_SURFACE_INIT* cp_data, u32 data_num, HEAPID heapID );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムを破棄する
 *
 *	@param	p_rend			ユーザー定義サーフェースデータ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_Delete( GFL_CLSYS_REND* p_rend );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェース左上座標を設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	cp_pos			設定する座標
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_SetSurfacePos( GFL_CLSYS_REND* p_rend, u32 idx, const GFL_CLACTPOS* cp_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェース左上座標を取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	p_pos			座標格納先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_GetSurfacePos( const GFL_CLSYS_REND* cp_rend, u32 idx, GFL_CLACTPOS* p_pos );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェースサイズを設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	cp_size			サイズ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_SetSurfaceSize( GFL_CLSYS_REND* p_rend, u32 idx, const GFL_CLACTPOS* cp_size );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムからサーフェースサイズを取得する
 *
 *	@param	cp_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	p_size			サイズ取得先
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_GetSurfaceSize( const GFL_CLSYS_REND* cp_rend, u32 idx, GFL_CLACTPOS* p_size );
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザー定義レンダラーシステムにサーフェースタイプを設定する
 *
 *	@param	p_rend			ユーザー定義レンダラーシステム
 *	@param	idx				サーフェースインデックス
 *	@param	type			サーフェースタイプ
 */
//-----------------------------------------------------------------------------
extern void GFL_CLACT_USERREND_SetSurfaceType( GFL_CLSYS_REND* p_rend, u32 idx, CLSYS_DRAW_TYPE type );
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
extern CLSYS_DRAW_TYPE GFL_CLACT_USERREND_GetSurfaceType( const GFL_CLSYS_REND* cp_rend, u32 idx );


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif		// __CLACT_H__

