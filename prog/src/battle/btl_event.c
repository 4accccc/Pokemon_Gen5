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
#include "btl_sideeff.h"
#include "btl_field.h"

#include "btl_event.h"
#include "btl_event_factor.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  FACTOR_PER_POKE = EVENT_WAZA_STICK_MAX + 3,            ///< ワザ貼り付き最大数+使用ワザ+とくせい+アイテム = 11
  FACTOR_MAX_FOR_POKE = FACTOR_PER_POKE * BTL_POS_MAX,   ///< ポケモンごと最大数(11) * 場所数(6) = 66
  FACTOR_MAX_FOR_SIDEEFF = BTL_SIDEEFF_MAX * 2,          ///< サイドエフェクト最大数(9)×陣営数(2) = 18
  FACTOR_MAX_FOR_POSEFF = BTL_POSEFF_MAX * BTL_POS_MAX,  ///< 位置エフェクト最大数(3) * 場所数(6) = 18
  FACTOR_MAX_FOR_FIELD = BTL_FLDEFF_MAX,                 ///< フィールドエフェクト最大数 = 6

  // 登録できるイベントファクター最大数（66 + 18 + 18 + 6) = 108
  FACTOR_REGISTER_MAX = FACTOR_MAX_FOR_POKE + FACTOR_MAX_FOR_SIDEEFF + FACTOR_MAX_FOR_FIELD + FACTOR_MAX_FOR_POSEFF,
  EVENTVAL_STACK_DEPTH = 128,   ///< イベント変数スタックの容量

};

/**
 * 値の書き換え可否状態
 */

typedef enum {

  REWRITE_FREE = 0,   ///< 自由に可能
  REWRITE_ONCE,       ///< １回だけ可能
  REWRITE_LOCK,       ///< 不可

}RewriteState;

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
  BtlEventSkipCheckHandler  skipCheckHandler;
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
  u32   sp;                                 ///< スタックポインタ
  u16   label[ EVENTVAL_STACK_DEPTH ];      ///< 変数ラベル
  int   value[ EVENTVAL_STACK_DEPTH ];      ///< 変数
  fx32  mulMax[ EVENTVAL_STACK_DEPTH ];     ///< 乗算対応実数の最大値
  fx32  mulMin[ EVENTVAL_STACK_DEPTH ];     ///< 乗算対応実数の最小値
  u8    rewriteState[ EVENTVAL_STACK_DEPTH ];///< 書き換え可否状態
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
static BOOL check_handler_skip( BTL_SVFLOW_WORK* flowWork, BtlEventFactor factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void varStack_Init( void );
static int evar_getNewPoint( const VAR_STACK* stack, BtlEvVarLabel label );
static int evar_getExistPoint( const VAR_STACK* stack, BtlEvVarLabel label );
static fx32 evar_mulValueRound( const VAR_STACK* stack, int ptr, fx32 value );


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
    newFactor->skipCheckHandler = NULL;
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
 * スキップチェックハンドラをアタッチする
 *
 * @param   factor
 * @param   handler
 */
//=============================================================================================
void BTL_EVENT_FACTOR_AttachSkipCheckHandler( BTL_EVENT_FACTOR* factor, BtlEventSkipCheckHandler handler )
{
  factor->skipCheckHandler = handler;
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
 * 関連ポケモンIDを返す
 *
 * @param   factor
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_EVENT_FACTOR_GetPokeID( const BTL_EVENT_FACTOR* factor )
{
  return factor->pokeID;
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
 * ワーク内容を設定
 *
 * @param   factor
 * @param   workIdx
 * @param   value
 */
//=============================================================================================
void BTL_EVENT_FACTOR_SetWorkValue( BTL_EVENT_FACTOR* factor, u8 workIdx, int value )
{
  GF_ASSERT(workIdx < EVENT_HANDLER_WORK_ELEMS);
  factor->work[ workIdx ] = value;
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
//----------------------------------------------------------------------------------
/**
 * 登録ハンドラ呼び出し
 *
 * @param   factor
 * @param   eventType
 * @param   flowWork
 */
//----------------------------------------------------------------------------------
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
        if( !check_handler_skip(flowWork, factor->factorType, eventType, factor->subID, factor->pokeID) )
        {
          factor->callingFlag = TRUE;
          tbl[i].handler( factor, flowWork, factor->pokeID, factor->work );
          factor->callingFlag = FALSE;
        }
      }
    }
  }
}
/**
 *  特定ハンドラをスキップするかチェック
 */
static BOOL check_handler_skip( BTL_SVFLOW_WORK* flowWork, BtlEventFactor factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  BTL_EVENT_FACTOR* factor;
  const BTL_POKEPARAM* bpp;

  for( factor=FirstFactorPtr; factor!=NULL; factor=factor->next )
  {
    // 「いえき」状態のポケモンは、とくせいハンドラを呼び出さない
    bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWork, pokeID );
    if( BPP_CheckSick(bpp, WAZASICK_IEKI) && (factorType == BTL_EVENT_FACTOR_TOKUSEI) ){
      return TRUE;
    }

    // スキップチェックハンドラが登録されていたら判断させる
    if( factor->skipCheckHandler ){
      if( factor->skipCheckHandler( factor, factorType, eventType, subID, pokeID) ){
        return TRUE;
      }
    }
  }
  return FALSE;
}





BTL_EVENT_FACTOR* BTL_EVENT_SeekFactorCore( BtlEventFactor factorType )
{
  BTL_EVENT_FACTOR* factor;

  for( factor=FirstFactorPtr; factor!=NULL; factor=factor->next )
  {
    if( factor->factorType == factorType ){
      return factor;
    }
  }
  return NULL;
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
    BtlEventFactor type = factor->factorType;
    u8 pokeID = factor->pokeID;

    factor = factor->next;
    while( factor ){
      if( (factor->factorType == type) && (factor->pokeID == pokeID) ){
        BTL_Printf("イベントタイプ[%d], ポケ[%d]の次のハンドラが見つかったよ\n", type, pokeID );
        return factor;
      }
      factor = factor->next;
    }
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


//======================================================================================================
//  イベント変数スタック構造
//======================================================================================================
enum {
  EVAR_PRINT_FLG = FALSE,
};

static void varStack_Init( void )
{
  VAR_STACK* stack = &VarStack;
  int i;

  stack->sp = 0;
  for(i=0; i<NELEMS(stack->label); ++i)
  {
    stack->label[i] = BTL_EVAR_NULL;
  }
}

void BTL_EVENTVAR_Push( void )
{
  VAR_STACK* stack = &VarStack;

  while( stack->sp < NELEMS(stack->label) )
  {
    if( stack->label[stack->sp] == BTL_EVAR_NULL ){
      break;
    }
    stack->sp++;
  }

  if( stack->sp < (NELEMS(stack->label)-1) )
  {
    BTL_PrintfEx( EVAR_PRINT_FLG, "[EVAR] PUSH sp:%d\n", stack->sp );
    stack->label[ stack->sp++ ] = BTL_EVAR_SYS_SEPARATE;
    #ifdef PM_DEBUG
    if( stack->sp >= (NELEMS(stack->label)/8*7) ){
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

  if( stack->sp > 0 )
  {
    u16 p = stack->sp;
    while( (p < NELEMS(stack->label)) && (stack->label[p] != BTL_EVAR_NULL) ){
      BTL_PrintfEx( EVAR_PRINT_FLG, "[EVAR]  <prePOPv clear:%d\n", p);
      stack->label[p++] = BTL_EVAR_NULL;
    }

    stack->sp--;
    stack->label[ stack->sp ] = BTL_EVAR_NULL;
    BTL_PrintfEx( EVAR_PRINT_FLG, "[EVAR]  <prePOPs clear:%d\n", stack->sp);


    if( stack->sp )
    {
      while( stack->sp )
      {
        stack->sp--;
        if( stack->label[stack->sp] == BTL_EVAR_SYS_SEPARATE ){
          stack->sp++;
          break;
        }
      }
    }
    BTL_PrintfEx( EVAR_PRINT_FLG, "[EVAR] POP  sp:%d\n", stack->sp);
  }
  else
  {
    GF_ASSERT(0);
  }
}

//=============================================================================================
/**
 * ラベルの値を新規セット（サーバフロー用）
 *
 * @param   label
 * @param   value
 */
//=============================================================================================
void BTL_EVENTVAR_SetValue( BtlEvVarLabel label, int value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;
    int p = evar_getNewPoint( stack, label );
    stack->label[p] = label;
    stack->value[p] = value;
    stack->mulMin[p] = 0;
    stack->mulMax[p] = 0;
    stack->rewriteState[p] = REWRITE_FREE;
  }
}
//=============================================================================================
/**
 * 上書き禁止ラベルの値を新規セット（サーバフロー用）
 *
 * @param   label
 * @param   value
 */
//=============================================================================================
void BTL_EVENTVAR_SetConstValue( BtlEvVarLabel label, int value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;
    int p = evar_getNewPoint( stack, label );
    stack->label[p] = label;
    stack->value[p] = value;
    stack->mulMin[p] = 0;
    stack->mulMax[p] = 0;
    stack->rewriteState[p] = REWRITE_LOCK;
  }
}
//=============================================================================================
/**
 * 上書き１回のみ有効なラベルの値を新規セット（サーバフロー用）
 *
 * @param   label
 * @param   value
 */
//=============================================================================================
void BTL_EVENTVAR_SetRewriteOnceValue( BtlEvVarLabel label, int value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;
    int p = evar_getNewPoint( stack, label );
    stack->label[p] = label;
    stack->value[p] = value;
    stack->mulMin[p] = 0;
    stack->mulMax[p] = 0;
    stack->rewriteState[p] = REWRITE_ONCE;
  }
}
//=============================================================================================
/**
 * 乗算対応ラベルの値を新規セット（サーバフロー用）
 *
 * @param   label
 * @param   value
 * @param   mulMin
 * @param   mulMax
 */
//=============================================================================================
void BTL_EVENTVAR_SetMulValue( BtlEvVarLabel label, int value, fx32 mulMin, fx32 mulMax )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;
    int p = evar_getNewPoint( stack, label );
    stack->label[p] = label;
    stack->value[p] = value;
    stack->mulMin[p] = mulMin;
    stack->mulMax[p] = mulMax;
    stack->rewriteState[p] = REWRITE_FREE;
  }
}
//=============================================================================================
/**
 * 既存ラベルの値を上書き（ハンドラ用）
 *
 * @param   label
 * @param   value
 *
 * @retval  BOOL    上書き出来たらTRUE
 */
//=============================================================================================
BOOL BTL_EVENTVAR_RewriteValue( BtlEvVarLabel label, int value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;
    int p = evar_getExistPoint( stack, label );
    if( p >= 0 )
    {
      if( stack->rewriteState[p] != REWRITE_LOCK )
      {
        stack->label[p] = label;
        stack->value[p] = (int)value;
        if( stack->rewriteState[p] == REWRITE_ONCE ){
          stack->rewriteState[p] = REWRITE_LOCK;
        }
        return TRUE;
      }
      else{
        GF_ASSERT_MSG(0, "this label(%d) can't rewrite\n", label);
      }
    }
    return FALSE;
  }
}
//=============================================================================================
/**
 * 既存の乗算対応ラベルの値に数値を乗算する
 *
 * @param   label   ラベルID
 * @param   value   乗算する値
 */
//=============================================================================================
void BTL_EVENTVAR_MulValue( BtlEvVarLabel label, fx32 value )
{
  GF_ASSERT(label!=BTL_EVAR_NULL);
  GF_ASSERT(label!=BTL_EVAR_SYS_SEPARATE);

  {
    VAR_STACK* stack = &VarStack;
    int p = evar_getExistPoint( stack, label );
    if( p >= 0 )
    {
      value = FX_Mul( stack->value[p], value );
      value = evar_mulValueRound( stack, p, value );
      stack->value[p] = (int)value;
    }
  }
}
//=============================================================================================
/**
 * 既存ラベルの値を取得
 *
 * @param   label
 *
 * @retval  int
 */
//=============================================================================================
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
// 新規ラベル用に確保した位置のポインタを返す
static int evar_getNewPoint( const VAR_STACK* stack, BtlEvVarLabel label )
{
  int p = stack->sp;
  while( p < NELEMS(stack->label) ){
    if( stack->label[p] == BTL_EVAR_NULL ){ break; }
    if( stack->label[p] == label ){
      GF_ASSERT_MSG(0, "Find Same Label ID : %d\n", label);
      break;
    }
    ++p;
  }

  if( p < NELEMS(stack->value) )
  {
    #ifdef PM_DEBUG
    if( p >= (NELEMS(stack->label)/8*7) ){
      BTL_Printf("Var Stack sp=%d 危険水域です！！\n", p);
    }
    #endif
    BTL_PrintfEx( EVAR_PRINT_FLG, "[EVAR]  >regist pos:%d label=%d\n", p, label);
    return p;
  }
  GF_ASSERT(0); // stack overflow
  return 0;
}
// 既存ラベル位置のポインタを返す
static int evar_getExistPoint( const VAR_STACK* stack, BtlEvVarLabel label )
{
  int p = stack->sp;
  while( (p < NELEMS(stack->label)) && (stack->label[p] != BTL_EVAR_NULL) )
  {
    if( stack->label[p] == label ){
      return p;
    }
    ++p;
  }
  GF_ASSERT_MSG(0, "label[%d] not found", label);
  return -1;
}
// 乗算対応変数の上下限を越えていたら丸めて返す
static fx32 evar_mulValueRound( const VAR_STACK* stack, int ptr, fx32 value )
{
  if( stack->mulMax[ptr] || stack->mulMin[ptr] )  // どちらかが0以外なら乗算対応と見なす
  {
    if( stack->mulMin[ptr] > value ){
      value = stack->mulMin[ptr];
    }
    if( stack->mulMax[ptr] < value ){
      value = stack->mulMax[ptr];
    }
  }
  return value;
}

