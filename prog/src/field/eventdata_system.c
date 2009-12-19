//============================================================================================
/**
 * @file	eventdata_system.c
 * @brief	イベント起動用データのロード・保持システム
 * @author	tamada GAME FREAK inc.
 * @date	2008.11.20
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"

#include "system/main.h"
#include "field/field_const.h"

#include "eventdata_local.h"
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

#include "field/location.h"
#include "field/rail_location.h"
#include "field_rail.h"

#include "fldmmdl.h"
#include "script.h"
#include "script_hideitem.h"

#include "fieldmap/zone_id.h"

#include "field/zonedata.h"
#include "field/encount_data.h"

#include "debug/debug_flg.h" //DEBUG_FLG_～

//============================================================================================
//============================================================================================
#ifdef PM_DEBUG

//#define DEBUG_EVENTDATA_PRINT // 読み込んだイベントデータを出力

#endif  // PM_DEBUG

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
	EVDATA_SIZE = 0x880,
	SPSCR_DATA_SIZE = 0x100,
};

//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
/**
 * @brief	イベント起動データシステムの定義
 */
//------------------------------------------------------------------
struct _EVDATA_SYS {
	HEAPID heapID;

  ARCHANDLE * eventHandle;
  ARCHANDLE * encountHandle;
	u16 now_zone_id;
	u16 bg_count;
	u16 npc_count;
	u16 connect_count;
	u16 pos_count;
	const BG_TALK_DATA * bg_data;
	const MMDL_HEADER *npc_data;
	const CONNECT_DATA * connect_data;
	const POS_EVENT_DATA * pos_data;

  // 格納可能最大イベント数
	u16 bg_max;
	u16 npc_max;
	u16 connect_max;
	u16 pos_max;

	ENCOUNT_DATA encount_work;
	u32 load_buffer[EVDATA_SIZE / sizeof(u32)];
	u32 spscr_buffer[SPSCR_DATA_SIZE / sizeof(u32)];
};


//------------------------------------------------------------------
//------------------------------------------------------------------

#include "arc/fieldmap/zone_id.h"

//============================================================================================
//============================================================================================
#ifdef DEBUG_EVENTDATA_PRINT
static void debugPrint_Connect( const CONNECT_DATA* cp_data );
static void debugPrint_BGTalk( const BG_TALK_DATA* cp_data );
static void debugPrint_MMLD( const MMDL_HEADER* cp_data );
static void debugPrint_PosData( const POS_EVENT_DATA* cp_data );
#endif  //DEBUG_EVENTDATA_PRINT

//============================================================================================
//============================================================================================
static void loadEventDataTableNormal(EVENTDATA_SYSTEM * evdata, u16 zone_id);
static void loadEventDataTableExact(EVENTDATA_SYSTEM * evdata, u16 zone_id, u16 bg_num, u16 mmdl_num, u16 connect_num, u16 pos_num);
static void loadSpecialScriptData( EVENTDATA_SYSTEM * evdata, u16 zone_id );
static void loadEncountDataTable(EVENTDATA_SYSTEM* evdata, u16 zone_id, u8 season_id );
static void clearEventDataTable( EVENTDATA_SYSTEM* evdata );


//============================================================================================
// CONNECT_DATA ポジション
//============================================================================================
static void ConnectData_GPOS_GetPos( const CONNECT_DATA* cp_data, LOC_EXIT_OFS exit_ofs, VecFx32* p_pos );
static BOOL ConnectData_GPOS_IsHit( const CONNECT_DATA* cp_data, const VecFx32* cp_pos );
static void ConnectData_RPOS_GetLocation( const CONNECT_DATA* cp_data, LOC_EXIT_OFS exit_ofs, RAIL_LOCATION* p_location );
static BOOL ConnectData_RPOS_IsHit( const CONNECT_DATA* cp_data, const RAIL_LOCATION* cp_location );
static LOC_EXIT_OFS ConnectData_GPOS_GetExitOfs( const CONNECT_DATA * cp_data, const VecFx32 * p_pos );
static LOC_EXIT_OFS ConnectData_RPOS_GetExitOfs( const CONNECT_DATA * cp_data, const RAIL_LOCATION * cp_location );


//============================================================================================
// BG_TALK_DATA ポジション
//============================================================================================
static void BGTalkData_GPOS_GetPos( const BG_TALK_DATA* cp_data, VecFx32* p_pos );
static BOOL BGTalkData_GPOS_IsHit( const BG_TALK_DATA* cp_data, const VecFx32* cp_pos );
static void BGTalkData_RPOS_GetLocation( const BG_TALK_DATA* cp_data, RAIL_LOCATION* p_location );
static BOOL BGTalkData_RPOS_IsHit( const BG_TALK_DATA* cp_data, const RAIL_LOCATION* cp_location );

//============================================================================================
// POS_EVENT_DATA ポジション
//============================================================================================
static void PosEventData_GPOS_GetPos( const POS_EVENT_DATA* cp_data, VecFx32* p_pos );
static BOOL PosEventData_GPOS_IsHit( const POS_EVENT_DATA* cp_data, const VecFx32* cp_pos );
static BOOL PosEventData_GPOS_IsHit_XZ( const POS_EVENT_DATA* cp_data, const VecFx32* cp_pos );
static void PosEventData_RPOS_GetLocation( const POS_EVENT_DATA* cp_data, RAIL_LOCATION* p_location );
static BOOL PosEventData_RPOS_IsHit( const POS_EVENT_DATA* cp_data, const RAIL_LOCATION* cp_location );


//============================================================================================
//============================================================================================

//============================================================================================
//
//	イベント起動データシステム関連
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
EVENTDATA_SYSTEM * EVENTDATA_SYS_Create(HEAPID heapID)
{
	EVENTDATA_SYSTEM * evdata = GFL_HEAP_AllocClearMemory(heapID, sizeof(EVENTDATA_SYSTEM));
  evdata->eventHandle = GFL_ARC_OpenDataHandle( ARCID_FLD_EVENTDATA, heapID );
  evdata->encountHandle = GFL_ARC_OpenDataHandle( ARCID_ENCOUNT, heapID );
	return evdata;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENTDATA_SYS_Delete(EVENTDATA_SYSTEM * evdata)
{
  GFL_ARC_CloseDataHandle( evdata->eventHandle );
  GFL_ARC_CloseDataHandle( evdata->encountHandle );
	GFL_HEAP_FreeMemory(evdata);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENTDATA_SYS_Clear(EVENTDATA_SYSTEM * evdata)
{
  clearEventDataTable( evdata );
	GFL_STD_MemClear(evdata->spscr_buffer, SPSCR_DATA_SIZE);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENTDATA_SYS_Load( EVENTDATA_SYSTEM * evdata, u16 zone_id, u8 season_id )
{
	EVENTDATA_SYS_Clear(evdata);
	evdata->now_zone_id = zone_id;

  loadSpecialScriptData( evdata, zone_id );
  loadEventDataTableNormal(evdata, zone_id);
  loadEncountDataTable( evdata, zone_id, season_id );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void * EVENTDATA_GetSpecialScriptData( EVENTDATA_SYSTEM * evdata )
{
  return &evdata->spscr_buffer;
}




//----------------------------------------------------------------------------
/**
 *	@brief  ヘッダーから、BGTALKイベント取得
 *
 *	@param	cp_header   ヘッダー
 *
 *	@return ポインタ
 */
//-----------------------------------------------------------------------------
const BG_TALK_DATA* EVENTDATA_SYS_HEADER_GetTalkBgEvent( const EVENTDATA_TABLE* cp_header )
{
  u32 addr;
  
  GF_ASSERT( cp_header );

  addr = (u32)(&cp_header->buf[0]);
  
  return (const BG_TALK_DATA*)addr;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヘッダーから、MMDLイベント取得
 *
 *	@param	cp_header   ヘッダー
 *
 *	@return ポインタ
 */
//-----------------------------------------------------------------------------
const MMDL_HEADER* EVENTDATA_SYS_HEADER_GetMmdlEvent( const EVENTDATA_TABLE* cp_header )
{
  u32 addr;
  
  GF_ASSERT( cp_header );

  addr = (u32)(&cp_header->buf[0]);
  addr += sizeof(BG_TALK_DATA) * cp_header->bg_count;
  
  return (const MMDL_HEADER*)addr;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヘッダーから、接続イベント取得
 *
 *	@param	cp_header   ヘッダー
 *
 *	@return ポインタ
 */
//-----------------------------------------------------------------------------
const CONNECT_DATA* EVENTDATA_SYS_HEADER_GetConnectEvent( const EVENTDATA_TABLE* cp_header )
{
  u32 addr;
  
  GF_ASSERT( cp_header );

  addr = (u32)(&cp_header->buf[0]);
  addr += sizeof(BG_TALK_DATA) * cp_header->bg_count;
  addr += sizeof(MMDL_HEADER) * cp_header->npc_count;
  
  return (const CONNECT_DATA*)addr;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヘッダーから、POSイベント取得
 *
 *	@param	cp_header   ヘッダー
 *
 *	@return ポインタ
 */
//-----------------------------------------------------------------------------
const POS_EVENT_DATA* EVENTDATA_SYS_HEADER_GetPosEvent( const EVENTDATA_TABLE* cp_header )
{
  u32 addr;
  
  GF_ASSERT( cp_header );

  addr = (u32)(&cp_header->buf[0]);
  addr += sizeof(BG_TALK_DATA) * cp_header->bg_count;
  addr += sizeof(MMDL_HEADER) * cp_header->npc_count;
  addr += sizeof(CONNECT_DATA) * cp_header->connect_count;
  
  return (const POS_EVENT_DATA*)addr;
}


//----------------------------------------------------------------------------
/**
 *	@brief  イベント情報をイベントデータの枠を確保しつつ読み込み
 *
 *	@param	evdata        イベントデータ
 *	@param	bg_num        BGイベント数
 *	@param	mmdl_num      モデルイベント数
 *	@param	connect_num   接続イベント数
 *	@param	pos_num       ポスイベント数
 */
//-----------------------------------------------------------------------------
void EVENTDATA_SYS_ReloadEventDataEx(EVENTDATA_SYSTEM * evdata, u16 bg_num, u16 mmdl_num, u16 connect_num, u16 pos_num )
{
  clearEventDataTable( evdata );
  loadEventDataTableExact(evdata, evdata->now_zone_id, bg_num, mmdl_num, connect_num, pos_num);
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGイベントを追加
 *
 *	@param	evdata    イベントシステム
 *	@param	cp_data   BGイベント
 *  
 *	@return 追加INDEX
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_SYS_AddTalkBgEvent( EVENTDATA_SYSTEM * evdata, const BG_TALK_DATA* cp_data )
{
  u32 index;
  BG_TALK_DATA * p_bgevent;
  
  GF_ASSERT( evdata );
  GF_ASSERT( cp_data );

  if( evdata->bg_max > evdata->bg_count )
  {
    // 追加
    evdata->bg_count ++;
  }
  else
  {
    // BGTALK追加オーバー 
    // 登録せずに終わる
    GF_ASSERT( 0 );
    return 0;
  }

  index = evdata->bg_count;

  // 追加
  p_bgevent = (BG_TALK_DATA *)evdata->bg_data;
  GFL_STD_MemCopy( cp_data, &p_bgevent[ index ], sizeof(BG_TALK_DATA) );
  
  return index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MMDLイベントの追加
 *
 *	@param	evdata    イベントシステム
 *	@param	cp_data   動作モデル
 *
 *	@return 追加INDEX
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_SYS_AddMmdlEvent( EVENTDATA_SYSTEM * evdata, const MMDL_HEADER* cp_data )
{
  u32 index;
  MMDL_HEADER * p_event;
  
  GF_ASSERT( evdata );
  GF_ASSERT( cp_data );

  if( evdata->npc_max > evdata->npc_count )
  {
    // 追加
    evdata->npc_count ++;
  }
  else
  {
    // MMDL追加オーバー 
    // 登録せずに終わる
    GF_ASSERT( 0 );
    return 0;
  }

  index = evdata->npc_count;

  // 追加
  p_event = (MMDL_HEADER*)evdata->npc_data;
  GFL_STD_MemCopy( cp_data, &p_event[ index ], sizeof(MMDL_HEADER) );
  
  return index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続イベントの追加
 *    
 *	@param	evdata    イベントシステム
 *	@param	cp_data   接続イベント
 *
 *	@return 追加INDEX
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_SYS_AddConnectEvent( EVENTDATA_SYSTEM * evdata, const CONNECT_DATA* cp_data )
{
  u32 index;
  CONNECT_DATA * p_event;
  
  GF_ASSERT( evdata );
  GF_ASSERT( cp_data );

  index = evdata->connect_count;

  if( evdata->connect_max > evdata->connect_count )
  {
    // 追加
    evdata->connect_count ++;
  }
  else
  {
    //　接続追加オーバー 
    // 登録せずに終わる
    GF_ASSERT( 0 );
    return 0;
  }

  // 追加
  p_event = (CONNECT_DATA*)evdata->connect_data;
  GFL_STD_MemCopy( cp_data, &p_event[ index ], sizeof(CONNECT_DATA) );
  
  return index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  POSイベントの追加
 *
 *	@param	evdata    イベントシステム
 *	@param	cp_data   POSイベント
 *
 *	@return 追加INDEX
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_SYS_AddPosEvent( EVENTDATA_SYSTEM * evdata, const POS_EVENT_DATA* cp_data )
{
  u32 index;
  POS_EVENT_DATA * p_event;
  
  GF_ASSERT( evdata );
  GF_ASSERT( cp_data );

  if( evdata->pos_max > evdata->pos_count )
  {
    // 追加
    evdata->pos_count ++;
  }
  else
  {
    // POS追加オーバー 
    // 登録せずに終わる
    GF_ASSERT( 0 );
    return 0;
  }

  index = evdata->pos_count;

  // 追加
  p_event = (POS_EVENT_DATA*)evdata->pos_data;
  GFL_STD_MemCopy( cp_data, &p_event[ index ], sizeof(POS_EVENT_DATA) );
  
  return index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続イベント　削除
 *
 *	@param	evdata          イベントデータ
 *	@param	link_zone_id    接続先ZONEID
 *	@param	link_exit_id    接続先EXITID
 */
//-----------------------------------------------------------------------------
void EVENTDATA_SYS_DelConnectEvent( EVENTDATA_SYSTEM * evdata, u16 link_zone_id, u16 link_exit_id )
{
  int i;
  CONNECT_DATA * connect_data = (CONNECT_DATA *)evdata->connect_data;

  //  link_zone_id link_exit_idのデータを調べる
  for( i=0; i<evdata->connect_count; i++ )
  {
    if( (evdata->connect_data[i].link_zone_id == link_zone_id) && (evdata->connect_data[i].link_zone_id == link_exit_id) )
    {
      // 情報の破棄
      connect_data[i].link_zone_id = ZONE_ID_NONE; 
      connect_data[i].link_exit_id = EXIT_ID_NONE; 
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続イベント　削除
 *
 *	@param	evdata          イベントデータ
 *	@param	idx             削除するデータのインデックス
 */
//-----------------------------------------------------------------------------
void EVENTDATA_SYS_DelConnectEventIdx( EVENTDATA_SYSTEM * evdata, u16 idx )
{
  int i;
  CONNECT_DATA * connect_data = (CONNECT_DATA *)evdata->connect_data;

  if( evdata->connect_count <= idx )
  {
    GF_ASSERT( evdata->connect_count > idx );
    return ;
  }
  
  // 情報の破棄
  connect_data[idx].link_zone_id = ZONE_ID_NONE; 
  connect_data[idx].link_exit_id = EXIT_ID_NONE; 
}




//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadSpecialScriptData( EVENTDATA_SYSTEM * evdata, u16 zone_id )
{
  u16 sp_scr_id = ZONEDATA_GetSpScriptArcID( zone_id );
  GFL_ARC_LoadData(evdata->spscr_buffer, ARCID_SCRSEQ,  sp_scr_id );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadEventDataTableNormal(EVENTDATA_SYSTEM * evdata, u16 zone_id)
{
  int i;
  const EVENTDATA_TABLE* table;
  s32 ofs;
  const u8* buf;
  u32 arcID;
  u32 size;

  arcID = ZONEDATA_GetEventDataArcID(zone_id);
  size = GFL_ARC_GetDataSizeByHandle( evdata->eventHandle, arcID );
  // サイズオーバーチェック
  GF_ASSERT( EVDATA_SIZE > size );
  GFL_ARC_LoadDataOfsByHandle(evdata->eventHandle, arcID, 0, size, evdata->load_buffer);
  
  table = (const EVENTDATA_TABLE*)evdata->load_buffer; 
  buf = (const u8*)table->buf;
  ofs = 0;

  // データ格納
  evdata->bg_count = table->bg_count;
  evdata->bg_data = (const BG_TALK_DATA*)&buf[ofs];
  ofs += sizeof(BG_TALK_DATA) * table->bg_count;
  evdata->npc_count = table->npc_count;
  evdata->npc_data = (const MMDL_HEADER*)&buf[ofs];
  ofs += sizeof(MMDL_HEADER) * table->npc_count;
  evdata->connect_count = table->connect_count;
  evdata->connect_data = (const CONNECT_DATA*)&buf[ofs];
  ofs += sizeof(CONNECT_DATA) * table->connect_count;
  evdata->pos_count = table->pos_count;
  evdata->pos_data = (const POS_EVENT_DATA*)&buf[ofs];
  

  // 格納最大数＝データ数
  evdata->bg_max        = evdata->bg_count;
  evdata->npc_max       = evdata->npc_count;
  evdata->connect_max   = evdata->connect_count;
  evdata->pos_max       = evdata->pos_count;


#ifdef DEBUG_EVENTDATA_PRINT
  {
    OS_TPrintf( "DEBUG PRINT Event Load ZoneID %d\n", zone_id );
    
    // 接続情報
    for( i=0; i<evdata->connect_count; i++ )
    {
      debugPrint_Connect( &evdata->connect_data[i] );
    }

    for( i=0; i<evdata->bg_count; i++ )
    {
      debugPrint_BGTalk( &evdata->bg_data[i] );
    }

    for( i=0; i<evdata->npc_count; i++ )
    {
      debugPrint_MMLD( &evdata->npc_data[i] );
    }

    for( i=0; i<evdata->pos_count; i++ )
    {
      debugPrint_PosData( &evdata->pos_data[i] );
    }
  }
#endif // DEBUG_EVENTDATA_PRINT
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベント情報の読み込み　各イベント個数指定バージョン
 *
 *	@param	evdata      イベントデータ
 *	@param	zone_id     ゾーンID
 */
//-----------------------------------------------------------------------------
static void loadEventDataTableExact(EVENTDATA_SYSTEM * evdata, u16 zone_id, u16 bg_num, u16 mmdl_num, u16 connect_num, u16 pos_num )
{
  int i;
  const EVENTDATA_TABLE* table;
  s32 ofs;
  u8* buf;
  u32 arcID;
  u32 size;
  u16 event_num;

  // 総サイズオーバーチェック
  size = 0;
  size += 4;
  size += sizeof(BG_TALK_DATA) * bg_num;
  size += sizeof(MMDL_HEADER) * mmdl_num;
  size += sizeof(CONNECT_DATA) * connect_num;
  size += sizeof(POS_EVENT_DATA) * connect_num;
  // サイズオーバーチェック
  GF_ASSERT( EVDATA_SIZE > size );
  
  

  arcID = ZONEDATA_GetEventDataArcID(zone_id);
  size = GFL_ARC_GetDataSizeByHandle( evdata->eventHandle, arcID );
  // サイズオーバーチェック
  GF_ASSERT( EVDATA_SIZE > size );

  ofs = 0;
  table = (const EVENTDATA_TABLE*)evdata->load_buffer; 
  buf = (u8*)table->buf;

  // まず、ヘッダー部を読み込む
  GFL_ARC_LoadDataOfsByHandle(evdata->eventHandle, arcID, ofs, 4, evdata->load_buffer);
  ofs += 4;

  // それぞれのイベント数がオーバーしてないかチェック
  GF_ASSERT( table->bg_count <= bg_num );
  GF_ASSERT( table->npc_count <= mmdl_num );
  GF_ASSERT( table->connect_count <= connect_num );
  GF_ASSERT( table->pos_count <= pos_num );

  // BGイベント
  GFL_ARC_LoadDataOfsByHandle(evdata->eventHandle, arcID, ofs, table->bg_count * sizeof(BG_TALK_DATA), &buf[ofs]);
  evdata->bg_count = table->bg_count;
  evdata->bg_data = (const BG_TALK_DATA*)&buf[ofs];
  ofs += sizeof(BG_TALK_DATA) * bg_num;

  // MMDLイベント
  GFL_ARC_LoadDataOfsByHandle(evdata->eventHandle, arcID, ofs, table->npc_count * sizeof(MMDL_HEADER), &buf[ofs]);
  evdata->npc_count = table->npc_count;
  evdata->npc_data = (const MMDL_HEADER*)&buf[ofs];
  ofs += sizeof(MMDL_HEADER) * mmdl_num;

  // CONNECTイベント
  GFL_ARC_LoadDataOfsByHandle(evdata->eventHandle, arcID, ofs, table->connect_count * sizeof(CONNECT_DATA), &buf[ofs]);
  evdata->connect_count = table->connect_count;
  evdata->connect_data = (const CONNECT_DATA*)&buf[ofs];
  ofs += sizeof(CONNECT_DATA) * connect_num;

  // POSイベント
  GFL_ARC_LoadDataOfsByHandle(evdata->eventHandle, arcID, ofs, table->pos_count * sizeof(POS_EVENT_DATA), &buf[ofs]);
  evdata->pos_count = table->pos_count;
  evdata->pos_data = (const POS_EVENT_DATA*)&buf[ofs];



  // 格納最大数
  evdata->bg_max        = bg_num;
  evdata->npc_max       = mmdl_num;
  evdata->connect_max   = connect_num;
  evdata->pos_max       = pos_num;
}

//------------------------------------------------------------------
/*
 *  @brief  ゾーンエンカウントデータロード
 *
 *  @retval true  エンカウントデータ取得に成功
 *  @retval false 指定ゾーンにはエンカウントデータがない
 */
//------------------------------------------------------------------
static void loadEncountDataTable(EVENTDATA_SYSTEM* evdata, u16 zone_id, u8 season_id )
{
  u32 size;
  u16 id,season;
  MI_CpuClear8(&evdata->encount_work,sizeof(ENCOUNT_DATA));

  id = ZONEDATA_GetEncountDataID(zone_id);
  if(id == ENC_DATA_INVALID_ID){
    return;
  }
  //データの個数を取得
  size = GFL_ARC_GetDataSizeByHandle( evdata->encountHandle, id ) / sizeof(ENCOUNT_DATA);
  if(size != 1 && size != 4){
    GF_ASSERT(0);
    return;
  }
  //季節を取得
  if(size == 1){
    season = 0;
  }else{
    season = season_id;
  }
  //ロード
  GFL_ARC_LoadDataOfsByHandle(evdata->encountHandle, id,
      season*sizeof(ENCOUNT_DATA), sizeof(ENCOUNT_DATA), &evdata->encount_work);
  evdata->encount_work.enable_f = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントデータテーブルクリア
 *
 *	@param	evdata  イベントデータシステム
 */
//-----------------------------------------------------------------------------
static void clearEventDataTable( EVENTDATA_SYSTEM* evdata )
{
	evdata->bg_max = 0;
	evdata->npc_max = 0;
	evdata->connect_max = 0;
	evdata->pos_max = 0;
	evdata->bg_count = 0;
	evdata->npc_count = 0;
	evdata->connect_count = 0;
	evdata->pos_count = 0;
	evdata->bg_data = NULL;
	evdata->npc_data = NULL;
	evdata->connect_data = NULL;
	evdata->pos_data = NULL;
	GFL_STD_MemClear(evdata->load_buffer, EVDATA_SIZE);
}


//------------------------------------------------------------------
/*
 *  @brief  現ゾーンエンカウントデータ取得
 *  @return 現在のゾーンに読み込まれているエンカウントデータテーブルへのポインタ
 */
//------------------------------------------------------------------
void* EVENTDATA_GetEncountDataTable(EVENTDATA_SYSTEM* evdata)
{
  return &evdata->encount_work;
}


//----------------------------------------------------------------------------
/**
 *	@brief  BGイベントの数を取得
 *
 *	@param	evdata  イベントワーク
 *
 *	@return BGイベント数
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_GetBgEventNum( const EVENTDATA_SYSTEM * evdata )
{
  GF_ASSERT( evdata );
  return evdata->bg_count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MMLDイベントの数を取得
 *
 *	@param	evdata  イベントワーク
 *
 *	@return MMDLイベント数
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_GetNpcEventNum( const EVENTDATA_SYSTEM * evdata )
{
  GF_ASSERT( evdata );
  return evdata->npc_count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続イベントの数を取得
 *
 *	@param	evdata  イベントワーク
 *
 *	@return 接続イベントの数
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_GetConnectEventNum( const EVENTDATA_SYSTEM * evdata )
{
  GF_ASSERT( evdata );
  return evdata->connect_count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  POSイベントの数を取得
 *
 *	@param	evdata  イベントワーク
 *  
 *	@return POSイベントの数
 */
//-----------------------------------------------------------------------------
u32 EVENTDATA_GetPosEventNum( const EVENTDATA_SYSTEM * evdata )
{
  GF_ASSERT( evdata );
  return evdata->pos_count;
}



//============================================================================================
//============================================================================================

//============================================================================================
//
//	出入口接続データ関連
//
//============================================================================================
#define	ZONE_ID_SPECIAL		(0x0fff)
#define	EXIT_ID_SPECIAL		(0x0100)
enum {
  CONNECT_POS_OFS_X = 0,
  CONNECT_POS_OFS_Y = 0,
  CONNECT_POS_OFS_Z = 0,

  CONNECT_DATA_RANGE = 2,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
int EVENTDATA_SearchConnectIDByPos(const EVENTDATA_SYSTEM * evdata, const VecFx32 * pos)
{
	int i;
	const CONNECT_DATA * cnct = evdata->connect_data;
	for (i = 0; i < evdata->connect_count; i++, cnct++ ) {
    if( ConnectData_GPOS_IsHit( cnct, pos ) )
    {
  		TAMADA_Printf("CNCT:zone,exit,type=%d,%d,%d\n",
          cnct->link_zone_id,cnct->link_exit_id,cnct->exit_type);
		  return i;
    }
	}
	return EXIT_ID_NONE;
}

int EVENTDATA_SearchConnectIDByRailLocation(const EVENTDATA_SYSTEM * evdata, const RAIL_LOCATION* rail_location)
{
	int i;
	const CONNECT_DATA * cnct = evdata->connect_data;
	for (i = 0; i < evdata->connect_count; i++, cnct++ ) {
    if( ConnectData_RPOS_IsHit( cnct, rail_location ) )
    {
      TAMADA_Printf("CNCT:zone,exit,type=%d,%d,%d\n",
          cnct->link_zone_id,cnct->link_exit_id,cnct->exit_type);
      return i;
    }
	}
	return EXIT_ID_NONE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECT_DATA * EVENTDATA_GetConnectByID(const EVENTDATA_SYSTEM * evdata, u16 exit_id)
{
	const CONNECT_DATA * cnct = evdata->connect_data;
	if (cnct == NULL) {
		return NULL;
	}
	if (exit_id >= evdata->connect_count) {
		GF_ASSERT(0);
		return NULL;
	}
	return &cnct[exit_id];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL CONNECTDATA_IsSpecialExit(const CONNECT_DATA * connect)
{
	return (connect->link_exit_id == EXIT_ID_SPECIAL);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL EVENTDATA_SetLocationByExitID(const EVENTDATA_SYSTEM * evdata, LOCATION * loc, u16 exit_id,
    LOC_EXIT_OFS exit_ofs )
{
  /** TODO:　見た目含めてオフセットを再計算すること */
  enum{ GRIDSIZE = 8, MV = GRIDSIZE * FX32_ONE};
//  static const VecFx32 ofs = { CONNECT_POS_OFS_X * FX32_ONE, CONNECT_POS_OFS_Y * FX32_ONE, 0 * FX32_ONE }; GPOS_GetPos内で行っています。

	const CONNECT_DATA * connect = EVENTDATA_GetConnectByID(evdata, exit_id);
	if (connect == NULL) {
		return FALSE;
	}

	//CONNECTDATA_SetLocation(connect, loc);
	LOCATION_Init(loc);
	if (connect->link_exit_id == EXIT_ID_SPECIAL) {
		loc->type = LOCATION_TYPE_SPID;
	} else {
		loc->type = LOCATION_TYPE_EXITID;
	}

	//loc->pos = connect->pos;
  //グリッドマップORレールマップのロケーションを作成
  if( connect->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    VecFx32 pos;
  	ConnectData_GPOS_GetPos( connect, exit_ofs, &pos );
    LOCATION_Set( loc, evdata->now_zone_id, exit_id, connect->exit_dir, exit_ofs, pos.x, pos.y, pos.z );
  }
  else
  {
    RAIL_LOCATION pos;
  	ConnectData_RPOS_GetLocation( connect, exit_ofs, &pos );
    LOCATION_SetRail( loc, evdata->now_zone_id, exit_id, connect->exit_dir, exit_ofs, pos.rail_index, pos.line_grid, pos.width_grid );
  }

	return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void CONNECTDATA_SetNextLocation(const CONNECT_DATA * connect, LOCATION * loc, LOC_EXIT_OFS exit_ofs)
{
	LOCATION_SetID(loc, connect->link_zone_id, connect->link_exit_id, exit_ofs);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
int EVENTDATA_SearchConnectIDBySphere(const EVENTDATA_SYSTEM * evdata, const VecFx32 * sphere)
{
  enum { HIT_RANGE = FX32_ONE * 16 };
	int i;
  VecFx32 target, check;
  fx32 len;

	const CONNECT_DATA * cnct = evdata->connect_data;
  target = *sphere;
  target.y -= FX32_ONE * (FIELD_CONST_GRID_SIZE / 2);
  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_R)
  {
    TAMADA_Printf("CNCT:check position (%d, %d, %d)\n",
        FX_Whole(sphere->x), FX_Whole(sphere->y), FX_Whole(sphere->z));
  }

	for (i = 0; i < evdata->connect_count; i++, cnct++ ) {
    if( cnct->pos_type == EVENTDATA_POSTYPE_GRID )
    {
      ConnectData_GPOS_GetPos(cnct, LOCATION_DEFAULT_EXIT_OFS, &check );
      len = VEC_Distance(&check, sphere); 
      
      if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_R)
      {
        TAMADA_Printf("CNCT:ID%02d (%08x, %08x, %08x)", i, FX_Whole(check.x), FX_Whole(check.y), FX_Whole(check.z));
        TAMADA_Printf(" (%3d, %3d, %3d)",
            FX_Whole(check.x) / FIELD_CONST_GRID_SIZE,
            FX_Whole(check.y) / FIELD_CONST_GRID_SIZE,
            FX_Whole(check.z) / FIELD_CONST_GRID_SIZE);
        TAMADA_Printf("\tlen = %d\n", FX_Whole(len) );
      }
      if ( len > HIT_RANGE) continue;
      TAMADA_Printf("CNCT:zone,exit,type=%d,%d,%d\n",cnct->link_zone_id,cnct->link_exit_id,cnct->exit_type);
      return i;
    }
	}
	return EXIT_ID_NONE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
LOC_EXIT_OFS CONNECTDATA_GetExitOfs(const CONNECT_DATA * connect, const VecFx32 * pos)
{
  return ConnectData_GPOS_GetExitOfs( connect, pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールマップの出口オフセットを取得
 *
 *	@param	connect
 *	@param	loc
 *
 *	@return 出口オフセット
 */
//-----------------------------------------------------------------------------
LOC_EXIT_OFS CONNECTDATA_GetRailExitOfs(const CONNECT_DATA * connect, const RAIL_LOCATION* loc)
{
  return ConnectData_RPOS_GetExitOfs( connect, loc );
}
//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
EXIT_DIR CONNECTDATA_GetExitDir(const CONNECT_DATA * connect)
{
  return connect->exit_dir;
}
//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
EXIT_TYPE CONNECTDATA_GetExitType(const CONNECT_DATA * connect)
{
  return connect->exit_type;
}

//------------------------------------------------------------------
/**
 * @brief 無効出入口のチェック
 * @retval  TRUE  無効出入口
 * @retval  FALSE 有効な出入口
 */
//------------------------------------------------------------------
BOOL CONNECTDATA_IsClosedExit( const CONNECT_DATA * connect )
{
  return ( connect->link_zone_id == ZONE_ID_NONE )
    || ( connect->link_exit_id == EXIT_ID_NONE );
}

//============================================================================================
//		動作モデル関連
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	動作モデルヘッダーを取得
 * @param	evdata		イベントデータへのポインタ
 * @retval	MMDL_HEADER*
 */
//------------------------------------------------------------------
const MMDL_HEADER * EVENTDATA_GetNpcData( const EVENTDATA_SYSTEM *evdata )
{
	return( evdata->npc_data );
}

//------------------------------------------------------------------
/**
 * @brief	動作モデル総数を取得
 * @param	evdata		イベントデータへのポインタ
 * @retval	u16
 */
//------------------------------------------------------------------
u16 EVENTDATA_GetNpcCount( const EVENTDATA_SYSTEM *evdata )
{
	return( evdata->npc_count );
}

//============================================================================================
//  POSイベント関連
//============================================================================================
//------------------------------------------------------------------
/// 向きにあったPosCheckCodeを取得する
//------------------------------------------------------------------
static u16 getPosCheckCode( u16 dir )
{
  switch (dir) {
  case DIR_UP:
    return POS_CHECK_TYPE_UP;
  case DIR_DOWN:
    return POS_CHECK_TYPE_DOWN;
  case DIR_LEFT:
    return POS_CHECK_TYPE_LEFT;
  case DIR_RIGHT:
    return POS_CHECK_TYPE_RIGHT;
  }
  return POS_CHECK_TYPE_NORMAL;   //DIR_NOTの場合、通常チェック
}

//------------------------------------------------------------------
/**
 * @brief	座標イベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param evwork イベントワークへのポインタ 
 * @param pos チェックする座標
 * @param dir チェックする方向（DIR_NOTのとき、向き指定なし）
 * @retval POS_EVENT_DATA* (NULL = イベントなし)
 */
//------------------------------------------------------------------
const POS_EVENT_DATA * EVENTDATA_GetPosEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const VecFx32 *pos, u16 dir )
{
  const POS_EVENT_DATA *data = evdata->pos_data;
  
  if( data != NULL )
  {
    u16 check_type = getPosCheckCode( dir );
    u16 i = 0;
    u16 *work_val;
    u16 max = evdata->pos_count;

    for( ; i < max; i++, data++ )
    {
      if( !PosEventData_GPOS_IsHit(data, pos) )
      {
        continue;
      }
      if ( data->check_type != check_type )
      {
        continue;
      }
      work_val = EVENTWORK_GetEventWorkAdrs( evwork, data->workID );
      if( (*work_val) == data->param )
      {
        return data;
      }
    }
  }
  return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	座標イベントチェック(xz平面ver.)
 * @param	evdata イベントデータへのポインタ
 * @param evwork イベントワークへのポインタ 
 * @param pos チェックする座標
 * @param dir チェックする方向
 * @retval POS_EVENT_DATA* (NULL = イベントなし)
 */
//------------------------------------------------------------------
const POS_EVENT_DATA * EVENTDATA_GetPosEvent_XZ(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const VecFx32 *pos, u16 dir )
{
  const POS_EVENT_DATA *data = evdata->pos_data;
  
  if( data != NULL )
  {
    u16 check_type = getPosCheckCode( dir );
    u16 i = 0;
    u16 *work_val;
    u16 max = evdata->pos_count;

    for( ; i < max; i++, data++ )
    { 
      if( !PosEventData_GPOS_IsHit_XZ(data, pos) )  // あたり判定でy軸を無視する
      {
        continue;
      }
      if ( data->check_type != check_type )
      {
        continue;
      }
      work_val = EVENTWORK_GetEventWorkAdrs( evwork, data->workID );
      if( (*work_val) == data->param )
      {
        return data;
      }
    }
  }
  return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	座標イベントを取得
 * @param	evdata イベントデータへのポインタ
 * @param evwork イベントワークへのポインタ 
 * @param location チェックするレールロケーション
 * @retval NULL = イベントなし
 */
//------------------------------------------------------------------
const POS_EVENT_DATA * EVENTDATA_GetPosEventRailLocation( 
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const RAIL_LOCATION* location )
{
  const POS_EVENT_DATA *data = evdata->pos_data;
  
  if( data != NULL )
  {
    u16 i = 0;
    u16 *work_val;
    u16 max = evdata->pos_count;

    for( ; i < max; i++, data++ )
    {
      if( !PosEventData_RPOS_IsHit(data, location) )
      {
        continue;
      }
      {
        work_val = EVENTWORK_GetEventWorkAdrs( evwork, data->workID );
        if( (*work_val) == data->param ){
          return data;
        }
      }
    }
  }
  return NULL;
}



//------------------------------------------------------------------
/**
 * @brief	座標イベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param evwork イベントワークへのポインタ 
 * @param pos チェックする座標
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
u16 EVENTDATA_CheckPosEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const VecFx32 *pos, u16 dir )
{
  const POS_EVENT_DATA* data = EVENTDATA_GetPosEvent( evdata,evwork,pos,dir );
  
  if ( DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEvents) ) return EVENTDATA_ID_NONE;
  if( data != NULL ){
    return data->id;
  }
  return EVENTDATA_ID_NONE;
}

//----------------------------------------------------------------------------
/**
 * @brief	座標イベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param evwork イベントワークへのポインタ 
 * @param pos チェックする座標
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//-----------------------------------------------------------------------------
u16 EVENTDATA_CheckPosEventRailLocation(
  const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const RAIL_LOCATION *pos )
{
  const POS_EVENT_DATA* data = EVENTDATA_GetPosEventRailLocation( evdata,evwork,pos );
  
  if ( DEBUG_FLG_GetFlg(DEBUG_FLG_DisableEvents) ) return EVENTDATA_ID_NONE;
  if( data != NULL ){
    return data->id;
  }
  return EVENTDATA_ID_NONE;
}


//============================================================================================
//  BGTALKイベント関連
//============================================================================================
enum {
  BG_TALK_DATA_RANGE = 2,
};

//------------------------------------------------------------------
/**
 * @brief	BG話し掛けイベントチェック    内部でPOSTYPEを判断
 * @param	evdata イベントデータへのポインタ
 * @param pos チェックする座標　　VecFx32* or RAIL_LOCATION*
 * @param talk_dir 話し掛け対象の向き
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
static u16 EventData_CheckTalkBGEventBase( 
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const void *pos, u16 talk_dir )
{
  const BG_TALK_DATA *data = evdata->bg_data;
  
  if( data != NULL )
  {
    u16 i = 0;
    u16 max = evdata->bg_count;
    for( ; i < max; i++, data++ )
    {
      if(data->pos_type == EVENTDATA_POSTYPE_GRID)
      {
        if( !BGTalkData_GPOS_IsHit( data, pos ) )
        {
          continue;
        }
      }
      else
      {
        if( !BGTalkData_RPOS_IsHit( data, pos ) )
        {
          continue;
        }
      }
      
      if( data->type == BG_TALK_TYPE_HIDE )
      {
        u16 flag = SCRIPT_GetHideItemFlagNoByScriptID( data->id );
          
        if( EVENTWORK_CheckEventFlag(evwork,flag) == FALSE ){
          return data->id;
        }
      }
      else
      {
        switch( data->dir )
        {
        case BG_TALK_DIR_DOWN:
          if( talk_dir == DIR_UP ){
            return data->id;
          }
          break;
        case BG_TALK_DIR_LEFT:
          if( talk_dir == DIR_RIGHT ){
            return data->id;
          }
          break;
        case BG_TALK_DIR_RIGHT:
          if( talk_dir == DIR_LEFT ){
            return data->id;
          }
          break;
        case BG_TALK_DIR_UP:
          if( talk_dir == DIR_DOWN ){
             return data->id;
          }
          break;
        case BG_TALK_DIR_ALL:
          return data->id;
        case BG_TALK_DIR_SIDE:
          if( talk_dir == DIR_LEFT || talk_dir == DIR_RIGHT ){
            return data->id;
          }
          break;
        case BG_TALK_DIR_UPDOWN:
          if( talk_dir == DIR_UP || talk_dir == DIR_DOWN ){
            return data->id;
          }
          break;
        default:
          GF_ASSERT( 0 );
        }
      }
    }
  }
  
  return EVENTDATA_ID_NONE;
}
  
//------------------------------------------------------------------
/**
 * @brief	BG話し掛けイベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param pos チェックする座標
 * @param talk_dir 話し掛け対象の向き
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
u16 EVENTDATA_CheckTalkBGEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const VecFx32 *pos, u16 talk_dir )
{
  return EventData_CheckTalkBGEventBase( evdata, evwork, pos, talk_dir );
}

//------------------------------------------------------------------
/**
 * @brief	BG話し掛けイベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param pos チェックする座標
 * @param talk_dir 話し掛け対象の向き
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
u16 EVENTDATA_CheckTalkBGEventRailLocation(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const RAIL_LOCATION *pos, u16 talk_dir )
{
  return EventData_CheckTalkBGEventBase( evdata, evwork, pos, talk_dir );
}



//------------------------------------------------------------------
/**
 * @brief	看板話し掛けイベントチェック    内部でPOSTYPE判定
 * @param	evdata イベントデータへのポインタ
 * @param pos チェックする座標
 * @param talk_dir 話し掛け対象の向き
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
static u16 EventData_CheckTalkBoardEventBase( 
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const void *pos, u16 talk_dir )
{
  const BG_TALK_DATA *data = evdata->bg_data;
  
  /*
  if( data->type != BG_TALK_TYPE_BOARD )
  { // 看板以外はチェックしない
    continue;
  }
  */
  if( talk_dir == DIR_UP && data != NULL )
  {
    u16 i = 0;
    u16 max = evdata->bg_count;
    
    for( ; i < max; i++, data++ )
    {
      if( data->pos_type == EVENTDATA_POSTYPE_GRID )
      {
        if( !BGTalkData_GPOS_IsHit( data, pos ) )
        {
          continue;
        }
      }
      else
      {
        if( !BGTalkData_RPOS_IsHit( data, pos ) )
        {
          continue;
        }
      }
      return data->id;
    }
  }
  
  return EVENTDATA_ID_NONE;
}

//------------------------------------------------------------------
/**
 * @brief	看板話し掛けイベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param pos チェックする座標
 * @param talk_dir 話し掛け対象の向き
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
u16 EVENTDATA_CheckTalkBoardEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const VecFx32 *pos, u16 talk_dir )
{
  return EventData_CheckTalkBoardEventBase( evdata, evwork, pos, talk_dir );
}

//------------------------------------------------------------------
/**
 * @brief	看板話し掛けイベントチェック
 * @param	evdata イベントデータへのポインタ
 * @param pos チェックする座標
 * @param talk_dir 話し掛け対象の向き
 * @retval u16 EVENTDATA_ID_NONE = イベントなし
 */
//------------------------------------------------------------------
u16 EVENTDATA_CheckTalkBoardEventRailLocation(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const RAIL_LOCATION *pos, u16 talk_dir )
{
  return EventData_CheckTalkBoardEventBase( evdata, evwork, pos, talk_dir );
}



#if 0
//----------------------------------------------------------------------------
/**
 *	@brief  出入り口イベント　中心３D座標を取得
 *  
 *	@param	data    イベント
 *	@param	pos     座標格納先
 */
//-----------------------------------------------------------------------------
void EVENTDATA_GetConnectCenterPos( const CONNECT_DATA * data, VecFx32* pos )
{
  GF_ASSERT( data );
  GF_ASSERT( pos );
  ConnectData_GPOS_GetPos( data, LOCATION_DEFAULT_EXIT_OFS, pos );
}
void EVENTDATA_GetConnectCenterRailLocation( const CONNECT_DATA * data, RAIL_LOCATION* location )
{
  GF_ASSERT( data );
  GF_ASSERT( location );
  ConnectData_RPOS_GetLocation( data, location );
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  BG話しかけイベント　中心３D座標を取得
 *  
 *	@param	data    イベント
 *	@param	pos     座標格納先
 */
//-----------------------------------------------------------------------------
void EVENTDATA_GetBGTalkCenterPos( const BG_TALK_DATA * data, VecFx32* pos )
{
  GF_ASSERT( data );
  GF_ASSERT( pos );
  BGTalkData_GPOS_GetPos( data, pos );
}
void EVENTDATA_GetBGTalkCenterRailLocation( const BG_TALK_DATA * data, RAIL_LOCATION* location )
{
  GF_ASSERT( data );
  GF_ASSERT( location );
  BGTalkData_RPOS_GetLocation( data, location );
}


//----------------------------------------------------------------------------
/**
 *	@brief  POS発動イベント　中心３D座標を取得
 *  
 *	@param	data    イベント
 *	@param	pos     座標格納先
 */
//-----------------------------------------------------------------------------
void EVENTDATA_GetPosEventCenterPos( const POS_EVENT_DATA * data, VecFx32* pos )
{
  GF_ASSERT( data );
  GF_ASSERT( pos );
  PosEventData_GPOS_GetPos( data, pos );
}
void EVENTDATA_GetPosEventCenterRailLocation( const POS_EVENT_DATA * data, RAIL_LOCATION* location )
{
  GF_ASSERT( data );
  GF_ASSERT( location );
  PosEventData_RPOS_GetLocation( data, location );
}









//============================================================================================
// CONNECT_DATA ポジション
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  飛び先と飛び元のサイズから、飛び先での、オフセットを求める
 *
 *	@param	exit_ofs    飛び元出口オフセット
 *	@param	size        飛び先サイズ
 *
 *	@return オフセット
 */
//-----------------------------------------------------------------------------
static u16 convertOfs(const LOC_EXIT_OFS exit_ofs, u16 size)
{
  if (exit_ofs == LOCATION_DEFAULT_EXIT_OFS) return 0;
  {
    u32 enter_size = LOC_EXIT_OFS_GET_SIZE(exit_ofs);
    u32 enter_ofs = LOC_EXIT_OFS_GET_OFS(exit_ofs);
    s32 ret_ofs;
    s32 size_diff;
    u32 enter_size_half;
    u32 size_half;

    // 差、中心を求める
    size_diff = size - enter_size;
    enter_size_half = enter_size / 2;
    size_half = size / 2;
    
    // 偶数ー＞奇数
    //    中心の２グリッドを奇数の中心１グリッドにまとめ
    //    その他は、そのまま移す。
    //     a,bc,d
    //       ↑
    //    a,b,c,d
    //
    //  奇数ー＞偶数
    //    中心は、2グリッドのインデックスの小さいほうに格納し、
    //    その他は、そのまま移す
    //    a,b,0,c
    //       ↑
    //     a,b,c

    // 同じ
    if( size_diff == 0 )
    {
      ret_ofs = enter_ofs;
    }
    // 奇数
    else if( (size_diff % 2) )
    {
      // 飛び元オフセットを中心からの距離にして、飛び先の中心を足してあげる。　飛び先では、インデックスの小さいほうを選択
      if( enter_ofs <= enter_size_half )
      {
        ret_ofs = enter_ofs - enter_size_half + (size_half-1);
      }
      else
      {
        // はんぶんから大きい場合は、インデックスの大きいほうを選択
        ret_ofs = enter_ofs - enter_size_half + (size_half);
      }
    }
    // 偶数
    else
    {
      // 中心の2つを飛び先の中心に合わせる
      if( enter_ofs < enter_size_half )
      {
        ret_ofs = enter_ofs - (enter_size_half-1) + (size_half);
      }
      else
      {
        ret_ofs = enter_ofs - enter_size_half + (size_half);
      }
    }

    // 範囲オーバーチェック
    if( ret_ofs < 0 )
    {
      ret_ofs = 0;
    }
    else if( ret_ofs >= size )
    {
      ret_ofs = size -1;
    }
    return ret_ofs;
  }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static LOC_EXIT_OFS ConnectData_GPOS_GetExitOfs( const CONNECT_DATA * cp_data, const VecFx32 * p_pos )
{
  const CONNECT_DATA_GPOS * cp_gpos;
  
  GF_ASSERT( cp_data );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_GRID );

  cp_gpos = (const CONNECT_DATA_GPOS *)cp_data->pos_buf;
  if (cp_gpos->sizex > 1 )
  {
    s32 ofs = (FX_Whole(p_pos->x) - cp_gpos->x) / FIELD_CONST_GRID_SIZE;
    OS_Printf("p_pos->x:%d cp_gpos->x:%d ofs:%d\n", FX_Whole(p_pos->x), cp_gpos->x, ofs);
    GF_ASSERT( ofs >= 0 );
    GF_ASSERT( ofs < cp_gpos->sizex );
    return LOC_EXIT_OFS_DEF(cp_gpos->sizex, ofs);
  }
  else if (cp_gpos->sizez > 1 )
  {
    s32 ofs = (FX_Whole(p_pos->z) - cp_gpos->z) / FIELD_CONST_GRID_SIZE;
    OS_Printf("p_pos->z:%d cp_gpos->z:%d ofs:%d\n", FX_Whole(p_pos->z), cp_gpos->z, ofs);
    GF_ASSERT( ofs >= 0 );
    GF_ASSERT( ofs < cp_gpos->sizez );
    return LOC_EXIT_OFS_DEF(cp_gpos->sizez, ofs);
  }
  else {
    return LOC_EXIT_OFS_DEF(1, 0);
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static LOC_EXIT_OFS ConnectData_RPOS_GetExitOfs( const CONNECT_DATA * cp_data, const RAIL_LOCATION * cp_location )
{
  const CONNECT_DATA_RPOS * cp_rpos;
  
  GF_ASSERT( cp_data );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL );

  cp_rpos = (const CONNECT_DATA_RPOS *)cp_data->pos_buf;
  if (cp_rpos->side_grid_size > 1 )
  {
    s32 ofs = cp_location->width_grid - cp_rpos->side_grid;
    GF_ASSERT( ofs >= 0 );
    GF_ASSERT( ofs < cp_rpos->side_grid_size );
    TOMOYA_Printf( "side grid size %d   ofs %d\n", cp_rpos->side_grid_size, ofs );
    return LOC_EXIT_OFS_DEF( cp_rpos->side_grid_size, ofs );
  }
  else if (cp_rpos->front_grid_size > 1 )
  {
    s32 ofs = cp_location->line_grid - cp_rpos->front_grid;
    GF_ASSERT( ofs >= 0 );
    GF_ASSERT( ofs < cp_rpos->front_grid_size );
    TOMOYA_Printf( "front grid size %d   ofs %d\n", cp_rpos->front_grid_size, ofs );
    return LOC_EXIT_OFS_DEF( cp_rpos->front_grid_size, ofs );
  }
  else {
    return LOC_EXIT_OFS_DEF( 1, 0 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  中心　３D座標の取得
 *
 *	@param	cp_data   データ
 *	@param  exit_ofs  出入口オフセット
 *	@param	p_pos     座標格納先
 */
//-----------------------------------------------------------------------------
static void ConnectData_GPOS_GetPos( const CONNECT_DATA* cp_data, LOC_EXIT_OFS exit_ofs, VecFx32* p_pos )
{
  const CONNECT_DATA_GPOS * cp_pos;
  
  GF_ASSERT( cp_data );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_GRID );

  cp_pos = (const CONNECT_DATA_GPOS *)cp_data->pos_buf;

  p_pos->x = (cp_pos->x + CONNECT_POS_OFS_X)<<FX32_SHIFT;
  p_pos->y = (cp_pos->y + CONNECT_POS_OFS_Y)<<FX32_SHIFT;
  p_pos->z = (cp_pos->z + CONNECT_POS_OFS_Z)<<FX32_SHIFT;

  // 両方のサイズが1以上ならアサート
  GF_ASSERT( !((cp_pos->sizex > 1) && (cp_pos->sizez > 1)) );

  //exit_ofsを加えた座標を返す
  if (cp_pos->sizex > 1 )
  {
    u16 ret_ofs = convertOfs( exit_ofs, cp_pos->sizex );
    OS_Printf("p_pos->x:%d cp_gpos->x:%d ofs:%d\n", FX_Whole(p_pos->x), cp_pos->x, ret_ofs);
    OS_Printf("add x ofs %d\n", ret_ofs );
    p_pos->x += ret_ofs * FIELD_CONST_GRID_FX32_SIZE;
  }
  else if (cp_pos->sizez > 1 )
  {
    u16 ret_ofs = convertOfs( exit_ofs, cp_pos->sizez );
    OS_Printf("p_pos->z:%d cp_gpos->z:%d ofs:%d\n", FX_Whole(p_pos->z), cp_pos->z, ret_ofs);
    OS_Printf("add z ofs %d\n", ret_ofs );
    p_pos->z += ret_ofs * FIELD_CONST_GRID_FX32_SIZE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  グリッド座標と３D座標の判定
 *
 *	@param	cp_data   データ
 *	@param	cp_pos    ３D座標
 *
 *	@retval TRUE  ヒットした
 *	@retval FALSE ヒットしない
 */
//-----------------------------------------------------------------------------
static BOOL ConnectData_GPOS_IsHit( const CONNECT_DATA* cp_data, const VecFx32* cp_pos )
{
  const CONNECT_DATA_GPOS * cp_gpos;
  int x, y, z;
  
  GF_ASSERT( cp_data );

  // レールは反応しない
  if( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL )
  {
    return FALSE;
  }

  // グリッドポジション取得
  cp_gpos = (const CONNECT_DATA_GPOS *)cp_data->pos_buf;


	x = FX_Whole(cp_pos->x) - CONNECT_POS_OFS_X;
	y = FX_Whole(cp_pos->y) - CONNECT_POS_OFS_Y;
	z = FX_Whole(cp_pos->z) - CONNECT_POS_OFS_Z;

  GF_ASSERT (cp_gpos->sizex > 0 );
  GF_ASSERT (cp_gpos->sizez > 0 );
  if( (cp_gpos->x <= x) && ((cp_gpos->x+(cp_gpos->sizex-1)*FIELD_CONST_GRID_SIZE) >= x) )
  {
    if( ((cp_gpos->y-CONNECT_DATA_RANGE) <= y) && ((cp_gpos->y+CONNECT_DATA_RANGE) >= y) )
    {
      if( (cp_gpos->z <= z) && ((cp_gpos->z+(cp_gpos->sizez-1)*FIELD_CONST_GRID_SIZE) >= z) )
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  中心　レールロケーションの取得
 *
 *	@param	cp_data     データ
 *	@param	p_location  レールロケーションの格納先
 */
//-----------------------------------------------------------------------------
static void ConnectData_RPOS_GetLocation( const CONNECT_DATA* cp_data, LOC_EXIT_OFS exit_ofs, RAIL_LOCATION* p_location )
{
  const CONNECT_DATA_RPOS * cp_pos;
  u16 dir;
  
  GF_ASSERT( cp_data );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL );

  cp_pos = (const CONNECT_DATA_RPOS *)cp_data->pos_buf;

  switch( cp_data->exit_dir )
  {
	case EXIT_DIR_UP:
    dir = DIR_UP;
    break;
	case EXIT_DIR_DOWN:
    dir = DIR_DOWN;
    break;
	case EXIT_DIR_LEFT:
    dir = DIR_LEFT;
    break;
	case EXIT_DIR_RIGHT:
    dir = DIR_RIGHT;
    break;
  default:
    dir = DIR_DOWN;
    break;
  }

  p_location->rail_index  = cp_pos->rail_index;
  p_location->type        = FIELD_RAIL_TYPE_LINE;
  p_location->key         = FIELD_RAIL_TOOL_ConvertDirToRailKey( dir );

  // 両方のサイズが1以上ならアサート
  GF_ASSERT( !((cp_pos->side_grid_size > 1) && (cp_pos->front_grid_size > 1)) );

  // 
  p_location->width_grid  = cp_pos->side_grid;
  p_location->line_grid   = cp_pos->front_grid;

  //exit_ofsを加えた座標を返す
  if (cp_pos->side_grid_size > 1 )
  {
    u16 ret_ofs = convertOfs( exit_ofs, cp_pos->side_grid_size );
    p_location->width_grid += ret_ofs;
  }
  else if (cp_pos->front_grid_size > 1 )
  {
    u16 ret_ofs = convertOfs( exit_ofs, cp_pos->front_grid_size );
    p_location->line_grid += ret_ofs;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール座標の判定
 *
 *	@param	cp_data       データ
 *	@param	cp_location   レールロケーション
 *
 *	@retval TRUE  範囲内
 *	@retval FALSE 範囲外
 */
//-----------------------------------------------------------------------------
static BOOL ConnectData_RPOS_IsHit( const CONNECT_DATA* cp_data, const RAIL_LOCATION* cp_location )
{
  const CONNECT_DATA_RPOS * cp_pos;
  
  GF_ASSERT( cp_data );

  // グリッドは反応しない
  if( cp_data->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    return FALSE;
  }
  

  cp_pos = (const CONNECT_DATA_RPOS *)cp_data->pos_buf;

  if( cp_pos->rail_index == cp_location->rail_index )
  {
    if( (cp_pos->front_grid <= cp_location->line_grid) && ((cp_pos->front_grid+cp_pos->front_grid_size) > cp_location->line_grid) )
    {
      if( (cp_pos->side_grid <= cp_location->width_grid) && ((cp_pos->side_grid+cp_pos->side_grid_size) > cp_location->width_grid) )
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}



//============================================================================================
// BG_TALK_DATA ポジション
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  BGTalk　グリッドマップ　３D座標の取得
 *
 *	@param	cp_data データ
 *	@param	p_pos   ３D座標格納先
 */
//-----------------------------------------------------------------------------
static void BGTalkData_GPOS_GetPos( const BG_TALK_DATA* cp_data, VecFx32* p_pos )
{
  const BG_TALK_DATA_GPOS* cp_pos;

  GF_ASSERT( cp_data );
  GF_ASSERT( p_pos );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_GRID );

  cp_pos = (const BG_TALK_DATA_GPOS*)cp_data->pos_buf; 

  VEC_Set( p_pos, GRID_TO_FX32(cp_pos->gx), cp_pos->height<<FX32_SHIFT, GRID_TO_FX32(cp_pos->gz) );
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGTalk グリッドマップ　座標あたり判定
 *
 *	@param	cp_data   データ
 *	@param	cp_pos    ３D座標
 *
 *	@retval TRUE  範囲内
 *	@retval FALSE 範囲外
 */
//-----------------------------------------------------------------------------
static BOOL BGTalkData_GPOS_IsHit( const BG_TALK_DATA* cp_data, const VecFx32* cp_pos )
{
  const BG_TALK_DATA_GPOS* cp_gpos;
  int gx, gz, y;

  GF_ASSERT( cp_data );
  GF_ASSERT( cp_pos );
  
  if( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL )
  {
    return FALSE;
  }

  cp_gpos = (const BG_TALK_DATA_GPOS*)cp_data->pos_buf; 

  gx  = SIZE_GRID_FX32(cp_pos->x);
  gz  = SIZE_GRID_FX32(cp_pos->z);
  y   = FX_Whole(cp_pos->y);
  
  if( (cp_gpos->gx == gx) && (cp_gpos->gz == gz) )
  {
    if( ((cp_gpos->height-BG_TALK_DATA_RANGE) <= y) && ((cp_gpos->height+BG_TALK_DATA_RANGE) > y) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGTalk  レールロケーションの取得
 *
 *	@param	cp_data       データ
 *	@param	p_location    レールロケーション格納先
 */
//-----------------------------------------------------------------------------
static void BGTalkData_RPOS_GetLocation( const BG_TALK_DATA* cp_data, RAIL_LOCATION* p_location )
{
  const BG_TALK_DATA_RPOS * cp_pos;
  u16 dir;
  
  GF_ASSERT( cp_data );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL );

  cp_pos = (const BG_TALK_DATA_RPOS *)cp_data->pos_buf;

  switch( cp_data->dir )
  {
  case BG_TALK_DIR_DOWN:
    dir = DIR_DOWN;
    break;
  case BG_TALK_DIR_LEFT:
    dir = DIR_LEFT;
    break;
  case BG_TALK_DIR_RIGHT:
    dir = DIR_RIGHT;
    break;
  case BG_TALK_DIR_UP:
    dir = DIR_UP;
    break;
  default:
    dir = DIR_UP;
    break;
  }

  p_location->rail_index  = cp_pos->rail_index;
  p_location->type        = FIELD_RAIL_TYPE_LINE;
  p_location->key         = FIELD_RAIL_TOOL_ConvertDirToRailKey( dir );
  p_location->width_grid  = cp_pos->side_grid;
  p_location->line_grid   = cp_pos->front_grid;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール座標の判定
 *
 *	@param	cp_data       データ
 *	@param	cp_location   レールロケーション
 *
 *	@retval TRUE  範囲内
 *	@retval FALSE 範囲外
 */
//-----------------------------------------------------------------------------
static BOOL BGTalkData_RPOS_IsHit( const BG_TALK_DATA* cp_data, const RAIL_LOCATION* cp_location )
{
  const BG_TALK_DATA_RPOS * cp_pos;
  
  GF_ASSERT( cp_data );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    return FALSE;
  }

  cp_pos = (const BG_TALK_DATA_RPOS *)cp_data->pos_buf;

  if( cp_pos->rail_index == cp_location->rail_index )
  {
    if( cp_pos->front_grid == cp_location->line_grid )
    {
      if( cp_pos->side_grid == cp_location->width_grid )
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

//============================================================================================
// POS_EVENT_DATA ポジション
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  PosEvent　グリッドマップ　３D座標の取得
 *
 *	@param	cp_data データ
 *	@param	p_pos   ３D座標格納先
 */
//-----------------------------------------------------------------------------
static void PosEventData_GPOS_GetPos( const POS_EVENT_DATA* cp_data, VecFx32* p_pos )
{
  const POS_EVENT_DATA_GPOS* cp_pos;

  GF_ASSERT( cp_data );
  GF_ASSERT( p_pos );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_GRID );

  cp_pos = (const POS_EVENT_DATA_GPOS*)cp_data->pos_buf; 

  VEC_Set( p_pos, 
      GRID_TO_FX32(cp_pos->gx) + FX_Div( GRID_TO_FX32(cp_pos->sx), FX32_CONST(2) ),
      cp_pos->height<<FX32_SHIFT,
      GRID_TO_FX32(cp_pos->gz) + FX_Div( GRID_TO_FX32(cp_pos->sz), FX32_CONST(2) ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  PosEvent グリッドマップ　座標あたり判定
 *
 *	@param	cp_data   データ
 *	@param	cp_pos    ３D座標
 *
 *	@retval TRUE  範囲内
 *	@retval FALSE 範囲外
 */
//-----------------------------------------------------------------------------
static BOOL PosEventData_GPOS_IsHit( const POS_EVENT_DATA* cp_data, const VecFx32* cp_pos )
{
  const POS_EVENT_DATA_GPOS* cp_gpos;
  int gx, gz, y;

  GF_ASSERT( cp_data );
  GF_ASSERT( cp_pos );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL )
  {
    return FALSE;
  }

  cp_gpos = (const POS_EVENT_DATA_GPOS*)cp_data->pos_buf; 

  gx  = SIZE_GRID_FX32(cp_pos->x);
  gz  = SIZE_GRID_FX32(cp_pos->z);
  y   = FX_Whole(cp_pos->y);
  
  if( (cp_gpos->gx <= gx) && ((cp_gpos->gx+cp_gpos->sx) > gx) )
  {
    if( (cp_gpos->gz <= gz) && ((cp_gpos->gz+cp_gpos->sz) > gz) )
    {
      if( ((cp_gpos->height-BG_TALK_DATA_RANGE) <= y) && ((cp_gpos->height+BG_TALK_DATA_RANGE) > y) )
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  PosEvent グリッドマップ　座標あたり判定(xz平面ver.)
 *
 *	@param	cp_data   データ
 *	@param	cp_pos    ３D座標
 *
 *	@retval TRUE  範囲内
 *	@retval FALSE 範囲外
 */
//-----------------------------------------------------------------------------
static BOOL PosEventData_GPOS_IsHit_XZ( const POS_EVENT_DATA* cp_data, const VecFx32* cp_pos )
{
  const POS_EVENT_DATA_GPOS* cp_gpos;
  int gx, gz, y;

  GF_ASSERT( cp_data );
  GF_ASSERT( cp_pos );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL )
  {
    return FALSE;
  }

  cp_gpos = (const POS_EVENT_DATA_GPOS*)cp_data->pos_buf; 

  gx  = SIZE_GRID_FX32(cp_pos->x);
  gz  = SIZE_GRID_FX32(cp_pos->z);
  y   = FX_Whole(cp_pos->y);
  
  if( (cp_gpos->gx <= gx) && ((cp_gpos->gx+cp_gpos->sx) > gx) )
  {
    if( (cp_gpos->gz <= gz) && ((cp_gpos->gz+cp_gpos->sz) > gz) )
    {
      // y軸判定を行わないこと以外は PosEventData_GPOS_IsHit と同じ
      return TRUE;
    }
  }
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  PosEvent  レールロケーションの取得
 *
 *	@param	cp_data       データ
 *	@param	p_location    レールロケーション格納先
 */
//-----------------------------------------------------------------------------
static void PosEventData_RPOS_GetLocation( const POS_EVENT_DATA* cp_data, RAIL_LOCATION* p_location )
{
  const POS_EVENT_DATA_RPOS * cp_pos;
  
  GF_ASSERT( cp_data );
  GF_ASSERT( cp_data->pos_type == EVENTDATA_POSTYPE_RAIL );

  cp_pos = (const POS_EVENT_DATA_RPOS *)cp_data->pos_buf;

  p_location->rail_index  = cp_pos->rail_index;
  p_location->type        = FIELD_RAIL_TYPE_LINE;
  p_location->key         = 0;
  p_location->width_grid  = cp_pos->side_grid + (cp_pos->side_grid_size/2);
  p_location->line_grid   = cp_pos->front_grid + (cp_pos->front_grid_size/2);
}

//----------------------------------------------------------------------------
/**
 *	@brief  PosEvent レール座標の判定
 *
 *	@param	cp_data       データ
 *	@param	cp_location   レールロケーション
 *
 *	@retval TRUE  範囲内
 *	@retval FALSE 範囲外
 */
//-----------------------------------------------------------------------------
static BOOL PosEventData_RPOS_IsHit( const POS_EVENT_DATA* cp_data, const RAIL_LOCATION* cp_location )
{
  const POS_EVENT_DATA_RPOS * cp_pos;
  
  GF_ASSERT( cp_data );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    return FALSE;
  }

  cp_pos = (const POS_EVENT_DATA_RPOS *)cp_data->pos_buf;

  if( cp_pos->rail_index == cp_location->rail_index )
  {
    if( (cp_pos->front_grid <= cp_location->line_grid) && ((cp_pos->front_grid+cp_pos->front_grid_size) > cp_location->line_grid) )
    {
      if( (cp_pos->side_grid <= cp_location->width_grid) && ((cp_pos->side_grid+cp_pos->side_grid_size) > cp_location->width_grid) )
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}



#ifdef DEBUG_EVENTDATA_PRINT


//----------------------------------------------------------------------------
/**
 *	@brief  ドアデータの表示
 */
//-----------------------------------------------------------------------------
static void debugPrint_Connect( const CONNECT_DATA* cp_data )
{
  OS_TPrintf( "print CONNECT_DATA\n" );
  OS_TPrintf( "link_zone_id=%d\n", cp_data->link_zone_id );
  OS_TPrintf( "link_exit_id=%d\n", cp_data->link_exit_id );
  OS_TPrintf( "exit_dir=%d\n", cp_data->exit_dir );
  OS_TPrintf( "exit_type=%d\n", cp_data->exit_type );

  OS_TPrintf( "pos_type=%d\n", cp_data->pos_type );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    const CONNECT_DATA_GPOS* cp_pos = (const CONNECT_DATA_GPOS*)cp_data->pos_buf;
    OS_TPrintf( "x=%d\n", cp_pos->x );
    OS_TPrintf( "y=%d\n", cp_pos->y );
    OS_TPrintf( "z=%d\n", cp_pos->z );
    OS_TPrintf( "sizex=%d\n", cp_pos->sizex );
    OS_TPrintf( "sizez=%d\n", cp_pos->sizez );
  }
  else
  {
    const CONNECT_DATA_RPOS* cp_pos = (const CONNECT_DATA_RPOS*)cp_data->pos_buf;
    OS_TPrintf( "rail_index=%d\n", cp_pos->rail_index );
    OS_TPrintf( "front_grid=%d\n", cp_pos->front_grid );
    OS_TPrintf( "side_grid=%d\n", cp_pos->side_grid );
    OS_TPrintf( "front_grid_size=%d\n", cp_pos->front_grid_size );
    OS_TPrintf( "side_grid_size=%d\n", cp_pos->side_grid_size );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG話しかけデータの表示
 */
//-----------------------------------------------------------------------------
static void debugPrint_BGTalk( const BG_TALK_DATA* cp_data )
{
  OS_TPrintf( "print BG_TALK_DATA\n" );
  OS_TPrintf( "id=%d\n", cp_data->id );
  OS_TPrintf( "type=%d\n", cp_data->type );
  OS_TPrintf( "dir=%d\n", cp_data->dir );

  OS_TPrintf( "pos_type=%d\n", cp_data->pos_type );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    const BG_TALK_DATA_GPOS* cp_pos = (const BG_TALK_DATA_GPOS*)cp_data->pos_buf;
    OS_TPrintf( "gx=%d\n", cp_pos->gx );
    OS_TPrintf( "gz=%d\n", cp_pos->gz );
    OS_TPrintf( "height=%d\n", cp_pos->height );
  }
  else
  {
    const BG_TALK_DATA_RPOS* cp_pos = (const BG_TALK_DATA_RPOS*)cp_data->pos_buf;
    OS_TPrintf( "rail_index=%d\n", cp_pos->rail_index );
    OS_TPrintf( "front_grid=%d\n", cp_pos->front_grid );
    OS_TPrintf( "side_grid=%d\n", cp_pos->side_grid );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPC情報の表示
 */
//-----------------------------------------------------------------------------
static void debugPrint_MMLD( const MMDL_HEADER* cp_data )
{
  OS_TPrintf( "print MMDL_HEADER\n" );
  OS_TPrintf( "id=%d\n", cp_data->id );
  OS_TPrintf( "obj_code=%d\n", cp_data->obj_code );
  OS_TPrintf( "move_code=%d\n", cp_data->move_code );
  OS_TPrintf( "event_type=%d\n", cp_data->event_type );
  OS_TPrintf( "event_flag=%d\n", cp_data->event_flag );
  OS_TPrintf( "event_id=%d\n", cp_data->event_id );
  OS_TPrintf( "dir=%d\n", cp_data->dir );
  OS_TPrintf( "param0=%d\n", cp_data->param0 );
  OS_TPrintf( "param1=%d\n", cp_data->param1 );
  OS_TPrintf( "param2=%d\n", cp_data->param2 );
  OS_TPrintf( "move_limit_x=%d\n", cp_data->move_limit_x );
  OS_TPrintf( "move_limit_z=%d\n", cp_data->move_limit_z );

  OS_TPrintf( "pos_type=%d\n", cp_data->pos_type );

  if( cp_data->pos_type == MMDL_HEADER_POSTYPE_GRID )
  {
    const MMDL_HEADER_GRIDPOS* cp_pos = (const MMDL_HEADER_GRIDPOS*)cp_data->pos_buf;
    OS_TPrintf( "gx=%d\n", cp_pos->gx );
    OS_TPrintf( "gz=%d\n", cp_pos->gz );
    OS_TPrintf( "y=%d\n", cp_pos->y );
  }
  else
  {
    const MMDL_HEADER_RAILPOS* cp_pos = (const MMDL_HEADER_RAILPOS*)cp_data->pos_buf;
    OS_TPrintf( "rail_index=%d\n", cp_pos->rail_index );
    OS_TPrintf( "front_grid=%d\n", cp_pos->front_grid );
    OS_TPrintf( "side_grid=%d\n", cp_pos->side_grid );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  POS発動データの表示
 */
//-----------------------------------------------------------------------------
static void debugPrint_PosData( const POS_EVENT_DATA* cp_data )
{
  OS_TPrintf( "print POS_EVENT_DATA\n" );
  OS_TPrintf( "id=%d\n", cp_data->id );
  OS_TPrintf( "check=%d\n", cp_data->check_type );
  OS_TPrintf( "param=%d\n", cp_data->param );
  OS_TPrintf( "workID=%d\n", cp_data->workID );

  OS_TPrintf( "pos_type=%d\n", cp_data->pos_type );

  if( cp_data->pos_type == EVENTDATA_POSTYPE_GRID )
  {
    const POS_EVENT_DATA_GPOS* cp_pos = (const POS_EVENT_DATA_GPOS*)cp_data->pos_buf;
    OS_TPrintf( "gx=%d\n", cp_pos->gx );
    OS_TPrintf( "gz=%d\n", cp_pos->gz );
    OS_TPrintf( "sx=%d\n", cp_pos->sx );
    OS_TPrintf( "sz=%d\n", cp_pos->sz );
    OS_TPrintf( "height=%d\n", cp_pos->height );
  }
  else
  {
    const POS_EVENT_DATA_RPOS* cp_pos = (const POS_EVENT_DATA_RPOS*)cp_data->pos_buf;
    OS_TPrintf( "rail_index=%d\n", cp_pos->rail_index );
    OS_TPrintf( "front_grid=%d\n", cp_pos->front_grid );
    OS_TPrintf( "side_grid=%d\n", cp_pos->side_grid );
    OS_TPrintf( "front_grid_size=%d\n", cp_pos->front_grid_size );
    OS_TPrintf( "side_grid_size=%d\n", cp_pos->side_grid_size );
  }
}

#endif //DEBUG_EVENTDATA_PRINT




