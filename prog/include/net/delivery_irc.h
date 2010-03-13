//======================================================================
/**
 * @file	  delivery_irc.h
 * @brief	  IRC配信
 * @author	k.ohno
 * @data	  10/02/27
 */
//======================================================================

#pragma once
#include "net/delivery_base.h"


#define DELIVERY_IRC_FUNC (0)   //動作中
#define DELIVERY_IRC_SUCCESS (1)  //成功
#define DELIVERY_IRC_FAILED (2)   //失敗
#define DELIVERY_IRC_NOTLANG (3)  //言語コードが違う

typedef struct _DELIVERY_IRC_LOCALWORK DELIVERY_IRC_WORK;


//--------------------------------------------------------------
/**
 * @brief   IRC配送初期化
 * @param   DELIVERY_IRC_INIT 初期化構造体 ローカル変数でも大丈夫
 * @retval  DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------
extern DELIVERY_IRC_WORK* DELIVERY_IRC_Init(DELIVERY_IRC_INIT* pInit);

//--------------------------------------------------------------
/**
 * @brief   IRC配信開始
 * @param   DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------
extern BOOL DELIVERY_IRC_SendStart(DELIVERY_IRC_WORK* pWork);

//--------------------------------------------------------------
/**
 * @brief   IRC受信開始
 * @param   DELIVERY_IRC_WORK 管理ワーク
 * @retval  開始できたらTRUE
 */
//--------------------------------------------------------------
extern BOOL DELIVERY_IRC_RecvStart(DELIVERY_IRC_WORK* pWork);

//--------------------------------------------------------------
/**
 * @brief   IRC受信完了かどうか
 * @param   DELIVERY_IRC_WORK 管理ワーク
 * @retval  受け取れた場合DELIVERY_IRC_SUCCESS 失敗DELIVERY_IRC_FAILED
 */
//--------------------------------------------------------------
extern int DELIVERY_IRC_RecvCheck(DELIVERY_IRC_WORK* pWork);

//--------------------------------------------------------------
/**
 * @brief   IRC配送メイン
 * @param   DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------

extern void DELIVERY_IRC_Main(DELIVERY_IRC_WORK*  pWork);

//--------------------------------------------------------------
/**
 * @brief   IRC配送終了
 * @param   DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------
extern void DELIVERY_IRC_End(DELIVERY_IRC_WORK*  pWork);
