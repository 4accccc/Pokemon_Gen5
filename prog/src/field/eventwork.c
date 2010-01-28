//======================================================================
/**
 * @file	eventwork.c
 * @brief	セーブフラグ、ワーク操作
 * @author	Tamada
 * @author	Satoshi Nohara
 * @date	05.10.22
 */
//======================================================================
#include "eventwork.h"
#include "eventwork_def.h"

static u8 * EVENTWORK_GetEventFlagAdrs( EVENTWORK * ev, u16 flag_no );
//======================================================================
//	定義
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	イベントワーク構造体の定義
 */
//------------------------------------------------------------------
struct _EVENTWORK {
	u16 work[EVENT_WORK_AREA_MAX];	//ワーク
	u8	flag[EVENT_FLAG_AREA_MAX];	//フラグ
  u8  time_request;
};

//------------------------------------------------------------------
/**
 * @brief セーブしないフラグ
 */
//------------------------------------------------------------------
static u8 EventCtrlFlag[CTRLFLAG_AREA_MAX] = {};

//システムフラグ範囲内チェック
#define CHECK_SYS_FLAG_RANGE(flag) \
  GF_ASSERT_MSG(flag>=SYS_FLAG_AREA_START && flag<=SYS_FLAG_AREA_END,"ERROR SYS FLAG OUTRANGE")

//======================================================================
//  proto
//======================================================================

//======================================================================
//	EVENTWORK
//======================================================================
//------------------------------------------------------------------
//
//------------------------------------------------------------------
int EVENTWORK_GetWorkSize(void)
{
	return sizeof(EVENTWORK);
}

void EVENTWORK_InitWork( u8 *work )
{
  GFL_STD_MemClear( work, sizeof(EVENTWORK) );
}

//======================================================================
//
//    フラグ操作関連
//
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	イベントフラグをチェックする
 *
 * @param	ev			イベントワークへのポインタ
 * @param	flag_no		フラグナンバー
 *
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
BOOL EVENTWORK_CheckEventFlag( EVENTWORK * ev, u16 flag_no)
{
	u8 * p = EVENTWORK_GetEventFlagAdrs( ev, flag_no );
	if( p != NULL ){
		if( *p & ( 1 << (flag_no % 8) ) ){ return 1; }
	}
	return 0;
}


//------------------------------------------------------------------
/**
 * @brief	イベントフラグをセットする
 *
 * @param	ev			イベントワークへのポインタ
 * @param	flag_no		フラグナンバー
 *
 * @return	none
 */
//------------------------------------------------------------------
void EVENTWORK_SetEventFlag( EVENTWORK * ev, u16 flag_no)
{
	u8 * p = EVENTWORK_GetEventFlagAdrs( ev, flag_no );
	if( p == NULL ){ return; }
	*p |= 1 << ( flag_no % 8 );
#if 0
	SVLD_SetCrc(GMDATA_ID_EVENT_WORK);
#endif //CRC_LOADCHECK
	return;
}


//------------------------------------------------------------------
/**
 * @brief	イベントフラグをリセットする
 *
 * @param	ev			イベントワークへのポインタ
 * @param	flag_no		フラグナンバー
 *
 * @return	none
 */
//------------------------------------------------------------------
void EVENTWORK_ResetEventFlag( EVENTWORK * ev, u16 flag_no)
{
	u8 * p = EVENTWORK_GetEventFlagAdrs( ev, flag_no );
	if( p == NULL ){ return; }
	*p &= ( 0xff ^ ( 1 << (flag_no % 8) ) );
#if 0
	SVLD_SetCrc(GMDATA_ID_EVENT_WORK);
#endif //CRC_LOADCHECK
	return;
}

//--------------------------------------------------------------
/**
 * @brief イベントフラグの領域クリア
 * @param	ev			イベントワークへのポインタ
 * @param	flag_no		フラグナンバー
 * 
 * @noto
 * ベタな記述なので処理速度の問題があるかもしれない
 */
//--------------------------------------------------------------
void EVENTWORK_ClearEventFlags( EVENTWORK * ev, u16 start_no, u16 end_no )
{
  int no;

  GF_ASSERT( start_no < end_no );
  GF_ASSERT( ( end_no < SCFLG_START ) || ( SCFLG_START <= start_no ) );

  for ( no = start_no ; no <= end_no; no ++ )
  {
    EVENTWORK_ResetEventFlag( ev, no );
  }
}

//--------------------------------------------------------------
/**
 * @brief	イベントフラグのアドレスを取得する
 *
 * @param	flag_no		フラグナンバー
 *
 * @retval	"NULL != フラグアドレス"
 * @retval	"NULL = 未定義"
 */
//--------------------------------------------------------------
static u8 * EVENTWORK_GetEventFlagAdrs( EVENTWORK * ev, u16 flag_no )
{
	if( flag_no == 0 ){
		return NULL;
	}else if( flag_no < SCFLG_START ){
		if( (flag_no / 8) >= EVENT_FLAG_AREA_MAX ){
			GF_ASSERT_MSG(0, "フラグナンバー(0x%04x)が最大数を超えています！\n", flag_no );
      return NULL;
		}
		return &ev->flag[flag_no/8];
	}else{

		if( ((flag_no - SCFLG_START) / 8) >= CTRLFLAG_AREA_MAX ){
			GF_ASSERT_MSG(0, "フラグナンバー(0x%04x)が最大数を超えています！\n", flag_no );
      return NULL;
		}
		return &( EventCtrlFlag[ (flag_no - SCFLG_START) / 8 ] );
	}
}

//======================================================================
//
//
//    ワーク操作関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief	イベントワークアドレスを取得
 *
 * @param	ev			イベントワークへのポインタ
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークのアドレス"
 *
 * @li	work_no < 0x8000	通常のセーブワーク
 * @li	work_no >= 0x8000	スクリプト制御ワークの中に確保しているワーク
 */
//--------------------------------------------------------------
u16 * EVENTWORK_GetEventWorkAdrs( EVENTWORK * ev, u16 work_no )
{
	if( (work_no - SVWK_START) >= EVENT_WORK_AREA_MAX ){
		GF_ASSERT_MSG( 0, "ワークナンバー(0x%04x)が最大数を超えています！\n", work_no );
	}
	
	return &ev->work[ work_no - SVWK_START ];
}

//--------------------------------------------------------------
/**
 * @brief
 * @param	ev			イベントワークへのポインタ
 * @param	work_no		ワークナンバー
 */
//--------------------------------------------------------------
void EVENTWORK_ClearEventWorks( EVENTWORK * ev, u16 start_no, u16 end_no )
{
  int no;
  GF_ASSERT( start_no < end_no );
  GF_ASSERT( SVWK_START <= start_no );
  GF_ASSERT( end_no < SVWK_START + EVENT_WORK_AREA_MAX );

  GFL_STD_MemClear(
      EVENTWORK_GetEventWorkAdrs( ev, start_no ),
      sizeof(u16) * (end_no - start_no) );
}



//======================================================================
//
//
//
//======================================================================
//--------------------------------------------------------------
///    時間フラグのクリア
//--------------------------------------------------------------
void EVENTWORK_ClearTimeFlags( EVENTWORK * ev )
{
  EVENTWORK_ClearEventFlags( ev, TMFLG_AREA_START, TMFLG_AREA_END );
  ev->time_request = TRUE;
}
//--------------------------------------------------------------
///    時間更新処理リクエストの取り出し
//    ※現状、隠しアイテム復活用
//--------------------------------------------------------------
BOOL EVENTWORK_PopTimeRequest( EVENTWORK * ev )
{
  BOOL result = ev->time_request;
  ev->time_request = FALSE;
  return result;
}

