//======================================================================
/**
 * @file	field_dir.h
 * @brief	フィールドで指定する方向定義 アセンブラソース読み込みアリ enum禁止
 * @author	kagaya
 * @data	05.07.20
 */
//======================================================================
#ifndef FIELD_DIR_H_FILE
#define FIELD_DIR_H_FILE

//======================================================================
//	方向
//======================================================================
#define DIR_UP			(0)					///<上向き
#define DIR_DOWN		(1)					///<下向き
#define DIR_LEFT		(2)					///<左向き
#define DIR_RIGHT		(3)					///<右向き
#define DIR_LEFTUP		(4)					///<左上向き
#define DIR_RIGHTUP		(5)					///<右上向き
#define DIR_LEFTDOWN	(6)					///<左下向き
#define DIR_RIGHTDOWN	(7)					///<右下向き
#define DIR_MAX			(8)					///<向き最大
#define DIR_NOT			(9)					///<向きが無い
#define DIR_MAX4		(DIR_RIGHT+1)		///<四方向最大

// update ichiraku 2010/10/19
//=============================================================================
/**
 *    _OBJ_ANIME_AUTOのパラメータ用
 */
//=============================================================================
// force_typeパラメータ
#define FORCE_TYPE_NOT  (0x0) // 障害物無視しない
#define FORCE_TYPE_NPC  (0x1) // NPCのみ無視する
#define FORCE_TYPE_MAP  (0x2) // マップのみ無視する
#define FORCE_TYPE_ALL  (0x4) // 全て無視する


// first_lineパラメータ
#define DIR_LR    (0)   // 左右（X軸）
#define DIR_UD    (1)   // 上下（Z軸）

#endif //FIELD_DIR_H_FILE
