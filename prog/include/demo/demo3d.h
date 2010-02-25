//============================================================================
/**
 *
 *	@file		demo3d.h
 *	@brief  3Dデモ再生アプリ
 *	@author	genya hosaka -> miyuki iwasawa
 *	@data		2009.11.27
 *
 */
//============================================================================
#pragma once

//	lib
#include <gflib.h>

//system
#include "gamesystem/gamesystem.h"

#include "demo/demo3d_demoid.h" // DEMO3D_ID(コンバータから生成)

// typedef宣言
typedef u32 DEMO3D_ID;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern const GFL_PROC_DATA Demo3DProcData;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

typedef enum
{ 
  DEMO3D_RESULT_NULL = 0,
  DEMO3D_RESULT_USER_END, ///< ユーザーによる終了
  DEMO3D_RESULT_FINISH,   ///< デモの終了
} DEMO3D_RESULT;

//-------------------------------------
///	PROCに渡す引数
//=====================================
typedef struct {	
  // [IN]
  DEMO3D_ID     demo_id;      ///< デモID
  u32           start_frame;  ///< デモ開始フレーム(1sync=1)

  u8            hour;         ///<呼び出し時間(時)
  u8            min;          ///<呼び出し時間(分)
  u8            season;       ///<季節

  // [OUT]
  u32           end_frame;    ///< デモ終了フレーム(1sync=1)
  DEMO3D_RESULT result;       ///< デモ終了タイプ

} DEMO3D_PARAM;

FS_EXTERN_OVERLAY(demo3d);

/*
 *  @brief  demo3d呼び出しパラメータセット(常駐関数です)
 *
 *  @param  pp      DEMO3D_PARAM型構造体へのポインタ
 *  @param  demo_id 呼び出すデモID DEMO3D_ID_NULL～ include/demo/demo3d_demoid.h 
 *  @param  s_frame デモのスタートフレーム指定
 *  @param  t_hour  デモのライト設定に使う時間指定(時)
 *  @param  t_min   デモのライト設定に使う時間指定(分)
 *  @param  season  デモのライト設定に使う季節 PMSEASON_SPRING～ include/gamesystem/pm_season.h
 */
extern void DEMO3D_PARAM_Set( DEMO3D_PARAM* pp, u8 demo_id, u32 s_frame, u8 t_hour, u8 t_min, u8 season );

/*
 *  @brief  demo3d呼び出しパラメータセット(RTC依存)
 *
 *  @param  pp      DEMO3D_PARAM型構造体へのポインタ
 *  @param  demo_id 呼び出すデモID DEMO3D_ID_NULL～ include/demo/demo3d_demoid.h 
 *  @param  s_frame デモのスタートフレーム指定
 *
 *  デモのライト設定に使う時間・季節をリアルタイムクロックから算出します
 */
extern void DEMO3D_PARAM_SetFromRTC( DEMO3D_PARAM* pp, u8 demo_id, u32 s_frame );

/*
 *  @brief  demo3d呼び出しパラメータセット(EV-TIME依存)
 *
 *  @param  pp      DEMO3D_PARAM型構造体へのポインタ
 *  @param  demo_id 呼び出すデモID DEMO3D_ID_NULL～ include/demo/demo3d_demoid.h 
 *  @param  s_frame デモのスタートフレーム指定
 *  @param  gdata   GAMEDATAの参照ポインタ
 *
 *  デモのライト設定に使う時間・季節をEV-TIMEから算出します
 */
extern void DEMO3D_PARAM_SetFromEvTime( DEMO3D_PARAM* pp, u8 demo_id, u32 s_frame, const GAMEDATA* gdata );

