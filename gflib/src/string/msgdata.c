//============================================================================================
/**
 * @file	msgdata.c
 * @brief	各種メッセージデータ取り扱い
 * @author	taya
 */
//============================================================================================
#include "gflib.h"
#include "strbuf.h"
#include "msgdata.h"

// このdefineを有効にすると、暗号化メッセージ対応
#define  MSGDATA_CODED	

/*------------------------------------------------------------*/
/** 文字列パラメータ                                          */
/*------------------------------------------------------------*/
typedef struct {
	u32  offset;	///< ファイル先頭からのバイトオフセット
	u32  len;		///< 文字数（EOMを含む）
}MSG_PARAM_BLOCK;

/*------------------------------------------------------------*/
/** 文字列データヘッダ                                        */
/*------------------------------------------------------------*/
struct _MSGDATA_HEADER{
	#ifdef MSGDATA_CODED
	u16					numMsgs;
	u16					randValue;
	#else
	union {
		u32				numMsgs;
		u32				randValue;
	};
	#endif
	MSG_PARAM_BLOCK		params[];
};


//===============================================================================================
// インライン関数
//===============================================================================================
//------------------------------------------------------------------
/**
 * 文字列ごとのパラメータ構造体オフセットを計算
 *
 * @param   strID		
 *
 * @retval  inline int		
 */
//------------------------------------------------------------------
static inline int CalcParamBlockOfs( u32 strID )
{
	return sizeof(MSGDATA_HEADER) + sizeof(MSG_PARAM_BLOCK) * strID;
}


//------------------------------------------------------------------
/**
 * 暗号化された文字列パラメータ構造体を復号する
 *
 * @param   param		パラメータ構造体へのポインタ
 * @param   strID		文字列ID
 * @param   rand		暗号キー乱数
 */
//------------------------------------------------------------------
static inline void DecodeParam( MSG_PARAM_BLOCK* param, u32 strID, u32 rand )
{
#ifdef MSGDATA_CODED
	rand = (rand * 765 * (strID+1)) & 0xffff;
	rand |= (rand << 16);

	param->offset ^= rand;
	param->len ^= rand;
#endif
}


//------------------------------------------------------------------
/**
 * 暗号化された文字列を復号する
 *
 * @param   str			文字列ポインタ
 * @param   len			文字列長
 * @param   strID		文字列ID
 * @param   rand		暗号キー乱数
 *
 */
//------------------------------------------------------------------
static inline void DecodeStr( STRCODE* str, u32 len, u32 strID, u16 rand )
{
#ifdef MSGDATA_CODED
	rand = (strID + 1) * 596947;
	while(len--)
	{
		*str ^= rand;
		str++;
		rand += 18749;
	}
#endif
}


//------------------------------------------------------------------
/**
 * コンバートされている文字列を指定のローカルバッファにコピー
 *
 * @param   dst			コピー先バッファ
 * @param   src			コピー元文字列
 * @param   param		コピー元文字列のパラメータ
 *
 */
//------------------------------------------------------------------
static inline void StrSet( STRCODE* dst, const STRCODE* src, const MSG_PARAM_BLOCK* param )
{
	GFL_STD_MemCopy16( src, dst, param->len*sizeof(STRCODE) );
}


/*============================================================================================*/
/*                                                                                            */
/*                                                                                            */
/*  メッセージデータのコントロール                                                            */
/*                                                                                            */
/*                                                                                            */
/*============================================================================================*/
//------------------------------------------------------------------
/**
 * メッセージデータをアーカイブからロードする
 *
 * @param   arcID		アーカイブファイルＩＤ
 * @param   datID		アーカイブファイル内のメッセージデータＩＤ
 * @param   heapID		ヒープＩＤ
 *
 * @retval  MSGDATA_HEADER*		メッセージデータポインタ
 */
//------------------------------------------------------------------
MSGDATA_HEADER*
	GFL_MSG_DataLoad
		( u32 arcID, u32 datID, u32 heapID )
{
#if 0
//	return ArchiveDataLoadMalloc( arcID, datID, heapID );
#else
	return NULL;
#endif
}


//------------------------------------------------------------------
/**
 * ロードしたメッセージデータをアンロードする
 *
 * @param   msgdat		メッセージデータポインタ
 *
 */
//------------------------------------------------------------------
void
	GFL_MSG_DatUnload
		( MSGDATA_HEADER* msgdat )
{
	GFL_HEAP_FreeMemory( msgdat );
}


//------------------------------------------------------------------
/**
 * 文字列バッファ型オブジェクトに、指定文字列を取得
 *
 * @param   msgdat		メッセージデータポインタ
 * @param   strID		文字列ID
 * @param   dst			文字列バッファ型
 *
 */
//------------------------------------------------------------------
void
	GFL_MSG_GetStr
		( const MSGDATA_HEADER* msgdat, u32 strID, STRBUF* dst )
{
	if( strID < msgdat->numMsgs )
	{
		MSG_PARAM_BLOCK param;
		STRCODE* str;
		u32 size;

		param = msgdat->params[strID];
		DecodeParam( &param, strID, msgdat->randValue );

		size = param.len * sizeof(STRCODE);
		str = GFL_HEAP_AllocMemory( HeapGetLow( GFL_HEAPID_SYSTEM ), size );
		if( str )
		{
			GFL_STD_MemCopy16( (((u8*)msgdat) + param.offset), str, size );
			DecodeStr( str, param.len, strID, msgdat->randValue );
			GFL_STR_SetStringCodeOrderLength( dst, str, param.len );
			GFL_HEAP_FreeMemory( str );
		}
	}
	else
	{
		GF_ASSERT_MSG(0, "strID:%d", strID);
		GFL_STR_BufferClear( dst );
	}
}


//------------------------------------------------------------------
/**
 * 文字列バッファ型オブジェクトをメモリ確保し、指定文字列を取得
 *
 * @param   msgdat		メッセージデータポインタ
 * @param   strID		文字列ID
 * @param   heapID		メモリ確保用のヒープID
 *
 * @retval  文字列がコピーされた文字列バッファ型オブジェクトへのポインタ
 *
 */
//------------------------------------------------------------------
STRBUF*
	GFL_MSG_GetStrAlloc
	( const MSGDATA_HEADER* msgdat, u32 strID, u32 heapID )
{
	if( strID < msgdat->numMsgs )
	{
		MSG_PARAM_BLOCK param;
		STRCODE* str;
		u32 size;

		param = msgdat->params[strID];
		DecodeParam( &param, strID, msgdat->randValue );
		size = param.len * sizeof(STRCODE);
		str = GFL_HEAP_AllocMemory( HeapGetLow( heapID) , size );
		if( str )
		{
			STRBUF *dst;

			GFL_STD_MemCopy16( (((u8*)msgdat) + param.offset), str, size );
			DecodeStr( str, param.len, strID, msgdat->randValue );

			dst = GFL_STR_BufferCreate( param.len, heapID );
			if( dst )
			{
				GFL_STR_SetStringCodeOrderLength( dst, str, param.len );
			}
			GFL_HEAP_FreeMemory( str );
			return dst;
		}
		return NULL;
	}
	else
	{
		// 存在しないインデックスの要求が来たら空文字列を返している
		GF_ASSERT_MSG(0, "strID:%d", strID);
		return GFL_STR_BufferCreate( 4, heapID );
	}
}


//------------------------------------------------------------------
/**
 * 指定文字列のみをロード＆指定バッファにコピー（アーカイブID指定）
 *
 * @param   arcID		アーカイブファイルＩＤ
 * @param   datID		アーカイブファイル内のメッセージデータＩＤ
 * @param   msgID		メッセージデータ内の文字列ＩＤ
 * @param   heapID		一時メモリ確保用のヒープＩＤ
 * @param   dst			文字列コピー先バッファ
 *
 */
//------------------------------------------------------------------
void
	GFL_MSG_GetStrDirect
		( u32 arcID, u32 datID, u32 strID, u32 heapID, STRBUF* dst )
{
#if 0
	ARCHANDLE*  arc;

	arc = ArchiveDataHandleOpen(arcID, heapID);
	if( arc )
	{
		MSGDAT_GetStrDirectByHandle( arc, datID, strID, heapID, dst );
		ArchiveDataHandleClose( arc );
	}
#endif
}


//------------------------------------------------------------------
/**
 * 指定文字列のみをロード＆指定バッファにコピー（アーカイブハンドル使用）
 *
 * @param   arcHandle	アーカイブハンドル
 * @param   datID		アーカイブファイル内のメッセージデータＩＤ
 * @param   msgID		メッセージデータ内の文字列ＩＤ
 * @param   heapID		一時メモリ確保用のヒープＩＤ
 * @param   dst			文字列コピー先バッファ
 *
 */
//------------------------------------------------------------------
#if 0
void
	GFL_MSG_GetStrDirectByHandle
		( ARCHANDLE* arcHandle, u32 datID, u32 strID, u32 heapID, STRBUF* dst )
{
	MSGDATA_HEADER  header;
	MSG_PARAM_BLOCK  param;
	STRCODE*  str;
	u32 size;

	ArchiveDataLoadOfsByHandle( arcHandle, datID, 0, sizeof(MSGDATA_HEADER), &header );

	if( strID < header.numMsgs )
	{
		ArchiveDataLoadOfsByHandle( arcHandle, datID, CalcParamBlockOfs(strID),
									sizeof(MSG_PARAM_BLOCK), &param );
		DecodeParam( &param, strID, header.randValue );

		size = param.len * sizeof(STRCODE);
		str = sys_AllocMemoryLo( heapID, size );
		if( str )
		{
			ArchiveDataLoadOfsByHandle( arcHandle, datID, param.offset, size, str );
			DecodeStr( str, param.len, strID, header.randValue );
			STRBUF_SetStringCodeOrderLength( dst, str, param.len );
			sys_FreeMemoryEz( str );
		}
	}
	else
	{
		GF_ASSERT_MSG(0, "datID:%d strID:%d", datID, strID);
		STRBUF_Clear( dst );
	}
}
#endif


//------------------------------------------------------------------
/**
 * 指定文字列のみをロード後、内部でアロケートしたメモリにコピーして返す
 *（アーカイブID指定）
 *
 * @param   arcID		アーカイブファイルＩＤ
 * @param   datID		アーカイブファイル内のメッセージデータＩＤ
 * @param   msgID		メッセージデータ内の文字列ＩＤ
 * @param   heapID		一時メモリ確保用のヒープＩＤ
 *
 * @retval  文字列がコピーされた文字列バッファ型オブジェクトへのポインタ
 */
//------------------------------------------------------------------
STRBUF*
	GFL_MSG_GetStrDirectAlloc
		( u32 arcID, u32 datID, u32 strID, u32 heapID )
{
#if 0
	ARCHANDLE*  arc;
	STRBUF* ret;

	arc = ArchiveDataHandleOpen(arcID, heapID);
	if( arc )
	{
		ret = MSGDAT_GetStrDirectAllocByHandle( arc, datID, strID, heapID );
		ArchiveDataHandleClose( arc );
	}
	else
	{
		ret = STRBUF_Create( 4, heapID );
	}
	return ret;
#else
	return NULL;
#endif
}


//------------------------------------------------------------------
/**
 * 指定文字列のみをロード後、内部でアロケートしたメモリにコピーして返す
 *（アーカイブハンドル使用）
 *
 * @param   arcHandle	アーカイブハンドル
 * @param   datID		アーカイブファイル内のメッセージデータＩＤ
 * @param   msgID		メッセージデータ内の文字列ＩＤ
 * @param   heapID		一時メモリ確保用のヒープＩＤ
 *
 * @retval  文字列がコピーされた文字列バッファ型オブジェクトへのポインタ
 */
//------------------------------------------------------------------
#if 0
STRBUF*
	GFL_MSG_GetStrDirectAllocByHandle
		( ARCHANDLE* arcHandle, u32 datID, u32 strID, u32 heapID )
{
	MSGDATA_HEADER   header;

	ArchiveDataLoadOfsByHandle( arcHandle, datID, 0, sizeof(MSGDATA_HEADER), &header );

	if( strID < header.numMsgs )
	{
		MSG_PARAM_BLOCK  param;
		STRBUF*   dst;

		ArchiveDataLoadOfsByHandle( arcHandle, datID, CalcParamBlockOfs(strID),
									sizeof(MSG_PARAM_BLOCK), &param );
		DecodeParam( &param, strID, header.randValue );

		dst = STRBUF_Create( param.len, heapID );
		if( dst )
		{
			STRCODE*  str;
			u32 size;

			size = param.len * sizeof(STRCODE);
			str = sys_AllocMemoryLo( heapID, size );
			if( str )
			{
				ArchiveDataLoadOfsByHandle( arcHandle, datID, param.offset, size, str );
				DecodeStr( str, param.len, strID, header.randValue );
				STRBUF_SetStringCodeOrderLength( dst, str, param.len );
				sys_FreeMemoryEz( str );
			}
		}
		return dst;
	}
	else
	{
		// 存在しないインデックスの要求が来たら空文字列を返している
		GF_ASSERT_MSG(0, "datID:%d strID:%d", datID, strID);
		return STRBUF_Create( 4, heapID );
	}
}
#endif


//------------------------------------------------------------------
/**
 * ロードしたメッセージデータを基に、格納されているメッセージの数を返す
 *
 * @param   msgdat		メッセージデータポインタ
 *
 * @retval  u32			メッセージ数
 */
//------------------------------------------------------------------
u32
	GFL_MSG_GetMessageCount
		( const MSGDATA_HEADER* msgdat )
{
	return msgdat->numMsgs;
}


//------------------------------------------------------------------
/**
 * アーカイブ＆データIDを基に、格納されているメッセージの数を返す
 *
 * @param   arcID		アーカイブID
 * @param   datID		データID
 *
 * @retval  u32			メッセージ数
 */
//------------------------------------------------------------------
u32
	GFL_MSG_GetMessageCountDirect
		( u32 arcID, u32 datID )
{
#if 0
	MSGDATA_HEADER  header;
	ArchiveDataLoadOfs( &header, arcID, datID, 0, sizeof(MSGDATA_HEADER) );
	return header.numMsgs;
#else
	return 0;
#endif
}




#if 0
/*============================================================================================*/
/*                                                                                            */
/*                                                                                            */
/*  メッセージデータマネージャ                                                                */
/*                                                                                            */
/*                                                                                            */
/*============================================================================================*/
//----------------------------------------------------
/**
 *  マネージャワーク構造体
 */
//----------------------------------------------------
struct _MSGDATA_MANAGER {

	u16		type;
	u16		heapID;
	u16		arcID;
	u16		datID;

	union {
		MSGDATA_HEADER*  msgData;
		ARCHANDLE*       arcHandle;
	};
};


//------------------------------------------------------------------
/**
 * メッセージデータマネージャ作成
 *
 * @param   type		マネージャタイプ指定
 * @param   arcID		メッセージデータの含まれるアーカイブＩＤ
 * @param   datID		メッセージデータ本体のアーカイブ内ＩＤ
 * @param   heapID		メモリ確保用ヒープＩＤ
 *
 * @retval  MSGDATA_MANAGER*		作成したマネージャワークポインタ
 */
//------------------------------------------------------------------
MSGDATA_MANAGER*  MSGMAN_Create( MSGMAN_TYPE type, u32 arcID, u32 datID, u32 heapID )
{
	// マネージャ作成→文字列取得→マネージャ廃棄…の流れが思ったより多そうなので
	// マネージャワークはメモリブロックの後方から取得する
	MSGDATA_MANAGER* man = sys_AllocMemoryLo( heapID, sizeof(MSGDATA_MANAGER) );
	if( man )
	{
		if( type == MSGMAN_TYPE_NORMAL )
		{
			man->msgData = MSGDAT_Load( arcID, datID, heapID );
			if( man->msgData == NULL )
			{
				sys_FreeMemoryEz( man );
				return NULL;
			}
		}
		else
		{
			man->arcHandle = ArchiveDataHandleOpen(arcID, heapID);
		}
		man->type = type;
		man->arcID = arcID;
		man->datID = datID;
		man->heapID = heapID;
	}
	return man;
}


//------------------------------------------------------------------
/**
 * メッセージデータマネージャの破棄
 *
 * @param   man		マネージャワークポインタ
 *
 */
//------------------------------------------------------------------
void MSGMAN_Delete( MSGDATA_MANAGER* man )
{
	if( man )
	{
		switch( man->type ){
		case MSGMAN_TYPE_NORMAL:
			MSGDAT_Unload( man->msgData );
			break;
		case MSGMAN_TYPE_DIRECT:
			ArchiveDataHandleClose( man->arcHandle );
			break;
		}
		sys_FreeMemoryEz( man );
	}
}


//------------------------------------------------------------------
/**
 * メッセージデータマネージャを使って文字列を指定バッファにコピー
 *
 * @param   man			マネージャワークポインタ
 * @param   strID		文字列ＩＤ
 * @param   dst			コピー先バッファポインタ
 *
 */
//------------------------------------------------------------------
void MSGMAN_GetString( const MSGDATA_MANAGER* man, u32 strID, STRBUF* dst )
{
	switch( man->type ){
	case MSGMAN_TYPE_NORMAL:
		MSGDAT_GetStr( man->msgData, strID, dst );
		break;

	case MSGMAN_TYPE_DIRECT:
		MSGDAT_GetStrDirectByHandle( man->arcHandle, man->datID, strID, man->heapID, dst );
		break;
	}
}


//------------------------------------------------------------------
/**
 * メッセージデータマネージャを使って文字列を取得
 *（内部でSTRBUFを作成して返す。作成されたSTRBUFの破棄は各自で行う）
 *
 * @param   man			マネージャワークポインタ
 * @param   strID		文字列ID
 *
 * @retval  STRBUF*		コピー先バッファポインタ
 */
//------------------------------------------------------------------
STRBUF* MSGMAN_AllocString( const MSGDATA_MANAGER* man, u32 strID )
{
	switch( man->type ){
	case MSGMAN_TYPE_NORMAL:
		return MSGDAT_GetStrAlloc( man->msgData, strID, man->heapID );

	case MSGMAN_TYPE_DIRECT:
		return MSGDAT_GetStrDirectAllocByHandle( man->arcHandle, man->datID, strID, man->heapID );
	}
	return NULL;
}


//------------------------------------------------------------------
/**
 * 格納されているメッセージ数を返す
 *
 * @param   man		マネージャワークポインタ
 *
 * @retval  u32		メッセージ数
 */
//------------------------------------------------------------------
u32 MSGMAN_GetMessageCount( const MSGDATA_MANAGER* man )
{
	switch( man->type ){
	case MSGMAN_TYPE_NORMAL:
		return MSGDAT_GetMessageCount( man->msgData );

	case MSGMAN_TYPE_DIRECT:
		return MSGDAT_GetMessageCountDirect( man->arcID, man->datID );
	}
	return 0;
}

#endif

