/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_response.c

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
#include "d_nhttp_common.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_response.h"
#include "NHTTP_recvbuf.h"
#include "NHTTP_nonport.h"


/*-------------------------------------------------------------------------*
  Name:         NHTTPi_DestroyResponse
  Description:  HTTPレスポンスオブジェクトの使用するメモリを全て解放する
  Arguments:    o_res	処理対象のNHTTPResオブジェクトへのポインタ
  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTPi_DestroyResponse(NHTTPMutexInfo* mutexInfo_p, NHTTPRes* res_p)
{
    NHTTPi_HDRBUFLIST* list_p = NULL;

    NHTTPi_ASSERT(res_p);
    
    while (res_p->hdrBufBlock_p)
    {
        list_p = res_p->hdrBufBlock_p->next_p;
        
        NHTTPi_free(res_p->hdrBufBlock_p);
        res_p->hdrBufBlock_p = list_p;
    }
    
    if (res_p->allHeader_p)
    {
        NHTTPi_free(res_p->allHeader_p);
    }
    
    if (res_p->foundHeader_p)
    {
        NHTTPi_free(res_p->foundHeader_p);
    }

    if (res_p->freeBuf != NULL)
    {
        res_p->freeBuf(res_p->recvBuf_p, NHTTPi_free, res_p->param_p);
        res_p->recvBuf_p = NULL;
        res_p->recvBufLen = 0;
    }

    /* 相互参照解除 */
    {
        NHTTPConnection* connection_p = NHTTPi_Response2Connection(mutexInfo_p, res_p);
        
        if (connection_p != NULL)
        {
            connection_p->res_p = NULL;
        }
    }
        
    NHTTPi_free(res_p);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_getHeaderValue
  Description:  受信したヘッダからlabelのものを探して値を得る
  Arguments:    res_p
                label_p  
                pos_p    発見位置
  Returns:      値のバイト長
 *-------------------------------------------------------------------------*/
extern int NHTTPi_getHeaderValue(const NHTTPRes* res_p, const char* label_p, int* pos_p)
{
    int	linetop, nextlinetop;
    int labeltop, labelend;
    int valuetop, valueend;
    int return_code_size = 0;
    
    // Status-Lineの次の行頭を探す
    linetop = NHTTPi_findNextLineHdrRecvBuf(res_p, 12, (int)res_p->headerLen, &labelend, &return_code_size);

    while (linetop > 0)
    {
        // 次の行頭を探す
        nextlinetop = NHTTPi_findNextLineHdrRecvBuf(res_p, linetop, (int)res_p->headerLen, &labelend, &return_code_size);
        
        if (labelend > 0)
        {
            //
            // Date: 6 Feb. 2006 \n
            // |   | |           |
            // |   | valuetop   valueend
            // |  labelend
            // | 
            //linetop = labeltop
            //
            labeltop = linetop;
            
            if (NHTTPi_compareTokenN_HdrRecvBuf(res_p, labeltop, labelend, label_p, 0) == 0)
            {
                // valueを取り出す
                if (labelend+1 < res_p->headerLen)
                {
                    // valueの長さを得る
                    valueend = NHTTPi_findNextLineHdrRecvBuf(res_p, labelend+1, (int)res_p->headerLen, NULL, &return_code_size);
                    if (valueend <= 0)
                    {
                        // 改行無しにバッファの終わりまで到達した
                        // または、改行直後がバッファの終わりだった
                        valueend = res_p->headerLen;
                    }
                    else
                    {
                        if (valueend < return_code_size)
                        {
                             return -1;
                        }
                        // 改行コードの長さを除く
                        valueend -= return_code_size;
                    }
                    
                    // value頭の空白を読み飛ばす
                    valuetop = NHTTPi_skipSpaceHdrRecvBuf( res_p, labelend+1, valueend );
                    if (valuetop < 0)
                    {
                        valuetop = valueend;		// valueは全部空白だった
                    }
                    
                    *pos_p = valuetop;
                    return valueend - valuetop;
                }
                else
                {
                    // 「:」の後すぐ終わり
                    // 空
                    return 0;
                }
            }
        }
        linetop = nextlinetop;
    } 
    
    return -1;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetResponseHeaderField
  Description:  HTTPヘッダの情報を、ラベル名を指定して取得する
  Arguments:    res_p   	処理対象のNHTTPResオブジェクトへのポインタ
                label_p         取得を試みるヘッダ情報のラベル名
                value_pp	取得結果のアドレスを格納するためのポインタへのポインタ
  Returns:      0以上	取得したHTTPヘッダ情報の文字数
                0未満	指定したラベルはHTTPヘッダ内に存在しない
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetResponseHeaderField(NHTTPRes* res_p, const char* label_p, char** value_pp)
{
    int pos, size;
	
    NHTTPi_ASSERT(NHTTPi_IsOpened(&(NHTTPi_GetSystemInfoP()->bgnEndInfo)));
    NHTTPi_ASSERT(res_p);
    NHTTPi_ASSERT(label_p);
    NHTTPi_ASSERT(value_pp);
    
    if (!res_p->isHeaderParse)
    {
        return -1;
    }
    
    if (res_p->foundHeader_p)
    {
        NHTTPi_free(res_p->foundHeader_p);
        res_p->foundHeader_p = NULL;
    }
    
    size = NHTTPi_getHeaderValue(res_p, label_p, &pos);
    if (size >= 0)
    {
        res_p->foundHeader_p = (char*)NHTTPi_alloc((u32)(size+1), 4);
        //エラー
        if (res_p->foundHeader_p == NULL)
        {
            return -1;
        }
        else
        {
            res_p->foundHeader_p[size] = 0;
            NHTTPi_loadFromHdrRecvBuf(res_p, res_p->foundHeader_p, pos, size);
            *value_pp = res_p->foundHeader_p;
            return size;
        }
    }
    
    if (NHTTPi_strcmp("HTTPSTATUSCODE", label_p) == 0)
    {
        res_p->foundHeader_p = (char*)NHTTPi_alloc(3+1, 4);
        //エラー
        if (res_p->foundHeader_p == NULL)
        {
            return -1;
        }
        else
        {
            res_p->foundHeader_p[3] = 0;
            NHTTPi_loadFromHdrRecvBuf(res_p, res_p->foundHeader_p, 9, 3);
            *value_pp = res_p->foundHeader_p;
            return 3;
        }
    }

    // エラーコードは？
    return -1;
}


/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetResponseHeaderAll
  Description:  HTTPヘッダ全体を取得する
  Arguments:    o_res	処理対象のNHTTPResオブジェクトへのポインタ
                o_value	取得結果のアドレスを格納するためのポインタへのポインタ
  Returns:      0以上	取得したHTTPヘッダの長さ(Byte単位)
                0未満	HTTPヘッダを取得できない
 *-------------------------------------------------------------------------*/
extern int NHTTPi_GetResponseHeaderAll(NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPRes* res_p, char** value_pp)
{
    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    NHTTPi_ASSERT(res_p);
    NHTTPi_ASSERT(value_pp);
    
    if (!res_p->isHeaderParse || !res_p->headerLen)
    {
        return -1;
    }
    
    if (res_p->headerLen < NHTTP_HDRRECVBUF_INILEN)
    {
        *value_pp = res_p->hdrBufFirst;
    }
    else
    {
        if (!res_p->allHeader_p)
        {
            res_p->allHeader_p = (char*)NHTTPi_alloc((u32)res_p->headerLen, 4);
            if (!res_p->allHeader_p)
            {
                DEBUG_PRINTF("NHTTP_ERROR_ALLOC function : %s line : %d\n", __func__, __LINE__);
                NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
                return -1;
            }
            NHTTPi_loadFromHdrRecvBuf(res_p, res_p->allHeader_p, 0, res_p->headerLen);
        }
        *value_pp = res_p->allHeader_p;
    }
    
    return res_p->headerLen;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetResponseBodyAll
  Description:  HTTPボディ全体を取得する
  Arguments:    res	処理対象のNHTTPResオブジェクトへのポインタ
                value	取得結果のアドレスを格納するためのポインタへのポインタ
  Returns:      0以上	取得したHTTPボディの長さ(Byte単位)
                0未満	HTTPボディを取得できない
 *-------------------------------------------------------------------------*/
extern int NHTTPi_GetResponseBodyAll(const NHTTPRes* res_p, char** value_pp)
{
    NHTTPi_ASSERT(NHTTPi_IsOpened(&(NHTTPi_GetSystemInfoP()->bgnEndInfo)));
    NHTTPi_ASSERT(res_p);
    NHTTPi_ASSERT(value_pp);
    
    if (!res_p->isSuccess || !res_p->bodyLen)
    {
        return -1;
    }
    *value_pp = res_p->recvBuf_p;
    
    return res_p->bodyLen;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

