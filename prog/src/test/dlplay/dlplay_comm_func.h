//======================================================================
/**
 *
 * @file	dlplay_comm_func.c	
 * @brief	DL通信 親子共通の通信関係
 * @author	ariizumi
 * @data	08/10/21
 */
//======================================================================

#ifndef DLPALY_COMM_FUNC__
#define DLPALY_COMM_FUNC__

#include "dlplay_func.h"

#define TEST_DATA_LINE (16)
#define TEST_DATA_NUM  (256)

//DP・PTのサイズに合わせる( pt_save.h sizeof(pt_box_data))
#define LARGEPACKET_POKE_SIZE (0x1000)
typedef struct _DLPLAY_COMM_DATA DLPLAY_COMM_DATA;
//送信用データ
typedef struct
{
	u8	cardType_;
	u8 pokeData_[LARGEPACKET_POKE_SIZE];
}DLPLAY_LARGE_PACKET;

//送信用 BOXインデックス
#define BOX_TRAY_NUM (18)
#define BOX_MONS_NUM (30)
#define BOX_TRAY_NAME_LEN (20)
#define BOX_MONS_NAME_LEN (10+1)
typedef struct
{
	u16 name_[BOX_MONS_NAME_LEN];
	u16 pokeNo_:9;
	u16	lv_:7;
	u8	sex_:2;
	u8	form_:6;
	u8	color_;
}DLPLAY_MONS_INDEX;
typedef struct
{
	u16	boxName_[BOX_TRAY_NAME_LEN];
	DLPLAY_MONS_INDEX pokeData_[BOX_TRAY_NUM][BOX_MONS_NUM];
}DLPLAY_BOX_INDEX;

//本体に刺さっているカードの種類
typedef enum {
	CARD_TYPE_DP,		//ダイアモンド＆パール
	CARD_TYPE_PT,		//プラチナ
	CARD_TYPE_GS,		//ゴールド＆シルバー
	CARD_TYPE_INVALID,
}DLPLAY_CARD_TYPE;

extern DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID );
extern void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm);
#if DLPLAY_FUNC_USE_PRINT
extern void	DLPlayComm_SetMsgSys( DLPLAY_MSG_SYS *msgSys , DLPLAY_COMM_DATA *d_comm );
#endif
extern void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_InitChild( DLPLAY_COMM_DATA *d_comm , u8 *macAddress );
extern BOOL	DLPlayComm_IsFinish_ConnectParent( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );
extern BOOL DLPlayComm_IsConnect( DLPLAY_COMM_DATA *d_comm );
extern BOOL DLPlayComm_IsPostIndex( DLPLAY_COMM_DATA *d_comm );

extern DLPLAY_LARGE_PACKET*	DLPlayComm_GetLargePacketBuff( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_LargeData( DLPLAY_COMM_DATA *d_comm );
extern DLPLAY_BOX_INDEX* DLPlayComm_GetBoxIndexBuff( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_BoxIndex( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_Send_ConnectSign( DLPLAY_COMM_DATA *d_comm );

#endif // DLPALY_COMM_FUNC__

