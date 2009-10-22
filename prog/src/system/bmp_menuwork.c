//======================================================================
/**
 * @file	bmp_menu_list.h
 * @brief	bmp_menu と bmp_list で使用する文字列指定構造体の処理
 * @author	taya
 * @date	2005.12.02
 *
 * bmp_menu と bmp_list で全く同じ型・同じ用途の構造体を別個に定義しているのでまとめておく。
 *
 *	・2008.09 DPからWBへ移植 元ファイル bmp_menu_list.c,h
 *
 */
//======================================================================
#include "system/bmp_menuwork.h"

//==============================================================
// Prototype
//==============================================================
static BMP_MENULIST_DATA * SeekEmptyListPos(
		BMP_MENULIST_DATA* list, u32* heapID );

/// リストの終端コード
#define LIST_ENDCODE		((const void*)0xffffffff)

//======================================================================
//	関数
//======================================================================
//------------------------------------------------------------------
/**
 * リストバッファを作成する。
 * 作成後、BMP_MENULIST_AddArchiveString か
 * BMP_MENULIST_AddString を使って文字列をセットする
 *
 * @param   maxElems		リストに登録する文字列の最大件数
 * @param   heapID			リスト作成先ヒープID
 *
 * @retval  BMP_MENULIST_DATA*		作成されたリストバッファ
 */
//------------------------------------------------------------------
BMP_MENULIST_DATA * BmpMenuWork_ListCreate( u32 maxElems, u32 heapID )
{
	// 指定数より１つ多めに作っておき、最後に終端コードを置いて管理する
	BMP_MENULIST_DATA * list = GFL_HEAP_AllocClearMemory(
		heapID, sizeof(BMP_MENULIST_DATA)*(maxElems+1) );

	if( list )
	{
		u32 i;

		for(i=0; i<maxElems; i++)
		{
			list[i].str = NULL;
			list[i].param = 0;
		}

		list[i].str = LIST_ENDCODE;
		list[i].param = heapID;
	}
	return list;
}


//------------------------------------------------------------------
/**
 * リストバッファを破棄する
 * ※BMP_MENULIST_Createで作成したもの以外を渡してはダメ。
 *
 * @param   list		リストバッファ
 *
 */
//------------------------------------------------------------------
void BmpMenuWork_ListDelete( BMP_MENULIST_DATA* list_top )
{
	BmpMenuWork_ListSTRBUFDelete(list_top);
	GFL_HEAP_FreeMemory( list_top );
}

//------------------------------------------------------------------
/**
 * リストバッファに文字列＆パラメータをセットする（マネージャを介してアーカイブデータから読み込み）
 *
 * @param   list		[in] リストバッファ
 * @param   man			[in] メッセージデータマネージャ
 * @param   strID		文字列ID
 * @param   param		文字列パラメータ
 *
 */
//------------------------------------------------------------------
void BmpMenuWork_ListAddArchiveString( BMP_MENULIST_DATA* list,
		GFL_MSGDATA *msgdata, u32 strID, u32 param, HEAPID heap_id )
{
	u32 dmy;

	list = SeekEmptyListPos(list, &dmy);

	if( list )
	{
#if 0
		u32 len = 32;	//msgdataから引っ張るのが必要
		list->str = GFL_STR_CreateBuffer( len, heap_id );
		GFL_MSG_GetString( msgdata, strID, (STRBUF*)list->str );
#else
		list->str	= GFL_MSG_CreateString( msgdata, strID );
#endif
		list->param = param;
	}
}

#if 0	//dp
void BmpMenuList_AddArchiveString( BMP_MENULIST_DATA* list,
		const MSGDATA_MANAGER *man, u32 strID, u32 param )
{
	u32 dmy;

	list = SeekEmptyListPos(list, &dmy);

	if( list )
	{
		list->str = 
//		list->str = MSGMAN_AllocString( man, strID );
		list->param = param;
	}
}
#endif

//------------------------------------------------------------------
/**
 * リストバッファに文字列＆パラメータをセットする（文字列を直接指定）
 *
 * @param   list		[in] リストバッファ
 * @param   str			[in] 文字列
 * @param   param		文字列パラメータ
 *
 */
//------------------------------------------------------------------
void BmpMenuWork_ListAddString( BMP_MENULIST_DATA* list,
		const STRBUF *str, u32 param, HEAPID heapID  )
{
	u32 dmy;
	
	list = SeekEmptyListPos(list, &dmy );
	
	if( list )
	{
		list->str = GFL_STR_CreateCopyBuffer( str, heapID );
		list->param = param;
	}
}

#if 0
void BmpMenuList_AddString( BMP_MENULIST_DATA* list,
		const STRBUF* str, u32 param )
{
	u32 heapID;

	list = SeekEmptyListPos(list, &heapID);

	if( list )
	{
		list->str = STRBUF_CreateBufferCopy( str, heapID );
		list->param = param;
	}
}
#endif

//------------------------------------------------------------------
/**
 * リストバッファに他のリストのデータをLinkする
 * @param   list		[inout] リストバッファ
 * @param   insList		[in] リストポインタ
 *
 */
//------------------------------------------------------------------
void BmpMenuWork_ListAddLinkList(
	BMP_MENULIST_DATA* list, const BMP_MENULIST_DATA* insList )
{
	u32 heapID;

	list = SeekEmptyListPos(list, &heapID);

	if( list )
	{
		list->str = insList->str;
		list->param = insList->param;
	}
}

//------------------------------------------------------------------
/**
 * リストの先頭から空きがある位置をシークする
 *
 * @param   list		[in]  空き位置ポインタ（無ければNULL）
 * @param   heapID		[out] リスト作成時に保存したヒープIDを受け取る
 *
 * @retval  BMP_MENULIST_DATA*		
 */
//------------------------------------------------------------------
static BMP_MENULIST_DATA * SeekEmptyListPos(
		BMP_MENULIST_DATA* list, u32* heapID )
{
	BMP_MENULIST_DATA* ret;

	while( list->str != NULL )
	{
		if( list->str == LIST_ENDCODE )
		{
			GF_ASSERT(0);
			return NULL;
		}
		list++;
	}

	ret = list;

	while( list->str != LIST_ENDCODE )
	{
		list++;
	}

	*heapID = list->param;

	return ret;
}

//------------------------------------------------------------------
/**
 * リストバッファのSTRBUFを破棄する
 * ※BMP_MENULIST_Createで作成したもの以外を渡してはダメ。
 *
 * @param   list		リストバッファ
 *
 */
//------------------------------------------------------------------
void BmpMenuWork_ListSTRBUFDelete( BMP_MENULIST_DATA *list_top )
{
	BMP_MENULIST_DATA* list = list_top;
	
	while( list->str != LIST_ENDCODE )
	{
		if( list->str == NULL )
		{
			break;
		}
//		STRBUF_Delete((STRBUF*)(list->str));
		GFL_STR_DeleteBuffer( (STRBUF*)list->str );
        list->str = NULL;
		list++;
	}
}

//--------------------------------------------------------------
/**
 * リストバッファ、リスト最大文字数を取得
 * @param list_top
 * @retval u32 
 */
//--------------------------------------------------------------
u32 BmpMenuWork_GetListMaxLength( const BMP_MENULIST_DATA *list_top )
{
  u32 len,max = 0;
	const BMP_MENULIST_DATA* list = list_top;
  
  while( list->str != LIST_ENDCODE )
  {
    if( list->str == NULL )
    {
      break;
    }
    
    len = GFL_STR_GetBufferLength( (const STRBUF*)list->str );
    
    if( len > max )
    {
      max = len;
    }
  
    list++;
  }
  
  return( max );
}

//--------------------------------------------------------------
/**
 * リストバッファ　リスト最大数を取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
u32 BmpMenuWork_GetListMax( const BMP_MENULIST_DATA *list_top )
{
  u32 max = 0;
	const BMP_MENULIST_DATA* list = list_top;
  
  while( list->str != LIST_ENDCODE )
  {
    if( list->str != NULL )
    {
      max++;
    }
    list++;
  }
  return( max );
}

