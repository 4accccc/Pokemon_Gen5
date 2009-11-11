/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_control.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "nhttp.h"
#include "NHTTPlib_int.h"
#include "NHTTP_control.h"
#include "d_nhttp_common.h"
#include "NHTTP_request.h"
#include "NHTTP_nonport.h"

/*-------------------------------------------------------------------------*
  Name:         addHdrList

  Description:  送信リストにHeaderを追加する
                 
  Arguments:    list_pp       追加するリストのポインタ
                error_p       エラー値格納用ポインタ
                label         追加するラベル
                value         追加する要素

  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
static BOOL addHdrList(NHTTPi_DATALIST** list_pp, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p)
{
    const int listsize = sizeof(NHTTPi_DATALIST);
    
    int                 a_isFound = FALSE; // 既存のHeaderFieldに同じラベルを見つけたか
    NHTTPi_DATALIST*    list_p;
    
    if (*list_pp)
    {
        // 既存のHeaderFieldに同じ labelのものがないか探す
        list_p = *list_pp;
        if (NHTTPi_compareToken(label_p, list_p->label_p) != 0)
        {
            for (list_p = list_p->next_p; list_p != *list_pp; list_p = list_p->next_p)
            {
                if (NHTTPi_compareToken(label_p, list_p->label_p) == 0)
                {
                    a_isFound = TRUE;
                    break;
                }
            }
        }
        else
        {
            a_isFound = TRUE;
        }
    }
    
    if (a_isFound)
    {
        // 同じlabelがあれば上書き
        list_p->value_p = value_p;
    }
    else
    {
        // リスト要素を一つ追加
        list_p = (NHTTPi_DATALIST*)NHTTPi_alloc(listsize, 4);
        if (!list_p)
        {
            //DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
            PRINTF("failed to allocate memory\n");
            NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
            return FALSE;
        }
        list_p->label_p = label_p;
        list_p->value_p = value_p;
        list_p->length = 0;
        list_p->isBinary = FALSE;
        
        if (*list_pp)
        {
            list_p->prev_p = (*list_pp)->prev_p;
            list_p->next_p = *list_pp;
            (*list_pp)->prev_p->next_p = list_p;
            (*list_pp)->prev_p = list_p;
        }
        else
        {
            list_p->next_p = list_p;
            list_p->prev_p = list_p;
            *list_pp = list_p;
        }
    }
    
    return TRUE;
}

/*-------------------------------------------------------------------------*
  Name:         incAscii

  Description:  ASCIIコードを1文字インクリメントする
                 
  Arguments:    o_c             インクリメントしたい文字のポインタ

  Returns:      インクリメントした文字のポインタ
 *-------------------------------------------------------------------------*/
static char incAscii(u8 c)
{
    c++;
    if (c == 'z'+1)
    {
        c = '0';
    }
    else if (c == 'Z'+1)
    {
        c = 'a';
    }
    else if (c == '9'+1)
    {
        c = 'A';
    }

    return (char)c;
}

/*-------------------------------------------------------------------------*
  Name:         checkTagPost

  Description:  tagPostと重複するフレーズを見つけたらtagPostを変更
                 
  Arguments:    req     リクエスト構造体のポインタ
                value   探すフレーズ
                length  フレーズの長さ
  
  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
static BOOL checkTagPost(NHTTPReq* req_p, const char* value_p, u32 length)
{
    int  n;
    char c;
    
    if (NHTTPi_memfind(value_p, (int)length, &req_p->tagPost[2], LEN_POSTBOUND) < 0)
    {
        return TRUE;
    }
    
    // tagPostを改変    
    for (n = 2 + LEN_POSTBOUND - 1; n >= 2; n--)
    {
        for (c = req_p->tagPost[n];  ; )
        {
            c = incAscii((u8)c);
            req_p->tagPost[n] = c;
            
            if (c == NHTTPi_strMultipartBound[n])
            {
                break;
            }
            if (NHTTPi_memfind(value_p, (int)length, &req_p->tagPost[2], LEN_POSTBOUND) < 0)
            {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_getHdrFromList

  Description:  リストの先頭要素を取得
                取得された要素はリストから取り除かれる
                ただし、メモリ領域は解放されない
                
  Arguments:    o_list_pp       リスト
  
  Returns:      リストの先頭要素
 *-------------------------------------------------------------------------*/
extern NHTTPi_DATALIST* NHTTPi_getHdrFromList(NHTTPi_DATALIST** list_pp)
{
    NHTTPi_DATALIST* list_p = *list_pp;

    if (list_p)
    {
        if (list_p != list_p->prev_p)
        {
            list_p->prev_p->next_p = list_p->next_p;
            list_p->next_p->prev_p = list_p->prev_p;
            *list_pp = list_p->next_p;
        }
        else
        {
            *list_pp = NULL;
        }
    }
    
    return list_p;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_AddRequestHeaderField

  Description:  HTTPリクエスト実行時に送信するHTTPヘッダ項目を追加する

  Arguments:    req_p		処理対象のHTTPReqオブジェクトへのポインタ
                bgnEndInfo_p	NHTTPBgnEndInfo構造体ポインタ
                label_p		追加するHTTPヘッダ項目のラベル(NULL終端ASCII文字列)へのポインタ
                value_p		追加するHTTPヘッダ項目の値(NULL終端ASCII文字列)へのポインタ

  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_AddRequestHeaderField(NHTTPReq* req_p, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p)
{
    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    NHTTPi_ASSERT(req_p);
    NHTTPi_ASSERT(label_p);
    NHTTPi_ASSERT(value_p);
    
    if (req_p->isStarted)
    {
        return FALSE;
    }

    return addHdrList(&req_p->listHeader_p, bgnEndInfo_p, label_p, value_p);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_AddRequestPostDataAscii

  Description:  HTTPリクエスト実行時に送信するPOST項目(ASCII文字列)を追加する

  Arguments:    req_p		処理対象のHTTPReqオブジェクトへのポインタ
                bgnEndInfo_p	NHTTPBgnEndInfo構造体ポインタ
                label_p		追加するPOST項目のラベル(NULL終端ASCII文字列)へのポインタ
                value_p		追加するPOST項目の値(NULL終端ASCII文字列)へのポインタ

  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_AddRequestPostDataAscii(NHTTPReq* req_p, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p)
{
    // 一行が998文字を超えないこと
    int rc = FALSE;
    u32 length = 0;
    
    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    NHTTPi_ASSERT(req_p);
    
    if (req_p->isStarted)
    {
        return FALSE;
    }

#if 1 // 2006/09/28 noma
    // 既 に NHTTPAddPostDataRawが呼ばれている
    if ( req_p->isRawData )
    {
        //DEBUG_PRINTF("[%s] error: 既に NHTTPAddPostDataRaw が呼ばれている\n", __func__);
        PRINTF("already called NHTTPAddPostDataRaw (exclusive fucntion)\n");
        return FALSE;
    }
#endif
    
    if (value_p)
    {
        length = (u32)NHTTPi_strlen(value_p);
    }
    if (checkTagPost(req_p, value_p, length))
    {
        rc = addHdrList(&req_p->listPost_p, bgnEndInfo_p, label_p, value_p);
        if (rc)
        {
            req_p->listPost_p->prev_p->length = length;
        }
    }
    
    return rc;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_AddRequestPostDataBinary

  Description:  HTTPリクエスト実行時に送信するPOST項目(バイナリデータ)を追加する

  Arguments:    req_p		処理対象のHTTPReqオブジェクトへのポインタ
                bgnEndInfo_p	NHTTPBgnEndInfo構造体ポインタ
                label_p		追加するPOST項目のラベル(NULL終端ASCII文字列)へのポインタ
                value_p		追加するPOST項目の値(バイナリデータ)へのポインタ
                length		追加するPOST項目の値の長さ(Byte単位)

  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_AddRequestPostDataBinary(NHTTPReq* req_p, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p, u32 length)
{
    int rc;
	
    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    NHTTPi_ASSERT(req_p);
    
    if (req_p->isStarted)
    {
        return FALSE;
    }
    
    // 既 に NHTTPAddPostDataRawが呼ばれている
    if ( req_p->isRawData )
    {
        //DEBUG_PRINTF("[%s] error: 既に NHTTPAddPostDataRaw が呼ばれている\n", __func__);
        PRINTF("already called NHTTPAddPostDataRaw (exclusive fucntion)\n");
        return FALSE;
    }

    rc = FALSE;
    if (checkTagPost(req_p, value_p, length))
    {
        rc = addHdrList(&req_p->listPost_p, bgnEndInfo_p, label_p, value_p);
        if (rc)
        {
            req_p->listPost_p->prev_p->length = length;
            req_p->listPost_p->prev_p->isBinary = TRUE;
        }
    }
    
    return rc;
}

