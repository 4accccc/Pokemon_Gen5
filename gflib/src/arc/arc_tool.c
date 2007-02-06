
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

static	void	ArchiveDataLoadIndex(void *data,const char *name,int index,int ofs,int ofs_size);
static	void	*ArchiveDataLoadIndexMalloc(const char *name,int index,HEAPID heapID,int ofs,int ofs_size);

void	GFL_ARC_sysInit(const char **tbl,int tbl_max);
void	GFL_ARC_sysExit(void);

void	GFL_ARC_DataLoad(void *data,int arcID,int datID);
void	*GFL_ARC_DataLoadMalloc(int arcID,int datID,HEAPID heapID);
void	GFL_ARC_DataLoadOfs(void *data,int arcID,int datID,int ofs,int size);
void	*GFL_ARC_DataLoadMallocOfs(int arcID,int datID,HEAPID heapID,int ofs,int size);
void	*GFL_ARC_DataLoadFilePathMalloc(const char *name,int datID,HEAPID heapID);
u16		GFL_ARC_DataFileCntGet(int arcID,int datID);
u32		GFL_ARC_DataSizeGet(int arcID,int datID);

#define	ALLOC_HEAD	(0)		//領域を確保するとき先頭から確保
#define	ALLOC_TAIL	(1)		//領域を確保するとき後ろから確保

//============================================================================================
//	アーカイブテーブル格納変数
//============================================================================================

static	char	***ArchiveFileTable=NULL;
static	int		ArchiveFileTableMax=0;

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
void	GFL_ARC_sysInit(const char **tbl,int tbl_max)
{
	ArchiveFileTable=(char ***)tbl;
	ArchiveFileTableMax=tbl_max;
}

//============================================================================================
/**
 *
 *	アーカイブシステム終了
 *
 */
//============================================================================================
void	GFL_ARC_sysExit(void)
{
}

//============================================================================================
/**
 *
 *	nnsarcで作成したアーカイブファイルに対してIndexを指定して任意のデータを取り出す
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	index		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	ofs_size	読み込むデータサイズ
 *
 */
//============================================================================================
static	void	ArchiveDataLoadIndex(void *data,const char *name,int index,int ofs,int ofs_size)
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
	GF_ASSERT_MSG(file_cnt>index,"ServerArchiveDataLoadIndex file=%s, fileCnt=%d, index=%d", name, file_cnt, index);
	fnt_top=fat_top+size;
	FS_SeekFile(&p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(&p_file,fat_top+FAT_HEAD_SIZE+index*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(&p_file,&top,4);									///<FATテーブルtopをロード
	FS_ReadFile(&p_file,&bottom,4);									///<FATテーブルbottomをロード

	FS_SeekFile(&p_file,img_top+IMG_HEAD_SIZE+top+ofs,FS_SEEK_SET);	///<取り出したいIMGの先頭に移動
	
	if(ofs_size){
		size=ofs_size;
	}
	else{
		size=bottom-top;
	}

	GF_ASSERT_MSG(size!=0,"ServerArchiveDataLoadIndex:ReadDataSize=0!");
	FS_ReadFile(&p_file,data,size);									///<データをロード

	FS_CloseFile(&p_file);
}


//============================================================================================
/**
 *
 *	nnsarcで作成したアーカイブファイルに対してIndexを指定して任意のデータを取り出す
 *	読み込んだデータを格納するワークもこの関数内で確保して、ポインタを返す
 *
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	index		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	ofs_size	読み込むデータサイズ
 *
 */
//============================================================================================
static	void	*ArchiveDataLoadIndexMalloc(const char *name,int index,HEAPID heapID,int ofs,int ofs_size)
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
	GF_ASSERT_MSG(file_cnt>index,"ServerArchiveDataLoadIndex file=%s, fileCnt=%d, index=%d", name, file_cnt, index);
	fnt_top=fat_top+size;
	FS_SeekFile(&p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(&p_file,fat_top+FAT_HEAD_SIZE+index*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(&p_file,&top,4);									///<FATテーブルtopをロード
	FS_ReadFile(&p_file,&bottom,4);									///<FATテーブルbottomをロード

	FS_SeekFile(&p_file,img_top+IMG_HEAD_SIZE+top+ofs,FS_SEEK_SET);		///<取り出したいIMGの先頭に移動

	if(ofs_size){
		size=ofs_size;
	}
	else{
		size=bottom-top;
	}
	GF_ASSERT_MSG(size!=0,"ServerArchiveDataLoadIndex:ReadDataSize=0!");

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
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー（arc_tool.hに記述）
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 */
//============================================================================================
void	GFL_ARC_DataLoad(void *data, int arcID, int datID)
{
	ArchiveDataLoadIndex(data, (char *)ArchiveFileTable[arcID], datID, OFS_NO_SET, SIZE_NO_SET);
}

//============================================================================================
/**
 * アーカイブファイルデータロード（読み込んだデータを格納するワークを関数内で確保するバージョン）
 *
 * ※関数内でワークをAllocするので、自前で開放処理をする必要があります(sys_FreeMemoryEz(領域ポインタ)で開放）
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー（arc_tool.hに記述）
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 *
 * @retval	関数内で確保したデータ格納ワークのアドレス
 */
//============================================================================================
void* GFL_ARC_DataLoadMalloc(int arcID, int datID, HEAPID heapID)
{
	return	ArchiveDataLoadIndexMalloc((char *)ArchiveFileTable[arcID],datID,heapID,OFS_NO_SET,SIZE_NO_SET);
}

//============================================================================================
/**
 * アーカイブファイルデータロード（オフセット指定あり）
 *
 * @param[in]	data		読み込んだデータを格納するワークのポインタ
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー（arc_tool.hに記述）
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	size		読み込むデータサイズ
 */
//============================================================================================
void GFL_ARC_DataLoadOfs(void *data, int arcID, int datID, int ofs, int size)
{
	ArchiveDataLoadIndex(data, (char *)ArchiveFileTable[arcID], datID, ofs, size);
}

//============================================================================================
/**
 * アーカイブファイルデータロード（読み込んだデータを格納するワークを関数内で確保するバージョンとオフセット指定あり）
 *
 * ※関数内でワークをAllocするので、自前で開放処理をする必要があります(sys_FreeMemoryEz(領域ポインタ)で開放）
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー（arc_tool.hに記述）
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 * @param[in]	ofs			読み込むデータの先頭からのオフセット
 * @param[in]	size		読み込むデータサイズ
 *
 * @retval	関数内で確保したデータ格納ワークのアドレス
 */
//============================================================================================
void* GFL_ARC_DataLoadMallocOfs(int arcID, int datID, HEAPID heapID, int ofs, int size)
{
	return	ArchiveDataLoadIndexMalloc((char *)ArchiveFileTable[arcID],datID,heapID,ofs,size);
}

//============================================================================================
/**
 *
 *	nnsarcで作成したアーカイブファイルに対して直接ファイル名を指定して任意のデータを取り出す
 *	読み込んだデータを格納するワークもこの関数内で確保して、ポインタを返す
 *
 * @param[in]	name		読み込むアーカイブファイル名
 * @param[in]	index		読み込むデータのアーカイブ上のインデックスナンバー
 * @param[in]	heapID		メモリを確保するヒープ領域のID
 *
 */
//============================================================================================
void	*GFL_ARC_DataLoadFilePathMalloc(const char *name,int datID,HEAPID heapID)
{
	return	ArchiveDataLoadIndexMalloc(name,datID,heapID,OFS_NO_SET,SIZE_NO_SET);
}

//============================================================================================
/**
 * アーカイブファイルデータのファイル数を取得
 *
 * @param[in]	arcID	読み込むアーカイブファイルの種類インデックスナンバー（arc_tool.hに記述）
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 */
//============================================================================================
u16		GFL_ARC_DataFileCntGet(int arcID, int datID)
{
	FSFile		p_file;
	u32			size=0;
	u32			fat_top=0;
	u16			file_cnt=0;

	FS_InitFile(&p_file);
	FS_OpenFile(&p_file,(char *)ArchiveFileTable[arcID]);
	FS_SeekFile(&p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(&p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(&p_file,&file_cnt,2);								///<FileCountをロード

	return	file_cnt;
}

//============================================================================================
/**
 * アーカイブファイルデータのサイズを取得
 *
 *	ArchiveDataLoadを使用する時、確保するメモリサイズを取得するのに使用します
 *
 * @param[in]	arcID		読み込むアーカイブファイルの種類インデックスナンバー（arc_tool.hに記述）
 * @param[in]	datID		読み込むデータのアーカイブファイル上のインデックスナンバー
 */
//============================================================================================
u32		GFL_ARC_DataSizeGet(int arcID,int datID)
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
	FS_OpenFile(&p_file,(char *)ArchiveFileTable[arcID]);
	FS_SeekFile(&p_file,ARC_HEAD_SIZE_POS,FS_SEEK_SET);				///<アーカイブヘッダのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,2);									///<アーカイブヘッダサイズをロード
	fat_top=size;
	FS_SeekFile(&p_file,fat_top+SIZE_OFFSET,FS_SEEK_SET);			///<FATのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FATサイズをロード
	FS_ReadFile(&p_file,&file_cnt,2);								///<FileCountをロード
	GF_ASSERT_MSG(file_cnt>datID,"ServerArchiveDataLoadIndex:FileCnt<Index");
	fnt_top=fat_top+size;
	FS_SeekFile(&p_file,fnt_top+SIZE_OFFSET,FS_SEEK_SET);			///<FNTのサイズ格納位置に移動
	FS_ReadFile(&p_file,&size,4);									///<FNTサイズをロード
	img_top=fnt_top+size;
	
	FS_SeekFile(&p_file,fat_top+FAT_HEAD_SIZE+datID*8,FS_SEEK_SET);	///<取り出したいFATテーブルに移動
	FS_ReadFile(&p_file,&top,4);									///<FATテーブルtopをロード
	FS_ReadFile(&p_file,&bottom,4);									///<FATテーブルbottomをロード

	FS_SeekFile(&p_file,img_top+IMG_HEAD_SIZE+top,FS_SEEK_SET);		///<取り出したいIMGの先頭に移動
	size=bottom-top;
	GF_ASSERT_MSG(size!=0,"ServerArchiveDataLoadIndex:ReadDataSize=0!");

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
ARCHANDLE* GFL_ARC_DataHandleOpen( u32 arcID, HEAPID heapID )
{
	ARCHANDLE* handle = GFL_HEAP_AllocMemory( heapID, sizeof(ARCHANDLE) );
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
void	GFL_ARC_DataHandleClose( ARCHANDLE* handle )
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
void* GFL_ARC_DataLoadAllocByHandle( ARCHANDLE* handle, u32 datId, HEAPID heapID )
{
	u32 top, bottom;
	void* buf;

	GF_ASSERT_MSG(handle->file_cnt>datId, "DatCount=%d, DatID=%d", handle->file_cnt, datId);

	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datId*8, FS_SEEK_SET );
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
 * @param   datId		アーカイブ内のデータインデックス
 * @param   buffer		データ読み込み先バッファ
 *
 */
//------------------------------------------------------------------
void GFL_ARC_DataLoadByHandle( ARCHANDLE* handle, u32 datId, void* buffer )
{
	u32 top, bottom;
	
	GF_ASSERT_MSG(handle->file_cnt>datId, "DatCount=%d, DatID=%d", handle->file_cnt, datId);
	
	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datId*8, FS_SEEK_SET );
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
 * @param   datId		アーカイブ内のデータインデックス
 * @retval	u32			インデックスのデータサイズ
 *
 */
//------------------------------------------------------------------
u32 GFL_ARC_DataSizeGetByHandle( ARCHANDLE* handle, u32 datId )
{
	u32 top, bottom;
	
	GF_ASSERT_MSG(handle->file_cnt>datId, "DatCount=%d, DatID=%d", handle->file_cnt, datId);
	
	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datId*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	FS_ReadFile( &(handle->file), &bottom, 4 );
	
	return( bottom-top );
}

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
void GFL_ARC_DataLoadOfsByHandle( ARCHANDLE* handle, u32 datId, u32 ofs, u32 size, void* buffer )
{
	u32 top;

	GF_ASSERT_MSG(handle->file_cnt>datId, "DatCount=%d, DatID=%d", handle->file_cnt, datId);

	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datId*8, FS_SEEK_SET );
	FS_ReadFile( &(handle->file), &top, 4 );
	FS_SeekFile( &(handle->file), handle->img_top+IMG_HEAD_SIZE+top+ofs, FS_SEEK_SET );

	FS_ReadFile( &(handle->file), buffer, size );
}

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
void GFL_ARC_DataLoadImgofsByHandle( ARCHANDLE* handle, u32 datId, u32* offset )
{
	u32 top;

	GF_ASSERT_MSG(handle->file_cnt>datId, "DatCount=%d, DatID=%d", handle->file_cnt, datId);

	FS_SeekFile( &(handle->file), handle->fat_top+FAT_HEAD_SIZE+datId*8, FS_SEEK_SET );
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
void GFL_ARC_DataLoadByHandleContinue( ARCHANDLE* handle, u32 size, void* buffer )
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
void GFL_ARC_DataSeekByHandle( ARCHANDLE* handle, u32 size )
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
u16		GFL_ARC_DataFileCntGetByHandle(ARCHANDLE* handle)
{
	return	handle->file_cnt;
}
