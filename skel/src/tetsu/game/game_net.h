//============================================================================================
/**
 * @file	game_net.h
 * @brief	í êMìÆçÏópä÷êî
 */
//============================================================================================
extern void		InitGameNet(void);
extern BOOL		ConnectGameNet(void);
extern void		MainGameNet(void);
extern BOOL		ExitGameNet(void);
extern void		SendGameNet( int comm, void* commWork );
extern NetID	GetNetID(void);

//#define NET_WORK_ON

