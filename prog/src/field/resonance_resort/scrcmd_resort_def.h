//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_scrcmd.h
 *	@brief  スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/*
 *					スクリプトコマンドとソースの共通定義
*/
//=============================================================================
//_RESORT_GET_DATA用
#define RESORT_GET_PARAM_FRONT_OBJID        (0)
#define RESORT_GET_PARAM_IS_SECREARY_FULL   (1)
#define RESORT_GET_PARAM_MERGE_FLAG         (2)
#define RESORT_GET_PARAM_PRESENT_ID         (3)
#define RESORT_GET_PARAM_PRESENT_NUM        (4)
#define RESORT_GET_PARAM_PRESENT_IS_ITEM    (5)
#define RESORT_GET_PARAM_IS_INSTITUTION_EMPTY   (6)
#define RESORT_GET_PARAM_IS_EVENT           (7)
//_RESORT_JOB_CHANGE用
#define RESORT_JOBCHANGE_RESIDENT_AUTO      (0)
#define RESORT_JOBCHANGE_RESIDENT_LOT       (1)
#define RESORT_JOBCHANGE_RESIDENT_PARLOR    (2)
#define RESORT_JOBCHANGE_RESIDENT_BREEDER   (3)
#define RESORT_JOBCHANGE_RESIDENT_MARKET    (4)
#define RESORT_JOBCHANGE_RESIDENT_FLORIST   (5)
#define RESORT_JOBCHANGE_RESIDENT_EXRCICE   (6)
#define RESORT_JOBCHANGE_RESIDENT_NURSERY   (7)
#define RESORT_JOBCHANGE_RESIDENT_JEWELLEY  (8)
#define RESORT_JOBCHANGE_RESIDENT_ANTIQUE   (9)
#define RESORT_JOBCHANGE_RESIDENT_CAFE      (10)
#define RESORT_JOBCHANGE_RESIDENT_GYM       (11)
#define RESORT_JOBCHANGE_RESIDENT_HOUSE     (12)
#define RESORT_JOBCHANGE_SECETARY           (13)
#define RESORT_JOBCHANGE_VISITOR            (14)
///_RESORT_ACTION用
#define RESORT_ACTION_LEAVE           (0)
//_RESORT_SET_WORD用
#define RESORT_WORD_PEOPLE            (0)
#define RESORT_WORD_SHOP              (1) //「フラワー○○」とか
#define RESORT_WORD_FREE_GREET        (3)
#define RESORT_WORD_FREE_NAME         (4)
#define RESORT_WORD_FREE_PHRASE       (6)
#define RESORT_WORD_SHOP_GENRE        (7) //「花屋」とか
#define RESORT_WORD_FREE_CITY         (8)
#define RESORT_WORD_SHOP_LABEL        (2) //「売る」とか

//_RESORT_CALL_LIST用
#define RESORT_CALL_LIST_MODE_ITEM            (0)
#define RESORT_CALL_LIST_MODE_SECRETARY_ADD   (0)
#define RESORT_CALL_LIST_MODE_INSTITUTION     (1)
#define RESORT_CALL_LIST_MODE_SERVICE         (3)
#define RESORT_CALL_LIST_MODE_SWAP            (3)

//_RESORT_GET_MSGID用
  //エクセルのMSG
#define RESORT_MSGID_00             (0)
#define RESORT_MSGID_01             (1)
#define RESORT_MSGID_02             (2)
#define RESORT_MSGID_03             (3)
#define RESORT_MSGID_04             (4)
#define RESORT_MSGID_05             (5)

  //特殊MSG
#define RESORT_MSGID_RECENT         (10)  //最近MSG
#define RESORT_MSGID_WHERETOGO      (11)  //どこへ行きたいか
#define RESORT_MSGID_APPEAL         (12)  //アピール文
#define RESORT_MSGID_SHOP_INFO      (13)  //お店情報
#define RESORT_MSGID_GOSSIP         (14)  //噂（特殊NPCの話)

  //旅行者
#define RESORT_TOURIST_GREET_MSGID  (RESORT_MSGID_00)
#define RESORT_TOURIST_BYE_MSGID    (RESORT_MSGID_01)
#define RESORT_TOURIST_ITEM_MSGID   (RESORT_MSGID_02)
#define RESORT_TOURIST_JOY_MSGID    (RESORT_MSGID_03)
#define RESORT_TOURIST_WAIT_MSGID   (RESORT_MSGID_04)

  //代理人
#define RESORT_PROXY_GREET_MSGID  (RESORT_MSGID_00)
#define RESORT_PROXY_ITEM_MSGID   (RESORT_MSGID_01)

  //住人
#define RESORT_RESIDENT_GREET_MSGID           (RESORT_MSGID_00)
#define RESORT_RESIDENT_BYE_MSGID             (RESORT_MSGID_01)
#define RESORT_RESIDENT_CHANGE_MSGID          (RESORT_MSGID_02)
#define RESORT_RESIDENT_CHANGE_CONFIM_MSGID   (RESORT_MSGID_03)
#define RESORT_RESIDENT_CHANGE_OK_MSGID       (RESORT_MSGID_04)
#define RESORT_RESIDENT_CHANGE_NG_MSGID       (RESORT_MSGID_05)

  //滞在者
#define RESORT_VISITOR_GREET_MSGID            (RESORT_MSGID_00)
#define RESORT_VISITOR_BYE_MSGID              (RESORT_MSGID_01)

  //秘書
#define RESORT_SECRETARY_GREET_MSGID          (0)
#define RESORT_SECRETARY_BYE_MSGID            (1)


//_RESORT_SEARCH_RECEPTION用
#define RESORT_SEARCH_PROXY_GTS               (0)
#define RESORT_SEARCH_PROXY_MYSTERY           (0)
