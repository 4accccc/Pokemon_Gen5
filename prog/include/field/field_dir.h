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

#endif //FIELD_DIR_H_FILE
