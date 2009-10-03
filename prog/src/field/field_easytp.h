//======================================================================
/**
 *
 * @file	field_easytp.h
 * @brief	フィールド簡易タッチパネル
 * @author	kagaya
 * @date	08.09.30
 */
//======================================================================
#ifndef __FIELD_EASYTP_H__
#define __FIELD_EASYTP_H__

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	FLDEASYTP_TCHDIR	押された方向を表す
//--------------------------------------------------------------
typedef enum
{
	FLDEASYTP_TCHDIR_UP = 0,	//上
	FLDEASYTP_TCHDIR_DOWN,		//下 
	FLDEASYTP_TCHDIR_LEFT,		//左
	FLDEASYTP_TCHDIR_RIGHT,		//右
	FLDEASYTP_TCHDIR_LEFTUP,	//左上
	FLDEASYTP_TCHDIR_LEFTDOWN,	//左下
	FLDEASYTP_TCHDIR_RIGHTUP,	//右上
	FLDEASYTP_TCHDIR_RIGHTDOWN,	//右下
	FLDEASYTP_TCHDIR_CENTER,	//中心
	FLDEASYTP_TCHDIR_NON,		//押されていない
	
	FLDEASYTP_TCHDIR_MAX = FLDEASYTP_TCHDIR_NON,	//最大
}FLDEASYTP_TCHDIR;

//======================================================================
//	typedef struct
//======================================================================
//typedef struct _FLD_EASYTP FLD_EASYTP;

//======================================================================
//	extern
//======================================================================
extern FLDEASYTP_TCHDIR FieldEasyTP_TouchDirGet( void );

#endif //__FIELD_EASYTP_H__
