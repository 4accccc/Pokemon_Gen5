////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_place_name.c
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @data   
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "field_place_name.h"


//===================================================================================
/**
 * システム・ワーク
 */
//===================================================================================
struct _FIELD_PLACE_NAME
{
	int temp;
};


FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( HEAPID heap_id )
{
	FIELD_PLACE_NAME* p_sys;

	// システム・ワーク作成
	p_sys = (FIELD_PLACE_NAME*)GFL_HEAP_AllocClearMemory( heap_id, sizeof( FIELD_PLACE_NAME ) );

	// 作成したシステムを返す
	return p_sys;
}

void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* p_sys ){}
void FIELD_PLACE_NAME_Draw( FIELD_PLACE_NAME* p_sys ){}
void FIELD_PLACE_NAME_Destroy( FIELD_PLACE_NAME* p_sys )
{
	// システム・ワーク解放
	GFL_HEAP_FreeMemory( p_sys );
}
