
//============================================================================================
/**
 * @file	btl_field.h
 * @brief	戦闘画面描画用関数（フィールド）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================

#ifndef	__BTL_FIELD_H_
#define	__BTL_FIELD_H_

enum{
	BTL_FIELD_GROUND = 0,		//地面
	BTL_FIELD_BG,				//背景
	BTL_FIELD_MAX				//フィールド構成OBJ数
};

typedef struct _BTL_FIELD_WORK BTL_FIELD_WORK;

extern	BTL_FIELD_WORK	*BTL_FIELD_Init( int index, HEAPID heapID );
extern	void			BTL_FIELD_Exit( BTL_FIELD_WORK *bsw );
extern	void			BTL_FIELD_Main( BTL_FIELD_WORK *bsw );
extern	void			BTL_FIELD_Draw( BTL_FIELD_WORK *bsw );

#endif	//__BTL_FIELD_H_
