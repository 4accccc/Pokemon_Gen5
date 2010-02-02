//============================================================================================
/**
 * @file	ev_time.h
 * @brief	ÉQÅ[ÉÄì‡éûä‘êßå‰ä÷òA
 * @author	tamada
 * @date	2006.02.03
 */
//============================================================================================

#ifndef	__EV_TIME_H__
#define	__EV_TIME_H__

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void EVTIME_Update(GAMEDATA * gdata);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const RTCDate * EVTIME_GetRTCDate( const GAMEDATA * gdata );
extern int EVTIME_GetTimeZone(const GAMEDATA * gdata);
extern int EVTIME_GetHour(const GAMEDATA * gdata);
extern int EVTIME_GetMinute(const GAMEDATA * gdata);
extern int EVTIME_GetMonth(const GAMEDATA * gdata);
extern int EVTIME_GetDay(const GAMEDATA * gdata);
extern int EVTIME_GetWeek(const GAMEDATA * gdata);

extern void EVTIME_GetGameStartDateTime(const GAMEDATA * gdata, RTCDate * date, RTCTime * time);
extern void EVTIME_GetGameClearDateTime(const GAMEDATA * gdata, RTCDate * date, RTCTime * time);
extern void EVTIME_SetGameClearDateTime(const GAMEDATA * gdata);

extern BOOL EVTIME_IsPenaltyMode(GAMEDATA * gdata);


#endif	/* __EV_TIME_H__ */
