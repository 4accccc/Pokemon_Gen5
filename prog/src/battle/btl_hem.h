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

#define BTL_HEM_OBO (1)


extern void BTL_Hem_Init( HANDLER_EXHIBISION_MANAGER* wk );
extern u32 BTL_Hem_PushState_Impl( HANDLER_EXHIBISION_MANAGER* wk, u32 line );
extern u32 BTL_Hem_PushStateUseItem_Impl( HANDLER_EXHIBISION_MANAGER* wk, u16 itemNo, u32 line );
extern void BTL_Hem_PopState_Impl( HANDLER_EXHIBISION_MANAGER* wk, u32 state, u32 line );
extern BTL_HANDEX_PARAM_HEADER* BTL_Hem_ReadWork( HANDLER_EXHIBISION_MANAGER* wk );
extern BOOL BTL_Hem_IsPushed( const HANDLER_EXHIBISION_MANAGER* wk );
extern BTL_HANDEX_PARAM_HEADER* BTL_Hem_PushWork( HANDLER_EXHIBISION_MANAGER* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID );
extern void BTL_Hem_PopWork( HANDLER_EXHIBISION_MANAGER* wk, void* exWork );



extern BOOL BTL_Hem_IsExistWork( const HANDLER_EXHIBISION_MANAGER* wk );
extern u16 BTL_Hem_GetUseItemNo( const HANDLER_EXHIBISION_MANAGER* wk );
extern void BTL_Hem_SetResult( HANDLER_EXHIBISION_MANAGER* wk, BOOL fSucceed  );
extern BOOL BTL_Hem_GetPrevResult( const HANDLER_EXHIBISION_MANAGER* wk  );
extern BOOL BTL_Hem_GetTotalResult( const HANDLER_EXHIBISION_MANAGER* wk );
extern BOOL BTL_Hem_IsPushed( const HANDLER_EXHIBISION_MANAGER* wk );
extern BOOL BTL_Hem_IsUsed( const HANDLER_EXHIBISION_MANAGER* wk );
extern u16 BTL_Hem_GetUseItemNo( const HANDLER_EXHIBISION_MANAGER* wk );


#define BTL_Hem_PushState(wk) BTL_Hem_PushState_Impl((wk), __LINE__ )
#define BTL_Hem_PushStateUseItem(wk, itemNo) BTL_Hem_PushStateUseItem_Impl((wk), (itemNo), __LINE__ )
#define BTL_Hem_PopState(wk, state)  BTL_Hem_PopState_Impl((wk), (state), __LINE__ )
