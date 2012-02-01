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

#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_nagihashi_toru) | defined(DEBUG_ONLY_FOR_shimoyamada)
#define DEBUG_PEOPLE_BUFF_PRINT
#endif
#endif //PM_DEBUG

//=============================================================================
/**
 *				人物ワーク
 *				  人物個別操作、または人物情報設定や読み取り
*/
//=============================================================================
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
  const RESORT_BINARY_SHOP   *cp_shop;
  const RESORT_BINARY_INSTITUTION *cp_institution;
  FIELD_PLAYER *p_player;       //プレイヤー
  RESORT_PEOPLE_CREATE  create;
}RESORT_PEOPLE_INIT_PARAM;

//-------------------------------------
///	人物抽象動作
//=====================================
typedef struct
{
  void (*SetupMmdHeader)( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header );
  void (*GetMapPos)( const RESORT_PEOPLE *cp_wk, u16 *p_x, u16 *p_z, u16 *p_dir );
} RESORT_PEOPLE_FUNCTION;

//-------------------------------------
///	人物ワーク
//=====================================
struct _RESORT_PEOPLE
{
  RESORT_PEOPLE_TYPE people_type;
  u32   peopleID;
  BOOL  is_talk;
  BOOL  is_master_visible;
  RESORT_PEOPLE_CREATE  create;
  MMDLSYS * p_mmdlsys;
  MMDL    * p_mmdl;
  RESORT_ABSTRUCT_DATA *p_abstruct_data;
  const RESORT_BINARY_PEOPLE_DATA *cp_binary;
  const RESORT_PEOPLE_FUNCTION *cp_function;
  const RESORT_BINARY_SHOP   *cp_shop;
  const RESORT_BINARY_INSTITUTION *cp_institution;
};

//-------------------------------------
///	PRIVATE
//=====================================
//住民
static void Resident_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header );
static void Resident_GetMapPos( const RESORT_PEOPLE *cp_wk, u16 *p_x, u16 *p_z, u16 *p_dir );

//代理人
static void Proxy_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header );

//観光客
static void Tourist_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header );

//秘書
static void Secretary_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header );
static void Secretary_GetMapPos( const RESORT_PEOPLE *cp_wk, u16 *p_x, u16 *p_z, u16 *p_dir );

//その他
static void Resort_People_CreateMmdl( RESORT_PEOPLE *p_wk,  const RESORT_PEOPLE_INIT_PARAM *cp_param );

#ifdef DEBUG_PEOPLE_BUFF_PRINT
static void Resort_People_PrintBuff( RESORT_PEOPLE_SYS *p_wk );
#endif

//-------------------------------------
///	抽象動作
//=====================================
static const RESORT_PEOPLE_FUNCTION sc_people_function[RESORT_PEOPLE_TYPE_MAX]  =
{
  //RESORT_PEOPLE_TYPE_RESIDENT
  {
    Resident_SetupMmdHeader,
    Resident_GetMapPos,
  },
  //RESORT_PEOPLE_TYPE_PROXY
  {
    Proxy_SetupMmdHeader,
    NULL,
  },
  //RESORT_PEOPLE_TYPE_TOURIST
  {
    Tourist_SetupMmdHeader,
    NULL,
  },
  //RESORT_PEOPLE_TYPE_SECRETARY
  {
    Secretary_SetupMmdHeader,
    Secretary_GetMapPos,
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
  int sex;

  RESORT_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_PEOPLE));
  p_wk->peopleID    = cp_param->peopleID;
  p_wk->p_mmdlsys   = cp_param->p_mmdlsys;
  p_wk->p_abstruct_data = cp_param->p_data;
  p_wk->create      = cp_param->create;
  p_wk->cp_shop     = cp_param->cp_shop;
  p_wk->cp_institution = cp_param->cp_institution;

  //人物生成時には必ず不正検査を行う  //重いのでやらなくなりました
  //RESORT_ABSTRUCT_DATA_ModifyProfWord( p_wk->p_abstruct_data, heapID );

  p_wk->people_type = RESORT_ABSTRUCT_DATA_GetPeopleType( p_wk->p_abstruct_data );

  kind = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_TYPE, NULL );
  sex  = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_SEX, NULL );
  p_wk->cp_binary   = RESORT_BINARY_SearchData( cp_param->cp_binary, p_wk->people_type, kind, sex );

  p_wk->cp_function = &sc_people_function[ p_wk->people_type ];

  //もし自分のMMDLがなかったら作成
  //（メニュー・セーブ復帰など存在することがありえる）
  {
    p_wk->p_mmdl = MMDLSYS_SearchOBJID( p_wk->p_mmdlsys, RESORT_PEOPLE_PEOPLEID_TO_MMDLID( p_wk->peopleID ) );
      
    if( p_wk->p_mmdl == NULL )
    {
      Resort_People_CreateMmdl( p_wk,  cp_param );
    }
    p_wk->is_master_visible = !MMDL_CheckStatusBitVanish( p_wk->p_mmdl );
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
  u16 code = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_OBJCODE, NULL );
  MMDL_ChangeOBJCode( p_mmdl, code );
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
 *	@brief  MMDLを削除→生成
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_PEOPLE_ResetMmdl( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param )
{
  if( p_wk->p_mmdl )
  {
    MMDL_Delete( p_wk->p_mmdl );
    p_wk->p_mmdl = NULL;
  }
  Resort_People_CreateMmdl( p_wk,  cp_param );
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
 *	@brief  位置を再セット
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ResetPos( RESORT_PEOPLE *p_wk )
{
  if( p_wk->cp_function->GetMapPos )
  {
    u16 grid_x, grid_z, dir;
    VecFx32 vec;

    MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );

    p_wk->cp_function->GetMapPos( p_wk, &grid_x, &grid_z, &dir );

    vec.x = GRID_SIZE_FX32( grid_x ) + MMDL_VEC_X_GRID_OFFS_FX32;
    vec.y = 0;
    vec.z = GRID_SIZE_FX32( grid_z ) + MMDL_VEC_Z_GRID_OFFS_FX32;

    MMDL_SetVectorPos( p_mmdl, &vec );
    MMDL_SetGridPosX( p_mmdl, grid_x );
    MMDL_SetGridPosZ( p_mmdl, grid_z );
    MMDL_SetDirDisp( p_mmdl, dir );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設の前の位置を取得
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *	@param	*p_x    X
 *	@param	*p_z    Z
 *	@param	*p_dir  方向
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_GetInstitutionFrontPos( RESORT_PEOPLE *p_wk, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  GF_ASSERT( p_wk->people_type == RESORT_PEOPLE_TYPE_RESIDENT );

  p_wk->cp_function->GetMapPos( p_wk, p_x, p_z, p_dir );

  if( *p_dir == DIR_LEFT )
  {
    *p_x  -= 1;
    *p_dir  = DIR_RIGHT;
  }
  else
  {
    *p_x  += 1;
    *p_dir  = DIR_LEFT;
  }
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
  MMDL_SetStatusBitFellowHit( p_mmdl, is_visible );
  p_wk->is_master_visible = is_visible;
}
//----------------------------------------------------------------------------
/**
 *	@brief  一時消去
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SetTemporaryVisible( RESORT_PEOPLE *p_wk, RESORT_PEOPLE_VISIBLE flag )
{

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk );
  BOOL now = MMDL_CheckStatusBitVanish( p_mmdl );

  switch( flag )
  {
  case RESORT_PEOPLE_VISIBLE_ON:
    MMDL_SetStatusBitVanish( p_mmdl, FALSE );
    break;
  case RESORT_PEOPLE_VISIBLE_OFF:
    MMDL_SetStatusBitVanish( p_mmdl, TRUE );
    break;
  case RESORT_PEOPLE_VISIBLE_MASTER:
    MMDL_SetStatusBitVanish( p_mmdl, !p_wk->is_master_visible );
    break;
  default:
    GF_ASSERT(0);
  }
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
  case RESORT_PEOPLE_DATA_PARAM_CREATE:
    return cp_wk->create;
  default:
    GF_ASSERT(0);
    return 0;
  }
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
  return cp_wk->p_mmdl; 
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
 *	@param	*p_header           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Resident_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header )
{
  u16 grid_x, grid_z, dir;

  Resident_GetMapPos( p_wk, &grid_x, &grid_z, &dir );

  p_header->dir = dir;
  MMDLHEADER_SetGridPos( p_header, grid_x, grid_z, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  位置取得
 *
 *	@param	const RESORT_PEOPLE *cp_wk
 *	@param	*p_x
 *	@param	*p_z
 *	@param	*p_dir 
 */
//-----------------------------------------------------------------------------
static void Resident_GetMapPos( const RESORT_PEOPLE *cp_wk, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  u16 idx = RESORT_ABSTRUCT_DATA_GetBufferIndex(cp_wk->p_abstruct_data);
  RESORT_INSTITUTION_DATA *p_insti = RESORT_PEOPLE_GetInstitution( (RESORT_PEOPLE*)cp_wk );
  u16 id  = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
  const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( cp_wk->cp_shop,id);
  u16 data_3dmd = RESORT_BINARY_SHOP_DATA_GetData(cp_shop_data,RESORT_BINARY_SHOP_PARAM_3DMD_L+!(idx%2));

  RESORT_MAP_GetResidentGridPosOfs( data_3dmd, p_x, p_z, p_dir );

  (*p_x) += RESORT_BINARY_INSTITUTION_GetData(cp_wk->cp_institution, idx, RESORT_BINARY_INSTITUTION_PARAM_GRID_X );
  (*p_z) += RESORT_BINARY_INSTITUTION_GetData(cp_wk->cp_institution, idx, RESORT_BINARY_INSTITUTION_PARAM_GRID_Z );
}
//-------------------------------------
///代理人
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_header           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Proxy_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header )
{
  p_header->dir = DIR_UP;
  MMDLHEADER_SetGridPos( p_header, 0, 0, 0 );
}

//-------------------------------------
///観光客
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_header           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Tourist_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header )
{
  p_header->dir = DIR_UP;
  MMDLHEADER_SetGridPos( p_header, 0, 0, 0 );
}

//-------------------------------------
///秘書
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLヘッダ作成
 *
 *	@param	void *p_data_adrs   データ
 *	@param	*p_header           MMDL作成用ヘッダ
 */
//-----------------------------------------------------------------------------
static void Secretary_SetupMmdHeader( RESORT_PEOPLE *p_wk, const RESORT_PEOPLE_INIT_PARAM *cp_param, MMDL_HEADER *p_header )
{
  u16 grid_x, grid_z, dir;
  Secretary_GetMapPos( p_wk, &grid_x, &grid_z, &dir );

  p_header->dir = dir;
  MMDLHEADER_SetGridPos( p_header, grid_x, grid_z, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  初回位置取得
 *
 *	@param	const RESORT_PEOPLE *cp_wk
 *	@param	*p_x
 *	@param	*p_z
 *	@param	*p_dir 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void Secretary_GetMapPos( const RESORT_PEOPLE *cp_wk, u16 *p_x, u16 *p_z, u16 *p_dir )
{
  u16 kind = RESORT_ABSTRUCT_DATA_GetData( cp_wk->p_abstruct_data, RESORT_DATA_PARAM_TYPE, NULL );
  RESORT_MAP_GetSecretaryGridPos( kind, p_x, p_z, p_dir );
}

//----------------------------------------------------------------------------
/**
 *	@brief  MMDL作成
 *
 *	@param	RESORT_PEOPLE *p_wk ワーク
 *	@param	RESORT_PEOPLE_INIT_PARAM *cp_param 引数
 */
//-----------------------------------------------------------------------------
static void Resort_People_CreateMmdl( RESORT_PEOPLE *p_wk,  const RESORT_PEOPLE_INIT_PARAM *cp_param )
{
  MMDL_HEADER header = sc_default_header;

  GF_ASSERT( p_wk->p_mmdl == NULL );

  //ヘッダ共通設定
  {
    u8 objcode = RESORT_ABSTRUCT_DATA_GetData( p_wk->p_abstruct_data, RESORT_DATA_PARAM_OBJCODE, NULL );
    header.obj_code = objcode;
    header.event_id = RESORT_BINARY_PEOPLE_DATA_GetScrID( p_wk->cp_binary  );
    header.id = RESORT_PEOPLE_PEOPLEID_TO_MMDLID(cp_param->peopleID);
  }

  //ヘッダ個別設定
  p_wk->cp_function->SetupMmdHeader( p_wk, cp_param, &header );

  //MMDL作成
  p_wk->p_mmdl  = MMDLSYS_AddMMdl( cp_param->p_mmdlsys, &header, RESORT_MAP_PlaceIDToZoneID(cp_param->placeID) );

  //作成した場合は、データとMMDLをひもづけるため、データにPEOPLEIDとPLACEIDを設定
  RESORT_PEOPLE_SetData( p_wk, RESORT_DATA_PARAM_PEOPLE_ID, cp_param->peopleID );
  RESORT_PEOPLE_SetData( p_wk, RESORT_DATA_PARAM_PLACE_ID, cp_param->placeID );

  p_wk->is_master_visible = !MMDL_CheckStatusBitVanish( p_wk->p_mmdl );
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
static RESORT_PEOPLE * RESORT_PEOPLE_SYS_Add( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param, RESORT_PEOPLE_CREATE create, u16 peopleID );
static void Resort_People_DeleteNotBideMMdl( RESORT_PEOPLE_SYS *p_wk );

static void Resort_People_Sys_SetInitParam( const RESORT_PEOPLE_SYS *cp_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_add_param, u32 peopleID, RESORT_PEOPLE_INIT_PARAM *p_param );

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
  GF_ASSERT( cp_param->max < RESORT_PEOPLE_MAX_CREATE_NUM );
  {
    u32 work_size = sizeof(RESORT_PEOPLE_SYS)+sizeof(RESORT_PEOPLE *)*cp_param->max;
    RESORT_PEOPLE_SYS *p_wk = GFL_HEAP_AllocClearMemory(heapID, work_size );
    p_wk->param  = *cp_param;
    p_wk->heapID = heapID;

    p_wk->p_binary = RESORT_BINARY_PEOPLE_Init( heapID );

    return p_wk;
  }
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
      RESORT_PEOPLE_SYS_Add( p_wk, &param, RESORT_PEOPLE_CREATE_ADDALL, peopleID );
    }
  }

  Resort_People_DeleteNotBideMMdl( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物バッファの人を追加
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_AddPeopleBufferType( RESORT_PEOPLE_SYS *p_wk, const RESORT_DATA_BUFFER_TYPE c_buffer_tbl[], u32 tbl_len )
{
  RESORT_PLACEID placeID = p_wk->param.placeID;
  int peopleID;

  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->param.p_abstruct, c_buffer_tbl, tbl_len );

  while(TRUE)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->param.p_abstruct, &ite, c_buffer_tbl, tbl_len );
    if( p_data == NULL )
    {
      break;
    }

    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      peopleID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      if( !RESORT_DATA_MANAGER_IsPlaceEmpty( p_wk->param.p_data_manager, placeID, peopleID ) )
      {
        RESORT_PEOPLE_SYS_ADD_PARAM param;
        GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_ADD_PARAM) );
        param.p_data      = p_data;
        param.placeID     = p_wk->param.placeID;
        RESORT_PEOPLE_SYS_Add( p_wk, &param, RESORT_PEOPLE_CREATE_NEW, peopleID );
      }
    }
  }


  Resort_People_DeleteNotBideMMdl( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアル用の人物作成
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_AddPeopleTutorial( RESORT_PEOPLE_SYS *p_wk )
{

  static const RESORT_DATA_BUFFER_TYPE sc_type[] = 
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
  };

  RESORT_PLACEID placeID = p_wk->param.placeID;

  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->param.p_abstruct, sc_type, NELEMS(sc_type) );

  while(TRUE)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->param.p_abstruct, &ite, sc_type, NELEMS(sc_type) );
    if( p_data == NULL )
    {
      break;
    }

    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      BOOL is_add = FALSE;

      int peopleID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      {

        if( !RESORT_DATA_MANAGER_IsPlaceEmpty( p_wk->param.p_data_manager, placeID, peopleID ) )
        {
          is_add = TRUE;
        }
      }

      {
        u16 type = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_TYPE, NULL );
        RESORT_DATA_BUFFER_TYPE buff = RESORT_ABSTRUCT_DATA_GetBufferType( p_data );
        if( buff == RESORT_DATA_BUFFER_TYPE_PROXY_BUFF )
        {
          if( type == RESORT_PROXY_TYPE_TUTORIAL_01
              || type == RESORT_PROXY_TYPE_TUTORIAL_02 )
          {
            is_add |= TRUE;
          }
          else
          {
            is_add = FALSE;
          }
        }
      }

      if( is_add )
      {
        RESORT_PEOPLE_SYS_ADD_PARAM param;
        GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_ADD_PARAM) );
        param.p_data      = p_data;
        param.placeID     = p_wk->param.placeID;
        RESORT_PEOPLE_SYS_Add( p_wk, &param, RESORT_PEOPLE_CREATE_NEW, peopleID );
      }
    }
  }


  Resort_People_DeleteNotBideMMdl( p_wk );
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
static RESORT_PEOPLE * RESORT_PEOPLE_SYS_Add( RESORT_PEOPLE_SYS *p_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_param, RESORT_PEOPLE_CREATE create, u16 peopleID )
{
  if( p_wk->p_people[peopleID] == NULL )
  {
    RESORT_PEOPLE_INIT_PARAM init_param;

    Resort_People_Sys_SetInitParam( p_wk, cp_param, peopleID, &init_param );
    init_param.create = create;

    NAGI_Printf( "Add=%d\n",peopleID );

#ifdef DEBUG_PEOPLE_BUFF_PRINT
    Resort_People_PrintBuff( p_wk );
#endif

    p_wk->p_people[peopleID] = RESORT_PEOPLE_Init( p_wk->heapID, &init_param );
    RESORT_DATA_MANAGER_Update( p_wk->param.p_data_manager );
    return p_wk->p_people[peopleID];
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
    NAGI_Printf( "AddNew=%d\n", peopleID );
    {
      RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_Add( p_wk, cp_param, RESORT_PEOPLE_CREATE_NEW, peopleID );
      return p_people;
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

      NAGI_Printf( "Remove=%d\n", i );
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

      NAGI_Printf( "RemoveAll=%d\n", i );
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
    if( RESORT_PEOPLE_TOOL_IsObjID( MMDL_GetOBJID(mmdl) )
        && MMDL_GetZoneID(mmdl) == RESORT_MAP_PlaceIDToZoneID(p_wk->param.placeID) )
    {
      MMDL_Delete( mmdl );
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  全ての人物にマスター表示設定
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 *	@param	is_visible  TRUE表示  FALSE非表示
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_ReturnMasterVisibleAll( RESORT_PEOPLE_SYS *p_wk)
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = p_wk->p_people[i];
    if( p_people )
    {
      RESORT_PEOPLE_SetTemporaryVisible( p_people, RESORT_PEOPLE_VISIBLE_MASTER );
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  MMDLを破棄→作成
 *
 *	@param	RESORT_PEOPLE_SYS *p_sys  システム
 *	@param	*p_people 作成する人物
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_SYS_ResetMmdl( RESORT_PEOPLE_SYS *p_sys, RESORT_PEOPLE *p_people )
{
  RESORT_PEOPLE_INIT_PARAM  init_param;

  RESORT_PEOPLE_SYS_ADD_PARAM add_param;
  u32 peopleID  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PEOPLE_ID, NULL );

  GFL_STD_MemClear( &add_param, sizeof(RESORT_PEOPLE_SYS_ADD_PARAM));
  add_param.p_data  = RESORT_PEOPLE_GetAbstructData( p_people );
  add_param.placeID = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PLACE_ID, NULL );

  Resort_People_Sys_SetInitParam( p_sys, &add_param, peopleID, &init_param );
  init_param.create = RESORT_PEOPLE_GetPeopleData( p_people, RESORT_PEOPLE_DATA_PARAM_CREATE );

  RESORT_PEOPLE_ResetMmdl( p_people, &init_param );
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

  //人物を消す
  RESORT_PEOPLE_SYS_Remove( p_wk, p_people );

  //消したので更新
  RESORT_DATA_MANAGER_Update( p_wk->param.p_data_manager );


#ifdef DEBUG_PEOPLE_BUFF_PRINT
    Resort_People_PrintBuff( p_wk );
#endif
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


//----------------------------------------------------------------------------
/**
 *	@brief  バインドできなかったMMDLを消す
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_People_DeleteNotBideMMdl( RESORT_PEOPLE_SYS *p_wk )
{
  //作成してバインドできなかったMMDLは消す
  //->拡張バッファ対策(セーブ復帰時は拡張ばっふぁが消えている)
  {
    MMDL *p_mmdl;
    u32 no  = 0;
    while( MMDLSYS_SearchUseMMdl(p_wk->param.p_mmdlsys,&p_mmdl,&no) == TRUE ){
      if( RESORT_PEOPLE_TOOL_IsObjID( MMDL_GetOBJID(p_mmdl) )
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
 *	@brief  RESORT_PEOPLE_INIT_PARAMを設定
 *
 *	@param	const RESORT_PEOPLE_SYS *cp_wk  ワーク
 *	@param	*p_param  構造体
 */
//-----------------------------------------------------------------------------
static void Resort_People_Sys_SetInitParam( const RESORT_PEOPLE_SYS *cp_wk, const RESORT_PEOPLE_SYS_ADD_PARAM *cp_add_param, u32 peopleID, RESORT_PEOPLE_INIT_PARAM *p_param )
{
  GFL_STD_MemClear( p_param, sizeof(RESORT_PEOPLE_INIT_PARAM) );
  p_param->p_data       = cp_add_param->p_data;
  p_param->peopleID     = peopleID;
  p_param->placeID      = cp_add_param->placeID;
  p_param->cp_binary    = cp_wk->p_binary;
  p_param->p_mmdlsys    = cp_wk->param.p_mmdlsys;
  p_param->cp_shop      = cp_wk->param.cp_shop;
  p_param->cp_institution = cp_wk->param.cp_institution;
}


//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	int objid 
 *
 *	@return TRUE リゾート人物IDである FALSE 
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PEOPLE_TOOL_IsObjID( int objid )
{
  return RESORT_PEOPLE_MMDLID_OFS_S <= objid  &&  objid < RESORT_PEOPLE_MMDLID_OFS_E;
}


#ifdef DEBUG_PEOPLE_BUFF_PRINT
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ状況をプリント
 *
 *	@param	RESORT_PEOPLE_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_People_PrintBuff( RESORT_PEOPLE_SYS *p_wk )
{
  int i;
  OS_TFPrintf(2,"人物配置\n");
  for( i = 0; i < p_wk->param.max; i++ )
  {
    RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByIndex( p_wk, i );
    if( p_people )
    {
      OS_TFPrintf(2,"[%d]○t=%d,i=%d",i, RESORT_ABSTRUCT_DATA_GetBufferType(p_people->p_abstruct_data),RESORT_ABSTRUCT_DATA_GetBufferIndex(p_people->p_abstruct_data));
    }
    else
    {
      OS_TFPrintf(2,"[%d]×",i);
    }
  }
  OS_TFPrintf(2,"\n");
}
#endif
