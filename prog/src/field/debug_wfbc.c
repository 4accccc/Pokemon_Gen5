//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_wfbc.c
 *	@brief  WFBCオートデバック機能
 *	@author	tomoya takahashi
 *	@date		2010.05.24
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

#include "debug_wfbc.h"

#include "field/field_wfbc_data.h" 

#include "fieldmap/zone_id.h"
#include "event_mapchange.h"

#include "system/bmp_winframe.h"

#include "arc_def.h"
#include "debug_message.naix"

#include "print/wordset.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "msg/debug/msg_d_tomoya.h"

#include "font/font.naix"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	全人物PATTERNチェック
//=====================================
typedef struct {

  FIELD_WFBC_CORE* p_wfbc_data;
  GAMESYS_WORK* p_gamesys;
  FIELDMAP_WORK* p_fieldmap;

  int p[10];

  int wait;

  int pattern_count;
  
} EVENT_DEBUG_WFBC_PEOPLE;

//-------------------------------------
///	WFBC_View
//=====================================
typedef struct {

  
  FIELD_WFBC_CORE wfbc_data_tmp;
  FIELD_WFBC_CORE* p_wfbc_data;
  GAMESYS_WORK* p_gamesys;
  FIELDMAP_WORK* p_fieldmap;

  WORDSET*    p_wordset;
  GFL_FONT*   p_font;
  GFL_MSGDATA*  p_msgdata;
  STRBUF*     p_strbuff;
  STRBUF*     p_strbuff_tmp;


  GFL_BMPWIN* p_win;

  u16 page;
  u16 change;
  
  
  HEAPID heapID;
  
} EVENT_DEBUG_WFBC_VIEW;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static GMEVENT_RESULT ev_DEBUG_WFBC_PeopleCheck( GMEVENT* p_event, int* p_seq, void* p_work );

static BOOL ev_DEBUG_WFBC_PeopleCheckNextID( u32 index, const EVENT_DEBUG_WFBC_PEOPLE* cp_wk, int* p_nextID );

static GMEVENT_RESULT ev_DEBUG_WFBC_100SetUp( GMEVENT* p_event, int* p_seq, void* p_work );


static GMEVENT_RESULT ev_DEBUG_WFBC_View( GMEVENT* p_event, int* p_seq, void* p_work );


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCcontrollerを起動
 *
 *	@param	gsys  ゲームシステム
 *
 *	@return イベントデータ
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_View( GAMESYS_WORK *gsys )
{
  EVENT_DEBUG_WFBC_VIEW* p_wk;
  GMEVENT* p_event;
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );

  p_event = GMEVENT_Create( gsys, NULL,
      ev_DEBUG_WFBC_View, sizeof(EVENT_DEBUG_WFBC_VIEW) );

  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_wfbc_data = GAMEDATA_GetMyWFBCCoreData( p_gdata );
  p_wk->p_gamesys = gsys;
  p_wk->p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  p_wk->page = 0;
  p_wk->heapID = FIELDMAP_GetHeapID( p_wk->p_fieldmap );

  return p_event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  人物全PATTERNチェック
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_PeopleCheck(
    GAMESYS_WORK *gsys )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk;
  GMEVENT* p_event;
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );

  p_event = GMEVENT_Create( gsys, NULL,
      ev_DEBUG_WFBC_PeopleCheck, sizeof(EVENT_DEBUG_WFBC_PEOPLE) );

  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_wfbc_data = GAMEDATA_GetMyWFBCCoreData( p_gdata );

  p_wk->p_gamesys = gsys;
  p_wk->p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  return p_event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  100回SetUpを読んでみる。
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_100SetUp(
    GAMESYS_WORK *gsys )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk;
  GMEVENT* p_event;
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );

  p_event = GMEVENT_Create( gsys, NULL,
      ev_DEBUG_WFBC_100SetUp, sizeof(EVENT_DEBUG_WFBC_PEOPLE) );

  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_wfbc_data = GAMEDATA_GetMyWFBCCoreData( p_gdata );

  p_wk->p_gamesys = gsys;
  p_wk->p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );


  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ev_DEBUG_WFBC_100SetUp( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk = p_work;
  int i;
  HEAPID heapID = FIELDMAP_GetHeapID( p_wk->p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_wk->p_gamesys );
  const MYSTATUS* cp_mystatus = GAMEDATA_GetMyStatus(p_gdata);

  for( i=0; i<100; i++ ){
    FIELD_WFBC_CORE_Clear( p_wk->p_wfbc_data );
    FIELD_WFBC_CORE_SetUp( p_wk->p_wfbc_data, cp_mystatus, heapID );
  }

  return GMEVENT_RES_FINISH;
}


//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  人物PATTERN総チェック
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ev_DEBUG_WFBC_PeopleCheck( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk = p_work;

  switch( *p_seq ){
  // 初期化
  case 0:
    {
      /*
      int i;
      for( i=0; i<10; i++ ){
        p_wk->p[i]  = i;
      }
      //*/
      
      //wfbcData[92607]:0 1 2 3 4 5 14 23 16 26  
      int p[10] = {0, 1, 2, 3, 4, 5, 14, 23, 16, 26};
      GFL_STD_MemCopy32( p, p_wk->p, sizeof(int)*10 );
    }
    p_wk->pattern_count = 92607;
    (*p_seq)++;
    break;

  // ジャンプ
  case 1:

    {
      int i;
      OS_SetPrintOutput_Arm9( 2 );
      TOMOYA_Printf( "wfbcData[%d]:", p_wk->pattern_count );
      // データ設定
      for( i=0; i<10; i++ ){
        p_wk->p_wfbc_data->people[i].data_in = TRUE;
        p_wk->p_wfbc_data->people[i].npc_id  = p_wk->p[i];
        p_wk->p_wfbc_data->people[i].mood  = 50;
        p_wk->p_wfbc_data->people[i].one_day_msk  = FIELD_WFBC_ONEDAY_MSK_INIT;
        TOMOYA_Printf( "%d ", p_wk->p[i] );
      }
      TOMOYA_Printf( "\n" );
      OS_SetPrintOutput_Arm9( 0 );
    }
    
    p_wk->wait = 10;
    {
      GMEVENT* p_subevent;
      p_subevent = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->p_gamesys, p_wk->p_fieldmap, ZONE_ID_BC10 );
      GMEVENT_CallEvent( p_event, p_subevent );
    }
    
    (*p_seq)++;
    break;

  // ウエイト
  case 2:
    p_wk->wait --;
    if( p_wk->wait <= 0 ){
      (*p_seq)++;
    }
    break;
    
  // 次のPATTERN
  case 3:
    {

      int check_index;
      int next_id;
      
      // カウントアップ
      for( check_index = 9; check_index>=0; check_index-- ){

        if( ev_DEBUG_WFBC_PeopleCheckNextID( check_index, p_wk, &next_id ) == FALSE ){
          p_wk->p[check_index] = next_id;
          break;
        }

        // check_indexの桁ではすべてチェックしたのでLOOPクリア
        p_wk->p[check_index] = -1;
      }

      // check_indexがマイナスになったら終わり
      if( check_index<0 ){

        OS_SetPrintOutput_Arm9( 2 );
        TOMOYA_Printf( "ALL WFBC PEOPLE CHECK END\n" );
        OS_SetPrintOutput_Arm9( 0 );
        return( GMEVENT_RES_FINISH );
      }

      // check_indexより大きいインデックスのワークを再設定
      for( check_index += 1; check_index<10; check_index++ ){
        GF_ASSERT( p_wk->p[check_index] == -1 );
        p_wk->p[check_index] = 0;
        if( ev_DEBUG_WFBC_PeopleCheckNextID( check_index, p_wk, &next_id ) == FALSE ){
          p_wk->p[check_index] = next_id;
        }else{
          GF_ASSERT( 0 ); // おかしい。
        }
      }
    }
    (*p_seq) = 1;
    p_wk->pattern_count++;
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}


//----------------------------------------------------------------------------
/**
 *	@brief  次のチェックIDを調べる
 *
 *	@param	index インデックス　０～９
 *	@param	p_wk      ワーク
 *	@param	p_nextID  次のID可能先
 *
 *	@retval p_nextIDがFIELD_WFBC_NPCID_MAXを超える
 */
//-----------------------------------------------------------------------------
static BOOL ev_DEBUG_WFBC_PeopleCheckNextID( u32 index, const EVENT_DEBUG_WFBC_PEOPLE* cp_wk, int* p_nextID )
{
  int i;
  int check;
  BOOL not_hit;
  BOOL ret = FALSE;

  
  check = cp_wk->p[index];

  do{
    check ++;
    not_hit = TRUE;

    for( i=0; i<10; i++ ){

      if( i != index ){
        if( cp_wk->p[i] == check ){
          not_hit = FALSE;
        }
      }
    } 
    
  }while( not_hit == FALSE );

  *p_nextID = check;
  
  if( check >= FIELD_WFBC_NPCID_MAX ){
    return TRUE;
  }
  return FALSE;
  
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBC状態ビューアー
 */
//-----------------------------------------------------------------------------
extern u8 DEBUG_WFBC_RandData[FIELD_WFBC_BLOCK_SIZE_X][FIELD_WFBC_BLOCK_SIZE_X];
extern u8 DEBUG_WFBC_LevelData[FIELD_WFBC_BLOCK_SIZE_X][FIELD_WFBC_BLOCK_SIZE_X];
static GMEVENT_RESULT ev_DEBUG_WFBC_View( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_DEBUG_WFBC_VIEW* p_wk = p_work;
  enum{
    SEQ_INIT,
    SEQ_UPDATE,
    SEQ_EXIT,
  };

  switch( *p_seq ){
  // 初期化処理
  case SEQ_INIT:
    {
      //サブ画面セットアップ
      // インフォーバーの非表示
      FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(p_wk->p_fieldmap));
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

      // ビットマップウィンドウ初期化
      {
        static const GFL_BG_BGCNT_HEADER header_sub3 = {
          0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
          GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
          GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
          GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
        };

        GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
        GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
        GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

        // パレット情報を転送
        GFL_ARC_UTIL_TransVramPalette(
          ARCID_FONT, NARC_font_default_nclr,
          PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, p_wk->heapID );

        // ビットマップウィンドウを作成
        p_wk->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
          1, 1, 30, 22,
          FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_win ), 0xf );
        GFL_BMPWIN_MakeScreen( p_wk->p_win );
        GFL_BMPWIN_TransVramCharacter( p_wk->p_win );
        GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

        // ウィンドウ
        BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, p_wk->heapID );
        BmpWinFrame_Write( p_wk->p_win, TRUE, 1, 15 );
      
        
        // ワードセット作成
        p_wk->p_wordset = WORDSET_Create( p_wk->heapID );
        p_wk->p_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_tomoya_dat, p_wk->heapID );

        p_wk->p_strbuff    = GFL_STR_CreateBuffer( 256, p_wk->heapID );
        p_wk->p_strbuff_tmp  = GFL_STR_CreateBuffer( 256, p_wk->heapID );

        // フォントデータ
        p_wk->p_font = GFL_FONT_Create(
          ARCID_FONT, NARC_font_large_gftr,
          GFL_FONT_LOADTYPE_FILE, FALSE, p_wk->heapID );
      }

    }
    (*p_seq) ++;
    break;

  // 描画処理
  case SEQ_UPDATE:
    // 終了
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*p_seq) ++;
      break;
    }

    // ページ切り替え
    if( GFL_UI_TP_GetTrg() ){
      p_wk->page = (p_wk->page + 1) % 3;
      p_wk->change = TRUE;
    }
    
    if( (p_wk->page == 2) && p_wk->change ){
      int i, j;
        
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_win ), 0xf );

          
      GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT07, p_wk->p_strbuff );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_wk->p_win ), 0, 0, p_wk->p_strbuff, p_wk->p_font );


      GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT06, p_wk->p_strbuff_tmp );

      for( i=0; i<FIELD_WFBC_BLOCK_SIZE_Z; i++ ){
        
        for( j=0; j<FIELD_WFBC_BLOCK_SIZE_X; j++ ){

          WORDSET_RegisterNumber( p_wk->p_wordset, 0, DEBUG_WFBC_LevelData[i][j], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
          WORDSET_RegisterNumber( p_wk->p_wordset, 1, DEBUG_WFBC_RandData[i][j], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

          WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuff, p_wk->p_strbuff_tmp );
          PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_wk->p_win ), j*30, 20+(i*16), p_wk->p_strbuff, p_wk->p_font );
          
        }
      }

      GFL_BMPWIN_TransVramCharacter( p_wk->p_win );

      p_wk->change = FALSE;
      
    }else{
      if( (GFL_STD_MemComp( p_wk->p_wfbc_data, &p_wk->wfbc_data_tmp, sizeof(FIELD_WFBC_CORE) ) != 0) || p_wk->change ){
        const FIELD_WFBC_CORE_PEOPLE* cp_array;
        int i;
        s16 pos_x, pos_y;
        u32 str_id;
    
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_win ), 0xf );

        // 表
        if( p_wk->page == 0 ){
          cp_array = p_wk->p_wfbc_data->people;
        }
        // 裏
        else{
          cp_array = p_wk->p_wfbc_data->back_people;
        }

        // タイトル描画
        if( (p_wk->page==0) && (p_wk->p_wfbc_data->type == FIELD_WFBC_CORE_TYPE_BLACK_CITY) ){
          
          GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT02, p_wk->p_strbuff );
        }else if( (p_wk->page==1) && (p_wk->p_wfbc_data->type == FIELD_WFBC_CORE_TYPE_BLACK_CITY) ){

          GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT03, p_wk->p_strbuff );
        }else if( (p_wk->page==0) && (p_wk->p_wfbc_data->type == FIELD_WFBC_CORE_TYPE_WHITE_FOREST) ){
          
          GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT00, p_wk->p_strbuff );
        }else if( (p_wk->page==1) && (p_wk->p_wfbc_data->type == FIELD_WFBC_CORE_TYPE_WHITE_FOREST) ){
          
          GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT01, p_wk->p_strbuff );
        }
        PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_wk->p_win ), 0, 0, p_wk->p_strbuff, p_wk->p_font );

        // 人データ描画
        for( i=0; i<FIELD_WFBC_PEOPLE_MAX; i++ ){

          pos_x = (i / (FIELD_WFBC_PEOPLE_MAX/2) ) * 120;
          pos_y = ((i % (FIELD_WFBC_PEOPLE_MAX/2) ) * 20) + 16;
          
          // １人描画
          WORDSET_RegisterNumber( p_wk->p_wordset, 0, i, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
          if( FIELD_WFBC_CORE_PEOPLE_IsInData( &cp_array[i] ) == FALSE ){
            GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT05, p_wk->p_strbuff_tmp );
          }else{
            WORDSET_RegisterNumber( p_wk->p_wordset, 1, cp_array[i].npc_id, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
            WORDSET_RegisterNumber( p_wk->p_wordset, 2, cp_array[i].mood, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
            GFL_MSG_GetString( p_wk->p_msgdata, WFBC_PRINT04, p_wk->p_strbuff_tmp );
          }
        
          WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuff, p_wk->p_strbuff_tmp );
          PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_wk->p_win ), pos_x, pos_y, p_wk->p_strbuff, p_wk->p_font );
        }

        //転送
        GFL_BMPWIN_TransVramCharacter( p_wk->p_win );


        // データコピー
        GFL_STD_MemCopy( p_wk->p_wfbc_data, &p_wk->wfbc_data_tmp, sizeof(FIELD_WFBC_CORE) );
        p_wk->change = FALSE;
      }
    }
    break;

  // 破棄処理
  case SEQ_EXIT:
    {
      // フォントデータ
      GFL_FONT_Delete( p_wk->p_font );
      p_wk->p_font = NULL;


      GFL_MSG_Delete( p_wk->p_msgdata );
      p_wk->p_msgdata = NULL;

      WORDSET_Delete( p_wk->p_wordset );
      p_wk->p_wordset = NULL;

      GFL_STR_DeleteBuffer( p_wk->p_strbuff );
      p_wk->p_strbuff = NULL;
      GFL_STR_DeleteBuffer( p_wk->p_strbuff_tmp );
      p_wk->p_strbuff_tmp = NULL;


      // ビットマップウィンドウ破棄
      {
        GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
        GFL_BMPWIN_Delete( p_wk->p_win );
      }

      // インフォーバーの表示
      FIELDMAP_SetFieldSubscreenWork(p_wk->p_fieldmap,
          FIELD_SUBSCREEN_Init( p_wk->heapID, p_wk->p_fieldmap, FIELD_SUBSCREEN_NORMAL ));

    }
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }

  return GMEVENT_RES_CONTINUE;

}

#endif //PM_DEBUG

