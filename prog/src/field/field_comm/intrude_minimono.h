//==============================================================================
/**
 * @file    intrude_minimono.h
 * @brief   ミニモノリス処理のヘッダ
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void MINIMONO_AddMMdl(FIELDMAP_WORK *fieldWork, u16 grid_x, u16 grid_z, fx32 fx_y);
extern void MINIMONO_AddPosRand(GAME_COMM_SYS_PTR game_comm, FIELDMAP_WORK *fieldWork);

//--------------------------------------------------------------
//  デバッグ
//--------------------------------------------------------------
#ifdef PM_DEBUG
extern void DEBUG_INTRUDE_Pokemon_Add(FIELDMAP_WORK *fieldWork);
extern void DEBUG_INTRUDE_BingoPokeSet(FIELDMAP_WORK *fieldWork);
#endif
