//============================================================================
/**
 *
 *	@file		intro_g3d.h
 *	@brief
 *	@author	hosaka genya
 *	@data		2009.12.20
 *
 */
//============================================================================
#pragma once

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
#define	INTRO_3D_SEL_SEX_DEF_FRAME					( 60 )		// 性別選択のデフォルトフレーム
#define	INTRO_3D_SEL_SEX_ENTER_MALE_FRAME		( 0 )			// 性別選択の男の子決定時のフレーム
#define	INTRO_3D_SEL_SEX_ENTER_FEMALE_FRAME	( 120 )		// 性別選択の女の子決定時のフレーム
#define	INTRO_3D_SEL_SEX_MOVE_MALE_FRAME		( 50 )		// 性別選択の男の子選択時のフレーム
#define	INTRO_3D_SEL_SEX_MOVE_FEMALE_FRAME	( 70 )		// 性別選択の女の子選択時のフレーム

// 性別選択フレーム動作モード
enum {
	INTRO_3D_SEL_SEX_MODE_MOVE_DEFAULT = 0,
	INTRO_3D_SEL_SEX_MODE_MOVE_MALE,
	INTRO_3D_SEL_SEX_MODE_MOVE_FEMALE,
	INTRO_3D_SEL_SEX_MODE_ENTER_MALE,
	INTRO_3D_SEL_SEX_MODE_ENTER_FEMALE,
};


//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================

typedef struct _INTRO_G3D_WORK INTRO_G3D_WORK;

extern INTRO_G3D_WORK* INTRO_G3D_Create( INTRO_GRAPHIC_WORK* graphic , HEAPID heap_id );
extern void INTRO_G3D_Exit( INTRO_G3D_WORK* wk );
extern void INTRO_G3D_Main( INTRO_G3D_WORK* wk );

extern BOOL INTRO_G3D_SelectStart( INTRO_G3D_WORK* wk );

extern void INTRO_G3D_SelectDecideStart( INTRO_G3D_WORK* wk, u32 is_mode );
extern BOOL INTRO_G3D_SelectDecideWait( INTRO_G3D_WORK* wk );

extern void INTRO_G3D_SelectVisible( INTRO_G3D_WORK* wk, BOOL is_visible );

extern void INTRO_G3D_SelectSet( INTRO_G3D_WORK* wk, u32 in_frame );
extern BOOL INTRO_G3D_SelectDecideReturn( INTRO_G3D_WORK* wk );

