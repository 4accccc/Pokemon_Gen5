//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resonance_resort_data.c
 *	@brief  レゾナンスリゾートのデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.02.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include "gamesystem/game_data.h"
#include "savedata/questionnaire_save.h"
#include "net_app/union/union_beacon_tool.h"
#include "system/gfl_use.h"
#include "print/str_tool.h"
#include "savedata/misc.h"
#include "system/main.h"

//arc
#include "arc/arc_def.h"
#include "arc/script_message.naix"
#include "msg/script/msg_resort_scr.h"

//mine 
#include "field/resonance_resort/resort_map.h"

//外部公開
#include "field/resonance_resort_data.h"


#ifdef PM_DEBUG
#define DEBUG_RESORT_PRINT_ON
#endif

#if defined(DEBUG_RESORT_PRINT_ON) && ( defined(DEBUG_ONLY_FOR_nagihashi_toru) || defined(DEBUG_ONLY_FOR_shimoyamada) )
#define RESORT_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#else
#define RESORT_Printf(...)  /* */
#endif


#define RESORT_VALID_ASSERT(...) GF_ASSERT_MSG_LIGHT( 0, __VA_ARGS__ );

static inline RESORT_GET_TrainerView( u8 view, u8 sex ) 
{
  if( sex == PM_MALE )
  {
    return view;
  }
  else
  {
    return view + UNION_VIEW_INDEX_WOMAN_START;
  }
}


//=============================================================================
/**
 *  施設データ
 */
//=============================================================================
//-------------------------------------
///	施設データ
//=====================================
struct _RESORT_INSTITUTION_DATA 
{
  u8  institutionID;
  u8  lv;
  u16 exp;
  u32 ev_bit;
  u32 work;
};

SDK_COMPILER_ASSERT( sizeof(RESORT_INSTITUTION_DATA) == RESORT_INSTITUTION_DATA_SIZE );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ初期化
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return 施設データ
 */
//-----------------------------------------------------------------------------
RESORT_INSTITUTION_DATA * RESORT_INSTITUTION_DATA_Init( HEAPID heapID )
{
  RESORT_INSTITUTION_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_INSTITUTION_DATA));
  RESORT_INSTITUTION_DATA_InitInstance(p_wk);
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ破棄
 *
 *	@param	RESORT_INSTITUTION_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_INSTITUTION_DATA_Exit( RESORT_INSTITUTION_DATA *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ初期化
 *
 *	@param	RESORT_INSTITUTION_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_INSTITUTION_DATA_InitInstance( RESORT_INSTITUTION_DATA *p_wk )
{
  GFL_STD_MemClear(p_wk,sizeof(RESORT_INSTITUTION_DATA));
  p_wk->institutionID = RESORT_RESIDENT_TYPE_MAX;
}
//----------------------------------------------------------------------------
/**
 *	@brief  空かどうかチェック
 *
 *	@param	const RESORT_INSTITUTION_DATA *cp_wk  ワーク
 *
 *	@return TRUE空  FALSE空ではない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_INSTITUTION_DATA_IsEmpty( const RESORT_INSTITUTION_DATA *cp_wk )
{
  return cp_wk->institutionID == RESORT_RESIDENT_TYPE_MAX;
}

//----------------------------------------------------------------------------
/**
 *	@brief  内部データ取得
 *
 *	@param	const RESORT_INSTITUTION_DATA *cp_wk  ワーク
 *	@param	param   取得するデータ
 *
 *	@return　データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_INSTITUTION_DATA_GetData( const RESORT_INSTITUTION_DATA *cp_wk, RESORT_INSTITUTION_DATA_PARAM param )
{
  switch( param )
  {
  case RESORT_INSTITUTION_DATA_PARAM_ID:
    return cp_wk->institutionID;
  case RESORT_INSTITUTION_DATA_PARAM_LV:
    return cp_wk->lv;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief　内部データ設定
 *
 *	@param	RESORT_INSTITUTION_DATA *p_wk ワーク
 *	@param	param 設定するデータ
 *	@param	datga 値
 */
//-----------------------------------------------------------------------------
void RESORT_INSTITUTION_DATA_SetData( RESORT_INSTITUTION_DATA *p_wk, RESORT_INSTITUTION_DATA_PARAM param, u32 data )
{
  switch( param )
  {
  case RESORT_INSTITUTION_DATA_PARAM_ID:
    p_wk->institutionID   = data;
    break;
  case RESORT_INSTITUTION_DATA_PARAM_LV:
    p_wk->lv   = data;
    break;
  default:
    GF_ASSERT(0);
  }
}

SDK_COMPILER_ASSERT( RESORT_INSTITUTION_DATA_BIT_MAX < 32 );
//----------------------------------------------------------------------------
/**
 *	@brief  イベント用ビットを取得
 *
 *	@param	const RESORT_INSTITUTION_DATA *cp_wk  ワーク
 *	@param	bit   イベントビット番号
 *
 *	@return TRUEならばON  FASEならばOFF
 */
//-----------------------------------------------------------------------------
BOOL RESORT_INSTITUTION_DATA_IsBit( const RESORT_INSTITUTION_DATA *cp_wk, RESORT_INSTITUTION_DATA_BIT bit )
{
  return (cp_wk->ev_bit & (1<<bit)) != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  イベント用ビットを取得
 *
 *	@param	const RESORT_INSTITUTION_DATA *cp_wk  ワーク
 *	@param	bit   イベントビット番号
 *	@param  enable TRUEでON　FALSEでOFF
 *
 */
//-----------------------------------------------------------------------------
void RESORT_INSTITUTION_DATA_SetBit( RESORT_INSTITUTION_DATA *p_wk, RESORT_INSTITUTION_DATA_BIT bit, BOOL enable )
{
  if( enable )
  {
    p_wk->ev_bit |= (1<<bit);
  }
  else
  {
    p_wk->ev_bit &= ~(1<<bit);
  }
}


//=============================================================================
/**
 *	リゾート用基本、拡張データ
*/
//=============================================================================
//-------------------------------------
/// 基本データ１
//=====================================
typedef struct
{
  STRCODE name[PERSON_NAME_SIZE];   ///<トレーナー名(!! EOM除く !!) 14
  u16 padding3;
  u8  nation;                       ///<国コード      1 15
  u8  area;                         ///<地域コード    1 16
  u8  work;                         ///<仕事          1 17
  u8  hobby;                        ///<趣味          1 18
  STRCODE introduction[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN]; //<自己紹介(!! EOM除く !!) 16 34
  u8  version;                      ///<バージョン    1 35
  u8  language;                     ///<言語コード    1 36
  u8  trainer_view  :4;                 ///<見た目        1 37
  u8  sex           :1;
  u8  padding1      :3;
  u8  wnd_color;                    ///ウィンドウ背景色   1 38
  u32 pass_cnt;                     //すれ違い回数        4 42
  u16 trainerID;                    ///<トレーナーID(下位16ビット)  2 44
  u16 padding2;                       ///< 2 46
  QUESTIONNAIRE_ANSWER_WORK qanswer;  ///<アンケート  10  56
  u16 play_hour     :10;  ///< プレイ時間 2 58
  u16 play_min      :6;
}RESORT_BASIC1_DATA;

SDK_COMPILER_ASSERT( sizeof(RESORT_BASIC1_DATA) == RESORT_BASIC1_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  基本データ１に自分のデータを設定
 *
 *	@param	RESORT_BASIC1_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC1_DATA_SetMyData( RESORT_BASIC1_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_BASIC1_DATA) );
  //MYSTATUS
  {
    MYSTATUS *p_mystatus  = GAMEDATA_GetMyStatus((GAMEDATA*)cp_gamedata);
    //EOMが入らないサイズを取得するので、自分でコピーする
    GFL_STD_MemCopy( MyStatus_GetMyName(p_mystatus), p_wk->name, sizeof(STRCODE)*PERSON_NAME_SIZE );
    p_wk->trainerID = MyStatus_GetID_Low( p_mystatus );
    p_wk->nation    = MyStatus_GetMyNation(p_mystatus);
    p_wk->area      = MyStatus_GetMyArea(p_mystatus);
    p_wk->sex       = MyStatus_GetMySex(p_mystatus);
    p_wk->version   = GET_VERSION();
    p_wk->language  = PM_LANG;

    if( p_wk->sex == PM_MALE )
    {
      p_wk->trainer_view = MyStatus_GetTrainerView(p_mystatus);
    }
    else
    {
      p_wk->trainer_view = MyStatus_GetTrainerView(p_mystatus) - UNION_VIEW_INDEX_WOMAN_START;
    }
  }

  {
    MISC *p_misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork((GAMEDATA*)cp_gamedata) );
    const STRCODE *cp_src = MISC_CrossComm_GetSelfIntroduction(p_misc);
    GFL_STD_MemCopy( cp_src, p_wk->introduction, sizeof(STRCODE)*GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN );

    p_wk->pass_cnt  = MISC_CrossComm_GetSuretigaiCount(p_misc);
  }

  //アンケート
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork((GAMEDATA*)cp_gamedata);
    QUESTIONNAIRE_SAVE_WORK *p_ques_sv = SaveData_GetQuestionnaire(p_sv);

    GFL_STD_MemCopy( Questionnaire_GetAnswerWork(p_ques_sv), &p_wk->qanswer, sizeof(QUESTIONNAIRE_ANSWER_WORK) );
  }

  //プレイ時間
  {
    PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( (GAMEDATA*)cp_gamedata );
    p_wk->play_hour      = PLAYTIME_GetHour( sv_playtime );
    p_wk->play_min       = PLAYTIME_GetMinute( sv_playtime );
  }

  //本体情報
  {
    OSOwnerInfo owner_info;
    OS_GetOwnerInfo(&owner_info);
    p_wk->wnd_color = owner_info.favoriteColor;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  基本データ１にビーコンからのデータを設定
 *
 *	@param	RESORT_BASIC1_DATA *p_wk  ワーク
 *	@param	GAMEBEACON_INFO   ビーコン
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC1_DATA_SetBeaconInfo( RESORT_BASIC1_DATA *p_wk, const GAMEBEACON_INFO *cp_beacon_info )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_BASIC1_DATA) );
  
  GFL_STD_MemCopy( cp_beacon_info->name, p_wk->name, sizeof(STRCODE)*PERSON_NAME_SIZE );
  p_wk->trainerID = cp_beacon_info->trainer_id;
  p_wk->sex       = cp_beacon_info->sex;
  p_wk->version   = cp_beacon_info->pm_version;
  p_wk->language  = cp_beacon_info->language;
  p_wk->nation    = cp_beacon_info->nation;
  p_wk->area      = cp_beacon_info->area;
  p_wk->trainer_view  = cp_beacon_info->trainer_view;
  p_wk->qanswer   = cp_beacon_info->question_answer;
  p_wk->play_hour = cp_beacon_info->play_hour;
  p_wk->play_min  = cp_beacon_info->play_min;
  p_wk->pass_cnt  = cp_beacon_info->suretigai_count;
  p_wk->wnd_color = cp_beacon_info->favorite_color_index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  基本データ１の正当性チェック
 *
 *	@param	const RESORT_BASIC1_DATA *cp_wk ワーク
 *
 *	@return TRUE正当  FALSE不正
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_BASIC1_DATA_IsValid( const RESORT_BASIC1_DATA *cp_wk )
{
  //性別チェック
  if( cp_wk->sex != PM_FEMALE && cp_wk->sex != PM_MALE )
  {
    RESORT_VALID_ASSERT( "基本データ１：性別が不正です\n" );
    return FALSE;
  }
  //言語チェックはローカライズを増やす可能性を考えてやらない
#if 0
  if( cp_wk->language )
  {
    RESORT_VALID_ASSERT( "基本データ１：言語が不正です\n" );
    return FALSE;
  }
#endif
  //見た目
  if( cp_wk->trainer_view > UNION_VIEW_INDEX_MAN_END )
  {
    RESORT_VALID_ASSERT( "基本データ１：見た目が不正です\n" );
    return FALSE;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  同じ所有者かチェック
 *
 *	@param	const RESORT_BASIC1_DATA *cp_a  比較１
 *	@param	RESORT_BASIC1_DATA *cp_b      比較２
 *
 *	@return TRUEで同じ所有者  FALSEで異なる所有者
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_BASIC1_DATA_IsSameOwner( const RESORT_BASIC1_DATA *cp_a, const RESORT_BASIC1_DATA *cp_b )
{
  BOOL is_trainerID = (cp_a->trainerID == cp_b->trainerID);
  BOOL is_name  = (GFL_STD_MemComp( cp_a->name, cp_b->name ,  sizeof(STRCODE)*PERSON_NAME_SIZE) == 0);
  BOOL is_sex   = (cp_a->sex == cp_b->sex);

  return is_trainerID && is_name && is_sex;
}

//----------------------------------------------------------------------------
/**
 *	@brief  空かチェック
 *
 *	@param	const RESORT_BASIC1_DATA *cp_data データ
 *
 *	@return TRUEで空  FALSEで存在
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_BASIC1_DATA_IsEmpty( const RESORT_BASIC1_DATA *cp_data )
{
  return cp_data->version == 0 || cp_data->name[0]  == 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief 名前をEOM付きで取得
 *
 *	@param	const RESORT_BASIC1_DATA *cp_data ワーク
 *	@param	*p_str  受け取りバッファ
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC1_DATA_GetName( const RESORT_BASIC1_DATA *cp_data, STRCODE *p_str )
{
  GFL_STD_MemCopy( cp_data->name, p_str, sizeof(STRCODE)*PERSON_NAME_SIZE);
  *((p_str)+PERSON_NAME_SIZE) = GFL_STR_GetEOMCode();
}

//----------------------------------------------------------------------------
/**
 *	@brief 名前を挨拶付きで取得
 *
 *	@param	const RESORT_BASIC1_DATA *cp_data ワーク
 *	@param	*p_str  受け取りバッファ
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC1_DATA_GetIntroduction( const RESORT_BASIC1_DATA *cp_data, STRCODE *p_str )
{
  GFL_STD_MemCopy( cp_data->introduction, p_str, sizeof(STRCODE)*GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN);
  *((p_str)+GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN) = GFL_STR_GetEOMCode();
}

#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ表示
 *
 *	@param	const RESORT_BASIC1_DATA *cp_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC1_DATA_PrintDebug( const RESORT_BASIC1_DATA *cp_wk )
{
  STRCODE name[PERSON_NAME_SIZE+1];
  char str_name[PERSON_NAME_SIZE+1];

  GFL_STD_MemCopy( cp_wk->name, name, PERSON_NAME_SIZE*sizeof(STRCODE) );
  name[PERSON_NAME_SIZE]  = GFL_STR_GetEOMCode();

  GFL_STR_CONV_StrcodeToSjis( name,str_name,PERSON_NAME_SIZE+1 );
  RESORT_Printf( "name=%s\n", str_name );
  RESORT_Printf( "trainerID=%d\n", cp_wk->trainerID );
}
#endif

//-------------------------------------
///	基本データ２
//=====================================
typedef struct
{
  u8  continue_data :1;
  u8  personality   :5;
  u8  special_deliverly :1;
  u8  padding1      :1;
  u8  quest1        :4;
  u8  quest2        :4;
  u8  quest3        :4;
  u8  quest4        :4;
  u8  padding2;
              //TODO WBから受け取ったときは中身がからなので何らかの認識をすること
}RESORT_BASIC2_DATA;

SDK_COMPILER_ASSERT( sizeof(RESORT_BASIC2_DATA) == RESORT_BASIC2_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  基本データ2に自分のデータを設定
 *
 *	@param	RESORT_BASIC2_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC2_DATA_SetMyData( RESORT_BASIC2_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_BASIC2_DATA) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  基本データ2の正当性チェック
 *
 *	@param	const RESORT_BASIC2_DATA *cp_wk ワーク
 *
 *	@return TRUE正当  FALSE不正
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_BASIC2_DATA_IsValid( const RESORT_BASIC2_DATA *cp_wk )
{
  return TRUE;
}

//-------------------------------------
///	拡張データ１
//=====================================
typedef struct
{
  u8  medalID;
  u8  medal_num;        
  u8  medal_custom[14];

} RESORT_EXTEND1_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND1_DATA) == RESORT_EXTEND1_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ1に自分のデータを設定
 *
 *	@param	RESORT_EXTEND1_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND1_DATA_SetMyData( RESORT_EXTEND1_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND1_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ1の正当性チェック
 *
 *	@param	RESORT_EXTEND1_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND1_DATA_IsValid( const RESORT_EXTEND1_DATA *cp_wk )
{
  return TRUE;
}

//-------------------------------------
///	拡張データ2
//=====================================
typedef struct
{
  u32 rank_data[4]; //１～４
} RESORT_EXTEND2_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND2_DATA) == RESORT_EXTEND2_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ2に自分のデータを設定
 *
 *	@param	RESORT_BASIC2_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND2_DATA_SetMyData( RESORT_EXTEND2_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND2_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ2の正当性チェック
 *
 *	@param	RESORT_EXTEND2_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND2_DATA_IsValid( const RESORT_EXTEND2_DATA *cp_wk )
{
  return TRUE;
}

//-------------------------------------
///	拡張データ3 
//=====================================
typedef RESORT_EXTEND2_DATA RESORT_EXTEND3_DATA;  //２と３は同じ構造体（ただし中身が違う）
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND3_DATA) == RESORT_EXTEND3_DATA_SIZE );
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ3に自分のデータを設定
 *
 *	@param	RESORT_BASIC3_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND3_DATA_SetMyData( RESORT_EXTEND3_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND3_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ3の正当性チェック
 *
 *	@param	RESORT_EXTEND3_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND3_DATA_IsValid( const RESORT_EXTEND3_DATA *cp_wk )
{
  return TRUE;
}
//-------------------------------------
///	拡張データ4
//=====================================
typedef struct
{
  u8  gossip[16];
} RESORT_EXTEND4_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND4_DATA) == RESORT_EXTEND4_DATA_SIZE );
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ4に自分のデータを設定
 *
 *	@param	RESORT_BASIC4_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND4_DATA_SetMyData( RESORT_EXTEND4_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND4_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ4の正当性チェック
 *
 *	@param	RESORT_EXTEND4_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND4_DATA_IsValid( const RESORT_EXTEND4_DATA *cp_wk )
{
  return TRUE;
}
//-------------------------------------
///	拡張データ5
//=====================================
typedef struct
{
  u16 monsno;
  u8  form  :5;
  u8  sex   :2;
  u8  rare  :1;
  u8  tokusei;
  u16 waza[4];
  u16 item;
  u16 pad;
} RESORT_EXTEND5_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND5_DATA) == RESORT_EXTEND5_DATA_SIZE );
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ5に自分のデータを設定
 *
 *	@param	RESORT_BASIC5_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND5_DATA_SetMyData( RESORT_EXTEND5_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND5_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ5の正当性チェック
 *
 *	@param	RESORT_EXTEND5_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND5_DATA_IsValid( const RESORT_EXTEND5_DATA *cp_wk )
{
  return TRUE;
}
//-------------------------------------
///	拡張データ6
//=====================================
typedef RESORT_EXTEND5_DATA RESORT_EXTEND6_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND6_DATA) == RESORT_EXTEND6_DATA_SIZE );
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ6に自分のデータを設定
 *
 *	@param	RESORT_BASIC6_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND6_DATA_SetMyData( RESORT_EXTEND6_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND6_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ6の正当性チェック
 *
 *	@param	RESORT_EXTEND6_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND6_DATA_IsValid( const RESORT_EXTEND6_DATA *cp_wk )
{
  return TRUE;
}
//-------------------------------------
///	拡張データ7
//=====================================
typedef RESORT_EXTEND5_DATA RESORT_EXTEND7_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND7_DATA) == RESORT_EXTEND7_DATA_SIZE );
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ7に自分のデータを設定
 *
 *	@param	RESORT_BASIC7_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND7_DATA_SetMyData( RESORT_EXTEND7_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND7_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ7の正当性チェック
 *
 *	@param	RESORT_EXTEND7_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND7_DATA_IsValid( const RESORT_EXTEND7_DATA *cp_wk )
{
  return TRUE;
}
//-------------------------------------
///	拡張データ8
//=====================================
typedef struct
{
  u8  record[16];
} RESORT_EXTEND8_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND8_DATA) == RESORT_EXTEND8_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ8に自分のデータを設定
 *
 *	@param	RESORT_BASIC8_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND8_DATA_SetMyData( RESORT_EXTEND8_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND8_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ8の正当性チェック
 *
 *	@param	RESORT_EXTEND7_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND8_DATA_IsValid( const RESORT_EXTEND8_DATA *cp_wk )
{
  return TRUE;
}

//-------------------------------------
///	拡張データ9
//=====================================
typedef struct
{
  STRCODE  phrase[RESORT_PHRASE_LEN]; //EOMなし
} RESORT_EXTEND9_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND9_DATA) == RESORT_EXTEND9_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ9に自分のデータを設定
 *
 *	@param	RESORT_BASIC9_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND9_DATA_SetMyData( RESORT_EXTEND9_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND9_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ9の正当性チェック
 *
 *	@param	RESORT_EXTEND7_DATA *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND9_DATA_IsValid( const RESORT_EXTEND9_DATA *cp_wk )
{
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief 口癖をEOM付きで取得
 *
 *	@param	const RESORT_BASIC1_DATA *cp_data ワーク
 *	@param	*p_str  受け取りバッファ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND9_DATA_GetPhrase( const RESORT_EXTEND9_DATA *cp_data, STRCODE *p_str )
{
  GFL_STD_MemCopy( cp_data->phrase, p_str, sizeof(STRCODE)*RESORT_PHRASE_LEN);
  *((p_str)+RESORT_PHRASE_LEN) = GFL_STR_GetEOMCode();
}

//-------------------------------------
///	拡張データ関数
//=====================================
typedef void (*RESORT_EXTEND_DATA_SetMyData)( void *p_wk_adrs, const GAMEDATA *cp_gamedata );
static const RESORT_EXTEND_DATA_SetMyData sc_ResortExtendData_SetMyData[RESORT_EXTEND_TYPE_MAX] = 
{
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND1_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND2_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND3_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND4_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND5_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND6_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND7_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND8_DATA_SetMyData,
  (RESORT_EXTEND_DATA_SetMyData)RESORT_EXTEND9_DATA_SetMyData,
};

typedef BOOL (*RESORT_EXTEND_DATA_IsValid)( const void *cp_wk_adrs );

static const RESORT_EXTEND_DATA_IsValid sc_ResortExtendData_IsValid[RESORT_EXTEND_TYPE_MAX] =
{
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND1_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND2_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND3_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND4_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND5_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND6_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND7_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND8_DATA_IsValid,
  (RESORT_EXTEND_DATA_IsValid)RESORT_EXTEND9_DATA_IsValid,
};

//=============================================================================
/**
 *    データ定義
 */
//=============================================================================
//-------------------------------------
///	１…リゾート用全受信データ
//=====================================
struct _RESORT_RECV_DATA
{
  RESORT_BASIC1_DATA  basic1;
  RESORT_BASIC2_DATA  basic2;
  RESORT_EXTEND1_DATA extend1;
  RESORT_EXTEND2_DATA extend2;
  RESORT_EXTEND3_DATA extend3;
  RESORT_EXTEND4_DATA extend4;
  RESORT_EXTEND5_DATA extend5;
  RESORT_EXTEND6_DATA extend6;
  RESORT_EXTEND7_DATA extend7;
  RESORT_EXTEND8_DATA extend8;
  RESORT_EXTEND9_DATA extend9;
  u16                 presentID;
  u8                  precent_num;
  u8                  meet_year;         //4
  u8                  meet_month;
  u8                  meet_day;
  u8                  meet_hour;
  u8                  meet_min;       //8
  u8                  commID;   //通信識別ID
  u8                  merge     :1;
  u8                  padding1  :5;
  u8                  day_flag  :1;
  u8                  keep_flag :1;
  u16                 padding2;
  RESORT_INSTITUTION_DATA institution;
#if 0
  u8                  lv;
  u8                  institutionID;//12
  u8                  ev_work1;
  u8                  ev_work2; //16
  u32                 ev_bit; //20
#endif
  u32                 kosei;  //TODO　個性？  //16
  u32                 extend_flag:8;  //20
  u32                 lock      :1;
  u32                 update    :2; //RESORT_RECV_DATA_UPDATEの値 
  u32                 peopleID  :8;
  u32                 placeID   :8;
  u32                 type     :5;  //自分がなんのタイプRESORT_RESIDENT_TYPE_xxx
};
SDK_COMPILER_ASSERT( sizeof(RESORT_RECV_DATA) == RESORT_RECV_DATA_SIZE );

//-------------------------------------
///	２…リゾート用ビーコンパックデータ
//=====================================
struct _RESORT_BEACON_DATA
{
  RESORT_BASIC1_DATA  basic1;
  RESORT_BASIC2_DATA  basic2;
  u8                  extend[RESORT_EXTEND_DATA_SIZE];
  RESORT_EXTEND_TYPE  extend_type;
};

//-------------------------------------
///	３…リゾート用代理データ
//=====================================
struct _RESORT_PROXY_DATA
{
  u8    type;
  u8    present_num;
  u8    placeID;
  u8    peopleID;
  u16   presentID;
  u16   dummy;
};

SDK_COMPILER_ASSERT(sizeof(RESORT_PROXY_DATA)==RESORT_PROXY_DATA_SIZE);

//=============================================================================
/**
 *    RESORT_RECV_DATAの関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  受信データ  初期化
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return 受信データハンドル
 */
//-----------------------------------------------------------------------------
RESORT_RECV_DATA * RESORT_RECV_DATA_Init( HEAPID heapID )
{
  RESORT_RECV_DATA *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_RECV_DATA));
  RESORT_RECV_DATA_InitInstance( p_wk );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データ  破棄
 *
 *	@param	RESORT_RECV_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_Exit( RESORT_RECV_DATA *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データ  自分のデータを作成
 *
 *	@param	RESORT_RECV_DATA *p_wk ワーク
 *	@param  GAMEDATA  ゲームデータ
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_SetMyData( RESORT_RECV_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  RESORT_BASIC1_DATA_SetMyData( &p_wk->basic1, cp_gamedata );
  RESORT_BASIC2_DATA_SetMyData( &p_wk->basic2, cp_gamedata );

  {
    int i;
    u8  *p_extend_adrs  = (u8*)&p_wk->extend1;
    for( i = 0; i < RESORT_EXTEND_TYPE_MAX; i++ )
    {
      sc_ResortExtendData_SetMyData[i]( (void*)p_extend_adrs, cp_gamedata );
      p_extend_adrs += RESORT_EXTEND_DATA_SIZE;
    }
  }

  p_wk->extend_flag = 0xFF;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データ  初期化
 *
 *	@param	RESORT_RECV_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_InitInstance( RESORT_RECV_DATA *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_RECV_DATA) );

  RESORT_INSTITUTION_DATA_InitInstance( &p_wk->institution );
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データ  中身が空かチェック
 *
 *	@param	const RESORT_RECV_DATA *cp_wk   ワーク
 *
 *	@return TRUEで空  FALSEでデータあり
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_IsEmpty( const RESORT_RECV_DATA *cp_wk )
{
  return cp_wk->extend_flag == 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じ所有者かチェック
 *
 *	@param	const RESORT_RECV_DATA *cp_wk   ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   ビーコン
 *
 *	@return TRUEで同じ所有者  FALSEで異なる所有者
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_IsSameOwnerBeacon( const RESORT_RECV_DATA *cp_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  return RESORT_BASIC1_DATA_IsSameOwner( &cp_wk->basic1, &cp_beacon->basic1 );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンをマージ
 *
 *	@param	RESORT_RECV_DATA *p_wk          ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   マージするビーコン
 *
 *	@retval TRUEマージした  FALSEマージしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_MergeBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  //ロック
  if( RESORT_RECV_DATA_GetData(p_wk,RESORT_DATA_PARAM_LOCK,NULL) )
  {
    return FALSE;
  }
  //不正対策
  if( !RESORT_BEACON_DATA_IsValid( cp_beacon ) )
  {
    return FALSE;
  }

  if( RESORT_RECV_DATA_IsEmpty( p_wk ) )
  {
    RESORT_RECV_DATA_InitInstance( p_wk );

    //空だったら、全部いれる
    p_wk->basic1  = cp_beacon->basic1;

    //中身があったら入れる
    if( cp_beacon->extend_type !=  RESORT_EXTEND_TYPE_NULL )
    {
      p_wk->basic2  = cp_beacon->basic2;
    
      //入れる処理
      //TODO

      //拡張マージフラグ
      p_wk->extend_flag |=  1<<cp_beacon->extend_type;
    }

    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_FIRST);

    if( p_wk->extend_flag == 0xFF )
    {
      RESORT_Printf( "!!!すべてマージ完了\n!!!" );
    }
  }
  else
  {
    //空だったら、全部いれる
    p_wk->basic1  = cp_beacon->basic1;


    //中身があったら入れる
    if( cp_beacon->extend_type !=  RESORT_EXTEND_TYPE_NULL )
    {
      p_wk->basic2  = cp_beacon->basic2;

      //入れる処理
      //TODO

      //拡張マージフラグ
      p_wk->extend_flag |=  1<<cp_beacon->extend_type;
    };

    if( p_wk->extend_flag == 0xFF )
    {
      RESORT_Printf( "!!!すべてマージ完了\n!!!" );
    }

    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE);
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データをマージ
 *
 *	@param	RESORT_RECV_DATA *p_wk      ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信データ
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_MergeRecv( RESORT_RECV_DATA *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  //ロック
  if( RESORT_RECV_DATA_GetData(p_wk,RESORT_DATA_PARAM_LOCK,NULL) )
  {
    return;
  }

  //不正対策
  if( !RESORT_RECV_DATA_IsValid( cp_recv ) )
  {
    return;
  }

  if( RESORT_RECV_DATA_IsEmpty( p_wk ) )
  {
    GFL_STD_MemCopy( cp_recv, p_wk, sizeof(RESORT_RECV_DATA) );

    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_FIRST);
  }
  else
  {
    //TODO
    //マージ処理

    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データの正当性をチェック
 *
 *	@param	const RESORT_RECV_DATA *cp_wk   ワーク
 *
 *	@return TRUE正当  FALSE不正
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_IsValid( const RESORT_RECV_DATA *cp_wk )
{
  if( !RESORT_BASIC1_DATA_IsValid( &cp_wk->basic1 ) )
  {
    RESORT_VALID_ASSERT( "受信データの基本データ１が不正です\n" );
    return FALSE;
  }
  if( !RESORT_BASIC2_DATA_IsValid( &cp_wk->basic2 ) )
  {
    RESORT_VALID_ASSERT( "受信データの基本データ２が不正です\n" );
    return FALSE;
  }
  //拡張データのチェック
  {
    int i;
    u8  *p_extend_adrs  = (u8*)&cp_wk->extend1;
    for( i = 0; i < RESORT_EXTEND_TYPE_MAX; i++ )
    {
      if( !sc_ResortExtendData_IsValid[i]( (void*)p_extend_adrs ) )
      {
        RESORT_VALID_ASSERT( "受信データの[%d]番拡張データが不正です\n", i );
        return FALSE;
      }
      p_extend_adrs += RESORT_EXTEND_DATA_SIZE;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データの内部データ取得
 *
 *	@param	const RESORT_RECV_DATA *cp_wk ワーク
 *	@param	param 引数
 *	@param  p_ret_adrs 取得バッファ
 *
 *	@return   受信データのデータ取得
 */
//-----------------------------------------------------------------------------
u32 RESORT_RECV_DATA_GetData( const RESORT_RECV_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    return cp_wk->peopleID;
  case RESORT_DATA_PARAM_PLACE_ID:
    return cp_wk->placeID;
  case RESORT_DATA_PARAM_LOCK:
    return cp_wk->lock;
  case RESORT_DATA_PARAM_UPDATE:
    return cp_wk->update;
  case RESORT_DATA_PARAM_SEX:
    return cp_wk->basic1.sex;
  case RESORT_DATA_PARAM_TRAINERVIEW:
    return RESORT_GET_TrainerView( cp_wk->basic1.trainer_view, cp_wk->basic1.sex );
  case RESORT_DATA_PARAM_NAME:
    RESORT_BASIC1_DATA_GetName(&cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_INTRODUCTION:
    RESORT_BASIC1_DATA_GetIntroduction( &cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_WANT_JOB:
    return cp_wk->basic1.trainerID % RESORT_RESIDENT_TYPE_MAX; //TODO
  case RESORT_DATA_PARAM_PRESENT_ID:
    return 10; //TODO;
  case RESORT_DATA_PARAM_PRESENT_NUM:
    return 1; //TODO
  case RESORT_DATA_PARAM_EVENT_FLAG:
    return 0; //TODO
  case RESORT_DATA_PARAM_PHRASE:
    //もしパックがなかったら通常の挨拶文
    if( cp_wk->extend_flag & RESORT_EXTEND_TYPE_09 )
    {
      RESORT_EXTEND9_DATA_GetPhrase( &cp_wk->extend9, p_ret_adrs );
    }
    else
    {
      RESORT_BASIC1_DATA_GetIntroduction( &cp_wk->basic1, p_ret_adrs );
    }
    return 0;
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  case RESORT_DATA_PARAM_DAY_FLAG:
    return cp_wk->day_flag;

  case RESORT_DATA_PARAM_NATION:
    return cp_wk->basic1.nation;
  case RESORT_DATA_PARAM_AREA:
    return cp_wk->basic1.area;
  case RESORT_DATA_PARAM_WORK:
    return cp_wk->basic1.work;
  case RESORT_DATA_PARAM_HOBBY:
    return cp_wk->basic1.hobby;
  case RESORT_DATA_PARAM_WND_COLOR:
    return cp_wk->basic1.wnd_color;
  case RESORT_DATA_PARAM_PLAY_HOUR:
    return cp_wk->basic1.play_hour;
  case RESORT_DATA_PARAM_PLAY_MIN:
    return cp_wk->basic1.play_min;
  case RESORT_DATA_PARAM_QANSWER:  //p_ret_adrsから取得
    GFL_STD_MemCopy( &cp_wk->basic1.qanswer, p_ret_adrs, sizeof(QUESTIONNAIRE_ANSWER_WORK) );
    return 0;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データの内部データ設定
 *
 *	@param	RESORT_RECV_DATA *p_wk ワーク
 *	@param	param 引数
 *	@param	data  受信データに設定する値
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_SetData( RESORT_RECV_DATA *p_wk, RESORT_DATA_PARAM param, u32 data )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    p_wk->peopleID  = data;
    break;
  case RESORT_DATA_PARAM_PLACE_ID:
    p_wk->placeID   = data;
    break;
  case RESORT_DATA_PARAM_LOCK:
    p_wk->lock      = data;
    break;
  case RESORT_DATA_PARAM_UPDATE:
    p_wk->update    = data;
    break;
  case RESORT_DATA_PARAM_KEEP:
    p_wk->keep_flag = data;
    break;
  case RESORT_DATA_PARAM_DAY_FLAG:
    p_wk->day_flag = data;
    break;
  case RESORT_DATA_PARAM_NATION:
    p_wk->basic1.nation = data;
    break;
  case RESORT_DATA_PARAM_AREA:
    p_wk->basic1.area = data;
    break;
  case RESORT_DATA_PARAM_WORK:
    p_wk->basic1.work = data;
    break;
  case RESORT_DATA_PARAM_HOBBY:
    p_wk->basic1.hobby = data;
    break;
  case RESORT_DATA_PARAM_WND_COLOR:
    p_wk->basic1.wnd_color = data;
    break;
  case RESORT_DATA_PARAM_PLAY_HOUR:
    p_wk->basic1.play_hour = data;
    break;
  case RESORT_DATA_PARAM_PLAY_MIN:
    p_wk->basic1.play_min = data;
    break;
  case RESORT_DATA_PARAM_QANSWER:  //p_ret_adrsから取得
    GFL_STD_MemCopy( (const void*)data, &p_wk->basic1.qanswer, sizeof(QUESTIONNAIRE_ANSWER_WORK) );
    break;
  default:
    GF_ASSERT(0);
  }
}
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ用データを作成
 *
 *	@param	RESORT_RECV_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata   ゲームデータ
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_SetDebugData( RESORT_RECV_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  //自分のデータをいれた後、トレーナーIDと名前を変える
  RESORT_RECV_DATA_SetMyData( p_wk, cp_gamedata );

  p_wk->basic1.trainerID = GFUser_GetPublicRand(0xFFFF);

  {
    static const struct
    {
      STRCODE *p_str;
      u32 len;
    } sc_name[] = 
    {
      {
        L"いちろう",
        4,
      },
      {
        L"じろう",
        3,
      },
      {
        L"さぶろう",
        4,
      },
      {
        L"しろう",
        3,
      },
      {
        L"ごろう",
        3,
      },
      {
        L"ろくろう",
        4,
      },
      {
        L"ななろう",
        4,
      },
      {
        L"はちろう",
        4,
      },
      {
        L"くろう",
        3,
      },
      {
        L"たろう",
        3,
      }
    };

    u8 idx = GFUser_GetPublicRand(NELEMS(sc_name));
    GFL_STD_MemCopy( sc_name[idx].p_str, p_wk->basic1.name, sizeof(STRCODE)*sc_name[idx].len );
    p_wk->basic1.name[sc_name[idx].len] = GFL_STR_GetEOMCode();

    p_wk->basic1.trainer_view = GFUser_GetPublicRand(8);

  }
}
#endif

//=============================================================================
/**
 *    RESORT_BEACON_DATAの関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンデータを作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ビーコンデータ
 */
//-----------------------------------------------------------------------------
RESORT_BEACON_DATA * RESORT_BEACON_DATA_Init( HEAPID heapID )
{
  RESORT_BEACON_DATA *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BEACON_DATA) );

  p_wk->extend_type = RESORT_EXTEND_TYPE_NULL;
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンデータを破棄
 *
 *	@param	RESORT_BEACON_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BEACON_DATA_Exit( RESORT_BEACON_DATA *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンデータに自分情報を作成
 *
 *	@param	RESORT_BEACON_DATA *p_wk  ワーク
 *	@param	type  作成する拡張タイプ
 */
//-----------------------------------------------------------------------------
void RESORT_BEACON_DATA_SetMyData( RESORT_BEACON_DATA *p_wk, RESORT_EXTEND_TYPE type, const GAMEDATA *cp_gamedata )
{
  RESORT_BASIC1_DATA_SetMyData( &p_wk->basic1, cp_gamedata );
  RESORT_BASIC2_DATA_SetMyData( &p_wk->basic2, cp_gamedata );

  //エラー値チェック
  if( type >= RESORT_EXTEND_TYPE_MAX )
  {
    GF_ASSERT(0);
    type  = RESORT_EXTEND_TYPE_01;
  }

  //拡張データセット
  sc_ResortExtendData_SetMyData[type]( p_wk->extend, cp_gamedata );
  p_wk->extend_type = type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データを取得
 *
 *  @param  cp_wk             ビーコンデータ
 *	@param	u8 *p_extend_buff 拡張データ（RESORT_EXTEND_DATA_SIZEサイズのバッファ）
 *	@param	*p_type           拡張データの種類
 */
//-----------------------------------------------------------------------------
void RESORT_BEACON_DATA_GetExtendData( const RESORT_BEACON_DATA *cp_wk, u8 *p_extend_buff, RESORT_EXTEND_TYPE *p_type )
{
  GF_ASSERT( cp_wk->extend_type < RESORT_EXTEND_TYPE_MAX );
  GFL_STD_MemCopy( cp_wk->extend, p_extend_buff, RESORT_EXTEND_DATA_SIZE );
  *p_type = cp_wk->extend_type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンからデータを取得
 *
 *	@param	RESORT_BEACON_DATA *p_wk  ワーク
 *	@param	GAMEBEACON_INFO *cp_beacon_info   ビーコンデータ
 */
//-----------------------------------------------------------------------------
void RESORT_BEACON_DATA_SetBeaconInfo( RESORT_BEACON_DATA *p_wk, const GAMEBEACON_INFO *cp_beacon_info )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_BEACON_DATA) );
  RESORT_BASIC1_DATA_SetBeaconInfo( &p_wk->basic1, cp_beacon_info );

  if( cp_beacon_info->pm_version == VERSION_W2 || cp_beacon_info->pm_version == VERSION_B2 )
  {
    //W2B2ならば拡張データあり
    //拡張データの先頭はBASIC2
    {
      const void *cp_basic2_adrs = cp_beacon_info->action.extend_buff;
      GFL_STD_MemCopy( cp_basic2_adrs, &p_wk->basic2, RESORT_BASIC2_DATA_SIZE );
    }
    //後ろがEXTEND
    {
      const void *cp_extend_adrs = (const u8 *)cp_beacon_info->action.extend_buff + RESORT_BASIC2_DATA_SIZE;
      GFL_STD_MemCopy( cp_extend_adrs, p_wk->extend, RESORT_EXTEND_DATA_SIZE );
    }
    p_wk->extend_type = cp_beacon_info->action.resort_extend_type;
  }
  else
  {
    //その他のソフトでは基本情報のみ
    p_wk->extend_type = RESORT_EXTEND_TYPE_NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンデータの正当性チェック
 *
 *	@param	const RESORT_BEACON_DATA *cp_wk ワーク 
 *
 *	@return TRUEならば正常  FALSEならば不正
 */
//-----------------------------------------------------------------------------
BOOL RESORT_BEACON_DATA_IsValid( const RESORT_BEACON_DATA *cp_wk )
{
  if( !RESORT_BASIC1_DATA_IsValid( &cp_wk->basic1 ) )
  {
    RESORT_VALID_ASSERT( "ビーコン情報の基本データ１が不正です\n" );
    return FALSE;
  }
  if( !RESORT_BASIC2_DATA_IsValid( &cp_wk->basic2 ) )
  {
    RESORT_VALID_ASSERT( "ビーコン情報の基本データ２が不正です\n" );
    return FALSE;
  }
  if( cp_wk->extend_type > RESORT_EXTEND_TYPE_MAX )
  {
    RESORT_VALID_ASSERT( "ビーコン情報の拡張番号が不正です=%d\n", cp_wk->extend_type );
    return FALSE;
  }
  else if( cp_wk->extend_type < RESORT_EXTEND_TYPE_MAX )
  {
    if( !sc_ResortExtendData_IsValid[cp_wk->extend_type]( cp_wk->extend ) )
    {
      RESORT_VALID_ASSERT( "ビーコン情報の[%d]番拡張データが不正です\n", cp_wk->extend_type );
      return FALSE;
    }
  }

  return TRUE;
}

//=============================================================================
/**
 *    RESORT_PROXY_DATAの関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データの作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return 代理人でーた
 */
//-----------------------------------------------------------------------------
RESORT_PROXY_DATA *RESORT_PROXY_DATA_Init( HEAPID heapID )
{
  RESORT_PROXY_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_PROXY_DATA));
  RESORT_PROXY_DATA_InitInstance( p_wk );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データの破棄
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_DATA_Exit( RESORT_PROXY_DATA *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データの初期化
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_DATA_InitInstance( RESORT_PROXY_DATA *p_wk )
{
  GFL_STD_MemClear(p_wk, sizeof(RESORT_PROXY_DATA) );
  p_wk->type  = RESORT_PROXY_TYPE_MAX;
}

//----------------------------------------------------------------------------
/**
 *	@brief  代理人データが空かチェック
 *
 *	@param	const RESORT_PROXY_DATA *cp_wk ワーク
 *
 *	@return TRUEならば空  FALSEならば存在
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_DATA_IsEmpty( const RESORT_PROXY_DATA *cp_wk )
{
  return cp_wk->type == RESORT_PROXY_TYPE_MAX;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データを設定
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	type  代理人タイプ  RESORT_PROXY_TYPE_xxxx
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_DATA_SetCommData( RESORT_PROXY_DATA *p_wk, u16 type )
{
  p_wk->type = type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データを取得
 *
 *	@param	const RESORT_PROXY_DATA *cp_wk  ワーク
 *	@param	param   取得するもの
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_PROXY_DATA_GetData( const RESORT_PROXY_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    return cp_wk->peopleID;
  case RESORT_DATA_PARAM_PLACE_ID:
    return cp_wk->placeID; 
  case RESORT_DATA_PARAM_SEX:
    return 1; //TODO
  case RESORT_DATA_PARAM_TRAINERVIEW:
    return 9; //TODO
  case RESORT_DATA_PARAM_NAME:
    {
      //TODO
      static const STRCODE sc_strcode[PERSON_NAME_SIZE+EOM_SIZE] =
      {
        L'だ',
        L'い',
        L'り',
        L'に',
        L'ん',
        0xffff
      };
      GFL_STD_MemCopy( sc_strcode, p_ret_adrs, sizeof(u16)*(PERSON_NAME_SIZE+EOM_SIZE) );
    }
    return 0;
  case RESORT_DATA_PARAM_PRESENT_ID:
    return 10; //TODO;
  case RESORT_DATA_PARAM_PRESENT_NUM:
    return 1; //TODO
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief 代理人データへデータ設定
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	param                   引数
 *	@param	data                    データ
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_DATA_SetData( RESORT_PROXY_DATA *p_wk, RESORT_DATA_PARAM param, u32 data )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    p_wk->peopleID = data;
    break;
  case RESORT_DATA_PARAM_PLACE_ID:
    p_wk->placeID   = data;
    break;
  case RESORT_DATA_PARAM_LOCK:
    //なにもしない
    break;
  default:
    GF_ASSERT(0);
  }
}
//=============================================================================
/**
 *	リゾート用代理人バッファ（セーブデータのみ存在）
*/
//=============================================================================
//-------------------------------------
///	受信バッファ
//=====================================
struct _RESORT_PROXY_BUFF
{
  u32         max;
  RESORT_PROXY_DATA data[0];
};

SDK_COMPILER_ASSERT( sizeof(RESORT_PROXY_BUFF) == RESORT_PROXY_BUFF_HEADER_SIZE );

//-------------------------------------
///	private
//=====================================
static RESORT_PROXY_DATA * Resort_Proxy_Buff_GetEmptyData( RESORT_PROXY_BUFF *p_wk );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾート  すでに生成した者に対しての初期化
 *
 *	@param	RESORT_PROXY_BUFF *p_wk  ワーク
 *	@param	num   バッファ数
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_BUFF_InitInstance( RESORT_PROXY_BUFF *p_wk, u32 num )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_PROXY_BUFF) );

  p_wk->max = num;
  {
    int i;
    for( i = 0; i < p_wk->max; i++ )
    {
      RESORT_PROXY_DATA *p_data  = &p_wk->data[i];
      RESORT_PROXY_DATA_InitInstance( p_data );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファクリア
 *
 *	@param	RESORT_PROXY_BUFF *p_wk  ワーク
 *	@param	index                   インデックス
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_BUFF_Clear( RESORT_PROXY_BUFF *p_wk, u32 index )
{
  GF_ASSERT( index < p_wk->max );
  RESORT_PROXY_DATA_InitInstance( &p_wk->data[ index ] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ受信バッファにビーコン差込
 *
 *	@param	RESORT_PROXY_BUFF *p_wk  ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   代理人データ
 *
 *	@retval TRUEビーコン挿し込みした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_BUFF_Add( RESORT_PROXY_BUFF *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  RESORT_PROXY_DATA *p_data;

  //空のデータがあったら入れる
  p_data  = Resort_Proxy_Buff_GetEmptyData( p_wk );
  if( p_data )
  {
    *p_data = *cp_proxy;
    RESORT_Printf( "空のデータに入れた！\n" );
    return TRUE;
  }

  RESORT_Printf( "代理人データを受け取ったが、バッファがいっぱい！\n" );
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファの最大数取得
 *
 *	@param	const RESORT_PROXY_BUFF *cp_wk ワーク
 *
 *	@return バッファの最大数取得
 */
//-----------------------------------------------------------------------------
u32 RESORT_PROXY_BUFF_GetMax( const RESORT_PROXY_BUFF *cp_wk )
{
  return cp_wk->max;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファから代理人データ取得
 *
 *	@param	RESORT_PROXY_BUFF *p_wk ワーク
 *	@param	index   インデックス
 *
 *	@return 代理人データ
 */
//-----------------------------------------------------------------------------
RESORT_PROXY_DATA * RESORT_PROXY_BUFF_GetData( RESORT_PROXY_BUFF *p_wk, u32 index )
{
  GF_ASSERT( index < RESORT_PROXY_BUFF_GetMax(p_wk ) );
  return &p_wk->data[index];
}
//----------------------------------------------------------------------------
/**
 *	@brief  空のデータを取得
 *
 *	@param	RESORT_PROXY_BUFF *p_wk  ワーク
 *
 *	@return あいているデータ  NULLならばあいていない
 */
//-----------------------------------------------------------------------------
static RESORT_PROXY_DATA * Resort_Proxy_Buff_GetEmptyData( RESORT_PROXY_BUFF *p_wk )
{
  int i;
  for( i = 0; i < p_wk->max; i++ )
  {
    RESORT_PROXY_DATA  *p_data = &p_wk->data[i];
    if( RESORT_PROXY_DATA_IsEmpty( p_data ) )
    {
      return p_data;
    }
  }

  return NULL;
}

//=============================================================================
/**
 *	リゾート用データ受信バッファ（セーブデータにも存在）
*/
//=============================================================================
//-------------------------------------
///	受信バッファ
//=====================================
struct _RESORT_RECV_BUFF
{
  u32         max;
  BOOL        is_lock;
  RESORT_RECV_DATA data[0];
};

SDK_COMPILER_ASSERT( sizeof(RESORT_RECV_BUFF) == RESORT_RECV_BUFF_HEADER_SIZE );

//-------------------------------------
///	private
//=====================================
static RESORT_RECV_DATA * Resort_Recv_Buff_GetEmptyData( RESORT_RECV_BUFF *p_wk );
static RESORT_RECV_DATA * Resort_Recv_Buff_GetSameOwnerData( RESORT_RECV_BUFF *p_wk, const RESORT_BASIC1_DATA *cp_owner );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートのデータ受信バッファ  作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	num           バッファ数
 *
 *	@return データバッファ
 */
//-----------------------------------------------------------------------------
RESORT_RECV_BUFF * RESORT_RECV_BUFF_Init( HEAPID heapID, u32 num )
{
  RESORT_RECV_BUFF *p_wk  = GFL_HEAP_AllocClearMemory( heapID, RESORT_RECV_BUFF_SIZE( num ) );

  RESORT_RECV_BUFF_InitInstance( p_wk, num );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートのデータ受信バッファ  破棄
 *
 *	@param	RESORT_RECV_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_BUFF_Exit( RESORT_RECV_BUFF *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾート  すでに生成した者に対しての初期化
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	num   バッファ数
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_BUFF_InitInstance( RESORT_RECV_BUFF *p_wk, u32 num )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_RECV_BUFF) );

  p_wk->max = num;
  {
    int i;
    for( i = 0; i < p_wk->max; i++ )
    {
      RESORT_RECV_DATA *p_data  = &p_wk->data[i];
      RESORT_RECV_DATA_InitInstance( p_data );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファへの追加アクセスをロック
 *
 *	@param	RESORT_RECV_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_BUFF_Lock( RESORT_RECV_BUFF *p_wk )
{
  p_wk->is_lock = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファへの追加アクセスロックを解除
 *
 *	@param	RESORT_RECV_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_BUFF_UnLock( RESORT_RECV_BUFF *p_wk )
{
  p_wk->is_lock = FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファの数を取得
 *
 *	@param	const RESORT_RECV_BUFF *cp_wk   ワーク
 *
 *	@return バッファ最大値
 */
//-----------------------------------------------------------------------------
u32 RESORT_RECV_BUFF_GetMax( const RESORT_RECV_BUFF *cp_wk )
{
  return cp_wk->max;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートのデータ受信バッファ  データ取得
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return データ取得
 */
//-----------------------------------------------------------------------------
RESORT_RECV_DATA * RESORT_RECV_BUFF_GetData( RESORT_RECV_BUFF *p_wk, u32 index )
{
  GF_ASSERT( index < p_wk->max );
  return &p_wk->data[ index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファクリア
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	index                   インデックス
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_BUFF_Clear( RESORT_RECV_BUFF *p_wk, u32 index )
{
  GF_ASSERT( index < p_wk->max );
  RESORT_RECV_DATA_InitInstance( &p_wk->data[ index ] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ受信バッファにビーコン差込
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   ビーコンデータ
 *
 *	@retval TRUEビーコン挿し込みした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_BUFF_AddBeacon( RESORT_RECV_BUFF *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  RESORT_RECV_DATA *p_data;

  //ロック中は追加しない
  if( p_wk->is_lock )
  {
    RESORT_Printf( "リゾート　ロック中\n" );
    return FALSE;
  }

  //同じ所有者のものがあったら優先的に入れる
  p_data  = Resort_Recv_Buff_GetSameOwnerData( p_wk, &cp_beacon->basic1 );
  if( p_data )
  {
    RESORT_RECV_DATA_MergeBeacon( p_data, cp_beacon );
    RESORT_Printf( "マージしていれた！%d\n", cp_beacon->extend_type );
    return TRUE;
  }

  //空のデータがあったら入れる
  p_data  = Resort_Recv_Buff_GetEmptyData( p_wk );
  if( p_data )
  {
    RESORT_RECV_DATA_MergeBeacon( p_data, cp_beacon );

#ifdef  PM_DEBUG
    RESORT_BASIC1_DATA_PrintDebug( &cp_beacon->basic1 );
#endif
    RESORT_Printf( "空のデータに入れた！%d\n", cp_beacon->extend_type );
    return TRUE;
  }

  RESORT_Printf( "ビーコンを受け取ったが、バッファがいっぱい！\n" );
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ受信バッファにビーコン差込
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_recv 受信したデータ
 *
 *	@retval TRUEビーコン挿し込みした  FALSEしていない*
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_BUFF_AddRecv( RESORT_RECV_BUFF *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  RESORT_RECV_DATA *p_data;

  //ロック中は追加しない
  if( p_wk->is_lock )
  {
    RESORT_Printf( "リゾート　ロック中\n" );
    return FALSE;
  }

  //同じ所有者のものがあったら優先的に入れる
  p_data  = Resort_Recv_Buff_GetSameOwnerData( p_wk, &cp_recv->basic1 );
  if( p_data )
  {
    RESORT_RECV_DATA_MergeRecv( p_data, cp_recv );
    RESORT_Printf( "受信データをマージした！\n" );
    return TRUE;
  }

  //空のデータがあったら入れる
  p_data  = Resort_Recv_Buff_GetEmptyData( p_wk );
  if( p_data )
  {
    RESORT_RECV_DATA_MergeRecv( p_data, cp_recv );

#ifdef  PM_DEBUG
    RESORT_BASIC1_DATA_PrintDebug( &cp_recv->basic1 );
#endif
    RESORT_Printf( "受信データを空のデータに入れた！\n" );
    return TRUE;
  }

  RESORT_Printf( "受信データを受け取ったが、バッファがいっぱい！\n" );
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  空のデータを取得
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *
 *	@return あいているデータ  NULLならばあいていない
 */
//-----------------------------------------------------------------------------
static RESORT_RECV_DATA * Resort_Recv_Buff_GetEmptyData( RESORT_RECV_BUFF *p_wk )
{
  int i;
  for( i = 0; i < p_wk->max; i++ )
  {
    RESORT_RECV_DATA  *p_data = &p_wk->data[i];
    if( RESORT_RECV_DATA_IsEmpty( p_data ) )
    {
      return p_data;
    }
  }

  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じ所有者のデータを取得
 *
 *	@param	RESORT_RECV_BUFF *p_wk        ワーク
 *	@param	RESORT_BASIC1_DATA *cp_owner  比べる基本データ１
 *
 *	@return 同じ所有者のデータ  NULLならば同じ所有者のデータがない
 */
//-----------------------------------------------------------------------------
static RESORT_RECV_DATA * Resort_Recv_Buff_GetSameOwnerData( RESORT_RECV_BUFF *p_wk, const RESORT_BASIC1_DATA *cp_owner )
{
  int i;
  for( i = 0; i < p_wk->max; i++ )
  {
    RESORT_RECV_DATA  *p_data = &p_wk->data[i];
    if( RESORT_BASIC1_DATA_IsSameOwner( &p_data->basic1, cp_owner ) )
    {
      return p_data;
    }
  }

  return NULL;
}

//=============================================================================
/**
 *  住民データ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  受信データを住民データに設定
 *
 *	@param	RESORT_RESIDENT_DATA *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_recv  受信データ
 *	@param  type RESORT_RESIDENT_TYPE_xxx タイプ
 */
//-----------------------------------------------------------------------------
void RESORT_RESIDENT_DATA_SetRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type )
{
  //TODO このあたりで変換するかも
  *p_wk   = *cp_recv;
  RESORT_RESIDENT_DATA_SetData(p_wk,RESORT_DATA_PARAM_LOCK,FALSE);
  p_wk->peopleID = peopleID;
  p_wk->placeID   = placeID;
  p_wk->type      = type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データから施設データを取得
 *
 *	@param	RESORT_RESIDENT_DATA *p_wk  ワーク
 *
 *	@return 施設データを取得
 */
//-----------------------------------------------------------------------------
RESORT_INSTITUTION_DATA * RESORT_RESIDENT_DATA_GetInstitution( RESORT_RESIDENT_DATA *p_wk )
{
  return &p_wk->institution;
}

//=============================================================================
/**
 *  滞在者
 */
//=============================================================================
//-------------------------------------
///	滞在者
//=====================================
struct _RESORT_VISITOR_DATA
{
  RESORT_BASIC1_DATA basic1;
  RESORT_BASIC2_DATA basic2;
  RESORT_EXTEND4_DATA extend4;
  u8    placeID;
  u8    peopleID;
  u8    dataID;
  u8    event_flag;
  u8    type;
  u8    day_flag;
  u8    dummy[2];
};
SDK_COMPILER_ASSERT( sizeof(RESORT_VISITOR_DATA) == RESORT_VISITOR_DATA_SIZE );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  滞在者ワーク作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return 滞在者ワーク
 */
//-----------------------------------------------------------------------------
RESORT_VISITOR_DATA * RESORT_VISITOR_DATA_Init( HEAPID heapID )
{
  RESORT_VISITOR_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_VISITOR_DATA) );
  RESORT_VISITOR_DATA_InitInstance(p_wk);
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  滞在者ワーク破棄
 *
 *	@param	RESORT_VISITOR_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_VISITOR_DATA_Exit( RESORT_VISITOR_DATA *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  滞在者ワーク初期化
 *
 *	@param	RESORT_VISITOR_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_VISITOR_DATA_InitInstance( RESORT_VISITOR_DATA *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_VISITOR_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  滞在者ワークが空かチェック
 *
 *	@param	const RESORT_VISITOR_DATA *cp_wk  ワーク
 *
 *	@return TRUEで空  FALSEで存在
 */
//-----------------------------------------------------------------------------
BOOL RESORT_VISITOR_DATA_IsEmpty( const RESORT_VISITOR_DATA *cp_wk )
{
  return RESORT_BASIC1_DATA_IsEmpty( &cp_wk->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief　受信データから滞在者ワークを設定
 *
 *	@param	RESORT_VISITOR_DATA *p_wk   ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信でーた
 */
//-----------------------------------------------------------------------------
void RESORT_VISITOR_DATA_SetRecv( RESORT_VISITOR_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type )
{
  //TODO
  p_wk->basic1    = cp_recv->basic1;
  p_wk->basic2    = cp_recv->basic2;
  p_wk->extend4   = cp_recv->extend4;
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;
  p_wk->type      = type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  住民データから滞在者ワークを設定
 *
 *	@param	RESORT_VISITOR_DATA *p_wk ワーク
 *	@param	RESORT_RESIDENT_DATA *cp_resident 住民データ
 *	@param	peopleID  ID
 *	@param	placeID   場所ID
 */
//-----------------------------------------------------------------------------
void RESORT_VISITOR_DATA_SetResident( RESORT_VISITOR_DATA *p_wk, const RESORT_RESIDENT_DATA *cp_resident, u16 peopleID, u16 placeID )
{
  //TODO
  p_wk->basic1    = cp_resident->basic1;
  p_wk->basic2    = cp_resident->basic2;
  p_wk->extend4   = cp_resident->extend4;
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	const RESORT_VISITOR_DATA *cp_wk  ワーク
 *	@param	なんのデータを取得するか
 *
 *	@return　データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_VISITOR_DATA_GetData( const RESORT_VISITOR_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    return cp_wk->peopleID;
  case RESORT_DATA_PARAM_PLACE_ID:
    return cp_wk->placeID;
  case RESORT_DATA_PARAM_NAME:
    RESORT_BASIC1_DATA_GetName(&cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_INTRODUCTION:
    RESORT_BASIC1_DATA_GetIntroduction( &cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_SEX: 
    return cp_wk->basic1.sex;
  case RESORT_DATA_PARAM_TRAINERVIEW:
    return RESORT_GET_TrainerView( cp_wk->basic1.trainer_view, cp_wk->basic1.sex );
  case RESORT_DATA_PARAM_WANT_JOB:
    return cp_wk->basic1.trainerID % RESORT_RESIDENT_TYPE_MAX; //TODO
  case RESORT_DATA_PARAM_EVENT_FLAG:
    return cp_wk->event_flag;
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  case RESORT_DATA_PARAM_DAY_FLAG:
    return cp_wk->day_flag;
  default:
    GF_ASSERT(0);
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ設定
 *
 *	@param	RESORT_VISITOR_DATA *cp_wk  ワーク
 *	@param	param   なんのデータを設定するか
 *	@param   data   設定するデータ
 */
//-----------------------------------------------------------------------------
void RESORT_VISITOR_DATA_SetData( RESORT_VISITOR_DATA *p_wk, RESORT_DATA_PARAM param, u32 data )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    p_wk->peopleID  = data;
    break;
  case RESORT_DATA_PARAM_PLACE_ID:
    p_wk->placeID   = data;
    break;
  case RESORT_DATA_PARAM_LOCK:
    //なにもしない
    break;
  case RESORT_DATA_PARAM_DAY_FLAG:
    p_wk->day_flag  = data;
    break;
  default:
    GF_ASSERT(0);
  }
}
//=============================================================================
/**
 *  秘書データ
 */
//=============================================================================
//-------------------------------------
///	秘書データ
//=====================================
struct _RESORT_SECRETARY_DATA 
{
  RESORT_BASIC1_DATA basic1;
  RESORT_BASIC2_DATA basic2;
  u8    placeID;
  u8    peopleID;
  u8    type;
  u8    dummy;
};
SDK_COMPILER_ASSERT( sizeof(RESORT_SECRETARY_DATA) == RESORT_SECRETARY_DATA_SIZE );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  秘書ワーク初期化
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return　秘書ワーク
 */
//-----------------------------------------------------------------------------
RESORT_SECRETARY_DATA * RESORT_SECRETARY_DATA_Init( HEAPID heapID )
{
  RESORT_SECRETARY_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_SECRETARY_DATA));
  RESORT_SECRETARY_DATA_InitInstance(p_wk);
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書ワーク破棄
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_SECRETARY_DATA_Exit( RESORT_SECRETARY_DATA *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書ワーク初期化
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_SECRETARY_DATA_InitInstance( RESORT_SECRETARY_DATA *p_wk )
{
  GFL_STD_MemClear(p_wk,sizeof(RESORT_SECRETARY_DATA));
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書ワーク空チェック
 *
 *	@param	const RESORT_SECRETARY_DATA *cp_wk  ワーク
 *
 *	@return TRUEで空  FALSEで存在
 */
//-----------------------------------------------------------------------------
BOOL RESORT_SECRETARY_DATA_IsEmpty( const RESORT_SECRETARY_DATA *cp_wk )
{
  return RESORT_BASIC1_DATA_IsEmpty( &cp_wk->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	const RESORT_SECRETARY_DATA *cp_wk ワーク
 *	@param	param 引数
 *	@param	*p_ret_adrs   受け取りワーク
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_SECRETARY_DATA_GetData( const RESORT_SECRETARY_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    return cp_wk->peopleID;
  case RESORT_DATA_PARAM_PLACE_ID:
    return cp_wk->placeID;
  case RESORT_DATA_PARAM_NAME:
    RESORT_BASIC1_DATA_GetName(&cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_INTRODUCTION:
    RESORT_BASIC1_DATA_GetIntroduction( &cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_SEX: 
    return cp_wk->basic1.sex;
  case RESORT_DATA_PARAM_TRAINERVIEW:
    return RESORT_GET_TrainerView( cp_wk->basic1.trainer_view, cp_wk->basic1.sex ); 
  case RESORT_DATA_PARAM_PRESENT_ID:
    return 10; //TODO;
  case RESORT_DATA_PARAM_PRESENT_NUM:
    return 1; //TODO
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief　データを設定
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk
 *	@param	param　引数
 *	@param	data  データ
 */
//-----------------------------------------------------------------------------
void RESORT_SECRETARY_DATA_SetData( RESORT_SECRETARY_DATA *p_wk, RESORT_DATA_PARAM param, u32 data )
{
  switch( param )
  {
  case RESORT_DATA_PARAM_PEOPLE_ID:
    p_wk->peopleID = data;
    break;
  case RESORT_DATA_PARAM_PLACE_ID:
    p_wk->placeID = data;
    break;
  case RESORT_DATA_PARAM_LOCK:
    //なにもしない
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief 受信データからデータを設定
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信データ
 *	@param	peopleID  人物ID
 *	@param	placeID   場所
 *	@param  type      RESORT_SECRETARY_TYPE_xxx
 */
//-----------------------------------------------------------------------------
void RESORT_SECRETARY_DATA_SetRecv( RESORT_SECRETARY_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type )
{
  //TODO
  p_wk->basic1    = cp_recv->basic1;
  p_wk->basic2    = cp_recv->basic2;
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;
  p_wk->type      = type;
}

//=============================================================================
/**
 *	リゾート人物全データ
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	人物データ
//=====================================
struct _RESORT_PEOPLE_ALL_DATA
{
  RESORT_RESIDENT_DATA    resident[RESORT_RESIDENT_DATA_NUM];
  RESORT_VISITOR_DATA     visitor[RESORT_VISITOR_DATA_NUM];
  RESORT_SECRETARY_DATA   secretary[RESORT_SECRETARY_DATA_NUM];
};

SDK_COMPILER_ASSERT(sizeof(RESORT_PEOPLE_ALL_DATA) == RESORT_PEOPLE_ALL_DATA_SIZE );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  街データ作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return 街データ
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_ALL_DATA * RESORT_PEOPLE_ALL_DATA_Init( HEAPID heapID )
{
  RESORT_PEOPLE_ALL_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_PEOPLE_ALL_DATA) );
  RESORT_PEOPLE_ALL_DATA_InitInstance(p_wk);
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データ破棄
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ALL_DATA_Exit( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データ初期化
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ALL_DATA_InitInstance( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  int i;
  GFL_STD_MemClear(p_wk,sizeof(RESORT_PEOPLE_ALL_DATA));

  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA_InitInstance(&p_wk->resident[i] );
  }
  for( i = 0; i < RESORT_VISITOR_DATA_NUM; i++ )
  {
    RESORT_VISITOR_DATA_InitInstance(&p_wk->visitor[i]);
  }
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA_InitInstance(&p_wk->secretary[i]);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  住民データ取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return 住民データ取得
 */
//-----------------------------------------------------------------------------
RESORT_RESIDENT_DATA * RESORT_PEOPLE_ALL_DATA_GetResident( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index )
{
  GF_ASSERT( index < RESORT_RESIDENT_DATA_NUM );
  return &p_wk->resident[ index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  空きの住民データを取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *
 *	@return 空の住民データ  NULLならば空がない
 */
//-----------------------------------------------------------------------------
RESORT_RESIDENT_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptyResident( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  int i;
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident=  RESORT_PEOPLE_ALL_DATA_GetResident( p_wk, i );
    if( RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
    {
      return p_resident;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  住民数を取得
 *
 *	@param	const RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *
 *	@return 住民数
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_ALL_DATA_GetResidentCount( const RESORT_PEOPLE_ALL_DATA *p_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident=  RESORT_PEOPLE_ALL_DATA_GetResident( (RESORT_PEOPLE_ALL_DATA *)p_wk, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
    {
      cnt++;
    }
  }
  return cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief  滞在者データ取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return　滞在者データ
 */
//-----------------------------------------------------------------------------
RESORT_VISITOR_DATA * RESORT_PEOPLE_ALL_DATA_GetVisitor( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index )
{
  GF_ASSERT( index < RESORT_VISITOR_DATA_NUM );
  return &p_wk->visitor[ index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  空きの滞在者データを取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *
 *	@return 空きの滞在者データ  NULLならば空きがない
 */
//-----------------------------------------------------------------------------
RESORT_VISITOR_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptyVisitor( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  int i;
  for( i = 0; i < RESORT_VISITOR_DATA_NUM; i++ )
  {
    RESORT_VISITOR_DATA *p_visitor = RESORT_PEOPLE_ALL_DATA_GetVisitor( p_wk, i );
    if( RESORT_VISITOR_DATA_IsEmpty(p_visitor) )
    {
      return p_visitor;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書データ取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return　秘書データ取得
 */
//-----------------------------------------------------------------------------
RESORT_SECRETARY_DATA * RESORT_PEOPLE_ALL_DATA_GetSecretary( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index )
{
  GF_ASSERT( index < RESORT_SECRETARY_DATA_NUM );
  return &p_wk->secretary[ index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書の数を取得
 *
 *	@param	const RESORT_PEOPLE_ALL_DATA *cp_wk ワーク
 *
 *	@return 秘書の数
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_ALL_DATA_GetSecretaryMax( const RESORT_PEOPLE_ALL_DATA *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_secretary = RESORT_PEOPLE_ALL_DATA_GetSecretary( (RESORT_PEOPLE_ALL_DATA *)cp_wk, i );
    if( !RESORT_SECRETARY_DATA_IsEmpty(p_secretary) )
    {
      cnt++;
    }
  }
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  空きの秘書データ取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *
 *	@return 空きの秘書データ  NULLならば空きがいない
 */
//-----------------------------------------------------------------------------
RESORT_SECRETARY_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  int i;
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_secretary = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_wk, i );
    if( RESORT_SECRETARY_DATA_IsEmpty(p_secretary) )
    {
      return p_secretary;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  開いている施設の場所IDを取得
 *
 *	@param	const RESORT_PEOPLE_ALL_DATA *cp_wk  ワーク
 *
 *	@return 施設の場所ID  開いてない場合RESORT_PLACEID_NONE
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_ALL_DATA_SearchEmptyInstitutionPlaceID( const RESORT_PEOPLE_ALL_DATA *cp_wk )
{
  int i;
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident=  RESORT_PEOPLE_ALL_DATA_GetResident( (RESORT_PEOPLE_ALL_DATA *)cp_wk, i );
    if( RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
    {
      return RESORT_PLACEID_INSTITUTION_00+i;
    }
  }
  return RESORT_PLACEID_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  開いているコンドミニアムの場所IDを取得(滞在者インデックス＝コンドミニアムインデックス)
 *
 *	@param	const RESORT_PEOPLE_ALL_DATA *cp_wk  ワーク
 *
 *	@return コンドミニアムの場所ID  開いてない場合RESORT_PLACEID_NONE
 */
//-----------------------------------------------------------------------------
u32 RESORT_PEOPLE_ALL_DATA_SearchEmptyCondominiumPlaceID( const RESORT_PEOPLE_ALL_DATA *cp_wk )
{
  int i;
  for( i = 0; i < RESORT_VISITOR_DATA_NUM; i++ )
  {
    RESORT_VISITOR_DATA *p_visitor = RESORT_PEOPLE_ALL_DATA_GetVisitor( (RESORT_PEOPLE_ALL_DATA *)cp_wk, i );
    if( RESORT_VISITOR_DATA_IsEmpty(p_visitor) )
    {
      return RESORT_PLACEID_CONDOMINIUM_00+i;
    }
  }
  return RESORT_PLACEID_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンを他のデータにマージ
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   ビーコンデータ
 *
 *	@return TRUEいれた　FALSEいれなかった
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PEOPLE_ALL_DATA_AddBeacon( RESORT_PEOPLE_ALL_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  int i;
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_wk, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) 
      &&  RESORT_RESIDENT_DATA_IsSameOwnerBeacon(p_resident,cp_beacon) )
    {
      if( RESORT_RESIDENT_DATA_MergeBeacon( p_resident, cp_beacon ) )
      {
        return TRUE;
      }
    }
  }

  //TODO  住人以外もマージする？


  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  一日一回更新
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ALL_DATA_UpdateDay( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  int i, j;
  //一日一回フラグを消去
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident =  RESORT_PEOPLE_ALL_DATA_GetResident( p_wk, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident ) )
    {
      RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
      RESORT_RESIDENT_DATA_SetData(p_resident,RESORT_DATA_PARAM_DAY_FLAG, FALSE );

      for( j = RESORT_INSTITUTION_DATA_BIT_SHOP_START; j < RESORT_INSTITUTION_DATA_BIT_SHOP_END; j++ )
      {
        RESORT_INSTITUTION_DATA_SetBit( p_institution, j, FALSE );
      }
    }
  }
  for( i = 0; i < RESORT_VISITOR_DATA_NUM; i++ )
  {
    RESORT_VISITOR_DATA *p_visitor =  RESORT_PEOPLE_ALL_DATA_GetVisitor( p_wk, i );
    if( !RESORT_VISITOR_DATA_IsEmpty(p_visitor) )
    {
      RESORT_VISITOR_DATA_SetData(p_visitor,RESORT_DATA_PARAM_DAY_FLAG, FALSE );
    }
  }
}

//=============================================================================
/**
 *	リゾート街データ
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
#define RESORT_CITY_NAME_BUFF_MAX  (20)
#define RESORT_MY_NAME_BUFF_MAX  (20)

//-------------------------------------
///	街データ
//=====================================
struct _RESORT_CITY_DATA
{
  STRCODE city_name[RESORT_CITY_NAME_BUFF_MAX+EOM_SIZE];
  STRCODE my_name[RESORT_MY_NAME_BUFF_MAX+EOM_SIZE];  //84
  u16     lv; 
  u16     state;  //92
  u32     trainerID[32];  //128 220
  u8      encount[32];  //32 252
  u8      decoration[32]; //32 284
  u8      ev_flag[32];  //32 316
  u16     arrangement[16];  //32 348
  u32     talk_cnt;     //7*4=28 376
  u32     build_cnt;
  u32     quest_clear_cnt;
  u32     todo1;  //TODO
  u32     todo2;  //TODO
  u32     todo3;  //TODO
  u8      secret_present_type;  
  u8      secret_present_num;
  u8      secret_present_flag;
  u8      pad;
};
SDK_COMPILER_ASSERT(sizeof(RESORT_CITY_DATA) == RESORT_CITY_DATA_SIZE );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  街データ作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return 街データ
 */
//-----------------------------------------------------------------------------
RESORT_CITY_DATA * RESORT_CITY_DATA_Init( HEAPID heapID )
{
  RESORT_CITY_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_CITY_DATA) );
  RESORT_CITY_DATA_InitInstance(p_wk);
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データ破棄
 *
 *	@param	RESORT_CITY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_CITY_DATA_Exit( RESORT_CITY_DATA *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データ初期化
 *
 *	@param	RESORT_CITY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_CITY_DATA_InitInstance( RESORT_CITY_DATA *p_wk )
{
  int i;
  GFL_STD_MemClear(p_wk,sizeof(RESORT_CITY_DATA));
  p_wk->city_name[0] = GFL_STR_GetEOMCode();
  p_wk->my_name[0]  = GFL_STR_GetEOMCode();

  {
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(HEAPID_SAVE) );
    STRBUF *p_strbuf = GFL_MSG_CreateString( p_msg, resort_sys_name_00 );
    GFL_STR_GetStringCode( p_strbuf, p_wk->city_name, RESORT_CITY_NAME_BUFF_MAX+EOM_SIZE );
    GFL_STR_DeleteBuffer( p_strbuf );


    p_strbuf = GFL_MSG_CreateString( p_msg, resort_sys_name_01 );
    GFL_STR_GetStringCode( p_strbuf, p_wk->my_name, RESORT_MY_NAME_BUFF_MAX+EOM_SIZE );
    GFL_STR_DeleteBuffer( p_strbuf );

    GFL_MSG_Delete( p_msg );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	const RESORT_CITY_DATA *cp_wk  街データ
 *	@param	param                   引数
 *	@param	*p_ret_adrs             取得バッファ
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_CITY_DATA_GetData( const RESORT_CITY_DATA *cp_wk, RESORT_CITY_DATA_PARAM param, void *p_ret_adrs )
{
  switch( param )
  {
  case RESORT_CITY_DATA_PARAM_CITY_NAME: //p_ret_adrs
    STRTOOL_Copy( cp_wk->city_name, p_ret_adrs, RESORT_CITY_NAME_BUFF_MAX+EOM_SIZE );
    return 0;
  case RESORT_CITY_DATA_PARAM_AUTHER_NAME: //p_ret_adrs
    STRTOOL_Copy( cp_wk->my_name, p_ret_adrs, RESORT_MY_NAME_BUFF_MAX+EOM_SIZE );
    return 0;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	RESORT_CITY_DATA *p_wk  街データ
 *	@param	param                   引数
 *	@param	data                    値orアドレス
 */
//-----------------------------------------------------------------------------
void RESORT_CITY_DATA_SetData( RESORT_CITY_DATA *p_wk, RESORT_CITY_DATA_PARAM param, u32 data )
{
  switch( param )
  {
  case RESORT_CITY_DATA_PARAM_CITY_NAME: //p_ret_adrs
    STRTOOL_Copy( (const STRCODE*)data, p_wk->city_name, RESORT_CITY_NAME_BUFF_MAX+EOM_SIZE );
    break;
  case RESORT_CITY_DATA_PARAM_AUTHER_NAME: //p_ret_adrs
    STRTOOL_Copy( (const STRCODE*)data, p_wk->my_name, RESORT_MY_NAME_BUFF_MAX+EOM_SIZE );
    break;
  default:
    GF_ASSERT(0);
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  一日一回更新
 *
 *	@param	RESORT_CITY_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_CITY_DATA_UpdateDay( RESORT_CITY_DATA *p_wk )
{
}
