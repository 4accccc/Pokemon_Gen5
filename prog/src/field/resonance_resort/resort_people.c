//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_people.c
 *	@brief  レゾナンスリゾート人物管理
 *	@author	Toru=Nagihashi
 *	@data		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "net_app/union/union_beacon_tool.h"

//mine
#include "resort_map.h"
#include "resort_data_manager.h"

//outer
#include "resort_people.h"

//=============================================================================
/**
 *				人物ワーク
 *				  人物個別操作、または人物情報設定や読み取り
*/
//=============================================================================

//-------------------------------------
///	人物抽象動作
//=====================================
typedef struct
{
  void (*SetupMmdHeader)( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
  u32 (*GetData)( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
  void (*SetData)( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );
} RESORT_PEOPLE_FUNCTION;

//-------------------------------------
///	人物ワーク
//=====================================
struct _RESORT_PEOPLE
{
  u16 peopleID;
  MMDLSYS * p_mmdlsys;
  void *p_data_adrs;
  const RESORT_BINARY_PEOPLE_DATA *cp_binary;
  const RESORT_PEOPLE_FUNCTION *cp_function;
  BOOL (*Action)( RESORT_PEOPLE *p_wk, u32 *p_seq, FIELD_PLAYER *p_player );
  u32 action_seq;
};

//-------------------------------------
///	人物初期化ワーク
//=====================================
typedef struct
{
  RESORT_PEOPLE_TYPE people_type;
  void *p_data_adrs;
  u16 peopleID;
  ZONEID zoneID;
  MMDLSYS * p_mmdlsys;
  const RESORT_BINARY_PEOPLE *cp_binary;
  FIELD_PLAYER *p_player;       //プレイヤー
}RESORT_PEOPLE_INIT_PARAM;

//-------------------------------------
///	PRIVATE
//=====================================
//住民
static void Resident_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
static u32 Resident_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
static void Resident_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );

//代理人
static void Proxy_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
static u32 Proxy_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
static void Proxy_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );

//観光客
static void Tourist_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
static u32 Tourist_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
static void Tourist_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );

//滞在者
static void Visitor_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
static u32 Visitor_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
static void Visitor_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );

//秘書
static void Secretary_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
static u32 Secretary_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
static void Secretary_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );

//NPC
static void Npc_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );
static u32 Npc_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs );
static void Npc_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data );

//行動関数
static BOOL Action_WarpDown( RESORT_PEOPLE *p_wk, u32 *p_seq, FIELD_PLAYER *p_player );
static BOOL Action_WarpUp( RESORT_PEOPLE *p_wk, u32 *p_seq, FIELD_PLAYER *p_player );

//-------------------------------------
///	抽象動作
//=====================================
static const RESORT_PEOPLE_FUNCTION sc_people_function[RESORT_PEOPLE_TYPE_MAX]  =
{
  //RESORT_PEOPLE_TYPE_RESIDENT
  {
    Resident_SetupMmdHeader,
    Resident_GetData,
    Resident_SetData,
  },
  //RESORT_PEOPLE_TYPE_PROXY
  {
    Proxy_SetupMmdHeader,
    Proxy_GetData,
    Proxy_SetData,
  },
  //RESORT_PEOPLE_TYPE_TOURIST
  {
    Tourist_SetupMmdHeader,
    Tourist_GetData,
    Tourist_SetData,
  },
  //RESORT_PEOPLE_TYPE_VISITOR
  {
    Visitor_SetupMmdHeader,
    Visitor_GetData,
    Visitor_SetData,
  },
  //RESORT_PEOPLE_TYPE_SECRETARY
  {
    Secretary_SetupMmdHeader,
    Secretary_GetData,
    Secretary_SetData,
  },
  //RESORT_PEOPLE_TYPE_NPC
  {
    Npc_SetupMmdHeader,
    Npc_GetData,
    Npc_SetData,
  },
};

//-------------------------------------
///	MMDLHEADER基本情報
//=====================================
static const MMDL_HEADER sc_default_header = 
{
	0,		///<識別ID
	0,	  ///<表示するOBJコード
	0,	  ///<動作コード
	0,	  ///<イベントタイプ
	0,	  ///<イベントフラグ
	0,	  ///<イベントID
	0,		///<指定方向
	0,		///<指定パラメタ 0
	0,		///<指定パラメタ 1
	0,		///<指定パラメタ 2
	0,		///<X方向移動制限
	0,		///<Z方向移動制限
};

//-------------------------------------
///	ソース内PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク作成
 *
 *	@param	HEAPID heapID　ヒープID
 *	@param	RESORT_PEOPLE_SYS_ADD_PARAM *cp_param 追加用引数
 *
 *	@return 人物ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE * RESORT_PEOPLE_Init( HEAPID heapID, const RESORT_PEOPLE_INIT_PARAM *cp_param )
{
  RESORT_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_PEOPLE));
  p_wk->peopleID    = cp_param->peopleID;
  p_wk->p_mmdlsys   = cp_param->p_mmdlsys;
  p_wk->p_data_adrs = cp_param->p_data_adrs;
  p_wk->cp_binary   = RESORT_BINARY_SearchData( cp_param->cp_binary, cp_param->people_type, 0 );//TODO kind
  p_wk->cp_function = &sc_people_function[ cp_param->people_type ];

  {
    MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );
    if( p_mmdl == NULL )
    {
      RESORT_PLACEID placeID = RESORT_MAP_ZoneIDToPlaceID( cp_param->zoneID );
      u16 grid_x, grid_z;
      MMDL_HEADER header = sc_default_header;

      header.id = RESORT_PEOPLE_PEOPLEID_TO_MMDLID(cp_param->peopleID);
      p_wk->cp_function->SetupMmdHeader( p_wk, &header );

      RESORT_MAP_GetPlaceGridPosByIndex( placeID, cp_param->peopleID, &grid_x, &grid_z );

      MMDLHEADER_SetGridPos( &header, grid_x, grid_z, 0 );
      MMDLSYS_AddMMdl( cp_param->p_mmdlsys, &header, cp_param->zoneID );

      //作成した場合は、データとMMDLをひもづけるため、データにPEOPLEIDとPLACEIDを設定
      RESORT_PEOPLE_SetData( p_wk, RESORT_PEOPLE_PARAM_PEOPLEID, cp_param->peopleID );
      RESORT_PEOPLE_SetData( p_wk, RESORT_PEOPLE_PARAM_PLACEID, placeID );
    }
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物ワーク破棄
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_PEOPLE_Exit( RESORT_PEOPLE *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物ワークメイン処理
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_PEOPLE_Main( RESORT_PEOPLE *p_wk,FIELD_PLAYER *p_player )
{
  //アクション実行
  if( p_wk->Action )
  {
    if( p_wk->Action( p_wk, &p_wk->action_seq, p_player ) )
    {
      //終了したらNULL
      p_wk->Action = NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief MMDLを比較
 *
 *	@param	const RESORT_PEOPLE *cp_wk  ワーク
 *	@param	MMDL 
 *
 *	@return TRUEならば同じMMDL　FALSEならば違うMMDL
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_PEOPLE_IsCompMmdl( const RESORT_PEOPLE *cp_wk, const MMDL *cp_mmdl)
{
  return cp_mmdl == RESORT_PEOPLE_GetMmdl( cp_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  データを比較
 *
 *	@param	const RESORT_PEOPLE *cp_wk  ワーク
 *	@param	void *cp_data_adrs          比較するデータ
 *
 *	@return TRUEならば同じデータ　FALSEならば違うデータ
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_PEOPLE_IsCompData( const RESORT_PEOPLE *cp_wk, const void *cp_data_adrs )
{
  return cp_wk->p_data_adrs == cp_data_adrs;
}

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief 個別行動設定
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *	@param	action 行動させるもの
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ActionReq( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_ACTION action )
{
  p_wk->action_seq = 0;

  switch( action )
  {
  case RESORT_PEOPLE_ACTION_WARPDOWN:  //下にワープ
    p_wk->Action = Action_WarpDown;
    break;
  case RESORT_PEOPLE_ACTION_WARPUP:    //上にワープ
    p_wk->Action = Action_WarpUp;
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アクションをしていないかチェック
 *
 *	@param	const RESORT_PEOPLE *cp_wk ワーク
 *
 *	@return TRUEならば何もアクションをしていない　FALSEでアクション実行中
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PEOPLE_IsActionEnd( const RESORT_PEOPLE *cp_wk )
{
  return cp_wk->Action == NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示設定
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *	@param	is_visible  TRUEで表示  FALSEで非表示
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SetVisible( RESORT_PEOPLE *p_wk, BOOL is_visible )
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );
  MMDL_SetStatusBitVanish( p_mmdl, !is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief データを設定
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *	@param	param 引数
 *	@param	data  設定するデータ
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
  p_wk->cp_function->SetData( p_wk, param, data );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	const RESORT_PEOPLE *cp_wk ワーク
 *	@param	param   引数
 *	@param  p_ret_adrs バッファ
 *
 *	@return　データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  return cp_wk->cp_function->GetData( cp_wk, param, p_ret_adrs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ番号を取得
 *
 *	@param	const RESORT_PEOPLE *cp_wkワーク
 *	@param	index インデックス 
 *
 *	@return MSGID
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_GetMsgID( const RESORT_PEOPLE *cp_wk, u32 index )
{
  return RESORT_BINARY_PEOPLE_DATA_GetMsgID( cp_wk->cp_binary, index );
}
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLを取得
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *
 *	@return MMDL
 */
//-----------------------------------------------------------------------------
MMDL * RESORT_PEOPLE_GetMmdl( const RESORT_PEOPLE *cp_wk )
{
  return MMDLSYS_SearchOBJID( cp_wk->p_mmdlsys, RESORT_PEOPLE_PEOPLEID_TO_MMDLID( cp_wk->peopleID ) ); 
}

//-------------------------------------
///住民
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_hedder           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Resident_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder )
{
  RESORT_RESIDENT_DATA *p_data =p_wk->p_data_adrs;

  p_hedder->obj_code = GIRL2; //TODO
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ設定・取得
 */
//-----------------------------------------------------------------------------
static u32 Resident_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  const RESORT_RESIDENT_DATA *cp_data = cp_wk->p_data_adrs;
  return 0;//TODO
}
static void Resident_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
}

//-------------------------------------
///代理人
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_hedder           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Proxy_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder )
{
  RESORT_PROXY_DATA *p_data =p_wk->p_data_adrs;
  //u8 trainer_view = RESORT_RECV_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_TRAINERVIEW, NULL );
  p_hedder->obj_code = GIRL2;//UnionView_GetObjCode( trainer_view );
  p_hedder->move_code = MV_DOWN; //TODO
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ取得・設定
 */
//-----------------------------------------------------------------------------
static u32 Proxy_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  const RESORT_PROXY_DATA *cp_data = cp_wk->p_data_adrs;
  switch( param )
  {
  case RESORT_PEOPLE_PARAM_PEOPLEID:
    return RESORT_PROXY_DATA_GetData( cp_data, RESORT_PROXY_DATA_PARAM_PEOPLEID );
  case RESORT_PEOPLE_PARAM_PLACEID:
    return RESORT_PROXY_DATA_GetData( cp_data, RESORT_PROXY_DATA_PARAM_PLACEID );
  case RESORT_PEOPLE_PARAM_SEX:
    return 1;//TODO
  case RESORT_PEOPLE_PARAM_TRAINERVIEW:
    return 9;//TODO;
  case RESORT_PEOPLE_PARAM_NAME:
    {
      //TODO
      static const STRCODE sc_strcode[PERSON_NAME_SIZE+EOM_SIZE] =
      {
        L'だ',
        L'い',
        L'り',
        L'に',
        L'ん',
        0xffff
      };
      GFL_STD_MemCopy( sc_strcode, p_ret_adrs, sizeof(u16)*(PERSON_NAME_SIZE+EOM_SIZE) );
    }
    return 0;
  case RESORT_PEOPLE_PARAM_LOCK:
    return 0;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
static void Proxy_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
 RESORT_PROXY_DATA *p_data = p_wk->p_data_adrs;
  switch( param )
  {
  case RESORT_PEOPLE_PARAM_PEOPLEID:
    RESORT_PROXY_DATA_SetData( p_data, RESORT_PROXY_DATA_PARAM_PEOPLEID, data );
    break;
  case RESORT_PEOPLE_PARAM_PLACEID:
    RESORT_PROXY_DATA_SetData( p_data, RESORT_PROXY_DATA_PARAM_PLACEID, data );
    break;
  case RESORT_PEOPLE_PARAM_LOCK:
    /* なにもしない */
    break;
  case RESORT_PEOPLE_PARAM_KEEP:
    /* なにもしない */
    break;
  default:
    GF_ASSERT(0);
  }
}

//-------------------------------------
///観光客
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_hedder           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Tourist_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder )
{
  RESORT_RECV_DATA *p_data = p_wk->p_data_adrs;
  u8 trainer_view = RESORT_RECV_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_TRAINERVIEW, NULL );
  p_hedder->obj_code = UnionView_GetObjCode( trainer_view );
  p_hedder->move_code = MV_DOWN; //TODO
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ取得・設定
 */
//-----------------------------------------------------------------------------
static u32 Tourist_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  const RESORT_RECV_DATA *cp_data = cp_wk->p_data_adrs;
  switch( param )
  {
  case RESORT_PEOPLE_PARAM_PEOPLEID:
    return RESORT_RECV_DATA_GetData( cp_data, RESORT_RECV_DATA_PARAM_PEOPLE_ID, NULL );
  case RESORT_PEOPLE_PARAM_PLACEID:
    return RESORT_RECV_DATA_GetData( cp_data, RESORT_RECV_DATA_PARAM_PLACE_ID, NULL );
  case RESORT_PEOPLE_PARAM_LOCK:
    return RESORT_RECV_DATA_GetData( cp_data , RESORT_RECV_DATA_PARAM_LOCK, NULL );
  case RESORT_PEOPLE_PARAM_SEX:
    return RESORT_RECV_DATA_GetData( cp_data , RESORT_RECV_DATA_PARAM_SEX, NULL );
  case RESORT_PEOPLE_PARAM_TRAINERVIEW:
    return RESORT_RECV_DATA_GetData( cp_data , RESORT_RECV_DATA_PARAM_TRAINERVIEW, NULL );
  case RESORT_PEOPLE_PARAM_NAME:
    RESORT_RECV_DATA_GetData( cp_data , RESORT_RECV_DATA_PARAM_NAME, p_ret_adrs );
    return 0;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
static void Tourist_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
  RESORT_RECV_DATA *p_data = p_wk->p_data_adrs;
  switch( param )
  {
  case RESORT_PEOPLE_PARAM_PEOPLEID:
    RESORT_RECV_DATA_SetData( p_data, RESORT_RECV_DATA_PARAM_PEOPLE_ID, data );
    break;
  case RESORT_PEOPLE_PARAM_PLACEID:
    RESORT_RECV_DATA_SetData( p_data, RESORT_RECV_DATA_PARAM_PLACE_ID, data );
    break;
  case RESORT_PEOPLE_PARAM_LOCK:
    RESORT_RECV_DATA_SetData( p_data , RESORT_RECV_DATA_PARAM_LOCK, data );
    break;
  case RESORT_PEOPLE_PARAM_KEEP:
    RESORT_RECV_DATA_SetData( p_data , RESORT_RECV_DATA_PARAM_LOCK, data );
    break;
  default:
    GF_ASSERT(0);
  }
}

//-------------------------------------
///滞在者
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_hedder           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Visitor_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder )
{
  RESORT_VISITOR_DATA *p_data = p_wk->p_data_adrs;
  p_hedder->obj_code = GIRL2; //TODO
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ取得・設定
 */
//-----------------------------------------------------------------------------
static u32 Visitor_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  return 0;//TODO
}
static void Visitor_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
}

//-------------------------------------
///秘書
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_hedder           MMDL作成用ヘッダ
 *	@param	RESORT_BINARY_PEOPLE *cp_binary バイナリデータ
 */
//-----------------------------------------------------------------------------
static void Secretary_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder )
{
  RESORT_SECRETARY_DATA *p_data = p_wk->p_data_adrs;
  p_hedder->obj_code = GIRL2; //TODO
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary  );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ取得・設定
 */
//-----------------------------------------------------------------------------
static u32 Secretary_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  return 0; //TODO
}
static void Secretary_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
}

//-------------------------------------
///NPC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_hedder           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Npc_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder )
{
  p_hedder->obj_code = GIRL2; //TODO
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物データ取得・設定
 */
//-----------------------------------------------------------------------------
static u32 Npc_GetData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_PARAM param, void *p_ret_adrs )
{
  return 0; //TODO
}
static void Npc_SetData( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_PARAM param, u32 data )
{
}
//-------------------------------------
///ACTION
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  ワープして落ちてくる
 *
 *	@param	RESORT_PEOPLE *p_wk　ワーク
 *	@param	*p_seq シーケンス
 *
 *	@return TRUEで行動終了　FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Action_WarpDown( RESORT_PEOPLE *p_wk, u32 *p_seq, FIELD_PLAYER *p_player )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
  };
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );

  switch( *p_seq )
  {
  case SEQ_START:
    if(MMDL_CheckPossibleAcmd(p_mmdl))
    {
      MMDL_GRIDPOS grid_pos, player_pos;
      MMDL_GetGridPos( FIELD_PLAYER_GetMMdl( p_player ), &player_pos );
      MMDL_GetGridPos( p_mmdl, &grid_pos );

      if(grid_pos.gx != player_pos.gx || grid_pos.gz != player_pos.gz)
      {
        MMDL_SetAcmd(p_mmdl, AC_WARP_DOWN);
        MMDL_SetStatusBitVanish( p_mmdl, FALSE );
        (*p_seq)++;
      }
    }
    break;
  case SEQ_WAIT:
    if(MMDL_CheckEndAcmd(p_mmdl))
    {
      MMDL_EndAcmd(p_mmdl);
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワープしてあがっていく
 *
 *	@param	RESORT_PEOPLE *p_wk　ワーク
 *	@param	*p_seq シーケンス
 *
 *	@return TRUEで行動終了　FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Action_WarpUp( RESORT_PEOPLE *p_wk, u32 *p_seq, FIELD_PLAYER *p_player )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
  };
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );

  switch( *p_seq )
  {
  case SEQ_START:
    if(MMDL_CheckPossibleAcmd(p_mmdl))
    {
      MMDL_SetAcmd(p_mmdl, AC_WARP_UP);
      (*p_seq)++;
    }
    break;
  case SEQ_WAIT:
    if(MMDL_CheckEndAcmd(p_mmdl) == TRUE)
    {
      MMDL_EndAcmd(p_mmdl);
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

//=============================================================================
/**
 *					人物管理ワーク
 *					  人物ワークの作成、破棄、一括処理
*/
//=============================================================================
//-------------------------------------
///	人物管理ワーク
//=====================================
struct _RESORT_PEOPLE_SYS
{
  RESORT_PEOPLE_SYS_PARAM param;
  HEAPID heapID;
  RESORT_BINARY_PEOPLE *p_binary;
  RESORT_PEOPLE *p_people[0];
};

//-------------------------------------
///	PRIVATE
//=====================================
static RESORT_PEOPLE * RESORT_PEOPLE_SYS_Add( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param, u16 peopleID );

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク　初期化処理
 *
 *	@param	const RESORT_PEOPLE_SYS_PARAM *cp_param 引数
 *	@param	heapID 　ヒープID
 *
 *	@return　人物管理ワーク
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_SYS * RESORT_PEOPLE_SYS_Init( const RESORT_PEOPLE_SYS_PARAM *cp_param, HEAPID heapID )
{
  u32 work_size = sizeof(RESORT_PEOPLE_SYS)+sizeof(RESORT_PEOPLE *)*cp_param->max;
  RESORT_PEOPLE_SYS *p_wk = GFL_HEAP_AllocClearMemory(heapID, work_size );
  p_wk->param  = *cp_param;
  p_wk->heapID = heapID;

  p_wk->p_binary = RESORT_BINARY_PEOPLE_Init( heapID );

  RESORT_PEOPLE_SYS_AddAll( p_wk );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク　終了処理
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_Exit( RESORT_PEOPLE_SYS *p_wk )
{
  RESORT_PEOPLE_SYS_RemoveAll( p_wk );

  RESORT_BINARY_PEOPLE_Exit( p_wk->p_binary );

  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク　メイン処理
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_Main( RESORT_PEOPLE_SYS *p_wk,FIELD_PLAYER *p_player )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = p_wk->p_people[i];
    if( p_people )
    {
      RESORT_PEOPLE_Main( p_people, p_player );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  データから人物を追加
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_AddAll( RESORT_PEOPLE_SYS *p_wk )
{
  RESORT_PLACEID placeID;
  int peopleID;

  RESORT_DATA_MANAGER_INIT_PARAM param;
  RESORT_DATA_MANAGER *p_info;
  
  param.p_all = p_wk->param.p_people_all_data;
  param.p_recv_sv_buff = p_wk->param.p_recv_sv_buff;
  param.p_recv_tmp_buff = p_wk->param.p_recv_recep_buff;
  param.p_proxy_buff = p_wk->param.p_proxy_buff;
  p_info =  RESORT_DATA_MANAGER_Init( &param, GFL_HEAP_LOWID(p_wk->heapID) );

  //場所情報から存在しているデータのみ人物登録する
  for( placeID = RESORT_PLACEID_START; placeID < RESORT_PLACEID_MAX; placeID++ )
  {
    for( peopleID = 0; peopleID < RESORT_DATA_MANAGER_MAX; peopleID++ )
    {
      if( !RESORT_DATA_MANAGER_IsPlaceEmpty( p_info, placeID, peopleID ) )
      {
        void *p_data_adrs;
        RESORT_DATA_MANAGER_TYPE type;
        u16 data_idx;

        RESORT_DATA_MANAGER_GetPlaceData( p_info, placeID, peopleID, &type, &data_idx );
        p_data_adrs = RESORT_DATA_MANAGER_GetDataAdrs( p_info, type, data_idx );

        {
          RESORT_PEOPLE_SYS_ADD_PARAM param;
          GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_ADD_PARAM) );
          param.people_type = RESORT_DATA_MANAGER_GetPeopleType( type );
          param.p_data_adrs = p_data_adrs;
          param.zoneID      = p_wk->param.zoneID;
          RESORT_PEOPLE_SYS_Add( p_wk, &param, peopleID );
        }
      }
    }
  }

  RESORT_DATA_MANAGER_Exit( p_info );

}

//----------------------------------------------------------------------------
/**
 *	@brief  人物を一人追加
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	RESORT_PEOPLE_SYS_ADD_PARAM *cp_param   引数
 *	@param  peopleID 人物ID
 *
 *	@return 人物ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE * RESORT_PEOPLE_SYS_Add( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param, u16 peopleID )
{
  int i = peopleID;
  if( p_wk->p_people[i] == NULL )
  {
    RESORT_PEOPLE_INIT_PARAM init_param;
    GFL_STD_MemClear( &init_param, sizeof(RESORT_PEOPLE_INIT_PARAM) );
    init_param.people_type  = cp_param->people_type;
    init_param.p_data_adrs  = cp_param->p_data_adrs;
    init_param.peopleID     = peopleID;
    init_param.zoneID       = cp_param->zoneID;
    init_param.cp_binary    = p_wk->p_binary;
    init_param.p_mmdlsys    = p_wk->param.p_mmdlsys;

    p_wk->p_people[i] = RESORT_PEOPLE_Init( p_wk->heapID, &init_param );
    return p_wk->p_people[i];
  }

  GF_ASSERT(0);
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物を一人新規追加
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	RESORT_PEOPLE_SYS_ADD_PARAM *cp_param   引数
 *
 *	@return 人物ワーク
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * RESORT_PEOPLE_SYS_AddNew( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    //開いているところを検索して追加
    if( p_wk->p_people[i] == NULL )
    {
      RESORT_MAP_ZoneIDToPlaceID( cp_param->zoneID );
      return RESORT_PEOPLE_SYS_Add( p_wk, cp_param,  i );
    }
  }

  GF_ASSERT(0);
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief 人物ワークを削除
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	*p_people   人物ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_Remove( RESORT_PEOPLE_SYS *p_wk, RESORT_PEOPLE *p_people )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    if( p_people == p_wk->p_people[i] )
    {
      RESORT_PEOPLE_Exit( p_people );
      p_wk->p_people[i] = NULL;
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物ワークを全て削除
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_RemoveAll( RESORT_PEOPLE_SYS *p_wk )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = p_wk->p_people[i];
    if( p_people )
    {
      RESORT_PEOPLE_Exit( p_people );
      p_wk->p_people[i] = NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物を取得  MMDL版
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	MMDL* cp_mmdl   MMDL
 *
 *	@return 人物
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByMmdl( RESORT_PEOPLE_SYS *p_wk, const MMDL* cp_mmdl )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = p_wk->p_people[i];
    if( p_people )
    {
      if( RESORT_PEOPLE_IsCompMmdl( p_people, cp_mmdl ) )
      {
        return p_people;
      }
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物を取得  データ版
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	const void *cp_data_adrs データ
 *
 *	@return 人物
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByData( RESORT_PEOPLE_SYS *p_wk, const void *cp_data_adrs )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = p_wk->p_people[i];
    if( p_people )
    {
      if( RESORT_PEOPLE_IsCompData( p_people, cp_data_adrs ) )
      {
        return p_people;
      }
    }
  }
  return NULL;
}
