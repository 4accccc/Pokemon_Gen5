////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_place_name.c
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @date   2009.07.08
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "field_place_name.h"


//===================================================================================
/**
 * 定数
 */
//===================================================================================

//------------------
// 各状態の動作時間
//------------------
#define PROCESS_TIME_APPEAR    (10)	// 表示状態
#define PROCESS_TIME_WAIT      (60) // 待機状態
#define PROCESS_TIME_DISAPPEAR (10) // 退出状態

//-----------------
// システムの状態
//-----------------
typedef enum
{
	FIELD_PLACE_NAME_STATE_HIDE,		// 非表示
	FIELD_PLACE_NAME_STATE_APPEAR,		// 出現中
	FIELD_PLACE_NAME_STATE_WAIT,		// 待機(表示)中
	FIELD_PLACE_NAME_STATE_DISAPPEAR,	// 退出中
	FIELD_PLACE_NAME_STATE_MAX,
}
FIELD_PLACE_NAME_STATE;



//===================================================================================
/**
 * システム・ワーク
 */
//===================================================================================
struct _FIELD_PLACE_NAME
{ 
	FIELD_PLACE_NAME_STATE state;			// 状態
	u16	                   stateCount;		// 状態カウンタ
	u32	                   currentZoneID;	// 現在表示中の地名に対応するゾーンID
	u32                    nextZoneID;		// 次に表示する地名に対応するゾーンID
};


//===================================================================================
/**
 * 非公開関数のプロトタイプ宣言
 */
//===================================================================================
// 状態を変更する
static void SetState( FIELD_PLACE_NAME* p_sys, FIELD_PLACE_NAME_STATE next_state );
// 各状態時の動作
static void Process_HIDE( FIELD_PLACE_NAME* p_sys );
static void Process_APPEAR( FIELD_PLACE_NAME* p_sys );
static void Process_WAIT( FIELD_PLACE_NAME* p_sys );
static void Process_DISAPPEAR( FIELD_PLACE_NAME* p_sys );


//===================================================================================
/**
 * 公開関数の実装( システムの稼動に関する関数 )
 */
//===================================================================================
//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを作成する
 *
 * @param heap_id 使用するヒープID
 *
 * @return 地名表示システム・ワークへのポインタ
 */
//------------------------------------------------------------------------------------
FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( HEAPID heap_id )
{
	FIELD_PLACE_NAME* p_sys;

	// システム・ワーク作成
	p_sys = (FIELD_PLACE_NAME*)GFL_HEAP_AllocClearMemory( heap_id, sizeof( FIELD_PLACE_NAME ) );

	// 作成したシステムを返す
	return p_sys;
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムの動作処理
 *
 * @param p_sys 動かすシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* p_sys )
{
	// 状態カウンタを更新
	p_sys->stateCount++;

	// 状態に応じた動作を行う
	switch( p_sys->state )
	{
		case FIELD_PLACE_NAME_STATE_HIDE:		Process_HIDE( p_sys );		break;
		case FIELD_PLACE_NAME_STATE_APPEAR:		Process_APPEAR( p_sys );	break;
		case FIELD_PLACE_NAME_STATE_WAIT:		Process_WAIT( p_sys );		break;
		case FIELD_PLACE_NAME_STATE_DISAPPEAR:	Process_DISAPPEAR( p_sys );	break;
	}
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示ウィンドウの描画処理
 *
 * @param p_sys 描画対象システム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Draw( FIELD_PLACE_NAME* p_sys )
{
	char* state;
	switch( p_sys->state )
	{
		case FIELD_PLACE_NAME_STATE_HIDE:		state = "HIDE";	break;
		case FIELD_PLACE_NAME_STATE_APPEAR:		state = "APPEAR";	break;
		case FIELD_PLACE_NAME_STATE_WAIT:		state = "WAIT";	break;
		case FIELD_PLACE_NAME_STATE_DISAPPEAR:	state = "DISAPPER";	break;
		default:								state = "error"; break;
	}
	OBATA_Printf( "state = %s,  stateCount = %d\n", state, p_sys->stateCount );
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを破棄する
 *
 * @param p_sys 破棄するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Delete( FIELD_PLACE_NAME* p_sys )
{
	// システム・ワーク解放
	GFL_HEAP_FreeMemory( p_sys );
} 


//===================================================================================
/**
 * 公開関数の実装( システムの制御に関する関数 )
 */
//===================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief ゾーンの切り替えを通知し, 新しい地名を表示する
 *
 * @param p_sys        ゾーン切り替えを通知するシステム
 * @param next_zone_id 新しいゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_ZoneChange( FIELD_PLACE_NAME* p_sys, u32 next_zone_id )
{
	// 指定されたゾーンIDを次に表示すべきものとして記憶
	p_sys->nextZoneID = next_zone_id;

	// 出現中 or 待機中 なら, 強制的に退出させる
	if( ( p_sys->state == FIELD_PLACE_NAME_STATE_APPEAR ) ||
		( p_sys->state == FIELD_PLACE_NAME_STATE_WAIT ) )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_DISAPPEAR );
	}
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウの表示を強制的に終了する
 *
 * @param p_sys 表示を終了するシステム
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Hide( FIELD_PLACE_NAME* p_sys )
{
	SetState( p_sys, FIELD_PLACE_NAME_STATE_HIDE );
}



//===================================================================================
/**
 * 非公開関数の実装
 */
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param p_sys      状態を変更するシステム
 * @param next_state 設定する状態
 */
//-----------------------------------------------------------------------------------
static void SetState( FIELD_PLACE_NAME* p_sys, FIELD_PLACE_NAME_STATE next_state )
{
	// 状態を変更し, 状態カウンタを初期化する
	p_sys->state      = next_state;
	p_sys->stateCount = 0;

	// 出現状態に移行する時は, 表示中のゾーンIDを更新する
	if( next_state == FIELD_PLACE_NAME_STATE_APPEAR )
	{
		p_sys->currentZoneID = p_sys->nextZoneID;
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態( FIELD_PLACE_NAME_STATE_HIDE )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_HIDE( FIELD_PLACE_NAME* p_sys )
{
	// ゾーンの切り替え通知を受けていたら, 出現状態に移行する
	if( p_sys->currentZoneID != p_sys->nextZoneID )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_APPEAR );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 出現状態( FIELD_PLACE_NAME_STATE_APPEAR )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_APPEAR( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 待機状態へ移行する
	if( PROCESS_TIME_APPEAR < p_sys->stateCount )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_WAIT );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機( FIELD_PLACE_NAME_STATE_WAIT )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 退出状態へ移行する
	if( PROCESS_TIME_WAIT < p_sys->stateCount )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_DISAPPEAR );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 退出状態( FIELD_PLACE_NAME_STATE_DISAPPEAR )時の動作
 *
 * @param p_sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_DISAPPEAR( FIELD_PLACE_NAME* p_sys )
{
	// 一定時間が経過したら, 非表示状態へ移行する
	if( PROCESS_TIME_DISAPPEAR < p_sys->stateCount )
	{
		SetState( p_sys, FIELD_PLACE_NAME_STATE_HIDE );
	}
}
