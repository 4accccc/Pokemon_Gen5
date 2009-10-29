//============================================================================================
/**
 * @file	el_scoreboard.h
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#pragma once

typedef struct _EL_SCOREBOARD				EL_SCOREBOARD;
typedef struct _EL_SCOREBOARD_TEX		EL_SCOREBOARD_TEX;

typedef enum {
	ELB_MODE_NONE = 0,	// スクロール処理なし
	ELB_MODE_S,					// 横スクロール
	ELB_MODE_T,					// 縦スクロール
}ELB_MODE;

//============================================================================================
/**
 *
 * @brief	電光掲示板オブジェクト
 *
 */
//============================================================================================
EL_SCOREBOARD*	ELBOARD_Add( const STRBUF* str, const ELB_MODE mode, HEAPID heapID );
void						ELBOARD_Delete( EL_SCOREBOARD* elb );
void						ELBOARD_Main( EL_SCOREBOARD* elb );
void						ELBOARD_Draw( EL_SCOREBOARD* elb, 
															VecFx32* trans, fx32 scale, u16 width, u16 height, 
															GFL_G3D_CAMERA* g3Dcamera, GFL_G3D_LIGHTSET* g3Dlightset );

//============================================================================================
/**
 *
 * @brief	電光掲示板テクスチャ（既存テクスチャに貼り付けて使用）
 *
 */
//============================================================================================
EL_SCOREBOARD_TEX*	ELBOARD_TEX_Add_Ex(
    const GFL_G3D_RES* g3Dtex, const char* tex_name, const char* plt_name, 
    const STRBUF* str, u16 xpos, u16 ypos, HEAPID heapID );
#define ELBOARD_TEX_Add( g3Dtex, str, heapID ) \
        ELBOARD_TEX_Add_Ex( g3Dtex, "elboard_core", "elboard_core_pl", str, 0, 2, heapID )
void								ELBOARD_TEX_Delete( EL_SCOREBOARD_TEX* elb_tex );
void								ELBOARD_TEX_Main( EL_SCOREBOARD_TEX* elb_tex );

