//======================================================================
/**
 *
 * @file	dlplay_data_main.c
 * @brief	セーブデータを読み込むところ
 * @author	ariizumi
 * @data	
 */
//======================================================================

#ifndef	DLPLAY_DATA_MAIN_H__
#define DLPLAY_DATA_MAIN_H__

typedef struct _DLPLAY_DATA_DATA DLPLAY_DATA_DATA;	//DLPLAYDATAのDATA・・・ややこしい・・・

extern	DLPLAY_DATA_DATA* DLPlayData_InitSystem( int heapID , DLPLAY_MSG_SYS *msgSys );
extern	void			  DLPlayData_TermSystem( DLPLAY_DATA_DATA *d_data );

extern const	DLPLAY_CARD_TYPE DLPlayData_GetCardType( DLPLAY_DATA_DATA *d_data );
extern void		DLPlayData_SetCardType( DLPLAY_DATA_DATA *d_data , const DLPLAY_CARD_TYPE type );

extern	BOOL	DLPlayData_LoadDataFirst( DLPLAY_DATA_DATA *d_data );
extern	BOOL	DLPlayData_SaveData( DLPLAY_DATA_DATA *d_data );
extern	void	DLPlayData_SetBoxIndex( DLPLAY_DATA_DATA *d_data , DLPLAY_BOX_INDEX *boxIndex );
extern	u8*		DLPlayData_GetPokeSendData( DLPLAY_DATA_DATA *d_data );

#endif // DLPLAY_DATA_MAIN_H__

