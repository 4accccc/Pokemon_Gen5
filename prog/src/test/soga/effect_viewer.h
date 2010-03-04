//============================================================================================
/**
 * @file	effect_viewer.h
 * @brief	エフェクトビューワーで使用するデータ群
 * @author	soga
 * @date	2009.04.09
 */
//============================================================================================

#include "msg/msg_d_soga.h"

#define	NO_MOVE	( 0xff )
#define	NO_EDIT	( -1 )

#define	STR_DOT	( 12 )		//文字サイズ

//typedef BOOL (*EV_FUNC)( EFFECT_VIEWER_WORK * );

typedef struct
{
	u8		cursor_pos_x;	//カーソルX位置
	u8		cursor_pos_y;	//カーソルY位置
	u8		move_up;		//上キーを押したときの移動先
	u8		move_down;		//下キーを押したときの移動先
	u8		move_left;		//左キーを押したときの移動先
	u8		move_right;		//右キーを押したときの移動先
}MENU_LIST_PARAM;

enum{
	EDIT_NONE = 0,
	EDIT_DECIDE,
	EDIT_CANCEL,
	EDIT_INT,
	EDIT_FX32,
	EDIT_COMBOBOX,
};

typedef struct
{
	u16	label_x;	//見出しX座標
	u16	label_y;	//見出しY座標
	u32	strID;		//見出しstrID
	u16	data_x;		//データX座標
	u16	data_y;		//データY座標
	u16	edit_type;	//編集タイプ
	u8	edit_min;	//編集タイプがEDIT_COMBOBOXのときの最小値
	u8	edit_max;	//編集タイプがEDIT_COMBOBOXのときの最大値
}MENU_SCREEN_DATA;

typedef struct
{
	int							count;
	const	MENU_SCREEN_DATA	**msd;
	const	MENU_LIST_PARAM		*mlp;
}MENU_SCREEN_PARAM;

//============================================================================================
/**
 *		共通見出し
 */
//============================================================================================

#define	ATTACK_LABEL_X	( STR_DOT * 1 )
#define	ATTACK_LABEL_Y	( STR_DOT * 1 )
#define	ATTACK_DATA_X	( ATTACK_LABEL_X + STR_DOT * 6 )
#define	ATTACK_DATA_Y	( ATTACK_LABEL_Y )
#define	DEFENCE_LABEL_X	( STR_DOT * 1 )
#define	DEFENCE_LABEL_Y	( STR_DOT * 2 )
#define	DEFENCE_DATA_X	( DEFENCE_LABEL_X + STR_DOT * 6 )
#define	DEFENCE_DATA_Y	( DEFENCE_LABEL_Y )
#define	DECIDE_LABEL_X	( STR_DOT * 1 )
#define	DECIDE_LABEL_Y	( STR_DOT * 12 )
#define	CANCEL_LABEL_X	( DECIDE_LABEL_X + STR_DOT * 8 )
#define	CANCEL_LABEL_Y	( STR_DOT * 12 )

static const MENU_SCREEN_DATA menu_attack = {
	ATTACK_LABEL_X, ATTACK_LABEL_Y, EVMSG_ATTACK,
	ATTACK_DATA_X, ATTACK_DATA_Y, EDIT_COMBOBOX, EVMSG_POS_AA, EVMSG_POS_D, //EVMSG_POS_F,
};

static const MENU_SCREEN_DATA menu_defence = {
	DEFENCE_LABEL_X, DEFENCE_LABEL_Y, EVMSG_DEFENCE,
	DEFENCE_DATA_X, DEFENCE_DATA_Y, EDIT_COMBOBOX, EVMSG_POS_AA, EVMSG_POS_D, //EVMSG_POS_F,
};

static const MENU_SCREEN_DATA menu_decide = {
	DECIDE_LABEL_X, DECIDE_LABEL_Y, EVMSG_DECIDE,
	0, 0, EDIT_DECIDE, 0, 0,
};

static const MENU_SCREEN_DATA menu_cancel = {
	CANCEL_LABEL_X, CANCEL_LABEL_Y, EVMSG_CANCEL,
	0, 0, EDIT_CANCEL, 0, 0,
};

//============================================================================================
/**
 *		タイトル
 */
//============================================================================================

#define	TITLE_STRLEN	( 14 )
#define	TITLE_LABEL_X	( 128 - ( TITLE_STRLEN * STR_DOT / 2 ) )
#define	TITLE_LABEL_Y	( 96 - ( STR_DOT / 2 ) )

static const MENU_SCREEN_DATA menu_title = {
	TITLE_LABEL_X, TITLE_LABEL_Y, EVMSG_TITLE,
	0, 0, EDIT_NONE, 0, 0,
};

static const MENU_SCREEN_DATA *msd_title[] = {
	&menu_title,
};

static const MENU_SCREEN_PARAM	msp_title = {
	NELEMS( msd_title ),
	&msd_title[ 0 ],
	NULL
};

//============================================================================================
/**
 *		パーティクル再生エディット
 */
//============================================================================================

#define	PPE_OFSY_LABEL_X	( STR_DOT * 1 )
#define	PPE_OFSY_LABEL_Y	( STR_DOT * 6 )
#define	PPE_OFSY_DATA_X		( PPE_OFSY_LABEL_X + STR_DOT * 8 )
#define	PPE_OFSY_DATA_Y		( PPE_OFSY_LABEL_Y )
#define	PPE_ANGLE_LABEL_X	( STR_DOT * 1 )
#define	PPE_ANGLE_LABEL_Y	( STR_DOT * 7 )
#define	PPE_ANGLE_DATA_X	( PPE_ANGLE_LABEL_X + STR_DOT * 8 )
#define	PPE_ANGLE_DATA_Y	( PPE_ANGLE_LABEL_Y )

#define	PPE_SEND_SIZE		( 4 * 3 )		//MCSでPCに送信するデータサイズ

static const MENU_SCREEN_DATA menu_ppe_ofsy = {
	PPE_OFSY_LABEL_X, PPE_OFSY_LABEL_Y, EVMSG_PPLAY_OFSY,
	PPE_OFSY_DATA_X, PPE_OFSY_DATA_Y, EDIT_FX32, 0, 0,
};

static const MENU_SCREEN_DATA menu_ppe_angle = {
	PPE_ANGLE_LABEL_X, PPE_ANGLE_LABEL_Y, EVMSG_PPLAY_ANGLE,
	PPE_ANGLE_DATA_X, PPE_ANGLE_DATA_Y, EDIT_FX32, 0, 0,
};

static const MENU_SCREEN_DATA *msd_ppe[] = {
	&menu_attack,
	&menu_defence,
	&menu_ppe_ofsy,
	&menu_ppe_angle,
	&menu_decide,
	&menu_cancel,
};

enum{
	MLP_PPE_ATTACK = 0,
	MLP_PPE_DEFENCE,
	MLP_PPE_OFSY,
	MLP_PPE_ANGLE,
	MLP_PPE_DECIDE,
	MLP_PPE_CANCEL,
};

enum{
	PPEPARAM_NUM = 0,	//再生パーティクルナンバー
	PPEPARAM_INDEX,		//spa内インデックスナンバー
	PPEPARAM_START_POS,	//パーティクル再生開始立ち位置
	PPEPARAM_DIR_POS,	//パーティクル再生方向立ち位置
	PPEPARAM_OFS_Y,		//パーティクル再生Y方向オフセット
	PPEPARAM_DIR_ANGLE,	//パーティクル再生方向Y角度
};

enum{ 
  SPPARAM_SE_NO = 0, 
  SPPARAM_PLAYER, 
  SPPARAM_PAN, 
  SPPARAM_WAIT, 
  SPPARAM_PITCH, 
  SPPARAM_VOLUME, 
  SPPARAM_MODUDEPTH, 
  SPPARAM_MODUSPEED, 
};

enum{ 
  SPANPARAM_PLAYER = 0,
  SPANPARAM_TYPE,
  SPANPARAM_START,
  SPANPARAM_END,
  SPANPARAM_START_WAIT,
  SPANPARAM_FRAME,
  SPANPARAM_WAIT,
  SPANPARAM_COUNT,
};

enum{ 
  SEFFPARAM_PLAYER = 0,
  SEFFPARAM_TYPE,
  SEFFPARAM_PARAM,
  SEFFPARAM_START,
  SEFFPARAM_END,
  SEFFPARAM_START_WAIT,
  SEFFPARAM_FRAME,
  SEFFPARAM_WAIT,
  SEFFPARAM_COUNT,
};

static const MENU_LIST_PARAM mlp_ppe[]={
	//MLP_PPE_ATTACK
	{
		ATTACK_LABEL_X - STR_DOT,
		ATTACK_LABEL_Y,
		NO_MOVE,	
		MLP_PPE_DEFENCE,	
		NO_MOVE,
		NO_MOVE,
	},
	//MLP_PPE_DEFENCE
	{
		DEFENCE_LABEL_X - STR_DOT,
		DEFENCE_LABEL_Y,
		MLP_PPE_ATTACK,	
		MLP_PPE_OFSY,	
		NO_MOVE,
		NO_MOVE,
	},
	//MLP_PPE_OFSY
	{
		PPE_OFSY_LABEL_X - STR_DOT,
		PPE_OFSY_LABEL_Y,
		MLP_PPE_DEFENCE,	
		MLP_PPE_ANGLE,	
		NO_MOVE,
		NO_MOVE,
	},
	//MLP_PPE_ANGLE
	{
		PPE_ANGLE_LABEL_X - STR_DOT,
		PPE_ANGLE_LABEL_Y,
		MLP_PPE_OFSY,	
		MLP_PPE_DECIDE,	
		NO_MOVE,
		NO_MOVE,
	},
	//MLP_PPE_DECIDE
	{
		DECIDE_LABEL_X - STR_DOT,
		DECIDE_LABEL_Y,
		MLP_PPE_ANGLE,	
		NO_MOVE,	
		NO_MOVE,
		MLP_PPE_CANCEL,
	},
	//MLP_PPE_CANCEL
	{
		CANCEL_LABEL_X - STR_DOT,
		CANCEL_LABEL_Y,
		MLP_PPE_ANGLE,	
		NO_MOVE,	
		MLP_PPE_DECIDE,
		NO_MOVE,
	},
};

static const MENU_SCREEN_PARAM msp_ppe = {
	NELEMS( msd_ppe ),
	&msd_ppe[ 0 ],
	&mlp_ppe[ 0 ],
};

//============================================================================================
/**
 *		メニューリスト
 */
//============================================================================================

enum{
	MENULIST_TITLE = 0,
	MENULIST_PPE,
};

static const MENU_SCREEN_PARAM	*msp[] = {
	&msp_title,
	&msp_ppe,
};

