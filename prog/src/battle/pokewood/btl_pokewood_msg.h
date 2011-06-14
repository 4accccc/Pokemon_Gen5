//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		btl_pokewood_msg.h
 *	@brief  ポケウッドメッセージタグ
 *	@author	Toru=Nagihashi
 *	@date		2011.06.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "msg/msg_pokewood_senario01_m.h" //01_m～すべて同じタグ

typedef enum
{
  BTL_PWMSG_PLAYER,
  BTL_PWMSG_ENEMY,
}BTL_PWMSG_TYPE;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

static inline int BTL_PWMSG_Title( void )
{
  return pw_scene_title;
}
static inline int BTL_PWMSG_Mission( void )
{
  return pw_scene_mission;
}
static inline int BTL_PWMSG_Hint( void )
{
  return pw_scene_hint;
}

static inline int BTL_PWMSG_WazaSel( BTL_PWMSG_TYPE type, int turn )
{
  return pw_wazasel_hero_0 + type + turn * 2;
}
static inline int BTL_PWMSG_Situation( BTL_PWMSG_TYPE type, int turn )
{
  return pw_situ_hero_0 + type + turn * 2;
}
static inline int BTL_PWMSG_Turn( BTL_PWMSG_TYPE type, int turn )
{
  return pw_turn_hero_0 + type + turn * 2;
}
static inline int BTL_PWMSG_Event( BTL_PWMSG_TYPE type, int ev_no, int ev_turn )
{
  const int event_msg_no = pw_event1_hero_0 - pw_event0_hero_0;
  return pw_event0_hero_0 + type + ev_turn * 2 + event_msg_no * ev_no;
}

static inline int BTL_PWMSG_EventReAct( BTL_PWMSG_TYPE type, int ev_no, int react_no )
{
  const int event_msg_no = pw_event1_react_hero_0 - pw_event0_react_hero_0;
  return pw_event0_react_hero_0 + type + react_no * 2 + event_msg_no * ev_no;
}

