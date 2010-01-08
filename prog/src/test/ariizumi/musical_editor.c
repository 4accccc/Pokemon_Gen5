//======================================================================
/**
 * @file  dressup_system.h
 * @brief ミュージカルのエディタ
 * @author  ariizumi
 * @data  09/03/02
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "font/font.naix"
#include "mus_debug.naix"
#include "message.naix"
#include "msg/msg_mus_item_name.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "print/printsys.h"
#include "print/str_tool.h"
#include "print/global_msg.h"

#include "test/ariizumi/musical_editor.h"
#include "musical/musical_system.h"
#include "musical/musical_stage_sys.h"
#include "musical/mus_item_draw.h"

#include "musical/stage/script/sta_act_script_def.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_act_poke.h"
#include "musical/musical_camera_def.h"

#include "debug/gf_mcs.h"
#include "debug/debug_str_conv.h"
#include "test/ariizumi/ari_debug.h"

#if USE_MUSICAL_EDIT

//======================================================================
//  define
//======================================================================
#pragma mark [>define
#define HEAD_SIZE     ( 4 )

#define EDIT_FRAME_SUB_BG   GFL_BG_FRAME1_S

#define EDIT_PAL_FONT   (0xF)

#define MCS_READ_CH     ( 0 )
#define MCS_WRITE_CH    ( 0 )

#define EDIT_STATE_FIRST  (0xFE)
#define EDIT_STATE_NONE (0xFF)
#define EDIT_STATE_ON (0x01)

//======================================================================
//  enum
//======================================================================
#pragma mark [>enum
enum  //メインシーケンス
{
  STA_SEQ_INIT_SETTING, //設定画面
  STA_SEQ_LOOP_SETTING,
  STA_SEQ_TERM_SETTING,
  STA_SEQ_INIT_ACTING,  //ステージ演技部分
  STA_SEQ_LOOP_ACTING,
  STA_SEQ_TERM_ACTING,
};

//MCS用定義
enum
{
  SEND_SEQUENCE = 1,
  SEND_SEQUENCE_OK,
  SEND_RESOURCE,
  SEND_RESOURCE_OK,
  SEND_RECEIVE,
  SEND_RECEIVE_OK,
  SEND_DECIDE,
  SEND_CANCEL,
  SEND_IDLE,
};

//MCS用Seq
typedef enum
{
  MSEQ_WAIT,
  MSEQ_LOAD_SEQ,
  MSEQ_LOAD_RES,
  MSEQ_SEND,
}MSC_SEQ;

//ポケ画面Win
enum
{
  EDIT_POKEWIN_POKENAME,
  EDIT_POKEWIN_POS,
  EDIT_POKEWIN_EQUIP,
//  EDIT_POKEWIN_POSITION,

  EDIT_POKEWIN_MAX,
};
//サブWin
enum
{
  EDIT_SUBWIN_OBJNAME,
  EDIT_SUBWIN_LIGHT,

  EDIT_SUBWIN_MAX,
};
//======================================================================
//  typedef struct
//======================================================================
#pragma mark [>struct
typedef struct
{
  HEAPID        heapId;
  STAGE_INIT_WORK  *actInitWork;
  ACTING_WORK     *actWork;
  
  MSC_SEQ       mcsSeq;
  
  GFL_BMPWIN      **winArr;
  GFL_BMPWIN      **winArrSub;
  GFL_FONT      *fontHandle;
  GFL_MSGDATA     *itemMsgHandle;
  
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS     *bbdSys;

  //ScriptEdit時
  u8    dispEquipPos[4];
  u8    objNum;
  u8    befObjId;
  u8    lightNum;
  u8    befLightState;
  BOOL  isInitMcs;

  //Setting時
  u8    pokeNo;
  BOOL  isFinishSetting;
  BOOL  isItemAuto;
  BOOL  isPokeAnime;
  BOOL  isDrawEPos;
  
  MUS_POKE_DRAW_SYSTEM *pokeSys;
  MUS_POKE_DRAW_WORK   *pokeWork;
  MUS_ITEM_DRAW_SYSTEM  *itemDrawSys;
  MUS_ITEM_DRAW_WORK  *itemWork[MUS_POKE_EQUIP_MAX];
  GFL_G3D_RES     *itemRes[MUS_POKE_EQUIP_MAX];


}MUS_EDIT_LOCAL_WORK;

//======================================================================
//  proto
//======================================================================
#pragma mark [>prot
static GFL_PROC_RESULT MusicalEditProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalEditProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalEditProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

static void MusicalEdit_InitGraphic( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_TermGraphic( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_InitBgPokeEquip( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_TermBgPokeEquip( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_PrintFunc( MUS_EDIT_LOCAL_WORK *work , GFL_BMPWIN *win , STRBUF *str );
static void MusicalEdit_DrawPokeData( MUS_EDIT_LOCAL_WORK *work , const u8 pokeNo );
static void MusicalEdit_DrawObjData( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_DrawLightData( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_UpdatePokeData( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_UpdateNumberFunc( const s32 num , const u8 digit , const u8 x , const u8 y );
static void MusicalEdit_UpdateTouch( MUS_EDIT_LOCAL_WORK *work );

static void MusicalEdit_McsMain( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsWait( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsLoadSeq( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsLoadRes( MUS_EDIT_LOCAL_WORK *work );

static void MusicalSetting_Init( MUS_EDIT_LOCAL_WORK *work );
static const BOOL MusicalSetting_Main( MUS_EDIT_LOCAL_WORK *work );
static void MusicalSetting_Term( MUS_EDIT_LOCAL_WORK *work );

static void MusicalSetting_DrawPoke( MUS_EDIT_LOCAL_WORK *work );
static void MusicalSetting_LoadPoke( MUS_EDIT_LOCAL_WORK *work );
static void MusicalSetting_UnloadPoke( MUS_EDIT_LOCAL_WORK *work );
static void MusicalSetting_UpdatePoke( MUS_EDIT_LOCAL_WORK *work );
static void MusicalSetting_UpdateTouch( MUS_EDIT_LOCAL_WORK *work );

static void MusicalSetting_DrawEquipPos( MUS_EDIT_LOCAL_WORK *work );

GFL_PROC_DATA MusicalEdit_ProcData =
{
  MusicalEditProc_Init,
  MusicalEditProc_Main,
  MusicalEditProc_Term
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------

static GFL_PROC_RESULT MusicalEditProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  int ePos;
  MUS_EDIT_LOCAL_WORK *work;
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STAGE, 0xa0000 );

  work = GFL_PROC_AllocWork( proc, sizeof(MUS_EDIT_LOCAL_WORK), HEAPID_MUSICAL_STAGE );
  work->actInitWork = MUSICAL_STAGE_CreateStageWork( HEAPID_MUSICAL_STAGE , NULL );
  MUSICAL_STAGE_SetData_NPC( work->actInitWork , 0 , MONSNO_PIKUSII , HEAPID_MUSICAL_STAGE );
  MUSICAL_STAGE_SetData_NPC( work->actInitWork , 1 , MONSNO_RAITYUU , HEAPID_MUSICAL_STAGE );
  MUSICAL_STAGE_SetData_NPC( work->actInitWork , 2 , MONSNO_EREBUU  , HEAPID_MUSICAL_STAGE );
  MUSICAL_STAGE_SetData_NPC( work->actInitWork , 3 , MONSNO_RUKARIO , HEAPID_MUSICAL_STAGE );
  
  work->actInitWork->musPoke[0]->charaType = MUS_CHARA_PLAYER;


  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STRM|HEAPDIR_MASK, 0x80000 );
  work->actInitWork->distData = MUSICAL_SYSTEM_InitDistributeData( GFL_HEAPID_APP );
  MUSICAL_SYSTEM_LoadDistributeData( work->actInitWork->distData , 0 , HEAPID_MUSICAL_STRM );

  work->actInitWork->progWork = MUSICAL_PROGRAM_InitProgramData( HEAPID_MUSICAL_STAGE , work->actInitWork->distData );
  MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID_MUSICAL_STAGE , work->actInitWork->progWork , work->actInitWork );

  //mcs用初期化
  work->heapId = HEAPID_MUSICAL_STAGE;
  work->mcsSeq = MSEQ_WAIT;
  work->isInitMcs = FALSE;
  
  OS_TPrintf("Heap[%x]\n",GFL_HEAP_GetHeapFreeSize(GFL_HEAPID_APP));
  
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalEditProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_EDIT_LOCAL_WORK *work = mywk;
  
  MUSICAL_STAGE_DeleteStageWork( work->actInitWork );
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STAGE );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalEditProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUS_EDIT_LOCAL_WORK *work = mywk;
  switch( *seq )
  {
  case STA_SEQ_INIT_SETTING:
    MusicalSetting_Init( work );
    *seq = STA_SEQ_LOOP_SETTING;
    break;
    
  case STA_SEQ_LOOP_SETTING:
    if( MusicalSetting_Main( work ) == TRUE )
    {
      *seq = STA_SEQ_TERM_SETTING;
    }
    break;
    
  case STA_SEQ_TERM_SETTING:
    MusicalSetting_Term( work );
    *seq = STA_SEQ_INIT_ACTING;
    break;
    
  case STA_SEQ_INIT_ACTING:
    work->actWork = STA_ACT_InitActing( work->actInitWork , HEAPID_MUSICAL_STAGE );
    STA_ACT_EDITOR_SetEditorMode( work->actWork );
    //MusicalEdit_InitGraphic( work );
    //MusicalEdit_InitBgPokeEquip( work );
    *seq = STA_SEQ_LOOP_ACTING;
    break;
    
  case STA_SEQ_LOOP_ACTING:
    {
      ACTING_RETURN ret;
      ret = STA_ACT_LoopActing( work->actWork );
      MusicalEdit_McsMain( work );
      if( ret == ACT_RET_GO_END )
      {
        *seq = STA_SEQ_TERM_ACTING;
      }
      //MusicalEdit_UpdateTouch( work );
      //MusicalEdit_UpdatePokeData( work );
    }
    break;

  case STA_SEQ_TERM_ACTING:
    //MusicalEdit_TermBgPokeEquip( work );
    //MusicalEdit_TermGraphic( work );
    STA_ACT_TermActing( work->actWork );
    /*
    if( work->isInitMcs == TRUE )
    {
      MCS_Exit();
    }
    */
    *seq = STA_SEQ_INIT_ACTING;
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

#pragma mark [>bg func
static void MusicalEdit_InitGraphic( MUS_EDIT_LOCAL_WORK *work )
{
  GFL_ARC_UTIL_TransVramPalette( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_NCLR , 
                    PALTYPE_SUB_BG , 0 , sizeof(u16)*16 , work->heapId );
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_NCGR ,
                    EDIT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_FONTSYS_SetColor( 1,0,0 );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , EDIT_PAL_FONT*0x20, 16*sizeof(u16), work->heapId );

  //メッセージ
  work->itemMsgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_mus_item_name_dat , work->heapId );

}

static void MusicalEdit_TermGraphic( MUS_EDIT_LOCAL_WORK *work )
{
  GFL_MSG_Delete( work->itemMsgHandle );
  GFL_FONTSYS_SetDefaultColor();
  GFL_FONT_Delete( work->fontHandle );
}

static void MusicalEdit_InitBgPokeEquip( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  GFL_ARC_UTIL_TransVramScreen( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_poke_NSCR , 
                    EDIT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  //PokeWin
  work->winArr = GFL_HEAP_AllocMemory( work->heapId , sizeof(GFL_BMPWIN*)*EDIT_POKEWIN_MAX*4 );
  for(i=0;i<4;i++)
  {
    u8 baseNo = EDIT_POKEWIN_MAX*i;
    work->winArr[ baseNo+EDIT_POKEWIN_POKENAME ] = 
          GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
              1+i*8 , 3 , 6 , 1 , 
              EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
    work->winArr[ baseNo+EDIT_POKEWIN_POS ] = 
          GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
              1+i*8 , 5 , 4 , 1 , 
              EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
    work->winArr[ baseNo+EDIT_POKEWIN_EQUIP ] = 
          GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
              1+i*8 , 6 , 6 , 1 , 
              EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
  }
  
  for(i=0;i<4*EDIT_POKEWIN_MAX;i++)
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArr[ i ] ) , 0xF );
    GFL_BMPWIN_MakeScreen( work->winArr[ i ] );
  }

  for(i=0;i<4;i++)
  {
    work->dispEquipPos[i] = 0;
    MusicalEdit_DrawPokeData( work , i );
  }

  //SubWin
  work->winArrSub = GFL_HEAP_AllocMemory( work->heapId , sizeof(GFL_BMPWIN*)*EDIT_SUBWIN_MAX );
  work->winArrSub[ EDIT_SUBWIN_OBJNAME ] = 
        GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 9 , 16 , 6 , 1 , 
        EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
  work->winArrSub[ EDIT_SUBWIN_LIGHT ] = 
        GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG ,17 , 16 , 6 , 1 , 
        EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
  
  for(i=0;i<EDIT_SUBWIN_MAX;i++)
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArrSub[ i ] ) , 0xF );
    GFL_BMPWIN_MakeScreen( work->winArrSub[ i ] );
  }

  work->objNum = 0;
  work->lightNum = 0;
  work->befObjId = EDIT_STATE_FIRST;
  work->befLightState = EDIT_STATE_FIRST;
  MusicalEdit_DrawObjData( work );
  MusicalEdit_DrawLightData( work );
  GFL_BG_LoadScreenV_Req( EDIT_FRAME_SUB_BG );
  
}

static void MusicalEdit_TermBgPokeEquip( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  for(i=0;i<EDIT_SUBWIN_MAX;i++)
  {
    GFL_BMPWIN_Delete( work->winArrSub[ i ] );
  }
  GFL_HEAP_FreeMemory( work->winArrSub );
  
  for(i=0;i<4*EDIT_POKEWIN_MAX;i++)
  {
    GFL_BMPWIN_Delete( work->winArr[ i ] );
  }
  GFL_HEAP_FreeMemory( work->winArr );
}

static void MusicalEdit_PrintFunc( MUS_EDIT_LOCAL_WORK *work , GFL_BMPWIN *win , STRBUF *str )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( win ) , 0xF );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(win) , 1 , 1 , str , work->fontHandle );
  GFL_BMPWIN_TransVramCharacter( win );
}

static void MusicalEdit_DrawPokeData( MUS_EDIT_LOCAL_WORK *work , const u8 pokeNo )
{
  u8 i;
  GFL_BMPWIN **baseWin = &work->winArr[pokeNo*EDIT_POKEWIN_MAX];
  STRBUF  *str = GFL_STR_CreateBuffer( 32 , work->heapId );
  STRCODE posName[MUS_POKE_EQUIP_MAX][20] =
  {
    { L'H',L'A',L'R',L'_',L'R',0xFFFF },
    { L'H',L'A',L'R',L'_',L'L',0xFFFF },
    { L'H',L'E',L'A',L'D',0xFFFF },
    { L'E',L'Y',L'E',0xFFFF },
    { L'F',L'A',L'C',L'E',0xFFFF },
    { L'B',L'O',L'D',L'Y',0xFFFF },
    { L'W',L'A',L'I',L'S',L'T',0xFFFF },
    { L'H',L'A',L'N',L'D',L'_',L'R',0xFFFF },
    { L'H',L'A',L'N',L'D',L'_',L'L',0xFFFF },
  };
  MUSICAL_POKE_PARAM *musPoke = work->actInitWork->musPoke[pokeNo];

  u8 Col[3];
  GFL_FONTSYS_GetColor( &Col[0] , &Col[1] , &Col[2] );
  GFL_FONTSYS_SetColor( 1 , 0 , 0 );

  //名前
  GFL_MSG_GetString( GlobalMsg_PokeName , musPoke->mcssParam.monsno , str );
  MusicalEdit_PrintFunc( work , baseWin[EDIT_POKEWIN_POKENAME] , str );
  GFL_STR_ClearBuffer( str );
  
  //装備箇所
  GFL_STR_SetStringCode(str,posName[work->dispEquipPos[pokeNo]]);
  MusicalEdit_PrintFunc( work , baseWin[EDIT_POKEWIN_POS] , str );
  GFL_STR_ClearBuffer( str );
  
  //装備品
  {
    const u16 itemNo = musPoke->equip[ work->dispEquipPos[pokeNo] ].itemNo;
    if( itemNo == MUSICAL_ITEM_INVALID )
    {
      GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_NONE , str );
    }
    else
    {
      GFL_MSG_GetString( work->itemMsgHandle , MUS_ITEM_DRAW_GetArcIdx( itemNo )+ITEM_NAME_000 , str );
    }
    
    MusicalEdit_PrintFunc( work , baseWin[EDIT_POKEWIN_EQUIP] , str );
    GFL_STR_ClearBuffer( str );
  }

  GFL_STR_DeleteBuffer( str );
  GFL_FONTSYS_SetColor( Col[0] , Col[1] , Col[2] );
}

static void MusicalEdit_DrawObjData( MUS_EDIT_LOCAL_WORK *work )
{
  STRBUF  *str = GFL_STR_CreateBuffer( 32 , work->heapId );
  STA_OBJ_SYS  *objSys  = STA_ACT_GetObjectSys( work->actWork );
  STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , work->objNum );
  
  u8 Col[3];
  GFL_FONTSYS_GetColor( &Col[0] , &Col[1] , &Col[2] );
  GFL_FONTSYS_SetColor( 1 , 0 , 0 );

  if( objWork == NULL )
  {
    GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_NONE , str );
    work->befObjId = EDIT_STATE_NONE;
  }
  else
  {
    GFL_MSG_GetString( work->itemMsgHandle , STA_OBJ_GetObjNo(objSys,objWork)+OBJ_NAME_000 , str );
    work->befObjId = STA_OBJ_GetObjNo(objSys,objWork);
  }
  
  MusicalEdit_PrintFunc( work , work->winArrSub[EDIT_SUBWIN_OBJNAME] , str );

  GFL_STR_DeleteBuffer( str );
  
  MusicalEdit_UpdateNumberFunc( work->objNum , 1 , 14 , 15 );
  GFL_FONTSYS_SetColor( Col[0] , Col[1] , Col[2] );
  
}

static void MusicalEdit_DrawLightData( MUS_EDIT_LOCAL_WORK *work )
{
  STRBUF  *str = GFL_STR_CreateBuffer( 32 , work->heapId );
  STA_LIGHT_SYS  *lightSys  = STA_ACT_GetLightSys( work->actWork );
  STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , work->lightNum );

  u8 Col[3];
  GFL_FONTSYS_GetColor( &Col[0] , &Col[1] , &Col[2] );
  GFL_FONTSYS_SetColor( 1 , 0 , 0 );

  if( lightWork == NULL )
  {
    GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_OFF , str );
    work->befLightState = EDIT_STATE_NONE;
  }
  else
  {
    GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_ON  , str );
    work->befLightState = EDIT_STATE_ON;
  }
  
  MusicalEdit_PrintFunc( work , work->winArrSub[EDIT_SUBWIN_LIGHT] , str );

  GFL_STR_DeleteBuffer( str );

  MusicalEdit_UpdateNumberFunc( work->lightNum , 1 , 22 , 15 );
  GFL_FONTSYS_SetColor( Col[0] , Col[1] , Col[2] );
}

static void MusicalEdit_UpdatePokeData( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  STA_POKE_SYS *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  //ポケ座標
  for( i=0;i<4;i++ )
  {
//    GFL_BMPWIN *win = work->winArr[i*EDIT_POKEWIN_WINMAX+EDIT_POKEWIN_POSITION];
    STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , i );
    //座標
    {
      VecFx32 pos;
      STA_POKE_GetPosition( pokeSys , pokeWork , &pos );
      MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.x) , 6 , 6+i*8 , 10 );
      MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.y) , 6 , 6+i*8 , 11 );
      MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.z) , 6 , 6+i*8 , 12 );
    }
  }
  
  //ステージ・カーテン
  MusicalEdit_UpdateNumberFunc( STA_ACT_GetStageScroll(work->actWork) , 6 , 6 , 16 );
  MusicalEdit_UpdateNumberFunc( STA_ACT_GetCurtainHeight(work->actWork) , 6 , 6 , 18 );

  //オブジェ
  {
    STA_OBJ_SYS  *objSys  = STA_ACT_GetObjectSys( work->actWork );
    STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , work->objNum );
    VecFx32 pos;
    if( objWork == NULL )
    {
      VEC_Set( &pos,0,0,0 );
    }
    else
    {
      STA_OBJ_GetPosition( objSys , objWork , &pos );
    }
    MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.x) , 6 , 14 , 20 );
    MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.y) , 6 , 14 , 21 );
    MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.z) , 6 , 14 , 22 );
    
  }

  //ライト
  {
    STA_LIGHT_SYS  *lightSys  = STA_ACT_GetLightSys( work->actWork );
    STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , work->lightNum );
    VecFx32 pos;
    if( lightWork == NULL )
    {
      VEC_Set( &pos,0,0,0 );
    }
    else
    {
      STA_LIGHT_GetPosition( lightSys , lightWork , &pos );
    }
    MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.x) , 6 , 22 , 20 );
    MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.y) , 6 , 22 , 21 );
    MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.z) , 6 , 22 , 22 );
  }

  GFL_BG_LoadScreenV_Req( EDIT_FRAME_SUB_BG );
}
static void MusicalEdit_UpdateNumberFunc( const s32 num , const u8 digit , const u8 x , const u8 y )
{
  const BOOL isMinus = (num<0?TRUE:FALSE);
  u32 tempNum = MATH_ABS(num);
  u16 nowDigit = 0;
  
  while( tempNum != 0 || nowDigit == 0 )
  {
    u16 chrNo = tempNum%10 + 0x02;
    GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , x-nowDigit,y,1,1);
    nowDigit++;
    tempNum/=10;
  }
  if( isMinus == TRUE )
  {
    u16 chrNo = 0x0C;
    GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , x-nowDigit,y,1,1);
    nowDigit++;
  }
  while( nowDigit < digit )
  {
    u16 chrNo = 0x01;
    GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , x-nowDigit,y,1,1);
    nowDigit++;
  }
  
}

static void MusicalEdit_UpdateTouch( MUS_EDIT_LOCAL_WORK *work )
{
  static const GFL_UI_TP_HITTBL hitTbl[24+1] =
  {
    { 56 , 68 ,  8 , 20 },
    { 56 , 68 , 20 , 32 },
    { 56 , 68 , 32 , 44 },
    { 56 , 68 , 44 , 56 },

    { 56 , 68 , 72 , 84 },
    { 56 , 68 , 84 , 96 },
    { 56 , 68 , 96 ,108 },
    { 56 , 68 ,108 ,120 },

    { 56 , 68 ,136 ,148 },
    { 56 , 68 ,148 ,150 },
    { 56 , 68 ,150 ,162 },
    { 56 , 68 ,162 ,174 },

    { 56 , 68 ,200 ,212 },
    { 56 , 68 ,212 ,224 },
    { 56 , 68 ,224 ,236 },
    { 56 , 68 ,236 ,248 },

    {136 ,148 , 72 , 84 },
    {136 ,148 , 84 , 96 },
    {136 ,148 , 96 ,108 },
    {136 ,148 ,108 ,120 },

    {136 ,148 ,136 ,148 },
    {136 ,148 ,148 ,150 },
    {136 ,148 ,150 ,162 },
    {136 ,148 ,162 ,174 },
    
    { GFL_UI_TP_HIT_END },
  };
  
  const int ret = GFL_UI_TP_HitTrg( hitTbl );
  
  if( ret != GFL_UI_TP_HIT_NONE )
  {
    const u8 type = ret/4;
    const s8 valueArr[4] = {-5,-1,1,5};
    s8 value = valueArr[ ret%4 ];
    
    if( type < 4 )
    {
      if( work->dispEquipPos[type] + value < 0 )
      {
        work->dispEquipPos[type] = (work->dispEquipPos[type]+MUS_POKE_EQUIP_MAX+value)%MUS_POKE_EQUIP_MAX;
      }
      else
      {
        work->dispEquipPos[type] = (work->dispEquipPos[type]+value)%MUS_POKE_EQUIP_MAX;
      }
      MusicalEdit_DrawPokeData( work , type );
    }
    if( type == 4 )
    {
      OS_TPrintf("[%d]+[%d]->",work->objNum,value);
      if( work->objNum + value < 0 )
      {
        work->objNum = (work->objNum+ACT_OBJECT_MAX+value)%ACT_OBJECT_MAX;
      }
      else
      {
        work->objNum = (work->objNum+value)%ACT_OBJECT_MAX;
      }
      OS_TPrintf("[%d]\n",work->objNum);
      MusicalEdit_DrawObjData( work );
    }
    if( type == 5 )
    {
      if( work->lightNum + value < 0 )
      {
        work->lightNum = (work->lightNum+ACT_LIGHT_MAX+value)%ACT_LIGHT_MAX;
      }
      else
      {
        work->lightNum = (work->lightNum+value)%ACT_LIGHT_MAX;
      }
      MusicalEdit_DrawLightData( work );
    }
  }
}

#pragma mark [>mcs func


static void MusicalEdit_McsMain( MUS_EDIT_LOCAL_WORK *work )
{
  u32 size;
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
  {
    if( work->isInitMcs == FALSE )
    {
      work->isInitMcs = !MCS_Init( work->heapId );
    }
    
  }
  if( work->isInitMcs == FALSE )
  {
    return;
  }
  
  MCS_Main();

  switch(work->mcsSeq)
  {
  case MSEQ_WAIT:
    MusicalEdit_McsWait( work );
    break;
  case MSEQ_LOAD_SEQ:
    MusicalEdit_McsLoadSeq( work );
    break;
  case MSEQ_LOAD_RES:
  case MSEQ_SEND:
    MusicalEdit_McsLoadRes( work );
    break;
  }

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    STA_ACT_EDITOR_StartScript( work->actWork );
  }
}

static void MusicalEdit_McsWait( MUS_EDIT_LOCAL_WORK *work )
{
  const u32 size = MCS_CheckRead();
  if( size > 0 )
  {
    u32 head;
    MCS_Read( &head, HEAD_SIZE );
    if( head == SEND_SEQUENCE )
    {
      work->mcsSeq = MSEQ_LOAD_SEQ;
      head = SEND_SEQUENCE_OK;
      MCS_Write( MCS_WRITE_CH, &head, 4 );
      OS_TPrintf("sequence_data head:%d\n",size);
    }
    else if( head == SEND_RESOURCE )
    {
      work->mcsSeq = MSEQ_LOAD_RES;
      head = SEND_RESOURCE_OK;
      MCS_Write( MCS_WRITE_CH, &head, 4 );
      OS_TPrintf("resource_data head:%d\n",size);
    }
    else if( head == SEND_RECEIVE )
    {
      work->mcsSeq = MSEQ_SEND;
//        evw->sub_seq_no = SUB_SEQ_INIT;
//        evw->answer = 0;
      head = SEND_RECEIVE_OK;
      MCS_Write( MCS_WRITE_CH, &head, 4 );
      OS_TPrintf("receive head:%d\n",size);
    }
  }
}
static void MusicalEdit_McsLoadSeq( MUS_EDIT_LOCAL_WORK *work )
{
  const u32 size = MCS_CheckRead();
  if( size )
  {
    u32 head;
    void *scriptData = GFL_HEAP_AllocClearMemory( work->heapId, size );
    MCS_Read( scriptData, size );
    STA_ACT_EDITOR_SetScript( work->actWork , scriptData );
    head = SEND_IDLE;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
    work->mcsSeq = MSEQ_WAIT;
    OS_TPrintf("sequence_data load:%d\n",size);
    
  }
  
}

static void MusicalEdit_McsLoadRes( MUS_EDIT_LOCAL_WORK *work )
{
  //ダミー読み込み
  const u32 size = MCS_CheckRead();

  if( size )
  {
    u32 head;
    void* data;
    data = GFL_HEAP_AllocClearMemory( work->heapId, size );
    MCS_Read( data, size );
    head = SEND_IDLE;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
    work->mcsSeq = MSEQ_WAIT;

    GFL_HEAP_FreeMemory( data );
    OS_TPrintf("dummy load:%d\n",size);
  }
}

#pragma mark [>editor func

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_D,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_23_G,     // メイン2DエンジンのBG拡張パレット
//  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_16_F,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_NONE,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_01_AB,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_0123_E,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};

static void MusicalSetting_Init( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  
  work->pokeNo = 0;
  work->isFinishSetting = FALSE;
  work->isItemAuto = TRUE;
  work->isPokeAnime = FALSE;
  work->isDrawEPos = FALSE;
  work->pokeWork = NULL;
  
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );

  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  GFL_DISP_SetBank( &vramBank );
  GFL_BG_Init( work->heapId );
  GFL_BMPWIN_Init( work->heapId );

  //Vram割り当ての設定
  {
    static const GFL_BG_SYS_HEADER sys_data = {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    // BG1 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );
    GFL_BG_SetBGControl3D( 1 );
    GFL_BG_SetBGControl( EDIT_FRAME_SUB_BG, &header_sub1, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( EDIT_FRAME_SUB_BG, VISIBLE_ON );
    GFL_BG_ClearFrame( EDIT_FRAME_SUB_BG );
    GFL_BG_LoadScreenReq( EDIT_FRAME_SUB_BG );
    GFL_BG_SetVisible( GFL_BG_FRAME0_M , TRUE );
  }
  
  { //3D系の設定
    static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
    static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
            0, work->heapId, NULL );
    GFL_BG_SetBGControl3D( 2 ); //NNS_g3dInitの中で表示優先順位変わる・・・
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
    //正射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
                       FX32_ONE*12.0f,
                       0,
                       0,
                       FX32_ONE*16.0f,
                       MUSICAL_CAMERA_NEAR,
                       MUSICAL_CAMERA_FAR,
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
    
    GFL_G3D_CAMERA_Switching( work->camera );
    //エッジマーキングカラーセット
    G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( FALSE );
    G3X_AlphaBlend( TRUE );
    
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
  } 
  {
    GFL_BBD_SETUP bbdSetup = {
      128,128,
      {FX32_ONE,FX32_ONE,FX32_ONE},
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      ACT_POLYID_SHADOW, //影用の
			GFL_BBD_ORIGIN_CENTER,
    };
    VecFx32 scale ={FX32_ONE*4,FX32_ONE*4,FX32_ONE};
    //ビルボードシステム構築
    work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
    //背景のために全体を４倍する
    GFL_BBD_SetScale( work->bbdSys , &scale );
  }
  
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_FONTSYS_SetColor( 1,0,0 );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , EDIT_PAL_FONT*0x20, 16*sizeof(u16), work->heapId );

  //メッセージ
  work->itemMsgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_mus_item_name_dat , work->heapId );

  //システム
  work->pokeSys = MUS_POKE_DRAW_InitSystem( work->heapId );
  work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUS_POKE_EQUIP_MAX , work->heapId );

  //BG
  GFL_ARC_UTIL_TransVramPalette( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_NCLR , 
                    PALTYPE_SUB_BG , 0 , sizeof(u16)*16*2 , work->heapId );
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_NCGR ,
                    EDIT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARC_UTIL_TransVramScreen( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_setting_NSCR , 
                    EDIT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  //PokeWin
  work->winArr = GFL_HEAP_AllocMemory( work->heapId , sizeof(GFL_BMPWIN*) );
  work->winArr[ 0 ] = 
        GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 1 , 2 , 6 , 1 , 
            EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArr[ 0 ] ) , 0xF );
  GFL_BMPWIN_MakeScreen( work->winArr[ 0 ] );

  //EquipWin
  work->winArrSub = GFL_HEAP_AllocMemory( work->heapId , sizeof(GFL_BMPWIN*)*EDIT_POKEWIN_MAX*9 );
  for(i=0;i<9;i++)
  {
    work->winArrSub[ i ] = 
          GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
              16 , 5+i*2 , 6 , 1 , 
              EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
  }
  
  for(i=0;i<9;i++)
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArrSub[ i ] ) , 0xF );
    GFL_BMPWIN_MakeScreen( work->winArrSub[ i ] );
  }
  GFL_BG_LoadScreenV_Req( EDIT_FRAME_SUB_BG );
  
  MusicalSetting_DrawPoke( work );
  MusicalSetting_LoadPoke( work );
}

static void MusicalSetting_Term( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  for(i=0;i<9;i++)
  {
    GFL_BMPWIN_Delete( work->winArrSub[ i ] );
  }
  GFL_HEAP_FreeMemory( work->winArrSub );

  GFL_BMPWIN_Delete( work->winArr[ 0 ] );
  GFL_HEAP_FreeMemory( work->winArr );
  
  MusicalSetting_UnloadPoke( work );

  MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );
  MUS_POKE_DRAW_TermSystem( work->pokeSys );

  GFL_MSG_Delete( work->itemMsgHandle );
  GFL_FONTSYS_SetDefaultColor();
  GFL_FONT_Delete( work->fontHandle );

  GFL_BBD_DeleteSys( work->bbdSys );
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();

  GFL_BG_FreeBGControl( EDIT_FRAME_SUB_BG );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

static const BOOL MusicalSetting_Main( MUS_EDIT_LOCAL_WORK *work )
{
  MusicalSetting_UpdateTouch( work );
  MusicalSetting_UpdatePoke( work );
  
  MUS_POKE_DRAW_UpdateSystem( work->pokeSys );
  MUS_ITEM_DRAW_UpdateSystem( work->itemDrawSys ); 

  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    MUS_POKE_DRAW_DrawSystem( work->pokeSys ); 
    GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
    if( work->isDrawEPos == TRUE )
    {
      MusicalSetting_DrawEquipPos( work );
    }
  }
  GFL_G3D_DRAW_End();

  return  work->isFinishSetting;
;
}

static void MusicalSetting_DrawPoke( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  STRBUF  *str = GFL_STR_CreateBuffer( 32 , work->heapId );

  MUSICAL_POKE_PARAM *musPoke = work->actInitWork->musPoke[work->pokeNo];

  GFL_FONTSYS_SetColor( 1 , 0 , 0 );

  //番号
  MusicalEdit_UpdateNumberFunc( work->pokeNo , 1 , 5 , 1 );

  //名前
  GFL_MSG_GetString( GlobalMsg_PokeName , musPoke->mcssParam.monsno , str );
  MusicalEdit_PrintFunc( work , work->winArr[0] , str );
  GFL_STR_ClearBuffer( str );

  //装備品
  for( i=0;i<9;i++ )
  {
    const u16 itemNo = musPoke->equip[ i ].itemNo;
    if( itemNo == MUSICAL_ITEM_INVALID )
    {
      GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_NONE , str );
    }
    else
    {
      GFL_MSG_GetString( work->itemMsgHandle , MUS_ITEM_DRAW_GetArcIdx( itemNo )+ITEM_NAME_000 , str );
    }
    
    MusicalEdit_PrintFunc( work , work->winArrSub[i] , str );
    GFL_STR_ClearBuffer( str );
  }

  GFL_STR_DeleteBuffer( str );
  
  //ON/OFF切り替え
  //AUTO
  GFL_BG_ChangeScreenPalette( EDIT_FRAME_SUB_BG, 16,1,3,1, ( work->isItemAuto==TRUE ? 0 : 1  ) );
  //ANIME
  GFL_BG_ChangeScreenPalette( EDIT_FRAME_SUB_BG, 12,1,3,1, ( work->isPokeAnime==TRUE ? 0 : 1  ) );
  //EPOS
  GFL_BG_ChangeScreenPalette( EDIT_FRAME_SUB_BG,  8,3,3,1, ( work->isDrawEPos==TRUE ? 0 : 1  ) );
}

static void MusicalSetting_LoadPoke( MUS_EDIT_LOCAL_WORK *work )
{
  u8 ePos;  //equipPos
  MUSICAL_POKE_PARAM *musPoke = work->actInitWork->musPoke[work->pokeNo];
  VecFx32 pos = {MUSICAL_POS_X( 128.0f ),MUSICAL_POS_Y( 112.0f ),FX32_CONST(40.0f)};
  if( work->pokeWork == NULL )
  {
    work->pokeWork = MUS_POKE_DRAW_Add( work->pokeSys , musPoke , TRUE );
    MUS_POKE_DRAW_SetPosition( work->pokeWork , &pos );
    MUS_POKE_DRAW_SetShowFlg( work->pokeWork , TRUE );
  }

  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    const u16 itemNo = musPoke->equip[ePos].itemNo;
    if( itemNo != MUSICAL_ITEM_INVALID )
    {
      work->itemRes[ePos] = MUS_ITEM_DRAW_LoadResource( itemNo );
      work->itemWork[ePos] = MUS_ITEM_DRAW_AddResource( work->itemDrawSys , itemNo , work->itemRes[ePos] , &pos );
      MUS_ITEM_DRAW_SetSize( work->itemDrawSys , work->itemWork[ePos] , FX16_CONST(0.25f) , FX16_CONST(0.25f) );
    }
    else
    {
      work->itemRes[ePos] = NULL;
      work->itemWork[ePos] = NULL;
    }
  }
  if( work->isPokeAnime == TRUE )
  {
    MUS_POKE_DRAW_StartAnime( work->pokeWork );
  }
  else
  {
    MUS_POKE_DRAW_StopAnime( work->pokeWork );
  }
}

static void MusicalSetting_UnloadPoke( MUS_EDIT_LOCAL_WORK *work )
{
  u8 i;
  if( work->pokeWork != NULL )
  {
    MUS_POKE_DRAW_Del( work->pokeSys , work->pokeWork );
    work->pokeWork = NULL;
  }
  for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
  {
    if( work->itemRes[i] != NULL )
    {
      MUS_ITEM_DRAW_DelItem( work->itemDrawSys , work->itemWork[i] );
      MUS_ITEM_DRAW_DeleteResource( work->itemRes[i] );
    }
  }
}

static void MusicalSetting_UpdatePoke( MUS_EDIT_LOCAL_WORK *work )
{
  int ePos;
  VecFx32 pos;
  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->pokeWork , ePos );
    if( equipData->isEnable == TRUE )
    {
      const u16 itemRot = equipData->itemRot;
      const u16 rotZ = 0x10000-equipData->rot;

      MtxFx33 rotWork;
      VecFx32 rotOfs;
      VecFx32 ofs;
      MTX_RotZ33( &rotWork , -FX_SinIdx( rotZ ) , FX_CosIdx( rotZ ) );
      MTX_MultVec33( &equipData->ofs , &rotWork , &ofs );
      
      {
        MTX_MultVec33( &equipData->rotOfs , &rotWork , &rotOfs );
        VEC_Subtract( &equipData->rotOfs , &rotOfs , &rotOfs );
      }

      MusicalEdit_UpdateNumberFunc( F32_CONST(equipData->pos.x+ofs.x+rotOfs.x) , 3 ,10 , 5+ePos*2 );
      MusicalEdit_UpdateNumberFunc( F32_CONST(equipData->pos.y+ofs.y+rotOfs.y) , 3 ,14 , 5+ePos*2 );
      if( work->itemWork[ePos] != NULL )
      { 
        GFL_POINT itemOfs;
        pos.x = (equipData->pos.x+ofs.x+FX32_CONST(128.0f) + rotOfs.x);
        pos.y = (equipData->pos.y+ofs.y+FX32_CONST(96.0f) + rotOfs.y);
        if( MUS_ITEM_DRAW_IsBackItem( work->itemWork[ePos] ) == TRUE )
        {
          pos.z = FX32_CONST(0.0f); //後ろ！
        }
        else
        {
          pos.z = FX32_CONST(60.0f);  //とりあえずポケの前に出す
        }

        MUS_ITEM_DRAW_SetPosition(  work->itemDrawSys , 
                      work->itemWork[ePos] ,
                      &pos );
        MUS_ITEM_DRAW_SetRotation(  work->itemDrawSys , 
                      work->itemWork[ePos] ,
                      itemRot-rotZ );
//        MUS_ITEM_DRAW_SetSize(    work->itemDrawSys , 
//                      work->itemWork[ePos] ,
//                      equipData->scale.x /16 /4,
//                      equipData->scale.y /16 /4);

        MUS_ITEM_DRAW_GetDispOffset( work->itemWork[ePos] , &itemOfs );
        MusicalEdit_UpdateNumberFunc( itemOfs.x , 3 ,25 , 5+ePos*2 );
        MusicalEdit_UpdateNumberFunc( itemOfs.y , 3 ,29 , 5+ePos*2 );
      }
      else
      {
        u16 chrNo[3] = {0x01,0x01,0x01};
        GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , 24,5+ePos*2,3,1);
        GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , 27,5+ePos*2,3,1);
      }
    }
    else
    {
      u16 chrNo[3] = {0x0e,0x0f,0x10};
      GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo ,  8,5+ePos*2,3,1);
      GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , 12,5+ePos*2,3,1);
    }
    //デバッグ用に一回見たらFALSEに
    equipData->isEnable = FALSE;
  }
  GFL_BG_LoadScreenV_Req( EDIT_FRAME_SUB_BG );
}

static void MusicalSetting_UpdateTouch( MUS_EDIT_LOCAL_WORK *work )
{
  BOOL isRefresh = FALSE;
  BOOL isReload = FALSE;
  MUSICAL_POKE_PARAM *musPoke = work->actInitWork->musPoke[work->pokeNo];

  //アイテム変更
  {
    static const GFL_UI_TP_HITTBL hitTbl[27+1] =
    {
      { 48 , 56 , 136 ,144},{ 48 , 56 , 152, 160 },{ 48 , 56 ,168 ,176},
      { 64 , 72 , 136 ,144},{ 64 , 72 , 152, 160 },{ 64 , 72 ,168 ,176},
      { 80 , 88 , 136 ,144},{ 80 , 88 , 152, 160 },{ 80 , 88 ,168 ,176},
      { 96 ,104 , 136 ,144},{ 96 ,104 , 152, 160 },{ 96 ,104 ,168 ,176},
      {112 ,120 , 136 ,144},{112 ,120 , 152, 160 },{112 ,120 ,168 ,176},
      {128 ,136 , 136 ,144},{128 ,136 , 152, 160 },{128 ,136 ,168 ,176},
      {144 ,152 , 136 ,144},{144 ,152 , 152, 160 },{144 ,152 ,168 ,176},
      {160 ,168 , 136 ,144},{160 ,168 , 152, 160 },{160 ,168 ,168 ,176},
      {176 ,184 , 136 ,144},{176 ,184 , 152, 160 },{176 ,184 ,168 ,176},
      { GFL_UI_TP_HIT_END },
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    
    if( ret != GFL_UI_TP_HIT_NONE )
    {
      const s8 valueArr[3] = {-1,1,0};
      const u8 ePos = ret/3;
      s8 moveVal = valueArr[ret%3];
      u16 *itemNo = &musPoke->equip[ePos].itemNo;
      
      if( moveVal == 0 )
      {
        *itemNo = MUSICAL_ITEM_INVALID;
      }
      else
      {
        u16 count = 0;
        if( *itemNo == MUSICAL_ITEM_INVALID )
        {
          *itemNo = 0;
        }
        while( TRUE )
        {
          if( *itemNo + moveVal < 0 )
          {
            *itemNo = (*itemNo + MUSICAL_ITEM_MAX + moveVal )%MUSICAL_ITEM_MAX;
          }
          else
          {
            *itemNo = (*itemNo + moveVal)%MUSICAL_ITEM_MAX;
          }
          if( work->isItemAuto == TRUE )
          {
            if( MUS_ITEM_DRAW_CanEquipPosItemNo(work->itemDrawSys,*itemNo,ePos) == TRUE )
            {
              break;
            }
            else
            {
              count++;
              if( count >= MUSICAL_ITEM_MAX )
              {
                //ないと思うが装備品がなかったとき
                *itemNo = MUSICAL_ITEM_INVALID;
                break;
              }
            }
          }
          else
          {
            break;
          }
        }//while
      }
      isReload = TRUE;
    }
  }
  //ポケ系変更
  {
    enum{ POKEBUT_NO_DOWN , POKEBUT_NO_UP , 
          POKEBUT_POKE_DOWN2 , POKEBUT_POKE_DOWN ,
          POKEBUT_POKE_UP , POKEBUT_POKE_UP2 ,
          POKE_BUTTON_MAX};
    static const GFL_UI_TP_HITTBL hitTbl[POKE_BUTTON_MAX+1] =
    {
      {  8 , 16 , 32 , 40},{  8 , 16 , 48 , 56},
      { 24 , 36 ,  8 , 20},{ 24 , 36 , 20 , 32},
      { 24 , 36 , 32 , 44},{ 24 , 36 , 44 , 56},
      { GFL_UI_TP_HIT_END ,0,0,0},
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    const u8 befPokeNo = work->pokeNo;
    
    switch( ret )
    {
    case POKEBUT_NO_DOWN:
      ( work->pokeNo == 0 ? work->pokeNo = 3 : work->pokeNo-- );
      isReload = TRUE;
      work->actInitWork->musPoke[befPokeNo]->charaType = MUS_CHARA_NPC;
      work->actInitWork->musPoke[work->pokeNo]->charaType = MUS_CHARA_PLAYER;
      break;
    case POKEBUT_NO_UP:
      ( work->pokeNo == 3 ? work->pokeNo = 0 : work->pokeNo++ );
      isReload = TRUE;
      work->actInitWork->musPoke[befPokeNo]->charaType = MUS_CHARA_NPC;
      work->actInitWork->musPoke[work->pokeNo]->charaType = MUS_CHARA_PLAYER;
      break;
    case POKEBUT_POKE_DOWN2:
    case POKEBUT_POKE_DOWN:
    case POKEBUT_POKE_UP:
    case POKEBUT_POKE_UP2:
      {
        const s8 valueArr2[4] = {-5,-1,1,5};
        s8 moveVal = valueArr2[ ret-POKEBUT_POKE_DOWN2 ];
        s32 tempNo = musPoke->mcssParam.monsno;
        while( moveVal != 0 )
        {
          while( TRUE )
          {
            ( moveVal<0 ? tempNo-- : tempNo++ );
            tempNo = (tempNo+MONSNO_MAX+1)%(MONSNO_MAX+1);
            if( MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( tempNo ) == TRUE )
            {
              break;
            }
          }
          ( moveVal<0 ? moveVal++ : moveVal-- );
        }
        GFL_HEAP_FreeMemory( musPoke );
        musPoke = MUSICAL_SYSTEM_InitMusPokeParam( tempNo , 0 , 0 , 0 , HEAPID_MUSICAL_STAGE );
        isReload = TRUE;
      }
      break;
    }
  }

  //特殊ボタン
  {
    enum{ SPBUT_LOAD , SPBUT_ANIME, SPBUT_E_POS , SPBUT_AUTO_ITEM, SPBUT_CLEAR , SPBUT_START , SP_BUTTON_MAX};
    static const GFL_UI_TP_HITTBL hitTbl[SP_BUTTON_MAX+1] =
    {
      {  8 , 16 , 64 , 88},
      {  8 , 16 , 96 ,120},
      { 24 , 32 , 64 , 88},
      {  8 , 16 ,128 ,152},
      {  8 , 16 ,160 ,184},
      {  0 ,  8 ,232 ,256},
      { GFL_UI_TP_HIT_END ,0,0,0},
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    switch( ret )
    {
    case SPBUT_LOAD:
      isReload = TRUE;
      break;
    case SPBUT_AUTO_ITEM:
      work->isItemAuto = (work->isItemAuto==TRUE ? FALSE : TRUE);
      isRefresh = TRUE;
      break;
    case SPBUT_ANIME:
      if( work->isPokeAnime == TRUE )
      {
        work->isPokeAnime = FALSE;
        MUS_POKE_DRAW_StopAnime( work->pokeWork );
      }
      else
      {
        work->isPokeAnime = TRUE;
        MUS_POKE_DRAW_StartAnime( work->pokeWork );
      }
      isRefresh = TRUE;
      break;
    case SPBUT_E_POS:
      work->isDrawEPos = (work->isDrawEPos==TRUE ? FALSE : TRUE);
      isRefresh = TRUE;
      break;
    case SPBUT_START:
      work->isFinishSetting = TRUE;
      break;
    case SPBUT_CLEAR:
      {
        u8 ePos;
        for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
        {
          musPoke->equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
        }
        isReload = TRUE;
      }
      break;
    }
  }
  if( isReload == TRUE )
  {
    MusicalSetting_UnloadPoke( work );
    MusicalSetting_LoadPoke( work );
    isRefresh = TRUE;
  }
  if( isRefresh == TRUE )
  {
    MusicalSetting_DrawPoke( work );
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    int ePos;
    int cnt = 0;
    const u8 langId = GFL_MSGSYS_GetLangID();
    GFL_MSGSYS_SetLangID( 0 );
    OS_TPrintf("-----------------------------------\n");
    for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
    {
      const u16 itemNo = musPoke->equip[ ePos ].itemNo;
      if( itemNo != MUSICAL_ITEM_INVALID )
      {
        int i;
        static const char *posName[] = {"右耳","左耳","頭","目鼻","顔","胴","腰","右手","左手"};
        int jisSize = 128 , uniSize = 32;
        char jisStr[128];
        STRBUF *str = GFL_STR_CreateBuffer( uniSize , work->heapId );
        STRCODE *code = (STRCODE*)GFL_STR_GetStringCodePointer(str);
        code[GFL_STR_GetBufferLength(str)] = 0;
        GFL_STD_MemClear( jisStr , 128 );
        GFL_MSG_GetString( work->itemMsgHandle , MUS_ITEM_DRAW_GetArcIdx( itemNo )+ITEM_NAME_000 , str );
        //STD_ConvertStringUnicodeToSjis( jisStr , &jisSize , code , &uniSize , NULL );
        DEB_STR_CONV_StrcodeToSjis( code , jisStr , uniSize );
        
        OS_TPrintf("%s %s 5 ",jisStr,posName[ePos]);
        cnt++;
      }
    }
    for( ePos=cnt;ePos<MUS_POKE_EQUIP_MAX-1;ePos++ )
    {
      OS_TPrintf("装備無し 目鼻 5 ");
    }
    
    GFL_MSGSYS_SetLangID( langId );
    OS_TPrintf("\n-----------------------------------\n");
  }
  
}

static void MusicalSetting_DrawEquipPos( MUS_EDIT_LOCAL_WORK *work )
{
  int ePos;
  VecFx32 pos;
  MtxFx33       mtxBillboard;
  VecFx16       vecN;
  G3_PushMtx();
  G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );
  //カメラ設定取得
  {
    VecFx32   vecNtmp;
    MtxFx43   mtxCamera, mtxCameraInv;
    static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
    static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
    static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
  
    G3_LookAt( &cam_pos, &cam_up, &cam_target, &mtxCamera );  //mtxCameraには行列計算結果が返る
    MTX_Inverse43( &mtxCamera, &mtxCameraInv );     //カメラ逆行列取得
    MTX_Copy43To33( &mtxCameraInv, &mtxBillboard );   //カメラ逆行列から回転行列を取り出す

    VEC_Subtract( &cam_pos, &cam_target, &vecNtmp );
    VEC_Normalize( &vecNtmp, &vecNtmp );
    VEC_Fx16Set( &vecN, vecNtmp.x, vecNtmp.y, vecNtmp.z );
  }
  G3_PolygonAttr(GX_LIGHTMASK_NONE,       // no lights
           GX_POLYGONMODE_MODULATE,     // modulation mode
           GX_CULL_NONE,          // cull back
           63,                // polygon ID(0 - 63)
           0,                 // alpha(0 - 31)
           0                  // OR of GXPolygonAttrMisc's value
           );


  for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
  {
    MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->pokeWork , ePos );
    if( equipData->isEnable == TRUE )
    {
      const u16 itemRot = equipData->itemRot;
      const u16 rotZ = 0x10000-equipData->rot;

      MtxFx33 rotWork;
      VecFx32 rotOfs;
      VecFx32 ofs;

      G3_PushMtx();


      MTX_RotZ33( &rotWork , -FX_SinIdx( rotZ ) , FX_CosIdx( rotZ ) );
      MTX_MultVec33( &equipData->ofs , &rotWork , &ofs );
      MTX_MultVec33( &equipData->rotOfs , &rotWork , &rotOfs );
      VEC_Subtract( &equipData->rotOfs , &rotOfs , &rotOfs );

      pos.x = MUSICAL_POS_X_FX(equipData->pos.x+ofs.x+FX32_CONST(128.0f) + rotOfs.x);
      pos.y = MUSICAL_POS_Y_FX(equipData->pos.y+ofs.y+FX32_CONST(96.0f) + rotOfs.y);
      pos.z = FX32_CONST(120.0f); //とりあえずポケの前に出す

      G3_Translate( pos.x, pos.y, pos.z );
      G3_RotZ( -FX_SinIdx( itemRot-rotZ ), FX_CosIdx( itemRot-rotZ ) );
      G3_Scale( FX32_CONST(0.25f), FX32_CONST(0.25f), pos.z );
      {
        const fx32 size = FX32_ONE;
        G3_Begin(GX_BEGIN_QUADS);
        G3_Color( GX_RGB( 0, 0, 0 ) );
/*  
        //矩形        
        G3_Vtx( -size, size, 0 );
        G3_Vtx(  size, size, 0 );
        G3_Vtx(  size,-size, 0 );
        G3_Vtx( -size,-size, 0 );
*/
        //十字
        G3_Vtx( -size, 0, 0 );
        G3_Vtx(  size, 0, 0 );
        G3_Vtx(  size, 0, 0 );
        G3_Vtx( -size, 0, 0 );
        G3_Vtx( 0, size, 0 );
        G3_Vtx( 0, size, 0 );
        G3_Vtx( 0,-size, 0 );
        G3_Vtx( 0,-size, 0 );
        G3_End();
      }
/*
      MUS_ITEM_DRAW_SetPosition(  work->itemDrawSys , 
                    work->itemWork[ePos] ,
                    &pos );
      MUS_ITEM_DRAW_SetRotation(  work->itemDrawSys , 
                    work->itemWork[ePos] ,
                    itemRot-rotZ );
*/
      G3_PopMtx(1);
    }
  } 
  G3_PopMtx(1);
}


#endif //USE_MUSICAL_EDIT
