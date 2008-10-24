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

#define TEST_DATA_LINE (16)
#define TEST_DATA_NUM  (256)
typedef struct _DLPLAY_COMM_DATA DLPLAY_COMM_DATA;
//送信用データ
typedef struct
{
	//ポケモンのデータは1匹100バイトくらいらしい
	char data_[TEST_DATA_LINE][TEST_DATA_NUM];
}DLPLAY_LARGE_PACKET;

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

extern void	DLPlayComm_InitParent( DLPLAY_COMM_DATA *d_comm );
extern void	DLPlayComm_InitChild( DLPLAY_COMM_DATA *d_comm , u8 *macAddress );
extern BOOL	DLPlayComm_IsFinish_ConnectParent( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );

void	DLPlayComm_Send_LargeData( DLPLAY_LARGE_PACKET *data , DLPLAY_COMM_DATA *d_comm );
void	DLPlayComm_Send_ConnectSign( DLPLAY_COMM_DATA *d_comm );

#endif // DLPALY_COMM_FUNC__

