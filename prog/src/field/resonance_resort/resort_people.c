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
} RESORT_PEOPLE_FUNCTION;

//-------------------------------------
///	人物ワーク
//=====================================
struct _RESORT_PEOPLE
{
  RESORT_PEOPLE_TYPE people_type;
  u16   peopleID;
  BOOL  is_talk;
  MMDLSYS * p_mmdlsys;
  RESORT_ABSTRUCT_DATA *p_abstruct_data;
  const RESORT_BINARY_PEOPLE_DATA *cp_binary;
  const RESORT_PEOPLE_FUNCTION *cp_function;
};

//-------------------------------------
///	人物初期化ワーク
//=====================================
typedef struct
{
  RESORT_ABSTRUCT_DATA *p_data;
  u16 peopleID;
  RESORT_PLACEID placeID;
  MMDLSYS * p_mmdlsys;
  const RESORT_BINARY_PEOPLE *cp_binary;
  FIELD_PLAYER *p_player;       //プレイヤー
}RESORT_PEOPLE_INIT_PARAM;

//-------------------------------------
///	PRIVATE
//=====================================
//住民
static void Resident_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );

//代理人
static void Proxy_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );

//観光客
static void Tourist_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );

//秘書
static void Secretary_SetupMmdHeader( RESORT_PEOPLE *p_wk, MMDL_HEADER *p_hedder );

//-------------------------------------
///	抽象動作
//=====================================
static const RESORT_PEOPLE_FUNCTION sc_people_function[RESORT_PEOPLE_TYPE_MAX]  =
{
  //RESORT_PEOPLE_TYPE_RESIDENT
  {
    Resident_SetupMmdHeader,
  },
  //RESORT_PEOPLE_TYPE_PROXY
  {
    Proxy_SetupMmdHeader,
  },
  //RESORT_PEOPLE_TYPE_TOURIST
  {
    Tourist_SetupMmdHeader,
  },
  //RESORT_PEOPLE_TYPE_SECRETARY
  {
    Secretary_SetupMmdHeader,
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
  int kind;

  RESORT_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_PEOPLE));
  p_wk->peopleID    = cp_param->peopleID;
  p_wk->p_mmdlsys   = cp_param->p_mmdlsys;
  p_wk->p_abstruct_data = cp_param->p_data;
  p_wk->people_type = RESORT_ABSTRUCT_DATA_GetPeopleType( p_wk->p_abstruct_data );

  kind = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TYPE, NULL );
  p_wk->cp_binary   = RESORT_BINARY_SearchData( cp_param->cp_binary, p_wk->people_type, kind );

  p_wk->cp_function = &sc_people_function[ p_wk->people_type ];

  {
    MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );
    if( p_mmdl == NULL )
    {
      u16 grid_x, grid_z, dir;
      MMDL_HEADER header = sc_default_header;

      header.id = RESORT_PEOPLE_PEOPLEID_TO_MMDLID(cp_param->peopleID);
      p_wk->cp_function->SetupMmdHeader( p_wk, &header );

      if( p_wk->people_type == RESORT_PEOPLE_TYPE_TOURIST
          || p_wk->people_type == RESORT_PEOPLE_TYPE_PROXY )
      {
        grid_x  = 0;
        grid_z  = 0;
        dir = DIR_UP;
      }
      else if( p_wk->people_type == RESORT_PEOPLE_TYPE_RESIDENT )
      {
        RESORT_MAP_GetResidentGridPos( RESORT_ABSTRUCT_DATA_GetBufferIndex(p_wk->p_abstruct_data), 0, &grid_x, &grid_z, &dir );
      }
      else
      {
        RESORT_MAP_GetPlaceGridPosByIndex( cp_param->placeID, p_wk->people_type, kind, cp_param->peopleID, &grid_x, &grid_z, &dir );
      }


      header.dir = dir;

      MMDLHEADER_SetGridPos( &header, grid_x, grid_z, 0 );
      MMDLSYS_AddMMdl( cp_param->p_mmdlsys, &header, RESORT_MAP_PlaceIDToZoneID(cp_param->placeID) );

      //作成した場合は、データとMMDLをひもづけるため、データにPEOPLEIDとPLACEIDを設定
      RESORT_PEOPLE_SetData( p_wk, RESORT_DATA_PARAM_PEOPLE_ID, cp_param->peopleID );
      RESORT_PEOPLE_SetData( p_wk, RESORT_DATA_PARAM_PLACE_ID, cp_param->placeID );
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
 *	@brief  人物に登録してるデータアドレスを取得
 *
 *	@param	RESORT_PEOPLE *p_wk   ワーク
 *
 *	@return データアドレス
 */
//-----------------------------------------------------------------------------
void * RESORT_PEOPLE_GetDataAdrs( RESORT_PEOPLE *p_wk )
{
  return RESORT_ABSTRUCT_DATA_GetDataAdrs( p_wk->p_abstruct_data );
}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJコードを変更
 *
 *	@param	RESORT_PEOPLE *p_people 人物
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ChangeOBJCode( RESORT_PEOPLE *p_wk )
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );
  u16 code = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
  MMDL_ChangeOBJCode( p_mmdl, UnionView_GetObjCode( code ) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  話しかけ中フラグ操作
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *	@param	is_talk TRUEで話せる  FALSEで話せない
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SetTalkFlag( RESORT_PEOPLE *p_wk, BOOL is_talk )
{
  p_wk->is_talk = is_talk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  話しかけられているかチェック
 *
 *	@param	const RESORT_PEOPLE *cp_wk  ワーク
 *
 *	@return TRUE話しかけられている  FALSE話しかけられていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PEOPLE_GetTalkFlag( const RESORT_PEOPLE *cp_wk )
{
  return cp_wk->is_talk;
}
//----------------------------------------------------------------------------
/**
 *	@brief 施設データを取得
 *
 *	@param	RESORT_PEOPLE *p_wk   ワーク
 *
 *	@return 施設データ
 */
//-----------------------------------------------------------------------------
RESORT_INSTITUTION_DATA * RESORT_PEOPLE_GetInstitution( RESORT_PEOPLE *p_wk )
{
  RESORT_RESIDENT_DATA *p_resident = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_wk->p_abstruct_data );

  GF_ASSERT( p_wk->people_type == RESORT_PEOPLE_TYPE_RESIDENT );
  return RESORT_RESIDENT_DATA_GetInstitution( p_resident );
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
  return RESORT_ABSTRUCT_DATA_GetDataAdrs( cp_wk->p_abstruct_data ) == cp_data_adrs;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを比較
 *
 *	@param	const RESORT_PEOPLE *cp_wk  ワーク
 *	@param	RESORT_ABSTRUCT_DATA *cp_abstruct 比較するデータ
 *
 *	@return TRUEならば同じデータ　FALSEならば違うデータ
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_PEOPLE_IsCompAbstructData( const RESORT_PEOPLE *cp_wk, const RESORT_ABSTRUCT_DATA *cp_abstruct )
{
  return cp_wk->p_abstruct_data == cp_abstruct;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを初期化
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_PEOPLE_ClearData( RESORT_PEOPLE *p_wk )
{
  RESORT_ABSTRUCT_DATA_InitInstance( p_wk->p_abstruct_data );
}

//-------------------------------------
///	PUBLIC
//=====================================
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
void RESORT_PEOPLE_SetData( RESORT_PEOPLE *p_wk, RESORT_DATA_PARAM param, u32 data )
{
  RESORT_ABSTRUCT_DATA_SetData( p_wk->p_abstruct_data, param, data );
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
u32 RESORT_PEOPLE_GetData( const RESORT_PEOPLE *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs )
{
  return RESORT_ABSTRUCT_DATA_GetData( cp_wk->p_abstruct_data, param, p_ret_adrs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物のデータを取得
 *
 *	@param	const RESORT_PEOPLE *cp_wk  ワーク
 *	@param	param   引数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_GetPeopleData( const RESORT_PEOPLE *cp_wk, RESORT_PEOPLE_DATA_PARAM param )
{
  switch( param )
  {
  case RESORT_PEOPLE_DATA_PARAM_TYPE:
    return cp_wk->people_type;
  default:
    GF_ASSERT(0);
    return 0;
  }
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
//----------------------------------------------------------------------------
/**
 *	@brief  抽象データ取得
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *
 *	@return 抽象データ
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_PEOPLE_GetAbstructData( RESORT_PEOPLE *p_wk )
{
  return p_wk->p_abstruct_data;
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
  u8 trainer_view = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
  p_hedder->obj_code = UnionView_GetObjCode( trainer_view );
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
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
  u8 trainer_view = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
  p_hedder->obj_code = UnionView_GetObjCode( trainer_view );
  p_hedder->move_code = MV_DOWN;
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
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
  u8 trainer_view = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
  NAGI_Printf( "mmdl view %d\n", trainer_view );
  p_hedder->obj_code = UnionView_GetObjCode( trainer_view );
  p_hedder->move_code = MV_DOWN;
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary );
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
  u8 trainer_view = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
  p_hedder->obj_code = UnionView_GetObjCode( trainer_view );
  p_hedder->event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary  );
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
  //音符マークのコマンドは、MMDLの内部でフィールドエフェクトタスクを
  //使っている。フィールドエフェクトタスクはフィールドが破棄されると
  //破棄→作成されるが、MMDLの内部でタスクを作り保持→再生しているため
  //すべて消す必要がある
  {
    int i;
    for( i = 0; i < p_wk->param.max; i++ )
    {
      RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByIndex( p_wk, i );
      if( p_people )
      {
        MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_people );
        if( AC_MARK_ONPU_NO_SE == MMDL_GetAcmdCode( p_mmdl ) )
        {
          MMDL_FreeAcmd( p_mmdl );
        }
      }
    }
  }

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
 *	@brief  人物管理ワーク  人物数取得
 *
 *	@param	const RESORT_PEOPLE_SYS *cp_wk  ワーク
 *
 *	@return 最大値
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_SYS_GetPeopleMax( const RESORT_PEOPLE_SYS *cp_wk )
{
  return cp_wk->param.max;
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
  RESORT_PLACEID placeID = p_wk->param.placeID;
  int peopleID;

  //場所情報から存在しているデータのみ人物登録する
  for( peopleID = 0; peopleID < RESORT_DATA_MANAGER_MAX; peopleID++ )
  {
    if( !RESORT_DATA_MANAGER_IsPlaceEmpty( p_wk->param.p_data_manager, placeID, peopleID ) )
    {
      RESORT_PEOPLE_SYS_ADD_PARAM param;
      GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_ADD_PARAM) );
      param.p_data      =  RESORT_DATA_MANAGER_GetAbstructDataByPlace( p_wk->param.p_data_manager, placeID, peopleID );
      param.placeID     = p_wk->param.placeID;
      RESORT_PEOPLE_SYS_Add( p_wk, &param, peopleID );
    }
  }

  //作成してバインドできなかったMMDLは消す
  //->拡張バッファ対策(セーブ復帰時は拡張ばっふぁが消えている)
  {
    MMDL *p_mmdl;
    u32 no  = 0;
    while( MMDLSYS_SearchUseMMdl(p_wk->param.p_mmdlsys,&p_mmdl,&no) == TRUE ){
      if( MMDL_GetOBJID(p_mmdl) >= RESORT_PEOPLE_MMDLID_OFS
          && MMDL_GetZoneID(p_mmdl) == RESORT_MAP_PlaceIDToZoneID(p_wk->param.placeID)
          && RESORT_PEOPLE_SYS_GetPeopleByMmdl(p_wk,p_mmdl) == NULL )
      {
        MMDL_Delete( p_mmdl );
      }
    }
  }
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
    init_param.p_data       = cp_param->p_data;
    init_param.peopleID     = peopleID;
    init_param.placeID      = cp_param->placeID;
    init_param.cp_binary    = p_wk->p_binary;
    init_param.p_mmdlsys    = p_wk->param.p_mmdlsys;

    NAGI_Printf( "Add=%d\n",peopleID );
    p_wk->p_people[i] = RESORT_PEOPLE_Init( p_wk->heapID, &init_param );
    return p_wk->p_people[i];
  }

  GF_ASSERT_MSG(0,"peopleID=%d\n", peopleID);
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
  u32 peopleID = RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( p_wk->param.p_data_manager, cp_param->placeID );

  //開いているところを検索して追加
  if( peopleID != RESORT_PEOPLE_ID_NULL  )
  {
    RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_Add( p_wk, cp_param,  peopleID );
    NAGI_Printf( "AddNew=%d\n", peopleID );
    RESORT_DATA_MANAGER_Update( p_wk->param.p_data_manager );
    return p_people;
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
 *	@brief  人物に関係するMMDLを削除
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_EraseMMdl( RESORT_PEOPLE_SYS *p_wk )
{
  u32 no = 0;
  MMDL *mmdl;

  while( MMDLSYS_SearchUseMMdl(p_wk->param.p_mmdlsys,&mmdl,&no) == TRUE ){
    if( MMDL_GetOBJID(mmdl) >= RESORT_PEOPLE_MMDLID_OFS
        && MMDL_GetZoneID(mmdl) == RESORT_MAP_PlaceIDToZoneID(p_wk->param.placeID) )
    {
      MMDL_Delete( mmdl );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物ワークを完全に消す（データも動作モデルも）
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk
 *	@param	*p_people 
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_Delete( RESORT_PEOPLE_SYS *p_wk, RESORT_PEOPLE *p_people )
{
  MMDL *p_mmdl=  RESORT_PEOPLE_GetMmdl( p_people );

  //モデルを消す
  MMDL_Delete( p_mmdl );

  //データを消す
  RESORT_PEOPLE_ClearData( p_people );

  //人物を消す
  RESORT_PEOPLE_SYS_Remove( p_wk, p_people );

  //消したので更新
  RESORT_DATA_MANAGER_Update( p_wk->param.p_data_manager );
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
//----------------------------------------------------------------------------
/**
 *	@brief  人物を取得  インデックス版
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	u32 idx　インデックス
 *
 *	@return 人物
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByIndex( RESORT_PEOPLE_SYS *p_wk, u32 idx )
{
  GF_ASSERT( idx < p_wk->param.max );
  return p_wk->p_people[idx];
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物を取得  抽象データ版
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	RESORT_ABSTRUCT_DATA *cp_abstruct 抽象データ
 *
 *	@return 人物
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByAbstruct( RESORT_PEOPLE_SYS *p_wk, const RESORT_ABSTRUCT_DATA *cp_abstruct )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = p_wk->p_people[i];
    if( p_people )
    {
      if( RESORT_PEOPLE_IsCompAbstructData( p_people, cp_abstruct ) )
      {
        return p_people;
      }
    }
  }
  return NULL;
}
