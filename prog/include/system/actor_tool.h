//==============================================================================
/**
 * @file	actor_tool.h
 * @brief	アクター操作用ツールのヘッダ
 * @author	matsuda
 * @date	2008.12.25(木)
 */
//==============================================================================
#ifndef __ACTOR_TOOL_H__
#define __ACTOR_TOOL_H__

//#include <obj_graphic_man.h>

//==============================================================================
//	型定義
//==============================================================================
///パレットスロット管理ワークの不定形アクセスポインタ
typedef struct _PLTTSLOT_SYSTEM * PLTTSLOT_SYS_PTR;


//==============================================================================
//	外部関数宣言
//==============================================================================
#if 1
extern PLTTSLOT_SYS_PTR PLTTSLOT_Init(HEAPID heap_id, int main_num, int sub_num);
extern void PLTTSLOT_Exit(PLTTSLOT_SYS_PTR pssp);
extern u32 PLTTSLOT_ResourceSet(PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, 
	CLSYS_DRAW_TYPE draw_type, int pltt_num, int heap_id);
extern u32 PLTTSLOT_ResourceCompSet(PLTTSLOT_SYS_PTR pssp, ARCHANDLE *handle, u32 data_id, 
  CLSYS_DRAW_TYPE draw_type, int pltt_num, int heap_id);
extern u32 PLTTSLOT_GetPalNo(PLTTSLOT_SYS_PTR pssp, u32 index, CLSYS_DRAW_TYPE draw_type);
extern void PLTTSLOT_ResourceFree(PLTTSLOT_SYS_PTR pssp, u32 index, CLSYS_DRAW_TYPE draw_type);
#endif

#endif	//__ACTOR_TOOL_H__
