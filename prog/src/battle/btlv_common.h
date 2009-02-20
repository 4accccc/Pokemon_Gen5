//=============================================================================================
/**
 * @file	btlv_common.h
 * @brief	ポケモンWB バトル 描画処理共有ヘッダ
 * @author	taya
 *
 * @date	2008.11.18	作成
 */
//=============================================================================================
#ifndef __BTLV_COMMON_H__
#define __BTLV_COMMON_H__

/**
 *	画面サイズ／レイアウト関連定数
 */
enum {
	BTLV_LCD_HEIGHT = 192,
	BTLV_LCD_WIDTH  = 256,
};



/**
 *	描画処理用タスクプライオリティ
 */
typedef enum {

	BTLV_TASKPRI_MAIN_WINDOW = 0,
	BTLV_TASKPRI_DAMAGE_EFFECT,


}BtlvTaskPri;


#endif
