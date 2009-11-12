//============================================================================================
/**
 * @file		box_mode.h
 * @brief		ボックス画面の呼び出し/終了モード定義
 * @author	Hiroyuki Nakamura
 * @date		09.11.09
 */
//============================================================================================
#pragma	once

// ボックス画面動作モード
#define BOX_MODE_AZUKERU			( 0 )		///< あずける
#define BOX_MODE_TURETEIKU		( 1 )		///< つれていく
#define BOX_MODE_SEIRI				( 2 )		///< せいり
#define BOX_MODE_ITEM					( 3 )		///< どうぐせいり
#define BOX_MODE_COMPARE			( 4 )		///< つよさくらべ

// ボックス画面終了モード
#define	BOX_END_MODE_MENU			( 0 )		///< 選択メニューへ戻る
#define	BOX_END_MODE_C_GEAR		( 1 )		///< C-gearまで戻る
