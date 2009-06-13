//======================================================================
/**
 * @file  field_hit_check.h
 * @date  2009.06.13
 * @author  tamada GAME FREAK inc.
 */
//======================================================================
#pragma once

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  fx32 top, bottom;
  fx32 left, right;
}FLDHIT_RECT;

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  VecFx32 pos;
  fx32 x_len, y_len, z_len;
}FLDHIT_BOX;



//--------------------------------------------------------------
//--------------------------------------------------------------
static inline BOOL FLDHIT_RECT_IsIntersectRect(const FLDHIT_RECT * rect0, const FLDHIT_RECT * rect1)
{
  if( (rect0->left <= rect1->right) && (rect1->left <= rect0->right)
      && (rect0->top <= rect1->bottom) && (rect1->top <= rect0->bottom)) {
		return TRUE;
	}else{
		return FALSE;
	}
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static inline BOOL FLDHIT_RECT_IsIncludePos(const FLDHIT_RECT * rect, fx32 x, fx32 y)
{
	if( rect->left <= x && rect->right >= x && rect->top <= y && rect->bottom >= y ){
		return( TRUE );
	}
	return( FALSE );
}

