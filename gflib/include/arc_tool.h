
//============================================================================================
/**
 * @file	arc_tool.h
 * @bfief	アーカイバ用ツールプログラム
 * @author	HisashiSogabe
 * @date	05.05.30
 */
//============================================================================================

#ifndef	__ARC_TOOL_H_
#define	__ARC_TOOL_H_

//============================================================================================
//		定数定義
//============================================================================================

#define	OFS_NO_SET		(0)		///<ArchiveDataLoadOfs,ArchiveDataLoadOfs用OFS値なしの定数
#define	SIZE_NO_SET		(0)		///<ArchiveDataLoadOfs,ArchiveDataLoadOfs用SIZE値なしの定数

//============================================================================================
/**
 *
 *	アーカイブシステム初期化
 *
 * @param[in]	tbl		アーカイブデータテーブルのアドレス
 * @param[in]	tbl_max	アーカイブデータテーブルの要素数のMAX
 *
 */
//============================================================================================
extern	void	GFL_ARC_sysInit(char *tbl,int tbl_max);

//============================================================================================
/**
 *
 *	アーカイブシステム終了（現状は特になにもしない）
 *
 */
//============================================================================================
extern	void	GFL_ARC_sysExit(void);

extern	void	GFL_ARC_DataLoad(void *data,int file_kind,int index);
extern	void	*GFL_ARC_DataLoadMalloc(int file_kind,int index,int heap_id);
extern	void	*GFL_ARC_DataLoadMallocLo(int file_kind,int index,int heap_id);
extern	void	GFL_ARC_DataLoadOfs(void *data,int file_kind,int index,int ofs,int size);
extern	void	*GFL_ARC_DataLoadMallocOfs(int file_kind,int index,int heap_id,int ofs,int size);
extern	void	*GFL_ARC_DataLoadMallocOfsLo(int file_kind,int index,int heap_id,int ofs,int size);
extern	u16		GFL_ARC_DataFileCntGet(int file_kind,int index);
extern	u32		GFL_ARC_DataSizeGet(int file_kind,int index);

/*====================================================================================*/
/*
  アーカイブデータハンドル処理

    １つのアーカイブファイルから何度もデータ取得を行いたい時、ファイルの
  オープン・クローズ回数を減らすことで処理負荷を軽減させるための仕組み。
*/
/*====================================================================================*/


//--------------------------------------------------
/**
 * アーカイブデータハンドル構造体の宣言
 */
//--------------------------------------------------
typedef struct _ARCHANDLE  ARCHANDLE;

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルオープン
 * （内部でファイルをオープンします）
 *
 * @param   arcId		アーカイブデータインデックス
 * @param   heapId		管理用ヒープＩＤ
 *
 * @retval  ARCHANDLE	オープンされたハンドルのポインタ（失敗ならNULL）
 */
//------------------------------------------------------------------
extern ARCHANDLE* GFL_ARC_DataHandleOpen( u32 arcId, u32 heapId );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルクローズ
 * （内部でファイルをクローズします）
 *
 * @param   handle			ハンドルポインタ
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_DataHandleClose( ARCHANDLE* handle );


//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータ取得
 *
 * @param   handle		ハンドルポインタ
 * @param   datId		アーカイブ内のデータインデックス
 * @param   buffer		データ読み込み先バッファ
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_DataLoadByHandle( ARCHANDLE* handle, u32 datId, void* buffer );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータサイズ取得
 *
 * @param   handle		ハンドルポインタ
 * @param   datId		アーカイブ内のデータインデックス
 * @retval	u32			インデックスのデータサイズ
 *
 */
//------------------------------------------------------------------
extern u32 GFL_ARC_DataSizeGetByHandle( ARCHANDLE* handle, u32 datId );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータ取得（読み込み開始オフセット＆サイズ指定）
 *
 * @param   handle		ハンドルポインタ
 * @param   datId		アーカイブ内のデータインデックス
 * @param   ofs			読み込み開始オフセット
 * @param   size		読み込みサイズ
 * @param   buffer		データ読み込み先バッファ
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_DataLoadOfsByHandle( ARCHANDLE* handle, u32 datId, u32 ofs, u32 size, void* buffer );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータ取得（内部でアロケートしたメモリに）
 *
 * @param   handle			ハンドルポインタ
 * @param   datID			アーカイブ内のデータインデックス
 * @param   heapID			メモリアロケート用のヒープＩＤ
 *
 * @retval  u32				データサイズ（バイト）
 */
//------------------------------------------------------------------
extern void* GFL_ARC_DataLoadAllocByHandle( ARCHANDLE* handle, u32 datID, u32 heapID );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってイメージデータオフセット取得
 *
 * @param   handle		ハンドルポインタ
 * @param   datId		アーカイブ内のデータインデックス
 * @param   offset		オフセット読み込み先バッファ
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_DataLoadImgofsByHandle( ARCHANDLE* handle, u32 datId, u32* offset );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータ取得（ファイルハンドル内情報の続きから読み込み）
 *
 * @param   handle		ハンドルポインタ
 * @param   size		読み込みサイズ
 * @param   buffer		データ読み込み先バッファ
 *
 */
//------------------------------------------------------------------
extern	void GFL_ARC_DataLoadByHandleContinue( ARCHANDLE* handle, u32 size, void* buffer );

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを介してファイルシーク
 *
 * @param   handle		ハンドルポインタ
 * @param   size
 *
 */
//------------------------------------------------------------------
extern	void GFL_ARC_DataSeekByHandle( ARCHANDLE* handle, u32 size );

//============================================================================================
/**
 * アーカイブファイルデータのファイル数を取得(ハンドル使用)
 *
 * @param	handle			ハンドルポインタ
 * @retval	u16				ファイル数	
 */
//============================================================================================
extern	u16		GFL_ARC_DataFileCntGetByHandle(ARCHANDLE* handle);

#endif	__ARC_TOOL_H_

