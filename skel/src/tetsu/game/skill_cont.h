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
	SKILL_SWORD,
	SKILL_ARROW,
	SKILL_STAFF,
}SKILL_CONTROL_COMMAND;

//スキルコントロールセット
extern SKILL_CONTROL* AddSkillControl( GAME_SYSTEM* gs, HEAPID heapID );
//スキルコントロール終了
extern void RemoveSkillControl( SKILL_CONTROL* sc );
//判定プレーヤーの追加
extern void AddSkillCheckPlayer( SKILL_CONTROL* sc, PLAYER_CONTROL* pc );
//判定プレーヤーの削除
extern void RemoveSkillCheckPlayer( SKILL_CONTROL* sc, PLAYER_CONTROL* pc );
//スキルの追加
extern void SetSkillControlCommand( SKILL_CONTROL* sc, PLAYER_CONTROL* pc, int skillCommand );
//スキルメインコントロール
extern void MainSkillControl( SKILL_CONTROL* sc );

