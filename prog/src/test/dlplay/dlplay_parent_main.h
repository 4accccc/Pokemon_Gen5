//============================================================================================
/**
 * @file	dlplay_parent_main.h
 * @brief	ダウンロードプレイ 子機配信部分
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================

#ifndef DLPLAY_PARENT_MAIN_H__
#define DLPLAY_PARENT_MAIN_H__

typedef struct _DLPLAY_SEND_DATA DLPLAY_SEND_DATA;

extern DLPLAY_SEND_DATA* DLPlaySend_Init( int heapID );
extern u8	DLPlaySend_Loop( DLPLAY_SEND_DATA *dlData );
extern void	DLPlaySend_Term( DLPLAY_SEND_DATA *dlData );

#endif //DLPLAY_PARENT_MAIN_H__

