//=============================================================================================
/**
 * @file  btl_event.c
 * @brief ポケモンWB バトルシステム イベント＆ハンドラ処理
 * @author  taya
 *
 * @date  2008.10.23  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"

#include "btl_common.h"
#include "btl_pokeparam.h"

#include "btl_event.h"
#include "btl_event_factor.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  FACTOR_REGISTER_MAX = 32,     ///< 登録できるイベントファクター最大数
  EVENTVAL_STACK_DEPTH = 128,   ///< イベント変数スタックの容量
};

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/

/**
* イベントデータ実体
*/
struct _BTL_EVENT_FACTOR {
  BTL_EVENT_FACTOR* prev;
  BTL_EVENT_FACTOR* next;
  const BtlEventHandlerTable* handlerTable;
  BtlEventFactor  factorType;
  u32       priority;
  int       work[ EVENT_HANDLER_WORK_ELEMS ];
  u16       subID;
  u8        pokeID;
  u8        callingFlag : 1;
  u8        sleepFlag : 1;
};

/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
static BTL_EVENT_FACTOR  Factors[ FACTOR_REGISTER_MAX ];
static BTL_EVENT_FACTOR* FactorStack[ FACTOR_REGISTER_MAX ];
static BTL_EVENT_FACTOR* FirstFactorPtr;
static u16 StackPtr;


/**
* イベント変数スタック
*/
typedef struct {
  u32 sp;
  u16 label[ EVENTVAL_STACK_DEPTH ];
  int value[ EVENTVAL_STACK_DEPTH ];
}VAR_STACK;

static VAR_STACK VarStack = {0};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BTL_EVENT_FACTOR* popFactor( void );
static void pushFactor( BTL_EVENT_FACTOR* factor );
static void clearFactorWork( BTL_EVENT_FACTOR* factor );
static inline u32 calcFactorPriority( BtlEventFactor factorType, u16 subPri );
static void callHandlers( BTL_EVENT_FACTOR* factor, BtlEventType eventType, BTL_SVFLOW_WORK* flowWork );
static void varStack_Init( void );


//=============================================================================================
/**
 * システム初期化 （バトル開始時、サーバマシンから１回呼び出す）
 */
//=============================================================================================
void BTL_EVENT_InitSystem( void )
{
  int i;

  for(i=0; i<NELEMS(Factors); i++)
  {
    clearFactorWork( &Factors[i] );
    FactorStack[i] = &Factors[i];
  }

  FirstFactorPtr = NULL;
  StackPtr = 0;

  varStack_Init();
}


//=============================================================================================
/**
 * ターンごとの初期化処理
 */
//=============================================================================================
void BTL_EVENT_StartTurn( void )
{
  // スリープフラグを落とす
  BTL_EVENT_FACTOR* factor;

  for(factor=FirstFactorPtr; factor!=NULL; factor=factor->next)
  {
    factor->sleepFlag = FALSE;
  }
}


//=============================================================================================
/**
 * イベント反応要素を追加
 *
 * @param   factorType      ファクタータイプ
 * @param   subID           タイプごとの個別ID（とくせいID，アイテムID等と一致）
 * @param   priority
 * @param   pokeID
 * @param   handlerTable
 *
 * @retval  BTL_EVENT_FACTOR*
 */
//=============================================================================================
BTL_EVENT_FACTOR* BTL_EVENT_AddFactor( BtlEventFactor factorType, u16 subID, u16 priority, u8 pokeID, const BtlEventHandlerTable* handlerTable )
{
  BTL_EVENT_FACTOR* newFactor;

  newFactor = popFactor();
  if( newFactor )
  {
    newFactor->priority = calcFactorPriority( factorType, priority );
    newFactor->factorType = factorType;
    newFactor->prev = NULL;
    newFactor->next = NULL;
    newFactor->handlerTable = handlerTable;
    newFactor->pokeID = pokeID;
    newFactor->subID = subID;
    newFactor->callingFlag = FALSE;
    newFactor->sleepFlag = FALSE;
    GFL_STD_MemClear( newFactor->work, sizeof(newFactor->work) );

    // 最初の登録
    if( FirstFactorPtr == NULL )
    {
      FirstFactorPtr = newFactor;
    }
    // 現在先頭より高プライオリティ
    else if( newFactor->priority < FirstFactorPtr->priority )
    {
      FirstFactorPtr->prev = newFactor;
      newFactor->next = FirstFactorPtr;
      FirstFactorPtr = newFactor;
    }
    // それ以外はふつうのリンクリスト接続
    else
    {
      BTL_EVENT_FACTOR *find, *last;

      last = FirstFactorPtr;
      for( find=FirstFactorPtr->next; find!=NULL; find=find->next )
      {
        if( newFactor->priority < find->priority )
        {
          newFactor->next = find;
          newFactor->prev = find->prev;
          find->prev->next = newFactor;
          find->prev = newFactor;
          break;
        }
        last = find;
      }
      // 最後まで見つからない場合
      if( find == NULL )
      {
        last->next = newFactor;
        newFactor->prev = last;
      }
    }
    return newFactor;
  }
  // スタックから見つからない
  else
  {
    GF_ASSERT(0);
    return NULL;
  }
}
//=============================================================================================
/**
 * イベント反応要素を削除
 *
 * @param   factor
 *
 */
//=============================================================================================
void BTL_EVENT_FACTOR_Remove( BTL_EVENT_FACTOR* factor )
{
  if( factor == FirstFactorPtr )
  {
    FirstFactorPtr = factor->next;
  }

  if( factor->prev )
  {
    factor->prev->next = factor->next;
  }

  if( factor->next )
  {
    factor->next->prev = factor->prev;
  }

  pushFactor( factor );
}
//=============================================================================================
/**
 * 反応要素の対応するポケモンを差し替え
 *
 * @param   factor
 * @param   pokeID
 * @param   pri
 *
 */
//=============================================================================================
void BTL_EVENT_FACTOR_ChangePokeParam( BTL_EVENT_FACTOR* factor, u8 pokeID, u16 pri )
{
  const BtlEventHandlerTable* handlerTable = factor->handlerTable;
  BtlEventFactor type = factor->factorType;
  u16 subID = factor->subID;

  BTL_EVENT_FACTOR_Remove( factor );
  BTL_EVENT_AddFactor( type, subID, pri, pokeID, handlerTable );
}
//=============================================================================================
/**
 * サブID（とくせいID，アイテムID等、要素タイプに準じる）を返す
 *
 * @param   factor
 *
 * @retval  u16
 */
//=============================================================================================
u16 BTL_EVENT_FACTOR_GetSubID( const BTL_EVENT_FACTOR* factor )
{
  return factor->subID;
}
//=============================================================================================
/**
 * ワーク内容を取得
 *
 * @param   factor
 * @param   workIdx
 *
 * @retval  int
 */
//=============================================================================================
int BTL_EVENT_FACTOR_GetWorkValue( const BTL_EVENT_FACTOR* factor, u8 workIdx )
{
  GF_ASSERT(workIdx < EVENT_HANDLER_WORK_ELEMS);
  return factor->work[ workIdx ];
}


//=============================================================================================
/**
 * 全登録要素に対し、指定イベントの通知
 *
 * @param   flowWork
 * @param   eventID
 */
//=============================================================================================
void BTL_EVENT_CallHandlers( BTL_SVFLOW_WORK* flowWork, BtlEventType eventID )
{
  BTL_EVENT_FACTOR* factor;
  BTL_EVENT_FACTOR* next_factor;

  for( factor=FirstFactorPtr; factor!=NULL; )
  {
    next_factor = factor->next;
    callHandlers( factor, eventID, flowWork );
    factor = next_factor;
  }
}

BTL_EVENT_FACTOR* BTL_EVENT_SeekFactor( BtlEventFactor factorType, u8 pokeID )
{
  BTL_EVENT_FACTOR* factor;

  for( factor=FirstFactorPtr; factor!=NULL; factor=factor->next )
  {
    if( (factor->factorType == factorType) && (factor->pokeID == pokeID) )
    {
      return factor;
    }
  }
  return NULL;
}

BTL_EVENT_FACTOR* BTL_EVENT_GetNextFactor( BTL_EVENT_FACTOR* factor )
{
  if( factor ){
    return factor->next;
  }
  return NULL;
}

//=============================================================================================
/**
 * 特定タイプ＆サブIDのファクターを１ターン休止させる
 *
 * @param   type
 * @param   subID
 *
 */
//=============================================================================================
void BTL_EVENT_SleepFactor( BtlEventFactor type, u16 subID )
{
  BTL_EVENT_FACTOR* factor;

  for( factor=FirstFactorPtr; factor!=NULL; factor=factor->next )
  {
    if( (factor->factorType == type) && (factor->subID == subID) )
    {
      factor->sleepFlag = TRUE;
    }
  }
}


static BTL_EVENT_FACTOR* popFactor( void )
{
  if( StackPtr == FACTOR_REGISTER_MAX )
  {
    return NULL;
  }
  return FactorStack[ StackPtr++ ];
}


static void pushFactor( BTL_EVENT_FACTOR* factor )
{
  if( StackPtr == 0 )
  {
    GF_ASSERT(0);
    return;
  }
  clearFactorWork( factor );  //値をクリアしてからスタックに積む
  --StackPtr;
  FactorStack[ StackPtr ] = factor;
}

static void clearFactorWork( BTL_EVENT_FACTOR* factor )
{
  GFL_STD_MemClear( factor, sizeof(factor) );
}

static inline u32 calcFactorPriority( BtlEventFactor factorType, u16 subPri )
{
  return (factorType << 16) | subPri;
}

static void callHandlers( BTL_EVENT_FACTOR* factor, BtlEventType eventType, BTL_SVFLOW_WORK* flowWork )
{
  if( (factor->callingFlag == FALSE) && (factor->sleepFlag == FALSE) )
  {
    const BtlEventHandlerTable* tbl = factor->handlerTable;

    int i;
    for(i=0; tbl[i].eventType!=BTL_EVENT_NULL; i++)
    {
      if( tbl[i].eventType == eventType )
      {
        factor->callingFlag = TRUE;
        tbl[i].handler( factor, flowWork, factor->pokeID, factor->work );
        factor->callingFlag = FALSE;
      }
    }
  }
}


//======================================================================================================
//  イベント変数スタック構造
//======================================================================================================


static void varStack_Init( void )
{
  VAR_STACK* stack = &VarStack;
  int i;

  stack->label[0] = BTL_EVAR_SYS_SEPARATE;
  stack->sp = 0;
  for(i=0; i<NELEMS(stack->label); ++i)
  {
    stack->label[i] = BTL_EVAR_NULL;
  }
}


void BTL_EVENTVAR_Push( void )
{
  VAR_STACK* stack = &VarStack;

  if( stack->sp < NELEMS(stack->label) )
  {
    stack->label[ stack->sp++ ] = BTL_EVAR_SYS_SEPARATE;
    #ifdef PM_DEBUG
    if( stack->sp >= (NELEMS(stack->label)/8*7) )
    {
      BTL_Printf("Var Stack sp=%d 危険水域です！！\n", stack->sp);
    }
    #endif
  }
  else
  {
    GF_ASSERT(0);   // stack overflow
  }
}

void BTL_EVENTVAR_Pop( void )
{
  VAR_STACK* stack = &VarStack;

  while( stack->sp )
  {
    stack->sp--;
    if( stack->label[stack->sp] == BTL_EVAR_SYS_SEPARATE ){ break; }
  }
}

void BTL_EVENTVAR_SetValue( BtlEvVarLabel label, int value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;

    int p = stack->sp;
    while( p < NELEMS(stack->label) )
    {
      if( (stack->label[p] == BTL_EVAR_NULL)
      ||  (stack->label[p] == label)
      ){
        break;
      }
      ++p;
    }
    if( p < NELEMS(stack->value) )
    {
      stack->label[p] = label;
      stack->value[p] = value;
      #ifdef PM_DEBUG
      if( p >= (NELEMS(stack->label)/8*7) )
      {
        BTL_Printf("Var Stack sp=%d 危険水域です！！\n", p);
      }
      #endif
    }
    else
    {
      GF_ASSERT(0); // stack overflow
    }
  }
}
void BTL_EVENTVAR_RewriteValue( BtlEvVarLabel label, int value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;

    int p = stack->sp;
    while( p < NELEMS(stack->label) )
    {
      if(stack->label[p] == label)
      {
        break;
      }
      ++p;
    }
    if( p < NELEMS(stack->value) )
    {
      stack->label[p] = label;
      stack->value[p] = value;
      #ifdef PM_DEBUG
      if( p >= (NELEMS(stack->label)/8*7) )
      {
        BTL_Printf("Var Stack sp=%d 危険水域です！！\n", p);
      }
      #endif
    }
    else
    {
      GF_ASSERT(0); // stack overflow
    }
  }
}
int BTL_EVENTVAR_GetValue( BtlEvVarLabel label )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);
  {
    VAR_STACK* stack = &VarStack;

    int p = stack->sp;
    while( p < NELEMS(stack->label) )
    {
      if( stack->label[p] == label ){
        return stack->value[p];
      }
      if( stack->label[p] == BTL_EVAR_NULL ){
        break;
      }
      ++p;
    }
    GF_ASSERT_MSG(0,"label=%d",label);  // label not found
    return 0;
  }
}





