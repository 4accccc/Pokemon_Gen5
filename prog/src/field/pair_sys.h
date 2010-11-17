//====================================================================
/*
 *  @file pair_sys.h
 *  @brief  連れ歩きシステム関連
 *  @author Miyuki Iwasawa
 *  @date   10.11.16
 */
//====================================================================

#ifndef __H_PAIR_SYS_H__
#define __H_PAIR_SYS_H__

#include "pair_sys_def.h"

//---------------------------------------------------
/*
 *  @brief  連れ歩きフラグチェック
 */
//---------------------------------------------------
extern BOOL PAIRSYS_CheckPairFlag( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きリセット
 */
//---------------------------------------------------
extern void PAIRSYS_PairFree( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペアID取得
 *
 *  @retval 0～ 連れ歩きペアID (定義はfield/pair_sys_def.h)
 *  @retval PAID_ID_NONE(0xFFFF)  連れ歩き無し
 */
//---------------------------------------------------
extern u16 PAIRSYS_GetPairID( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア トレーナーID取得
 *
 *  @retval 1～ 連れ歩きペアのトレーナーID (定義はtr_tool/trno_def.h)
 *  @retval TRID_NULL  連れ歩きトレーナーデータ無し
 */
//---------------------------------------------------
extern u16 PAIRSYS_GetPairTrainerID( GAMEDATA* gdata );



#endif  //__H_PAIR_SYS_H__
