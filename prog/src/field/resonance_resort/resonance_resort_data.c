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
#include "field/resort_work.h"
#include "app/research_radar/question_id.h"

//arc
#include "arc/arc_def.h"
#include "arc/message.naix"
#include "arc/script_message.naix"
#include "msg/script/msg_resort_scr.h"
#include "msg/msg_place_name.h"
#include "resort_binary.h"

//save
#include "savedata/resonance_resort_sv.h"
#include "savedata/record.h"
#include "savedata/medalbox.h"
#include "savedata/gametime.h"

//mine 
#include "field/resonance_resort/resort_map.h"
#include "field/resonance_resort/resort_shop.h"
#include "field/resonance_resort/scrcmd_resort_def.h"
#include "resort_shop_release.cdat"

//debug
#include "resort_debug.h"

//外部公開
#include "field/resonance_resort_data.h"

#ifdef PM_DEBUG
#define DEBUG_RESORT_PRINT_ON
//#define DEBUG_RESORT_RECV_PRINT_ON
//#define DEBUG_EXTEND_SET_GET_CHECK
#define DEBUG_PERMISSION_CHECK

static BOOL s_debug_permission = FALSE;
#endif

#if defined(DEBUG_RESORT_PRINT_ON) && ( defined(DEBUG_ONLY_FOR_nagihashi_toru) || defined(DEBUG_ONLY_FOR_shimoyamada) )
#define RESORT_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#else
#define RESORT_Printf(...)  /* */
#endif

#if defined(DEBUG_RESORT_RECV_PRINT_ON) && ( defined(DEBUG_ONLY_FOR_nagihashi_toru) || defined(DEBUG_ONLY_FOR_shimoyamada) )
#define RESORT_RECV_Printf(...)  OS_TFPrintf(1,__VA_ARGS__)
#else
#define RESORT_RECV_Printf(...)  /* */
#endif

#ifdef DEBUG_PERMISSION_CHECK
#define DEBUG_CHECK_PERMISSION()   GF_ASSERT(s_debug_permission)
#else
#define DEBUG_CHECK_PERMISSION() /* */
#endif

#ifdef DEBUG_EXTEND_SET_GET_CHECK
#define RESORT_EXT_CHECK( x, y )  if( s_debug_permission) { \
                                    GF_ASSERT(RESORT_RESIDENT_DATA_IsExtend( x, y ) ) \
                                  }
#else
#define RESORT_EXT_CHECK( x, y )  /* */
#endif

#define RESORT_VALID_ASSERT(...) GF_ASSERT_MSG_LIGHT( 0, __VA_ARGS__ );

//----------------------------------------------------------------------------
/**
 *	@brief  RTC構造体からリゾート用日時への変換
 *
 *	@param	const RTCDate *cp_date  RTC構造体
 *
 *	@return リゾート用日時
 */
//-----------------------------------------------------------------------------
RESORT_DATE RESORT_DATA_RtcToDate( const RTCDate *cp_date )
{
  return RESORT_DATE_SET( cp_date->year, cp_date->month, cp_date->day );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リゾート用日時からRTC構造体への変換
 *
 *	@param	RESORT_DATE date  リゾート用日時
 *	@param	*p_date RTC構造体
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_DateToRtc( RESORT_DATE date, RTCDate *p_date )
{
  GFL_STD_MemClear(p_date,sizeof(RTCDate));
  p_date->year  = RESORT_DATE_GET_YEAR( date );
  p_date->month = RESORT_DATE_GET_MONTH( date );
  p_date->day   = RESORT_DATE_GET_DAY( date );
}


//
extern u16 RESORT_DATA_GetWantJobModify( u16 want_job, u16 id, u16 ver, BOOL is_special, const RESORT_CITY_DATA *cp_data, const RESORT_BINARY_SHOP *cp_shop );
extern void RESORT_RESIDENT_DATA_JobChange( RESORT_RESIDENT_DATA *p_wk, const RESORT_BINARY_SHOP *cp_shop );
extern void RESORT_RESIDENT_DATA_ConvertRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );
extern void RESORT_RESIDENT_DATA_ConvertProxy( RESORT_RESIDENT_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );

//-------------------------------------
///	リゾートデータグローバル参照
//=====================================
static RESORT_CITY_DATA *sp_city_data = NULL;
static RESORT_PEOPLE_ALL_DATA *sp_people_all_data = NULL;

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
  u8  dummy;
  u8  lv;
  u16 exp;
  u32 bit;
  u16 institutionID;
  u16 work;
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
  p_wk->institutionID = 0xFFFF;
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
  return cp_wk->institutionID == 0xFFFF;
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
  case RESORT_INSTITUTION_DATA_PARAM_EXP:
    return cp_wk->exp;
  case RESORT_INSTITUTION_DATA_PARAM_BIT:
    return cp_wk->bit;
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
  case RESORT_INSTITUTION_DATA_PARAM_EXP:
    p_wk->exp = data;
    break;
  case RESORT_INSTITUTION_DATA_PARAM_BIT:
    p_wk->bit = data;
    break;
  default:
    GF_ASSERT(0);
  }
}

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
BOOL RESORT_INSTITUTION_DATA_IsBit( const RESORT_INSTITUTION_DATA *cp_wk, u32 bit )
{
  GF_ASSERT( bit < 32 );
  return (cp_wk->bit & (1<<bit)) != 0;
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
void RESORT_INSTITUTION_DATA_SetBit( RESORT_INSTITUTION_DATA *p_wk, u32 bit, BOOL enable )
{
  GF_ASSERT( bit < 32 );
  if( enable )
  {
    p_wk->bit |= (1<<bit);
  }
  else
  {
    p_wk->bit &= ~(1<<bit);
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
  u8  nation;                       ///<国コード      1 15
  u8  area;                         ///<地域コード    1 16
  STRCODE introduction[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN]; //<自己紹介(!! EOM除く !!) 16 32
  u8  version;                      ///<バージョン    1 33
  u8  language;                     ///<言語コード    1 34
  u8  dummy2:4;                 ///<見た目        1 35 -> trainer_view から objcodeにした
  u8  sex:4;                          ///<性別          1 36
  u8  dummy1;
  u16 trainerID;                    ///<トレーナーID(下位16ビット)  2 38
  u8 job;                        ///<仕事  1 39
  u8 hobby;                      ///<趣味  1 40
  u16 play_hour     :10;          ///< プレイ時間 2 42
  u16 play_min      :6;
  u16 objcode;            ///<みため
}RESORT_BASIC1_DATA;

SDK_COMPILER_ASSERT( sizeof(RESORT_BASIC1_DATA) == RESORT_BASIC1_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  BASIC1にMYSTATUSを設定する
 *
 *	@param	RESORT_BASIC1_DATA *p_wk  ワーク
 *	@param	MYSTATUS *cp_mystatus     MYSTATUS
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC1_DATA_SetMyStatus( RESORT_BASIC1_DATA *p_wk, const MYSTATUS *cp_mystatus )
{
  //EOMが入らないサイズを取得するので、自分でコピーする
  GFL_STD_MemCopy( MyStatus_GetMyName(cp_mystatus), p_wk->name, sizeof(STRCODE)*PERSON_NAME_SIZE );
  p_wk->trainerID = MyStatus_GetID_Low(cp_mystatus);
  p_wk->nation    = MyStatus_GetMyNation(cp_mystatus);
  p_wk->area      = MyStatus_GetMyArea(cp_mystatus);
  p_wk->sex       = MyStatus_GetMySex(cp_mystatus);
  p_wk->version   = MyStatus_GetRomCode( cp_mystatus );
  p_wk->language  = MyStatus_GetRegionCode( cp_mystatus );

  p_wk->objcode = UnionView_GetObjCode( MyStatus_GetTrainerView(cp_mystatus) );
}

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
    RESORT_BASIC1_DATA_SetMyStatus( p_wk, p_mystatus );
  }

  {
    MISC *p_misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork((GAMEDATA*)cp_gamedata) );
    const STRCODE *cp_src = MISC_CrossComm_GetSelfIntroduction(p_misc);
    GFL_STD_MemCopy( cp_src, p_wk->introduction, sizeof(STRCODE)*GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN );
  }

  //アンケート
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork((GAMEDATA*)cp_gamedata);
    QUESTIONNAIRE_SAVE_WORK *p_ques_sv = SaveData_GetQuestionnaire(p_sv);
    QUESTIONNAIRE_ANSWER_WORK *p_ques_ans = Questionnaire_GetAnswerWork(p_ques_sv);

    p_wk->job   = QuestionnaireAnswer_ReadBit(p_ques_ans, QUESTION_ID_JOB);
    p_wk->hobby = QuestionnaireAnswer_ReadBit(p_ques_ans, QUESTION_ID_HOBBY);
  }

  //プレイ時間
  {
    PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( (GAMEDATA*)cp_gamedata );
    p_wk->play_hour      = PLAYTIME_GetHour( sv_playtime );
    p_wk->play_min       = PLAYTIME_GetMinute( sv_playtime );
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
  GFL_STD_MemCopy( cp_beacon_info->self_introduction, p_wk->introduction, sizeof(STRCODE)*GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN );
  p_wk->trainerID = cp_beacon_info->trainer_id;
  p_wk->sex       = cp_beacon_info->sex;
  p_wk->version   = cp_beacon_info->pm_version;
  p_wk->language  = cp_beacon_info->language;
  p_wk->nation    = cp_beacon_info->nation;
  p_wk->area      = cp_beacon_info->area;
  if( p_wk->sex == PM_MALE )
  {
    p_wk->objcode  = UnionView_GetObjCode( cp_beacon_info->trainer_view );
  }
  else
  {
    p_wk->objcode  = UnionView_GetObjCode( cp_beacon_info->trainer_view + UNION_VIEW_INDEX_WOMAN_START );
  }
  {
    const QUESTIONNAIRE_ANSWER_WORK *cp_ques_ans = &cp_beacon_info->question_answer;
    p_wk->job   = QuestionnaireAnswer_ReadBit(cp_ques_ans, QUESTION_ID_JOB);
    p_wk->hobby = QuestionnaireAnswer_ReadBit(cp_ques_ans, QUESTION_ID_HOBBY);
  }

  p_wk->play_hour = cp_beacon_info->play_hour;
  p_wk->play_min  = cp_beacon_info->play_min;
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
  //見た目
  {
    int i;
    BOOL is_ok = FALSE;
    for( i = 0; i <= UNION_VIEW_INDEX_MAX; i++ )
    {
      if( cp_wk->objcode == UnionView_GetObjCode( i ) )
      {
        is_ok = TRUE;
        break;
      }
    }
    if( !is_ok )
    {
      RESORT_VALID_ASSERT( "基本データ１：見た目が不正です\n" );
      return FALSE;
    }
  }
  //国地域
  if( !WIFI_COUNTRY_CheckNG( cp_wk->nation, cp_wk->area, cp_wk->language) )
  {
    RESORT_VALID_ASSERT( "基本データ１：国、地域が不正です\n" );
    return FALSE;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  u16単位の一致
 *
 *	@param	STRCODE *s1 比較１
 *	@param	STRCODE *s2 比較２
 *	@param	u32 size    バイトサイズ
 *
 *	@return ０なら同じ  それ以外なら違う
 */
//-----------------------------------------------------------------------------
static int Resort_Basic1_Data_CompareU16(const u16 *s1, const u16 *s2, const u32 size)
{
    const u16  *p1 = (const u16 *)s1;
    const u16  *p2 = (const u16 *)s2;
    u32 n = size/2;

    while (n-- > 0) {
        if (*p1 != *p2)
            return (*p1 - *p2);
        p1++;
        p2++;
    }
    return (0);
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
  BOOL is_name  = FALSE;
  BOOL is_sex = FALSE;
  BOOL is_trainerID = (cp_a->trainerID == cp_b->trainerID);

  //トレーナーIDが一致しないとチェックしない
  if( is_trainerID )
  {
    is_name  = (Resort_Basic1_Data_CompareU16( cp_a->name, cp_b->name ,  sizeof(STRCODE)*PERSON_NAME_SIZE) == 0);
    is_sex   = (cp_a->sex == cp_b->sex);
  }

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
 *	@brief 挨拶をEOM付きで取得
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
  STRCODE name[128];
  char str_name[128];

  GFL_STD_MemCopy( cp_wk->name, name, PERSON_NAME_SIZE*sizeof(STRCODE) );
  name[PERSON_NAME_SIZE]  = GFL_STR_GetEOMCode();

  GFL_STR_CONV_StrcodeToSjis( name,str_name,PERSON_NAME_SIZE+1 );
  RESORT_RECV_Printf( "name=%s\n", str_name );
  RESORT_RECV_Printf( "trainerID=%d\n", cp_wk->trainerID );
}
#endif

//-------------------------------------
///	基本データ２
//=====================================
typedef struct
{
  u8  special_deliverly :1;
  u8  city_lv           :5;
  u8  dummy             :2;
  u8  dummy2;
  u16 want_job;       //なりたい施設IDが入っている
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

  p_wk->special_deliverly = 0; //製品版でここが０以外はありえない
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( (GAMEDATA *)cp_gamedata );
    RESONANCE_RESORT_SV *p_reso = RESONANCE_RESORT_SV_GetWork( p_sv );
    RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_reso );
    RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData( p_reso );
    RESORT_RECV_DATA *p_own = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
  
    MYSTATUS *p_mystatus  = GAMEDATA_GetMyStatus((GAMEDATA*)cp_gamedata);

    p_wk->city_lv = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_LV, NULL );
    p_wk->want_job = RESORT_OWNER_DATA_GetWantJob( p_own, p_mystatus );
 
  }
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
  if( cp_wk->want_job >= RESORT_SHOP_ID_MAX )
  {
    return FALSE;
  }
  return TRUE;
}

//-------------------------------------
///	拡張データ１
//=====================================
typedef struct
{

  u32 lot         :4;
  u32 parlor      :4;
  u32 market      :4;
  u32 florist     :4;
  u32 exercice    :4;
  u32 nursery     :4;
  u32 antique     :4;
  u32 cafe        :4;      //4

  u32 zukan       :10;
  u32 first_poke  :10;
  u32 medal_rank  :8;         //4
  u32 dummy       :4;

  u8  recommend_medalID;        //8
  u8  medal_num;
  RESORT_DATE   recommend_date;
  RESORT_DATE   start_date;
  RESORT_DATE   dendou_date;
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
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( (GAMEDATA *)cp_gamedata );

  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND1_DATA) );

  //冒険開始、殿堂入り
  {
    GMTIME *p_time = SaveData_GetGameTime( p_sv );

    RTCDate date;
    RTCTime time;

    RTC_ConvertSecondToDateTime( &date , &time , p_time->start_sec );

    p_wk->start_date = RESORT_DATA_RtcToDate( &date );
  
    if( p_time->clear_sec == 0 )
    {
      RTC_ConvertSecondToDateTime( &date , &time , p_time->clear_sec );

      p_wk->dendou_date = RESORT_DATA_RtcToDate( &date );
    }
  }

  //メダル情報
  {
    MEDALBOX *p_mdlbox  = SaveData_GetMedalBox(p_sv);
    p_wk->medal_num  = MEDALBOX_GetMedalCount(p_mdlbox,FALSE);
    p_wk->recommend_medalID = MEDALBOX_GetPushMedalID(p_mdlbox);
    p_wk->medal_rank = MEDALBOX_GetMedalBoxRank( p_mdlbox );
    if( p_wk->recommend_medalID < MDLID_MAX )
    {
      u8 year;
      u8 month;
      u8 day;

      MEDALBOX_GetDate( p_mdlbox, p_wk->recommend_medalID, &year , &month , &day );

      p_wk->recommend_date = RESORT_DATE_SET( year, month, day );
    }
  }

  //リゾート情報
  {
    RESONANCE_RESORT_SV *p_reso_sv = RESONANCE_RESORT_SV_GetWork(p_sv);
    RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData(p_reso_sv );
    int i;
    for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
    {
      RESORT_RESIDENT_DATA *p_resi = RESORT_PEOPLE_ALL_DATA_GetResident(p_people_all,i);
      if( !RESORT_RESIDENT_DATA_IsEmpty(p_resi) )
      {
        u32 type = RESORT_RESIDENT_DATA_GetData(p_resi, RESORT_DATA_PARAM_TYPE, NULL);
        switch(type)
        {
        case RESORT_RESIDENT_TYPE_NURSERY:
          p_wk->nursery++;
          break;
        case RESORT_RESIDENT_TYPE_EXERCICE:
          p_wk->exercice++;
          break;
        case RESORT_RESIDENT_TYPE_LOT:
          p_wk->lot++;
          break;
        case RESORT_RESIDENT_TYPE_ANTIQUE:
          p_wk->antique++;
          break;
        case RESORT_RESIDENT_TYPE_PARLOR:
          p_wk->parlor++;
          break;
        case RESORT_RESIDENT_TYPE_MARKET:
          p_wk->market++;
          break;
        case RESORT_RESIDENT_TYPE_CAFE:
          p_wk->cafe++;
          break;
        case RESORT_RESIDENT_TYPE_FLORIST:
          p_wk->florist++;
          break;
        }
      }
    }
  }

  //図鑑、先頭ポケ
  {
    ZUKAN_SAVEDATA *p_zukan = GAMEDATA_GetZukanSave((GAMEDATA *)cp_gamedata);
    POKEPARTY *p_party = GAMEDATA_GetMyPokemon((GAMEDATA *)cp_gamedata);

    p_wk->zukan = ZUKANSAVE_GetPokeGetCount( p_zukan );

    if( PokeParty_GetPokeCount(p_party) >= 1 )
    {
      POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer(p_party, 0);
      if( PP_Get( p_pp, ID_PARA_tamago_flag, NULL ) )
      {
        p_wk->first_poke = MONSNO_TAMAGO;
      }
      else
      {
        p_wk->first_poke = PP_Get( p_pp, ID_PARA_monsno, NULL );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ1の正当性チェック
 *
 *	@param	RESORT_EXTEND1_DATA *cp_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_EXTEND1_DATA_IsValid( const RESORT_EXTEND1_DATA *cp_wk )
{
  if( cp_wk->recommend_medalID < MDLID_MAX )
  {
    u8 year   = RESORT_DATE_GET_YEAR( cp_wk->recommend_date );
    u8 month  = RESORT_DATE_GET_MONTH( cp_wk->recommend_date );
    u8 day    = RESORT_DATE_GET_DAY( cp_wk->recommend_date );

    if( !( 0 <= year && year <= 99) )
    {
      return FALSE;
    }
    if( !( 1 <= month && month <= 12 ) )
    {
      return FALSE;
    }
    if( !( 1<= day && day <= 31 ) )
    {
      return FALSE;
    }
  }

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
 *	@param	RESORT_EXTEND2_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND2_DATA_SetMyData( RESORT_EXTEND2_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  RESORT_WORK *p_resort_wk  = GAMEDATA_GetReosrtWork( (GAMEDATA *)cp_gamedata );

  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND2_DATA) );
  p_wk->rank_data[0]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_NET_TRADE );
  p_wk->rank_data[1]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_NICKNAME );
  p_wk->rank_data[2]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_JA_TOURIST );
  p_wk->rank_data[3]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_BUY_MONEY );
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
 *	@param	RESORT_EXTEND3_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND3_DATA_SetMyData( RESORT_EXTEND3_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  RESORT_WORK *p_resort_wk  = GAMEDATA_GetReosrtWork( (GAMEDATA *)cp_gamedata );

  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND3_DATA) );
  p_wk->rank_data[0]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_CROSSING );
  p_wk->rank_data[1]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_NET_BATTLE );
  p_wk->rank_data[2]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_GET_POKE );
  p_wk->rank_data[3]  = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_RNKDATA_HATCH_EGG );
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
 *	@param	RESORT_EXTEND4_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND4_DATA_SetMyData( RESORT_EXTEND4_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( (GAMEDATA *)cp_gamedata );
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort);
  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
  int i;

  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND4_DATA) );

  for( i = 0; i < 16; i++ )
  {
    p_wk->gossip[i] = RESORT_RECV_DATA_GetData(p_owner,RESORT_DATA_PARAM_EXT4_GOSSIP_00+i, NULL);
  }

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
  u8  record[4];
  RESORT_DATE record_date[4];
  u8  padding[4];
} RESORT_EXTEND5_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND5_DATA) == RESORT_EXTEND5_DATA_SIZE );

//----------------------------------------------------------------------------
/**
 *	@brief  拡張データ5に自分のデータを設定
 *
 *	@param	RESORT_EXTEND5_DATA *p_wk  ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND5_DATA_SetMyData( RESORT_EXTEND5_DATA *p_wk, const GAMEDATA *cp_gamedata )
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( (GAMEDATA *)cp_gamedata );
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort);
  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
  int i;

  GFL_STD_MemClear( p_wk, sizeof(RESORT_EXTEND5_DATA) );

  for( i = 0; i < 4; i++ )
  {
    p_wk->record[i]       = RESORT_RECV_DATA_GetData(p_owner,RESORT_DATA_PARAM_EXT5_RECORD_00+i,NULL);
    p_wk->record_date[i]  = RESORT_RECV_DATA_GetData(p_owner,RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+i,NULL);
  }

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
typedef struct
{
  STRCODE  phrase[RESORT_PHRASE_LEN]; //EOMなし
} RESORT_EXTEND6_DATA;
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

  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( (GAMEDATA *)cp_gamedata );
    RESONANCE_RESORT_SV *p_reso = RESONANCE_RESORT_SV_GetWork( p_sv );
    RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData( p_reso );
    RESORT_RECV_DATA *p_own = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
    RESORT_RECV_DATA_GetData( p_own, RESORT_DATA_PARAM_PHRASE_CODE, p_wk->phrase );
  }
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
//----------------------------------------------------------------------------
/**
 *	@brief 口癖をEOM付きで取得
 *
 *	@param	const RESORT_EXTEND6_DATA *cp_data ワーク
 *	@param	*p_str  受け取りバッファ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND6_DATA_GetPhrase( const RESORT_EXTEND6_DATA *cp_data, STRCODE *p_str, u16 trainerID )
{
  if( cp_data->phrase[0] != GFL_STR_GetEOMCode() )
  {
    GFL_STD_MemCopy( cp_data->phrase, p_str, sizeof(STRCODE)*RESORT_PHRASE_LEN);
    *((p_str)+RESORT_PHRASE_LEN) = GFL_STR_GetEOMCode();
  }
  else
  {
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(HEAPID_PROC) );
    STRBUF *p_strbuf = GFL_MSG_CreateString( p_msg, resort_kutiguse_def_00+trainerID%RESORT_PHRASE_DEFAULT_NUM );
    GFL_STR_GetStringCode( p_strbuf, p_str, RESORT_PHRASE_LEN+EOM_SIZE );
    GFL_STR_DeleteBuffer( p_strbuf );
    GFL_MSG_Delete( p_msg );
  }
}
//-------------------------------------
///	拡張データ7
//=====================================
typedef struct
{
  STRCODE  thanks[RESORT_THANKS_LEN]; //EOMなし
} RESORT_EXTEND7_DATA;
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
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( (GAMEDATA *)cp_gamedata );
    RESONANCE_RESORT_SV *p_reso = RESONANCE_RESORT_SV_GetWork( p_sv );
    RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData( p_reso );
    RESORT_RECV_DATA *p_own = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
    RESORT_RECV_DATA_GetData( p_own, RESORT_DATA_PARAM_THANKS_CODE, p_wk->thanks );
  }
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
//----------------------------------------------------------------------------
/**
 *	@brief 感謝をEOM付きで取得
 *
 *	@param	const RESORT_EXTEND7_DATA *cp_data ワーク
 *	@param	*p_str  受け取りバッファ
 */
//-----------------------------------------------------------------------------
static void RESORT_EXTEND7_DATA_GetThanks( const RESORT_EXTEND7_DATA *cp_data, STRCODE *p_str, u16 trainerID )
{
  if( cp_data->thanks[0]  != GFL_STR_GetEOMCode() )
  {
    GFL_STD_MemCopy( cp_data->thanks, p_str, sizeof(STRCODE)*RESORT_THANKS_LEN);
    *((p_str)+RESORT_THANKS_LEN) = GFL_STR_GetEOMCode();
  }
  else
  {
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(HEAPID_PROC) );
    STRBUF *p_strbuf = GFL_MSG_CreateString( p_msg, resort_thanks_def_00+trainerID%RESORT_THANKS_DEFAULT_NUM );
    GFL_STR_GetStringCode( p_strbuf, p_str, RESORT_THANKS_LEN+EOM_SIZE );
    GFL_STR_DeleteBuffer( p_strbuf );
    GFL_MSG_Delete( p_msg );
  }
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
  u16                 present_bit;
  u8                  dummy10;
  u8                  meet_year;         //4
  u8                  meet_month;
  u8                  meet_day;
  u8                  meet_hour;
  u8                  meet_min;       //8
  u8                  commID;   //通信識別ID
  u8                  jobchange_day:1; //転職一日一回ビット
  u8                  replace_lock:1;   //置き換え不可ビット
  u8                  init_flag:1;   //初期化フラグ（初期化をABSTRUCT_MANAGER_InitにおこないROMアクセスをビーコン設定時に行わせないための処理）
  u8                  padding1  :4;
  u8                  day_flag  :1;
  u8                  keep_flag :1;
  u8                  want_spot;
  RESORT_INSTITUTION_DATA institution;
  u32                 kosei;  //未使用  //16
  u32                 extend_flag:9;  //20
  u32                 lock      :1; //受信（マージ・置き換え不可ビット）
  u32                 update    :3; //RESORT_RECV_DATA_UPDATEの値 
  u32                 peopleID  :8;
  u32                 placeID   :7;
  u32                 type     :4;  //自分がなんのタイプRESORT_RESIDENT_TYPE_xxx
  u32                 rnd_seed;     //乱数シード
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
  RESORT_COMM_TYPE    comm_type;
};

//-------------------------------------
///	３…共通構造体（PDW／GTS／ファン／セレブ）
//=====================================
struct _RESORT_PROXY_DATA
{
  RESORT_BASIC1_DATA  basic1;
  RESORT_EXTEND6_DATA extend6;
  RESORT_EXTEND7_DATA extend7;

  u8    id;
  u8    celevID;
  u8    type;
  u8    present_bit;
  u16   pokemon;
  u16   want_spot;
  u8    placeID;
  u8    peopleID;
  u8    comm_type;  //RESORT_COMM_TYPE

  u8                  meet_year;
  u8                  meet_month;
  u8                  meet_day;
  u16                 want_job;
  u32                 rnd_seed;     //乱数シード
};

SDK_COMPILER_ASSERT(sizeof(RESORT_PROXY_DATA)==RESORT_PROXY_DATA_SIZE);

//=============================================================================
/**
 *    RESORT_RECV_DATAのPRIVATE
 */
//=============================================================================
static void Resort_Recv_Data_InitInstitution( RESORT_RECV_DATA *p_wk, const RESORT_BINARY_SHOP *cp_shop );
static void Resort_Recv_Data_SetBeaconExtendData( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon );

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
  OS_TPrintf( "SetMyData1\n" );
  RESORT_BASIC1_DATA_SetMyData( &p_wk->basic1, cp_gamedata );
  OS_TPrintf( "SetMyData2\n" );
  RESORT_BASIC2_DATA_SetMyData( &p_wk->basic2, cp_gamedata );
  OS_TPrintf( "SetMyData3\n" );
  {
    int i;
    u8  *p_extend_adrs  = (u8*)&p_wk->extend1;
    for( i = 0; i < RESORT_EXTEND_TYPE_MAX; i++ )
    {
      OS_TPrintf( "SetExtend%d\n",i );
      sc_ResortExtendData_SetMyData[i]( (void*)p_extend_adrs, cp_gamedata );
      p_extend_adrs += RESORT_EXTEND_DATA_SIZE;
    }
  }

  p_wk->extend_flag = 0x1F;
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
  p_wk->extend6.phrase[0] = GFL_STR_GetEOMCode();
  p_wk->extend7.thanks[0] = GFL_STR_GetEOMCode();
  p_wk->basic1.introduction[0]  = GFL_STR_GetEOMCode();

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
  return RESORT_BASIC1_DATA_IsEmpty( &cp_wk->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データ  拡張があるか調べる
 *
 *	@param	const RESORT_RECV_DATA *cp_wk ワーク
 *	@param	type  調べる拡張
 *
 *	@return TRUEで存在  FALSEでなし
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_IsExtend( const RESORT_RECV_DATA *cp_wk,  RESORT_EXTEND_TYPE type )
{
  return (cp_wk->extend_flag & 1<<type) != 0;
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
 *	@brief  同じ所有者かチェック
 *
 *	@param	const RESORT_RECV_DATA *cp_wk   ワーク
 *	@param	const RESORT_RECV_DATA *cp_wk    受信
 *	@return TRUEで同じ所有者  FALSEで異なる所有者
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_IsSameOwnerRecv( const RESORT_RECV_DATA *cp_wk, const RESORT_RECV_DATA *cp_recv )
{
  return RESORT_BASIC1_DATA_IsSameOwner( &cp_wk->basic1, &cp_recv->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じ所有者かチェック
 *
 *	@param	const RESORT_RECV_DATA *cp_wk ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   代理人
 *
 *	@return TRUEで同じ所有者  FALSEで異なる所有者
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_IsSameOwnerProxy( const RESORT_RECV_DATA *cp_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  return RESORT_BASIC1_DATA_IsSameOwner( &cp_wk->basic1, &cp_proxy->basic1 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  初回生成時の設定  ここはフィールドのどこからも呼ばれるので
 *	                          ファイルアクセス禁止
 *
 *	@param	RESORT_RECV_DATA *p_wk  ワーク
 *	@param	type                    通信タイプ
 */
//-----------------------------------------------------------------------------
static void Resort_Recv_Data_InitSetup( RESORT_RECV_DATA *p_wk )
{

  NAGI_Printf( "ID[%d]\n", RESORT_RECV_DATA_GetData(p_wk,RESORT_DATA_PARAM_TRAINER_ID,NULL ) );

  //受信日時
  {
    RTCDate date;
    RTCTime time;
    GFL_RTC_GetDateTime(&date, &time);
    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_MEET_YEAR, date.year );
    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_MEET_MONTH, date.month );
    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_MEET_DAY, date.day );
    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_MEET_HOUR, time.hour );
    RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_MEET_MIN, time.minute );
  }

  //乱数
  RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_RND_SEED, GFUser_GetPublicRand(GFL_STD_RAND_MAX) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンを追加
 *
 *	@param	RESORT_RECV_DATA *p_wk          ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   マージするビーコン
 *
 *	@retval TRUE追加した  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_AddBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  GF_ASSERT( RESORT_RECV_DATA_IsEmpty( p_wk ) );

  //まずは初期化
  RESORT_RECV_DATA_InitInstance( p_wk );

  //通信経路
  RESORT_RECV_DATA_SetData( p_wk, RESORT_DATA_PARAM_COMM_ID, cp_beacon->comm_type );

  //ベーシックは必ずいれる
  p_wk->basic1  = cp_beacon->basic1;

  //パックの中身があったら入れる
  if( cp_beacon->extend_type < RESORT_EXTEND_TYPE_NULL )
  {
    p_wk->basic2  = cp_beacon->basic2;

    //入れる処理
    Resort_Recv_Data_SetBeaconExtendData( p_wk, cp_beacon );
  }

  //初期生成データ
  Resort_Recv_Data_InitSetup( p_wk );

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データを追加
 *
 *	@param	RESORT_RECV_DATA *p_wk      ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信データ
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_AddRecv( RESORT_RECV_DATA *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  GF_ASSERT( RESORT_RECV_DATA_IsEmpty( p_wk ) );

  //完全コピー
  *p_wk = *cp_recv;

  Resort_Recv_Data_InitSetup( p_wk );

  return TRUE;
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
RESORT_MERGE_RESULT RESORT_RECV_DATA_MergeBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  GF_ASSERT( !RESORT_RECV_DATA_IsEmpty( p_wk ) );

  //ロックされていたらマージしない
  if( RESORT_RECV_DATA_GetData(p_wk,RESORT_DATA_PARAM_LOCK,NULL) )
  {
    return RESORT_MERGE_RESULT_DISABLE;
  }
  else
  {
    //見た目が変わるか変更
    BOOL change_view   = p_wk->basic1.objcode != cp_beacon->basic1.objcode;

    //ベーシックは必ずマージ
    p_wk->basic1  = cp_beacon->basic1;

    //中身があったらマージ
    if( cp_beacon->extend_type <  RESORT_EXTEND_TYPE_NULL )
    {
      p_wk->basic2  = cp_beacon->basic2;

      //入れる処理
      Resort_Recv_Data_SetBeaconExtendData( p_wk, cp_beacon );
    };

    if( p_wk->extend_flag == 0x1F )
    {
      RESORT_RECV_Printf( "!!!すべてマージ完了\n!!!" );
    }

    if( change_view )
    {
      NAGI_Printf( "recv view %d\n", p_wk->basic1.objcode );
      RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE_VIEW);
    }
    else
    {
      RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE_DATA);
    }

    return RESORT_MERGE_RESULT_OK;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データをマージ
 *
 *	@param	RESORT_RECV_DATA *p_wk      ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信データ
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_RECV_DATA_MergeRecv( RESORT_RECV_DATA *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  GF_ASSERT( !RESORT_RECV_DATA_IsEmpty( p_wk ) );

  //ロック
  if( RESORT_RECV_DATA_GetData(p_wk,RESORT_DATA_PARAM_LOCK,NULL) )
  {
    return RESORT_MERGE_RESULT_DISABLE;
  }
  else
  {

    BOOL change_view   = p_wk->basic1.objcode != cp_recv->basic1.objcode;

    //必ずいれる
    p_wk->basic1  = cp_recv->basic1;
    p_wk->basic2  = cp_recv->basic2;

    //拡張パックがあるだけいれる
    {
      int i;
      const u8  *cp_extend_src  = (const u8*)&cp_recv->extend1;
      u8  *p_extend_dst  = (u8*)&p_wk->extend1;
      for( i = 0; i < RESORT_EXTEND_TYPE_MAX; i++ )
      {
        if( cp_recv->extend_flag & (1<<i) )
        {
          GFL_STD_MemCopy( cp_extend_src, p_extend_dst, RESORT_EXTEND_DATA_SIZE );
        }
        cp_extend_src += RESORT_EXTEND_DATA_SIZE;
        p_extend_dst += RESORT_EXTEND_DATA_SIZE;
      }
    }

    if( change_view )
    {
      RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE_VIEW);
    }
    else
    {
      RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE_DATA);
    }

    return RESORT_MERGE_RESULT_OK;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データをマージ
 *
 *	@param	RESORT_RECV_DATA *p_wk      ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy 代理人
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_RECV_DATA_MergeProxy( RESORT_RECV_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  GF_ASSERT( !RESORT_RECV_DATA_IsEmpty(p_wk ) );

  //ロック
  if( RESORT_RECV_DATA_GetData(p_wk,RESORT_DATA_PARAM_LOCK,NULL) )
  {
    return RESORT_MERGE_RESULT_DISABLE;
  }
  else
  {
    BOOL change_view   = p_wk->basic1.objcode != cp_proxy->basic1.objcode;

    //見た目だけマージ
    p_wk->basic1.objcode = cp_proxy->basic1.objcode;

    if( change_view )
    {
      RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE_VIEW);
    }
    else
    {
      RESORT_RECV_DATA_SetData(p_wk,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_MERGE_DATA);
    }
    return RESORT_MERGE_RESULT_OK;
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
  case RESORT_DATA_PARAM_SEX:
    return cp_wk->basic1.sex;
  case RESORT_DATA_PARAM_OBJCODE:
    return cp_wk->basic1.objcode;
  case RESORT_DATA_PARAM_NAME:
    RESORT_BASIC1_DATA_GetName(&cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_INTRODUCTION:
    RESORT_BASIC1_DATA_GetIntroduction( &cp_wk->basic1, p_ret_adrs );
    return 0;
  case RESORT_DATA_PARAM_EVENT_FLAG:
    return 0; //未使用
  case RESORT_DATA_PARAM_WANT_SPOT:
    return cp_wk->want_spot;
  case RESORT_DATA_PARAM_NATION:
    return cp_wk->basic1.nation;
  case RESORT_DATA_PARAM_AREA:
    return cp_wk->basic1.area;
  case RESORT_DATA_PARAM_PLAY_HOUR:
    return cp_wk->basic1.play_hour;
  case RESORT_DATA_PARAM_PLAY_MIN:
    return cp_wk->basic1.play_min;
  case RESORT_DATA_PARAM_JOB:
    return cp_wk->basic1.job;
  case RESORT_DATA_PARAM_HOBBY:
    return cp_wk->basic1.hobby;
  case RESORT_DATA_PARAM_TRAINER_ID:
    return cp_wk->basic1.trainerID;
  case RESORT_DATA_PARAM_VER:
    return cp_wk->basic1.version;
  case RESORT_DATA_PARAM_LANG:
    return cp_wk->basic1.language;
  //準基本データ
  case RESORT_DATA_PARAM_SPECIAL:
    return cp_wk->basic2.special_deliverly;
  case RESORT_DATA_PARAM_CITYLV:
    return cp_wk->basic2.city_lv;
  case RESORT_DATA_PARAM_WANT_JOB:
    return cp_wk->basic2.want_job;
  //生成データ
  case RESORT_DATA_PARAM_PRESENT_BIT:
    return cp_wk->present_bit;
  case RESORT_DATA_PARAM_MEET_YEAR:
    return cp_wk->meet_year;
  case RESORT_DATA_PARAM_MEET_MONTH:
    return cp_wk->meet_month;
  case RESORT_DATA_PARAM_MEET_DAY:
    return cp_wk->meet_day;
  case RESORT_DATA_PARAM_MEET_HOUR:
    return cp_wk->meet_hour;
  case RESORT_DATA_PARAM_MEET_MIN:
    return cp_wk->meet_min;
  case RESORT_DATA_PARAM_COMM_ID:
    return cp_wk->commID;
  case RESORT_DATA_PARAM_LOCK:
    return cp_wk->lock;
  case RESORT_DATA_PARAM_UPDATE:
    return cp_wk->update;
  case RESORT_DATA_PARAM_KEEP:
    return cp_wk->keep_flag;
  case RESORT_DATA_PARAM_DAY_FLAG:
    return cp_wk->day_flag;
  case RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG:
    return cp_wk->jobchange_day;
  case RESORT_DATA_PARAM_REPLACE_LOCK:
    return cp_wk->replace_lock;
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  case RESORT_DATA_PARAM_EXTEND_BIT:
    return cp_wk->extend_flag;
  case RESORT_DATA_PARAM_EXTEND_NUM:
    {
      int extend_num  = 0;
      int i;
      for( i = 0; i < RESORT_EXTEND_TYPE_MAX; i++ )
      {
        if( cp_wk->extend_flag & 1<<i )
        {
          extend_num++;
        }
      }
      return extend_num;
    }
  case RESORT_DATA_PARAM_INIT_FLAG:
    return cp_wk->init_flag;
  case RESORT_DATA_PARAM_KOSEI:
    return cp_wk->kosei;
  case RESORT_DATA_PARAM_RND_SEED:
    return cp_wk->rnd_seed;
  //拡張データ1
  case RESORT_DATA_PARAM_EXT1_RECOMMEND_MEDAL_ID:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_01 );
    return cp_wk->extend1.recommend_medalID;
  case RESORT_DATA_PARAM_EXT1_RECOMMEND_DATE:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_01 );
    return cp_wk->extend1.recommend_date;
  case RESORT_DATA_PARAM_EXT1_MEDAL_NUM:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_01 );
    return cp_wk->extend1.medal_num;
  case RESORT_DATA_PARAM_EXT1_MEDAL_BOX_RANK:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_01 );
    return cp_wk->extend1.medal_rank;
  case RESORT_DATA_PARAM_EXT1_START_DATE:
    return cp_wk->extend1.start_date;
  case RESORT_DATA_PARAM_EXT1_DENDOU_DATE:
    return cp_wk->extend1.dendou_date;
  case RESORT_DATA_PARAM_EXT1_ZUKAN:
    return cp_wk->extend1.zukan;
  case RESORT_DATA_PARAM_EXT1_FIRST_POKE:
    return cp_wk->extend1.first_poke;
  case RESORT_DATA_PARAM_EXT1_SHOP_LOT:
    return cp_wk->extend1.lot;
  case RESORT_DATA_PARAM_EXT1_SHOP_PARLOR:
    return cp_wk->extend1.parlor;
  case RESORT_DATA_PARAM_EXT1_SHOP_MARKET:
    return cp_wk->extend1.market;
  case RESORT_DATA_PARAM_EXT1_SHOP_FLORIST:
    return cp_wk->extend1.florist;
  case RESORT_DATA_PARAM_EXT1_SHOP_EXERCICE:
    return cp_wk->extend1.exercice;
  case RESORT_DATA_PARAM_EXT1_SHOP_NURSERY:
    return cp_wk->extend1.nursery;
  case RESORT_DATA_PARAM_EXT1_SHOP_ANTIQUE:
    return cp_wk->extend1.antique;
  case RESORT_DATA_PARAM_EXT1_SHOP_CAFE:
    return cp_wk->extend1.cafe;
  //拡張データ2
  case RESORT_DATA_PARAM_EXT2_RANK_00:
  case RESORT_DATA_PARAM_EXT2_RANK_01:
  case RESORT_DATA_PARAM_EXT2_RANK_02:
  case RESORT_DATA_PARAM_EXT2_RANK_03:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_02 );
    return cp_wk->extend2.rank_data[ param - RESORT_DATA_PARAM_EXT2_RANK_00 ];
  //拡張データ3
  case RESORT_DATA_PARAM_EXT3_RANK_04:
  case RESORT_DATA_PARAM_EXT3_RANK_05:
  case RESORT_DATA_PARAM_EXT3_RANK_06:
  case RESORT_DATA_PARAM_EXT3_RANK_07:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_03 );
    return cp_wk->extend3.rank_data[ param - RESORT_DATA_PARAM_EXT3_RANK_04 ];
  //拡張データ4
  case RESORT_DATA_PARAM_EXT4_GOSSIP_00:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_01:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_02:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_03:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_04:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_05:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_06:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_07:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_08:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_09:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_10:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_11:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_12:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_13:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_14:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_15:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_04 );
    return cp_wk->extend4.gossip[ param - RESORT_DATA_PARAM_EXT4_GOSSIP_00 ];
  //拡張データ5
  case RESORT_DATA_PARAM_EXT5_RECORD_00:
  case RESORT_DATA_PARAM_EXT5_RECORD_01:
  case RESORT_DATA_PARAM_EXT5_RECORD_02:
  case RESORT_DATA_PARAM_EXT5_RECORD_03:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_05 );
    return cp_wk->extend5.record[ param - RESORT_DATA_PARAM_EXT5_RECORD_00];
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_00:
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_01:
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_02:
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_03:
    RESORT_EXT_CHECK( cp_wk, RESORT_EXTEND_TYPE_05 );
    return cp_wk->extend5.record_date[ param - RESORT_DATA_PARAM_EXT5_RECORD_DATE_00];
  //拡張データ6
  case RESORT_DATA_PARAM_EXT6_PHRASE:
    RESORT_EXTEND6_DATA_GetPhrase( &cp_wk->extend6, (STRCODE *)p_ret_adrs, cp_wk->basic1.trainerID );
    return 0;
  //拡張データ7
  case RESORT_DATA_PARAM_EXT7_THANKS:
    RESORT_EXTEND7_DATA_GetThanks( &cp_wk->extend7, (STRCODE *)p_ret_adrs,cp_wk->basic1.trainerID );
    return 0;
  case RESORT_DATA_PARAM_PHRASE_CODE:  //p_ret_adrsから取得
    GFL_STD_MemCopy( cp_wk->extend6.phrase, p_ret_adrs, sizeof(STRCODE)*RESORT_PHRASE_LEN);
    *(((STRCODE *)p_ret_adrs)+RESORT_PHRASE_LEN) = GFL_STR_GetEOMCode();
    return 0;
  case RESORT_DATA_PARAM_THANKS_CODE:  //p_ret_adrsから取得
    GFL_STD_MemCopy( cp_wk->extend7.thanks, p_ret_adrs, sizeof(STRCODE)*RESORT_THANKS_LEN);
    *(((STRCODE *)p_ret_adrs)+RESORT_THANKS_LEN) = GFL_STR_GetEOMCode();
    return 0;
  default:
    GF_ASSERT_MSG(0,"%d",param);
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
  case RESORT_DATA_PARAM_WANT_SPOT:
    p_wk->want_spot = data;
    break;
  case RESORT_DATA_PARAM_NATION:
    p_wk->basic1.nation = data;
    break;
  case RESORT_DATA_PARAM_AREA:
    p_wk->basic1.area = data;
    break;
  case RESORT_DATA_PARAM_PLAY_HOUR:
    p_wk->basic1.play_hour = data;
    break;
  case RESORT_DATA_PARAM_PLAY_MIN:
    p_wk->basic1.play_min = data;
    break;
  case RESORT_DATA_PARAM_JOB:
    p_wk->basic1.job  = data;
    break;
  case RESORT_DATA_PARAM_HOBBY:
    p_wk->basic1.hobby  = data;
    break;
  case RESORT_DATA_PARAM_TRAINER_ID:
    p_wk->basic1.trainerID = data;
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  case RESORT_DATA_PARAM_VER:
    p_wk->basic1.version = data;
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  case RESORT_DATA_PARAM_LANG:
    p_wk->basic1.language = data;
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  case RESORT_DATA_PARAM_NAME:
    GFL_STD_MemCopy( (const void*)data, p_wk->basic1.name, sizeof(STRCODE)*PERSON_NAME_SIZE );
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  case RESORT_DATA_PARAM_INTRODUCTION:
    GFL_STD_MemCopy( (const void*)data, p_wk->basic1.introduction, sizeof(STRCODE)*GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN );
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  case RESORT_DATA_PARAM_WANT_JOB:
    p_wk->basic2.want_job  = data;
    break;
  case RESORT_DATA_PARAM_OBJCODE:
    p_wk->basic1.objcode = data;
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  case RESORT_DATA_PARAM_SEX:
    p_wk->basic1.sex = data;
    OS_TPrintf( "デバッグ用値設定\n" );
    break;
  //準基本データ
  case RESORT_DATA_PARAM_SPECIAL:
    p_wk->basic2.special_deliverly = data;
    break;
  case RESORT_DATA_PARAM_CITYLV:
    p_wk->basic2.city_lv  = data;
    break;
  //生成データ
  case RESORT_DATA_PARAM_PRESENT_BIT:
    p_wk->present_bit = data;
    break;
  case RESORT_DATA_PARAM_MEET_YEAR:
    p_wk->meet_year = data;
    break;
  case RESORT_DATA_PARAM_MEET_MONTH:
    p_wk->meet_month = data;
    break;
  case RESORT_DATA_PARAM_MEET_DAY:
    p_wk->meet_day = data;
    break;
  case RESORT_DATA_PARAM_MEET_HOUR:
    p_wk->meet_hour = data;
    break;
  case RESORT_DATA_PARAM_MEET_MIN:
    p_wk->meet_min = data;
    break;
  case RESORT_DATA_PARAM_COMM_ID:
    p_wk->commID = data;
    break;
  case RESORT_DATA_PARAM_LOCK:
    p_wk->lock = data;
    break;
  case RESORT_DATA_PARAM_UPDATE:
    //優先度高くないと入れられない
    if( p_wk->update == RESORT_RECV_DATA_UPDATE_NONE )
    {
      p_wk->update = data;
    }
    else if( p_wk->update > data )
    {
      p_wk->update = data;
    }
    break;
  case RESORT_DATA_PARAM_KEEP:
    p_wk->keep_flag = data;
    break;
  case RESORT_DATA_PARAM_DAY_FLAG:
    p_wk->day_flag = data;
    break;
  case RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG:
    p_wk->jobchange_day = data;
    break;
  case RESORT_DATA_PARAM_REPLACE_LOCK:
    p_wk->replace_lock  = data;
    break;
  case RESORT_DATA_PARAM_TYPE:
    p_wk->type = data;
    break;
  case RESORT_DATA_PARAM_EXTEND_BIT:
    p_wk->extend_flag = data;
    break;
  case RESORT_DATA_PARAM_EXTEND_ADD:
    GF_ASSERT( data < RESORT_EXTEND_TYPE_MAX );
    p_wk->extend_flag |= 1<<data;
    break;
  case RESORT_DATA_PARAM_INIT_FLAG:
    p_wk->init_flag = data;
    break;
  case RESORT_DATA_PARAM_KOSEI:
    p_wk->kosei = data;
    break;
  case RESORT_DATA_PARAM_RND_SEED:
    p_wk->rnd_seed  = data;
    break;
  case RESORT_DATA_PARAM_EXT1_RECOMMEND_MEDAL_ID:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_01 );
    p_wk->extend1.recommend_medalID = data;
    break;
  case RESORT_DATA_PARAM_EXT1_RECOMMEND_DATE:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_01 );
    p_wk->extend1.recommend_date = data;
    break;
  case RESORT_DATA_PARAM_EXT1_MEDAL_NUM:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_01 );
    p_wk->extend1.medal_num = data;
    break;
  case RESORT_DATA_PARAM_EXT1_MEDAL_BOX_RANK:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_01 );
    p_wk->extend1.medal_rank = data;
    break;
  case RESORT_DATA_PARAM_EXT1_START_DATE:
    p_wk->extend1.start_date  = data;
    break;
  case RESORT_DATA_PARAM_EXT1_DENDOU_DATE:
    p_wk->extend1.dendou_date = data;
    break;
  case RESORT_DATA_PARAM_EXT1_ZUKAN:
    p_wk->extend1.zukan = data;
    break;
  case RESORT_DATA_PARAM_EXT1_FIRST_POKE:
    p_wk->extend1.first_poke  = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_LOT:
    p_wk->extend1.lot = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_PARLOR:
    p_wk->extend1.parlor = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_MARKET:
    p_wk->extend1.market  = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_FLORIST:
    p_wk->extend1.florist = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_EXERCICE:
    p_wk->extend1.exercice  = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_NURSERY:
    p_wk->extend1.nursery = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_ANTIQUE:
    p_wk->extend1.antique = data;
    break;
  case RESORT_DATA_PARAM_EXT1_SHOP_CAFE:
    p_wk->extend1.cafe  = data;
    break;
  //拡張データ2
  case RESORT_DATA_PARAM_EXT2_RANK_00:
  case RESORT_DATA_PARAM_EXT2_RANK_01:
  case RESORT_DATA_PARAM_EXT2_RANK_02:
  case RESORT_DATA_PARAM_EXT2_RANK_03:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_02 );
    p_wk->extend2.rank_data[ param - RESORT_DATA_PARAM_EXT2_RANK_00 ] = data;
    break;
  //拡張データ3
  case RESORT_DATA_PARAM_EXT3_RANK_04:
  case RESORT_DATA_PARAM_EXT3_RANK_05:
  case RESORT_DATA_PARAM_EXT3_RANK_06:
  case RESORT_DATA_PARAM_EXT3_RANK_07:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_03 );
    p_wk->extend3.rank_data[ param - RESORT_DATA_PARAM_EXT3_RANK_04 ] = data;
    break;
  //拡張データ4
  case RESORT_DATA_PARAM_EXT4_GOSSIP_00:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_01:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_02:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_03:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_04:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_05:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_06:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_07:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_08:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_09:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_10:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_11:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_12:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_13:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_14:
  case RESORT_DATA_PARAM_EXT4_GOSSIP_15:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_04 );
    p_wk->extend4.gossip[ param - RESORT_DATA_PARAM_EXT4_GOSSIP_00 ] = data;
    break;
  //拡張データ5
  case RESORT_DATA_PARAM_EXT5_RECORD_00:
  case RESORT_DATA_PARAM_EXT5_RECORD_01:
  case RESORT_DATA_PARAM_EXT5_RECORD_02:
  case RESORT_DATA_PARAM_EXT5_RECORD_03:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_05 );
    p_wk->extend5.record[ param - RESORT_DATA_PARAM_EXT5_RECORD_00] = data;
    break;
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_00:
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_01:
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_02:
  case RESORT_DATA_PARAM_EXT5_RECORD_DATE_03:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_05 );
    p_wk->extend5.record_date[ param - RESORT_DATA_PARAM_EXT5_RECORD_DATE_00] = data;
    break;
  //拡張データ6
  case RESORT_DATA_PARAM_EXT6_PHRASE:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_06 );
    GFL_STD_MemCopy( (const void*)data, p_wk->extend6.phrase, sizeof(STRCODE)*RESORT_PHRASE_LEN );
    break;
  //拡張データ7
  case RESORT_DATA_PARAM_EXT7_THANKS:
    RESORT_EXT_CHECK( p_wk, RESORT_EXTEND_TYPE_07 );
    GFL_STD_MemCopy( (const void*)data, p_wk->extend7.thanks, sizeof(STRCODE)*RESORT_THANKS_LEN );
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  保存期間チェック
 *
 *	@param	const RESORT_RECV_DATA *p_wk ワーク
 *
 *	@return TRUE保存期間すぎた   FALSE保存期間内
 */
//-----------------------------------------------------------------------------
BOOL RESORT_RECV_DATA_OverReservePeriod( const RESORT_RECV_DATA *cp_wk, s64 now )
{
  s64 pre;

  if( cp_wk->replace_lock )
  {
    return FALSE;
  }

  //会った時間を取る
  {
    RTCDate date;
    RTCTime time;
    date.year   = cp_wk->meet_year;
    date.month  = cp_wk->meet_month;
    date.day    = cp_wk->meet_day;
    date.week   = 0;
    time.hour   = cp_wk->meet_hour;
    time.minute = cp_wk->meet_min;
    time.second = 0;
    pre = RTC_ConvertDateTimeToSecond( &date, &time );
  }

  {
    enum
    {
      SPECIAL_DELIVALY  = 180*60,
    };
    static const u32 sc_extend_num_time[RESORT_EXTEND_TYPE_MAX+1] =
    {
      1*60,3*60,4*60,5*60,7*60,8*60,1440*60
    };

    int extend_num  = RESORT_RECV_DATA_GetData( cp_wk, RESORT_DATA_PARAM_EXTEND_NUM, NULL );
    GF_ASSERT( extend_num < NELEMS(sc_extend_num_time) );

    if( cp_wk->basic2.special_deliverly )
    {
      return (s64)(now-pre) > SPECIAL_DELIVALY;
    }
    else
    {
      return (s64)(now-pre) > sc_extend_num_time[extend_num];
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  旅行者の乱数取得
 *
 *	@param	const RESORT_RECV_DATA *cp_wk ワーク
 *	@param	id  乱数使用ID
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_RECV_DATA_GetRnd( const RESORT_RECV_DATA *cp_wk, RESORT_RND_ID id, u32 range )
{
  u32 seed = RESORT_RECV_DATA_GetData( cp_wk, RESORT_DATA_PARAM_RND_SEED, NULL );
  return RESORT_DATA_GetRnd( seed, id, range );
}
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
  p_wk->comm_type = RESORT_COMM_TYPE_BEACON;

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
 *	@brief  ビーコンデータに拡張データをいれる（いれるのはaction.extend_buffのみ）
 *
 *  @param  cp_wk             ビーコンデータ
 *	@param	GAMEBEACON_INFO   *p_beacon_info   ビーコン
 */
//-----------------------------------------------------------------------------
void RESORT_BEACON_DATA_GetBeaconInfo( const RESORT_BEACON_DATA *cp_wk, GAMEBEACON_INFO *p_beacon_info )
{

  void * p_basic2_adrs = (u8 *)p_beacon_info->action.extend_buff;
  void * p_extend_adrs = (u8 *)p_beacon_info->action.extend_buff + RESORT_BASIC2_DATA_SIZE;

  GF_ASSERT( cp_wk->extend_type < RESORT_EXTEND_TYPE_MAX );

  //レゾ常駐を入れる
  {
    GFL_STD_MemCopy( &cp_wk->basic2, p_basic2_adrs, RESORT_BASIC2_DATA_SIZE );
  }
  //拡張データを入れる
  {
    GFL_STD_MemCopy( cp_wk->extend, p_extend_adrs, RESORT_EXTEND_DATA_SIZE );
  }
  //拡張データ番号を入れる

  p_beacon_info->action.resort_extend_type = cp_wk->extend_type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンからデータを取得
 *
 *	@param	RESORT_BEACON_DATA *p_wk  ワーク
 *	@param	GAMEBEACON_INFO *cp_beacon_info   ビーコンデータ
 */
//-----------------------------------------------------------------------------
void RESORT_BEACON_DATA_SetBeaconInfo( RESORT_BEACON_DATA *p_wk, const GAMEBEACON_INFO *cp_beacon_info, RESORT_COMM_TYPE comm_type )
{
  GF_ASSERT( comm_type == RESORT_COMM_TYPE_BEACON || comm_type == RESORT_COMM_TYPE_HYLINKFES );

  GFL_STD_MemClear( p_wk, sizeof(RESORT_BEACON_DATA) );
  p_wk->comm_type = comm_type;
  RESORT_BASIC1_DATA_SetBeaconInfo( &p_wk->basic1, cp_beacon_info );

  if( cp_beacon_info->pm_version == VERSION_W2 || cp_beacon_info->pm_version == VERSION_B2 )
  {
    if( comm_type == RESORT_COMM_TYPE_BEACON )
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
      p_wk->extend_type = RESORT_EXTEND_TYPE_NULL;
    }
  }
  else
  {
    //その他のソフトでは基本情報のみ
    p_wk->extend_type = RESORT_EXTEND_TYPE_NULL;
  }


//  OS_TFPrintf(2,"extend=%d\n",p_wk->extend_type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーID
 *
 *	@param	const RESORT_BEACON_DATA *cp_wk ワーク
 *
 *	@return トレーナーID
 */
//-----------------------------------------------------------------------------
u32 RESORT_BEACON_DATA_GetTrainerID( const RESORT_BEACON_DATA *cp_wk )
{
  return cp_wk->basic1.trainerID;
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
//----------------------------------------------------------------------------
/**
 *	@brief  空かチェック
 *
 *	@param	const RESORT_BEACON_DATA *cp_wk   ワーク
 *
 *	@return TRUE空  FALSE空じゃない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_BEACON_DATA_IsEmpty( const RESORT_BEACON_DATA *cp_wk )
{
  return RESORT_BASIC1_DATA_IsEmpty( &cp_wk->basic1 );
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
  p_wk->extend6.phrase[0] = GFL_STR_GetEOMCode();
  p_wk->extend7.thanks[0] = GFL_STR_GetEOMCode();
  p_wk->basic1.introduction[0]  = GFL_STR_GetEOMCode();
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
  return RESORT_BASIC1_DATA_IsEmpty( &cp_wk->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  不正かチェック
 *
 *	@param	const RESORT_PROXY_DATA *cp_wk  ワーク
 *
 *	@return TRUE不正じゃない  FALSE不正
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_DATA_IsValid( const RESORT_PROXY_DATA *cp_wk )
{
  if( cp_wk->type == RESORT_PROXY_TYPE_PDW || cp_wk->type == RESORT_PROXY_TYPE_GTS )
  {
    if( !RESORT_BASIC1_DATA_IsValid( &cp_wk->basic1 ) )
    {
      return FALSE;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データを設定(GTS, PDW)
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param  MYSTATUS  マイステータス
 *	@param	type  代理人タイプ  RESORT_PROXY_TYPE_xxxx
 */
//-----------------------------------------------------------------------------
void RESORT_PROXY_DATA_SetCommData( RESORT_PROXY_DATA *p_wk, const MYSTATUS *cp_status, u16 pokemon, RESORT_COMM_TYPE comm_type )
{
  RESORT_PROXY_DATA_InitInstance( p_wk );

  switch( comm_type )
  {
  default:
    GF_ASSERT(0);
    comm_type = RESORT_COMM_TYPE_GTS;
    /*  fall  */
  case RESORT_COMM_TYPE_GTS:
    p_wk->type = RESORT_PROXY_TYPE_GTS;
    break;
  case  RESORT_COMM_TYPE_PDW:
    p_wk->type = RESORT_PROXY_TYPE_PDW;
  }

  p_wk->comm_type = comm_type;
  p_wk->want_job  = 0;

  RESORT_BASIC1_DATA_SetMyStatus( &p_wk->basic1, cp_status );

  //ポケモン
  p_wk->pokemon = pokemon;


  //乱数
  p_wk->rnd_seed = GFUser_GetPublicRand(GFL_STD_RAND_MAX);

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
    return cp_wk->basic1.sex;
  case RESORT_DATA_PARAM_OBJCODE:
    return cp_wk->basic1.objcode;
  case RESORT_DATA_PARAM_NAME:
    {
      STRCODE *p_str  = p_ret_adrs;
      GFL_STD_MemCopy( cp_wk->basic1.name, p_str, sizeof(STRCODE)*(PERSON_NAME_SIZE) );
      p_str[PERSON_NAME_SIZE] = GFL_STR_GetEOMCode();
    }
    return 0;
  case RESORT_DATA_PARAM_PRESENT_BIT:
    return cp_wk->present_bit;
  case RESORT_DATA_PARAM_RND_SEED:
    return cp_wk->rnd_seed;
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  case RESORT_DATA_PARAM_COMM_ID:
    return cp_wk->comm_type;
  case RESORT_DATA_PARAM_WANT_JOB:
    return cp_wk->want_job;
  case RESORT_DATA_PARAM_NATION:
    return cp_wk->basic1.nation;
  case RESORT_DATA_PARAM_AREA:
    return cp_wk->basic1.area;
  case RESORT_DATA_PARAM_PLAY_HOUR:
    return 0;
  case RESORT_DATA_PARAM_PLAY_MIN:
    return 0;
  case RESORT_DATA_PARAM_TRAINER_ID:
    return cp_wk->basic1.trainerID;
  case RESORT_DATA_PARAM_VER:
    return cp_wk->basic1.version;
  case RESORT_DATA_PARAM_LANG:
    return cp_wk->basic1.language;

  case RESORT_DATA_PARAM_EXT6_PHRASE:
    RESORT_EXTEND6_DATA_GetPhrase( &cp_wk->extend6, p_ret_adrs, cp_wk->basic1.trainerID );
    return 0;
  case RESORT_DATA_PARAM_EXT7_THANKS:
    RESORT_EXTEND7_DATA_GetThanks( &cp_wk->extend7, p_ret_adrs, cp_wk->basic1.trainerID );
    return 0;
  case RESORT_DATA_PARAM_INTRODUCTION:
    {
      GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(HEAPID_PROC) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_introduction_def_00+cp_wk->basic1.trainerID%RESORT_INTRODUCTION_DEFAULT_NUM );
      GFL_STR_GetStringCode( p_str, (STRCODE*)p_ret_adrs, GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE );
      GFL_STR_DeleteBuffer( p_str );
      GFL_MSG_Delete( p_msg );
    }
    return 0;
  case RESORT_DATA_PARAM_EXTEND_NUM:
    return 0;
  case RESORT_DATA_PARAM_WANT_SPOT:
    return cp_wk->want_spot;

  case RESORT_DATA_PARAM_MEET_YEAR:
    return cp_wk->meet_year;
  case RESORT_DATA_PARAM_MEET_MONTH:
    return cp_wk->meet_month;
  case RESORT_DATA_PARAM_MEET_DAY:
    return cp_wk->meet_day;
  case RESORT_DATA_PARAM_SPECIAL:
    return FALSE;

  default:
    GF_ASSERT_MSG(0,"未対応%d\n",param);
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
  case RESORT_DATA_PARAM_NAME:
    GFL_STD_MemCopy( (const void*)data, p_wk->basic1.name, sizeof(STRCODE)*(PERSON_NAME_SIZE) );
    break;

  case RESORT_DATA_PARAM_EXT6_PHRASE:
    GFL_STD_MemCopy( (const void*)data, p_wk->extend6.phrase, sizeof(STRCODE)*RESORT_PHRASE_LEN );
    break;
  case RESORT_DATA_PARAM_EXT7_THANKS:
    GFL_STD_MemCopy( (const void*)data, p_wk->extend7.thanks, sizeof(STRCODE)*RESORT_THANKS_LEN );
    break;
  case RESORT_DATA_PARAM_INTRODUCTION:
    GFL_STD_MemCopy( (const void*)data, p_wk->basic1.introduction, sizeof(STRCODE)*GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN );
    break;
  case RESORT_DATA_PARAM_SEX:
    p_wk->basic1.sex  = data;
    break;
  case RESORT_DATA_PARAM_VER:
    p_wk->basic1.version = data;
    break;
  case RESORT_DATA_PARAM_LANG:
    p_wk->basic1.language = data;
    break;
  case RESORT_DATA_PARAM_RND_SEED:
    p_wk->rnd_seed  = data;
    break;
  case RESORT_DATA_PARAM_OBJCODE:
    p_wk->basic1.objcode  = data;
    break;
  case RESORT_DATA_PARAM_TRAINER_ID:
    p_wk->basic1.trainerID = data;
    break;
  case RESORT_DATA_PARAM_REPLACE_LOCK:
    /*  何もしない  */
    break;
  case RESORT_DATA_PARAM_TYPE:
    p_wk->type  = data;
    break;
  case RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG:
    /*  何もしない  */
    break;
  case RESORT_DATA_PARAM_WANT_SPOT:
    p_wk->want_spot = data;
    break;
  case RESORT_DATA_PARAM_WANT_JOB:
    p_wk->want_job  = data;
    break;

  case RESORT_DATA_PARAM_MEET_YEAR:
    p_wk->meet_year = data;
    break;
  case RESORT_DATA_PARAM_MEET_MONTH:
    p_wk->meet_month  = data;
    break;
  case RESORT_DATA_PARAM_MEET_DAY:
    p_wk->meet_day  = data;
    break;
  case RESORT_DATA_PARAM_PRESENT_BIT:
   p_wk->present_bit  = data;
   break;
  default:
    GF_ASSERT_MSG(0,"未対応%d\n",param);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人をマージ
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   代理人
 *
 *	@return TRUEマージした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_DATA_MergeProxy( RESORT_PROXY_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  GF_ASSERT( !RESORT_PROXY_DATA_IsEmpty(p_wk) );

  p_wk->basic1  = cp_proxy->basic1;
  p_wk->extend6  = cp_proxy->extend6;
  p_wk->extend7  = cp_proxy->extend7;

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人をセット
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   代理人
 *
 *	@return TRUEセットした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_DATA_AddProxy( RESORT_PROXY_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  GF_ASSERT( RESORT_PROXY_DATA_IsEmpty(p_wk) );
  *p_wk = *cp_proxy;
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人用の乱数取得
 *
 *	@param	const RESORT_PROXY_DATA *cp_wk ワーク
 *	@param	id  乱数使用ID
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_PROXY_DATA_GetRnd( const RESORT_PROXY_DATA *cp_wk, RESORT_RND_ID id, u32 range )
{
  u32 seed = RESORT_PROXY_DATA_GetData( cp_wk, RESORT_DATA_PARAM_RND_SEED, NULL );
  return RESORT_DATA_GetRnd( seed, id, range );
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
static RESORT_PROXY_DATA * Resort_Proxy_Buff_GetSameOwnerData( RESORT_PROXY_BUFF *p_wk, const RESORT_BASIC1_DATA *cp_owner );

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
RESORT_ADD_RESULT RESORT_PROXY_BUFF_Add( RESORT_PROXY_BUFF *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  RESORT_PROXY_DATA *p_data;

  if( !RESORT_PROXY_DATA_IsValid(cp_proxy) )
  {
    return RESORT_ADD_RESULT_DISABLE;
  }

  //空のデータがあったら入れる
  p_data  = Resort_Proxy_Buff_GetEmptyData( p_wk );
  if( p_data )
  {
    RESORT_PROXY_DATA_AddProxy( p_data, cp_proxy );
    RESORT_RECV_Printf( "空のデータに入れた！\n" );
    return RESORT_ADD_RESULT_OK;
  }

  return RESORT_ADD_RESULT_NG;
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
 *	@brief  バッファの数取得
 *
 *	@param	const RESORT_PROXY_BUFF *cp_wk  ワーク
 *
 *	@return バッファの数取得
 */
//-----------------------------------------------------------------------------
u32 RESORT_PROXY_BUFF_GetCnt( const RESORT_PROXY_BUFF *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->max; i++ )
  {
    const RESORT_PROXY_DATA *cp_data  = &cp_wk->data[i];
    if( !RESORT_PROXY_DATA_IsEmpty( cp_data ) )
    {
      cnt++;
    }
  }
  return cnt;
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
 *	@brief  ビーコンをマージ
 *
 *	@param	RESORT_PROXY_BUFF *p_wk ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   ビーコン
 *
 *	@return TRUEマージorセットした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_BUFF_MergeBeacon( RESORT_PROXY_BUFF *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  RESORT_PROXY_DATA *p_data = Resort_Proxy_Buff_GetSameOwnerData( p_wk, &cp_beacon->basic1 );
  if( p_data )
  {
    //何もしない。
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データをマージ
 *
 *	@param	RESORT_PROXY_BUFF *p_wk ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   ビーコン
 *
 *	@return TRUEマージorセットした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_BUFF_MergeRecv( RESORT_PROXY_BUFF *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  RESORT_PROXY_DATA *p_data = Resort_Proxy_Buff_GetSameOwnerData( p_wk, &cp_recv->basic1 );
  if( p_data )
  {
    //何もしない。
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データをマージ
 *
 *	@param	RESORT_PROXY_BUFF *p_wk       ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   代理人データ
 *
 *	@return TRUEマージorセットした  FALSEしていない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_PROXY_BUFF_MergeProxy( RESORT_PROXY_BUFF *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  RESORT_PROXY_DATA *p_data = Resort_Proxy_Buff_GetSameOwnerData( p_wk, &cp_proxy->basic1 );
  if( p_data )
  {
    RESORT_PROXY_DATA_MergeProxy( p_data, cp_proxy );
    return TRUE;
  }

  return FALSE;
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
//----------------------------------------------------------------------------
/**
 *	@brief  同じ所有者のデータを収録
 *
 *	@param	RESORT_PROXY_BUFF *p_wk ワーク
 *	@param	RESORT_BASIC1_DATA *cp_owner  比べる基本データ
 *
 *	@return 代理人データ
 */
//-----------------------------------------------------------------------------
static RESORT_PROXY_DATA * Resort_Proxy_Buff_GetSameOwnerData( RESORT_PROXY_BUFF *p_wk, const RESORT_BASIC1_DATA *cp_owner )
{
  int i;
  for( i = 0; i < p_wk->max; i++ )
  {
    RESORT_PROXY_DATA  *p_data = &p_wk->data[i];
    if( RESORT_BASIC1_DATA_IsSameOwner( &p_data->basic1, cp_owner ) )
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
static RESORT_RECV_DATA * Resort_Recv_Buff_GetOverReserveData( RESORT_RECV_BUFF *p_wk );

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
  if( p_wk == NULL )
  {
    return NULL;
  }

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
 *	@brief  バッファにあるデータ数
 *
 *	@param	const RESORT_RECV_BUFF *cp_wk ワーク
 *
 *	@return バッファにあるデータ数
 */
//-----------------------------------------------------------------------------
u32 RESORT_RECV_BUFF_GetCnt( const RESORT_RECV_BUFF *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->max; i++ )
  {
    const RESORT_RECV_DATA *cp_data  = &cp_wk->data[i];
    if( !RESORT_RECV_DATA_IsEmpty( cp_data ) )
    {
      cnt++;
    }
  }
  return cnt;
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
RESORT_MERGE_RESULT RESORT_RECV_BUFF_MergeBeacon( RESORT_RECV_BUFF *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  RESORT_RECV_DATA *p_data;


  //同じ所有者のものがあったら優先的に入れる
  p_data  = Resort_Recv_Buff_GetSameOwnerData( p_wk, &cp_beacon->basic1 );
  if( p_data )
  {
    //ロック中は追加しない
    if( p_wk->is_lock )
    {
      RESORT_RECV_Printf( "リゾート　ロック中\n" );
      return RESORT_MERGE_RESULT_DISABLE;
    }
    else
    {
      return RESORT_RECV_DATA_MergeBeacon( p_data, cp_beacon );
    }
  }

  return RESORT_MERGE_RESULT_NG;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ受信バッファにビーコン差込
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_recv 受信したデータ
 *	@param  RESORT_COMM_TYPE type 通信タイプ
 *
 *	@retval TRUEビーコン挿し込みした  FALSEしていない*
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_RECV_BUFF_MergeRecv( RESORT_RECV_BUFF *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  RESORT_RECV_DATA *p_data;


  //同じ所有者のものがあったら優先的に入れる
  p_data  = Resort_Recv_Buff_GetSameOwnerData( p_wk, &cp_recv->basic1 );
  if( p_data )
  {
    //ロック中は追加しない
    if( p_wk->is_lock )
    {
      RESORT_RECV_Printf( "リゾート　ロック中\n" );
      return RESORT_MERGE_RESULT_DISABLE;
    }
    else
    {
      return RESORT_RECV_DATA_MergeRecv( p_data, cp_recv );
    }
  }

  return RESORT_MERGE_RESULT_NG;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人バッファをマージ
 *  
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   代理人バッファ
 *
 *	@return TRUEマージした  FALSEしていない
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_RECV_BUFF_MergeProxy( RESORT_RECV_BUFF *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  RESORT_RECV_DATA *p_data;


  //同じ所有者のものがあったら優先的に入れる
  p_data  = Resort_Recv_Buff_GetSameOwnerData( p_wk, &cp_proxy->basic1 );
  if( p_data )
  {
    //ロック中は追加しない
    if( p_wk->is_lock )
    {
      RESORT_RECV_Printf( "リゾート　ロック中\n" );
      return RESORT_MERGE_RESULT_DISABLE;
    }
    else
    {
      return RESORT_RECV_DATA_MergeProxy( p_data, cp_proxy );
    }
  }

  return RESORT_MERGE_RESULT_NG;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンをセット
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon ビーコン
 *
 *	@return TRUEビーコンをセット  FALSEしていない
 */
//-----------------------------------------------------------------------------
RESORT_ADD_RESULT RESORT_RECV_BUFF_AddBeacon( RESORT_RECV_BUFF *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  RESORT_RECV_DATA *p_data;

  //ロック中は追加しない
  if( p_wk->is_lock )
  {
    RESORT_RECV_Printf( "リゾート　ロック中\n" );
    return RESORT_ADD_RESULT_DISABLE;
  }

  {
    u32 trainerID = RESORT_BEACON_DATA_GetTrainerID( cp_beacon );
    //連続で受信チェック
    if( RESORT_CITY_DATA_CheckTrainerID(sp_city_data, trainerID ) == FALSE )
    {
      //空のデータがあったら入れる
      p_data  = Resort_Recv_Buff_GetEmptyData( p_wk );
      if( p_data )
      {
        RESORT_RECV_DATA_AddBeacon( p_data, cp_beacon );
        RESORT_RECV_DATA_SetData(p_data,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_FIRST);

#ifdef  PM_DEBUG
        RESORT_BASIC1_DATA_PrintDebug( &cp_beacon->basic1 );
#endif
        RESORT_RECV_Printf( "空のデータに入れた！%d\n", cp_beacon->extend_type );

        RESORT_CITY_DATA_AddTrainerID( sp_city_data, trainerID );
        return RESORT_ADD_RESULT_OK;
      }

      //期間外のデータがあったら入れる
      p_data = Resort_Recv_Buff_GetOverReserveData( p_wk );
      if( p_data )
      {
        RESORT_RECV_DATA_InitInstance( p_data );
        RESORT_RECV_DATA_AddBeacon( p_data, cp_beacon );
        RESORT_RECV_DATA_SetData(p_data,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_OVERWRITE);

#ifdef  PM_DEBUG
        RESORT_BASIC1_DATA_PrintDebug( &cp_beacon->basic1 );
#endif
        RESORT_RECV_Printf( "受信データを押し出して入れた！\n" );

        RESORT_CITY_DATA_AddTrainerID( sp_city_data, trainerID );
        return RESORT_ADD_RESULT_OK;
      }
    }
  }

  return RESORT_ADD_RESULT_NG;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データをセット
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信データ
 *
 *	@return TRUE受信データをセット  FALSEしていない
 */
//-----------------------------------------------------------------------------
RESORT_ADD_RESULT RESORT_RECV_BUFF_AddRecv( RESORT_RECV_BUFF *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  RESORT_RECV_DATA *p_data;

  //ロック中は追加しない
  if( p_wk->is_lock )
  {
    RESORT_RECV_Printf( "リゾート　ロック中\n" );
    return RESORT_ADD_RESULT_DISABLE;
  }

  //空のデータがあったら入れる
  p_data  = Resort_Recv_Buff_GetEmptyData( p_wk );
  if( p_data )
  {
    RESORT_RECV_DATA_AddRecv(p_data, cp_recv);
    RESORT_RECV_DATA_SetData(p_data,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_FIRST);

#ifdef  PM_DEBUG
    RESORT_BASIC1_DATA_PrintDebug( &cp_recv->basic1 );
#endif
    RESORT_RECV_Printf( "受信データを空のデータに入れた！\n" );
    return RESORT_ADD_RESULT_OK;
  }

  //期間外のデータがあったら入れる
  p_data = Resort_Recv_Buff_GetOverReserveData( p_wk );

  if( p_data )
  {
    RESORT_RECV_DATA_InitInstance( p_data );
    RESORT_RECV_DATA_AddRecv(p_data, cp_recv);
    RESORT_RECV_DATA_SetData(p_data,RESORT_DATA_PARAM_UPDATE,RESORT_RECV_DATA_UPDATE_OVERWRITE);
#ifdef  PM_DEBUG
    RESORT_BASIC1_DATA_PrintDebug( &cp_recv->basic1 );
#endif
    RESORT_RECV_Printf( "受信データを押し出して入れた！\n" );
    return RESORT_ADD_RESULT_OK;
  }

  return RESORT_ADD_RESULT_NG;
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
//----------------------------------------------------------------------------
/**
 *	@brief  保存期間終了データを取得する
 *
 *	@param	RESORT_RECV_BUFF *p_wk  ワーク
 *
 *	@return 保存期間終了データ  なければNULL
 */
//-----------------------------------------------------------------------------
static RESORT_RECV_DATA * Resort_Recv_Buff_GetOverReserveData( RESORT_RECV_BUFF *p_wk )
{
  int i;

  //現在秒を取る
  RTCDate date;
  RTCTime time;
  s64 now;

  GFL_RTC_GetDateTime( &date, &time );
  now = RTC_ConvertDateTimeToSecond( &date, &time );

  for( i = 0; i < p_wk->max; i++ )
  {
    RESORT_RECV_DATA  *p_data = &p_wk->data[i];
    if( !RESORT_RECV_DATA_IsEmpty(p_data) )
    {
      if( RESORT_RECV_DATA_OverReservePeriod(p_data, now) )
      {
        return p_data;
      }
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
void RESORT_RESIDENT_DATA_ConvertRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID )
{
  if( p_wk != cp_recv )
  {
    *p_wk   = *cp_recv;
  }

  //場所を設定
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;

  //ロックフラグ解除
  RESORT_RESIDENT_DATA_SetData(p_wk,RESORT_DATA_PARAM_LOCK,FALSE);

  RESORT_RESIDENT_DATA_JobChange( p_wk, cp_shop );
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データを住民データに設定
 *
 *	@param	RESORT_RESIDENT_DATA *p_wk  ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy 受信データ
 *	@param	peopleID  人物ID
 *	@param	placeID   場所ID
 */
//-----------------------------------------------------------------------------
void RESORT_RESIDENT_DATA_ConvertProxy( RESORT_RESIDENT_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID )
{
  RESORT_RESIDENT_DATA_InitInstance( p_wk );
  p_wk->basic1  = cp_proxy->basic1;
  p_wk->extend6 = cp_proxy->extend6;
  p_wk->extend7 = cp_proxy->extend7;

  p_wk->meet_year = cp_proxy->meet_year;
  p_wk->meet_month = cp_proxy->meet_month;
  p_wk->meet_day  = cp_proxy->meet_day;

  p_wk->basic2.want_job  = cp_proxy->want_job;

  //場所を設定
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;

  //ロックフラグ解除
  RESORT_RESIDENT_DATA_SetData(p_wk,RESORT_DATA_PARAM_LOCK,FALSE);
  RESORT_RESIDENT_DATA_JobChange( p_wk,cp_shop );
}

//----------------------------------------------------------------------------
/**
 *	@brief  職業設定
 *
 *	@param	RESORT_RECV_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Recv_Data_InitInstitution( RESORT_RECV_DATA *p_wk, const RESORT_BINARY_SHOP *cp_shop )
{
  int want_job  = RESORT_RESIDENT_DATA_GetData( p_wk, RESORT_DATA_PARAM_WANT_JOB, NULL );
  int id  = RESORT_RESIDENT_DATA_GetData( p_wk, RESORT_DATA_PARAM_TRAINER_ID, NULL );
  int ver  = RESORT_RESIDENT_DATA_GetData( p_wk, RESORT_DATA_PARAM_VER, NULL );

  int sp  = RESORT_RESIDENT_DATA_GetData( p_wk, RESORT_DATA_PARAM_SPECIAL, NULL );

  u16 shopID  = RESORT_DATA_GetWantJobModify( want_job, id, ver, sp, sp_city_data, cp_shop );


  const RESORT_BINARY_SHOP_DATA* cp_shop_data = RESORT_BINARY_SHOP_GetData(cp_shop, shopID );
  u16 lv  = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_LV );


  {
    RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution(p_wk);

    RESORT_INSTITUTION_DATA_InitInstance( p_institution );
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID, shopID );
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, lv );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンの拡張情報を設定
 *
 *	@param	RESORT_RECV_DATA *p_wk　ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon ビーコン
 */
//-----------------------------------------------------------------------------
static void Resort_Recv_Data_SetBeaconExtendData( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  if( cp_beacon->extend_type < RESORT_EXTEND_TYPE_NULL )
  {
    u8  *p_extend_adrs  = (u8*)&p_wk->extend1;
    p_extend_adrs += (RESORT_EXTEND_DATA_SIZE * cp_beacon->extend_type);
    GFL_STD_MemCopy( cp_beacon->extend, p_extend_adrs, RESORT_EXTEND_DATA_SIZE );

    //拡張マージフラグ
    p_wk->extend_flag |=  1<<cp_beacon->extend_type;
  }
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
//----------------------------------------------------------------------------
/**
 *	@brief  転職
 *
 *	@param	RESORT_RESIDENT_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RESIDENT_DATA_JobChange( RESORT_RESIDENT_DATA *p_wk, const RESORT_BINARY_SHOP *cp_shop )
{
  Resort_Recv_Data_InitInstitution( p_wk, cp_shop );
  {
    const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( cp_shop, p_wk );

    p_wk->type      = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
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
  u8    placeID;
  u8    peopleID;
  u8    type;
  u8    dummy;
  u8    meet_year;
  u8    meet_month;
  u8    meet_day;
  u8    present_bit;
  RESORT_EXTEND6_DATA extend6;
  RESORT_EXTEND7_DATA extend7;
  u32   rnd_seed;
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
 *	@brief  同じ人物かチェック  ビーコン版
 *
 *	@param	const RESORT_SECRETARY_DATA *cp_wk  ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon ビーコン
 *
 *	@return TRUE同じ  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_SECRETARY_DATA_IsSameOwnerBeacon( const RESORT_SECRETARY_DATA *cp_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  return RESORT_BASIC1_DATA_IsSameOwner( &cp_wk->basic1, &cp_beacon->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じ人物かチェック  受信データ版
 *
 *	@param	const RESORT_SECRETARY_DATA *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_recv   受信データ
 *
 *	@return TRUE同じ  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_SECRETARY_DATA_IsSameOwnerRecv( const RESORT_SECRETARY_DATA *cp_wk, const RESORT_RECV_DATA *cp_recv )
{
  return RESORT_BASIC1_DATA_IsSameOwner( &cp_wk->basic1, &cp_recv->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じ人物かチェック  代理人版
 *
 *	@param	const RESORT_SECRETARY_DATA *cp_wk  ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy         代理人
 *
 *	@return TRUE同じ  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_SECRETARY_DATA_IsSameOwnerProxy( const RESORT_SECRETARY_DATA *cp_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  return RESORT_BASIC1_DATA_IsSameOwner( &cp_wk->basic1, &cp_proxy->basic1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンをマージ
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk     ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   マージするビーコン
 *
 *	@return TRUEマージした  FALSEマージしていない
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_SECRETARY_DATA_MergeBeacon( RESORT_SECRETARY_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  GF_ASSERT( !RESORT_SECRETARY_DATA_IsEmpty( p_wk ) );

  //ベーシックは必ずマージ
  p_wk->basic1  = cp_beacon->basic1;

  if( cp_beacon->extend_type ==  RESORT_EXTEND_TYPE_06 )
  {
    GFL_STD_MemCopy( cp_beacon->extend, &p_wk->extend6, RESORT_EXTEND_DATA_SIZE );
  }
  else if( cp_beacon->extend_type ==  RESORT_EXTEND_TYPE_07 )
  {
    GFL_STD_MemCopy( cp_beacon->extend, &p_wk->extend7, RESORT_EXTEND_DATA_SIZE );
  }

  //秘書は見た目とフリーワードが変わるだけなのでフラグたてる必要ないはず

  return RESORT_MERGE_RESULT_OK;
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信データをマージ
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 *	@param	RESORT_RECV_DATA *cp_recv マージする受信データ
 *	@param	type  通信タイプ
 *
 *	@return TRUEマージした  FALSEマージしていない
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_SECRETARY_DATA_MergeRecv( RESORT_SECRETARY_DATA *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  GF_ASSERT( !RESORT_SECRETARY_DATA_IsEmpty( p_wk ) );

  //ベーシックは必ずマージ
  p_wk->basic1  = cp_recv->basic1;

  if( cp_recv->extend_flag & (1<<RESORT_EXTEND_TYPE_06) )
  {
    p_wk->extend6 = cp_recv->extend6;
  }

  if( cp_recv->extend_flag & (1<<RESORT_EXTEND_TYPE_07) )
  {
    p_wk->extend7 = cp_recv->extend7;
  }

  //秘書は見た目とフリーワードが変わるだけなのでフラグたてる必要ないはず

  return RESORT_MERGE_RESULT_OK;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人をマージ
 *  
 *	@param	RESORT_SECRETARY_DATA *p_wk   ワーク
 *	@param	RESORT_PROXY_DATA *cp_proxy   マージする代理人データ
 *
 *	@return TRUEマージした  FALSEマージしていない
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_SECRETARY_DATA_MergeProxy( RESORT_SECRETARY_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy )
{
  GF_ASSERT( !RESORT_SECRETARY_DATA_IsEmpty( p_wk ) );

  //見た目だけマージ
  p_wk->basic1.objcode = cp_proxy->basic1.objcode;

  return RESORT_MERGE_RESULT_OK;
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
  case RESORT_DATA_PARAM_OBJCODE:
    return cp_wk->basic1.objcode; 
  case RESORT_DATA_PARAM_PRESENT_BIT:
    return cp_wk->present_bit;
  case RESORT_DATA_PARAM_RND_SEED:
    return cp_wk->rnd_seed;
  case RESORT_DATA_PARAM_MEET_YEAR:
    return cp_wk->meet_year;
  case RESORT_DATA_PARAM_MEET_MONTH:
    return cp_wk->meet_month;
  case RESORT_DATA_PARAM_MEET_DAY:
    return cp_wk->meet_day;
  case RESORT_DATA_PARAM_TYPE:
    return cp_wk->type;
  case RESORT_DATA_PARAM_EXT6_PHRASE:  //p_ret_adrsから取得
    RESORT_EXTEND6_DATA_GetPhrase( &cp_wk->extend6, p_ret_adrs, cp_wk->basic1.trainerID );
    return 0;
  case RESORT_DATA_PARAM_EXT7_THANKS:  //p_ret_adrsから取得
    RESORT_EXTEND7_DATA_GetThanks( &cp_wk->extend7, p_ret_adrs,cp_wk->basic1.trainerID );
    return 0;
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
  case RESORT_DATA_PARAM_MEET_YEAR:
    p_wk->meet_year = data;
    break;
  case RESORT_DATA_PARAM_MEET_MONTH:
    p_wk->meet_month = data;
    break;
  case RESORT_DATA_PARAM_MEET_DAY:
    p_wk->meet_day = data;
    break;
  case RESORT_DATA_PARAM_PRESENT_BIT:
    p_wk->present_bit = data;
    break;
  case RESORT_DATA_PARAM_RND_SEED:
    p_wk->rnd_seed  = data;
    break;
  case RESORT_DATA_PARAM_LOCK:
    //なにもしない
    break;
  case RESORT_DATA_PARAM_TYPE:
    p_wk->type = data;
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
void RESORT_SECRETARY_DATA_ConvertRecv( RESORT_SECRETARY_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type )
{
  RESORT_SECRETARY_DATA_InitInstance( p_wk );

  p_wk->basic1    = cp_recv->basic1;
  p_wk->meet_year = cp_recv->meet_year;
  p_wk->meet_month = cp_recv->meet_month;
  p_wk->meet_day  = cp_recv->meet_day;
  p_wk->extend6   = cp_recv->extend6;
  p_wk->extend7   = cp_recv->extend7;


  p_wk->type      = type;
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;

  p_wk->rnd_seed = p_wk->rnd_seed;
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人データからデータを設定
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 *	@param	RESORT_RROXY_DATA *cp_proxy 受信データ
 *	@param	peopleID  人物ID
 *	@param	placeID 場所
 *	@param  type      RESORT_SECRETARY_TYPE_xxx
 */
//-----------------------------------------------------------------------------
void RESORT_SECRETARY_DATA_ConvertProxy( RESORT_SECRETARY_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy, u16 peopleID, u16 placeID, u16 type )
{
  p_wk->basic1    = cp_proxy->basic1;

  p_wk->meet_year = cp_proxy->meet_year;
  p_wk->meet_month = cp_proxy->meet_month;
  p_wk->meet_day  = cp_proxy->meet_day;

  p_wk->extend6   = cp_proxy->extend6;
  p_wk->extend7   = cp_proxy->extend7;

  p_wk->type      = type;
  p_wk->peopleID  = peopleID;
  p_wk->placeID   = placeID;


  p_wk->rnd_seed = p_wk->rnd_seed;
}
//----------------------------------------------------------------------------
/**
 *	@brief  秘書の乱数を取得
 *
 *	@param	const RESORT_SECRETARY_DATA *cp_wk  ワーク
 *	@param	id  ID
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_SECRETARY_DATA_GetRnd( const RESORT_SECRETARY_DATA *cp_wk, RESORT_RND_ID id, u32 range )
{
  u32 seed = RESORT_SECRETARY_DATA_GetData( cp_wk, RESORT_DATA_PARAM_RND_SEED, NULL );
  return RESORT_DATA_GetRnd( seed, id, range );
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
  RESORT_SECRETARY_DATA   secretary[RESORT_SECRETARY_DATA_NUM];
  RESORT_RECV_DATA        owner;
  BOOL                    merge_lock;
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
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA_InitInstance(&p_wk->secretary[i]);
  }

  RESORT_RESIDENT_DATA_InitInstance(&p_wk->owner );
  {
    RESORT_RESIDENT_DATA_SetData( &p_wk->owner, RESORT_DATA_PARAM_WANT_JOB, RESORT_SHOP_ID_NULL );
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
 *	@brief  ビーコンを他のデータにマージ
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *	@param	RESORT_BEACON_DATA *cp_beacon   ビーコンデータ
 *
 *	@return TRUEいれた　FALSEいれなかった
 *
 */
//-----------------------------------------------------------------------------
RESORT_MERGE_RESULT RESORT_PEOPLE_ALL_DATA_MergeData( RESORT_PEOPLE_ALL_DATA *p_wk, const void *cp_data_adrs, RESORT_MERGE_TYPE type )
{
  typedef BOOL (*IS_SAME_OWNER)( const void *, const void * );
  typedef RESORT_MERGE_RESULT (*MERGE_DATA)( void *, const void * );

  static const struct
  {
    IS_SAME_OWNER Resident_IsSameOwner;
    MERGE_DATA    Resident_MergeData;
    IS_SAME_OWNER Secretary_IsSameOwner;
    MERGE_DATA    Secretary_MergeData;
  } sc_data_function_tbl[] =
  {
    //RESORT_MERGE_TYPE_BEACON
    {
      (IS_SAME_OWNER)RESORT_RECV_DATA_IsSameOwnerBeacon,
      (MERGE_DATA)RESORT_RECV_DATA_MergeBeacon,
      (IS_SAME_OWNER)RESORT_SECRETARY_DATA_IsSameOwnerBeacon,
      (MERGE_DATA)RESORT_SECRETARY_DATA_MergeBeacon,
    },
    //RESORT_MERGE_TYPE_RECV,
    {
      (IS_SAME_OWNER)RESORT_RECV_DATA_IsSameOwnerRecv,
      (MERGE_DATA)RESORT_RECV_DATA_MergeRecv,
      (IS_SAME_OWNER)RESORT_SECRETARY_DATA_IsSameOwnerRecv,
      (MERGE_DATA)RESORT_SECRETARY_DATA_MergeRecv,
    },
    //RESORT_MERGE_TYPE_PROXY,
    {
      (IS_SAME_OWNER)RESORT_RECV_DATA_IsSameOwnerProxy,
      (MERGE_DATA)RESORT_RECV_DATA_MergeProxy,
      (IS_SAME_OWNER)RESORT_SECRETARY_DATA_IsSameOwnerProxy,
      (MERGE_DATA)RESORT_SECRETARY_DATA_MergeRecv,
    },
  };

  int i;

  //住人マージ
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_wk, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
    {
      if( sc_data_function_tbl[type].Resident_IsSameOwner(p_resident,cp_data_adrs) )
      {
        if( p_wk->merge_lock )
        {
          return RESORT_MERGE_RESULT_DISABLE;
        }
        else
        {
          RESORT_MERGE_RESULT ret = sc_data_function_tbl[type].Resident_MergeData(p_resident,cp_data_adrs);
          if( ret == RESORT_MERGE_RESULT_OK || ret == RESORT_MERGE_RESULT_DISABLE )
          {
            return ret;
          }
        }
      }
    }
  }

  //秘書マージ
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_secret = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_wk, i );
    if( !RESORT_SECRETARY_DATA_IsEmpty(p_secret) 
      &&  sc_data_function_tbl[type].Secretary_IsSameOwner(p_secret,cp_data_adrs) )
    {
      if( p_wk->merge_lock )
      {
        return RESORT_MERGE_RESULT_DISABLE;
      }
      else
      {
        RESORT_MERGE_RESULT ret = sc_data_function_tbl[type].Secretary_MergeData(p_secret,cp_data_adrs);
        if( ret == RESORT_MERGE_RESULT_OK || ret == RESORT_MERGE_RESULT_DISABLE )
        {
          return ret;
        }
      }
    }
  }

  return RESORT_MERGE_RESULT_NG;
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
      RESORT_RESIDENT_DATA_SetData(p_resident,RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG, FALSE );
      RESORT_RESIDENT_DATA_SetData(p_resident,RESORT_DATA_PARAM_RND_SEED, GFUser_GetPublicRand(GFL_STD_RAND_MAX) );

      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_BIT, 0 );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  マージロックフラグ設定
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk  ワーク
 *	@param	is_lock TRUEロック  FALSEマージする
 */
//-----------------------------------------------------------------------------
void RESORT_PEOPLE_ALL_DATA_SetMergeLock( RESORT_PEOPLE_ALL_DATA *p_wk, BOOL is_lock )
{
  p_wk->merge_lock = is_lock;
}

//----------------------------------------------------------------------------
/**
 *	@brief  主人公データ
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_wk ワーク
 *
 *	@return　主人公用データを取得
 */
//-----------------------------------------------------------------------------
RESORT_RECV_DATA * RESORT_PEOPLE_ALL_DATA_GetOwnerData( RESORT_PEOPLE_ALL_DATA *p_wk )
{
  return &p_wk->owner;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーのお気に入り店を取得
 *
 *	@param	const RESORT_RECV_DATA *cp_wk ワーク
 *
 *	@return お気に入り店
 */
//-----------------------------------------------------------------------------
u32 RESORT_OWNER_DATA_GetWantJob( const RESORT_RECV_DATA *cp_wk, const MYSTATUS *cp_mystatus )
{
  u16 want_job = RESORT_RECV_DATA_GetData( cp_wk, RESORT_DATA_PARAM_WANT_JOB, NULL );
  u16 ver = MyStatus_GetRomCode( cp_mystatus );
  u16 id = MyStatus_GetID( cp_mystatus );


  return  RESORT_DATA_GetDefaultWantJob( want_job, id, ver );
}


//----------------------------------------------------------------------------
/**
 *	@brief  なりたいお店IDを修正
 *
 *	@param	u16 want_job  なりたいお店ID（want_jobがRESORT_SHOP_ID_NULLのときのみチェック）
 *	@param	id    トレーナーID
 *	@param	ver   バージョン
 *
 *	@return 修正後のなりたいお店IDを情報
 */
//-----------------------------------------------------------------------------
u32 RESORT_DATA_GetDefaultWantJob( u16 want_job, u16 id, u16 ver )
{
  if( want_job == RESORT_SHOP_ID_NULL )
  {
    u32 shop_type = sc_resort_institution_release_tbl[id%RESORT_INSTITUTION_TBL_MAX][0];

    switch( shop_type )
    {
    case RESORT_RESIDENT_TYPE_NURSERY:
      // 初期お店としてこないはず
      GF_ASSERT(0);
    default:
      GF_ASSERT(0);
    case RESORT_RESIDENT_TYPE_LOT:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_LOT_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_LOT_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_LOT_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_LOT_00_W2;
      }
    case RESORT_RESIDENT_TYPE_EXERCICE:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_EXERCICE_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_EXERCICE_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_EXERCICE_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_EXERCICE_00_W2;
      }
    case RESORT_RESIDENT_TYPE_MARKET:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_MARKET_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_MARKET_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_MARKET_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_MARKET_00_W2;
      }
    case RESORT_RESIDENT_TYPE_FLORIST:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_FLORIST_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_FLORIST_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_FLORIST_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_FLORIST_00_W2;
      }
    case RESORT_RESIDENT_TYPE_ANTIQUE:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_ANTIQUE_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_ANTIQUE_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_ANTIQUE_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_ANTIQUE_00_W2;
      }
    case RESORT_RESIDENT_TYPE_PARLOR:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_PARLOR_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_PARLOR_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_PARLOR_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_PARLOR_00_W2;
      }
    case RESORT_RESIDENT_TYPE_CAFE:
      switch( ver )
      {
      case VERSION_BLACK:
        return RESORT_SHOP_ID_CAFE_00_B;
      case VERSION_WHITE:
        return RESORT_SHOP_ID_CAFE_00_W;
      case VERSION_B2:
        return RESORT_SHOP_ID_CAFE_00_B2;
      default:
        GF_ASSERT(0);
      case VERSION_W2:
        return RESORT_SHOP_ID_CAFE_00_W2;
      }
    }
  }
    
  return want_job;
}

//----------------------------------------------------------------------------
/**
 *	@brief  リゾート用レコードID  を追加
 *
 *	@param	u32 record_id 
 */
//-----------------------------------------------------------------------------
void RESORT_RECORD_AddID( u32 record_id )
{
  //#define DEBUG_RECORD_ADDID

  enum
  {
    RESORT_DATA_RECORD_CNT  = RESORT_DATA_PARAM_EXT5_RECORD_03 - RESORT_DATA_PARAM_EXT5_RECORD_00 + 1,
  };

  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( sp_people_all_data );

  int i , j;
  u32 record_temp;
  RESORT_DATE temp_date;
  RTCDate date;
  RESORT_DATE resort_date;

  GF_ASSERT_MSG( record_id < RESORT_RECORD_ID_MAX, "%d<MAX", record_id );

  GFL_RTC_GetDate( &date );

  resort_date = RESORT_DATA_RtcToDate( &date );

#ifdef PM_DEBUG
#ifdef DEBUG_RECORD_ADDID
  {
    static const int test[] =
    {
      0,0,0,1,
    };
    for( i = 0; i < RESORT_DATA_RECORD_CNT; i++ )
    {
      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+i, test[i] );
      if( test[i] != 0 )
      {
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+i, resort_date );
      }
      else
      {
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+i, 0 );
      }
      NAGI_Printf( "b record %d\n", test[i] );
    }
  }
#endif
#endif


  //重複チェック
  for( i = 0; i < RESORT_DATA_RECORD_CNT; i++ )
  {
    u32 record = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+i, NULL );
    if( record_id == record )
    {
      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+i, resort_date );
      goto RESORT_RECORD_ADD_ID_SORT;
    }
  }
  //空きチェック
  for( i = 0; i < RESORT_DATA_RECORD_CNT; i++ )
  {
    u32 record = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+i, NULL );
    if( RESORT_RECORD_ID_NONE == record )
    {
      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+i, record_id );
      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+i, resort_date );
      goto RESORT_RECORD_ADD_ID_SORT;
    }
  }
  //押し出し
  for( i = 0; i < RESORT_DATA_RECORD_CNT; i++ )
  {
    u32 next_id = i + 1;
    if( next_id < RESORT_DATA_RECORD_CNT )
    {
      u32 next_record = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+next_id, NULL );
      RESORT_DATE next_date = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+next_id, NULL );

      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+i, next_record );
      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+i, next_date );
    }
  }
  //後尾にいれる
  RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+RESORT_DATA_RECORD_CNT-1, record_id );
  RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+RESORT_DATA_RECORD_CNT-1, resort_date );

RESORT_RECORD_ADD_ID_SORT:

#ifdef PM_DEBUG
#ifdef DEBUG_RECORD_ADDID
      {
        int k;
        for( k = 0; k < RESORT_DATA_RECORD_CNT; k++ )
        {
          u16 test = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+k, NULL );
          NAGI_Printf( "[%d]", test );
        }
        NAGI_Printf( "\n" );
      }
#endif
#endif


  //ソート
  for( i = 0; i < RESORT_DATA_RECORD_CNT-1; i++ )
  {
    for( j = RESORT_DATA_RECORD_CNT-1; j > i; j-- )
    {
      s16 idx1 = j-1;
      s16 idx2 = j;
      u32 record1 = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+idx1, NULL );
      RESORT_DATE r_date1 = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx1, NULL );
      RTCDate date1;
      s32 days1;
      u32 record2 = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+idx2, NULL );
      RESORT_DATE r_date2 = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx2, NULL );
      RTCDate date2;
      s32 days2;

      RESORT_DATA_DateToRtc( r_date1, &date1 );
      RESORT_DATA_DateToRtc( r_date2, &date2 );

      days1 = RTC_ConvertDateToDay( &date1 );
      days2 = RTC_ConvertDateToDay( &date2 );

      days1 = (record1 == RESORT_RECORD_ID_NONE)? 0: days1;
      days2 = (record2 == RESORT_RECORD_ID_NONE)? 0: days2;

      if( days1 < days2 )
      {
        //1
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+idx1, record2 );
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx1, r_date2 );
        //2
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+idx2, record1 );
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx2, r_date1 );
      }
    }
#ifdef PM_DEBUG
#ifdef DEBUG_RECORD_ADDID
      {
        int k;
        for( k = 0; k < RESORT_DATA_RECORD_CNT; k++ )
        {
          u16 test = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_00+k, NULL );
          RESORT_DATE r_date = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+k, NULL );
          RTCDate date;
          s32 days;
          RESORT_DATA_DateToRtc( r_date, &date );
          days = RTC_ConvertDateToDay( &date );
          NAGI_Printf( "[%d:%dy%dm%dd%d]", test, days, date.year, date.month, date.day );
        }
        NAGI_Printf( "\n" );
      }
#endif
#endif
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
#define RESORT_MY_NAME_BUFF_MAX  (20)

//-------------------------------------
///	街データ
//=====================================
struct _RESORT_CITY_DATA
{
  STRCODE city_name[RESORT_CITY_NAME_BUFF_MAX+EOM_SIZE]; //42
  STRCODE my_name[RESORT_MY_NAME_BUFF_MAX+EOM_SIZE];  //42 84
  u32     trainerID[RESORT_TRAINER_ID_STORE_SIZE];  //128 212
  u32     exp;              //4 216
  u16     lv; 
  u16     tex;              //4 220
  u32     fun_day_flag  :1;
  u32     is_tutorial   :1;
  u32     prize_sec03   :1;
  u32     prize_sec04   :1;
  u32     prize_neon    :1;
  u32     prize_scout   :1;
  u32     prize_cityname :1;
  u32     ev_prize_sec03   :1;
  u32     ev_prize_sec04   :1;
  u32     ev_prize_neon    :1;
  u32     ev_prize_scout   :1;
  u32     ev_prize_cityname :1;
  u32     celeb_day_flag    :1;
  u32     celeb_trade_flag  :1;
  u32     prize_dendou  :1;
  u32     ev_prize_dendou :1;
  u32     prize_parade  :1;
  u32     ev_prize_parade: 1;
  u32     prize_item  :1;
  u32     ev_prize_item: 1;
  u32     dummy         :12;    //4 224

  u16     trainerID_max;
  u16     trainerID_cnt;        //4 228
  u32     rnd_seed;             //4 232

  u16     fever_cnt;
  u16     is_fever;
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
  p_wk->rnd_seed    = GFUser_GetPublicRand(0);
  p_wk->lv          = 1;
  p_wk->is_tutorial = 1;

  {
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, GFL_HEAP_LOWID(HEAPID_SAVE) );
    STRBUF *p_strbuf = GFL_MSG_CreateString( p_msg, MAPNAME_C16 );
    GFL_STR_GetStringCode( p_strbuf, p_wk->city_name, RESORT_CITY_NAME_BUFF_MAX+EOM_SIZE );
    GFL_STR_DeleteBuffer( p_strbuf );
    GFL_MSG_Delete( p_msg );

    p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(HEAPID_SAVE) );
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
    if( p_ret_adrs != NULL )
    {
      STRTOOL_Copy( cp_wk->city_name, p_ret_adrs, RESORT_CITY_NAME_LEN+EOM_SIZE );
    }
    return (u32)cp_wk->city_name;
  case RESORT_CITY_DATA_PARAM_AUTHER_NAME: //p_ret_adrs
    if( p_ret_adrs != NULL )
    {
      STRTOOL_Copy( cp_wk->my_name, p_ret_adrs, RESORT_MY_NAME_LEN+EOM_SIZE );
    }
    return (u32)cp_wk->my_name;
  case RESORT_CITY_DATA_PARAM_LV:
    return cp_wk->lv;
  case RESORT_CITY_DATA_PARAM_EXP:
    return cp_wk->exp;
  case RESORT_CITY_DATA_PARAM_RND_SEED:
    return cp_wk->rnd_seed;
  case RESORT_CITY_DATA_PARAM_TEX:
    return cp_wk->tex;
  case RESORT_CITY_DATA_FUN_DAY_FLAG:
    return cp_wk->fun_day_flag;
  case RESORT_CITY_DATA_PARAM_IS_TUTORIAL:
    return cp_wk->is_tutorial;
  case RESORT_CITY_DATA_PARAM_PRIZE_SEC03:
    return cp_wk->prize_sec03;
  case RESORT_CITY_DATA_PARAM_PRIZE_SEC04:
    return cp_wk->prize_sec04;
  case RESORT_CITY_DATA_PARAM_PRIZE_CITYNAME:
    return cp_wk->prize_cityname;
  case RESORT_CITY_DATA_PARAM_PRIZE_NEON:
    return cp_wk->prize_neon;
  case RESORT_CITY_DATA_PARAM_PRIZE_SCOUT:
    return cp_wk->prize_scout;
  case RESORT_CITY_DATA_PARAM_PRIZE_DENDOU:
    return cp_wk->prize_dendou;
  case RESORT_CITY_DATA_PARAM_PRIZE_PARADE:
    return cp_wk->prize_parade;
  case RESORT_CITY_DATA_PARAM_PRIZE_ITEM:
    return cp_wk->prize_item;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03:
    return cp_wk->ev_prize_sec03;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC04:
    return cp_wk->ev_prize_sec04;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_CITYNAME:
    return cp_wk->ev_prize_cityname;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_NEON:
    return cp_wk->ev_prize_neon;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_SCOUT:
    return cp_wk->ev_prize_scout;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_DENDOU:
    return cp_wk->ev_prize_dendou;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_PARADE:
    return cp_wk->ev_prize_parade;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_ITEM:
    return cp_wk->ev_prize_item;
  case RESORT_CITY_DATA_CELEB_DAY_FLAG:
    return cp_wk->celeb_day_flag;
  case RESORT_CITY_DATA_CELEB_TRADE_FLAG:
    return cp_wk->celeb_trade_flag;
  case RESORT_CITY_DATA_FEVER_CNT:
    return cp_wk->fever_cnt;
  case RESORT_CITY_DATA_FEVER_FLAG:
    return cp_wk->is_fever;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ設定
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
    STRTOOL_Copy( (const STRCODE*)data, p_wk->city_name, RESORT_CITY_NAME_LEN+EOM_SIZE );
    break;
  case RESORT_CITY_DATA_PARAM_AUTHER_NAME: //p_ret_adrs
    STRTOOL_Copy( (const STRCODE*)data, p_wk->my_name, RESORT_MY_NAME_LEN+EOM_SIZE );
    break;
  case RESORT_CITY_DATA_PARAM_LV:
    p_wk->lv = data;
    break;
  case RESORT_CITY_DATA_PARAM_EXP:
    p_wk->exp = data;
    break;
  case RESORT_CITY_DATA_PARAM_TEX:
    p_wk->tex = data;
    break;
  case RESORT_CITY_DATA_FUN_DAY_FLAG:
    p_wk->fun_day_flag  = data;
    break;
  case RESORT_CITY_DATA_PARAM_IS_TUTORIAL:
    p_wk->is_tutorial = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_SEC03:
    p_wk->prize_sec03 = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_SEC04:
    p_wk->prize_sec04 = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_CITYNAME:
    p_wk->prize_cityname  = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_NEON:
    p_wk->prize_neon  = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_SCOUT:
    p_wk->prize_scout = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_DENDOU:
    p_wk->prize_dendou  = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_PARADE:
    p_wk->prize_parade  = data;
    break;
  case RESORT_CITY_DATA_PARAM_PRIZE_ITEM:
    p_wk->prize_item  = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03:
    p_wk->ev_prize_sec03 = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC04:
    p_wk->ev_prize_sec04 = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_CITYNAME:
    p_wk->ev_prize_cityname  = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_NEON:
    p_wk->ev_prize_neon  = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_SCOUT:
    p_wk->ev_prize_scout = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_DENDOU:
    p_wk->ev_prize_dendou = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_PARADE:
    p_wk->ev_prize_parade = data;
    break;
  case RESORT_CITY_DATA_PARAM_EV_PRIZE_ITEM:
    p_wk->ev_prize_item = data;
    break;
  case RESORT_CITY_DATA_CELEB_DAY_FLAG:
    p_wk->celeb_day_flag = data;
    break;
  case RESORT_CITY_DATA_CELEB_TRADE_FLAG:
    p_wk->celeb_trade_flag = data;
    break;
  case RESORT_CITY_DATA_FEVER_CNT:
    p_wk->fever_cnt = data;
    break;
  case RESORT_CITY_DATA_FEVER_FLAG:
    p_wk->is_fever  = data;
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
  //ビーコントレーナーセーブを破棄
  GFL_STD_MemClear( p_wk->trainerID, sizeof(u32)*RESORT_TRAINER_ID_STORE_SIZE );
  p_wk->trainerID_cnt = 0;
  p_wk->trainerID_max = 0;

  //乱数再抽選
  p_wk->rnd_seed      = GFUser_GetPublicRand(GFL_STD_RAND_MAX);

  //ファンを再度出現させる
  p_wk->fun_day_flag  = FALSE;

  //セレブ抽選フラグ
  p_wk->celeb_day_flag = FALSE;

  //もしフェーバーフラグがたっていたら、
  if( p_wk->is_fever )
  {
    p_wk->fever_cnt++;
    if( p_wk->fever_cnt >= RESORT_FEVER_DAY )
    {
      p_wk->is_fever = FALSE;
      p_wk->fever_cnt = 0;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーID追加処理
 *
 *	@param	RESORT_CITY_DATA *p_wk  ワーク
 *	@param	trainerID               トレーナーID
 */
//-----------------------------------------------------------------------------
void RESORT_CITY_DATA_AddTrainerID( RESORT_CITY_DATA *p_wk, u32 trainerID )
{
  p_wk->trainerID[ p_wk->trainerID_cnt ]  = trainerID;
  p_wk->trainerID_cnt++;
  p_wk->trainerID_cnt %= RESORT_TRAINER_ID_STORE_SIZE;
  p_wk->trainerID_max++;
  p_wk->trainerID_max = MATH_IMin( p_wk->trainerID_max, RESORT_TRAINER_ID_STORE_SIZE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーIDがあるかどうか
 *
 *	@param	const RESORT_CITY_DATA *cp_wk ワーク
 *	@param	trainerID   トレーナーID
 *
 *	@return TRUEで存在  FALSEでなし
 */
//-----------------------------------------------------------------------------
BOOL RESORT_CITY_DATA_CheckTrainerID( const RESORT_CITY_DATA *cp_wk, u32 trainerID )
{
  int i;
  for( i = 0; i < cp_wk->trainerID_max; i++ )
  {
    if( cp_wk->trainerID[ i ] == trainerID )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  街データでの抽選
 *
 *	@param	const RESORT_CITY_DATA *cp_wk ワーク
 *	@param	id  乱数使用ID
 *	@param	range   範囲
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_CITY_DATA_GetRnd( const RESORT_CITY_DATA *cp_wk, RESORT_RND_ID id, u32 range )
{
  u32 seed = RESORT_CITY_DATA_GetData( cp_wk, RESORT_CITY_DATA_PARAM_RND_SEED, NULL );
  return RESORT_DATA_GetRnd( seed, id, range );
}

//=============================================================================
/**
 *  データ初期化
 */
//=============================================================================
void RESORT_DATA_Boot( const SAVE_CONTROL_WORK* ctrl )
{
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( (SAVE_CONTROL_WORK*)ctrl );
  sp_city_data = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
  sp_people_all_data  = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );

#ifdef PM_DEBUG
  RESORT_DEBUG_FLAGWORK_Init();
#endif
}



//----------------------------------------------------------------------------
/**
 *	@brief  各人物の固有乱数を取得
 *
 *	@param	u32 seed  シード
 *	@param	id        乱数使用ID
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_DATA_GetRnd( u32 seed, RESORT_RND_ID id, u32 range )
{
  GFL_STD_RandContext context;
  GFL_STD_RandInit( &context, seed );

  {
    int i;
    u32 rnd = 0;
    for( i = 0; i <= id; i++ )
    {
      rnd = GFL_STD_Rand( &context, range );
    }

#ifdef PM_DEBUG
    {
      u32 ret = RESORT_DEBUG_WORK_Get(RESORT_DEBUG_WORK_FORCE_RND);
      if( ret != RESORT_DEBUG_WORK_DEFAULT )
      {
        return MATH_CLAMP( ret, 0, range-1 );
      }
    }
#endif

    return rnd;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  なりたいお店IDを修正
 *
 *	@param	u16 want_job  なりたいお店IDを
 *	@param	id  トレーナーID
 *	@param	ver バージョン
 *	@param	RESORT_CITY_DATA *cp_data   街情報
 *	@param	RESORT_BINARY_SHOP *cp_shop   お店情報
 *
 *	@return 修正後のお店ID
 */
//-----------------------------------------------------------------------------
u16 RESORT_DATA_GetWantJobModify( u16 want_job, u16 id, u16 ver, BOOL is_special, const RESORT_CITY_DATA *cp_data, const RESORT_BINARY_SHOP *cp_shop )
{
  BOOL is_dendou = RESORT_CITY_DATA_GetData( cp_data,RESORT_CITY_DATA_PARAM_PRIZE_DENDOU,NULL );
  const RESORT_BINARY_SHOP_DATA* cp_shop_data = RESORT_BINARY_SHOP_GetData(cp_shop, want_job );
  u16 lv = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_LV );
  u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

  if( (want_job == RESORT_SHOP_ID_NULL) || (!is_dendou && shop_type == RESORT_RESIDENT_TYPE_NURSERY ) || ((lv!=0) && !is_special ) )
  {
    return RESORT_DATA_GetDefaultWantJob( want_job, id, ver );
  }
    
  return want_job;
}

//=============================================================================
/**
 *　  デバッグ
*/
//=============================================================================
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief デバッグ用 外部からのアクセスを変更
 *
 *	@param	BOOL enable   TRUE　すべてアクセス許可する  FALSE　通常通りのアクセス
 */
//-----------------------------------------------------------------------------
void DEBUG_RESORT_DATA_SetPermission( BOOL enable )
{
  s_debug_permission = enable;
}
#endif //PM_DEBUG
