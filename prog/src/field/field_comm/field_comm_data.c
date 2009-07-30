//======================================================================
/**
 * @file  field_comm_data.c
 * @brief 通信で共有するデータ(各キャラの座標とか)
 * @author  ariizumi
 * @data  08/11/13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/playerwork.h"
#include "test/ariizumi/ari_debug.h"
#include "app/trainer_card.h"
#include "field_comm_main.h"
#include "field_comm_data.h"
#include "system/main.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================

//======================================================================
//  typedef struct
//======================================================================
//キャラ個人のデータ
typedef struct
{
  BOOL  isExist_; //キャラが存在する
  BOOL  isValid_; //データが有効
  F_COMM_CHARA_STATE  state_;
  F_COMM_TALK_STATE talkState_;
  PLAYER_WORK plWork_;
}FIELD_COMM_CHARA_DATA;

//会話後の行動用のデータ管理
typedef struct
{
  F_COMM_USERDATA_TYPE type_;
  void* selfData_;    //自身のデータ(送信バッファ
  void* partnerData_; //相手のデータ(受信バッファ
}FIELD_COMM_USERDATA;

struct _FIELD_COMM_DATA
{
  HEAPID heapID_;
  u8  backupCommState_; //FieldCommFuncのステートのバックアップ
  FIELD_COMM_CHARA_DATA selfData_;  //自身のデータ。
  FIELD_COMM_CHARA_DATA charaData_[FIELD_COMM_MEMBER_MAX];

  FIELD_COMM_USERDATA userData_;
};

//あえて独立して確保させる
//FIELD_COMM_DATA *commData = NULL;

//======================================================================
//  proto
//======================================================================
FIELD_COMM_DATA * FIELD_COMM_DATA_InitSystem( HEAPID heapID );
void  FIELD_COMM_DATA_TermSystem( FIELD_COMM_DATA *comm_data );

void  FIELD_COMM_DATA_SetFieldCommMode( FIELD_COMM_DATA *comm_data, const u8 state );
const u8 FIELD_COMM_DATA_GetFieldCommMode(FIELD_COMM_DATA *comm_data);

void  FIELD_COMM_DATA_SetSelfData_PlayerWork( const PLAYER_WORK *plWork );
void  FIELD_COMM_DATA_SetSelfDataPos( const ZONEID zoneID , const VecFx32 *pos , const u16 *dir );
PLAYER_WORK* FIELD_COMM_DATA_GetSelfData_PlayerWork(FIELD_COMM_DATA *commData);

static void FIELD_COMM_DATA_InitOneCharaData( FIELD_COMM_CHARA_DATA *charaData );
static FIELD_COMM_CHARA_DATA* FIELD_COMM_DATA_GetCharaDataWork( FIELD_COMM_DATA *commData, const u8 idx );

//ユーザーデータ関連
const u32 FIELD_COMM_DATA_GetUserDataSize( const F_COMM_USERDATA_TYPE type );
const F_COMM_USERDATA_TYPE  FIELD_COMM_DATA_GetUserDataType_From_Action( const F_COMM_ACTION_LIST action );

//--------------------------------------------------------------
//  通信用データ管理初期化
//--------------------------------------------------------------
FIELD_COMM_DATA * FIELD_COMM_DATA_InitSystem( HEAPID heapID )
{
  FIELD_COMM_DATA *comm_data;
  int i;

  //最初はヒープの確保と全初期化
  comm_data = GFL_HEAP_AllocClearMemory( heapID , sizeof(FIELD_COMM_DATA) );
  comm_data->heapID_ = heapID;
  FIELD_COMM_DATA_InitOneCharaData( &comm_data->selfData_ );
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    FIELD_COMM_DATA_InitOneCharaData( &comm_data->charaData_[i] );
  }
  comm_data->userData_.type_ = FCUT_NO_INIT;
  comm_data->userData_.selfData_ = NULL;
  comm_data->userData_.partnerData_ = NULL;

  return comm_data;
}

//--------------------------------------------------------------
//  データがすでにあるときはexistだけ切っておく(COMM_ACTOR再生成のため
//--------------------------------------------------------------
void FIELD_COMM_DATA_DataReset(FIELD_COMM_DATA *comm_data)
{
  int i;

  if(comm_data == NULL){
    return;
  }

  //データがすでにあるときはexistだけ切っておく(COMM_ACTOR再生成のため
  for( i=0;i<FIELD_COMM_MEMBER_MAX;i++ )
  {
    comm_data->charaData_[i].isExist_ = FALSE;
  }
}

//--------------------------------------------------------------
//  通信用データ管理初期化
//--------------------------------------------------------------
void  FIELD_COMM_DATA_TermSystem( FIELD_COMM_DATA *comm_data )
{
  GF_ASSERT( comm_data != NULL );
  GFL_HEAP_FreeMemory( comm_data );
}

//--------------------------------------------------------------
//  FieldCommFuncのステートのバックアップ
//--------------------------------------------------------------
void  FIELD_COMM_DATA_SetFieldCommMode( FIELD_COMM_DATA *comm_data, const u8 state )
{
  GF_ASSERT( comm_data != NULL );
  comm_data->backupCommState_ = state;
}
const u8 FIELD_COMM_DATA_GetFieldCommMode(FIELD_COMM_DATA *comm_data)
{
  GF_ASSERT( comm_data != NULL );
  return comm_data->backupCommState_;
}

//--------------------------------------------------------------
//  自分のデータセット
//--------------------------------------------------------------
void  FIELD_COMM_DATA_SetSelfData_Pos( FIELD_COMM_DATA *comm_data, const ZONEID *zoneID , const VecFx32 *pos , const u16 *dir )
{
  GF_ASSERT( comm_data != NULL );
  PLAYERWORK_setZoneID( &comm_data->selfData_.plWork_ , *zoneID );
  PLAYERWORK_setPosition( &comm_data->selfData_.plWork_ , pos );
  PLAYERWORK_setDirection( &comm_data->selfData_.plWork_ , *dir );
  comm_data->selfData_.isExist_ = TRUE;
  comm_data->selfData_.isValid_ = TRUE;
}

//--------------------------------------------------------------
//  自分のデータ取得
//--------------------------------------------------------------
PLAYER_WORK*  FIELD_COMM_DATA_GetSelfData_PlayerWork( FIELD_COMM_DATA *commData )
{
  return FIELD_COMM_DATA_GetCharaData_PlayerWork( commData, FCD_SELF_INDEX );
//  GF_ASSERT( commData != NULL );
//  return &commData->selfData_.plWork_;
}

//--------------------------------------------------------------
//  指定番号のフラグ取得・設定
//--------------------------------------------------------------
void  FIELD_COMM_DATA_SetCharaData_IsExist( FIELD_COMM_DATA *commData, const u8 idx , const BOOL flg)
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  pData->isExist_ = flg;
}
void  FIELD_COMM_DATA_SetCharaData_IsValid( FIELD_COMM_DATA *commData, const u8 idx , const BOOL flg)
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  pData->isValid_ = flg;
}
const BOOL  FIELD_COMM_DATA_GetCharaData_IsExist( FIELD_COMM_DATA *commData, const u8 idx )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  return pData->isExist_;
}
const BOOL  FIELD_COMM_DATA_GetCharaData_IsValid( FIELD_COMM_DATA *commData, const u8 idx )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  return pData->isValid_;
}

//--------------------------------------------------------------
//  会話系：会話ステートの取得・設定
//--------------------------------------------------------------
void  FIELD_COMM_DATA_SetTalkState( FIELD_COMM_DATA *commData, const u8 idx , const F_COMM_TALK_STATE state )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  pData->talkState_ = state;
}
const F_COMM_TALK_STATE FIELD_COMM_DATA_GetTalkState( FIELD_COMM_DATA *commData, const u8 idx )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  return pData->talkState_;
}

//--------------------------------------------------------------
//  キャラの移動後(停止時は現座標)を調べる
//  @return 移動中であればTRUE
//--------------------------------------------------------------
const BOOL  FIELD_COMM_DATA_GetGridPos_AfterMove( FIELD_COMM_DATA *commData, const u8 idx , int *posX , int *posZ )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  if( pData->isExist_ == FALSE )
  {
    //居ないっ
    *posX = -1;
    *posZ = -1;
    return FALSE;
  }
  {
    PLAYER_WORK *plWork = &pData->plWork_;
    //FIXME:正しいグリッドサイズ取得する？
    const u8 gridSize = 16;
    const int gridX = F32_CONST( plWork->position.x )+8;
    const int gridZ = F32_CONST( plWork->position.z )+8;
    u8  modSize;  //あまりの量
    *posX = gridX/gridSize;
    *posZ = gridZ/gridSize;
    //どれだけ中心位置からずれてるか？ XZ両方ずれていることは無いので片方だけ取る
    switch( plWork->direction )
    {
    case COMMDIR_UP:
    case COMMDIR_DOWN:
      modSize = gridZ - (*posZ*gridSize);
      break;
    case COMMDIR_LEFT:
    case COMMDIR_RIGHT:
      modSize = gridX - (*posX*gridSize);
      break;
    }
    //移動してないのでOK
    if( modSize == 0 )
      return FALSE;
    switch( plWork->direction )
    {
    case COMMDIR_UP:
      //処理なし
      break;
    case COMMDIR_DOWN:
      *posZ += 1;
      break;
    case COMMDIR_LEFT:
      //処理なし
      break;
    case COMMDIR_RIGHT:
      *posX += 1;
      break;
    }
    return TRUE;
  }
}

//--------------------------------------------------------------
//  指定番号のステータス取得
//--------------------------------------------------------------
const F_COMM_CHARA_STATE FIELD_COMM_DATA_GetCharaData_State( FIELD_COMM_DATA *commData, const u8 idx )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  return pData->state_;
}
void FIELD_COMM_DATA_SetCharaData_State( FIELD_COMM_DATA *commData, const u8 idx , const F_COMM_CHARA_STATE state )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  pData->state_ = state;
}

//--------------------------------------------------------------
//  指定番号のデータ取得
//--------------------------------------------------------------
PLAYER_WORK*  FIELD_COMM_DATA_GetCharaData_PlayerWork( FIELD_COMM_DATA *commData, const u8 idx )
{
  FIELD_COMM_CHARA_DATA *pData = FIELD_COMM_DATA_GetCharaDataWork(commData, idx);
  return &pData->plWork_;
}

//--------------------------------------------------------------
//  通信用データ管理初期化
//--------------------------------------------------------------
static void FIELD_COMM_DATA_InitOneCharaData( FIELD_COMM_CHARA_DATA *charaData )
{
  charaData->isExist_ = FALSE;
  charaData->isValid_ = FALSE;
  charaData->state_ = FCCS_NONE;
  charaData->talkState_ = FCTS_NONE;
  PLAYERWORK_init( &charaData->plWork_ );
}

//--------------------------------------------------------------
//  データ取得用　自キャラ・他キャラ管理
//--------------------------------------------------------------
static FIELD_COMM_CHARA_DATA* FIELD_COMM_DATA_GetCharaDataWork( FIELD_COMM_DATA *commData, const u8 idx )
{
  GF_ASSERT( commData != NULL );
  if( idx == FCD_SELF_INDEX )
    return &commData->selfData_;
  GF_ASSERT( idx < FIELD_COMM_MEMBER_MAX );
  return &commData->charaData_[idx];
}

//--------------------------------------------------------------
//ユーザーデータ関連
//--------------------------------------------------------------
void  FIELD_COMM_DATA_CreateUserData( FIELD_COMM_DATA *commData, const F_COMM_USERDATA_TYPE type )
{
  GF_ASSERT( commData != NULL );
  GF_ASSERT( commData->userData_.type_ == FCUT_NO_INIT );

  commData->userData_.type_ = type;
  commData->userData_.selfData_ = GFL_HEAP_AllocClearMemory( commData->heapID_ , FIELD_COMM_DATA_GetUserDataSize( type ));
  commData->userData_.partnerData_ = GFL_HEAP_AllocClearMemory( commData->heapID_ , FIELD_COMM_DATA_GetUserDataSize( type ));
}

void  FIELD_COMM_DATA_DeleteUserData(FIELD_COMM_DATA *commData)
{
  GF_ASSERT( commData != NULL );
  GF_ASSERT( commData->userData_.type_ < FCUT_MAX );

  GFL_HEAP_FreeMemory( commData->userData_.selfData_ );
  GFL_HEAP_FreeMemory( commData->userData_.partnerData_ );
  commData->userData_.type_ = FCUT_NO_INIT;
}

const u32 FIELD_COMM_DATA_GetUserDataSize( const F_COMM_USERDATA_TYPE type )
{
  static const u32 UserDataSizeTable[FCUT_MAX] =
  {
    sizeof(TR_CARD_DATA),
    0x400,
  };
  return UserDataSizeTable[type];
}
//--------------------------------------------------------------
//  行動に対応したデータの型を返す
//--------------------------------------------------------------
const F_COMM_USERDATA_TYPE  FIELD_COMM_DATA_GetUserDataType_From_Action( const F_COMM_ACTION_LIST action )
{
  static const F_COMM_USERDATA_TYPE UserDataTypeTable[FCAL_MAX] =
  {
    FCUT_TRAINERCARD,
    FCUT_BATTLE,
    FCUT_MAX
  };
  return UserDataTypeTable[action];
}

void* FIELD_COMM_DATA_GetSelfUserData( FIELD_COMM_DATA *commData, const F_COMM_USERDATA_TYPE type )
{
  GF_ASSERT( commData != NULL );
  //受信バッファでタイプの指定ができないのでFCUT_MAXで処置。
  GF_ASSERT( commData->userData_.type_ == type || type == FCUT_MAX );
  return commData->userData_.selfData_;
}

void* FIELD_COMM_DATA_GetPartnerUserData( FIELD_COMM_DATA *commData, const F_COMM_USERDATA_TYPE type )
{
  GF_ASSERT( commData != NULL );
  //受信バッファでタイプの指定ができないのでFCUT_MAXで処置。
  GF_ASSERT( commData->userData_.type_ == type || type == FCUT_MAX );
  return commData->userData_.partnerData_;
}

F_COMM_USERDATA_TYPE FIELD_COMM_DATA_GetUserDataType( FIELD_COMM_DATA *commData )
{
  if( commData == NULL )
  {
    return FCUT_NO_INIT;
  }
  return commData->userData_.type_;
}
