//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _SKILL_CONTROL	SKILL_CONTROL;

typedef enum {
	SKILL_NOP = 0,
	SKILL_SWORD0,
	SKILL_SWORD1,
	SKILL_SWORD2,
	SKILL_SWORD3,
	SKILL_ARROW,
	SKILL_STAFF,
}SKILL_CONTROL_COMMAND;

typedef struct _STATUSWIN_CONTROL	STATUSWIN_CONTROL;

typedef struct {
	HEAPID				heapID;
	GAME_SYSTEM*		gs;
	TEAM_CONTROL**		p_tc;
	int					teamCount;
}SKILLCONT_SETUP;

//スキルコントロールセット
extern SKILL_CONTROL* AddSkillControl( SKILLCONT_SETUP* setup );
//スキルコントロール終了
extern void RemoveSkillControl( SKILL_CONTROL* sc );
//スキルの追加
extern void SetSkillControlCommand
	( SKILL_CONTROL* sc, TEAM_CONTROL* tc, PLAYER_CONTROL* pc, PLAYER_SKILL_COMMAND skillCommand );
//スキルメインコントロール
extern void MainSkillControl( SKILL_CONTROL* sc, BOOL onGameFlag );


