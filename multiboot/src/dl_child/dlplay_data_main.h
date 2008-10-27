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

extern	BOOL	DLPlayData_LoadDataFirst( DLPLAY_DATA_DATA *d_data );
extern	BOOL	DLPlayData_SaveData( DLPLAY_DATA_DATA *d_data );


#endif // DLPLAY_DATA_MAIN_H__

