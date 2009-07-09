//=============================================================================================
/**
 * @file  hand_field.h
 * @brief ポケモンWB バトルシステム イベントファクター [フィールドエフェクト]
 * @author  taya
 *
 * @date  2009.07.03  作成
 */
//=============================================================================================
#pragma once

#include "..\btl_server_flow.h"
#include "..\btl_field.h"

extern BTL_EVENT_FACTOR*  BTL_HANDLER_FLD_Add( BtlFieldEffect effect, u8 sub_param );
extern void BTL_HANDLER_FLD_Remove( BTL_EVENT_FACTOR* factor );
extern BOOL BTL_HANDLER_FLD_IsExist( BtlFieldEffect effect );
extern BOOL BTL_HANDLER_FLD_GetSubParam( BtlFieldEffect effect, u8* subParam );

