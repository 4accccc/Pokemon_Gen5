//==============================================================================
/**
 * @file	save_control.h
 * @brief	セーブデータ管理ヘッダ
 * @author	matsuda
 * @date	2008.08.28(木)
 */
//==============================================================================
#ifndef __SAVE_CONTROL_H__
#define __SAVE_CONTROL_H__

#include <backup_system.h>


//==============================================================================
//	定数定義
//==============================================================================
//--------------------------------------------------------------
//	first_statusの結果BIT
//--------------------------------------------------------------
#define NORMAL_NG_BIT			(1<<0)		//通常セーブ：ミラーリングNG
#define NORMAL_BREAK_BIT		(1<<1)		//通常セーブ：破壊
#define FRONTIER_NG_BIT			(1<<2)		//外部フロンティアセーブ：ミラーリングNG
#define FRONTIER_BREAK_BIT		(1<<3)		//外部フロンティアセーブ：破壊
#define VIDEO_NG_BIT			(1<<4)		//外部ビデオセーブ：ミラーリングNG
#define VIDEO_BREAK_BIT			(1<<5)		//外部ビデオセーブ：破壊

//==============================================================================
//	型定義
//==============================================================================
///セーブデータ管理ワーク型の不定形型
typedef struct _SAVE_CONTROL_WORK SAVE_CONTROL_WORK;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern SAVE_CONTROL_WORK * SaveControl_SystemInit(int heap_id);
extern SAVE_CONTROL_WORK * SaveControl_GetPointer(void);
extern LOAD_RESULT SaveControl_Load(SAVE_CONTROL_WORK *ctrl);
extern SAVE_RESULT SaveControl_Save(SAVE_CONTROL_WORK *ctrl);
extern void * SaveControl_DataPtrGet(SAVE_CONTROL_WORK *ctrl, GFL_SVDT_ID gmdata_id);



#endif	//__SAVE_CONTROL_H__

