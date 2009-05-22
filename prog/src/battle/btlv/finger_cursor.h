//==============================================================================
/**
 * @file	finger_cursor.h
 * @brief	戦闘指カーソル
 * @author	matsuda
 * @date	2006.06.13(火)
 */
//==============================================================================
#ifndef __FINGER_CURSOR_H__
#define __FINGER_CURSOR_H__


//==============================================================================
//	型定義
//==============================================================================
///FINGER_WORKの不定形ポインタ
typedef struct _FINGER_WORK * FINGER_PTR;

///FINGER_RES_WORKの不定形ポインタ
typedef struct _FINGER_RES_WORK * FINGER_RES_PTR;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern FINGER_RES_PTR FINGER_ResourceLoad( HEAPID heap, PALETTE_FADE_PTR pfd );
extern void FINGER_ResourceFree( FINGER_RES_PTR frp );
extern FINGER_PTR FINGER_ActorCreate( FINGER_RES_PTR frp, HEAPID heapID, u32 soft_pri, u32 bg_pri);
extern void FINGER_ActorDelete(FINGER_PTR finger);
extern void FINGER_PosSetON_Surface(FINGER_PTR finger, int x, int y, fx32 surface_y);
extern void FINGER_PosSetON(FINGER_PTR finger, int x, int y);
extern void FINGER_OFF(FINGER_PTR finger);
extern void FINGER_TouchReq(FINGER_PTR finger, int wait);
extern BOOL FINGER_TouchAnimeCheck(FINGER_PTR finger);


#endif	//__FINGER_CURSOR_H__

