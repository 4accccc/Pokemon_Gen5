//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		ctvt_game_cam.h
 *	@brief  ゲームに使うカメラ画像撮影
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.01.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "comm_tvt_local_def.h"
#include "ctvt_game_cam_graphic.h"  // グラフィック設定

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

#define OUTPUT_WIDTH        (64)    // DSPで変換後の幅
#define OUTPUT_HEIGHT       (64)    // DSPで変換後の高さ
#define CAPTURE_SIZE    (OUTPUT_WIDTH * OUTPUT_HEIGHT * 2)  // 64*64*u16

#define CAPTURE_NUM         (3)     // 写真撮る枚数
#define CAPTURE_TRIMMING_W  (128)   // トリミング幅
#define CAPTURE_TRIMMING_H  (128)   // トリミング高さ
#define TRIMMING_SIZE   (CAPTURE_TRIMMING_W * CAPTURE_TRIMMING_H * 2) // 128x128xu16

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

// 不完全型
typedef struct _CTVT_GAME_CAM_WORK CTVT_GAME_CAM_WORK;

//-------------------------------------
///	メンバー情報 1人の。
//=====================================
typedef struct {
  BOOL      isTwl;
  u8        netID;
  void      *p_cap_buf[CAPTURE_NUM];
}CTVT_GAME_CAM_MEMBER_WORK;


//-------------------------------------
///	カメラ撮影用パラメータ
//=====================================
typedef struct {
  BOOL            isTwl;        //[in]    TWL起動だと1
  u8              select_chara; //[in]    NITRO時のみ 選択するキャラクター
  void            *p_cap[CAPTURE_NUM]; //[out] TWL起動時自分のキャプチャデータ
  CTVT_GAME_CAM_MEMBER_WORK member[ CTVT_MEMBER_NUM-1 ]; //[out] メンバー情報
  CAMERA_SYSTEM_WORK        *p_cam_sys;//[in]    カメラシステムへのポインタ
} CTVT_GAME_CAM_PARAM;



//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

CTVT_GAME_CAM_WORK* CTVT_GAME_CAM_Init( CTVT_GAME_CAM_PARAM *p_param, HEAPID heapID );
BOOL CTVT_GAME_CAM_Main( CTVT_GAME_CAM_WORK *p_wk );
void CTVT_GAME_CAM_Exit( CTVT_GAME_CAM_WORK *p_wk );

// パラメータ生成
CTVT_GAME_CAM_PARAM *CTVT_GAME_CAM_AllocParam( HEAPID heapID, BOOL isTwl, u8 select_chara, CTVT_GAME_CAM_MEMBER_WORK p_member[], CAMERA_SYSTEM_WORK *p_cam_sys );

// パラメータ破棄
void CTVT_GAME_CAM_FreeParam( CTVT_GAME_CAM_PARAM *p_param);

