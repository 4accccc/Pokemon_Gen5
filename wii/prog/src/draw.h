#ifdef __cplusplus
extern "C" {
#endif

#ifndef DRAW_H__
#define DRAW_H__

extern void	InitDrawSystem(void);
extern void	UpdateDrawSystem(void);
extern void	TermDrawSystem(void);

enum
{
	ANIM_DOWN_STOP,
	ANIM_DOWN_WALK,
	ANIM_LEFT_STOP,
	ANIM_LEFT_WALK,
	ANIM_RIGHT_STOP,
	ANIM_RIGHT_WALK,
	ANIM_UP_STOP,
	ANIM_UP_WALK,
	
	ANIM_TYPE_MAX,
		
};


#endif	DRAW_H__

#ifdef __cplusplus
} /* extern "C" */
#endif
