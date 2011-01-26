//======================================================================
/**
 * @file	ctvt_game.c
 * @brief	通信TVTシステム：ミニゲーム画面
 * @author	ichiraku
 * @data	2010/11/25
 *
 * モジュール名：CTVT_GAME
 */

//======================================================================
#pragma once

#include "comm_tvt_local_def.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define

// 下画面
#define CTVT_GAME_FRAME_M_3D      (GFL_BG_FRAME0_M)
#define CTVT_GAME_FRAME_M_BG      (GFL_BG_FRAME1_M)
#define CTVT_GAME_FRAME_M_MSG     (GFL_BG_FRAME2_M)
#define CTVT_GAME_FRAME_M_NONE    (GFL_BG_FRAME3_M)

// 上画面
#define CTVT_GAME_FRAME_S_BG      (GFL_BG_FRAME0_S)
#define CTVT_GAME_FRAME_S_PLATE   (GFL_BG_FRAME1_S)
#define CTVT_GAME_FRAME_S_NONE1   (GFL_BG_FRAME2_S)
#define CTVT_GAME_FRAME_S_NONE2   (GFL_BG_FRAME3_S)

// パレット
#define CTVT_GAME_PAL_BG_M_BG       (0) // 5本
#define CTVT_GAME_PAL_BG_M_WINFRAME (14) // 1本
#define CTVT_GAME_PAL_BG_M_FONT     (15) // 1本

#define CTVT_GAME_PAL_BG_S_BG       (0) // 5本
#define CTVT_GAME_PAL_BG_S_WINFRAME (14) // 1本
#define CTVT_GAME_PAL_BG_S_FONT     (15) // 1本

#define CTVT_GAME_PAL_OBJ_COMMON    (0)  // 
#define CTVT_GAME_PAL_OBJ_COMM      (14) // 通信アイコン用 
#define CTVT_GAME_PAL_OBJ_LOCALIZE  (15) // ローカライズ用

// フレームを読みこんだキャラ位置
#define CTVT_GAME_BMPWIN_CGX_MAIN (1)
#define CTVT_GAME_BMPWIN_CGX (1)

// ミニゲーム用 タイミングコマンド
#define CTVT_COMM_GAME_TITLE_SYNC       (22)    // タイトル同期
#define CTVT_COMM_GAME_BEFORE_SYNC      (23)    // ゲーム開始同期
#define CTVT_COMM_GAME_SCORE_SYNC       (24)    // スコア同期
#define CTVT_COMM_GAME_END_SYNC         (25)    // ゲーム終了同期


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

// ゲーム専用通信フラグ
typedef enum {
  CCGF_NULL,      // 
} CTVT_COMM_GAME_FLAG;

// ゲーム送信データの種類
typedef enum {
  CCGDT_BALOON_APPEAR,  // 風船出現
  CCGDT_RND,        // 乱数シード値
  CCGDT_FRAME,      // 毎フレーム送信
  CCGDT_TP,         // タッチ座標
  CCGDT_BURST,      // 風船割れた
} CTVT_COMM_GAME_DATA_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//-------------------------------------
///	送信データ ゲームデータ
//=====================================
struct _CTVT_GAME_SEND_DATA{

  u32 rnd_seed;     // 乱数シード値

  u8  data_type;    // 送信データの種類

  u8 tpx;           // TP座標
  u8 tpy;           // TP座標

  u8  burst_idx;    // 割れた風船IDX
  u8  burst_netID;  // 割った人がいる場合その人のNetID

  u16 frame;        // 親の経過フレーム数
};

//-------------------------------------
///	送信データ ゲームスコア
//=====================================
struct _CTVT_GAME_SEND_SCORE{
  s32 score[ CTVT_MEMBER_NUM ];
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern CTVT_GAME_WORK* CTVT_GAME_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId );
extern void CTVT_GAME_TermSystem( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );
extern const COMM_TVT_MODE CTVT_GAME_Main( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );
extern void CTVT_GAME_InitMode( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );
extern void CTVT_GAME_TermMode( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );

//----------------------------------------------------------------------------
/**
 *	@brief  風船出現
 *
 *	@param	CTVT_GAME_WORK  *gameWork
 *
 *	@retval TRUE  出現した
 *	@retval FALSE 出現できなかった
 */
//-----------------------------------------------------------------------------
extern BOOL CTVT_GAME_BalloonAppear( CTVT_GAME_WORK *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくる乱数シード値を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        rnd_seed    乱数シード値
 */
//-----------------------------------------------------------------------------
extern void CTVT_GAME_CommRndSeedSet( CTVT_GAME_WORK *p_wk, const u32 rnd_seed );

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくるフレーム経過数を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        frame   受信したフレーム
 */
//-----------------------------------------------------------------------------
extern void CTVT_GAME_CommFrameSet( CTVT_GAME_WORK *p_wk, const u16 frame );

//----------------------------------------------------------------------------
/**
 *	@brief  経過フレーム取得
 *
 *	@param	CTVT_GAME_WORK  *gameWork 
 *
 *	@return u16 経過フレーム
 */
//-----------------------------------------------------------------------------
extern u16 CTVT_GAME_GetFrame( CTVT_GAME_WORK *gameWork );

//----------------------------------------------------------------------------
/**
 *	@brief  親のフレーム取得
 *
 *	@param	CTVT_GAME_WORK  *gameWork 
 *
 *	@return u16 経過フレーム
 */
//-----------------------------------------------------------------------------
extern u16 CTVT_GAME_GetOyaFrame( CTVT_GAME_WORK *gameWork );


//----------------------------------------------------------------------------
/**
 *	@brief  通信からTP座標が送られてきた場合の設定関数
 *	        内部で当たり判定する
 *
 *	@param	commWork    通信構造体
 *	        p_wk        ゲーム管理構造体
 *	        data        設定するゲームデータ
 */
//-----------------------------------------------------------------------------
extern void CTVT_GAME_CommDataSet_TP( CTVT_COMM_WORK *commWork, CTVT_GAME_WORK *p_wk, const CTVT_GAME_SEND_DATA *data, int netID );

//----------------------------------------------------------------------------
/**
 *	@brief  指定したIDXの風船を割る（タスクから消す）
 *
 *	@param	CTVT_GAME_WORK *p_wk
 *	@param	idx   割りたい風船のIDX(受信したものをそのまま指定してOK)
 *	@param  netID 割った人のnetID(受信したものをそのまま指定してOK)
 */
//-----------------------------------------------------------------------------
extern void CTVT_GAME_BalloonDestroy( CTVT_GAME_WORK *p_wk, u16 idx, u8 netId );

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくるゲーム情報を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        score    設定するゲーム情報
 */
//-----------------------------------------------------------------------------
extern void CTVT_GAME_CommScoreSet( CTVT_GAME_WORK *p_wk, const CTVT_GAME_SEND_SCORE *score );
