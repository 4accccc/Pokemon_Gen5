//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_wfbc_data.c
 *	@brief  White Forest  Black City 基本データ 常駐
 *	@author	tomoya takahashi
 *	@date		2009.11.10
 *
 *	モジュール名：FIELD_WFBC_CORE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "field/field_wfbc_data.h"
#include "field/zonedata.h"

#include "system/net_err.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
///	通信バッファマスク
//=====================================
typedef enum {
  FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_0,
  FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_END = FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_0 + FIELD_COMM_MEMBER_MAX,
  FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS = FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_END,
  FIELD_WFBC_COMM_BUFF_MSK_SEND_REQ,
  FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_0,
  FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_END = FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_0 + FIELD_COMM_MEMBER_MAX,
    
} FIELD_WFBC_COMM_BUFF_MSK;
#define FIELD_WFBC_COMM_BUFF_MSK_CHECK(val, x)   ((val) & (1<<(x)))
#define FIELD_WFBC_COMM_BUFF_MSK_ON(val, x)   ((val) | (1<<(x)))
#define FIELD_WFBC_COMM_BUFF_MSK_OFF(val, x)   ((val) & ~(1<<(x)))

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	WFBC_COMM_DATA
//=====================================
static BOOL WFBC_COMM_DATA_IsError( const WFBC_COMM_DATA* cp_wk );

static void WFBC_COMM_DATA_MainOya( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc );

static BOOL WFBC_COMM_DATA_Oya_AnserThere( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans );
static BOOL WFBC_COMM_DATA_Oya_AnserWishTake( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans );
static BOOL WFBC_COMM_DATA_Oya_AnserTake( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans );



static void WFBC_COMM_DATA_SetRecvCommAnsData( WFBC_COMM_DATA* p_wk, const FIELD_WFBC_COMM_NPC_ANS* cp_ans );
static void WFBC_COMM_DATA_SetRecvCommReqData( WFBC_COMM_DATA* p_wk, u16 netID, const FIELD_WFBC_COMM_NPC_REQ* cp_req );
static BOOL WFBC_COMM_DATA_GetRecvCommAnsData( const WFBC_COMM_DATA* cp_wk, FIELD_WFBC_COMM_NPC_ANS* p_ans );
static BOOL WFBC_COMM_DATA_GetRecvCommReqData( const WFBC_COMM_DATA* cp_wk, u16 netID, FIELD_WFBC_COMM_NPC_REQ* p_req );
static void WFBC_COMM_DATA_SetSendCommAnsData( WFBC_COMM_DATA* p_wk, u16 netID, u16 npc_id, FIELD_WFBC_COMM_NPC_ANS_TYPE ans_type );
static void WFBC_COMM_DATA_SetSendCommReqData( WFBC_COMM_DATA* p_wk, u16 npc_id, FIELD_WFBC_COMM_NPC_REQ_TYPE req_type );
static BOOL WFBC_COMM_DATA_GetSendCommAnsData( const WFBC_COMM_DATA* cp_wk, u16 netID, FIELD_WFBC_COMM_NPC_ANS* p_ans );
static BOOL WFBC_COMM_DATA_GetSendCommReqData( const WFBC_COMM_DATA* cp_wk, FIELD_WFBC_COMM_NPC_REQ* p_req );

static void WFBC_COMM_DATA_ClearRecvCommAnsData( WFBC_COMM_DATA* p_wk );
static void WFBC_COMM_DATA_ClearRecvCommReqData( WFBC_COMM_DATA* p_wk, u16 netID );
static void WFBC_COMM_DATA_ClearSendCommAnsData( WFBC_COMM_DATA* p_wk, u16 netID );
static void WFBC_COMM_DATA_ClearSendCommReqData( WFBC_COMM_DATA* p_wk );




//----------------------------------------------------------------------------
/**
 *	@brief  ZONEDATAにWFBCの設定を行う
 *
 *	@param	cp_wk   ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_SetUpZoneData( const FIELD_WFBC_CORE* cp_wk )
{
  if( cp_wk->type == FIELD_WFBC_CORE_TYPE_WHITE_FOREST )
  {
    ZONEDATA_SetChangeZoneID( ZONEDATA_CHANGE_BC_WF_ID, TRUE );
  }
  if( cp_wk->type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    ZONEDATA_SetChangeZoneID( ZONEDATA_CHANGE_BC_WF_ID, FALSE );
  }
}


//-------------------------------------
///	FIELD_WFBC_CORE用関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  WFBCワークのクリア
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_Clear( FIELD_WFBC_CORE* p_wk )
{
  int i;

  GF_ASSERT( p_wk );
  
  p_wk->data_in = FALSE;
  p_wk->type    = FIELD_WFBC_CORE_TYPE_MAX;

  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    FIELD_WFBC_CORE_PEOPLE_Clear( &p_wk->people[i] );
    FIELD_WFBC_CORE_PEOPLE_Clear( &p_wk->back_people[i] );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  人を探す  NPCIDの人
 *
 *	@param	p_wk      ワーク
 *	@param	npc_id    NPC　ID
 *	@param  mapmode   マップモード
 *
 *	mapmode
 *	  マップモード
 *      MAPMODE_NORMAL,     ///<通常状態
 *      MAPMODE_INTRUDE,    ///<侵入中
 *      MAPMODE_MAX,        ///<両方をチェック
 *
 *	@retval 人ワーク
 *	@retval NULL    いない
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_CORE_PEOPLE* FIELD_WFBC_CORE_GetNpcIDPeople( FIELD_WFBC_CORE* p_wk, u32 npc_id, MAPMODE mapmode )
{
  int i;

  GF_ASSERT( p_wk );
  GF_ASSERT( npc_id < FIELD_WFBC_NPCID_MAX );

  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    if( (mapmode == MAPMODE_NORMAL) || (mapmode == MAPMODE_MAX) )
    {
      if( FIELD_WFBC_CORE_PEOPLE_IsInData( &p_wk->people[i] ) )
      {
        if( p_wk->people[i].npc_id == npc_id )
        {
          return &p_wk->people[i];
        }
      }
    }

    if( (mapmode == MAPMODE_INTRUDE) || (mapmode == MAPMODE_MAX) )
    {
      if( FIELD_WFBC_CORE_PEOPLE_IsInData( &p_wk->back_people[i] ) )
      {
        if( p_wk->back_people[i].npc_id == npc_id )
        {
          return &p_wk->back_people[i];
        }
      }
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  いるのかチェック
 *
 *	@param	p_wk      ワーク
 *	@param	npc_id    NPC　ID
 *	@param  mapmode   マップモード
 *
 *	mapmode
 *	  マップモード
 *      MAPMODE_NORMAL,     ///<通常状態
 *      MAPMODE_INTRUDE,    ///<侵入中
 *      MAPMODE_MAX,        ///<両方をチェック
 *
 *	@retval TRUE  いる
 *	@retval FALSE いない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_IsOnNpcIDPeople( const FIELD_WFBC_CORE* cp_wk, u32 npc_id, MAPMODE mapmode )
{
  // 内部でワークを書き換えるわけではないので、許可
  if( FIELD_WFBC_CORE_GetNpcIDPeople( (FIELD_WFBC_CORE*)cp_wk, npc_id, mapmode ) == NULL )
  {
    return FALSE;
  }
  return TRUE;
}

//-------------------------------------
///	FIELD_WFBC_CORE_PEOPLE用関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  人物ワークのクリア
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_PEOPLE_Clear( FIELD_WFBC_CORE_PEOPLE* p_wk )
{
  GF_ASSERT( p_wk );

  p_wk->data_in = FALSE;
  p_wk->npc_id  = 0;
  p_wk->mood    = 0;
  p_wk->one_day_msk  = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  情報の整合性チェック
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE    正常
 *	@retval FALSE   不正
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_PEOPLE_IsConfomity( const FIELD_WFBC_CORE_PEOPLE* cp_wk )
{
  GF_ASSERT( cp_wk );

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ調整  不正データの場合は、正常な情報に書き換えます。
 *
 *	@param	p_wk  人物ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_PEOPLE_Management( FIELD_WFBC_CORE_PEOPLE* p_wk )
{
  GF_ASSERT( p_wk );

  // npc_idが範囲内？

  // 機嫌値が最大値以上？

  // 親の名前に終端コードがある？

}

//----------------------------------------------------------------------------
/**
 *	@brief  データの有無   不正データの場合、FALSEを返します。
 *
 *	@param	cp_wk   人物ワーク
 *
 *	@retval TRUE    データあり
 *	@retval FALSE   データなし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_PEOPLE_IsInData( const FIELD_WFBC_CORE_PEOPLE* cp_wk )
{
  GF_ASSERT( cp_wk );
  
  if( !cp_wk->data_in )
  {
    return FALSE;
  }
  if( FIELD_WFBC_CORE_PEOPLE_IsConfomity( cp_wk ) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}





//----------------------------------------------------------------------------
/**
 *	@brief  アイテム配置情報すべてくりあ　
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
void WFBC_CORE_ITEM_ClaerAll( FIELD_WFBC_CORE_ITEM* p_wk )
{
  int i;
  
  // アイテムなし
  for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ )
  {
    p_wk->scr_item[i] = FIELD_WFBC_ITEM_NONE;
  }
}




//----------------------------------------------------------------------------
/**
 *	@brief  ワークのクリア
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_EVENT_Clear( FIELD_WFBC_EVENT* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(FIELD_WFBC_EVENT) );
  p_wk->bc_npc_win_target = FIELD_WFBC_EVENT_NPC_WIN_TARGET_INIT;
}





//-----------------------------------------------------------------------------
/**
 *					通信情報
*/
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  通信情報のクリア
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_Init( WFBC_COMM_DATA* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WFBC_COMM_DATA) );

  p_wk->netID = GFL_NET_SystemGetCurrentID();
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信情報の破棄
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_Exit( WFBC_COMM_DATA* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WFBC_COMM_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信情報のメイン処理
 *
 *	@param	p_wk      ワーク
 *	@param	p_wfbc    WFBC情報
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_Oya_Main( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc )
{
  // エラー中は空回り
  if( WFBC_COMM_DATA_IsError( p_wk ) ){
    return ;
  }
  
  // 親の処理
  if( GFL_NET_IsParentMachine() )
  {
    WFBC_COMM_DATA_MainOya( p_wk, p_mywfbc );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  子機側、親のセーブデータから子側にNPCの人を移す
 *
 *	@param	p_wk        ワーク
 *	@param	p_mywfbc    自分のWFBC
 *	@param	p_oyawfbc   親のWFBC
 *	@param  npc_id      NPCID
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_Ko_ChangeNpc( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, FIELD_WFBC_CORE* p_oyawfbc, const MYSTATUS* cp_mystatus, u16 npc_id )
{
  FIELD_WFBC_CORE_PEOPLE* p_people;

  p_people = FIELD_WFBC_CORE_GetNpcIDPeople( p_oyawfbc, npc_id, MAPMODE_INTRUDE );

  // いないのはおかしい
  GF_ASSERT( p_people );

  // 登録
  FIELD_WFBC_CORE_AddPeople( p_mywfbc, cp_mystatus, p_people );

  // 親の情報からは破棄
  FIELD_WFBC_CORE_PEOPLE_Clear( p_people );
}


//----------------------------------------------------------------------------
/**
 *	@brief  受信応答情報の設定  （子側の処理）
 *
 *	@param	p_wk        ワーク
 *	@param	cp_ans      受信応答情報
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_SetRecvCommAnsData( WFBC_COMM_DATA* p_wk, const FIELD_WFBC_COMM_NPC_ANS* cp_ans )
{
  // エラー中は空回り
  if( WFBC_COMM_DATA_IsError( p_wk ) ){
    return ;
  }

  WFBC_COMM_DATA_SetRecvCommAnsData( p_wk, cp_ans );
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信リクエスト情報の設定  （親側の処理）
 *
 *	@param	p_wk        ワーク
 *	@param	netID       送信者ネットID
 *	@param	cp_req      受信リクエスト情報
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_SetRecvCommReqData( WFBC_COMM_DATA* p_wk, u16 netID, const FIELD_WFBC_COMM_NPC_REQ* cp_req )
{
  // エラー中は空回り
  if( WFBC_COMM_DATA_IsError( p_wk ) ){
    return ;
  }

  WFBC_COMM_DATA_SetRecvCommReqData( p_wk, netID, cp_req );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信応答情報の取得    （親側の処理）
 *
 *	@param	cp_wk     ワーク
 *	@param	netID     送信先ネットID
 *	@param	p_ans     送信応答情報格納先
 *
 *	@retval TRUE    情報がある
 *	@retval FLASE   情報なし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_COMM_DATA_GetSendCommAnsData( const WFBC_COMM_DATA* cp_wk, u16 netID, FIELD_WFBC_COMM_NPC_ANS* p_ans )
{
  return WFBC_COMM_DATA_GetSendCommAnsData( cp_wk, netID, p_ans );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信リクエスト情報の取得  （子側の処理）
 *
 *	@param	cp_wk   ワーク
 *	@param	p_req   送信リクエスト情報の格納先
 *
 *	@retval TRUE  情報がある
 *	@retval FALSE 情報なし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_COMM_DATA_GetSendCommReqData( const WFBC_COMM_DATA* cp_wk, FIELD_WFBC_COMM_NPC_REQ* p_req )
{
  return WFBC_COMM_DATA_GetSendCommReqData( cp_wk, p_req );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信応答情報バッファのクリア  （送信できたらクリアしてください）
 *
 *	@param	p_wk    ワーク
 *	@param	netID   ネットID
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_ClearSendCommAnsData( WFBC_COMM_DATA* p_wk, u16 netID )
{
  WFBC_COMM_DATA_ClearSendCommAnsData( p_wk, netID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信リクエスト情報バッファのクリア  （送信できたらクリアしてください）
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_ClearSendCommReqData( WFBC_COMM_DATA* p_wk )
{
  WFBC_COMM_DATA_ClearSendCommReqData( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信リクエスト情報の設定
 *
 *	@param	p_wk      ワーク
 *	@param	npc_id    対象のNPCID
 *	@param	req_type  リクエストタイプ
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_SetReqData( WFBC_COMM_DATA* p_wk, u16 npc_id, FIELD_WFBC_COMM_NPC_REQ_TYPE req_type )
{
  
  if( WFBC_COMM_DATA_IsError(p_wk) == FALSE )
  {
    WFBC_COMM_DATA_SetSendCommReqData( p_wk, npc_id, req_type );
  }
  else
  {
    static const FIELD_WFBC_COMM_NPC_ANS_TYPE sc_REQ_TO_ANS[ FIELD_WFBC_COMM_NPC_REQ_TYPE_NUM ] = 
    {
      FIELD_WFBC_COMM_NPC_ANS_OFF,
      FIELD_WFBC_COMM_NPC_ANS_TAKE_NG,
      FIELD_WFBC_COMM_NPC_ANS_TYPE_NUM,
    };
    FIELD_WFBC_COMM_NPC_ANS ans;
    // エラー時の処理
    // req_typeに対するNGを受信したことにする
    if( sc_REQ_TO_ANS[ req_type ] != FIELD_WFBC_COMM_NPC_ANS_TYPE_NUM )
    {
      ans.net_id  = p_wk->netID;
      ans.npc_id  = npc_id;
      ans.ans_type= sc_REQ_TO_ANS[ req_type ];
      WFBC_COMM_DATA_SetRecvCommAnsData( p_wk, &ans );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  応答情報待ち
 *
 *	@param	cp_wk   ワーク
 *	@param	npc_id  リクエストを出したNPCID
 *
 *	@retval TRUE  応答がきた！
 *	@retval FLASE 応答がこない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_COMM_DATA_WaitAnsData( const WFBC_COMM_DATA* cp_wk, u16 npc_id )
{
  if(FIELD_WFBC_COMM_BUFF_MSK_CHECK( cp_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS ))
  {
    // これがずれるということは、何か進行状態にずれがあります。
    GF_ASSERT( npc_id == cp_wk->recv_ans.npc_id );
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  応答内容を取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return 応答内容
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_COMM_NPC_ANS_TYPE FIELD_WFBC_GetAnsData( const WFBC_COMM_DATA* cp_wk )
{
  GF_ASSERT( FIELD_WFBC_COMM_BUFF_MSK_CHECK( cp_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS ) );
  return cp_wk->recv_ans.ans_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  リクエストを出す前に呼んでください
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_COMM_DATA_ClearReqAnsData( WFBC_COMM_DATA* p_wk )
{
  WFBC_COMM_DATA_ClearSendCommReqData( p_wk );
  WFBC_COMM_DATA_ClearRecvCommAnsData( p_wk );
}










//-----------------------------------------------------------------------------
/**
 *    private関数
 */
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *    通信用データ
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  エラー状態チェック
 *
 *	@retval TRUE    エラー
 *	@retval FALSE   通常
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_IsError( const WFBC_COMM_DATA* cp_wk )
{
  
  
  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE )
  {
    return TRUE;
  }

  if( cp_wk->netID == GFL_NET_NO_PARENTMACHINE )
  {
    return TRUE;
  }

  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  通信情報のメイン処理　親
 *
 *	@param	p_wk      ワーク
 *	@param	p_wfbc    WFBC情報
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_MainOya( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc )
{
  int i;
  FIELD_WFBC_COMM_NPC_ANS ans;
  FIELD_WFBC_COMM_NPC_REQ req;
  BOOL result;
  BOOL ans_result;
  BOOL npc_in;
  static BOOL (*pAnser[FIELD_WFBC_COMM_NPC_REQ_TYPE_NUM])( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans ) = 
  {
    WFBC_COMM_DATA_Oya_AnserThere,
    WFBC_COMM_DATA_Oya_AnserWishTake,
    WFBC_COMM_DATA_Oya_AnserTake,
  };
  
  // 子からのリクエストに答える。
  for( i=0; i<FIELD_COMM_MEMBER_MAX; i++ )
  {
    result = WFBC_COMM_DATA_GetRecvCommReqData( p_wk, i, &req );

    if( result )
    {
      ans_result = pAnser[ req.req_type ]( p_wk, p_mywfbc, &req, &ans );
    }

    WFBC_COMM_DATA_ClearRecvCommReqData( p_wk, i );
    if( ans_result )
    {
      WFBC_COMM_DATA_SetSendCommAnsData( p_wk, i, ans.npc_id, ans.ans_type );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  いるのかの応答処理
 *
 *	@param	p_wk        ワーク
 *	@param	p_mywfbc    自分のWFBC情報
 *	@param	cp_req      リクエスト情報
 *	@param	p_ans       応答格納先
 *
 *	@retval TRUE    応答がある
 *	@retval FALSE   応答がない
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_Oya_AnserThere( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans )
{
  
  p_ans->net_id = cp_req->net_id;
  p_ans->npc_id = cp_req->npc_id;

  if( FIELD_WFBC_CORE_GetNpcIDPeople( p_mywfbc, cp_req->npc_id, MAPMODE_INTRUDE ) )
  {
    // いる
    p_ans->ans_type = FIELD_WFBC_COMM_NPC_ANS_ON;
  }
  else
  {
    // いない
    p_ans->ans_type = FIELD_WFBC_COMM_NPC_ANS_OFF;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  連れて行っていいか？
 *
 *	@param	p_wk        ワーク
 *	@param	p_mywfbc    自分のWFBC情報
 *	@param	cp_req      リクエスト情報
 *	@param	p_ans       応答格納先
 *
 *	@retval TRUE    応答がある
 *	@retval FALSE   応答がない
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_Oya_AnserWishTake( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans )
{
  p_ans->net_id = cp_req->net_id;
  p_ans->npc_id = cp_req->npc_id;

  if( FIELD_WFBC_CORE_GetNpcIDPeople( p_mywfbc, cp_req->npc_id, MAPMODE_INTRUDE ) )
  {
    // つれていってください
    p_ans->ans_type = FIELD_WFBC_COMM_NPC_ANS_TAKE_OK;
  }
  else
  {
    // つれていけません
    p_ans->ans_type = FIELD_WFBC_COMM_NPC_ANS_TAKE_NG;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人を連れて行かれてしまった処理。
 *
 *	@param	p_wk        ワーク
 *	@param	p_mywfbc    自分のWFBC情報
 *	@param	cp_req      リクエスト情報
 *	@param	p_ans       応答格納先
 *
 *	@retval TRUE    応答がある
 *	@retval FALSE   応答がない
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_Oya_AnserTake( WFBC_COMM_DATA* p_wk, FIELD_WFBC_CORE* p_mywfbc, const FIELD_WFBC_COMM_NPC_REQ* cp_req, FIELD_WFBC_COMM_NPC_ANS* p_ans )
{
  FIELD_WFBC_CORE_PEOPLE* p_people;

  // その人の情報を消す。
  p_people = FIELD_WFBC_CORE_GetNpcIDPeople( p_mywfbc, cp_req->npc_id, MAPMODE_INTRUDE );
  if( p_people )
  {
    FIELD_WFBC_CORE_PEOPLE_Clear( p_people );
  }
  
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  受信応答情報の設定    （子用）
 *
 *	@param	p_wk      ワーク
 *	@param	cp_ans    応答データ
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_SetRecvCommAnsData( WFBC_COMM_DATA* p_wk, const FIELD_WFBC_COMM_NPC_ANS* cp_ans )
{
  // 自分宛？
  if( cp_ans->net_id == p_wk->netID )
  {
    GF_ASSERT( FIELD_WFBC_COMM_BUFF_MSK_CHECK(p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS) == 0 );
    GFL_STD_MemCopy( cp_ans, &p_wk->recv_ans, sizeof(FIELD_WFBC_COMM_NPC_ANS) );
    p_wk->buff_msk = FIELD_WFBC_COMM_BUFF_MSK_ON( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信リクエスト情報の設定  （親用）
 *
 *	@param	p_wk      ワーク
 *	@param	netID     ネットID
 *	@param	cp_req    リクエストデータ
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_SetRecvCommReqData( WFBC_COMM_DATA* p_wk, u16 netID, const FIELD_WFBC_COMM_NPC_REQ* cp_req )
{
  GF_ASSERT( FIELD_WFBC_COMM_BUFF_MSK_CHECK(p_wk->buff_msk, (FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_0+netID)) == 0 );
  GF_ASSERT( netID < FIELD_COMM_MEMBER_MAX );
  GFL_STD_MemCopy( cp_req, &p_wk->recv_req_que[netID], sizeof(FIELD_WFBC_COMM_NPC_REQ) );
  p_wk->buff_msk = FIELD_WFBC_COMM_BUFF_MSK_ON(p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_0+netID);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信応答情報の取得  （子用）
 *
 *	@param	p_wk    ワーク
 *	@param	p_ans   応答情報の取得先
 *
 *	@retval TRUE    受信データあり
 *	@retval FALSE   受信データなし
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_GetRecvCommAnsData( const WFBC_COMM_DATA* cp_wk, FIELD_WFBC_COMM_NPC_ANS* p_ans )
{
  BOOL ret = FALSE;

  if( FIELD_WFBC_COMM_BUFF_MSK_CHECK(cp_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS) )
  {
    GFL_STD_MemCopy( &cp_wk->recv_ans, p_ans, sizeof(FIELD_WFBC_COMM_NPC_ANS) );
    ret = TRUE;
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信リクエスト情報の取得  （親用）
 *
 *	@param	cp_wk     ワーク
 *	@param	netID     ネットID
 *	@param	p_req     リクエスト情報の格納先
 *
 *	@retval TRUE    受信データあり
 *	@retval FALSE   受信データなし
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_GetRecvCommReqData( const WFBC_COMM_DATA* cp_wk, u16 netID, FIELD_WFBC_COMM_NPC_REQ* p_req )
{
  BOOL ret = FALSE;
  GF_ASSERT( netID < FIELD_COMM_MEMBER_MAX );

  if( FIELD_WFBC_COMM_BUFF_MSK_CHECK( cp_wk->buff_msk, (FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_0+netID) ) )
  {
    GFL_STD_MemCopy( &cp_wk->recv_req_que[netID], p_req, sizeof(FIELD_WFBC_COMM_NPC_REQ) );
    ret = TRUE;
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信応答情報の設定  (親用)
 *
 *	@param	p_wk    ワーク
 *	@param	netID   ネットID
 *	@param	cp_ans  応答情報
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_SetSendCommAnsData( WFBC_COMM_DATA* p_wk, u16 netID, u16 npc_id, FIELD_WFBC_COMM_NPC_ANS_TYPE ans_type )
{
  GF_ASSERT( FIELD_WFBC_COMM_BUFF_MSK_CHECK( p_wk->buff_msk, (FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_0+netID) ) == 0 );
  GF_ASSERT( netID < FIELD_COMM_MEMBER_MAX );
  
  p_wk->send_ans_que[netID].net_id    = netID;
  p_wk->send_ans_que[netID].npc_id    = npc_id;
  p_wk->send_ans_que[netID].ans_type  = ans_type;
  
  p_wk->buff_msk = FIELD_WFBC_COMM_BUFF_MSK_ON( p_wk->buff_msk, (FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_0+netID) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信リクエスト情報の設定  （子用）
 *
 *	@param	p_wk      ワーク
 *	@param  npc_id    NPCID
 *	@param  req_type  リクエストタイプ
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_SetSendCommReqData( WFBC_COMM_DATA* p_wk, u16 npc_id, FIELD_WFBC_COMM_NPC_REQ_TYPE req_type )
{
  GF_ASSERT( FIELD_WFBC_COMM_BUFF_MSK_CHECK( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_SEND_REQ ) == 0 );

  p_wk->send_req.net_id     = p_wk->netID;
  p_wk->send_req.npc_id     = npc_id;
  p_wk->send_req.req_type   = req_type;

  p_wk->buff_msk = FIELD_WFBC_COMM_BUFF_MSK_ON( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_SEND_REQ );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信応答情報の取得  （親用）
 *
 *	@param	cp_wk   ワーク
 *	@param	netID   ネットID
 *	@param	p_ans   応答情報格納先
 *
 *	@retval TRUE    応答情報がある
 *	@retval FALSE   応答情報がない
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_GetSendCommAnsData( const WFBC_COMM_DATA* cp_wk, u16 netID, FIELD_WFBC_COMM_NPC_ANS* p_ans )
{
  BOOL ret = FALSE;

  GF_ASSERT( netID < FIELD_COMM_MEMBER_MAX );

  if( FIELD_WFBC_COMM_BUFF_MSK_CHECK( cp_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_0+netID ) )
  {
    GFL_STD_MemCopy( &cp_wk->send_ans_que[netID], p_ans, sizeof(FIELD_WFBC_COMM_NPC_ANS) );
    ret = TRUE;
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信リクエスト情報の取得  （子用）
 *
 *	@param	cp_wk   ワーク
 *	@param	p_req   リクエスト情報格納先
 *
 *	@retval TRUE    応答情報がある
 *	@retval FALSE   応答情報がない
 */
//-----------------------------------------------------------------------------
static BOOL WFBC_COMM_DATA_GetSendCommReqData( const WFBC_COMM_DATA* cp_wk, FIELD_WFBC_COMM_NPC_REQ* p_req )
{
  BOOL ret = FALSE;

  if( FIELD_WFBC_COMM_BUFF_MSK_CHECK( cp_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_SEND_REQ ) )
  {
    GFL_STD_MemCopy( &cp_wk->send_req, p_req, sizeof(FIELD_WFBC_COMM_NPC_REQ) );
    ret = TRUE;
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信応答情報のクリア
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_ClearRecvCommAnsData( WFBC_COMM_DATA* p_wk )
{
  GFL_STD_MemClear( &p_wk->recv_ans, sizeof(FIELD_WFBC_COMM_NPC_ANS) );
  p_wk->buff_msk  = FIELD_WFBC_COMM_BUFF_MSK_OFF( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_ANS );
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信リクエスト情報のクリア
 *
 *	@param	p_wk    ワーク
 *	@param	netID   ネットID
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_ClearRecvCommReqData( WFBC_COMM_DATA* p_wk, u16 netID )
{
  GF_ASSERT( netID < FIELD_COMM_MEMBER_MAX );
  
  GFL_STD_MemClear( &p_wk->recv_req_que[netID], sizeof(FIELD_WFBC_COMM_NPC_REQ) );
  p_wk->buff_msk  = FIELD_WFBC_COMM_BUFF_MSK_OFF( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_RECV_REQ_0+netID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信応答情報のクリア
 *
 *	@param	p_wk    ワーク
 *	@param	netID   ネットID
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_ClearSendCommAnsData( WFBC_COMM_DATA* p_wk, u16 netID )
{
  GF_ASSERT( netID < FIELD_COMM_MEMBER_MAX );
  
  GFL_STD_MemClear( &p_wk->send_ans_que[netID], sizeof(FIELD_WFBC_COMM_NPC_ANS) );
  p_wk->buff_msk  = FIELD_WFBC_COMM_BUFF_MSK_OFF( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_SEND_ANS_0+netID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  送信リクエスト情報のクリア
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WFBC_COMM_DATA_ClearSendCommReqData( WFBC_COMM_DATA* p_wk )
{
  GFL_STD_MemClear( &p_wk->send_req, sizeof(FIELD_WFBC_COMM_NPC_REQ) );
  p_wk->buff_msk  = FIELD_WFBC_COMM_BUFF_MSK_OFF( p_wk->buff_msk, FIELD_WFBC_COMM_BUFF_MSK_SEND_REQ );
}



