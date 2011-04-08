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
#define RESORT_GET_PARAM_TYPE               (8)
#define RESORT_GET_PARAM_DAY_FLAG           (9)
#define RESORT_GET_PARAM_IS_INSTITUTION_FULL   (10)

//_RESORT_SET_MY_DATA, _RESORT_GET_MY_DATA用
#define RESORT_MY_PARAM_GOSSIP_00          (0)
#define RESORT_MY_PARAM_GOSSIP_01          (1)
#define RESORT_MY_PARAM_GOSSIP_02          (2)
#define RESORT_MY_PARAM_GOSSIP_03          (3)
#define RESORT_MY_PARAM_GOSSIP_04          (4)
#define RESORT_MY_PARAM_GOSSIP_05          (5)
#define RESORT_MY_PARAM_GOSSIP_06          (6)
#define RESORT_MY_PARAM_GOSSIP_07          (7)
#define RESORT_MY_PARAM_GOSSIP_08          (8)
#define RESORT_MY_PARAM_GOSSIP_09          (9)
#define RESORT_MY_PARAM_GOSSIP_10          (10)
#define RESORT_MY_PARAM_GOSSIP_11          (11)
#define RESORT_MY_PARAM_GOSSIP_12          (12)
#define RESORT_MY_PARAM_GOSSIP_13          (13)
#define RESORT_MY_PARAM_GOSSIP_14          (14)
#define RESORT_MY_PARAM_GOSSIP_15          (15)

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
#define RESORT_WORD_SHOP_LABEL        (2) //「売る」とか
#define RESORT_WORD_FREE_GREET        (3)
#define RESORT_WORD_FREE_NAME         (4)
#define RESORT_WORD_COMM_TYPE         (5) //通信種別
#define RESORT_WORD_FREE_PHRASE       (6)
#define RESORT_WORD_SHOP_GENRE        (7) //「花屋」とか
#define RESORT_WORD_FREE_CITY         (8)

//_RESORT_CALL_LIST用
#define RESORT_LIST_TYPE_BUY            (0)
#define RESORT_LIST_TYPE_SELECT         (1)
#define RESORT_LIST_TYPE_SORT           (2)

#define RESORT_LIST_CANCEL              (0xFF)

  //RESORT_LIST_TYPE_SELECT
#define RESORT_LIST_SELECT_SECRETARY_ADD      (0)
#define RESORT_LIST_SELECT_SECRETARY_DEL      (1)
#define RESORT_LIST_SELECT_INSTATITUTION      (2)
#define RESORT_LIST_SELECT_INSTATITUTION_DEL  (3)

  //RESORT_LIST_TYPE_SORT
#define RESORT_LIST_SORT_SECRETARY      (0)
#define RESORT_LIST_SORT_INSTATITUTION  (1)

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

//_RESORT_LOT用
#define RESORT_LOT_FAILURE                    (0)

//_RESORT_ADD_EFFORT
#define RESORT_ADD_EFFORT_HP                  (0)
#define RESORT_ADD_EFFORT_POW                 (1)
#define RESORT_ADD_EFFORT_DEF                 (2)
#define RESORT_ADD_EFFORT_AGI                 (3)
#define RESORT_ADD_EFFORT_SPEPOW              (4)
#define RESORT_ADD_EFFORT_SPEDEF              (5)

//_RESORT_DELETE_PEOPLE用
#define RESORT_DELTE_PEOPLE_MINE              (0xFF)

//_RESORT_GET_NPC_DATA用
#define RESORT_NPC_DATA_OBJID                 (0)
#define RESORT_NPC_DATA_INDEX                 (1)
#define RESORT_NPC_DATA_MOUTHCOMM_Q           (2)
#define RESORT_NPC_DATA_MOUTHCOMM_A_NUM       (3)
#define RESORT_NPC_DATA_MOUTHCOMM_A_00        (4)
#define RESORT_NPC_DATA_MOUTHCOMM_A_01        (5)
#define RESORT_NPC_DATA_MOUTHCOMM_A_02        (6)
#define RESORT_NPC_DATA_MOUTHCOMM_A_03        (7)


