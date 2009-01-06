//==============================================================================
/**
 *
 *@file		obj_graphic_man.h
 *@brief	OBJ用グラフィックデータ転送、管理システム
 *@author	taya
 *@data		2006.11.28
 *
 */
//==============================================================================
#ifndef __OBJ_GRAPHIC_MAN_H__
#define __OBJ_GRAPHIC_MAN_H__

#include <clact.h>
#include <arc_tool.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================
//	初期化用構造体
//==============================================================
typedef struct {
	u16  CGR_RegisterMax;			///< 登録できるキャラデータ数
	u16  PLTT_RegisterMax;			///< 登録できるパレットデータ数
	u16  CELL_RegisterMax;			///< 登録できるセルアニメパターン数
	u16  MULTICELL_RegisterMax;		///< 登録できるマルチセルアニメパターン数（※現状未対応）
}GFL_OBJGRP_INIT_PARAM;


//==============================================================
//	定数
//==============================================================
enum {
	GFL_OBJGRP_REGISTER_FAILED = 0xffffffff,
	GFL_OBJGRP_EXPLTT_OFFSET = 0x200,
};


//==============================================================
//	VRAM指定用シンボル
//==============================================================
typedef enum {
	GFL_VRAM_2D_MAIN = 1,
	GFL_VRAM_2D_SUB = 2,
	GFL_VRAM_2D_BOTH = GFL_VRAM_2D_MAIN | GFL_VRAM_2D_SUB,
}GFL_VRAM_TYPE;




//-----------------------------------------------------------------------------------------------------------
/**
 *	システム初期化＆終了処理
 *
 *	プログラム起動後に１回だけコールします。
 *	アプリケーションレベルでは呼び出す必要がありません。
 */
//-----------------------------------------------------------------------------------------------------------
extern void GFL_OBJGRP_Init( void );
extern void GFL_OBJGRP_Exit( void );

//-----------------------------------------------------------------------------------------------------------
/**
 *	アプリケーション単位の初期化＆終了処理
 *
 *	アプリケーションごとに呼び出してください。
 *	内部で必要なメモリを確保／解放します。
 */
//-----------------------------------------------------------------------------------------------------------
extern void GFL_OBJGRP_sysStart( HEAPID heapID, const GFL_DISP_VRAM* vramBank, const GFL_OBJGRP_INIT_PARAM* initParam );
extern void GFL_OBJGRP_sysEnd( void );


//-----------------------------------------------------------------------------------------------------------
/**
 *	CGRデータ関連処理
 *
 *	・登録
 *		GFL_OBJGRP_RegisterCGR					VRAM常駐型データ用
 *		GFL_OBJGRP_RegisterCGR_VramTransfer		VRAM転送型データ用
 *		GFL_OBJGRP_CGR_CreateAlies_VramTransfer	登録済みVRAM転送型データのエイリアスを生成
 *
 *	・解放
 *		GFL_OBJGRP_ReleaseCGR
 *
 *	・VRAM転送元データの差し替え
 *		GFL_OBJGRP_CGR_ReplaceSrc_VramTransfer
 *
 *	・VRAM転送元データのポインタ取得
 *		GFL_OBJGRP_CGR_GetSrcPointer_VramTransfer
 *
 *	・プロキシ取得
 *		GFL_OBJGRP_CGR_GetProxy
 *
 *
 *	※ホントは関数名をこうしたい※
 *
 *	GFL_OBJGRP_RegisterCGR -> GFL_OBJGRP_CGR_Register
 *	GFL_OBJGRP_RegisterCGR_VramTransfer -> GFL_OBJGRP_CGR_Register_VramTransfer
 *
 *	xxxx_VramTransfer の部分は長いので、xxxx_VT とかにしてもいいかも。
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
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern u32 GFL_OBJGRP_RegisterCGR( ARCHANDLE* arcHandle, u32 cgrDataID, BOOL compressedFlag, GFL_VRAM_TYPE targetVram, HEAPID heapID );


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
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern u32 GFL_OBJGRP_RegisterCGR_VramTransfer( ARCHANDLE* arcHandle, u32 cgrDataID, BOOL compressedFlag, GFL_VRAM_TYPE targetVram, u32 cellIndex, HEAPID heapID );

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
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern u32 GFL_OBJGRP_CGR_CreateAliesVramTransfer( u32 srcCgrIdx, u32 cellAnimIdx, GFL_VRAM_TYPE targetVram );

//==============================================================================================
/**
 * 登録されたCGRデータの解放
 *
 * @param   index		登録インデックス
 */
//==============================================================================================
extern void GFL_OBJGRP_ReleaseCGR( u32 index );

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
extern void GFL_OBJGRP_CGR_ReplaceSrc_VramTransfer( u32 index, ARCHANDLE* arc, u32 cgrDatIdx, BOOL compressedFlag, HEAPID heapID );

//==============================================================================================
/**
 * 登録済みVRAM転送型CGRの転送元データポインタを取得（取り扱いは慎重に！）
 *
 * @param   index			CGRデータ（VRAM転送型）の登録インデックス
 *
 * @retval  void*		データポインタ
 */
//==============================================================================================
extern void* GFL_OBJGRP_CGR_GetSrcPointer_VramTransfer( u32 index );

//==============================================================================================
/**
 * CGRプロキシ取得
 *
 * @param   index		[in]  登録インデックス
 * @param   proxy		[out] プロキシデータ取得のための構造体アドレス
 *
 */
//==============================================================================================
extern void GFL_OBJGRP_CGR_GetProxy( u32 index, NNSG2dImageProxy* proxy );


//-----------------------------------------------------------------------------------------------------------
/**
 *	セルアニメデータ関連処理
 *
 *	・登録
 *		GFL_OBJGRP_RegisterCellAnim
 *
 *	・解放
 *		GFL_OBJGRP_ReleaseCellAnim
 *
 *	・VRAM転送型かどうか判定
 *		GFL_OBJGRP_CELLANIM_IsVramTransfer
 *
 *	※ホントは関数名をこうしたい※
 *
 *	GFL_OBJGRP_RegisterCellAnim -> GFL_OBJGRP_CELLANIM_Register
 *	GFL_OBJGRP_ReleaseCellAnim  -> GFL_OBJGRP_CELLANIM_Release
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
extern u32 GFL_OBJGRP_RegisterCellAnim( ARCHANDLE* arcHandle, u32 cellDataID, u32 animDataID, HEAPID heapID );

//==============================================================================================
/**
 * 登録されたセルアニメデータの解放
 *
 * @param   index		登録インデックス
 */
//==============================================================================================
extern void GFL_OBJGRP_ReleaseCellAnim( u32 index );

//==============================================================================================
/**
 * 指定されたセルデータが、VRAM転送型かどうかをチェック
 *
 * @param   index		登録インデックス
 *
 * @retval  BOOL		TRUEならVRAM転送型データ
 */
//==============================================================================================
extern BOOL GFL_OBJGRP_CELLANIM_IsVramTransfer( u32 index );




//-----------------------------------------------------------------------------------------------------------
/**
 *	パレットデータ関連処理
 *
 *・登録
 *
 *		GFL_OBJGRP_RegisterPltt		（非圧縮パレットデータ専用）
 *			転送先オフセットを指定可能。パレットVRAM容量を超えない範囲で全転送します。
 *
 *		GFL_OBJGRP_RegisterPlttEx	（非圧縮パレットデータ専用）
 *			転送先オフセットに加え、転送元データの読み込み開始位置、転送本数を指定できます。
 *
 *		GFL_OBJGRP_RegisterCompPltt	（圧縮パレットデータ専用）
 *			NitroCharacterで編集した通りの位置に転送します。
 *			転送先オフセットを指定してずらすことも出来ます。
 *
 *・解放
 *		GFL_OBJGRP_ReleasePltt
 *
 *・プロキシ取得
 *		GFL_OBJGRP_GetPlttProxy
 *
 *
 *	※ホントは関数名をこうしたい※
 *
 *	GFL_OBJGRP_RegisterPltt     -> GFL_OBJGRP_PLTT_Register
 *	GFL_OBJGRP_RegisterPlttEx   -> GFL_OBJGRP_PLTT_RegisterEx
 *	GFL_OBJGRP_RegisterCompPltt -> GFL_OBJGRP_PLTT_RegisterComp
 *	GFL_OBJGRP_ReleasePltt      -> GFL_OBJGRP_PLTT_Release
 *	GFL_OBJGRP_GetPlttProxy     -> GFL_OBJGRP_PLTT_GetProxy
 *
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
 * ※OBJ拡張パレットを指定したい場合、 byteOffsの値に GFL_OBJGRP_EXPLTT_OFFSET + オフセットバイト数を指定する
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 *
 */
//==============================================================================================
extern u32 GFL_OBJGRP_RegisterPltt( ARCHANDLE* arcHandle, u32 plttDataID, GFL_VRAM_TYPE vramType, u16 byteOffs, HEAPID heapID );

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
 * ※OBJ拡張パレットを指定したい場合、 byteOffsの値に GFL_OBJGRP_EXPLTT_OFFSET を加算して指定する。
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 *
 */
//==============================================================================================
extern u32 GFL_OBJGRP_RegisterPlttEx( ARCHANDLE* arcHandle, u32 plttDataID, GFL_VRAM_TYPE vramType, u16 byteOffs, u16 srcStartLine, u16 numTransLines, HEAPID heapID );

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
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//=============================================================================================
extern u32 GFL_OBJGRP_RegisterCompPltt( ARCHANDLE* arcHandle, u32 plttDataID, GFL_VRAM_TYPE vramType, u16 byteOffs, HEAPID heapID );

//==============================================================================================
/**
 * 登録されたパレットデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
extern void GFL_OBJGRP_ReleasePltt( u32 index );

//==============================================================================================
/**
 * 登録されたパレットデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
extern void GFL_OBJGRP_GetPlttProxy( u32 index, NNSG2dImagePaletteProxy* proxy );



//-----------------------------------------------------------------------------------------------------------
//
//	セルアクター生成関数
//
//-----------------------------------------------------------------------------------------------------------
extern GFL_CLWK* GFL_OBJGRP_CreateClAct( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_DATA* param, u16 setSerface, HEAPID heapID );

extern GFL_CLWK* GFL_OBJGRP_CreateClActVT( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_DATA* param, u16 setSerface, HEAPID heapID );

extern GFL_CLWK* GFL_OBJGRP_CreateClActAffine( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex,
	const GFL_CLWK_AFFINEDATA* param, u16 setSerface, HEAPID heapID );

extern GFL_CLWK* GFL_OBJGRP_CreateClActVTAffine( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_AFFINEDATA* param, u16 setSerface, HEAPID heapID );



#ifdef __cplusplus
}/* extern "C" */
#endif

#endif /* #ifndef __OBJMAN_H__ */
