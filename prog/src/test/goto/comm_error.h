
#ifndef __COMM_ERROR_H__
#define __COMM_ERROR_H__


#define COMM_ERROR_IRREAGULARITY		( 0xFFFF )
#define COMM_ERROR_PALETTE_POS			( 15 )
#define COMM_ERROR_BG_PLTT_ADRS_MAIN	( HW_BG_PLTT + ( 0x20 * COMM_ERROR_PALETTE_POS ) )
#define COMM_ERROR_BG_PLTT_ADRS_SUB		( HW_DB_BG_PLTT + ( 0x20 * COMM_ERROR_PALETTE_POS ) )

extern void CommErrorSys_Setup( void );
extern void CommErrorSys_Init( int push_frame );
extern void CommErrorSys_Call( void );
extern BOOL CommErrorSys_IsActive( void );
extern void CommErrorSys_PushFrameParam( void );
extern void CommErrorSys_PopFrameParam( void );
extern void CommErrorSys_PushFrameVram( void );
extern void CommErrorSys_PopFrameVram( void );
extern void CommErrorSys_Draw( void );

extern void CommErrorSys_SampleView( void );

#endif
