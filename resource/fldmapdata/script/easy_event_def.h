//--------------------------------------------------------------
//--------------------------------------------------------------
  .macro  _ASM_EASY_ITEM_EVENT itemno, num, flag, before_msg, finish_msg, after_msg
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _PUSH_WORK  SCWK_PARAM2
  _PUSH_WORK  SCWK_PARAM3
  _PUSH_WORK  SCWK_PARAM4
  _PUSH_WORK  SCWK_PARAM5

  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _ASM_LDWKVAL  SCWK_PARAM2, \flag
  _ASM_LDWKVAL  SCWK_PARAM3, \before_msg
  _ASM_LDWKVAL  SCWK_PARAM4, \finish_msg
  _ASM_LDWKVAL  SCWK_PARAM5, \after_msg

  _CHG_COMMON_SCR SCRID_EASY_ITEM_EVENT
  
  _PUSH_WORK  SCWK_PARAM5
  _PUSH_WORK  SCWK_PARAM4
  _PUSH_WORK  SCWK_PARAM3
  _PUSH_WORK  SCWK_PARAM2
  _PUSH_WORK  SCWK_PARAM1
  _PUSH_WORK  SCWK_PARAM0
  .endm
/*
  .macro  _ASM_EASY_ITEM_EVENT itemno, num, flag, before_msg, finish_msg, after_msg
    _ASM_TALK_START_SE_PLAY

    _IF_FLAGON_JUMP \flag, item_already_get
  
    _ASM_LDWKVAL  SCWK_TEMP0, \itemno
    _ASM_LDWKVAL  SCWK_TEMP1, \num

    _ASM_BALLOONWIN_TALKOBJ_OPEN  \before_msg
    _ASM_AB_KEYWAIT
    _ASM_BALLOONWIN_CLOSE

    _ASM_ADD_ITEM_CHK \itemno, \num, SCWK_ANSWER
    _IFVAL_JUMP  SCWK_ANSWER,EQ,FALSE, item_full

    _CHG_COMMON_SCR SCRID_ITEM_EVENT_SUCCESS

    _ASM_BALLOONWIN_TALKOBJ_OPEN  \finish_msg
    _ASM_AB_KEYWAIT
    _ASM_BALLOONWIN_CLOSE

    _JUMP   item_event_end

item_full:
    _CHG_COMMON_SCR SCRID_ITEM_EVENT_FAIL
    _JUMP   item_event_end

item_already_get:
    _ASM_TALKWIN_OPEN
    _ASM_TALKMSG_ALLPUT \after_msg
    _ASM_AB_KEYWAIT
    _ASM_TALKWIN_CLOSE

item_event_end:
  .endm
*/

#define _EASY_ITEM_EVENT( itemno, num, flag, before_msg, finish_msg, after_msg) \
    _ASM_EASY_ITEM_EVENT itemno, num, flag, before_msg, finish_msg, after_msg

