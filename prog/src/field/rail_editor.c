//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		rail_editor.c
 *	@brief	レールエディター	DS側処理
 *	@author	tomoya takahashi
 *	@date		2009.07.13
 *
 *	モジュール名：RIAL_EDITOR
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// デバック用機能
#ifdef PM_DEBUG

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "event_fieldmap_control.h"
#include "field_player_nogrid.h"

#include "debug/gf_mcs.h"

#include "rail_editor.h"
#include "rail_editor_data.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	シーケンス
//=====================================
enum {
	RAIL_EDITOR_SEQ_INIT,				// 初期化
	RAIL_EDITOR_SEQ_CONNECT,		// 接続
	RAIL_EDITOR_SEQ_RESET,			// リセット起動
	RAIL_EDITOR_SEQ_MAIN,				// リクエスト対応
	RAIL_EDITOR_SEQ_CLOSE,			// 切断
	RAIL_EDITOR_SEQ_EXIT,				// 破棄

} ;

// 汎用受信バッファ
#define RE_TMP_BUFF_SIZE	( 8198 )

//-------------------------------------
///	レール、エリア受信バッファサイズ
//=====================================
#define RE_MCS_BUFF_RAIL_SIZE	( 8198 )
#define RE_MCS_BUFF_AREA_SIZE	( 1024 )


//-------------------------------------
///	CIRCLE移動
//=====================================
#define RM_DEF_ANGLE_MOVE	(16)
#define RM_DEF_LEN_MOVE		(FX32_ONE)
#define RM_DEF_CAMERA_LEN	( 0x38D000 )
#define RM_DEF_CAMERA_PITCH	( 0x800 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	受信情報バッファ
//=====================================
typedef struct {
	// フラグ部
	union
	{
		struct
		{
			u32	rail_recv:1;			// レール情報受信
			u32	area_recv:1;			// エリア情報受信
			u32	select_recv:1;		// 選択情報受信
			u32	rail_req:1;				// レール情報送信リクエスト
			u32	area_req:1;				// エリア情報送信リクエスト
			u32	player_req:1;			// プレイヤー情報送信リクエスト
			u32	camera_req:1;			// カメラ情報送信リクエスト
			u32	reset_req:1;			// リセットリクエスト
			u32 end_req:1;				// 終了リクエスト
			u32	pad_data:23;
		};
		u32 flag;	
	};

	// データ部
	u32	rail[RE_MCS_BUFF_RAIL_SIZE/4];
	u32 rail_size;
	u32 area[RE_MCS_BUFF_AREA_SIZE/4];
	u32 area_size;
	RE_MCS_SELECT_DATA select;
} RE_RECV_BUFF;


//-------------------------------------
///	レールエディターイベントワーク
//=====================================
typedef struct {
	FIELDMAP_WORK* p_fieldmap;		// フィールドマップ
	RE_RECV_BUFF*	p_recv;						// 受信バッファ
	u32*					p_tmp_buff;// 汎用受信バッファ
	VecFx32				camera_target;
} DEBUG_RAIL_EDITOR;


FS_EXTERN_OVERLAY(mcs_lib);

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static GMEVENT_RESULT DEBUG_RailEditorEvent(
    GMEVENT * p_event, int *  p_seq, void * p_work );


// 受信データ、対応関数
static void RE_Recv_ClearFlag( RE_RECV_BUFF* p_buff );
static BOOL RE_Recv_IsEnd( const RE_RECV_BUFF* cp_buff );
static BOOL RE_Recv_IsReset( const RE_RECV_BUFF* cp_buff );
static void RE_RecvControl( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Recv_Rail( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_Area( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_SelectData( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_RailReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_AreaReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_PlayerDataReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_CameraDataReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_ResetReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );
static void RE_Recv_EndReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size );

// 受信情報の反映
static void RE_Reflect( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Reflect_Rail( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Reflect_Area( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Reflect_Select( DEBUG_RAIL_EDITOR* p_wk );
static void RE_JumpPoint( DEBUG_RAIL_EDITOR* p_wk );
static void RE_JumpLine( DEBUG_RAIL_EDITOR* p_wk );
static void RE_JumpArea( DEBUG_RAIL_EDITOR* p_wk );

// 選択情報、コントロール関数
static void RE_InputControl( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputPoint_FreeNormal( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputPoint_FreeCircle( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputPoint_Rail( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLine_FreeNormal( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLine_FreeCircle( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLine_Rail( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputArea_FreeNormal( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputArea_FreeCircle( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputArea_Rail( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputCamera_FreeNormal( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputCamera_FreeCircle( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputCamera_Pos( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputCamera_Angle( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputCamera_Target( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputCamera_Rail( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLinePos_FreeNormal( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLinePos_FreeCircle( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLinePos_CenterPos( DEBUG_RAIL_EDITOR* p_wk );
static void RE_InputLinePos_Rail( DEBUG_RAIL_EDITOR* p_wk );

// 送信関数
static void RE_SendControl( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Send_RailData( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Send_AreaData( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Send_PlayerData( DEBUG_RAIL_EDITOR* p_wk );
static void RE_Send_CameraData( DEBUG_RAIL_EDITOR* p_wk );


// 主人公、ノーマル移動
static void RE_FreeMove_Normal( DEBUG_RAIL_EDITOR* p_wk );
// 主人公、回転移動
static void RE_FreeMove_Circle( DEBUG_RAIL_EDITOR* p_wk );
// 主人公　レール動作
static void RE_RailMove( DEBUG_RAIL_EDITOR* p_wk );


//----------------------------------------------------------------------------
/**
 *	@brief	レールエディターイベント開始
 *
 *	@param	gsys				システム
 *	@param	fieldmap		フィールドマップ
 *
 *	@return	イベントワーク
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_RailEditor( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT* p_event;
  DEBUG_RAIL_EDITOR* p_wk;

  p_event = GMEVENT_Create(
      gsys, NULL, DEBUG_RailEditorEvent, sizeof(DEBUG_RAIL_EDITOR) );

	p_wk = GMEVENT_GetEventWork( p_event );
	p_wk->p_fieldmap = fieldmap;

	return p_event;
}


//-----------------------------------------------------------------------------
/**
 *			private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	レールエディターイベント
 *
 *	@retval GMEVENT_RES_CONTINUE = 0,
 *	@retval	GMEVENT_RES_FINISH,
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT DEBUG_RailEditorEvent( GMEVENT * p_event, int *  p_seq, void * p_work )
{
  DEBUG_RAIL_EDITOR* p_wk = p_work;

	switch( *p_seq )
	{
	// 初期化
	case RAIL_EDITOR_SEQ_INIT:

		GFL_OVERLAY_Load(FS_OVERLAY_ID(mcs_lib));

		// バッファのメモリ確保
		p_wk->p_recv = GFL_HEAP_AllocClearMemory( FIELDMAP_GetHeapID(p_wk->p_fieldmap), sizeof(RE_RECV_BUFF) );
		p_wk->p_tmp_buff = GFL_HEAP_AllocClearMemory( FIELDMAP_GetHeapID(p_wk->p_fieldmap), RE_TMP_BUFF_SIZE );

		// カメラターゲット設定
//		FIELD_CAMERA_BindTarget( FIELDMAP_GetFieldCamera( p_wk->p_fieldmap ), &p_wk->camera_target );

		// 座標を設定
		FIELD_PLAYER_GetPos( FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap ), &p_wk->camera_target );
		
		(*p_seq) = RAIL_EDITOR_SEQ_CONNECT;
		break;

	// 接続
	case RAIL_EDITOR_SEQ_CONNECT:	
		if( !MCS_Init( FIELDMAP_GetHeapID(p_wk->p_fieldmap) ) )
		{
			(*p_seq) = RAIL_EDITOR_SEQ_MAIN;
		}
		break;

	// リセット起動
	case RAIL_EDITOR_SEQ_RESET:
		(*p_seq) = RAIL_EDITOR_SEQ_MAIN;
		break;


	// リクエスト対応
	case RAIL_EDITOR_SEQ_MAIN:

		// MCSメイン
		MCS_Main();

		// 受信情報フラグ部破棄
		RE_Recv_ClearFlag( p_wk->p_recv );

		// 受信情報対応
		RE_RecvControl( p_wk );

		// 終了受信
		if( RE_Recv_IsEnd( p_wk->p_recv ) )
		{
			(*p_seq)++;
			break;
		}

		// リセット受信
		if( RE_Recv_IsReset( p_wk->p_recv ) ){
			break;
		}
		
		// 受信情報反映
		RE_Reflect(p_wk);
		
		// 選択情報、操作対応
		RE_InputControl( p_wk );

		// 情報送信管理
		RE_SendControl( p_wk );

		break;

	// 切断
	case RAIL_EDITOR_SEQ_CLOSE:			
		(*p_seq) ++;
		break;

	// 破棄
	case RAIL_EDITOR_SEQ_EXIT:				
		MCS_Exit();

		GFL_OVERLAY_Unload( FS_OVERLAY_ID( mcs_lib ) );

		// バッファのメモリ破棄
		GFL_HEAP_FreeMemory( p_wk->p_recv );
		GFL_HEAP_FreeMemory( p_wk->p_tmp_buff );
		return GMEVENT_RES_FINISH;

	default:
		GF_ASSERT( 0 );
		break;
	}

  return GMEVENT_RES_CONTINUE;
}






// 受信データ、対応関数
//----------------------------------------------------------------------------
/**
 *	@brief	受信バッファフラグ部クリア
 *
 *	@param	p_buff バッファ
 */
//-----------------------------------------------------------------------------
static void RE_Recv_ClearFlag( RE_RECV_BUFF* p_buff )
{
	GF_ASSERT( p_buff );
	p_buff->flag = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信バッファ　終了フラグチェック
 *	
 *	@param	cp_buff		バッファ
 *
 *	@retval	TRUE	終了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
static BOOL RE_Recv_IsEnd( const RE_RECV_BUFF* cp_buff )
{
	GF_ASSERT( cp_buff );
	return cp_buff->end_req;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信バッファ	リセットフラグチェック
 *
 *	@param	cp_buff		バッファ
 *
 *	@retval	TRUE	リセット
 *	@retval	FALSE	リセットじゃない
 */
//-----------------------------------------------------------------------------
static BOOL RE_Recv_IsReset( const RE_RECV_BUFF* cp_buff )
{
	GF_ASSERT( cp_buff );
	return cp_buff->reset_req;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信情報管理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_RecvControl( DEBUG_RAIL_EDITOR* p_wk )
{
	const RE_MCS_HEADER* cp_header;
	u32 size;
	static void (* const cpRecv[RE_MCS_DATA_TYPE_MAX])( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size ) = 
	{
		NULL,
		NULL,
		RE_Recv_Rail,
		RE_Recv_Area,
		NULL,
		NULL,
		RE_Recv_SelectData,
		RE_Recv_RailReq,
		RE_Recv_AreaReq,
		RE_Recv_PlayerDataReq,
		RE_Recv_CameraDataReq,
		RE_Recv_ResetReq,
		RE_Recv_EndReq,
	};

	// 受信データがあるかチェック
	if( MCS_CheckRead() != 0 )
	{
		size = MCS_Read( p_wk->p_tmp_buff, RE_TMP_BUFF_SIZE );
		if( size != 0 )
		{
			cp_header = (const RE_MCS_HEADER*)p_wk->p_tmp_buff;
				
			// 未知のデータが来た
			GF_ASSERT( cp_header->data_type < RE_MCS_DATA_TYPE_MAX );

			if( cpRecv[ cp_header->data_type ] )
			{
				cpRecv[ cp_header->data_type ]( p_wk, p_wk->p_tmp_buff, size );
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール情報の受信
 */
//-----------------------------------------------------------------------------
static void RE_Recv_Rail( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_RAIL_DATA* cp_rail = cp_data;

	GF_ASSERT( cp_rail->header.data_type == RE_MCS_DATA_RAIL );

	p_wk->p_recv->rail_recv = TRUE;
	p_wk->p_recv->reset_req = cp_rail->reset;

	GF_ASSERT( size < RE_MCS_BUFF_RAIL_SIZE );
	GFL_STD_MemCopy( cp_rail, p_wk->p_recv->rail, size );

	p_wk->p_recv->rail_size = size;
}

//----------------------------------------------------------------------------
/**
 *	@brief	エリア情報受信
 */
//-----------------------------------------------------------------------------
static void RE_Recv_Area( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_AREA_DATA* cp_area = cp_data;

	GF_ASSERT( cp_area->header.data_type == RE_MCS_DATA_AREA );

	p_wk->p_recv->area_recv = TRUE;
	p_wk->p_recv->reset_req = cp_area->reset;

	GF_ASSERT( size < RE_MCS_BUFF_AREA_SIZE );
	GFL_STD_MemCopy( cp_area, p_wk->p_recv->area, size );
	p_wk->p_recv->area_size = size;
}

//----------------------------------------------------------------------------
/**
 *	@brief	選択情報の受信
 */
//-----------------------------------------------------------------------------
static void RE_Recv_SelectData( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_SELECT_DATA* cp_select = cp_data;

	GF_ASSERT( cp_select->header.data_type == RE_MCS_DATA_SELECTDATA );

	p_wk->p_recv->select_recv = TRUE;

	GF_ASSERT( size == sizeof(RE_MCS_SELECT_DATA) );
	GFL_STD_MemCopy( cp_select, &p_wk->p_recv->select, size );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール情報送信リクエスト
 */
//-----------------------------------------------------------------------------
static void RE_Recv_RailReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_HEADER* cp_header = cp_data;

	GF_ASSERT( cp_header->data_type == RE_MCS_DATA_RAIL_REQ );
	p_wk->p_recv->rail_req = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	エリア情報送信リクエスト
 */
//-----------------------------------------------------------------------------
static void RE_Recv_AreaReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_HEADER* cp_header = cp_data;

	GF_ASSERT( cp_header->data_type == RE_MCS_DATA_AREA_REQ );
	p_wk->p_recv->area_req = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤー情報送信リクエスト
 */
//-----------------------------------------------------------------------------
static void RE_Recv_PlayerDataReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_HEADER* cp_header = cp_data;

	GF_ASSERT( cp_header->data_type == RE_MCS_DATA_PLAYER_REQ );
	p_wk->p_recv->player_req = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ情報送信リクエスト
 */
//-----------------------------------------------------------------------------
static void RE_Recv_CameraDataReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_HEADER* cp_header = cp_data;

	GF_ASSERT( cp_header->data_type == RE_MCS_DATA_CAMERA_REQ );
	p_wk->p_recv->camera_req = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	リセット　リクエスト
 */
//-----------------------------------------------------------------------------
static void RE_Recv_ResetReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_HEADER* cp_header = cp_data;

	GF_ASSERT( cp_header->data_type == RE_MCS_DATA_RESET_REQ );
	p_wk->p_recv->reset_req = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了　リクエスト受信
 */
//-----------------------------------------------------------------------------
static void RE_Recv_EndReq( DEBUG_RAIL_EDITOR* p_wk, const void* cp_data, u32 size )
{
	const RE_MCS_HEADER* cp_header = cp_data;

	GF_ASSERT( cp_header->data_type == RE_MCS_DATA_END_REQ );
	p_wk->p_recv->end_req = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信情報を　反映	
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Reflect( DEBUG_RAIL_EDITOR* p_wk )
{
	if( p_wk->p_recv->rail_recv )
	{
		RE_Reflect_Rail( p_wk );
	}
	if( p_wk->p_recv->area_recv )
	{
		RE_Reflect_Area( p_wk );
	}
	if( p_wk->p_recv->select_recv )
	{
		RE_Reflect_Select( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール情報受信反映
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Reflect_Rail( DEBUG_RAIL_EDITOR* p_wk )
{
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	FIELD_RAIL_LOADER* p_railload = FIELDMAP_GetFieldRailLoader( p_wk->p_fieldmap );
	RAIL_LOCATION location;
	RE_MCS_RAIL_DATA* p_data = (RE_MCS_RAIL_DATA*)p_wk->p_recv->rail;	

	// ロケーション取得
	FIELD_RAIL_MAN_GetLocation( p_rail, &location );

	// レール情報ローダーに設定
	FIELD_RAIL_LOADER_Clear( p_railload );
	FIELD_RAIL_LOADER_DEBUG_LoadBinary( p_railload, p_data->rail, p_wk->p_recv->rail_size  - 8 );

	// レールマネージャに登録
	FIELD_RAIL_MAN_Load( p_rail,  FIELD_RAIL_LOADER_GetData( p_railload ) );

	// ロケーション設定
	FIELD_RAIL_MAN_SetLocation( p_rail, &location );
}

//----------------------------------------------------------------------------
/**
 *	@brief	エリア情報受信反映
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Reflect_Area( DEBUG_RAIL_EDITOR* p_wk )
{
	FLD_SCENEAREA* p_area = FIELDMAP_GetFldSceneArea( p_wk->p_fieldmap );
	FLD_SCENEAREA_LOADER* p_areaload = FIELDMAP_GetFldSceneAreaLoader( p_wk->p_fieldmap );
	RE_MCS_AREA_DATA* p_data = (RE_MCS_AREA_DATA*)p_wk->p_recv->area;	

	// areaローダーに設定
	FLD_SCENEAREA_LOADER_Clear( p_areaload );
	FLD_SCENEAREA_LOADER_LoadBinary( p_areaload, p_data->area, p_wk->p_recv->area_size - 8 );

	// areaマネージャに登録
	FLD_SCENEAREA_Release( p_area );
	FLD_SCENEAREA_Load( p_area, 
			FLD_SCENEAREA_LOADER_GetData( p_areaload ),
			FLD_SCENEAREA_LOADER_GetDataNum( p_areaload ),
			FLD_SCENEAREA_LOADER_GetFunc( p_areaload ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	選択情報の反映
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void RE_Reflect_Select( DEBUG_RAIL_EDITOR* p_wk )
{
	static void (*const cpJump[RE_SELECT_DATA_MAX])( DEBUG_RAIL_EDITOR* p_wk ) = 
	{
		RE_JumpPoint,
		RE_JumpLine,
		RE_JumpArea,
	};

	GF_ASSERT( p_wk->p_recv->select.select_data < RE_SELECT_DATA_MAX );
	if( cpJump[p_wk->p_recv->select.select_data] )
	{
		cpJump[p_wk->p_recv->select.select_data]( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポイントの場所にジャンプ
 */
//-----------------------------------------------------------------------------
static void RE_JumpPoint( DEBUG_RAIL_EDITOR* p_wk )
{
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	RAIL_LOCATION location;
	VecFx32 pos;

	location.type					= FIELD_RAIL_TYPE_POINT;
	location.rail_index		= p_wk->p_recv->select.select_index;
	location.line_ofs			= 0;
	location.width_ofs		= 0;
	location.key					= 0;
	FIELD_RAIL_MAN_SetLocation( p_rail, &location );

	// 位置を人物に設定
	FIELD_RAIL_MAN_GetPos( p_rail, &pos );
	// 
	FIELD_PLAYER_SetPos( FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap ), &pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインの開始位置場所にジャンプ
 */
//-----------------------------------------------------------------------------
static void RE_JumpLine( DEBUG_RAIL_EDITOR* p_wk )
{
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	RAIL_LOCATION location;
	VecFx32 pos;

	location.type					= FIELD_RAIL_TYPE_LINE;
	location.rail_index		= p_wk->p_recv->select.select_index;
	location.line_ofs			= 0;
	location.width_ofs		= 0;
	location.key					= 0;
	FIELD_RAIL_MAN_SetLocation( p_rail, &location );

	// 位置を人物に設定
	FIELD_RAIL_MAN_GetPos( p_rail, &pos );
	// 
	FIELD_PLAYER_SetPos( FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap ), &pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief	areaに近いポイントにジャンプ
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_JumpArea( DEBUG_RAIL_EDITOR* p_wk )
{
	// ポイントデータをなめて、area位置に近い場所に飛ぶ
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	FLD_SCENEAREA_LOADER* p_arealoader = FIELDMAP_GetFldSceneAreaLoader( p_wk->p_fieldmap );

	TOMOYA_Printf( "RE_JumpArea 未実装\n" );
}



//----------------------------------------------------------------------------
/**
 *	@brief	選択情報による、入力コントロール
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_InputControl( DEBUG_RAIL_EDITOR* p_wk )
{
	static void (*const cpInput[RE_SELECT_DATA_SEQ_TYPE_MAX])( DEBUG_RAIL_EDITOR* p_wk ) = 
	{
		NULL,
		RE_InputPoint_FreeNormal,
		RE_InputPoint_FreeCircle,
		RE_InputPoint_Rail,
		RE_InputLine_FreeNormal,
		RE_InputLine_FreeCircle,
		RE_InputLine_Rail,
		RE_InputArea_FreeNormal,
		RE_InputArea_FreeCircle,
		RE_InputArea_Rail,
		RE_InputCamera_FreeNormal,
		RE_InputCamera_FreeCircle,
		RE_InputCamera_Pos,
		RE_InputCamera_Angle,
		RE_InputCamera_Target,
		RE_InputCamera_Rail,
		RE_InputLinePos_FreeNormal,
		RE_InputLinePos_FreeCircle,
		RE_InputLinePos_CenterPos,
		RE_InputLinePos_Rail,
	};
	
	// 選択状態による、操作
	GF_ASSERT( p_wk->p_recv->select.select_seq < RE_SELECT_DATA_SEQ_TYPE_MAX);	
	if( cpInput[p_wk->p_recv->select.select_seq] )
	{
		cpInput[p_wk->p_recv->select.select_seq]( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポイント	自由移動
 */
//-----------------------------------------------------------------------------
static void RE_InputPoint_FreeNormal( DEBUG_RAIL_EDITOR* p_wk )
{
	// フリー自機動作
	RE_FreeMove_Normal( p_wk );
}
static void RE_InputPoint_FreeCircle( DEBUG_RAIL_EDITOR* p_wk )
{
	// フリー自機動作
	RE_FreeMove_Circle( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポイント	レール移動
 */
//-----------------------------------------------------------------------------
static void RE_InputPoint_Rail( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_RailMove( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインノーマル移動
 */
//-----------------------------------------------------------------------------
static void RE_InputLine_FreeNormal( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Normal( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ラインサークル移動
 */
//-----------------------------------------------------------------------------
static void RE_InputLine_FreeCircle( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Circle( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインレール移動
 */
//-----------------------------------------------------------------------------
static void RE_InputLine_Rail( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_RailMove( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	エリア入力　フリー動作
 */
//-----------------------------------------------------------------------------
static void RE_InputArea_FreeNormal( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Normal( p_wk );
}
static void RE_InputArea_FreeCircle( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Circle( p_wk );
}
static void RE_InputArea_Rail( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_RailMove( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ入力
 */
//-----------------------------------------------------------------------------
static void RE_InputCamera_FreeNormal( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Normal( p_wk );
}
static void RE_InputCamera_FreeCircle( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Circle( p_wk );
}
static void RE_InputCamera_Pos( DEBUG_RAIL_EDITOR* p_wk )
{
}
static void RE_InputCamera_Angle( DEBUG_RAIL_EDITOR* p_wk )
{
}
static void RE_InputCamera_Target( DEBUG_RAIL_EDITOR* p_wk )
{
}
static void RE_InputCamera_Rail( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_RailMove( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライン位置入力
 */
//-----------------------------------------------------------------------------
static void RE_InputLinePos_FreeNormal( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Normal( p_wk );
}

static void RE_InputLinePos_FreeCircle( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_FreeMove_Circle( p_wk );
}

static void RE_InputLinePos_CenterPos( DEBUG_RAIL_EDITOR* p_wk )
{
}

static void RE_InputLinePos_Rail( DEBUG_RAIL_EDITOR* p_wk )
{
	RE_RailMove( p_wk );
}



//----------------------------------------------------------------------------
/**
 *	@brief	情報送信管理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_SendControl( DEBUG_RAIL_EDITOR* p_wk )
{
	if( p_wk->p_recv->rail_req )
	{
		// レール情報の送信
		RE_Send_RailData( p_wk );
	}
	else if( p_wk->p_recv->area_req )
	{
		// area情報の送信
		RE_Send_AreaData( p_wk );
	}
	else if( p_wk->p_recv->player_req )
	{
		// player情報の送信
		RE_Send_PlayerData( p_wk );
	}
	else if( p_wk->p_recv->camera_req )
	{
		// camera情報の送信
		RE_Send_CameraData( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール情報を送信
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Send_RailData( DEBUG_RAIL_EDITOR* p_wk )
{
	const FIELD_RAIL_LOADER* cp_loader = FIELDMAP_GetFieldRailLoader( p_wk->p_fieldmap );
	const void* cp_data = FIELD_RAIL_LOADER_DEBUG_GetData( cp_loader );	
	u32 datasize = FIELD_RAIL_LOADER_DEBUG_GetDataSize( cp_loader );	
	RE_MCS_RAIL_DATA* p_senddata = (RE_MCS_RAIL_DATA*)p_wk->p_tmp_buff;
	BOOL result;

	p_senddata->header.data_type = RE_MCS_DATA_RAIL;
	p_senddata->reset						 = FALSE;
	GFL_STD_MemCopy( cp_data, p_senddata->rail, datasize );
	
	// 送信
	result = MCS_Write( MCS_CHANNEL0, p_senddata, datasize+4+sizeof(RE_MCS_HEADER) );
	GF_ASSERT( result );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	area情報を送信
 *	
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Send_AreaData( DEBUG_RAIL_EDITOR* p_wk )
{
	const FLD_SCENEAREA_LOADER* cp_loader = FIELDMAP_GetFldSceneAreaLoader( p_wk->p_fieldmap );
	const void* cp_data = FLD_SCENEAREA_LOADER_DEBUG_GetData( cp_loader );
	u32 datasize				= FLD_SCENEAREA_LOADER_DEBUG_GetDataSize( cp_loader );
	RE_MCS_AREA_DATA* p_senddata = (RE_MCS_AREA_DATA*)p_wk->p_tmp_buff;
	BOOL result;

	p_senddata->header.data_type = RE_MCS_DATA_AREA;
	p_senddata->reset						 = FALSE;
	GFL_STD_MemCopy( cp_data, p_senddata->area, datasize );
	
	
	// 送信
	result = MCS_Write( MCS_CHANNEL0, p_senddata, datasize+4+sizeof(RE_MCS_HEADER) );
	GF_ASSERT( result );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤー情報を送信
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Send_PlayerData( DEBUG_RAIL_EDITOR* p_wk )
{
	const FIELD_PLAYER* cp_player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
	RE_MCS_PLAYER_DATA* p_senddata = (RE_MCS_PLAYER_DATA*)p_wk->p_tmp_buff;
	VecFx32 pos;
	BOOL result;
	
	p_senddata->header.data_type = RE_MCS_DATA_PLAYERDATA;

	FIELD_PLAYER_GetPos( cp_player, &pos );
	p_senddata->pos_x = pos.x;
	p_senddata->pos_y = pos.y;
	p_senddata->pos_z = pos.z;

	// 送信
	result = MCS_Write( MCS_CHANNEL0, p_senddata, sizeof(RE_MCS_PLAYER_DATA) );
	GF_ASSERT( result );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ情報を送信
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void RE_Send_CameraData( DEBUG_RAIL_EDITOR* p_wk )
{
	const FIELD_CAMERA* cp_camera = FIELDMAP_GetFieldCamera( p_wk->p_fieldmap );
	RE_MCS_CAMERA_DATA* p_senddata = (RE_MCS_CAMERA_DATA*)p_wk->p_tmp_buff;
	VecFx32 pos;
	VecFx32 target;
	BOOL result;

	p_senddata->header.data_type = RE_MCS_DATA_CAMERADATA;

	FIELD_CAMERA_GetTargetPos( cp_camera, &target );
	FIELD_CAMERA_GetCameraPos( cp_camera, &pos );
	p_senddata->pos_x = pos.x;
	p_senddata->pos_y = pos.y;
	p_senddata->pos_z = pos.z;
	p_senddata->target_x = target.x;
	p_senddata->target_y = target.y;
	p_senddata->target_z = target.z;
	p_senddata->pitch = FIELD_CAMERA_GetAnglePitch( cp_camera );
	p_senddata->yaw		= FIELD_CAMERA_GetAngleYaw( cp_camera );
	p_senddata->len		= FIELD_CAMERA_GetAngleLen( cp_camera );

	// 送信
	result = MCS_Write( MCS_CHANNEL0, p_senddata, sizeof(RE_MCS_CAMERA_DATA) );
	GF_ASSERT( result );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	主人公　通常移動
 */
//-----------------------------------------------------------------------------
static void RE_FreeMove_Normal( DEBUG_RAIL_EDITOR* p_wk )
{
	FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_wk->p_fieldmap );
	FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	FLD_SCENEAREA* p_scenearea = FIELDMAP_GetFldSceneArea( p_wk->p_fieldmap );

	FIELD_CAMERA_SetMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

	FIELD_RAIL_MAN_SetActiveFlag(p_rail, FALSE);
	FLD_SCENEAREA_SetActiveFlag(p_scenearea, FALSE);
	
	FIELD_PLAYER_NOGRID_Move( p_player, GFL_UI_KEY_GetCont() );

	FIELD_PLAYER_GetPos( p_player, &p_wk->camera_target );
	FIELD_CAMERA_BindTarget( p_camera, &p_wk->camera_target );
}

//----------------------------------------------------------------------------
/**
 *	@brief	主人公　円移動
 */
//-----------------------------------------------------------------------------
static void RE_FreeMove_Circle( DEBUG_RAIL_EDITOR* p_wk )
{
	FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_wk->p_fieldmap );
	FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	FLD_SCENEAREA* p_scenearea = FIELDMAP_GetFldSceneArea( p_wk->p_fieldmap );
	VecFx32 target, pl_pos;
	u16 yaw;
	fx32 len;
	u32 key_cont = GFL_UI_KEY_GetCont();
	s16 df_angle;
	s16 df_len;

	FIELD_CAMERA_SetMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

	FIELD_RAIL_MAN_SetActiveFlag(p_rail, FALSE);
	FLD_SCENEAREA_SetActiveFlag(p_scenearea, FALSE);

	FIELD_CAMERA_FreeTarget( p_camera );

	if( key_cont & PAD_BUTTON_B )
	{
		df_len		= 4*RM_DEF_LEN_MOVE;
		df_angle	= 4*RM_DEF_ANGLE_MOVE;
	}
	else
	{
		df_len		= RM_DEF_LEN_MOVE;
		df_angle	= RM_DEF_ANGLE_MOVE;
	}

	// カメラターゲットを中心に回転移動
	FIELD_CAMERA_GetTargetPos( p_camera, &target );
	FIELD_PLAYER_GetPos( p_player, &pl_pos );
	target.y	= pl_pos.y;
	len				= VEC_Distance( &target, &pl_pos );
	yaw				= FX_Atan2Idx( target.x, target.z );

	if( key_cont & PAD_KEY_LEFT )
	{
		yaw += df_angle;
	}
	else if( key_cont & PAD_KEY_RIGHT )
	{
		yaw -= df_angle;
	}
	else if( key_cont & PAD_KEY_UP )
	{
		len -= df_len;
	}
	else if( key_cont & PAD_KEY_DOWN )
	{
		len += df_len;
	}

	// 角度から、座標を求める
	pl_pos.x = FX_Mul( FX_SinIdx( yaw ), len );
	pl_pos.z = FX_Mul( FX_CosIdx( yaw ), len );
	FIELD_PLAYER_SetPos( p_player, &pl_pos );
	
	// 延長線上にカメラを配置
	FIELD_CAMERA_SetAnglePitch( p_camera, RM_DEF_CAMERA_PITCH );
	FIELD_CAMERA_SetAngleYaw( p_camera, yaw );
	FIELD_CAMERA_SetAngleLen( p_camera, RM_DEF_CAMERA_LEN );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール動作
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void RE_RailMove( DEBUG_RAIL_EDITOR* p_wk )
{
	FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
	FIELD_RAIL_MAN* p_rail = FIELDMAP_GetFieldRailMan( p_wk->p_fieldmap );
	FLD_SCENEAREA* p_scenearea = FIELDMAP_GetFldSceneArea( p_wk->p_fieldmap );
	VecFx32 pos;

	// レール移動
	FIELD_RAIL_MAN_SetActiveFlag(p_rail, FALSE);
	FLD_SCENEAREA_SetActiveFlag(p_scenearea, FALSE);
	
  FIELD_RAIL_MAN_GetPos(p_rail, &pos );
  FIELD_PLAYER_SetPos( p_player, &pos );

	FIELD_PLAYER_GetPos( p_player, &p_wk->camera_target );
//	FIELD_CAMERA_BindTarget( p_camera, &p_wk->camera_target );
}


#endif // PM_DEBUG
