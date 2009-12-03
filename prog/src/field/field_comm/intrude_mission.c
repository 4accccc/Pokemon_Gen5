//==============================================================================
/**
 * @file    intrude_mission.c
 * @brief   ミッション処理    受信データのセットなどを行う為、常駐に配置
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "intrude_types.h"
#include "intrude_mission.h"
#include "intrude_comm_command.h"
#include "msg/msg_invasion.h"
#include "intrude_main.h"
#include "intrude_work.h"
#include "print/wordset.h"

#include "mission.naix"


//==============================================================================
//  定数定義
//==============================================================================
///ミッション失敗までのタイム
#define MISSION_TIME_OVER     (60/2 * 60)   // 60/2 = フィールドは1/30の為。


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void MISSION_SendUpdate(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission);
static void MISSION_SetMissionFail(MISSION_SYSTEM *mission, int fail_netid);
static int _TragetNetID_Choice(INTRUDE_COMM_SYS_PTR intcomm, int accept_netid);



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ミッションデータ初期化
 *
 * @param   mission		ミッションシステムへのポインタ
 */
//==================================================================
void MISSION_Init(MISSION_SYSTEM *mission)
{
  GFL_STD_MemClear(mission, sizeof(MISSION_SYSTEM));
  mission->data.mission_no = MISSION_NO_NULL;
  mission->result.mission_data.mission_no = MISSION_NO_NULL;
  mission->data.accept_netid = INTRUDE_NETID_NULL;
  
  MISSION_Init_List(mission);
}

//==================================================================
/**
 * ミッションリスト初期化
 *
 * @param   mission		
 */
//==================================================================
void MISSION_Init_List(MISSION_SYSTEM *mission)
{
  int i;

  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    mission->list[i].accept_netid = INTRUDE_NETID_NULL;
  }
}

//==================================================================
/**
 * ミッション更新処理
 *
 * @param   mission		
 */
//==================================================================
void MISSION_Update(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission)
{
  //親機でミッション発動しているなら失敗までの秒数をカウントする
  if(GFL_NET_IsParentMachine() == TRUE 
      && MISSION_RecvCheck(mission) == TRUE 
      && mission->result.mission_data.mission_no == MISSION_NO_NULL
      && mission->result.mission_data.accept_netid == INTRUDE_NETID_NULL){
    mission->timer++;
    if(mission->timer > MISSION_TIME_OVER){
      GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
      MISSION_SetMissionFail(mission, GAMEDATA_GetIntrudeMyID(gamedata));
    }
  }
  
  //送信リクエストがあれば送信
  MISSION_SendUpdate(intcomm, mission);
}

//==================================================================
/**
 * ミッションリスト送信リクエストを設定する
 *
 * @param   mission		
 */
//==================================================================
void MISSION_Set_ListSendReq(MISSION_SYSTEM *mission, int palace_area)
{
  mission->list_send_req[palace_area] = TRUE;
}

//==================================================================
/**
 * ミッションデータ送信リクエストを設定する
 *
 * @param   mission		
 */
//==================================================================
void MISSION_Set_DataSendReq(MISSION_SYSTEM *mission)
{
  mission->data_send_req = TRUE;
}

//==================================================================
/**
 * ミッションリクエストデータを受け取り、ミッションを発動する
 *
 * @param   mission		      ミッションシステムへのポインタ
 * @param   req		          リクエストデータへのポインタ
 * @param   accept_netid		ミッション受注者のNetID
 *
 * @retval  BOOL		TRUE:ミッション発動。　FALSE:発動できない
 */
//==================================================================
BOOL MISSION_SetEntry(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission, const MISSION_REQ *req, int accept_netid)
{
  MISSION_DATA *mdata = &mission->data;
  
  if(mdata->mission_no != MISSION_NO_NULL || mdata->accept_netid != INTRUDE_NETID_NULL){
    return FALSE;
  }
  mdata->mission_no = GFUser_GetPublicRand0(MISSION_NO_MAX);
  mdata->accept_netid = accept_netid;
  mdata->monolith_type = req->monolith_type;
  mdata->zone_id = req->zone_id;
  mdata->target_netid = _TragetNetID_Choice(intcomm, accept_netid);
  
  return TRUE;
}

//--------------------------------------------------------------
/**
 * ミッションのターゲット対象となるNetIDを決定する
 *
 * @param   intcomm		
 * @param   accept_netid		ミッション受注者のNetID
 *
 * @retval  int		ターゲット対象者のNetID
 */
//--------------------------------------------------------------
static int _TragetNetID_Choice(INTRUDE_COMM_SYS_PTR intcomm, int accept_netid)
{
  int i, target_no, connect_num;

  connect_num = GFL_NET_GetConnectNum();
  target_no = GFUser_GetPublicRand0(connect_num - 1); // -1 = 受注者分
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(i != accept_netid && intcomm->recv_profile & (1 << i)){
      if(target_no == 0){
        return i;
      }
    }
  }
  
  GF_ASSERT_MSG(0, "connect_num = %d, accept_netid = %d, recv_profile = %d\n", 
    connect_num, accept_netid, intcomm->recv_profile);
  return 0;
}

//==================================================================
/**
 * ミッションリストをセット
 *
 * @param   mission		代入先
 * @param   MISSION_CHOICE_LIST		    セットするデータ
 */
//==================================================================
void MISSION_SetMissionList(MISSION_SYSTEM *mission, const MISSION_CHOICE_LIST *list)
{
  MISSION_CHOICE_LIST *mlist = &mission->list[list->palace_area];
  
  if(mlist->accept_netid != INTRUDE_NETID_NULL){
    return; //既に受信済み
  }
  
  *mlist = *list;
  //不正チェック
#if 0 //もう少ししっかりとデータの取り扱いが決まってから有効にする
  if(mdata->mission_no >= MISSION_NO_MAX || mdata->monolith_type >= MONOLITH_TYPE_MAX){
    return;
  }
#endif
}

//==================================================================
/**
 * ミッションデータをセット
 *
 * @param   mission		代入先
 * @param   src		    セットするデータ
 */
//==================================================================
void MISSION_SetMissionData(MISSION_SYSTEM *mission, const MISSION_DATA *src)
{
  MISSION_DATA *mdata = &mission->data;
  
  //親の場合、既にmisison_noはセットされているので判定の前に受信フラグをセット
  mission->parent_data_recv = TRUE;
  mission->timer = 0;
  if(mdata->mission_no != MISSION_NO_NULL || mdata->accept_netid != INTRUDE_NETID_NULL){
    return;
  }
  
  *mdata = *src;
  OS_TPrintf("mission受信 mission_no = %d\n", src->mission_no);
  //不正チェック
#if 0 //もう少ししっかりとデータの取り扱いが決まってから有効にする
  if(mdata->mission_no >= MISSION_NO_MAX || mdata->monolith_type >= MONOLITH_TYPE_MAX){
    return;
  }
#endif
}

//==================================================================
/**
 * ミッションデータ送信リクエストが発生していれば送信を行う
 *
 * @param   intcomm		
 * @param   mission		
 */
//==================================================================
static void MISSION_SendUpdate(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission)
{
  int i;
  
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(mission->list_send_req[i] == TRUE){
      if(IntrudeSend_MissionList(intcomm, mission, i) == TRUE){
        mission->list_send_req[i] = FALSE;
      }
    }
  }
  
  if(mission->data_send_req == TRUE){
    if(IntrudeSend_MissionData(intcomm, mission) == TRUE){
      mission->data_send_req = FALSE;
    }
  }
  
  if(mission->result_send_req == TRUE){
    if(IntrudeSend_MissionResult(intcomm, mission) == TRUE){
      mission->result_send_req = FALSE;
    }
  }
}

//==================================================================
/**
 * ミッションデータ受信済みかチェック
 *
 * @param   mission		
 *
 * @retval  BOOL		TRUE:受信済み　FALSE:受信していない
 */
//==================================================================
BOOL MISSION_RecvCheck(const MISSION_SYSTEM *mission)
{
  const MISSION_DATA *mdata = &mission->data;
  
  if(mdata->mission_no == MISSION_NO_NULL || mdata->accept_netid == INTRUDE_NETID_NULL || mission->parent_data_recv == FALSE){
    return FALSE;
  }
  return TRUE;
}

//==================================================================
/**
 * ミッションデータから対応したミッションメッセージIDを取得する
 *
 * @param   mission		
 *
 * @retval  u16		メッセージID
 */
//==================================================================
u16 MISSION_GetMissionMsgID(const MISSION_SYSTEM *mission)
{
  const MISSION_DATA *mdata = &mission->data;
  u16 msg_id;
  
  if(mdata->accept_netid == INTRUDE_NETID_NULL || mdata->mission_no == MISSION_NO_NULL || mdata->mission_no >= MISSION_NO_MAX){
    return msg_invasion_mission000;
  }
  msg_id = mdata->mission_no * 2 + mdata->monolith_type;
  return msg_invasion_mission001 + msg_id;
}

//==================================================================
/**
 * ミッション結果から対応した結果メッセージIDを取得する
 *
 * @param   mission		
 *
 * @retval  u16		メッセージID
 */
//==================================================================
u16 MISSION_GetAchieveMsgID(const MISSION_SYSTEM *mission, int my_netid)
{
  const MISSION_RESULT *result = &mission->result;
  
  if(result->mission_data.accept_netid == INTRUDE_NETID_NULL
      || result->mission_data.mission_no == MISSION_NO_FAIL
      || result->mission_data.mission_no == MISSION_NO_NULL 
      || result->mission_data.mission_no >= MISSION_NO_MAX){
    return msg_invasion_mission_sys002;
  }
  
  if(my_netid == result->achieve_netid){
    return msg_invasion_mission001_02;
  }
  return msg_invasion_mission001_03;
}

//==================================================================
/**
 * ミッション達成報告エントリー
 *
 * @param   mission		
 * @param   mdata		        達成したミッションデータ
 * @param   achieve_netid		達成者のNetID
 *
 * @retval  BOOL		TRUE：達成者として受け付けられた
 * @retval  BOOL		FALSE：先に達成者がいる等して受け付けられなかった
 */
//==================================================================
BOOL MISSION_EntryAchieve(MISSION_SYSTEM *mission, const MISSION_DATA *mdata, int achieve_netid)
{
  MISSION_RESULT *result = &mission->result;
  MISSION_DATA *data = &mission->data;
  
  OS_TPrintf("ミッション達成エントリー net_id=%d\n", achieve_netid);
  if(result->mission_data.accept_netid != INTRUDE_NETID_NULL || result->mission_data.mission_no != MISSION_NO_NULL || mission->parent_result_recv == TRUE){
    OS_TPrintf("先に達成者がいたので受け付けない\n");
    return FALSE;
  }
  if(GFL_STD_MemComp(data, mdata, sizeof(MISSION_DATA)) != 0){
    OS_TPrintf("親が管理しているミッションデータと内容が違うので受け付けない\n");
    return FALSE;
  }
  
  result->mission_data = *mdata;
  result->achieve_netid = achieve_netid;
  mission->result_send_req = TRUE;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * ミッション失敗をセット
 *
 * @param   mission		
 * @param   fail_netid		
 */
//--------------------------------------------------------------
static void MISSION_SetMissionFail(MISSION_SYSTEM *mission, int fail_netid)
{
  MISSION_RESULT *result = &mission->result;
  MISSION_DATA *data = &mission->data;
  
  if(data->accept_netid == INTRUDE_NETID_NULL || data->mission_no == MISSION_NO_NULL || result->mission_data.mission_no != MISSION_NO_NULL){
    OS_TPrintf("MissionFailは受け入れられない %d, %d\n", 
      data->mission_no, result->mission_data.mission_no);
    return;
  }
  
  result->mission_data = *data;
  result->mission_data.mission_no = MISSION_NO_FAIL;
  result->achieve_netid = fail_netid;
  mission->result_send_req = TRUE;
  OS_TPrintf("ミッション失敗をセット\n");
}

//==================================================================
/**
 * ミッション結果をセット
 *
 * @param   mission		
 * @param   cp_result		
 */
//==================================================================
void MISSION_SetResult(MISSION_SYSTEM *mission, const MISSION_RESULT *cp_result)
{
  MISSION_RESULT *result = &mission->result;
  
  GF_ASSERT(mission->parent_result_recv == FALSE);
  *result = *cp_result;
  mission->parent_result_recv = TRUE;
}

//==================================================================
/**
 * ミッション結果受信確認
 *
 * @param   mission		ミッションシステムへのポインタ
 *
 * @retval  BOOL		TRUE:受信した。　FALSE:受信していない
 */
//==================================================================
BOOL MISSION_RecvAchieve(const MISSION_SYSTEM *mission)
{
  const MISSION_RESULT *result = &mission->result;
  
  if(mission->parent_result_recv == TRUE && result->mission_data.accept_netid != INTRUDE_NETID_NULL && result->mission_data.mission_no != MISSION_NO_NULL){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 話しかけ：ミッション達成チェック
 *
 * @param   mission		    ミッションデータへのポインタ
 * @param   talk_netid		話しかけた相手のNetID
 *
 * @retval  BOOL		TRUE:達成。　FALSE:非達成
 */
//==================================================================
BOOL MISSION_Talk_CheckAchieve(const MISSION_SYSTEM *mission, int talk_netid)
{
  const MISSION_DATA *mdata = &mission->data;
  
  if(mdata->mission_no == MISSION_NO_NULL || mdata->accept_netid == INTRUDE_NETID_NULL){
    return FALSE;
  }
  if(talk_netid == mdata->target_netid){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * ミッション結果から得られるミッションポイントを取得する
 *
 * @param   intcomm		
 * @param   result		
 *
 * @retval  int		ミッションポイント
 */
//==================================================================
s32 MISSION_GetPoint(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_RESULT *result)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  s32 point;
  
  if(result->mission_data.mission_no == MISSION_NO_FAIL){
    return 0;
  }
  
  if(result->achieve_netid == GAMEDATA_GetIntrudeMyID(gamedata)){
    point = 100;
  }
  else{
    point = 50;
  }
  return point;
}

//==================================================================
/**
 * ミッション結果から得られるミッションポイント、占拠ポインタを加算する
 *
 * @param   intcomm		
 * @param   mission		
 *
 * @retval  BOOL		TRUE:加算した。　FALSE:ポイントは発生しなかった
 */
//==================================================================
BOOL MISSION_AddPoint(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  const MISSION_RESULT *result = &intcomm->mission.result;
  int point;
  
  point = MISSION_GetPoint(intcomm, result);
  if(point > 0){
    OCCUPY_INFO *mine_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
    PALACE_TOWN_RESULT town_result;
    
    //ミッションポイント加算
    mine_occupy->intrude_point += point;
    //街の占拠情報反映
    if(Intrude_SearchPalaceTown(result->mission_data.zone_id, &town_result) == TRUE){
      if(result->mission_data.monolith_type == MONOLITH_TYPE_WHITE){
        mine_occupy->town.town_occupy[town_result.tblno] = OCCUPY_TOWN_WHITE;
      }
      else{
        mine_occupy->town.town_occupy[town_result.tblno] = OCCUPY_TOWN_BLACK;
      }
    }
    else{
      GF_ASSERT_MSG(0, "zone_id = %d\n", result->mission_data.zone_id);
    }
    return TRUE;
  }
  return FALSE;
}


//==============================================================================
//
//  ミッションデータ
//
//==============================================================================
//--------------------------------------------------------------
/**
 * ミッションデータをロード
 *
 * @retval  MISSION_CONV_DATA *		ミッションデータへのポインタ
 */
//--------------------------------------------------------------
static MISSION_CONV_DATA * MISSIONDATA_Load(void)
{
  return GFL_ARC_LoadDataAlloc(ARCID_MISSION, NARC_mission_mission_data_bin, HEAPID_WORLD);
}

//--------------------------------------------------------------
/**
 * ミッションデータをアンロード
 *
 * @param   md		ミッションデータへのポインタ
 */
//--------------------------------------------------------------
static void MISSIONDATA_Unload(MISSION_CONV_DATA *md)
{
  GFL_HEAP_FreeMemory(md);
}

//--------------------------------------------------------------
/**
 * ミッションデータの要素数を取得
 *
 * @retval  u32		要素数
 */
//--------------------------------------------------------------
static u32 MISSIONDATA_GetArrayMax(void)
{
  return GFL_ARC_GetDataSize(ARCID_MISSION, NARC_mission_mission_data_bin);
}

//--------------------------------------------------------------
/**
 * ミッションデータを該当条件に合うものを選択してバッファに格納する
 *
 * @param   md		        ミッションデータへのポインタ
 * @param   dest_md		    該当条件にあったデータの代入先
 * @param   md_max		    ミッションデータ要素数
 * @param   choice_type		選択タイプ
 * @param   level		      パレスレベル
 *
 * @retval  BOOL		TRUE:条件に当てはまるものがあった。　FALSE:無かった
 */
//--------------------------------------------------------------
static BOOL MISSIONDATA_Choice(const MISSION_CONV_DATA *cdata, MISSION_CONV_DATA *dest_md, int md_max, MISSION_TYPE choice_type, int level)
{
  int i;
  
  for(i = 0; i < md_max; i++){
    if(cdata[i].type == choice_type && cdata[i].level <= level){
      if(cdata[i].odds == 100 || cdata[i].odds <= GFUser_GetPublicRand(100)){
        GFL_STD_MemCopy(&cdata[i], dest_md, sizeof(MISSION_CONV_DATA));
        return TRUE;
      }
    }
  }
  
  GF_ASSERT_MSG(0, "type=%d, level=%d\n", choice_type, level);
  GFL_STD_MemCopy(&cdata[0], dest_md, sizeof(MISSION_CONV_DATA));
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ターゲット情報をセット
 *
 * @param   intcomm		
 * @param   target	      	代入先
 * @param   accept_netid		ミッション受注者のNetID
 */
//--------------------------------------------------------------
static void MISSION_SetTargetInfo(INTRUDE_COMM_SYS_PTR intcomm, MISSION_TARGET_INFO *target, int accept_netid)
{
  const MYSTATUS *myst;
  
  target->net_id = _TragetNetID_Choice(intcomm, accept_netid);
  
  myst = Intrude_GetMyStatus(intcomm, target->net_id);
  MyStatus_CopyNameStrCode(myst, target->name, PERSON_NAME_SIZE + EOM_SIZE);
}

//--------------------------------------------------------------
/**
 * ミッションデータ毎の拡張パラメータをセットする
 *
 * @param   mdata		        
 * @param   accept_netid		ミッション受注者のNetID
 */
//--------------------------------------------------------------
static void MISSIONDATA_ExtraParamSet(INTRUDE_COMM_SYS_PTR intcomm, MISSION_DATA *mdata, int accept_netid)
{
  MISSION_TYPE_WORK *exwork = &mdata->exwork;
  
  switch(mdata->cdata.type){
  case MISSION_TYPE_VICTORY:     ///<勝利(LV)
    MISSION_SetTargetInfo(intcomm, &exwork->victory.target_info, accept_netid);
    break;
  case MISSION_TYPE_SKILL:       ///<技
    break;
  case MISSION_TYPE_BASIC:       ///<基礎
    MISSION_SetTargetInfo(intcomm, &exwork->victory.target_info, accept_netid);
    break;
  case MISSION_TYPE_SIZE:        ///<大きさ
  case MISSION_TYPE_ATTRIBUTE:   ///<属性
    break;
  case MISSION_TYPE_ITEM:        ///<道具
    MISSION_SetTargetInfo(intcomm, &exwork->victory.target_info, accept_netid);
    break;
  case MISSION_TYPE_OCCUR:       ///<発生(エンカウント)
  case MISSION_TYPE_PERSONALITY: ///<性格
    break;
  }
}

//==================================================================
/**
 * ミッション選択候補リストを作成
 *
 * @param   intcomm		
 * @param   mission		    
 * @param   accept_netid  ミッション受注者のNetID
 * @param   palace_area		選択候補を得る対象のパレスエリア
 */
//==================================================================
void MISSION_MissionList_Create(INTRUDE_COMM_SYS_PTR intcomm, MISSION_SYSTEM *mission, int accept_netid, int palace_area)
{
  MISSION_CHOICE_LIST *list;
  MISSION_TYPE type;
  MISSION_CONV_DATA *cdata;
  u32 md_array_max;
  int palace_level;
  const OCCUPY_INFO *occupy;
  
  list = &mission->list[palace_area];
  if(list->accept_netid != INTRUDE_NETID_NULL){
    return; //既に作成済み
  }
  list->accept_netid = accept_netid;
  list->palace_area = palace_area;
  
  occupy = Intrude_GetOccupyInfo(intcomm, palace_area);
  palace_level = occupy->intrude_level + 1;//ミッションデータが1originの為、合わせて1origin化
  
  cdata = MISSIONDATA_Load();
  md_array_max = MISSIONDATA_GetArrayMax();
  
  occupy = Intrude_GetOccupyInfo(intcomm, palace_area);
  
  for(type = 0; type < MISSION_TYPE_MAX; type++){
    MISSIONDATA_Choice(cdata, &list->md[type].cdata, md_array_max, type, palace_level);
    MISSIONDATA_ExtraParamSet(intcomm, &list->md[type], accept_netid);
  }

  MISSIONDATA_Unload(cdata);
}

//==================================================================
/**
 * ミッション選択候補リストが有効かどうかチェック
 *
 * @param   mission		
 * @param   palace_area		
 *
 * @retval  BOOL		TRUE:有効　FALSE:無効
 */
//==================================================================
BOOL MISSION_MissionList_CheckOcc(MISSION_SYSTEM *mission, int palace_area)
{
  if(mission->list[palace_area].accept_netid != INTRUDE_NETID_NULL){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 生のSTRCODEをWORDSETする
 *
 * @param   wordset		
 * @param   bufID		
 * @param   code		        
 * @param   str_length		  STRCODEの長さ
 * @param   temp_heap_id		テンポラリで使用するヒープID
 */
//--------------------------------------------------------------
static void _Wordset_Strcode(WORDSET *wordset, u32 bufID, const STRCODE *code, int str_length, HEAPID temp_heap_id)
{
  STRBUF *strbuf = 	GFL_STR_CreateBuffer(str_length, temp_heap_id);

	GFL_STR_SetStringCodeOrderLength(strbuf, code, str_length);
  WORDSET_RegisterWord(wordset, bufID, strbuf, 0, TRUE, PM_LANG);

  GFL_STR_DeleteBuffer(strbuf);
}

//==================================================================
/**
 * ミッションデータに対応したメッセージをWORDSETする
 *
 * @param   intcomm		
 * @param   mdata		        ミッションデータへのポインタ
 * @param   wordset		      
 * @param   temp_heap_id		テンポラリで使用するヒープID
 */
//==================================================================
void MISSIONDATA_Wordset(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_DATA *mdata, WORDSET *wordset, HEAPID temp_heap_id)
{
  switch(mdata->cdata.type){
  case MISSION_TYPE_VICTORY:     //勝利(LV)
    {
      const MISSION_TYPEDATA_VICTORY *d_vic = (void*)mdata->cdata.data;
      
      _Wordset_Strcode(wordset, 0, mdata->exwork.victory.target_info.name, 
        PERSON_NAME_SIZE + EOM_SIZE, temp_heap_id);
      
      WORDSET_RegisterNumber(wordset, 1, d_vic->battle_level, 3, 
        STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    }
    break;
  case MISSION_TYPE_SKILL:       //技
    break;
  case MISSION_TYPE_BASIC:       //基礎
    {
      const MISSION_TYPEDATA_BASIC *d_bas = (void*)mdata->cdata.data;
      
      _Wordset_Strcode(wordset, 0, mdata->exwork.basic.target_info.name, 
        PERSON_NAME_SIZE + EOM_SIZE, temp_heap_id);
    }
    break;
  case MISSION_TYPE_SIZE:        //大きさ
    break;
  case MISSION_TYPE_ATTRIBUTE:   //属性
    {
      const MISSION_TYPEDATA_ATTRIBUTE *d_attr = (void*)mdata->cdata.data;
      
      WORDSET_RegisterPlaceName( wordset, 0, d_attr->zone_id );
    }
    break;
  case MISSION_TYPE_ITEM:        //道具
    {
      const MISSION_TYPEDATA_ITEM *d_item = (void*)mdata->cdata.data;
      
      WORDSET_RegisterPlaceName( wordset, 0, d_item->item_no );
      
      _Wordset_Strcode(wordset, 1, mdata->exwork.item.target_info.name, 
        PERSON_NAME_SIZE + EOM_SIZE, temp_heap_id);
    }
    break;
  case MISSION_TYPE_OCCUR:       //発生(エンカウント)
  case MISSION_TYPE_PERSONALITY: //性格
    break;
  }
  
  //制限時間
  WORDSET_RegisterNumber(wordset, 2, mdata->cdata.time, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  //報酬1位～4位
  WORDSET_RegisterNumber(
    wordset, 3, mdata->cdata.reward[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(
    wordset, 4, mdata->cdata.reward[1], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(
    wordset, 5, mdata->cdata.reward[2], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(
    wordset, 6, mdata->cdata.reward[3], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
}
