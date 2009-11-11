/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_list.c

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
#include "NHTTP_list.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_request.h"
#include "NHTTP_control.h"
#include "d_nhttp_private.h"


/*-------------------------------------------------------------------------*

  Name:         NHTTPi_InitListInfo

  Description:  NHTTPListInfo構造体の初期設定
                 
  Arguments:    o_listInfo_p  NHTTPListInfo構造体へのポインタ

  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_InitListInfo(NHTTPListInfo* listInfo_p)
{
    listInfo_p->reqQueue = NULL;
    listInfo_p->idRequest = 0;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_setReqQueue

  Description:  新規にメモリを確保してキューリストにつなぐ
                ReqListはロックされていなければならない
                
  Arguments:    req     リクエスト構造体のポインタ

  Returns:      >=0     idを得る
                < 0	失敗（メモリが確保できない）
 *-------------------------------------------------------------------------*/
extern int NHTTPi_setReqQueue(NHTTPListInfo* listInfo_p, NHTTPReq const* req_p)
{
    NHTTPi_REQLIST* list_p;
    int id = -1;
    
    list_p = (NHTTPi_REQLIST*)NHTTPi_alloc(sizeof(NHTTPi_REQLIST), 4);
    
    if (list_p)
    {
        // execに接続
        if (listInfo_p->reqQueue)
        {
            list_p->prev_p = listInfo_p->reqQueue->prev_p;
            list_p->next_p = (NHTTPi_REQLIST*)listInfo_p->reqQueue;
            listInfo_p->reqQueue->prev_p->next_p = list_p;
            listInfo_p->reqQueue->prev_p = list_p;
        }
        else
        {
            list_p->prev_p = list_p;
            list_p->next_p = list_p;
            listInfo_p->reqQueue = list_p;
        }
        
        list_p->id = listInfo_p->idRequest++;
        list_p->req_p = (NHTTPReq*)req_p;
        list_p->socket_p = (void*)-1;
        id = list_p->id;
        if (listInfo_p->idRequest < 0)
        {
            listInfo_p->idRequest = 0;
        }
    }
    
    return id;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_findReqQueue

  Description:  リクエストリストから指定したidのリクエストをさがす
                ReqListはロックされていなければならない
  
  Arguments:    id

  Returns:      リクエストリスト
                取得できない場合はNULL  
 *-------------------------------------------------------------------------*/
extern NHTTPi_REQLIST* NHTTPi_findReqQueue(NHTTPListInfo* listInfo_p, const int id)
{
    NHTTPi_REQLIST*	list_p = NULL;
    NHTTPi_REQLIST*	listRet_p = NULL;

    if (listInfo_p->reqQueue)
    {
        list_p = (NHTTPi_REQLIST*)(listInfo_p->reqQueue);
        if (list_p->id == id)
        {
            listRet_p = list_p;
        }
        else
        {
            for (list_p=list_p->next_p; list_p!=(listInfo_p->reqQueue); list_p=list_p->next_p)
            {
                if (list_p->id == id)
                {
                    listRet_p = list_p;
                    break;
                }
            }
        }
    }

    return listRet_p;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_freeReqQueue

  Description:  指定したidをキューリストから解放
                リストからidに対応するやつをとって解放する
                ReqListはロックされていなければならない
  
  Arguments:    id

  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した（idのリクエストが存在しない）
 *-------------------------------------------------------------------------*/
extern int NHTTPi_freeReqQueue(NHTTPListInfo* listInfo_p, NHTTPMutexInfo* mutexInfo_p, const int id)
{
    NHTTPi_REQLIST*     list_p;
    NHTTPRes*           res_p;
    void*               param_p;
    int                 rc = FALSE;
    NHTTPConnection*	connection_p = NULL;
    
    // exec中から探す
    list_p = NHTTPi_findReqQueue(listInfo_p, id);
    if (list_p)
    {
        // 摘出
        if (listInfo_p->reqQueue != listInfo_p->reqQueue->prev_p)
        {
            list_p->prev_p->next_p = list_p->next_p;
            list_p->next_p->prev_p = list_p->prev_p;
            if (listInfo_p->reqQueue == list_p)
            {
                listInfo_p->reqQueue = list_p->next_p;
            }
        }
        else
        {
            listInfo_p->reqQueue = NULL;
        }

        res_p = list_p->req_p->res_p;
        param_p = res_p->param_p;
        connection_p = NHTTPi_Request2Connection(mutexInfo_p, list_p->req_p);
        NHTTPi_destroyRequestObject(mutexInfo_p, list_p->req_p);
        NHTTPi_free(list_p);

        /** NHTTPCreateConnection使用 */
        if (connection_p != NULL)
        {
            connection_p->nhttpError = NHTTP_ERROR_CANCELED;
            NHTTPi_CompleteCallback(mutexInfo_p, connection_p);
            
            NHTTPi_DeleteConnection(connection_p);
        }
        rc = TRUE;
    }
    
    return rc;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_allFreeReqQueue

  Description:  キューリスト中の要素をすべて解放
                ReqListはロックされていなければならない  
 *-------------------------------------------------------------------------*/
extern void NHTTPi_allFreeReqQueue(NHTTPListInfo* listInfo_p, NHTTPMutexInfo* mutexInfo_p)
{
    while (listInfo_p->reqQueue)
    {
        NHTTPi_freeReqQueue(listInfo_p, mutexInfo_p, listInfo_p->reqQueue->id);
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_getReqFromReqQueue

  Description:  キューリストの先頭を取得
                取得された要素はキューリストから取り除かれる
                ただし、メモリ領域は解放されない
                ReqListはロックされていなければならない
  
  Returns:      キューリストの先頭
                取得できない場合はNULL  
 *-------------------------------------------------------------------------*/
extern NHTTPi_REQLIST* NHTTPi_getReqFromReqQueue(NHTTPListInfo* listInfo_p)
{
    return (NHTTPi_REQLIST*)NHTTPi_getHdrFromList(
        (NHTTPi_DATALIST**)(void *)(&(listInfo_p->reqQueue)));
}
