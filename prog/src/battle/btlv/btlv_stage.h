
//============================================================================================
/**
 * @file	btlv_stage.h
 * @brief	êÌì¨âÊñ ï`âÊópä÷êîÅiÇ®ñ~Åj
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================

#ifndef	__BTLV_STAGE_H_
#define	__BTLV_STAGE_H_

enum{
	BTLV_STAGE_MINE = 0,			//é©ï™ë§Ç®ñ~
	BTLV_STAGE_ENEMY,			//ëäéËë§Ç®ñ~
	BTLV_STAGE_MAX				//âÊñ Ç…èoÇ∑Ç®ñ~ÇÃêî
};

typedef struct _BTLV_STAGE_WORK BTLV_STAGE_WORK;

extern	BTLV_STAGE_WORK	*BTLV_STAGE_Init( int index, HEAPID heapID );
extern	void			BTLV_STAGE_Exit( BTLV_STAGE_WORK *bsw );
extern	void			BTLV_STAGE_Main( BTLV_STAGE_WORK *bsw );
extern	void			BTLV_STAGE_Draw( BTLV_STAGE_WORK *bsw );

#endif	//__BTLV_STAGE_H_
