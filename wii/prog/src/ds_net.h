#ifndef DS_NET_H__
#define DS_NET_H__


typedef struct
{
	BOOL	isValid_;

	u16 posX_;
	u16 posY_;
	u16 posZ_;
	
	u8	oldAnim_;
	u8	newAnim_;
}PLAYER_STATE;

extern void	MPDS_Init(void);
extern const BOOL MPDS_PostShareData( const u8 *adr , const u8 aid );

extern PLAYER_STATE*	MPDS_GetPlayerState( const u8 idx );
extern void		MPDS_InitPlayerState( const u8 idx );

#endif //DS_NET_H__