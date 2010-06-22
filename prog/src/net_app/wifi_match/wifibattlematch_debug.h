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
#include "savedata/battle_box_save.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//デバッグウィンドウ使用するかどうか
#define DEBUGWIN_REG_USE
#define DEBUGWIN_SAKE_RECORD_DATA_USE
#define DEBUGWIN_WIFISCORE_USE
#define DEBUGWIN_LIVESCORE_USE
#define DEBUGWIN_REPORT_USE
#define DEBUGWIN_BTLBOX_USE
#define DEBUGWIN_BTLBGM_USE


#define DEBUGWIN_GROUP_REG (41)
#define DEBUGWIN_GROUP_REG_DATE (42)

#define DEBUGWIN_GROUP_SAKE_RECORD (51)
#define DEBUGWIN_GROUP_SAKE_RECORD_YOUPROFILE (52)
#define DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE (53)
#define DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE (54)
#define DEBUGWIN_GROUP_SAKE_RECORD_DATE  (55)
#define DEBUGWIN_GROUP_SAKE_RECORD_RESULT  (56)

#define DEBUGWIN_GROUP_WIFISCORE  (60)
#define DEBUGWIN_GROUP_LIVESCORE  (65)
#define DEBUGWIN_GROUP_LIVESCORE_MY  (66)
#define DEBUGWIN_GROUP_LIVESCORE_FOE  (67)

#define DEBUGWIN_GROUP_REPORT (70)

#define DEBUGWIN_GROUP_BTLBOX (80)

#define DEBUGWIN_GROUP_BTLBGM (90)


#include "wifibattlematch_debugdata.h"

//=============================================================================
/**
 *  デバッグ汎用
 */
//=============================================================================
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

static inline void DebugWin_Util_ChangeDataU32( DEBUGWIN_ITEM* item, u32 *p_param, u32 min, u32 max )
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

static inline void DebugWin_Util_ChangeDataU16( DEBUGWIN_ITEM* item, u16 *p_param, u16 min, u16 max )
{ 
  BOOL is_update  = FALSE;
  u16 value  = 1;

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    value = 10;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    value = 100;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    value = 1000;
  }

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    if( *p_param - value >= min )
    { 
      (*p_param)-=value;
    }
    else
    {
      (*p_param)  = min;
    }
    is_update = TRUE;
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    if( *p_param + value <= max )
    { 
      (*p_param)+=value;
    }
    else
    {
      (*p_param)  = max;
    }
    is_update = TRUE;
  }

  if( is_update )
  { 
    DEBUGWIN_RefreshScreen();
  }
}

static inline void DebugWin_Util_ChangeDataU8( DEBUGWIN_ITEM* item, u8 *p_param, u8 min, u8 max )
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


//=============================================================================
/**
 *  デバッグレギュレーション変更
 *     DEBUGWIN_REG_InitとDEBUGWIN_REG_Exitを呼ぶだけでおKです
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
  int camera;
} DEBUGWIN_REGULATION_DATA;

static DEBUGWIN_REGULATION_DATA debug_data  = {0};


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

static inline void DebugWin_Reg_U_ChangeCamera( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->camera, 0, REGULATION_STATE_WCS_02 );

}
static inline void DebugWin_Reg_D_ChangeCamera( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_tbl[]  =
  { 
    "NORMAL_15",
    "NORMAL_02",
    "WCS_15",
    "WCS_02",
  };
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じょうきょう[%s]", scp_tbl[p_wk->camera] );

}

//-------------------------------------
///	反映
//=====================================
static inline void DebugWin_Reg_Get( DEBUGWIN_REGULATION_DATA  *p_wk )
{
  REGULATION  *p_reg = RegulationData_GetRegulation(p_wk->p_regulation);
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
  p_wk->camera = Regulation_GetParam( p_reg, REGULATION_STATE );
}

static inline void DebugWin_Reg_U_Get( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    DebugWin_Reg_Get( p_wk );
    DEBUGWIN_RefreshScreen();
  }
}
static inline void DebugWin_Reg_U_Set( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    REGULATION  *p_reg = RegulationData_GetRegulation(p_wk->p_regulation);

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
    Regulation_SetParam( p_reg, REGULATION_STATE, p_wk->camera );
  }
}
static inline void DebugWin_Reg_U_Clear( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_REGULATION_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    p_wk->cup_no  = 0;
    p_wk->start_year = 0;
    p_wk->start_month = 0;
    p_wk->start_day = 0;
    p_wk->end_year = 0;
    p_wk->end_month = 0;
    p_wk->end_day = 0;
    p_wk->status = 0;
    p_wk->bgm = 0;
    p_wk->same_match = 0;
    DEBUGWIN_RefreshScreen();
  }
}

//-------------------------------------
///	public
//=====================================
static inline void DEBUGWIN_REG_Init( REGULATION_CARDDATA *p_regulation, HEAPID heapID )
{ 
  debug_data.p_regulation = p_regulation;

  DebugWin_Reg_Get( &debug_data );

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_REG, "レギュレーション", heapID );

  DEBUGWIN_AddItemToGroup( "しゅとく", DebugWin_Reg_U_Get, &debug_data, DEBUGWIN_GROUP_REG, heapID ); 
  DEBUGWIN_AddItemToGroup( "せってい", DebugWin_Reg_U_Set, &debug_data, DEBUGWIN_GROUP_REG, heapID ); 
  DEBUGWIN_AddItemToGroup( "けす", DebugWin_Reg_U_Clear, &debug_data, DEBUGWIN_GROUP_REG, heapID ); 
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
  DEBUGWIN_AddItemToGroupEx( DebugWin_Reg_U_ChangeCamera, DebugWin_Reg_D_ChangeCamera,
      &debug_data, DEBUGWIN_GROUP_REG, heapID );
}

static inline void DEBUGWIN_REG_Exit( void )
{ 
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_REG );
}

#endif  //DEBUGWIN_REG_USE

//=============================================================================
/**
 *  サケに置くデータ
 */
//=============================================================================
#ifdef DEBUGWIN_SAKE_RECORD_DATA_USE

typedef struct
{ 
  WIFIBATTLEMATCH_RECORD_DATA *p_src;
  WIFIBATTLEMATCH_RECORD_DATA data;
  int my_poke_idx;
  int you_poke_idx;
} DEBUGWIN_SAKERECORD_DATA;

static DEBUGWIN_SAKERECORD_DATA s_debug_record_data = {0};

//-------------------------------------
///	プロフィール
//=====================================
static inline void DebugWin_SakeRec_U_GameSyncID( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU32( item, &p_wk->data.your_gamesyncID, 0, 0xFFFFFFFF );
}
static inline void DebugWin_SakeRec_D_GameSyncID( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "SyncID[%x]", p_wk->data.your_gamesyncID );  
}
static inline void DebugWin_SakeRec_U_ProfileID( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU32( item, &p_wk->data.your_profileID, 0, 0xFFFFFFFF );
}
static inline void DebugWin_SakeRec_D_ProfileID( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "proID[%x]", p_wk->data.your_profileID );  
}
//-------------------------------------
///	じぶんポケ
//=====================================
static inline void DebugWin_SakeRec_U_Mypoke_Page( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->my_poke_idx, 0, TEMOTI_POKEMAX );
}
static inline void DebugWin_SakeRec_D_MyPoke_page( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ポケ[%d]", p_wk->my_poke_idx );  
}
static inline void DebugWin_SakeRec_U_Mypoke_MonsNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU16( item, &p_wk->data.my_monsno[ p_wk->my_poke_idx ], 0, MONSNO_MAX );
}
static inline void DebugWin_SakeRec_D_MyPoke_MonNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ばんごう[%d]", p_wk->data.my_monsno[ p_wk->my_poke_idx ] );  
}

static inline void DebugWin_SakeRec_U_Mypoke_FormNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.my_form[ p_wk->my_poke_idx ], 0, 31 );
}
static inline void DebugWin_SakeRec_D_MyPoke_FormNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "フォルム[%d]", p_wk->data.my_form[ p_wk->my_poke_idx ] );  
}

static inline void DebugWin_SakeRec_U_Mypoke_Lv( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.my_lv[ p_wk->my_poke_idx ], 0, 31 );
}
static inline void DebugWin_SakeRec_D_MyPoke_Lv( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "LV[%d]", p_wk->data.my_lv[ p_wk->my_poke_idx ] );  
}

static inline void DebugWin_SakeRec_U_Mypoke_Sex( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.my_sex[ p_wk->my_poke_idx ], 0, 31 );
}
static inline void DebugWin_SakeRec_D_MyPoke_Sex( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "せいべつ[%d]", p_wk->data.my_sex[ p_wk->my_poke_idx ] );  
}

//-------------------------------------
///	あいてポケ
//=====================================
static inline void DebugWin_SakeRec_U_Youpoke_Page( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeData( item, &p_wk->you_poke_idx, 0, TEMOTI_POKEMAX );
}
static inline void DebugWin_SakeRec_D_YouPoke_page( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ポケ[%d]", p_wk->you_poke_idx );  
}
static inline void DebugWin_SakeRec_U_Youpoke_MonsNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU16( item, &p_wk->data.your_monsno[ p_wk->you_poke_idx ], 0, MONSNO_MAX );
}
static inline void DebugWin_SakeRec_D_YouPoke_MonNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ばんごう[%d]", p_wk->data.your_monsno[ p_wk->you_poke_idx ] );  
}

static inline void DebugWin_SakeRec_U_Youpoke_FormNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.your_form[ p_wk->you_poke_idx ], 0, 31 );
}
static inline void DebugWin_SakeRec_D_YouPoke_FormNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "フォルム[%d]", p_wk->data.your_form[ p_wk->you_poke_idx ] );  
}

static inline void DebugWin_SakeRec_U_Youpoke_Lv( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.your_lv[ p_wk->you_poke_idx ], 0, 31 );
}
static inline void DebugWin_SakeRec_D_YouPoke_Lv( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "LV[%d]", p_wk->data.your_lv[ p_wk->you_poke_idx ] );  
}

static inline void DebugWin_SakeRec_U_Youpoke_Sex( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.your_sex[ p_wk->you_poke_idx ], 0, 31 );
}
static inline void DebugWin_SakeRec_D_YouPoke_Sex( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "せいべつ[%d]", p_wk->data.your_sex[ p_wk->you_poke_idx ] );  
}

//-------------------------------------
///	対戦日
//=====================================
static inline void DebugWin_SakeRec_U_Date_Y( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8 year = p_wk->data.year;

  DebugWin_Util_ChangeDataU8( item, &year, 0, 99 );

  p_wk->data.year = year;
}
static inline void DebugWin_SakeRec_D_Date_Y( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ねん[%d]", p_wk->data.year );  
}

static inline void DebugWin_SakeRec_U_Date_M( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8 month = p_wk->data.month;

  DebugWin_Util_ChangeDataU8( item, &month, 1, 12 );

  p_wk->data.month = month;
}
static inline void DebugWin_SakeRec_D_Date_M( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "つき[%d]", p_wk->data.month );  
}
static inline void DebugWin_SakeRec_U_Date_D( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8 day = p_wk->data.day;

  DebugWin_Util_ChangeDataU8( item, &day, 1, 31 );

  p_wk->data.day = day;
}
static inline void DebugWin_SakeRec_D_Date_D( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ひ[%d]", p_wk->data.day );  
}
static inline void DebugWin_SakeRec_U_Date_H( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8 hour = p_wk->data.hour;

  DebugWin_Util_ChangeDataU8( item, &hour, 0, 24 );

  p_wk->data.hour = hour;
}
static inline void DebugWin_SakeRec_D_Date_H( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じ[%d]", p_wk->data.hour );  
}
static inline void DebugWin_SakeRec_U_Date_Mi( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8 minute = p_wk->data.minute;

  DebugWin_Util_ChangeDataU8( item, &minute, 0, 60 );

  p_wk->data.minute = minute;
}
static inline void DebugWin_SakeRec_D_Date_Mi( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "ふん[%d]", p_wk->data.minute );  
}

//-------------------------------------
///	結果
//=====================================
static inline void DebugWin_SakeRec_U_Result_CupNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU32( item, &p_wk->data.cupNO, 0, 0xFFFFFFFF );
}
static inline void DebugWin_SakeRec_D_Result_CupNo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "たいかいNO[%d]", p_wk->data.cupNO );  
}
static inline void DebugWin_SakeRec_U_Result_Result( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.result, 0, 0xFF );
}
static inline void DebugWin_SakeRec_D_Result_Result( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "けっか[%d]", p_wk->data.result );  
}
static inline void DebugWin_SakeRec_U_Result_BtlType( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU8( item, &p_wk->data.btl_type, 0, 0xFF );
}
static inline void DebugWin_SakeRec_D_Result_BtlType( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "タイプ[%d]", p_wk->data.btl_type );  
}
static inline void DebugWin_SakeRec_U_Result_RestMyPoke( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8  rest  = p_wk->data.rest_my_poke;
  DebugWin_Util_ChangeDataU8( item, &rest, 0, 0xFF );
  p_wk->data.rest_my_poke = rest;
}
static inline void DebugWin_SakeRec_D_Result_RestMyPoke( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じぶんのこり[%d]", p_wk->data.rest_my_poke );  
}
static inline void DebugWin_SakeRec_U_Result_RestYouPoke( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  u8  rest  = p_wk->data.rest_you_poke;
  DebugWin_Util_ChangeDataU8( item, &rest, 0, 0xFF );
  p_wk->data.rest_you_poke = rest;
}
static inline void DebugWin_SakeRec_D_Result_RestYouPoke( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "あいてのこり[%d]", p_wk->data.rest_you_poke );  
}


//-------------------------------------
///	反映
//=====================================
static inline void DebugWin_SakeRec_U_Get( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    p_wk->data  = *p_wk->p_src;
    DEBUGWIN_RefreshScreen();
  }
}
static inline void DebugWin_SakeRec_U_Set( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_SAKERECORD_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    *p_wk->p_src  = p_wk->data;
  }
}

//-------------------------------------
///	public
//=====================================
static inline void DEBUGWIN_SAKERECORD_Init( WIFIBATTLEMATCH_RECORD_DATA *p_record, HEAPID heapID )
{ 
  s_debug_record_data.p_src = p_record;
  s_debug_record_data.data = *p_record;

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_SAKE_RECORD, "サケレコード", heapID );
  DEBUGWIN_AddItemToGroup( "しゅとく", DebugWin_SakeRec_U_Get, &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD, heapID ); 
  DEBUGWIN_AddItemToGroup( "せってい", DebugWin_SakeRec_U_Set, &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD, heapID ); 

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_SAKE_RECORD_YOUPROFILE, "プロフィール", DEBUGWIN_GROUP_SAKE_RECORD, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_GameSyncID, DebugWin_SakeRec_D_GameSyncID,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPROFILE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_ProfileID, DebugWin_SakeRec_D_ProfileID,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPROFILE, heapID );

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE, "じぶんポケ", DEBUGWIN_GROUP_SAKE_RECORD, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Mypoke_Page, DebugWin_SakeRec_D_MyPoke_page,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Mypoke_MonsNo, DebugWin_SakeRec_D_MyPoke_MonNo,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Mypoke_FormNo, DebugWin_SakeRec_D_MyPoke_FormNo,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Mypoke_Lv, DebugWin_SakeRec_D_MyPoke_Lv,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Mypoke_Sex, DebugWin_SakeRec_D_MyPoke_Sex,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_MYPOKE, heapID );

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE, "あいてポケ", DEBUGWIN_GROUP_SAKE_RECORD, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Youpoke_Page, DebugWin_SakeRec_D_YouPoke_page,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Youpoke_MonsNo, DebugWin_SakeRec_D_YouPoke_MonNo,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Youpoke_FormNo, DebugWin_SakeRec_D_YouPoke_FormNo,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Youpoke_Lv, DebugWin_SakeRec_D_YouPoke_Lv,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Youpoke_Sex, DebugWin_SakeRec_D_YouPoke_Sex,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_YOUPOKE, heapID );

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_SAKE_RECORD_DATE, "たいせんび", DEBUGWIN_GROUP_SAKE_RECORD, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Date_Y, DebugWin_SakeRec_D_Date_Y,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Date_M, DebugWin_SakeRec_D_Date_M,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Date_D, DebugWin_SakeRec_D_Date_D,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Date_H, DebugWin_SakeRec_D_Date_H,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_DATE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Date_Mi, DebugWin_SakeRec_D_Date_Mi,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_DATE, heapID );

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_SAKE_RECORD_RESULT, "けっか", DEBUGWIN_GROUP_SAKE_RECORD, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Result_CupNo, DebugWin_SakeRec_D_Result_CupNo,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_RESULT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Result_Result, DebugWin_SakeRec_D_Result_Result,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_RESULT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Result_BtlType, DebugWin_SakeRec_D_Result_BtlType,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_RESULT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Result_RestMyPoke, DebugWin_SakeRec_D_Result_RestMyPoke,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_RESULT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_SakeRec_U_Result_RestYouPoke, DebugWin_SakeRec_D_Result_RestYouPoke,
       &s_debug_record_data, DEBUGWIN_GROUP_SAKE_RECORD_RESULT, heapID );


}
static inline void DEBUGWIN_SAKERECORD_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_SAKE_RECORD );
}
#endif //DEBUGWIN_SAKE_RECORD_DATA_USE


#ifdef DEBUGWIN_WIFISCORE_USE
#include "savedata/save_control.h"
#include "savedata/battlematch_savedata.h"
#include "savedata/rndmatch_savedata.h"

static int s_score_page = 0;

//=============================================================================
/**
 *    WIFISCORE
 */
//=============================================================================
static inline void DebugWin_WifiScore_U_Page( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DebugWin_Util_ChangeData( item, &s_score_page, 0, RNDMATCH_TYPE_MAX-1 );
}
static inline void DebugWin_WifiScore_D_Page( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *sc_tbl[] =
  { 
    "フリーシングル",
    "フリーダブル",
    "フリートリプル",
    "フリーローテ",
    "フリーシュータ",
    "レートシングル",
    "レートダブル",
    "レートトリプル",
    "レートローテ",
    "レートシュータ",
    "WIFIたいかい"
  };

  DEBUGWIN_ITEM_SetNameV( item , "タイプ[%s]", sc_tbl[ s_score_page ] );  
}


static inline void DebugWin_WifiScore_U_Win( void* userWork , DEBUGWIN_ITEM* item )
{ 
  RNDMATCH_DATA *p_sv = userWork;
  u16 param = RNDMATCH_GetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_WIN );

  DebugWin_Util_ChangeDataU16( item, &param, 0, 0xFFFF );

  RNDMATCH_SetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_WIN, param ); 
}

static inline void DebugWin_WifiScore_D_Win( void* userWork , DEBUGWIN_ITEM* item )
{ 
  RNDMATCH_DATA *p_sv = userWork;
  u16 param = RNDMATCH_GetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_WIN );

  DEBUGWIN_ITEM_SetNameV( item , "かち[%d]", param );  
}

static inline void DebugWin_WifiScore_U_Lose( void* userWork , DEBUGWIN_ITEM* item )
{ 
  RNDMATCH_DATA *p_sv = userWork;
  u16 param = RNDMATCH_GetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_LOSE );

  DebugWin_Util_ChangeDataU16( item, &param, 0, 0xFFFF );

  RNDMATCH_SetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_LOSE, param ); 
}

static inline void DebugWin_WifiScore_D_Lose( void* userWork , DEBUGWIN_ITEM* item )
{ 
  RNDMATCH_DATA *p_sv = userWork;
  u16 param = RNDMATCH_GetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_LOSE );

  DEBUGWIN_ITEM_SetNameV( item , "まけ[%d]", param );  
}

static inline void DebugWin_WifiScore_U_Rate( void* userWork , DEBUGWIN_ITEM* item )
{ 
  RNDMATCH_DATA *p_sv = userWork;
  u16 param = RNDMATCH_GetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_RATE );

  DebugWin_Util_ChangeDataU16( item, &param, 0, 0xFFFF );

  RNDMATCH_SetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_RATE, param ); 
}

static inline void DebugWin_WifiScore_D_Rate( void* userWork , DEBUGWIN_ITEM* item )
{ 
  RNDMATCH_DATA *p_sv = userWork;
  u16 param = RNDMATCH_GetParam( p_sv, s_score_page, RNDMATCH_PARAM_IDX_RATE );

  DEBUGWIN_ITEM_SetNameV( item , "レート[%d]", param );
}


static inline void DEBUGWIN_WIFISCORE_Init( HEAPID heapID )
{ 
  RNDMATCH_DATA *p_sv  = BATTLEMATCH_GetRndMatch( SaveData_GetBattleMatch(SaveControl_GetPointer()) );

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_WIFISCORE, "WiFIスコア", heapID );


  DEBUGWIN_AddItemToGroupEx( DebugWin_WifiScore_U_Page, DebugWin_WifiScore_D_Page,
       p_sv, DEBUGWIN_GROUP_WIFISCORE, heapID );

  DEBUGWIN_AddItemToGroupEx( DebugWin_WifiScore_U_Win, DebugWin_WifiScore_D_Win,
       p_sv, DEBUGWIN_GROUP_WIFISCORE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_WifiScore_U_Lose, DebugWin_WifiScore_D_Lose,
       p_sv, DEBUGWIN_GROUP_WIFISCORE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_WifiScore_U_Rate, DebugWin_WifiScore_D_Rate,
       p_sv, DEBUGWIN_GROUP_WIFISCORE, heapID );
}

static inline void DEBUGWIN_WIFISCORE_Exit( void )
{ 
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_WIFISCORE );

}

#endif // DEBUGWIN_WIFISCORE_USE


#ifdef DEBUGWIN_LIVESCORE_USE
#include "savedata/save_control.h"
#include "savedata/battlematch_savedata.h"
#include "savedata/livematch_savedata.h"

static int s_livescore_page = 0;

//=============================================================================
/**
 *    WIFISCORE
 */
//=============================================================================
static inline void DebugWin_LiveScore_U_MyMacAddr( void* userWork , DEBUGWIN_ITEM* item )
{ 
//  DebugWin_Util_ChangeData( item, &s_livescore_page, 0, LIVEMATCH_FOEDATA_MAX-1 );
}
static inline void DebugWin_LiveScore_D_MyMacAddr( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 mac_address[6];
  u32 hight, low;

  LIVEMATCH_DATA_GetMyMacAddr( p_sv, mac_address );

  DEBUGWIN_ITEM_SetNameV( item , "mac%2x-%2x-%2x-%2x-%2x-%2x", mac_address[0], mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);  
}
static inline void DebugWin_LiveScore_U_MyWin( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_WIN );
  DebugWin_Util_ChangeDataU8( item, &param, 0, 0xFF );
  LIVEMATCH_DATA_SetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_WIN, param );
}
static inline void DebugWin_LiveScore_D_MyWin( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_WIN );
  DEBUGWIN_ITEM_SetNameV( item , "かち[%d]", param );  
}
static inline void DebugWin_LiveScore_U_MyLose( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_LOSE );
  DebugWin_Util_ChangeDataU8( item, &param, 0, 0xFF );
  LIVEMATCH_DATA_SetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_LOSE, param );
}
static inline void DebugWin_LiveScore_D_MyLose( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_LOSE );
  DEBUGWIN_ITEM_SetNameV( item , "まけ[%d]", param );  
}
static inline void DebugWin_LiveScore_U_MyCnt( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_BTLCNT );
  DebugWin_Util_ChangeDataU8( item, &param, 0, 0xFF );
  LIVEMATCH_DATA_SetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_BTLCNT, param );
}
static inline void DebugWin_LiveScore_D_MyCnt( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetMyParam( p_sv, LIVEMATCH_MYDATA_PARAM_BTLCNT );
  DEBUGWIN_ITEM_SetNameV( item , "かいすう[%d]", param );  
}

static inline void DebugWin_LiveScore_U_FoePage( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DebugWin_Util_ChangeData( item, &s_livescore_page, 0, LIVEMATCH_FOEDATA_MAX-1 );
}
static inline void DebugWin_LiveScore_D_FoePage( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_ITEM_SetNameV( item , "たいせんしゃ[%d]", s_livescore_page );  
}
static inline void DebugWin_LiveScore_U_FoeMacAddr( void* userWork , DEBUGWIN_ITEM* item )
{ 
}
static inline void DebugWin_LiveScore_D_FoeMacAddr( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 mac_address[6];
  u32 hight, low;

  LIVEMATCH_DATA_GetFoeMacAddr( p_sv, s_livescore_page, mac_address );
  DEBUGWIN_ITEM_SetNameV( item , "mac%2x-%2x-%2x-%2x-%2x-%2x", mac_address[0], mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);  

}
static inline void DebugWin_LiveScore_U_FoePoke( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_POKE );
  DebugWin_Util_ChangeDataU8( item, &param, 0, 0xFF );
  LIVEMATCH_DATA_SetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_POKE, param );
}
static inline void DebugWin_LiveScore_D_FoePoke( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_POKE );
  DEBUGWIN_ITEM_SetNameV( item , "のこりポケ[%d]", param );  
}
static inline void DebugWin_LiveScore_U_FoeHp( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_HP );
  DebugWin_Util_ChangeDataU8( item, &param, 0, 0xFF );
  LIVEMATCH_DATA_SetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_HP, param );
}
static inline void DebugWin_LiveScore_D_FoeHp( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;
  u8 param = LIVEMATCH_DATA_GetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_HP );
  DEBUGWIN_ITEM_SetNameV( item , "HP[%d]", param );  
}

static inline void DebugWin_LiveScore_U_FoeDelete( void* userWork , DEBUGWIN_ITEM* item )
{ 
  LIVEMATCH_DATA *p_sv  = userWork;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    u8  mac_address[6];
    GFL_STD_MemClear( mac_address, sizeof(mac_address) );
    LIVEMATCH_DATA_SetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_HP, 0 );
    LIVEMATCH_DATA_SetFoeParam( p_sv, s_livescore_page, LIVEMATCH_FOEDATA_PARAM_REST_POKE, 0 );
    LIVEMATCH_DATA_SetFoeMacAddr( p_sv, s_livescore_page, mac_address );
    DEBUGWIN_RefreshScreen();
  }

}
static inline void DebugWin_LiveScore_D_FoeDelete( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_ITEM_SetName( item , "けす" );  
}

static inline void DEBUGWIN_LIVESCORE_Init( HEAPID heapID )
{ 
  LIVEMATCH_DATA *p_sv  = BATTLEMATCH_GetLiveMatch( SaveData_GetBattleMatch(SaveControl_GetPointer()) );

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_LIVESCORE, "LIVEスコア", heapID );

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_LIVESCORE_MY, "じぶんのスコア", DEBUGWIN_GROUP_LIVESCORE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_MyMacAddr, DebugWin_LiveScore_D_MyMacAddr,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_MY, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_MyWin, DebugWin_LiveScore_D_MyWin,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_MY, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_MyLose, DebugWin_LiveScore_D_MyLose,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_MY, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_MyCnt, DebugWin_LiveScore_D_MyCnt,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_MY, heapID );

  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GROUP_LIVESCORE_FOE, "たいせんあいて", DEBUGWIN_GROUP_LIVESCORE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_FoePage, DebugWin_LiveScore_D_FoePage,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_FOE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_FoeMacAddr, DebugWin_LiveScore_D_FoeMacAddr,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_FOE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_FoePoke, DebugWin_LiveScore_D_FoePoke,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_FOE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_FoeHp, DebugWin_LiveScore_D_FoeHp,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_FOE, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_LiveScore_U_FoeDelete, DebugWin_LiveScore_D_FoeDelete,
       p_sv, DEBUGWIN_GROUP_LIVESCORE_FOE, heapID );
}

static inline void DEBUGWIN_LIVESCORE_Exit( void )
{ 
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_LIVESCORE );

}

#endif // DEBUGWIN_LIVESCORE_USE

#ifdef DEBUGWIN_REPORT_USE
//=============================================================================
/**
 *    レポートを送信した情報を表示
 */
//=============================================================================

static inline void DebugWin_Report_U_None( void* userWork , DEBUGWIN_ITEM* item )
{
  /* なにもしない */
}

static inline void DebugWin_Report_D_MyWin( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じぶん かち[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_MY ][DEBUGWIN_REPORT_WIN] );  
}
static inline void DebugWin_Report_D_MyLose( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じぶん まけ[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_MY ][DEBUGWIN_REPORT_LOSE] );  
}
static inline void DebugWin_Report_D_MyDirty( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じぶん ふせい[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_MY ][DEBUGWIN_REPORT_DIRTY] );  
}
static inline void DebugWin_Report_D_MyDisconnect( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "じぶん せつだん[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_MY ][DEBUGWIN_REPORT_DISCONNECT] );  
}
static inline void DebugWin_Report_D_YouWin( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "あいて かち[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_YOU ][DEBUGWIN_REPORT_WIN] );  
}
static inline void DebugWin_Report_D_YouLose( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "あいて まけ[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_YOU ][DEBUGWIN_REPORT_LOSE] );  
}
static inline void DebugWin_Report_D_YouDirty( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "あいて ふせい[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_YOU ][DEBUGWIN_REPORT_DIRTY] );  
}
static inline void DebugWin_Report_D_YouDisconnect( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_REPORT_DATA  *p_wk = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "あいて せつだん[%d]", p_wk->param[ DEBUGWIN_REPORT_WHO_YOU ][DEBUGWIN_REPORT_DISCONNECT] );  
}
static inline void DEBUGWIN_REPORT_Init( HEAPID heapID )
{
  DEBUGWIN_REPORT_DATA  *p_wk = DEBUGWIN_REPORT_DATA_GetInstance();

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_REPORT, "レポートけっか", heapID );

  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_MyWin,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_MyLose,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_MyDirty,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_MyDisconnect,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_YouWin,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_YouLose,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_YouDirty,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_Report_U_None, DebugWin_Report_D_YouDisconnect,
       p_wk, DEBUGWIN_GROUP_REPORT, heapID );
}

static inline void DEBUGWIN_REPORT_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_REPORT );
}

static inline void DEBUGWIN_REPORT_SetData( BOOL is_my, int win, int lose, int dirty, int disconnect )
{
  DEBUGWIN_REPORT_DATA  *p_wk = DEBUGWIN_REPORT_DATA_GetInstance();

  p_wk->param[ !is_my ][ DEBUGWIN_REPORT_WIN ]  = win;
  p_wk->param[ !is_my ][ DEBUGWIN_REPORT_LOSE ]  = lose;
  p_wk->param[ !is_my ][ DEBUGWIN_REPORT_DIRTY ]  = dirty;
  p_wk->param[ !is_my ][ DEBUGWIN_REPORT_DISCONNECT ]  = disconnect;
}

#endif  //DEBUGWIN_REPORT_USE

#ifdef DEBUGWIN_BTLBOX_USE

typedef struct
{
  BATTLE_BOX_SAVE *p_sv;
  BATTLE_BOX_LOCK_BIT lock_bit;
} DEBUGWIN_BTLBOX_DATA;

static DEBUGWIN_BTLBOX_DATA s_btlbox_data = {0};

static inline void DEBUGWIN_BTLBOX_GetData( void )
{
  DEBUGWIN_BTLBOX_DATA  *p_wk = &s_btlbox_data;
  p_wk->lock_bit  = 0;
  if( BATTLE_BOX_SAVE_GetLockType( p_wk->p_sv, BATTLE_BOX_LOCK_BIT_WIFI ) )
  {
    p_wk->lock_bit  |= BATTLE_BOX_LOCK_BIT_WIFI;
  }
  if( BATTLE_BOX_SAVE_GetLockType( p_wk->p_sv, BATTLE_BOX_LOCK_BIT_LIVE ) )
  {
    p_wk->lock_bit  |= BATTLE_BOX_LOCK_BIT_LIVE;
  }
}

static inline void DEBUGWIN_BTLBOX_SetData( void )
{
  DEBUGWIN_BTLBOX_DATA  *p_wk = &s_btlbox_data;
  if( p_wk->lock_bit & BATTLE_BOX_LOCK_BIT_WIFI )
  {
    BATTLE_BOX_SAVE_OnLockFlg( p_wk->p_sv,BATTLE_BOX_LOCK_BIT_WIFI );
  }
  else
  {
    BATTLE_BOX_SAVE_OffLockFlg( p_wk->p_sv,BATTLE_BOX_LOCK_BIT_WIFI );
  }
  if( p_wk->lock_bit & BATTLE_BOX_LOCK_BIT_LIVE )
  {
    BATTLE_BOX_SAVE_OnLockFlg( p_wk->p_sv,BATTLE_BOX_LOCK_BIT_LIVE );
  }
  else
  {
    BATTLE_BOX_SAVE_OffLockFlg( p_wk->p_sv,BATTLE_BOX_LOCK_BIT_LIVE );
  }
}

static inline void DebugWin_BtlBox_U_Get( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBOX_DATA  *p_wk = &s_btlbox_data;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    DEBUGWIN_BTLBOX_GetData();
    DEBUGWIN_RefreshScreen();
  }
}

static inline void DebugWin_BtlBox_U_Set( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_BTLBOX_DATA  *p_wk = &s_btlbox_data;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  { 
    DEBUGWIN_BTLBOX_SetData();
  }
}

static inline void DebugWin_BtlBox_U_ChangeWifi( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBOX_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT || GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    if( p_wk->lock_bit & BATTLE_BOX_LOCK_BIT_WIFI )
    {
      //存在しているなら消す
      p_wk->lock_bit  &= ~BATTLE_BOX_LOCK_BIT_WIFI;
    }
    else
    {
      //ないならつける
      p_wk->lock_bit |= BATTLE_BOX_LOCK_BIT_WIFI;
    }

    DEBUGWIN_RefreshScreen();
  }
}
static inline void DebugWin_BtlBox_D_ChangeWifi( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *sc_tbl[] =
  {
    "OFF",
    "ON",
  };

  DEBUGWIN_BTLBOX_DATA  *p_wk = userWork;

  BOOL is_on  = ((p_wk->lock_bit  & BATTLE_BOX_LOCK_BIT_WIFI ) != 0 );
  DEBUGWIN_ITEM_SetNameV( item , "WIFIロック[%s]", sc_tbl[ is_on ] );
}

static inline void DebugWin_BtlBox_U_ChangeLive( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBOX_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT || GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    if( p_wk->lock_bit & BATTLE_BOX_LOCK_BIT_LIVE )
    {
      //存在しているなら消す
      p_wk->lock_bit  &= ~BATTLE_BOX_LOCK_BIT_LIVE;
    }
    else
    {
      //ないならつける
      p_wk->lock_bit |= BATTLE_BOX_LOCK_BIT_LIVE;
    }

    DEBUGWIN_RefreshScreen();
  }
}
static inline void DebugWin_BtlBox_D_ChangeLive( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *sc_tbl[] =
  {
    "OFF",
    "ON",
  };

  DEBUGWIN_BTLBOX_DATA  *p_wk = userWork;

  BOOL is_on  = ((p_wk->lock_bit  & BATTLE_BOX_LOCK_BIT_LIVE ) != 0 );
  DEBUGWIN_ITEM_SetNameV( item , "LIVEロック[%s]", sc_tbl[ is_on ] );
}
static inline void DEBUGWIN_BTLBOX_Init( HEAPID heapID )
{
  DEBUGWIN_BTLBOX_DATA  *p_wk = &s_btlbox_data;
  GFL_STD_MemClear( &s_btlbox_data, sizeof( DEBUGWIN_BTLBOX_DATA ) );
  p_wk->p_sv  = BATTLE_BOX_SAVE_GetBattleBoxSave(SaveControl_GetPointer());
  DEBUGWIN_BTLBOX_GetData();

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_BTLBOX, "バトルボックス", heapID );
  DEBUGWIN_AddItemToGroup( "しゅとく", DebugWin_BtlBox_U_Get, p_wk, DEBUGWIN_GROUP_BTLBOX, heapID ); 
  DEBUGWIN_AddItemToGroup( "せってい", DebugWin_BtlBox_U_Set, p_wk, DEBUGWIN_GROUP_BTLBOX, heapID ); 

  DEBUGWIN_AddItemToGroupEx( DebugWin_BtlBox_U_ChangeWifi, DebugWin_BtlBox_D_ChangeWifi,
      p_wk, DEBUGWIN_GROUP_BTLBOX, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_BtlBox_U_ChangeLive, DebugWin_BtlBox_D_ChangeLive,
      p_wk, DEBUGWIN_GROUP_BTLBOX, heapID );
}
static inline void DEBUGWIN_BTLBOX_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_BTLBOX );
}
#endif//DEBUGWIN_BTLBOX_USE

#ifdef DEBUGWIN_BTLBGM_USE


static inline void DebugWin_BtlBgm_U_ChageUse( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBGM_DATA  *p_wk = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT || GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    p_wk->is_use  ^= 1;
    DEBUGWIN_RefreshScreen();
  }
}
static inline void DebugWin_BtlBgm_D_ChangeUse( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *sc_tbl[] =
  {
    "OFF",
    "ON",
  };

  DEBUGWIN_BTLBGM_DATA  *p_wk = userWork;

  DEBUGWIN_ITEM_SetNameV( item , "BGMこてい[%s]", sc_tbl[ p_wk->is_use ] );
}

static inline void DebugWin_BtlBgm_U_BtlBgm( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBGM_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU16( item, &p_wk->btl_bgm, 0, 0xFFFF );
}

static inline void DebugWin_BtlBgm_D_BtlBgm( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBGM_DATA  *p_wk = userWork;

  DEBUGWIN_ITEM_SetNameV( item , "せんとうきょく[%d]", p_wk->btl_bgm );
}

static inline void DebugWin_BtlBgm_U_WinBgm( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBGM_DATA  *p_wk = userWork;
  DebugWin_Util_ChangeDataU16( item, &p_wk->win_bgm, 0, 0xFFFF );
}

static inline void DebugWin_BtlBgm_D_WinBgm( void* userWork , DEBUGWIN_ITEM* item )
{ 
  DEBUGWIN_BTLBGM_DATA  *p_wk = userWork;

  DEBUGWIN_ITEM_SetNameV( item , "かちきょく[%d]", p_wk->win_bgm );
}

static inline void DEBUGWIN_BTLBGM_Init( HEAPID heapID )
{
  DEBUGWIN_BTLBGM_DATA *p_wk  = DEBUGWIN_BTLBGM_DATA_GetInstance();

  DEBUGWIN_AddGroupToTop( DEBUGWIN_GROUP_BTLBGM, "バトルきょく", heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_BtlBgm_U_ChageUse, DebugWin_BtlBgm_D_ChangeUse, p_wk, DEBUGWIN_GROUP_BTLBGM, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_BtlBgm_U_BtlBgm, DebugWin_BtlBgm_D_BtlBgm, p_wk, DEBUGWIN_GROUP_BTLBGM, heapID );
  DEBUGWIN_AddItemToGroupEx( DebugWin_BtlBgm_U_WinBgm, DebugWin_BtlBgm_D_WinBgm, p_wk, DEBUGWIN_GROUP_BTLBGM, heapID );
}
static inline void DEBUGWIN_BTLBGM_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGWIN_GROUP_BTLBGM );
}
#endif //DEBUGWIN_BTLBGM_USE

#endif  //PM_DEBUG



#ifndef DEBUGWIN_SAKE_RECORD_DATA_USE
#define DEBUGWIN_SAKERECORD_Init( ... )  /*  */
#define DEBUGWIN_SAKERECORD_Exit( ... )  /*  */
#endif

#ifndef DEBUGWIN_WIFISCORE_USE
#define DEBUGWIN_WIFISCORE_Init( ... )  /*  */
#define DEBUGWIN_WIFISCORE_Exit( ... )  /*  */
#endif

#ifndef DEBUGWIN_REG_USE
#define DEBUGWIN_REG_Init( ... )  /*  */
#define DEBUGWIN_REG_Exit( ... )  /*  */
#endif

#ifndef DEBUGWIN_LIVESCORE_USE
#define DEBUGWIN_LIVESCORE_Init( ... )  /*  */
#define DEBUGWIN_LIVESCORE_Exit( ... )  /*  */
#endif

#ifndef DEBUGWIN_REPORT_USE
#define DEBUGWIN_REPORT_Init( ... ) /* */
#define DEBUGWIN_REPORT_Exit( ... ) /* */
#define DEBUGWIN_REPORT_SetData( ... ) /* */
#endif

#ifndef DEBUGWIN_BTLBOX_USE
#define DEBUGWIN_BTLBOX_Init( ... ) /* */
#define DEBUGWIN_BTLBOX_Exit( ... ) /* */
#endif//DEBUGWIN_BTLBOX_USE


#ifndef DEBUGWIN_BTLBGM_USE
#define DEBUGWIN_BTLBGM_Init( ... ) /* */
#define DEBUGWIN_BTLBGM_Exit( ... ) /* */
#endif //DEBUGWIN_BTLBGM_USE


