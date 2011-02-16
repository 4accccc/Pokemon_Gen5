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
  u16 trainerID;                    ///<トレーナーID(下位16ビット)  2 16
  u8  nation;                       ///<国コード      1 17
  u8  area;                         ///<地域コード    1 18
  u8  version;                      ///<バージョン    1 19
  u8  language;                     ///<言語コード    1 20
  u8  trainer_view;                 ///<見た目        1 21
  u8  sex;                          ///<性別          1 22
  QUESTIONNAIRE_ANSWER_WORK qanswer;  ///<アンケート  10  32
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

  //アンケート
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork((GAMEDATA*)cp_gamedata);
    QUESTIONNAIRE_SAVE_WORK *p_ques_sv = SaveData_GetQuestionnaire(p_sv);

    GFL_STD_MemCopy( Questionnaire_GetAnswerWork(p_ques_sv), &p_wk->qanswer, sizeof(QUESTIONNAIRE_ANSWER_WORK) );
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
  int dummy;  //TODO
              //WBから受け取ったときは中身がからなので何らかの認識をすること
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
 *	@brief  基本データ2にビーコンからのデータを設定
 *
 *	@param	RESORT_BASIC2_DATA *p_wk  ワーク
 *	@param	GAMEBEACON_INFO   ビーコン
 */
//-----------------------------------------------------------------------------
static void RESORT_BASIC2_DATA_SetBeaconInfo( RESORT_BASIC2_DATA *p_wk, const GAMEBEACON_INFO *cp_beacon_info )
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
  u8  dummy[20];
} RESORT_EXTEND1_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND1_DATA) == RESORT_EXTEND_DATA_SIZE );

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
  {
    int i;
    for( i = 0; i < 20; i++ )
    {
      p_wk->dummy[i]  = i;
    }
  }
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
  u8  dummy[20];
} RESORT_EXTEND2_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND2_DATA) == RESORT_EXTEND_DATA_SIZE );

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
typedef struct
{
  u8  dummy[20];
} RESORT_EXTEND3_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND3_DATA) == RESORT_EXTEND_DATA_SIZE );
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
  u8  dummy[20];
} RESORT_EXTEND4_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND4_DATA) == RESORT_EXTEND_DATA_SIZE );
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
  u8  dummy[20];
} RESORT_EXTEND5_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND5_DATA) == RESORT_EXTEND_DATA_SIZE );
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
typedef struct
{
  u8  dummy[20];
} RESORT_EXTEND6_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND6_DATA) == RESORT_EXTEND_DATA_SIZE );
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
typedef struct
{
  u8  dummy[20];
} RESORT_EXTEND7_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND7_DATA) == RESORT_EXTEND_DATA_SIZE );
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
  u8  dummy[20];
} RESORT_EXTEND8_DATA;
SDK_COMPILER_ASSERT( sizeof(RESORT_EXTEND8_DATA) == RESORT_EXTEND_DATA_SIZE );

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
  u32                 extend_flag;
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
  u32 dummy;
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
 */
//-----------------------------------------------------------------------------
void RESORT_RECV_DATA_MergeBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon )
{
  //不正対策
  if( !RESORT_BEACON_DATA_IsValid( cp_beacon ) )
  {
    return;
  }

  if( RESORT_RECV_DATA_IsEmpty( p_wk ) )
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
    }

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
void RESORT_RECV_DATA_MergeRecv( RESORT_RECV_DATA *p_wk, const RESORT_RECV_DATA *cp_recv )
{
  //不正対策
  if( !RESORT_RECV_DATA_IsValid( cp_recv ) )
  {
    return;
  }

  if( RESORT_RECV_DATA_IsEmpty( p_wk ) )
  {
    GFL_STD_MemCopy( cp_recv, p_wk, sizeof(RESORT_RECV_DATA) );
  }
  else
  {
    //TODO
    //マージ処理
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
    RESORT_BASIC2_DATA_SetBeaconInfo( &p_wk->basic2, cp_beacon_info );
    GFL_STD_MemCopy( cp_beacon_info->action.extend_buff, p_wk->extend, RESORT_EXTEND_DATA_SIZE );
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
void RESORT_PROXY_DATA_InitInstance( RESORT_PROXY_DATA *p_wk )
{
}

BOOL RESORT_PROXY_DATA_IsEmpty( const RESORT_PROXY_DATA *cp_wk )
{
  return TRUE;
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
 *	@brief  レゾナンスリゾートのデータ受信バッファ  データ取得
 *
 *	@param	RESORT_PROXY_BUFF *p_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return データ取得
 */
//-----------------------------------------------------------------------------
RESORT_PROXY_DATA * RESORT_PROXY_BUFF_GetData( RESORT_PROXY_BUFF *p_wk, u32 index )
{
  GF_ASSERT( index < p_wk->max );
  return &p_wk->data[ index ];
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
//-------------------------------------
///	住民データ
//=====================================
struct _RESORT_RESIDENT_DATA
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
  u32                 extend_flag;
  //TODO 
};

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  住民データを作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return　住民データ
 */
//-----------------------------------------------------------------------------
RESORT_RESIDENT_DATA * RESORT_RESIDENT_DATA_Init( HEAPID heapID )
{
  RESORT_RESIDENT_DATA *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_RESIDENT_DATA) );
  RESORT_RESIDENT_DATA_InitInstance( p_wk );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  住民データ破棄
 *
 *	@param	RESORT_RESIDENT_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RESIDENT_DATA_Exit( RESORT_RESIDENT_DATA *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  住民データを初期化
 *
 *	@param	RESORT_RESIDENT_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RESIDENT_DATA_InitInstance( RESORT_RESIDENT_DATA *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_RESIDENT_DATA ) );
}

//=============================================================================
/**
 *  特殊NPCデータ
 */
//=============================================================================
//-------------------------------------
///	特殊NPCデータ
//=====================================
struct _RESORT_VISITOR_DATA
{
  int dummy;
};

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
  int dummy;
};

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
  int dummy;
};

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
}


//=============================================================================
/**
 *	リゾート街データ
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
#define RESORT_CITY_DATA_SIZE (1)

//-------------------------------------
///	街データ
//=====================================
struct _RESORT_CITY_DATA
{
  RESORT_RESIDENT_DATA    resident[RESORT_RESIDENT_DATA_NUM];
  RESORT_VISITOR_DATA     visitor[RESORT_VISITOR_DATA_NUM];
  RESORT_SECRETARY_DATA   secretary[RESORT_SECRETARY_DATA_NUM];
  RESORT_INSTITUTION_DATA institution[RESORT_INSTITUTION_DATA_NUM];
};

//SDK_COMPILER_ASSERT(sizeof(RESORT_CITY_DATA) == RESORT_CITY_DATA_SIZE );

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
  for( i = 0; i < RESORT_INSTITUTION_DATA_NUM; i++ )
  {
    RESORT_INSTITUTION_DATA_InitInstance(&p_wk->institution[i] );
  }
}
