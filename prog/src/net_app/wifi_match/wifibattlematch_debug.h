//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_debug.h
 *	@brief  デバッグ用
 *	@author	Toru=Nagihashi
 *	@date		2010.04.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#ifdef PM_DEBUG

#include "debug/debugwin_sys.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//デバッグウィンドウ使用するかどうか
#define DEBUGWIN_REG_USE

#define DEBUGWIN_GROUP_REG (41)
#define DEBUGWIN_GROUP_REG_DATE (42)
//=============================================================================
/**
 *  デバッグ
 */
//=============================================================================

#ifdef DEBUGWIN_REG_USE

typedef struct 
{ 
  REGULATION_CARDDATA *p_regulation;
  int page;

  int cup_no;
  int start_year;
  int start_month;
  int start_day;
  int end_year;
  int end_month;
  int end_day;
  int status;
  int bgm;
  int same_match;
} DEBUGWIN_REGULATION_DATA;

static DEBUGWIN_REGULATION_DATA debug_data  = {0};
static inline void DebugWin_Util_ChangeData( DEBUGWIN_ITEM* item, int *p_param, int min, int max )
{ 
  BOOL is_update  = FALSE;

  if( GFL_UI_KEY_GetTrg() == PAD_KEY_LEFT )
  { 
    if( *p_param > min )
    { 
      (*p_param)--;
      is_update = TRUE;
    }
  }
  if( GFL_UI_KEY_GetTrg() == PAD_KEY_RIGHT )
  { 
    if( *p_param  < max )
    { 
      (*p_param)++;
      is_update = TRUE;
    }
  }

  if( is_update )
  { 
    DEBUGWIN_RefreshScreen();
  }
}

static inline void DebugWin_Reg_U_ChangeNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->cup_no, 0, 65535 );
}
static inline void DebugWin_Reg_D_ChangeNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "たいかいNO[%d]", p_wk->cup_no );  
}
static inline void DebugWin_Reg_U_ChangeStartY( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->start_year, 0, 99 );
}
static inline void DebugWin_Reg_D_ChangeStartY( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "かいし ねん[%d]", p_wk->start_year );

}
static inline void DebugWin_Reg_U_ChangeStartM( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->start_month, 1, 12 );
}
static inline void DebugWin_Reg_D_ChangeStartM( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "かいし つき[%d]", p_wk->start_month );
}
static inline void DebugWin_Reg_U_ChangeStartD( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->start_day, 1, 31 );
}
static inline void DebugWin_Reg_D_ChangeStartD( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "かいし び[%d]", p_wk->start_day );

}
static inline void DebugWin_Reg_U_ChangeEndY( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->end_year, 1, 31 );
}
static inline void DebugWin_Reg_D_ChangeEndY( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "おわり ねん[%d]", p_wk->end_year );
}
static inline void DebugWin_Reg_U_ChangeEndM( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->end_month, 1, 12 );
}
static inline void DebugWin_Reg_D_ChangeEndM( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "おわり つき[%d]", p_wk->end_month );
}
static inline void DebugWin_Reg_U_ChangeEndD( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->end_day, 1, 31 );
}
static inline void DebugWin_Reg_D_ChangeEndD( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "おわり ひ[%d]", p_wk->end_day );

}
static inline void DebugWin_Reg_U_ChangeStatus( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->status, DREAM_WORLD_MATCHUP_NONE, DREAM_WORLD_MATCHUP_CHANGE_DS );

}
static inline void DebugWin_Reg_D_ChangeStatus( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_tbl[]  =
  { 
    "なし",
    "エントリー",
    "さんか",
    "しゅうりょう",
    "リタイア",
    "DSかえた",
  }; 

  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "かいさい[%s]", scp_tbl[p_wk->status] );
}
static inline void DebugWin_Reg_U_ChangeBgmNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->bgm, 0, REGULATION_CARD_BGM_WCS );
}
static inline void DebugWin_Reg_D_ChangeBgmNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_tbl[]  =
  { 
    "TRAINER",
    "WCS",
  };
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "BGM[%s]", scp_tbl[p_wk->bgm] );

}
static inline void DebugWin_Reg_U_ChangeSameMatch( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->same_match, 0, 1 );

}
static inline void DebugWin_Reg_D_ChangeSameMatch( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_tbl[]  =
  { 
    "たたかえない",
    "たたかえる",
  };
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "おなじひと[%s]", scp_tbl[p_wk->same_match] );

}

static inline void DebugWin_Reg_U_Get( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    p_wk->cup_no  = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_CUPNO );
    p_wk->start_year = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_START_YEAR );
    p_wk->start_month = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_START_MONTH );
    p_wk->start_day = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_START_DAY );
    p_wk->end_year = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_END_YEAR );
    p_wk->end_month = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_END_MONTH );
    p_wk->end_day = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_END_DAY );
    p_wk->status = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS );
    p_wk->bgm = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_BGM );
    p_wk->same_match = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_SAMEMATCH );
    DEBUGWIN_RefreshScreen();
  }
}
static inline void DebugWin_Reg_U_Set( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_CUPNO, p_wk->cup_no );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_START_YEAR, p_wk->start_year );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_START_MONTH, p_wk->start_month );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_START_DAY, p_wk->start_day );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_END_YEAR, p_wk->end_year );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_END_MONTH, p_wk->end_month );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_END_DAY, p_wk->end_day );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS, p_wk->status );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_BGM, p_wk->bgm );
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_SAMEMATCH, p_wk->same_match );
  }
}


static inline void DEBUGWIN_REG_Init( REGULATION_CARDDATA *p_regulation, HEAPID heapID )
{ 
  debug_data.p_regulation = p_regulation;

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_REG, "レギュレーション", heapID );

  DEBUGWIN_AddItemToGroup( "しゅとく", DebugWin_Reg_U_Get, &debug_data, DEBUGWIN_GROUP_REG, heapID ); 
  DEBUGWIN_AddItemToGroup( "せってい", DebugWin_Reg_U_Set, &debug_data, DEBUGWIN_GROUP_REG, heapID ); 
  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_REG_DATE, "かいさいきかん", DEBUGWIN_GROUP_REG, heapID );


  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeStartY, DebugWin_Reg_D_ChangeStartY,
      &debug_data, DEBUGWIN_GROUP_REG_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeStartM, DebugWin_Reg_D_ChangeStartM,
      &debug_data, DEBUGWIN_GROUP_REG_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeStartD, DebugWin_Reg_D_ChangeStartD,
      &debug_data, DEBUGWIN_GROUP_REG_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeEndY, DebugWin_Reg_D_ChangeEndY,
      &debug_data, DEBUGWIN_GROUP_REG_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeEndM, DebugWin_Reg_D_ChangeEndM,
      &debug_data, DEBUGWIN_GROUP_REG_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeEndD, DebugWin_Reg_D_ChangeEndD,
      &debug_data, DEBUGWIN_GROUP_REG_DATE, heapID );


  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeNo, DebugWin_Reg_D_ChangeNo,
      &debug_data, DEBUGWIN_GROUP_REG, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeStatus, DebugWin_Reg_D_ChangeStatus,
      &debug_data, DEBUGWIN_GROUP_REG, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeBgmNo, DebugWin_Reg_D_ChangeBgmNo,
      &debug_data, DEBUGWIN_GROUP_REG, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeSameMatch, DebugWin_Reg_D_ChangeSameMatch,
      &debug_data, DEBUGWIN_GROUP_REG, heapID );
}

static inline void DEBUGWIN_REG_Exit( void )
{ 
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_REG );
}




#else

#define DEBUGWIN_REG_Init( ... )  /*  */
#define DEBUGWIN_REG_Exit( ... )  /*  */

#endif  //DEBUGWIN_REG_USE
#endif  //PM_DEBUG
