//======================================================================
/**
 * @file	local_tvt_chara.c
 * @brief	非通信テレビトランシーバー キャラ
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT_CHARA
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "message.naix"
#include "local_tvt_graphic.naix"
#include "comm_tvt.naix"

#include "local_tvt_local_def.h"
#include "local_tvt_chara.h"
#include "local_tvt_graphic.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define LTVT_CHARA_SCREEN_WIDTH (16)
#define LTVT_CHARA_SCREEN_HEIGHT (24)

#define LTVT_CHARA_UPDATE_CNT (40)

#define LTVT_CHARA_NAME_WIDTH (16-4)
#define LTVT_CHARA_NAME_HEIGHT (2)

#define LTVT_CHARA_EYE_BLINK_CNT (150)
#define LTVT_CHARA_LIP_DELAY_CNT (70)
#define LTVT_CHARA_OFFSET (7)
#define LTVT_CHARA_OFFSET_SIZE (LTVT_CHARA_OFFSET*16*0x40)

typedef enum
{
  LTCR_PLT,
  LTCR_NCG,
  LTCR_ANM,
  LTCR_MAX,
  
}LOCAL_TVT_CELL_RES;

const u8 LOCAL_TVT_CHARA_BASE_POS[LOCAL_TVT_MEMBER_MAX][2] = {{0,0},{16,0},{0,12},{16,12}};

static const GFL_CLACTPOS sc_weather_pos[LOCAL_TVT_MEMBER_MAX] =
{
  {
    128,96
  },
  {
    256,96,
  },
  {
    128,192
  },
  {
    256,192
  },
};

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  LTCTS_WAIT_REQ,
  LTCTS_CHARA1,
  LTCTS_CHARA2,
  LTCTS_BG1,
  LTCTS_BG2,
  LTCTS_WAIT_TRANS,
  
}LOCAL_TVT_CHARA_TRANS_STATE;

typedef enum
{
  LTTS_CHARA1,
  LTTS_CHARA2,
  LTTS_CHARA3,
  LTTS_CHARA4,
  LTTS_TRANS_WAIT,
  
  LTTS_MAX,
}LOCAL_TVT_TRANS_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

struct _LOCAL_TVT_CHARA_SYS
{
  LOCAL_TVT_CHARA_SYS_PARAM param;
  s32 bufNo;
  s32 transCnt;
  LOCAL_TVT_TRANS_STATE transState;
  LOCAL_TVT_CHARA *charaWork[LOCAL_TVT_MEMBER_MAX];
  LOCAL_TVT_CHARA *transChara;
  GFL_MSGDATA     *msgHandle;
  LOCAL_TVT_MEMBER member;
  ARCHANDLE *arcHandle;

  u32 cellRes[LTCR_MAX];
  GFL_CLWK    *clwkRecIcon;
  HEAPID heapID;
};

struct _LOCAL_TVT_CHARA
{
  u8 charaIdx;
  u8 transIdx;
  
  u8 charaType;
  u8 bgType;
  u8 charaAnmIdx;
  u8 bgAnmIdx;
  u8 anmCnt;
  u8 nameLen;

  u32 weatherType;
  
  LOCAL_TVT_CHARA_TRANS_STATE state;

  NNSG2dCharacterData *charaData;
  void *resData;
  
  BOOL isUpdateQue;
  GFL_BMPWIN  *nameWin;

  u32 cellRes[3];
  GFL_CLWK *clwkEye;
  GFL_CLWK *clwkMouth;
  
  u16  lipDelayCnt;
  u16 eyeBlinkCnt;
  BOOL isLipSync;

  LOCAL_TVT_CHARA_SYS *sys;
  HEAPID heapID;


  GFL_CLWK    *weather;
  u32 weatherCellRes[LTCR_MAX];
};

static const u8 charaResList[LTCT_MAX][3] = {
    { NARC_local_tvt_graphic_chara_player_m_NCLR , NARC_local_tvt_graphic_chara_player_m_1_NCGR , NARC_local_tvt_graphic_obj_player_m_NCLR },
    { NARC_local_tvt_graphic_chara_rival_NCLR    , NARC_local_tvt_graphic_chara_rival_1_NCGR    , NARC_local_tvt_graphic_obj_rival_NCLR },
    { NARC_local_tvt_graphic_chara_support_NCLR  , NARC_local_tvt_graphic_chara_support_1_NCGR  , NARC_local_tvt_graphic_obj_support_NCLR },
    { NARC_local_tvt_graphic_chara_doctor_d_NCLR , NARC_local_tvt_graphic_chara_doctor_d_1_NCGR , NARC_local_tvt_graphic_obj_doctor_d_NCLR },
    { NARC_local_tvt_graphic_chara_mother_NCLR   , NARC_local_tvt_graphic_chara_mother_1_NCGR   , NARC_local_tvt_graphic_obj_mother_NCLR },
    //主人公分岐用・主人公(女)素材
    { NARC_local_tvt_graphic_chara_player_f_NCLR , NARC_local_tvt_graphic_chara_player_f_1_NCGR , NARC_local_tvt_graphic_obj_player_f_NCLR },
};


static const struct
{
  u16 nclr;
  u16 ncgr;
} bgResList[LOCAL_TVT_BACK_MAX][LTTZ_MAX] = {
  //部屋
  {
    { NARC_local_tvt_graphic_back_01_NCLR , NARC_local_tvt_graphic_back_01_NCGR },
    { NARC_local_tvt_graphic_back_01_NCLR , NARC_local_tvt_graphic_back_01_NCGR },
    { NARC_local_tvt_graphic_back_01_NCLR , NARC_local_tvt_graphic_back_01_NCGR },
  },
  //洞窟
  {
    { NARC_local_tvt_graphic_back_02_NCLR , NARC_local_tvt_graphic_back_01_NCGR },
    { NARC_local_tvt_graphic_back_02_NCLR , NARC_local_tvt_graphic_back_01_NCGR },
    { NARC_local_tvt_graphic_back_02_NCLR , NARC_local_tvt_graphic_back_01_NCGR },
  },
  //街
  {
    { NARC_local_tvt_graphic_back_03a_NCLR,NARC_local_tvt_graphic_back_03a_NCGR },
    { NARC_local_tvt_graphic_back_03b_NCLR,NARC_local_tvt_graphic_back_03b_NCGR },
    { NARC_local_tvt_graphic_back_03c_NCLR,NARC_local_tvt_graphic_back_03c_NCGR },
  },
  //平野
  {
    { NARC_local_tvt_graphic_back_04a_NCLR,NARC_local_tvt_graphic_back_04a_NCGR },
    { NARC_local_tvt_graphic_back_04b_NCLR,NARC_local_tvt_graphic_back_04b_NCGR },
    { NARC_local_tvt_graphic_back_04c_NCLR,NARC_local_tvt_graphic_back_04c_NCGR },
  },
  //山
  {
    { NARC_local_tvt_graphic_back_05a_NCLR,NARC_local_tvt_graphic_back_05a_NCGR },
    { NARC_local_tvt_graphic_back_05b_NCLR,NARC_local_tvt_graphic_back_05b_NCGR },
    { NARC_local_tvt_graphic_back_05c_NCLR,NARC_local_tvt_graphic_back_05c_NCGR },
  },
  //海
  {
    { NARC_local_tvt_graphic_back_06a_NCLR,NARC_local_tvt_graphic_back_06a_NCGR },
    { NARC_local_tvt_graphic_back_06b_NCLR,NARC_local_tvt_graphic_back_06b_NCGR },
    { NARC_local_tvt_graphic_back_06c_NCLR,NARC_local_tvt_graphic_back_06c_NCGR },
  },
  //砂漠
  {
    { NARC_local_tvt_graphic_back_07a_NCLR,NARC_local_tvt_graphic_back_07a_NCGR },
    { NARC_local_tvt_graphic_back_07b_NCLR,NARC_local_tvt_graphic_back_07b_NCGR },
    { NARC_local_tvt_graphic_back_07c_NCLR,NARC_local_tvt_graphic_back_07c_NCGR },
  },
};

static const struct
{
  u8  nclr;
  u8  ncgr;
  u8  ncer;
  u8  nanr;
}weatherResList[LOCAL_TVT_WEATHER_MAX][2]  ={
  {
    {NARC_local_tvt_graphic_weather_rain_NCLR,NARC_local_tvt_graphic_weather_rain2_NCGR,NARC_local_tvt_graphic_weather_rain2_NCER,NARC_local_tvt_graphic_weather_rain2_NANR},
    {NARC_local_tvt_graphic_weather_rain_NCLR,NARC_local_tvt_graphic_weather_rain4_NCGR,NARC_local_tvt_graphic_weather_rain4_NCER,NARC_local_tvt_graphic_weather_rain4_NANR},
  },
  {
    {NARC_local_tvt_graphic_weather_sand_NCLR,NARC_local_tvt_graphic_weather_sand2_NCGR,NARC_local_tvt_graphic_weather_sand2_NCER,NARC_local_tvt_graphic_weather_sand2_NANR},
    {NARC_local_tvt_graphic_weather_sand_NCLR,NARC_local_tvt_graphic_weather_sand4_NCGR,NARC_local_tvt_graphic_weather_sand4_NCER,NARC_local_tvt_graphic_weather_sand4_NANR},
  },
  {
    {NARC_local_tvt_graphic_weather_snow_NCLR,NARC_local_tvt_graphic_weather_snow2_NCGR,NARC_local_tvt_graphic_weather_snow2_NCER,NARC_local_tvt_graphic_weather_snow2_NANR},
    {NARC_local_tvt_graphic_weather_snow_NCLR,NARC_local_tvt_graphic_weather_snow4_NCGR,NARC_local_tvt_graphic_weather_snow4_NCER,NARC_local_tvt_graphic_weather_snow4_NANR},
  },
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static LOCAL_TVT_CHARA* LOCAL_TVT_CHARA_Init( LOCAL_TVT_CHARA_SYS *sys, u8 charaType, u8 bgType, LOCAL_TVT_WEATHER weather, LOCAL_TVT_CHARA_POS pos, HEAPID heapID );
static void LOCAL_TVT_CHARA_Term( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA *charaWork );
static void LOCAL_TVT_CHARA_Main( LOCAL_TVT_CHARA *charaWork );
static const BOOL LOCAL_TVT_CHARA_isFinishTrans( LOCAL_TVT_CHARA *charaWork );
static void LOCAL_TVT_CHARA_RedrawName( LOCAL_TVT_CHARA *charaWork );
static void LOCAL_TVT_CHARA_LoadResource( LOCAL_TVT_CHARA *charaWork , const u8 bufNo);
static u8 LOCAL_TVT_CHARA_GetCharaType( const LOCAL_TVT_CHARA *charaWork );
static u8 LOCAL_TVT_CHARA_GetBgType( const LOCAL_TVT_CHARA *charaWork );
static u32 LOCAL_TVT_CHARA_GetWeatherType( const LOCAL_TVT_CHARA *charaWork );


static void LOCAL_TVT_CHARA_SetLipSync( LOCAL_TVT_CHARA *charaWork , const BOOL flg );
static void LOCAL_TVT_CHARA_LipSyncForceStop( LOCAL_TVT_CHARA *charaWork );
static void LOCAL_TVT_CHARA_LoadCommonResource( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, HEAPID heapID );
static void LOCAL_TVT_CHARA_LoadCharaResource( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, HEAPID heapID );
static void LOCAL_TVT_CHARA_LoadNcgResource( LOCAL_TVT_CHARA *charaWork , ARCHANDLE *arcHandle, ARCDATID datId, HEAPID heapID );
static void LOCAL_TVT_CHARA_TransNcgResource( LOCAL_TVT_CHARA *charaWork , const u32 topOfs , const u32 transOfs , const u32 transSize );
static void LOCAL_TVT_CHARA_DispName( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, HEAPID heapID );
static const u8 LOCAL_TVT_CHARA_GetNameLen( LOCAL_TVT_CHARA *charaWork );
static const void LOCAL_TVT_CHARA_LoadWeather( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_WEATHER weather, LOCAL_TVT_MEMBER member, HEAPID heapID );
static const void LOCAL_TVT_CHARA_UnLoadWeather( LOCAL_TVT_CHARA *charaWork);

static void LOCAL_TVT_RECICON_Load( LOCAL_TVT_CHARA_SYS *sys );
static void LOCAL_TVT_RECICON_UnLoad( LOCAL_TVT_CHARA_SYS *sys );
static void LOCAL_TVT_RECICON_Visible( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA_POS pos );
static void LOCAL_TVT_RECICON_Hide( LOCAL_TVT_CHARA_SYS *sys );

//----------------------------------------------------------------------------
/**
 *	@brief  キャラシステム  初期化
 *
 *	@param	const LOCAL_TVT_CHARA_SYS_PARAM *param  引数
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CHARA_SYS * LOCAL_TVT_CHARA_SYS_Init( const LOCAL_TVT_CHARA_SYS_PARAM *param , HEAPID heapID )
{
  LOCAL_TVT_CHARA_SYS *sys = GFL_HEAP_AllocClearMemory( heapID, sizeof( LOCAL_TVT_CHARA_SYS ) );
  sys->param  = *param;
  sys->transCnt = 0;
  sys->bufNo = 1;
  sys->transState = LTTS_CHARA1;
  sys->heapID = heapID;
  

  sys->arcHandle  = GFL_ARC_OpenDataHandle( ARCID_LOCAL_TVT_GRAPHIC, sys->heapID );
  sys->msgHandle  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_tvt_event_system_dat , sys->heapID );

  LOCAL_TVT_RECICON_Load(sys );

  return sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラシステム破棄
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_SYS_Term( LOCAL_TVT_CHARA_SYS *sys )
{
  int i;

  LOCAL_TVT_RECICON_UnLoad(sys );

  GFL_MSG_Delete( sys->msgHandle );
  GFL_ARC_CloseDataHandle( sys->arcHandle );

  for( i=0 ; i<sys->member ; i++ )
  {
    if( sys->charaWork[i] != NULL )
    {
      LOCAL_TVT_CHARA_Term( sys, sys->charaWork[i] );
    }
  }

  GFL_HEAP_FreeMemory( sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャラシステムメイン処理
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys  ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_SYS_Main( LOCAL_TVT_CHARA_SYS *sys )
{
  int i;
  //フリップタイミング
  if( sys->transCnt < LTVT_TRANS_COUNT_MAX )
  {
    sys->transCnt++;
  }
  //else
  {
    if( sys->transChara == NULL &&
        sys->transState == LTTS_TRANS_WAIT )
    {
      GXBGCharBase charaBaseAdr;
      GXBGCharBase bgBaseAdr;
      if( sys->bufNo == 0 )
      {
        charaBaseAdr = GX_BG_CHARBASE_0x10000;
        bgBaseAdr = GX_BG_CHARBASE_0x28000;
      }
      else
      {
        charaBaseAdr = GX_BG_CHARBASE_0x1c000;
        bgBaseAdr = GX_BG_CHARBASE_0x34000;
      }
      G2_SetBG2Control256x16Affine( GX_BG_SCRSIZE_256x16PLTT_256x256 , 
                                    GX_BG_AREAOVER_REPEAT ,
                                    GX_BG_SCRBASE_0x6800 ,
                                    charaBaseAdr );
      G2_SetBG3Control256x16Affine( GX_BG_SCRSIZE_256x16PLTT_256x256 , 
                                    GX_BG_AREAOVER_REPEAT ,
                                    GX_BG_SCRBASE_0x7000 ,
                                    bgBaseAdr );
      sys->bufNo = !sys->bufNo;
      sys->transState = LTTS_CHARA1;
      sys->transCnt = 0;
    }
  }
  
  //絵の転送
  if( sys->transChara != NULL )
  {
    if( LOCAL_TVT_CHARA_isFinishTrans( sys->transChara ) == TRUE )
    {
      sys->transChara = NULL;
    }
  }
  
  if( sys->transChara == NULL )
  {
    switch( sys->transState )
    {
    case LTTS_CHARA1:
    case LTTS_CHARA2:
    case LTTS_CHARA3:
    case LTTS_CHARA4:
      {
        const u8 charaNo = sys->transState - LTTS_CHARA1;
        if( sys->charaWork[charaNo] != NULL )
        {
          LOCAL_TVT_CHARA_LoadResource( sys->charaWork[charaNo] , sys->bufNo );
          sys->transChara = sys->charaWork[charaNo];
          if( sys->member == LTM_2_MEMBER &&
              sys->transState == LTTS_CHARA2 )
          {
            sys->transState = LTTS_TRANS_WAIT;
          }
          else
          {
            sys->transState++;
          }
        }
        else
        {
          sys->transState++;
        }
      }
      break;
    }
  }

  for( i=0 ; i<sys->member ; i++ )
  {
    if( sys->charaWork[i] != NULL )
    {
      LOCAL_TVT_CHARA_Main( sys->charaWork[i] );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャラクター読込み  初期化直後に呼ぶこと
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys
 *	@param	LOCAL_TVT_CHARA_PARAM *cp_setup_tbl 
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_SYS_LoadChara( LOCAL_TVT_CHARA_SYS *sys, const LOCAL_TVT_CHARA_PARAM *cp_setup_tbl, LOCAL_TVT_MEMBER member )
{
  sys->member = member;
  sys->transCnt = 0;
  sys->bufNo = 1;
  sys->transState = LTTS_CHARA1;
  sys->transChara = NULL;

  {
    u32 datId;
    if( sys->member == LTM_2_MEMBER )
    {
      datId = NARC_local_tvt_graphic_local_tvt_scr_2_NSCR;
    }
    else
    {
      datId = NARC_local_tvt_graphic_local_tvt_scr_4_NSCR;
    }

    GFL_ARCHDL_UTIL_TransVramScreen( sys->arcHandle , 
        datId ,
        LTVT_FRAME_CHARA ,
        0,0,FALSE,sys->heapID );

    GFL_ARCHDL_UTIL_TransVramScreen( sys->arcHandle , 
        datId ,
        LTVT_FRAME_BG ,
        0,0,FALSE,sys->heapID );
  }


  {
    int i;
    for( i=0 ; i<sys->member ; i++ )
    {
      const LOCAL_TVT_CHARA_PARAM *chara_param  = &cp_setup_tbl[i];
      if( chara_param->charaType != 0xFF )
      {
        if( sys->charaWork[i] != NULL )
        {
          LOCAL_TVT_CHARA_Term( sys, sys->charaWork[i] );
        }

        sys->charaWork[i] = LOCAL_TVT_CHARA_Init( sys,  chara_param->charaType, chara_param->bgType, chara_param->weatherType, i, sys->heapID );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワークを取得
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys  システム
 *	@param	pos   位置
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CHARA* LOCAL_TVT_CHARA_SYS_GetWork( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA_POS pos )
{
  GF_ASSERT( pos < sys->member );
  return sys->charaWork[ pos ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラクター全員の名前を再描画
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys システム
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_SYS_RedrawName( LOCAL_TVT_CHARA_SYS *sys )
{
  int i;
  for( i=0 ; i<sys->member; i++ )
  {
    LOCAL_TVT_CHARA *charaWork  = LOCAL_TVT_CHARA_SYS_GetWork( sys, i );
    if( charaWork != NULL )
    {
      LOCAL_TVT_CHARA_RedrawName( charaWork );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイムゾーン再設定
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys  システム
 *	@param	timeZone タイムゾーン
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_SYS_SetTimeZone( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_TIME_ZONE timeZone )
{
  sys->param.timeZone = timeZone;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイムゾーン受け取り
 *
 *	@param	const LOCAL_TVT_CHARA_SYS *sys  システム
 *
 *	@return タイムゾーン
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_TIME_ZONE LOCAL_TVT_CHARA_SYS_GetTimeZone( const LOCAL_TVT_CHARA_SYS *sys )
{
  return sys->param.timeZone;
}
//----------------------------------------------------------------------------
/**
 *	@brief  引数受けとり
 *
 *	@param	const LOCAL_TVT_CHARA_SYS *sys  システム
 *	@param	*p_param  引数受け取りワーク
 *	@param	*p_member メンバー受け取りワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_SYS_GetLoadParam( const LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA_PARAM *p_param, LOCAL_TVT_MEMBER *p_member )
{
  int i; 
  for( i = 0; i < sys->member; i++ )
  {
    p_param[i].bgType       = LOCAL_TVT_CHARA_GetBgType(sys->charaWork[i]);
    p_param[i].weatherType  = LOCAL_TVT_CHARA_GetWeatherType( sys->charaWork[i]);
    p_param[i].charaType    = LOCAL_TVT_CHARA_GetCharaType(sys->charaWork[i]);
  }


  *p_member = sys->member;
}
//----------------------------------------------------------------------------
/**
 *	@brief  話開始
 *
 *	@param	LOCAL_TVT_CHARA *charaWork  キャラ
 *	@param	*p_msg  メッセージデータ
 *	@param	msgID   メッセージID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CHARA_StartTalk( LOCAL_TVT_CHARA *charaWork, GFL_MSGDATA *p_msg, u16 msgID )
{
  LOCAL_TVT_MSG_POS pos;

  //ウィンドウ位置を決める
  if( charaWork->charaIdx < 2 )
  {
    pos = LTMP_DOWN;
  }
  else
  {
    pos = LTMP_UP;
  }

  //ウィンドウON
  LOCAL_TVT_MSGWIN_OpenWindow( charaWork->sys->param.message, pos );

  
  //音声アイコン表示
  LOCAL_TVT_RECICON_Visible( charaWork->sys, charaWork->charaIdx );

  //メッセージON
  LOCAL_TVT_MSGWIN_DispMessage( charaWork->sys->param.message, p_msg, msgID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  話処理中
 *
 *	@param	LOCAL_TVT_CHARA *charaWork  キャラ
 *	@param	LOCAL_TVT_CHARA_SYS *sys  キャラシステム
 *
 *	@return TRUE話終了  FALSE話中
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_CHARA_WaitTalk( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys )
{
  if( LOCAL_TVT_MSGWIN_IsFinish( charaWork->sys->param.message ) )
  {
    //ウィンドウOFF
    LOCAL_TVT_MSGWIN_CloseWindow( charaWork->sys->param.message );

    //音声アイコン非表示
    LOCAL_TVT_RECICON_Hide( charaWork->sys );

    //名前表示
    LOCAL_TVT_CHARA_SYS_RedrawName( sys );
    //口を閉じる
    LOCAL_TVT_CHARA_LipSyncForceStop( charaWork );
    return TRUE;
  }
  else
  {
    //文字が流れているならば口パクする
    if( LOCAL_TVT_MSGWIN_IsPause( charaWork->sys->param.message ) )
    {
      LOCAL_TVT_CHARA_SetLipSync( charaWork, FALSE );
    }
    else
    {
      LOCAL_TVT_CHARA_SetLipSync( charaWork, TRUE );
    }
    return FALSE;
  }
}
//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static LOCAL_TVT_CHARA* LOCAL_TVT_CHARA_Init( LOCAL_TVT_CHARA_SYS *sys, u8 charaType, u8 bgType, LOCAL_TVT_WEATHER weather, LOCAL_TVT_CHARA_POS pos, HEAPID heapID )
{
  {
    LOCAL_TVT_CHARA *charaWork = GFL_HEAP_AllocClearMemory( heapID, sizeof( LOCAL_TVT_CHARA ) );
    
    charaWork->sys    = sys;
    charaWork->heapID = heapID;
    charaWork->charaIdx = pos;
    charaWork->charaType = charaType;
    charaWork->bgType = bgType;
    charaWork->weatherType  = weather;
    charaWork->charaAnmIdx = 0;
    charaWork->bgAnmIdx = 0;
    charaWork->isUpdateQue = FALSE;
    charaWork->isLipSync = FALSE;
    charaWork->eyeBlinkCnt = GFUser_GetPublicRand0(LTVT_CHARA_EYE_BLINK_CNT/2);
    charaWork->weatherCellRes[LTCR_PLT] =  0xFF;
    charaWork->weatherCellRes[LTCR_NCG] =  0xFF;
    charaWork->weatherCellRes[LTCR_ANM] =  0xFF;

    if( charaWork->charaType == LTCT_PLAYER_M )
    {
      MYSTATUS *mystatus = sys->param.mystatus;
      if( MyStatus_GetMySex(mystatus) == PM_FEMALE )
      {
        charaWork->charaType = LTCT_PLAYER_F;
      }
    }

    NAGI_Printf("[%d][%d:%d]\n",charaWork->charaIdx,charaWork->charaType,charaWork->bgType);
    
    LOCAL_TVT_CHARA_LoadCharaResource( charaWork, sys, heapID );
    LOCAL_TVT_CHARA_LoadCommonResource( charaWork, sys, heapID );

    LOCAL_TVT_CHARA_DispName( charaWork, sys, heapID );
    LOCAL_TVT_CHARA_LoadWeather( charaWork, sys, weather, sys->member, heapID );
    
    charaWork->state = LTCTS_WAIT_REQ;
    charaWork->resData = NULL;
    
    charaWork->anmCnt = GFUser_GetPublicRand0(LTVT_CHARA_UPDATE_CNT);
    charaWork->lipDelayCnt = 0;
    
    return charaWork;
  }
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_Term( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA *charaWork )
{

  LOCAL_TVT_CHARA_UnLoadWeather( charaWork );
  GFL_CLACT_WK_Remove( charaWork->clwkEye );
  GFL_CLACT_WK_Remove( charaWork->clwkMouth );
  GFL_CLGRP_PLTT_Release( charaWork->cellRes[0] );
  GFL_CLGRP_CGR_Release( charaWork->cellRes[1] );
  GFL_CLGRP_CELLANIM_Release( charaWork->cellRes[2] );

  GFL_BMPWIN_Delete( charaWork->nameWin );
  if( charaWork->resData != NULL )
  {
    GFL_HEAP_FreeMemory( charaWork->resData );
  }

  
  GFL_HEAP_FreeMemory( charaWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_Main( LOCAL_TVT_CHARA *charaWork )
{
  const u32 charSize = (charaWork->sys->member == LTM_2_MEMBER?0x6000:0x3000);
  const u32 bufOfs = (charaWork->sys->bufNo==0?0:LTVT_VRAM_PAGE_SIZE);
  const u32 transOfs = (charaWork->sys->member == LTM_2_MEMBER?0:LTVT_CHARA_OFFSET_SIZE);

  switch( charaWork->state )
  {
  case LTCTS_WAIT_REQ:
    break;
  case LTCTS_CHARA1:
    charaWork->transIdx = 0;
    LOCAL_TVT_CHARA_LoadNcgResource( charaWork , charaWork->sys->arcHandle, charaResList[charaWork->charaType][1] + charaWork->charaAnmIdx, charaWork->heapID );
    charaWork->state = LTCTS_CHARA2;
    break;
  case LTCTS_CHARA2:
    LOCAL_TVT_CHARA_TransNcgResource( charaWork ,
                                      charaWork->transIdx * LTVT_TRANS_SIZE + transOfs,
                                      LTVT_VRAM_ADR_CHARA + charSize * charaWork->charaIdx + charaWork->transIdx * LTVT_TRANS_SIZE + bufOfs ,
                                      LTVT_TRANS_SIZE );
    charaWork->transIdx++;
    if( charaWork->transIdx * LTVT_TRANS_SIZE >= charSize )
    {
      charaWork->state = LTCTS_BG1;
      GFL_HEAP_FreeMemory( charaWork->resData );
      charaWork->resData = NULL;
    }
    break;
  case LTCTS_BG1:
    charaWork->transIdx = 0;
    LOCAL_TVT_CHARA_LoadNcgResource( charaWork , charaWork->sys->arcHandle, bgResList[charaWork->bgType][charaWork->sys->param.timeZone].ncgr, charaWork->heapID );
    charaWork->state = LTCTS_BG2;
    break;
  case LTCTS_BG2:
    LOCAL_TVT_CHARA_TransNcgResource( charaWork ,
                                      charaWork->transIdx * LTVT_TRANS_SIZE + transOfs,
                                      LTVT_VRAM_ADR_BG + charSize * charaWork->charaIdx + charaWork->transIdx * LTVT_TRANS_SIZE + bufOfs ,
                                      LTVT_TRANS_SIZE );
    charaWork->transIdx++;
    if( charaWork->transIdx * LTVT_TRANS_SIZE >= charSize )
    {
      charaWork->state = LTCTS_WAIT_TRANS;
      GFL_HEAP_FreeMemory( charaWork->resData );
      charaWork->resData = NULL;
    }
    break;
  }
  if( charaWork->anmCnt < LTVT_CHARA_UPDATE_CNT )
  {
    charaWork->anmCnt++;
  }
  else
  {
    if( charaWork->state == LTCTS_WAIT_TRANS ||
        charaWork->state == LTCTS_WAIT_REQ )
    {
      charaWork->anmCnt = 0;
      charaWork->charaAnmIdx = !charaWork->charaAnmIdx;
      charaWork->bgAnmIdx = !charaWork->bgAnmIdx;
    }
  }
  {

    PRINT_QUE *printQue   = LOCAL_TVT_MESSAGE_GetPrintQue( charaWork->sys->param.message );
    if( charaWork->isUpdateQue == TRUE &&
        PRINTSYS_QUE_IsFinished( printQue ) == TRUE )
    {
      GFL_BMPWIN_TransVramCharacter( charaWork->nameWin );
      charaWork->isUpdateQue = FALSE;
    }
  }
  
  //目パチ
  {
    charaWork->eyeBlinkCnt += GFUser_GetPublicRand(2)+1;
    if( charaWork->eyeBlinkCnt >= LTVT_CHARA_EYE_BLINK_CNT )
    {
      GFL_CLACT_WK_StartAnm( charaWork->clwkEye );
      GFL_CLACT_WK_ResetAnm( charaWork->clwkEye );
      charaWork->eyeBlinkCnt = 0;
    }
  }
  //口パク
  if( charaWork->isLipSync == TRUE ||
      charaWork->lipDelayCnt > 0 )
  {
    if( GFL_CLACT_WK_CheckAnmActive( charaWork->clwkMouth ) == FALSE )
    {
      GFL_CLACT_WK_StartAnm( charaWork->clwkMouth );
    }
  }
  else
  {
    if( GFL_CLACT_WK_CheckAnmActive( charaWork->clwkMouth ) == TRUE &&
        GFL_CLACT_WK_GetAnmIndex( charaWork->clwkMouth ) == 0 )
    {
      GFL_CLACT_WK_StopAnm( charaWork->clwkMouth );
    }
  }
  
  if( charaWork->lipDelayCnt > 0 )
  {
    charaWork->lipDelayCnt--;
  }
}

//--------------------------------------------------------------
//	絵の読み込み要求
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_LoadResource( LOCAL_TVT_CHARA *charaWork , const u8 bufNo)
{
  charaWork->state = LTCTS_CHARA1;
}

static u8 LOCAL_TVT_CHARA_GetCharaType( const LOCAL_TVT_CHARA *charaWork )
{
  return charaWork->charaType;
}
static u8 LOCAL_TVT_CHARA_GetBgType( const LOCAL_TVT_CHARA *charaWork )
{
  return charaWork->bgType;
}
static u32 LOCAL_TVT_CHARA_GetWeatherType( const LOCAL_TVT_CHARA *charaWork )
{
  return charaWork->weatherType;
}
//--------------------------------------------------------------
//	絵の読み込み完了チェック
//--------------------------------------------------------------
const BOOL LOCAL_TVT_CHARA_isFinishTrans( LOCAL_TVT_CHARA *charaWork )
{
  if( charaWork->state == LTCTS_WAIT_TRANS )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	絵の読み込み(パレット・スクリーン
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_LoadCommonResource( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, HEAPID heapID )
{
  u8  screenHeight;
  if( sys->member == LTM_2_MEMBER )
  {
    screenHeight = LTVT_CHARA_SCREEN_HEIGHT;
  }
  else
  {
    screenHeight = LTVT_CHARA_SCREEN_HEIGHT/2;
  }
  
  GFL_ARCHDL_UTIL_TransVramPaletteEx( sys->arcHandle , 
                                      charaResList[charaWork->charaType][0] ,
                                      PALTYPE_MAIN_BG_EX ,
                                      0 ,
                                      (2*16*16) * charaWork->charaIdx + 0x4000 ,
                                      (2*16*16) ,
                                      heapID );
  GFL_ARCHDL_UTIL_TransVramPaletteEx( sys->arcHandle , 
                                      bgResList[charaWork->bgType][charaWork->sys->param.timeZone].nclr,
                                      PALTYPE_MAIN_BG_EX ,
                                      0 ,
                                      (2*16*16) * charaWork->charaIdx + 0x6000 ,
                                      (2*16*16) ,
                                      heapID );

  GFL_BG_ChangeScreenPalette( LTVT_FRAME_CHARA , 
                              charaWork->charaIdx%2 * LTVT_CHARA_SCREEN_WIDTH,
                              charaWork->charaIdx/2 * screenHeight,
                              LTVT_CHARA_SCREEN_WIDTH , 
                              screenHeight , 
                              charaWork->charaIdx+LTVT_PLT_CHARA );
  GFL_BG_ChangeScreenPalette( LTVT_FRAME_BG , 
                              charaWork->charaIdx%2 * LTVT_CHARA_SCREEN_WIDTH,
                              charaWork->charaIdx/2 * screenHeight,
                              LTVT_CHARA_SCREEN_WIDTH , 
                              screenHeight , 
                              charaWork->charaIdx+LTVT_PLT_BG );

  GFL_BG_LoadScreenV_Req( LTVT_FRAME_BG );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_CHARA );
}


//--------------------------------------------------------------
//	絵の読み込み(キャラ
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_LoadCharaResource( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, HEAPID heapID )
{
  u32 charSize;
  u32 transOfs;
  if( sys->member == LTM_2_MEMBER )
  {
    charSize = 0x6000;
    transOfs = 0;
  }
  else
  {
    charSize = 0x3000;
    transOfs = LTVT_CHARA_OFFSET_SIZE;
  }

  LOCAL_TVT_CHARA_LoadNcgResource( charaWork , sys->arcHandle, charaResList[charaWork->charaType][1], heapID );
  LOCAL_TVT_CHARA_TransNcgResource( charaWork ,
                                    transOfs ,
                                    LTVT_VRAM_ADR_CHARA + charSize * charaWork->charaIdx ,
                                    charSize );
  GFL_HEAP_FreeMemory( charaWork->resData );
  
  LOCAL_TVT_CHARA_LoadNcgResource( charaWork , sys->arcHandle, bgResList[charaWork->bgType][sys->param.timeZone].ncgr, heapID );
  LOCAL_TVT_CHARA_TransNcgResource( charaWork ,
                                    transOfs ,
                                    LTVT_VRAM_ADR_BG + charSize * charaWork->charaIdx ,
                                    charSize );
  GFL_HEAP_FreeMemory( charaWork->resData );

  //目パチ口パク素材
  charaWork->cellRes[0] = GFL_CLGRP_PLTT_RegisterComp( sys->arcHandle , 
        charaResList[charaWork->charaType][2] , CLSYS_DRAW_MAIN , 
        (LTVT_PLT_OBJ_CHARA+(charaWork->charaIdx*2))*32 , heapID  );
  charaWork->cellRes[1] = GFL_CLGRP_CGR_Register( sys->arcHandle , 
        charaResList[charaWork->charaType][2]+1 , FALSE , CLSYS_DRAW_MAIN , heapID  );
  charaWork->cellRes[2] = GFL_CLGRP_CELLANIM_Register( sys->arcHandle , 
        charaResList[charaWork->charaType][2]+2 , 
        charaResList[charaWork->charaType][2]+3, heapID  );
  
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 1;
    cellInitData.pos_x = 64 + (charaWork->charaIdx%2)*128;
    cellInitData.pos_y = 48 + (charaWork->charaIdx/2)*96;
    if( sys->member == LTM_2_MEMBER )
    {
      cellInitData.pos_y += LTVT_CHARA_OFFSET*8;
    }
    cellInitData.anmseq = 0;

    charaWork->clwkEye = GFL_CLACT_WK_Create( sys->param.cellUnit ,
              charaWork->cellRes[1],
              charaWork->cellRes[0],
              charaWork->cellRes[2],
              &cellInitData ,CLSYS_DRAW_MAIN , heapID );
    GFL_CLACT_WK_SetDrawEnable( charaWork->clwkEye , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( charaWork->clwkEye , TRUE );
    GFL_CLACT_WK_StopAnm( charaWork->clwkEye );

    cellInitData.anmseq = 1;
    charaWork->clwkMouth = GFL_CLACT_WK_Create( sys->param.cellUnit ,
              charaWork->cellRes[1],
              charaWork->cellRes[0],
              charaWork->cellRes[2],
              &cellInitData ,CLSYS_DRAW_MAIN , heapID );
    GFL_CLACT_WK_SetDrawEnable( charaWork->clwkMouth , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( charaWork->clwkMouth , TRUE );
    GFL_CLACT_WK_StopAnm( charaWork->clwkMouth );
  }
}

static void LOCAL_TVT_CHARA_LoadNcgResource( LOCAL_TVT_CHARA *charaWork , ARCHANDLE *arcHandle, ARCDATID datId, HEAPID heapID )
{
  //ARI_TPrintf("[%d]L\n",charaWork->charaIdx);
  charaWork->resData = GFL_ARCHDL_UTIL_LoadBGCharacter( arcHandle, 
                                   datId ,
                                   FALSE ,
                                   &charaWork->charaData ,
                                   heapID );
//  DC_FlushAll();
//  DC_FlushRange( charaWork->charaData->pRawData , charaWork->charaData->szByte );
}


//--------------------------------------------------------------
//	絵の読み込み機能部(キャラ
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_TransNcgResource( LOCAL_TVT_CHARA *charaWork ,
                                                   const u32 topOfs , const u32 transOfs , const u32 transSize )
{
  //ARI_TPrintf("[%d]T\n",charaWork->charaIdx);
  GFL_STD_MemCopy32( (void*)((u32)charaWork->charaData->pRawData + topOfs) , (void*)(HW_BG_VRAM + transOfs ) , transSize );

}

//--------------------------------------------------------------
//	名前表示
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_DispName( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, HEAPID heapID )
{
  {
    //BMPWINの作成
    charaWork->nameWin = GFL_BMPWIN_Create( LTVT_FRAME_NAME , 
                                            LOCAL_TVT_CHARA_BASE_POS[charaWork->charaIdx][0] + 2,
                                            LOCAL_TVT_CHARA_BASE_POS[charaWork->charaIdx][1] + 1, 
                                            LTVT_CHARA_NAME_WIDTH , 
                                            LTVT_CHARA_NAME_HEIGHT , 
                                            LTVT_PLT_MAIN_FONT ,
                                            GFL_BMP_CHRAREA_GET_B );
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( charaWork->nameWin ) , 0x0 );
    GFL_BMPWIN_TransVramCharacter( charaWork->nameWin );
    GFL_BMPWIN_MakeScreen( charaWork->nameWin );
    GFL_BG_LoadScreenReq(LTVT_FRAME_NAME);
  }
  
  {
    //名前書く
    STRBUF *str = GFL_MSG_CreateString( sys->msgHandle , charaWork->charaType );
    GFL_FONT  *fontHandle = LOCAL_TVT_MESSAGE_GetFont( sys->param.message );
    PRINT_QUE *printQue   = LOCAL_TVT_MESSAGE_GetPrintQue( sys->param.message );
    if( charaWork->charaType == LTCT_PLAYER_M ||
        charaWork->charaType == LTCT_PLAYER_F )
    {
      WORDSET *wordSet;
      STRBUF *msgWorkStr = GFL_STR_CreateBuffer( 128 , heapID );
      MYSTATUS *mystatus = sys->param.mystatus;
      wordSet = WORDSET_Create( heapID );
      
      WORDSET_RegisterPlayerName( wordSet , 0 , mystatus );
      WORDSET_ExpandStr( wordSet , msgWorkStr , str );
      GFL_STR_DeleteBuffer( str );
      
      WORDSET_Delete( wordSet );
      
      str = msgWorkStr;
    }
    charaWork->nameLen = PRINTSYS_GetStrWidth( str , fontHandle , 0 );

    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( charaWork->nameWin ) ,
                            ((LTVT_CHARA_NAME_WIDTH*8) - charaWork->nameLen)/2+1 , 1 , str ,
                            fontHandle , PRINTSYS_LSB_Make( 0xf,0,0 ) );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( charaWork->nameWin ) ,
                            ((LTVT_CHARA_NAME_WIDTH*8) - charaWork->nameLen)/2-1 , 1 , str ,
                            fontHandle , PRINTSYS_LSB_Make( 0xf,0,0 ) );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( charaWork->nameWin ) ,
                            ((LTVT_CHARA_NAME_WIDTH*8) - charaWork->nameLen)/2 , 1+1 , str ,
                            fontHandle , PRINTSYS_LSB_Make( 0xf,0,0 ) );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( charaWork->nameWin ) ,
                            ((LTVT_CHARA_NAME_WIDTH*8) - charaWork->nameLen)/2 , 1-1 , str ,
                            fontHandle , PRINTSYS_LSB_Make( 0xf,0,0 ) );

    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( charaWork->nameWin ) ,
                            ((LTVT_CHARA_NAME_WIDTH*8) - charaWork->nameLen)/2 , 1 , str ,
                            fontHandle , PRINTSYS_LSB_Make( 1,0,0 ) );


//    PRINTSYS_PrintQueColor( sys->printQue , GFL_BMPWIN_GetBmp( charaWork->nameWin ) ,
//                            ((LTVT_CHARA_NAME_WIDTH*8) - charaWork->nameLen)/2 , 0 , str ,
//                            sys->fontHandle , PRINTSYS_LSB_Make( 1,0xf,0 ) );
    
    GFL_STR_DeleteBuffer( str );
    charaWork->isUpdateQue = TRUE;
    
  }
}

static void LOCAL_TVT_CHARA_RedrawName( LOCAL_TVT_CHARA *charaWork )
{
  GFL_BMPWIN_MakeScreen( charaWork->nameWin );
  GFL_BG_LoadScreenV_Req(LTVT_FRAME_NAME);
}

static const u8 LOCAL_TVT_CHARA_GetNameLen( LOCAL_TVT_CHARA *charaWork )
{
  return charaWork->nameLen;
}

static void LOCAL_TVT_CHARA_SetLipSync( LOCAL_TVT_CHARA *charaWork , const BOOL flg )
{
  if( charaWork->isLipSync == TRUE &&
      flg == FALSE )
  {
    charaWork->lipDelayCnt = LTVT_CHARA_LIP_DELAY_CNT;
  }
  charaWork->isLipSync = flg;
}

static void LOCAL_TVT_CHARA_LipSyncForceStop( LOCAL_TVT_CHARA *charaWork )
{
  charaWork->lipDelayCnt = 0;
  charaWork->isLipSync = FALSE;
}
static const void LOCAL_TVT_CHARA_LoadWeather( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_WEATHER weather, LOCAL_TVT_MEMBER member, HEAPID heapID )
{
  if( weather == LOCAL_TVT_WEATHER_SUNNY )
  {
    return;
  }

  //リソース読込み
  {
    u8 nclr;
    u8 ncgr;
    u8 ncer;
    u8 nanr;

    if( member  == LTM_2_MEMBER )
    {
      nclr  = weatherResList[weather][0].nclr;
      ncgr  = weatherResList[weather][0].ncgr;
      ncer  = weatherResList[weather][0].ncer;
      nanr  = weatherResList[weather][0].nanr;
    }
    else
    {
      nclr  = weatherResList[weather][1].nclr;
      ncgr  = weatherResList[weather][1].ncgr;
      ncer  = weatherResList[weather][1].ncer;
      nanr  = weatherResList[weather][1].nanr;
    }

    charaWork->weatherCellRes[LTCR_PLT] = GFL_CLGRP_PLTT_RegisterComp( sys->arcHandle, 
        nclr, CLSYS_DRAW_MAIN , (12+charaWork->charaIdx)*32 , heapID  );
    charaWork->weatherCellRes[LTCR_NCG] = GFL_CLGRP_CGR_Register( sys->arcHandle, 
        ncgr, FALSE , CLSYS_DRAW_MAIN , heapID  );
    charaWork->weatherCellRes[LTCR_ANM] = GFL_CLGRP_CELLANIM_Register( sys->arcHandle,  ncer, nanr, heapID  );
  }

  //ワーク作成
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 2;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.anmseq = 0;
    cellInitData.pos_x = sc_weather_pos[charaWork->charaIdx].x;
    cellInitData.pos_y = sc_weather_pos[charaWork->charaIdx].y;

    charaWork->weather = GFL_CLACT_WK_Create( sys->param.cellUnit ,
              charaWork->weatherCellRes[LTCR_NCG],
              charaWork->weatherCellRes[LTCR_PLT],
              charaWork->weatherCellRes[LTCR_ANM],
              &cellInitData ,CLSYS_DRAW_MAIN , heapID );
    GFL_CLACT_WK_SetDrawEnable( charaWork->weather, TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( charaWork->weather, TRUE );
    GFL_CLACT_WK_SetAnmFrame( charaWork->weather, GFUser_GetPublicRand0(FX32_ONE*4) );
  }
}
static const void LOCAL_TVT_CHARA_UnLoadWeather( LOCAL_TVT_CHARA *charaWork)
{
  if( charaWork->weather != NULL )
  {
    GFL_CLACT_WK_Remove( charaWork->weather );
    charaWork->weather  = NULL;
  }

  if( charaWork->weatherCellRes[LTCR_PLT] != 0xFF )
  {
    GFL_CLGRP_PLTT_Release( charaWork->weatherCellRes[LTCR_PLT] );
    charaWork->weatherCellRes[LTCR_PLT] = 0xFF;
  }
  if( charaWork->weatherCellRes[LTCR_NCG] != 0xFF )
  {
    GFL_CLGRP_CGR_Release( charaWork->weatherCellRes[LTCR_NCG] );
    charaWork->weatherCellRes[LTCR_NCG] = 0xFF;
  }
  if( charaWork->weatherCellRes[LTCR_ANM] != 0xFF )
  {
    GFL_CLGRP_CELLANIM_Release( charaWork->weatherCellRes[LTCR_ANM] );
    charaWork->weatherCellRes[LTCR_ANM] = 0xFF;
  }
}

//======================================================================
//	録音アイコン
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  録音アイコン読込み
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys システム
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_RECICON_Load( LOCAL_TVT_CHARA_SYS *sys )
{
  {
    ARCHANDLE *ctvtArcHandle = GFL_ARC_OpenDataHandle( ARCID_COMM_TVT_GRA , sys->heapID );
    sys->cellRes[LTCR_PLT] = GFL_CLGRP_PLTT_RegisterComp( ctvtArcHandle , 
        NARC_comm_tvt_tv_t_obj_NCLR , CLSYS_DRAW_MAIN , 
        LTVT_PLT_OBJ_MAIN*32 , sys->heapID  );
    sys->cellRes[LTCR_NCG] = GFL_CLGRP_CGR_Register( ctvtArcHandle , 
        NARC_comm_tvt_tv_t_upper_obj_NCGR , FALSE , CLSYS_DRAW_MAIN , sys->heapID  );
    sys->cellRes[LTCR_ANM] = GFL_CLGRP_CELLANIM_Register( ctvtArcHandle , 
        NARC_comm_tvt_tv_t_upper_obj_NCER , NARC_comm_tvt_tv_t_upper_obj_NANR, sys->heapID  );
    GFL_ARC_CloseDataHandle( ctvtArcHandle );
  }

  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 1;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.anmseq = 0;

    sys->clwkRecIcon = GFL_CLACT_WK_Create( sys->param.cellUnit ,
              sys->cellRes[LTCR_NCG],
              sys->cellRes[LTCR_PLT],
              sys->cellRes[LTCR_ANM],
              &cellInitData ,CLSYS_DRAW_MAIN , sys->heapID );
    GFL_CLACT_WK_SetDrawEnable( sys->clwkRecIcon , FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( sys->clwkRecIcon , TRUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  録音アイコン破棄
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys システム
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_RECICON_UnLoad( LOCAL_TVT_CHARA_SYS *sys )
{
  GFL_CLACT_WK_Remove( sys->clwkRecIcon );

  GFL_CLGRP_PLTT_Release( sys->cellRes[LTCR_PLT] );
  GFL_CLGRP_CGR_Release( sys->cellRes[LTCR_NCG] );
  GFL_CLGRP_CELLANIM_Release( sys->cellRes[LTCR_ANM] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  録音アイコン表示
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys  システム
 *	@param	LOCAL_TVT_CHARA_POS       表示場所
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_RECICON_Visible( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA_POS pos )
{
  LOCAL_TVT_CHARA *charaWork  = LOCAL_TVT_CHARA_SYS_GetWork( sys, pos );
  const u8 nameLen = LOCAL_TVT_CHARA_GetNameLen( charaWork);
  GFL_CLACTPOS clpos;
  clpos.x = LOCAL_TVT_CHARA_BASE_POS[pos][0]*8 + 64 + nameLen/2 + 16;
  clpos.y = LOCAL_TVT_CHARA_BASE_POS[pos][1]*8 + 16;
  GFL_CLACT_WK_SetPos( sys->clwkRecIcon , &clpos , CLSYS_DRAW_MAIN );
  GFL_CLACT_WK_SetDrawEnable( sys->clwkRecIcon , TRUE );
  GFL_CLACT_WK_ResetAnm( sys->clwkRecIcon );
}
//----------------------------------------------------------------------------
/**
 *	@brief  録音アイコン非表示
 *
 *	@param	LOCAL_TVT_CHARA_SYS *sys システム
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_RECICON_Hide( LOCAL_TVT_CHARA_SYS *sys )
{
  GFL_CLACT_WK_SetDrawEnable( sys->clwkRecIcon , FALSE );
}
