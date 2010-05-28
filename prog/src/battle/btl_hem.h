//=============================================================================================
/**
 * @file  btl_hem.h
 * @brief ポケモンWB バトルシステム サーバコマンド生成ハンドラの反応処理プロセスマネージャ
 * @author  taya
 *
 * @date  2010.05.28  作成
 */
//=============================================================================================

#pragma once

#include "btl_server_flow_local.h"




extern void BTL_Hem_Init( HANDLER_EXHIBISION_MANAGER* wk );
extern u32 BTL_Hem_PushState_Impl( HANDLER_EXHIBISION_MANAGER* wk, u32 line );
extern void BTL_Hem_PopState_Impl( HANDLER_EXHIBISION_MANAGER* wk, u32 state, u32 line );
extern u16 BTL_Hem_GetStackPtr( const HANDLER_EXHIBISION_MANAGER* wk );
extern BTL_HANDEX_PARAM_HEADER* BTL_Hem_ReadWork( HANDLER_EXHIBISION_MANAGER* wk );
extern BTL_HANDEX_PARAM_HEADER* BTL_Hem_PushWork( HANDLER_EXHIBISION_MANAGER* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID );

#define BTL_Hem_PushState(wk) BTL_Hem_PushState_Impl((wk), __LINE__ )
#define BTL_Hem_PopState(wk, state)  BTL_Hem_PopState_Impl((wk), (state), __LINE__ )
