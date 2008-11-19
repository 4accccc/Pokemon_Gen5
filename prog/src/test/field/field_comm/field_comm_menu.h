//======================================================================
/**
 * @file	field_comm_menu.h
 * @brief	フィールド通信　メニュー部分
 * @author	ariizumi
 * @data	08/11/11
 */
//======================================================================

#ifndef FIELD_COMM_MENU_H__
#define FIELD_COMM_MENU_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
//はい・いいえ選択肢用返り値
enum FIELD_COMM_MENU_YES_NO_RETURN
{
	YNR_WAIT,	//選択中
	YNR_YES,	//はい
	YNR_NO,		//いいえ

	YNR_MAX,
};
//======================================================================
//	typedef struct
//======================================================================
typedef struct _FIELD_COMM_MENU FIELD_COMM_MENU;


//======================================================================
//	proto
//======================================================================
extern FIELD_COMM_MENU* FIELD_COMM_MENU_InitCommMenu( HEAPID heapID );
extern void	FIELD_COMM_MENU_TermCommMenu( FIELD_COMM_MENU *commMenu );
extern void	FIELD_COMM_MENU_OpenYesNoMenu( u8 bgPlane , FIELD_COMM_MENU *commMenu );
extern const u8	FIELD_COMM_MENU_UpdateYesNoMenu( FIELD_COMM_MENU *commMenu );
extern void	FIELD_COMM_MENU_CloseYesNoMenu( FIELD_COMM_MENU *commMenu );

extern void	FIELD_COMM_MENU_OpenMessageWindow( u8 bgPlane , FIELD_COMM_MENU *commMenu );
extern void	FIELD_COMM_MENU_CloseMessageWindow( FIELD_COMM_MENU *commMenu );
extern void	FIELD_COMM_MENU_SetMessage( u16 msgID , FIELD_COMM_MENU *commMenu );
extern void	FIELD_COMM_MENU_UpdateMessageWindow( FIELD_COMM_MENU *commMenu );

#if DEB_ARI
extern void	FIELD_COMM_MENU_SwitchDebugWindow( u8 bgPlane );
extern void	FIELD_COMM_MENU_UpdateDebugWindow( void );
#endif

#endif //FIELD_COMM_MENU_H__
