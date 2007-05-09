
//============================================================================================
/**
 * @file	arc_tool.c
 * @bfief	アーカイバ用ツールプログラム
 * @author	HisashiSogabe
 * @date	05.05.30
 */
//============================================================================================

#include "gflib.h"

#include "arc_tool.h"
#include "arc_tool_def.h"

//============================================================================================
//		定数定義
//============================================================================================

#define	OFS_NO_SET		(0)		///<ArchiveDataLoadOfs,ArchiveDataLoadOfs用OFS値なしの定数
#define	SIZE_NO_SET		(0)		///<ArchiveDataLoadOfs,ArchiveDataLoadOfs用SIZE値なしの定数

//============================================================================================
//	プロトタイプ宣言
//============================================================================================

static	void	ArchiveLoadDataIndex(void *data,const char *name,int datID,int ofs,int ofs_size);
static	void*	ArchiveLoadDataIndexMalloc(const char *name,int datID,HEAPID heapID,int ofs,int ofs_size);

//============================================================================================
//	アーカイブテーブル格納変数
//============================================================================================

static	char	***ArchiveFileTable=NULL;
static	int		ArchiveFileTableMax=0;

//============================================================================================
/**
 *	アーカイブシステム初期化
 *
 * @param[in]	tbl		アーカイブデータテーブルのアドレス
 * @param[in]	tbl_max	アーカイブデータテーブルの要素数のMAX
 */
//============================================================================================
void	GFL_ARC_Init(const char **tbl,int tbl_max)
{
	ArchiveFileTable=(char ***)tbl;
	ArchiveFileTableMax=tbl_max;
}

//============================================================================================
/**
 *	アーカイブシステム終了
 */
//============================================================================================
void	GFL_ARC_Exit(void)
{
	ArchiveFileTable=NULL;
	ArchiveFileTableMax=0;
}

//============================================================================================
/**
 *	nnsarcで作成したアーカイブファイルに対してIndexを指定して任意のデータを取り出す
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	datID		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	ofs_size	読み込むデータサイズ
 */
//============================================================================================
static	void	ArchiveLoadDataIndex(void *data,const char *name,int datID,int ofs,int ofs_size)
{
	FSFile		p_file;
	u32			size=0;
	u32			fat_top=0;
	u32			fnt_top=0;
	u32			img_top=0;
	u32			top=0;
	u32			bottom=0;
	u16			file_cnt=0;

	FS_InitFile(&p_file);
	FS_OpenFile(&p_file,name);
	FS_SeekFile(&p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(&p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(&p_file,&file_cnt,2);								///<FileCountをロード
	GF_ASSERT_MSG(file_cnt>datID,"ArchiveLoadDataIndex file=%s, fileCnt=%d, datID=%d", name, file_cnt, datID);
	fnt_top=fat_top+size;
	FS_SeekFile(&p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(&p_file,fat_top+FAT_HEAD_SIZE+datID*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(&p_file,&top,4);									///<FATテーブルtopをロード
	FS_ReadFile(&p_file,&bottom,4);									///<FATテーブルbottomをロード

	FS_SeekFile(&p_file,img_top+IMG_HEAD_SIZE+top+ofs,FS_SEEK_SET);	///<取り出したいIMGの先頭に移動
	
	if(ofs_size){
		size=ofs_size;
	}
	else{
		size=bottom-top;
	}

	GF_ASSERT_MSG(size!=0,"ArchiveLoadDataIndex:ReadDataSize=0!");
	FS_ReadFile(&p_file,data,size);									///<データをロード

	FS_CloseFile(&p_file);
}


//============================================================================================
/**
 *	nnsarcで作成したアーカイブファイルに対してIndexを指定して任意のデータを取り出す
 *	読み込んだデータを格納するワークもこの関数内で確保して、ポインタを返す
 *
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	datID		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	ofs_size	読み込むデータサイズ
 */
//============================================================================================
static	void*	ArchiveLoadDataIndexMalloc(const char *name,int datID,HEAPID heapID,int ofs,int ofs_size)
{
	FSFile		p_file;
	u32			size=0;
	u32			fat_top=0;
	u32			fnt_top=0;
	u32			img_top=0;
	u32			top=0;
	u32			bottom=0;
	void		*data;
	u16			file_cnt=0;

	FS_InitFile(&p_file);
	FS_OpenFile(&p_file,name);
	FS_SeekFile(&p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(&p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(&p_file,&file_cnt,2);								///<FileCountをロード
	GF_ASSERT_MSG(file_cnt>datID,"ArchiveLoadDataIndexMalloc file=%s, fileCnt=%d, datID=%d", name, file_cnt, datID);
	fnt_top=fat_top+size;
	FS_SeekFile(&p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(&p_file,fat_top+FAT_HEAD_SIZE+datID*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(&p_file,&top,4);									///<FATテーブルtopをロード
	FS_ReadFile(&p_file,&bottom,4);									///<FATテーブルbottomをロード

	FS_SeekFile(&p_file,img_top+IMG_HEAD_SIZE+top+ofs,FS_SEEK_SET);		///<取り出したいIMGの先頭に移動

	if(ofs_size){
		size=ofs_size;
	}
	else{
		size=bottom-top;
	}
	GF_ASSERT_MSG(size!=0,"ArchiveLoadDataIndexMalloc:ReadDataSize=0!");

	data=GFL_HEAP_AllocMemory(heapID,size);

	FS_ReadFile(&p_file,data,size);									///<データをロード

	FS_CloseFile(&p_file);

	return data;
}

//============================================================================================
/**
 * アーカイブファイルデータロード
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 */
//============================================================================================
void	GFL_ARC_LoadData(void *data, int arcID, int datID)
{
	GF_ASSERT(ArchiveFileTable!=NULL);
	ArchiveLoadDataIndex(data, (char *)ArchiveFileTable[arcID], datID, OFS_NO_SET, SIZE_NO_SET);
}

//============================================================================================
/**
 * アーカイブファイルデータロード（読み込んだデータを格納するワークを関数内で確保するバージョン）
 *
 * ※関数内でワークをAllocするので、自前で開放処理をする必要があります
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 *
 * @retval	関数内で確保したデータ格納ワークのアドレス
 */
//============================================================================================
void*	GFL_ARC_LoadDataAlloc(int arcID, int datID, HEAPID heapID)
{
	GF_ASSERT(ArchiveFileTable!=NULL);
	return	ArchiveLoadDataIndexMalloc((char *)ArchiveFileTable[arcID],datID,heapID,OFS_NO_SET,SIZE_NO_SET);
}

//============================================================================================
/**
 * アーカイブファイルデータロード（オフセット指定あり）
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	size		読み込むデータサイズ
 */
//============================================================================================
void	GFL_ARC_LoadDataOfs(void *data, int arcID, int datID, int ofs, int size)
{
	GF_ASSERT(ArchiveFileTable!=NULL);
	ArchiveLoadDataIndex(data, (char *)ArchiveFileTable[arcID], datID, ofs, size);
}

//============================================================================================
/**
 * アーカイブファイルデータロード（読み込んだデータを格納するワークを関数内で確保するバージョンとオフセット指定あり）
 *
 * ※関数内でワークをAllocするので、自前で開放処理をする必要があります
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	size		読み込むデータサイズ
 *
 * @retval	関数内で確保したデータ格納ワークのアドレス
 */
//============================================================================================
void*	GFL_ARC_LoadDataAllocOfs(int arcID, int datID, HEAPID heapID, int ofs, int size)
{
	GF_ASSERT(ArchiveFileTable!=NULL);
	return	ArchiveLoadDataIndexMalloc((char *)ArchiveFileTable[arcID],datID,heapID,ofs,size);
}

//============================================================================================
/**
 *
 *	nnsarcで作成したアーカイブファイルに対して直接ファイル名を指定して任意のデータを取り出す
 *	読み込んだデータを格納するワークもこの関数内で確保して、ポインタを返す
 *
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	datID		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 *
 * @retval	関数内で確保したデータ格納ワークのアドレス
 */
//============================================================================================
void*	GFL_ARC_LoadDataFilePathAlloc(const char *name,int datID,HEAPID heapID)
{
	return	ArchiveLoadDataIndexMalloc(name,datID,heapID,OFS_NO_SET,SIZE_NO_SET);
}

//============================================================================================
/**
 * アーカイブファイルデータのファイル数を取得
 *
 * @param[in]	arcID	読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID	読み込むデータのアーカイブファイル上のインデックスナンバー
 *
 * @retval	アーカイブファイルデータのファイル数を取得
 */
//============================================================================================
u16	GFL_ARC_GetDataFileCnt(int arcID, int datID)
{
	FSFile		p_file;
	u32			size=0;
	u32			fat_top=0;
	u16			file_cnt=0;

	GF_ASSERT(ArchiveFileTable!=NULL);

	FS_InitFile(&p_file);
	FS_OpenFile(&p_file,(char *)ArchiveFileTable[arcID]);
	FS_SeekFile(&p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(&p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(&p_file,&file_cnt,2);								///<FileCountをロード
	FS_CloseFile(&p_file);

	return	file_cnt;
}

//============================================================================================
/**
 * アーカイブファイルデータのサイズを取得
 *
 *	ArchiveDataLoadを使用する時、確保するメモリサイズを取得するのに使用します
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 *
 * @retval	アーカイブファイルデータのサイズ
 */
//============================================================================================
u32	GFL_ARC_GetDataSize(int arcID,int datID)
{
	FSFile		p_file;
	u32			size=0;
	u32			fat_top=0;
	u32			fnt_top=0;
	u32			img_top=0;
	u32			top=0;
	u32			bottom=0;
	void		*data;
	u16			file_cnt=0;

	GF_ASSERT(ArchiveFileTable!=NULL);

	FS_InitFile(&p_file);
	FS_OpenFile(&p_file,(char *)ArchiveFileTable[arcID]);
	FS_SeekFile(&p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(&p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(&p_file,&file_cnt,2);								///<FileCountをロード
	GF_ASSERT_MSG(file_cnt>datID,"GFL_ARC_GetDataSize:FileCnt<datID");
	fnt_top=fat_top+size;
	FS_SeekFile(&p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(&p_file,fat_top+FAT_HEAD_SIZE+datID*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(&p_file,&top,4);									///<FATテーブルtopをロード
	FS_ReadFile(&p_file,&bottom,4);									///<FATテーブルbottomをロード

	FS_SeekFile(&p_file,img_top+IMG_HEAD_SIZE+top,FS_SEEK_SET);		///<取り出したいIMGの先頭に移動
	size=bottom-top;
	GF_ASSERT_MSG(size!=0,"GFL_ARC_GetDataSize:ReadDataSize=0!");
	FS_CloseFile(&p_file);

	return	size;
}

//--------------------------------------------------------
/**
 * アーカイブファイルのハンドル管理構造体
 */
//--------------------------------------------------------
struct _ARCHANDLE{
	FSFile  file;
	u32     fat_top;
	u32     img_top;
	u16		file_cnt;
};


//------------------------------------------------------------------
/**
 * アーカイブデータのファイルハンドルオープン
 *
 * @param   arcID		アーカイブデータインデックス
 * @param   heapID		管理用ヒープＩＤ
 *
 * @retval  ARCHANDLE	オープンされたハンドルのポインタ（失敗ならNULL）
 */
//------------------------------------------------------------------
ARCHANDLE* GFL_ARC_OpenDataHandle( u32 arcID, HEAPID heapID )
{
	ARCHANDLE* handle = GFL_HEAP_AllocMemory( heapID, sizeof(ARCHANDLE) );

	GF_ASSERT(ArchiveFileTable!=NULL);

	if( handle )
	{
		u32 fnt_top, tmp;

		// ２バイトを読み込んだ時にゴミが入らないようにしておく
		handle->fat_top = 0;

		FS_InitFile( &(handle->file) );
		FS_OpenFile( &(handle->file), (char *)ArchiveFileTable[arcID] );
		FS_SeekFile( &(handle->file), ARC_HEAD_SIZE_POS, FS_SEEK_SET );
		FS_ReadFile( &(handle->file), &(handle->fat_top), 2 );
		FS_SeekFile( &(handle->file), handle->fat_top+SIZE_OFFSET, FS_SEEK_SET );
		FS_ReadFile( &(handle->file), &tmp, 4 );
		FS_ReadFile( &(handle->file), &(handle->file_cnt), 2 );
		fnt_top = handle->fat_top + tmp;
		FS_SeekFile( &(handle->file), fnt_top+SIZE_OFFSET, FS_SEEK_SET );
		FS_ReadFile( &(handle->file), &tmp, 4 );
		handle->img_top = fnt_top + tmp;

	}
	return handle;
}

//------------------------------------------------------------------
/**
 * アーカイブデータのハンドルクローズ
 *
 * @param   handle			ハンドルポインタ
 *
 */
//------------------------------------------------------------------
void	GFL_ARC_CloseDataHandle( ARCHANDLE* handle )
{
	FS_CloseFile( &(handle->file) );
	GFL_HEAP_FreeMemory( handle );
}

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
void* GFL_ARC_LoadDataAllocByHandle( ARCHANDLE* handle, u32 datID, HEAPID heapID )
{
	u32 top, bottom;
	void* buf;

	GF_ASSERT_MSG(handle->file_cnt>datID, "DatCount=%d, DatID=%d", handle->file_cnt, datID);

	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datID*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	FS_ReadFile( &(handle->file), &bottom, 4 );
	FS_SeekFile( &(handle->file), handle->img_top+IMG_HEAD_SIZE+top, FS_SEEK_SET );

	buf = GFL_HEAP_AllocMemory(heapID, bottom-top);
	if( buf )
	{
		FS_ReadFile( &(handle->file), buf, bottom-top );
	}

	return buf;
}

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータ取得
 *
 * @param   handle		ハンドルポインタ
 * @param   datID		アーカイブ内のデータインデックス
 * @param   buffer		データ読み込み先バッファ
 *
 */
//------------------------------------------------------------------
void GFL_ARC_LoadDataByHandle( ARCHANDLE* handle, u32 datID, void* buffer )
{
	u32 top, bottom;
	
	GF_ASSERT_MSG(handle->file_cnt>datID, "DatCount=%d, DatID=%d", handle->file_cnt, datID);
	
	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datID*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	FS_ReadFile( &(handle->file), &bottom, 4 );
	FS_SeekFile( &(handle->file), handle->img_top+IMG_HEAD_SIZE+top, FS_SEEK_SET );
	
	FS_ReadFile( &(handle->file), buffer, bottom-top );
}

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータサイズ取得
 *
 * @param   handle		ハンドルポインタ
 * @param   datID		アーカイブ内のデータインデックス
 * @retval	u32			インデックスのデータサイズ
 *
 */
//------------------------------------------------------------------
u32 GFL_ARC_GetDataSizeByHandle( ARCHANDLE* handle, u32 datID )
{
	u32 top, bottom;
	
	GF_ASSERT_MSG(handle->file_cnt>datID, "DatCount=%d, DatID=%d", handle->file_cnt, datID);
	
	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datID*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	FS_ReadFile( &(handle->file), &bottom, 4 );
	
	return( bottom-top );
}

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってデータ取得（読み込み開始オフセット＆サイズ指定）
 *
 * @param   handle		ハンドルポインタ
 * @param   datID		アーカイブ内のデータインデックス
 * @param   ofs			読み込み開始オフセット
 * @param   size		読み込みサイズ
 * @param   buffer		データ読み込み先バッファ
 *
 */
//------------------------------------------------------------------
void GFL_ARC_LoadDataOfsByHandle( ARCHANDLE* handle, u32 datID, u32 ofs, u32 size, void* buffer )
{
	u32 top;

	GF_ASSERT_MSG(handle->file_cnt>datID, "DatCount=%d, DatID=%d", handle->file_cnt, datID);

	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datID*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	FS_SeekFile( &(handle->file), handle->img_top+IMG_HEAD_SIZE+top+ofs, FS_SEEK_SET );

	FS_ReadFile( &(handle->file), buffer, size );
}

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを使ってイメージデータオフセット取得
 *
 * @param   handle		ハンドルポインタ
 * @param   datID		アーカイブ内のデータインデックス
 * @param   offset		オフセット読み込み先バッファ
 *
 */
//------------------------------------------------------------------
void GFL_ARC_LoadDataImgofsByHandle( ARCHANDLE* handle, u32 datID, u32* offset )
{
	u32 top;

	GF_ASSERT_MSG(handle->file_cnt>datID, "DatCount=%d, DatID=%d", handle->file_cnt, datID);

	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datID*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	
	*offset = handle->img_top+IMG_HEAD_SIZE+top;
}

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
void GFL_ARC_LoadDataByHandleContinue( ARCHANDLE* handle, u32 size, void* buffer )
{
	FS_ReadFile( &(handle->file), buffer, size );
}

//------------------------------------------------------------------
/**
 * アーカイブデータハンドルを介してファイルシーク
 *
 * @param   handle		ハンドルポインタ
 * @param   size
 *
 */
//------------------------------------------------------------------
void GFL_ARC_SeekDataByHandle( ARCHANDLE* handle, u32 size )
{
	FS_SeekFile( &(handle->file), size, FS_SEEK_SET );
}

//============================================================================================
/**
 * アーカイブファイルデータのファイル数を取得(ハンドル使用)
 *
 * @param	handle			ハンドルポインタ
 * @retval	u16				ファイル数	
 */
//============================================================================================
u16		GFL_ARC_GetDataFileCntByHandle(ARCHANDLE* handle)
{
	return	handle->file_cnt;
}

//============================================================================================
/**
 * アーカイブファイルデータをオープンして、FSFile構造体のtopメンバをimg_topで上書きする
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[out]	p_file		データを書き込むFSFile構造体のポインタ
 */
//============================================================================================
void	GFL_ARC_OpenFileTopPosWrite(int arcID,int datID,FSFile *p_file)
{
	u32			size=0;
	u32			fat_top=0;
	u32			fnt_top=0;
	u32			img_top=0;
	u32			top=0;
	u16			file_cnt=0;

	FS_InitFile(p_file);
	FS_OpenFile(p_file,(char *)ArchiveFileTable[arcID]);
	FS_SeekFile(p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(p_file,&file_cnt,2);								///<FileCountをロード
	GF_ASSERT_MSG(file_cnt>datID,"ArchiveLoadDataIndex fileCnt=%d, datID=%d", file_cnt, datID);
	fnt_top=fat_top+size;
	FS_SeekFile(p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(p_file,fat_top+FAT_HEAD_SIZE+datID*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(p_file,&top,4);										///<FATテーブルtopをロード

	p_file->prop.file.top+=img_top+IMG_HEAD_SIZE+top;				///<取り出したいIMGの先頭に移動
}

