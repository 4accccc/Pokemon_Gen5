//==============================================================================
/**
 * @file    intrude_monolith.h
 * @brief   モノリスのヘッダ
 * @author  matsuda
 * @date    2009.10.29(木)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void MONOLITH_AddMMdl(FIELDMAP_WORK *fieldWork, u16 grid_x, u16 grid_z, fx32 fx_y);
extern void MONOLITH_AddConnectAllMap(FIELDMAP_WORK *fieldWork);
