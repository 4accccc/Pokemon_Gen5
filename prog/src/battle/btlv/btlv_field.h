
//============================================================================================
/**
 * @file	btlv_field.h
 * @brief	戦闘画面描画用関数（フィールド）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================

#ifndef	__BTLV_FIELD_H_
#define	__BTLV_FIELD_H_

enum{
	BTLV_FIELD_GROUND = 0,		//地面
	BTLV_FIELD_BG,				//背景
//	BTLV_FIELD_SHADOW_LINE,		//影線
	BTLV_FIELD_MAX				//フィールド構成OBJ数
};

#define	BTLV_FIELD_SHADOW_LINE_START_POS	( FX32_ONE * 12 )	//影線の初期位置
#define	BTLV_FIELD_SHADOW_LINE_MOVE_FRAME	( 32 )				//影線の移動フレーム数
#define	BTLV_FIELD_SHADOW_LINE_MOVE_WAIT	( 2 )				//影線の移動ウェイト

typedef struct _BTLV_FIELD_WORK BTLV_FIELD_WORK;

extern	BTLV_FIELD_WORK	*BTLV_FIELD_Init( GFL_TCBSYS *tcb_sys, int index, HEAPID heapID );
extern	void			BTLV_FIELD_Exit( BTLV_FIELD_WORK *bsw );
extern	void			BTLV_FIELD_Main( BTLV_FIELD_WORK *bsw );
extern	void			BTLV_FIELD_Draw( BTLV_FIELD_WORK *bsw );

#endif	//__BTLV_FIELD_H_
