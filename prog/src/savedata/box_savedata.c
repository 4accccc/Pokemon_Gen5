//==============================================================================
/**
 * @file  contest_savedata.c
 * @brief ボックスセーブデータ
 * @author  ariizumi
 * @date  2008.11.07
 */
//==============================================================================

#include <gflib.h>

#include "savedata/box_savedata.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "arc_def.h"

#include "system/main.h"
#include "msg/msg_boxmenu.h"
#include "message.naix"
#include "savedata/save_tbl.h"


static void PPP_RecoverPP(const POKEMON_PASO_PARAM* pp){}  //ダミー関数
static BOX_TRAY_DATA* BOXTRAYDAT_GetTrayData( const BOX_MANAGER *boxData , const u32 trayNum );
static BOX_SAVEDATA* BOXDAT_GetBoxSaveData( const BOX_MANAGER *boxData );



//#define TRAY_ALL_USE_BIT  ((1<<BOX_MAX_TRAY)-1)
//#define TRAY_ALL_USE_BIT  (0b111111111111111111)

//フラッシュのページをまたがないように1トレー分のデータを256バイトアライメントする
struct _BOX_TRAY_DATA
{
  POKEMON_PASO_PARAM  ppp[BOX_MAX_POS];
//  u8 dummy[TRAY_DUMMY_NUM];
};

struct _BOX_SAVEDATA{
//  BOX_TRAY_DATA   btd[BOX_MAX_TRAY];  //WBよりトレーが個別のセーブになります。
  //ここより下のデータが２５６バイトアライメントされたところにマッピングされる
  u32     currentTrayNumber;                //4
//  u32     UseBoxBits;                   //4
  STRCODE trayName[BOX_MAX_TRAY][BOX_TRAYNAME_BUFSIZE]; //2*20*24 = 960
  u8      wallPaper[BOX_MAX_TRAY];            //24
  u8      daisukiBitFlag;                 //1
  u8      trayMax;          // 開放されてるトレイ数
  u8      dummy[2];         // アライメントダミー     //2

  //以下昔の情報
  //ポケモンデータが0x1000*18 バイト = 0x12000
  //ポケモンデータ以外で764バイト = 0x2fc
  //計0x122fcバイト
  //これにＣＲＣ４バイトを追加すると、0x12300となり、256バイトアライメントされ、
  //フッター情報のみの書き込みの際に、フラッシュページをまたがなくなる
};


// 構造体が想定のサイズとなっているかチェック
#ifdef PM_DEBUG
#ifdef _NITRO
SDK_COMPILER_ASSERT(sizeof(BOX_TRAY_DATA) == 4080); //ヘッダ分減らしました
SDK_COMPILER_ASSERT(sizeof(BOX_SAVEDATA) == 0x3e0);
#endif
#endif

struct _BOX_MANAGER{
  SAVE_CONTROL_WORK *sv;
};


//==============================================================
// Prototype
//==============================================================
static void boxdata_init( BOX_SAVEDATA* boxdat );


//------------------------------------------------------------------
/**
 * ボックスデータ初期化
 */
//------------------------------------------------------------------
void BOXDAT_Init( BOX_SAVEDATA* boxdat )
{
  boxdata_init(boxdat);
/// SaveData_RequestTotalSave();金銀で削除
}
//------------------------------------------------------------------
/**
 * ボックスデータの総サイズを返す
 *
 * @retval  u32   ボックスデータサイズ
 */
//------------------------------------------------------------------
u32 BOXDAT_GetTotalSize( void )
{
  return sizeof( BOX_SAVEDATA );
}


//------------------------------------------------------------------
/**
 * ボックスデータ領域初期化
 *
 * @param   boxdat
 *
 */
//------------------------------------------------------------------
static void boxdata_init( BOX_SAVEDATA* boxdat )
{
  u32 i, p;
  GFL_MSGDATA*  msgman;
/*
  for(i = 0; i < BOX_MAX_TRAY; i++)
  {
    for(p = 0; p < BOX_MAX_POS; p++)
    {
      PPP_Clear( &(boxdat->btd[i].ppp[p]) );
    }
  }
*/

  // 壁紙ナンバー，だいすきクラブ壁紙の取得フラグ
  for(i = 0, p = 0; i < BOX_MAX_TRAY; i++)
  {
    boxdat->wallPaper[i] = p++;
    if(p >= BOX_NORMAL_WALLPAPER_MAX)
    {
      p = 0;
    }
  }
  boxdat->daisukiBitFlag = 0;


  // デフォルトボックス名セット
  msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_boxmenu_dat, GFL_HEAPID_APP );
  if( msgman )
  {
    for(i = 0 ; i < BOX_MAX_TRAY; i++)
    {
      GFL_MSG_GetStringRaw( msgman, mes_boxmenu_02_02+i, boxdat->trayName[i], BOX_TRAYNAME_BUFSIZE );
    }
    GFL_MSG_Delete( msgman );
  }

  boxdat->currentTrayNumber = 0;

  // 開放されてるトレイ数
  boxdat->trayMax = BOX_MIN_TRAY;
}

static BOX_SAVEDATA* BOXDAT_GetBoxSaveData( const BOX_MANAGER *boxData )
{
  return SaveControl_DataPtrGet(boxData->sv, GMDATA_ID_BOXDATA);
}

//------------------------------------------------------------------
/**
 * ボックス全体からから空き領域を探してポケモンデータを格納
 *
 * @param   box     ボックスデータポインタ
 * @param   poke    ポケモンデータ
 *
 * @retval  BOOL    TRUE=格納された／FALSE=空きが無い
 */
//------------------------------------------------------------------
BOOL BOXDAT_PutPokemon( BOX_MANAGER* box, const POKEMON_PASO_PARAM* poke )
{
  u32 b;
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);

  b = boxData->currentTrayNumber;
  do
  {
    PPP_RecoverPP(poke);

    if( BOXDAT_PutPokemonBox( box, b, poke ) )
    {
      //SaveData_RequestTotalSave();  //金銀で変更
//      BOXDAT_SetTrayUseBit( box, b);
      return TRUE;
    }

//    if( ++b >= BOX_MAX_TRAY )
    if( ++b >= boxData->trayMax )
    {
      b = 0;
    }

  }while( b != boxData->currentTrayNumber );

  // 開放されていないトレイをチェック
  if( boxData->trayMax != BOX_MAX_TRAY ){
    BOXDAT_PutPokemonBox( box, boxData->trayMax, poke );
    //SaveData_RequestTotalSave();  //金銀で変更
//    BOXDAT_SetTrayUseBit( box, boxData->trayMax );
    boxData->trayMax += BOX_MIN_TRAY;
    return TRUE;
  }

  return FALSE;
}

//------------------------------------------------------------------
/**
 * ボックスを指定してポケモンデータ格納
 *
 * @param   box     ボックスデータポインタ
 * @param   trayNum   何番目のボックスに格納するか
 * @param   poke    ポケモンデータ
 *
 * @retval  BOOL    TRUE=格納された／FALSE=空きが無い
 */
//------------------------------------------------------------------
BOOL BOXDAT_PutPokemonBox( BOX_MANAGER* box, u32 trayNum, const POKEMON_PASO_PARAM* poke )
{
  u32 i;
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);

  PPP_RecoverPP(poke);

  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

  for(i = 0; i < BOX_MAX_POS; i++)
  {
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
    if( PPP_Get( &(trayData->ppp[i]), ID_PARA_monsno, NULL  ) == 0 )
    {
      trayData->ppp[i] = *poke;
      //SaveData_RequestTotalSave();  //金銀で変更
//      BOXDAT_SetTrayUseBit( box, trayNum);
      return TRUE;
    }
  }

  return FALSE;
}
//------------------------------------------------------------------
/**
 * ボックス、位置を指定してポケモンデータ格納
 *
 * @param   box     ボックスデータポインタ
 * @param   trayNum   何番目のボックスに格納するか
 * @param   pos     ボックス内の位置
 * @param   poke    ポケモンデータ
 *
 * @retval  BOOL    TRUE=格納された／FALSE=空きが無い
 */
//------------------------------------------------------------------
BOOL BOXDAT_PutPokemonPos( BOX_MANAGER* box, u32 trayNum, u32 pos, const POKEMON_PASO_PARAM* poke )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  PPP_RecoverPP(poke);

  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

  if( ( trayNum < boxData->trayMax ) && ( pos < BOX_MAX_POS ) ){
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
    trayData->ppp[pos] = *poke;
    //SaveData_RequestTotalSave();  金銀で変更
//    BOXDAT_SetTrayUseBit( box, trayNum);
    return TRUE;
  }else{
    GF_ASSERT(0);
  }
  return FALSE;
}


//------------------------------------------------------------------
/**
 * 指定位置のポケモンを入れ替える
 *
 * @param   box     ボックスデータポインタ
 * @param   trayNum   何番目のボックスか
 * @param   pos1    ボックス内の位置
 * @param   pos2    ボックス内の位置
 *
 */
//------------------------------------------------------------------
void BOXDAT_ChangePokeData( BOX_MANAGER * box, u32 tray1, u32 pos1, u32 tray2, u32 pos2 )
{
  BOX_TRAY_DATA * tray1Data = BOXTRAYDAT_GetTrayData( box, tray1 );
  BOX_TRAY_DATA * tray2Data = BOXTRAYDAT_GetTrayData( box, tray2 );

  POKEMON_PASO_PARAM  tmp = tray1Data->ppp[pos1];
  tray1Data->ppp[pos1] = tray2Data->ppp[pos2];
  tray2Data->ppp[pos2] = tmp;
  //SaveData_RequestTotalSave();  金銀で変更
//  BOXDAT_SetTrayUseBit( box, tray1 );
//  BOXDAT_SetTrayUseBit( box, tray2 );
}

//------------------------------------------------------------------
/**
 * 位置を指定してボックスのポケモンデータクリア
 *
 * @param   box     ボックスデータポインタ
 * @param   trayNum   何番目のボックスか
 * @param   pos     ボックス内の位置
 *
 */
//------------------------------------------------------------------
void BOXDAT_ClearPokemon( BOX_MANAGER* box, u32 trayNum, u32 pos )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

  if( (pos < BOX_MAX_POS) && (trayNum < BOX_MAX_TRAY) )
  {
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
    PPP_Clear( &(trayData->ppp[pos]) );
    //SaveData_RequestTotalSave();  金銀で変更
//    BOXDAT_SetTrayUseBit( box, trayNum);
  }
  else
  {
    GF_ASSERT(0);
  }
}

//------------------------------------------------------------------
/**
 * カレントのトレイナンバーを返す
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32   カレントトレイナンバー
 */
//------------------------------------------------------------------
u32 BOXDAT_GetCureentTrayNumber( const BOX_MANAGER* box )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  return boxData->currentTrayNumber;
}

//------------------------------------------------------------------
/**
 * 開放されているトレイ数を返す
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32   トレイ数
 */
//------------------------------------------------------------------
u32 BOXDAT_GetTrayMax( const BOX_MANAGER * box )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  return boxData->trayMax;
}

//------------------------------------------------------------------
/**
 * トレイを開放する
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32   トレイ数
 */
//------------------------------------------------------------------
u32 BOXDAT_AddTrayMax( BOX_MANAGER * box )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);

  if( boxData->trayMax != BOX_MAX_TRAY ){
    boxData->trayMax += BOX_MIN_TRAY;
  }
  return boxData->trayMax;
}

//------------------------------------------------------------------
/**
 * １つでも空きのあるトレイナンバーを返す（カレントから検索開始する）
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32   空きのあるトレイナンバー／見つからなければ BOXDAT_TRAYNUM_ERROR
 */
//------------------------------------------------------------------
u32 BOXDAT_GetEmptyTrayNumber( const BOX_MANAGER* box )
{
  int tray, pos;
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);

  tray = boxData->currentTrayNumber;

  while(1)
  {
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,tray);
    for(pos=0; pos<BOX_MAX_POS; pos++)
    {
      if( PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[pos])), ID_PARA_poke_exist, NULL  ) == 0 )
      {
        return tray;
      }
    }
    if( ++tray >= boxData->trayMax )
    {
      tray = 0;
    }
    if( tray == boxData->currentTrayNumber )
    {
      break;
    }
  }

  // 開放されていないトレイをチェック
  if( boxData->trayMax != BOX_MAX_TRAY ){
    tray = boxData->trayMax;
    boxData->trayMax += BOX_MIN_TRAY;
    return tray;
  }

  return BOXDAT_TRAYNUM_ERROR;
}
//------------------------------------------------------------------
/**
 * 空きのあるトレイナンバーとトレイ内の位置を検出
 *
 * @param   box     [in] ボックスデータポインタ
 * @param   trayNum   [in|out] 検出開始トレイナンバーを指定→最初に空きが見つかったトレイナンバーが入る
 * @param   pos     [in|out] 検出開始位置を指定→最初に空きが見つかったトレイ内の位置が入る
 *
 * @retval  空きが見つかったらTRUE, 見つからなかったらFALSE
 *
 */
//------------------------------------------------------------------
BOOL BOXDAT_GetEmptyTrayNumberAndPos( const BOX_MANAGER* box, int* trayNum, int* pos )
{
  int t, p;
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);

  if( *trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    *trayNum = boxData->currentTrayNumber;
  }

  t = *trayNum;
  p = *pos;

  while(1)
  {
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,t);
    for( ; p<BOX_MAX_POS; p++)
    {
      if( PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[p])), ID_PARA_poke_exist, NULL  ) == 0 )
      {
        *trayNum = t;
        *pos = p;
        return TRUE;
      }
    }
    if( ++t >= boxData->trayMax )
    {
      t = 0;
    }
    if( t == (*trayNum) )
    {
      break;
    }
    p = 0;
  }

  // 開放されていないトレイをチェック
  if( boxData->trayMax != BOX_MAX_TRAY ){
    *trayNum = boxData->trayMax;
    *pos = 0;
    boxData->trayMax += BOX_MIN_TRAY;
    return TRUE;
  }

  return BOXDAT_TRAYNUM_ERROR;
}

//------------------------------------------------------------------
/**
 * 全体から空きの数をカウントして返す
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32   空きの数
 */
//------------------------------------------------------------------
u32 BOXDAT_GetEmptySpaceTotal( const BOX_MANAGER* box )
{
  int t, pos;
  u32 cnt;

  cnt = 0;

  for(t=0; t<BOX_MAX_TRAY; t++)
  {
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,t);
    for(pos=0; pos<BOX_MAX_POS; pos++)
    {
      if( PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[pos])), ID_PARA_poke_exist, NULL  ) == 0 )
      {
        cnt++;
      }
    }
  }

  return cnt;
}
//------------------------------------------------------------------
/**
 * 指定トレイから空きの数をカウントして返す
 *
 * @param   box   ボックスデータポインタ
 * @param   trayNum トレイナンバー
 *
 * @retval  u32   空きの数
 */
//------------------------------------------------------------------
u32 BOXDAT_GetEmptySpaceTray( const BOX_MANAGER* box, u32 trayNum )
{
  int pos;
  u32 cnt;
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  BOX_TRAY_DATA *trayData;

  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }
  GF_ASSERT( trayNum < BOX_MAX_TRAY );


  cnt = 0;

  trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
  for(pos=0; pos<BOX_MAX_POS; pos++)
  {
    if( PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[pos])), ID_PARA_poke_exist, NULL  ) == 0 )
    {
      cnt++;
    }
  }

  return cnt;
}
//------------------------------------------------------------------
/**
 * カレントのトレイナンバーをセット
 *
 * @param   box   ボックスデータポインタ
 * @param   num   カレントトレイナンバー
 */
//------------------------------------------------------------------
void BOXDAT_SetCureentTrayNumber( BOX_MANAGER* box, u32 num )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( num < BOX_MAX_TRAY )
  {
    boxData->currentTrayNumber = num;
    //SaveData_RequestTotalSave();  //金銀で削除　ポケモン以外のデータはいかなる場合も書くのでこの関数でリクエストしなくてＯＫ
  }
  else
  {
    GF_ASSERT(0);
  }
}

//------------------------------------------------------------------
/**
 * 指定トレイの壁紙ナンバーを返す
 *
 * @param   box       ボックスデータポインタ
 * @param   trayNumber    トレイナンバー
 *
 * @retval  u32   壁紙ナンバー
 */
//------------------------------------------------------------------
u32 BOXDAT_GetWallPaperNumber( const BOX_MANAGER* box, u32 trayNum )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNum < BOX_MAX_TRAY )
  {
    return boxData->wallPaper[trayNum];
  }
  else
  {
    GF_ASSERT(0);
    return 0;
  }
}

//------------------------------------------------------------------
/**
 * 金銀で有効な壁紙番号をチェック
 *
 * @param num   壁紙番号
 *
 * @retval  "TRUE = 有効"
 * @retval  "FALSE = 無効"
 */
//------------------------------------------------------------------
/*
static BOOL WallPaperNumberCheck( u32 num )
{
  if( num < BOX_NORMAL_WALLPAPER_MAX ||
    ( num >= BOX_TOTAL_WALLPAPER_MAX_PL && num < BOX_TOTAL_WALLPAPER_MAX_GS ) ){
    return TRUE;
  }
  return FALSE;
}
*/

//------------------------------------------------------------------
/**
 * 指定トレイの壁紙ナンバーをセット
 *
 * @param   box       ボックスデータポインタ
 * @param   trayNum     トレイナンバー
 * @param   wallPaperNumber 壁紙ナンバー
 *
 */
//------------------------------------------------------------------
void BOXDAT_SetWallPaperNumber( BOX_MANAGER* box, u32 trayNum, u32 wallPaperNumber )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

//  if( trayNum < BOX_MAX_TRAY && WallPaperNumberCheck(wallPaperNumber) == TRUE ){
  if( trayNum < BOX_MAX_TRAY ){
    boxData->wallPaper[trayNum] = wallPaperNumber;
    //SaveData_RequestTotalSave();  金銀で削除　ポケモン以外のデータはいかなる場合も書くのでこの関数でリクエストしなくてＯＫ
  }else{
    GF_ASSERT(0);
  }
}

//------------------------------------------------------------------
/**
 * 指定トレイの名前取得
 *
 * @param   box       ボックスデータポインタ
 * @param   trayNumber    トレイナンバー
 * @param   buf       文字列コピー先バッファ
 *
 */
//------------------------------------------------------------------
void BOXDAT_GetBoxName( const BOX_MANAGER* box, u32 trayNumber, STRBUF* buf )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNumber == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNumber = boxData->currentTrayNumber;
  }

  if( trayNumber < BOX_MAX_TRAY )
  {
    GFL_STR_SetStringCode( buf, boxData->trayName[trayNumber] );
  }
  else
  {
    GF_ASSERT(0);
  }
}

//------------------------------------------------------------------
/**
 * 指定トレイの名前設定
 *
 * @param   box       ボックスデータポインタ
 * @param   trayNumber    トレイナンバー
 * @param   src       名前
 *
 */
//------------------------------------------------------------------
void BOXDAT_SetBoxName( BOX_MANAGER* box, u32 trayNumber, const STRBUF* src )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNumber == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNumber = boxData->currentTrayNumber;
  }

  if( trayNumber < BOX_MAX_TRAY )
  {
    GFL_STR_GetStringCode( src, boxData->trayName[trayNumber], BOX_TRAYNAME_BUFSIZE );
    //SaveData_RequestTotalSave();  //金銀で削除　ポケモン以外のデータはいかなる場合も書くのでこの関数でリクエストしなくてＯＫ
  }
}
//------------------------------------------------------------------
/**
 * 指定トレイに格納されているポケモン数を返す
 *
 * @param   box       ボックスデータポインタ
 * @param   trayNumber    トレイナンバー
 *
 * @retval  u32   トレイに格納されているポケモン数
 */
//------------------------------------------------------------------
u32 BOXDAT_GetPokeExistCount( const BOX_MANAGER* box, u32 trayNum )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

  if( trayNum < BOX_MAX_TRAY )
  {
    int i;
    u32 cnt = 0;
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
    for(i=0; i<BOX_MAX_POS; i++)
    {
      if(PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[i])), ID_PARA_poke_exist, NULL ))
      {
        cnt++;
      }
    }
    return cnt;
  }
  else
  {
    GF_ASSERT(0);
  }
  return 0;
}

//------------------------------------------------------------------
/**
 * 指定トレイに格納されているポケモン数を返す(タマゴ除外)
 *
 * @param   box       ボックスデータポインタ
 * @param   trayNumber    トレイナンバー
 *
 * @retval  u32   トレイに格納されているポケモン数
 */
//------------------------------------------------------------------
u32 BOXDAT_GetPokeExistCount2( const BOX_MANAGER* box, u32 trayNum )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

  if( trayNum < BOX_MAX_TRAY )
  {
    int i;
    u32 cnt = 0;
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
    for(i=0; i<BOX_MAX_POS; i++)
    {
      if(PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[i])), ID_PARA_poke_exist, NULL ))
      {
        if(PPP_Get( (POKEMON_PASO_PARAM*)(&(trayData->ppp[i])), ID_PARA_tamago_flag, NULL ) == 0) //タマゴ除外
        {
          cnt++;
        }
      }
    }
    return cnt;
  }
  else
  {
    GF_ASSERT(0);
  }
  return 0;
}

//------------------------------------------------------------------
/**
 * ボックス全体に含まれるポケモン数を返す
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32
 */
//------------------------------------------------------------------
u32 BOXDAT_GetPokeExistCountTotal( const BOX_MANAGER* box )
{
  u32 cnt, i;

  for(i=0, cnt=0; i<BOX_MAX_TRAY; i++)
  {
    cnt += BOXDAT_GetPokeExistCount( box, i );
  }
  return cnt;
}

//------------------------------------------------------------------
/**
 * ボックス全体に含まれるポケモン数を返す(タマゴ除外版）
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32
 */
//------------------------------------------------------------------
u32 BOXDAT_GetPokeExistCount2Total( const BOX_MANAGER* box )
{
  u32 cnt, i;

  for(i=0, cnt=0; i<BOX_MAX_TRAY; i++)
  {
    cnt += BOXDAT_GetPokeExistCount2( box, i );
  }
  return cnt;
}

//------------------------------------------------------------------
/**
 * ボックス格納ポケモンに対するPPP_Get
 *
 * @param   box     ボックスデータポインタ
 * @param   trayNum   トレイナンバー
 * @param   pos     格納位置
 * @param   param   PPPGet パラメータ
 * @param   buf     PPPGet バッファ
 *
 * @retval  u32     PPPGet 戻り値
 */
//------------------------------------------------------------------
u32 BOXDAT_PokeParaGet( const BOX_MANAGER* box, u32 trayNum, u32 pos, int param, void* buf )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  BOX_TRAY_DATA *trayData;
  GF_ASSERT((trayNum<BOX_MAX_TRAY)||(trayNum == BOXDAT_TRAYNUM_CURRENT));
  GF_ASSERT(pos<BOX_MAX_POS);

  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }
  trayData = BOXTRAYDAT_GetTrayData(box,trayNum);

  return PPP_Get( (POKEMON_PASO_PARAM*)(&trayData->ppp[pos]), param, buf );
}


//------------------------------------------------------------------
/**
 * ボックス格納ポケモンに対するPPPPut
 *
 * @param   box     ボックスデータポインタ
 * @param   trayNum   トレイナンバー
 * @param   pos     格納位置
 * @param   param   PPPGet パラメータ
 * @param   buf     PPPGet バッファ
 *
 */
//------------------------------------------------------------------
void BOXDAT_PokeParaPut( BOX_MANAGER* box, u32 trayNum, u32 pos, int param, u32 arg )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  BOX_TRAY_DATA *trayData;
  GF_ASSERT((trayNum<BOX_MAX_TRAY)||(trayNum == BOXDAT_TRAYNUM_CURRENT));
  GF_ASSERT(pos<BOX_MAX_POS);

  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }

  trayData = BOXTRAYDAT_GetTrayData(box,trayNum);
  PPP_Put( (POKEMON_PASO_PARAM*)(&trayData->ppp[pos]), param, arg );
  //SaveData_RequestTotalSave();  //金銀で変更
//  BOXDAT_SetTrayUseBit( box, trayNum);

}










//------------------------------------------------------------------
/**
 * 指定位置のポケモンデータへのポインタを返す
 *（あまり使ってほしくはない）
 *
 * @param   box     ボックスデータポインタ
 * @param   boxNum    ボックスナンバー
 * @param   pos
 *
 * @retval  POKEMON_PASO_PARAM*
 */
//------------------------------------------------------------------
POKEMON_PASO_PARAM* BOXDAT_GetPokeDataAddress( const BOX_MANAGER* box, u32 trayNum, u32 pos )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  BOX_TRAY_DATA *trayData;
  GF_ASSERT( ((trayNum<BOX_MAX_TRAY)||(trayNum == BOXDAT_TRAYNUM_CURRENT)) );
  GF_ASSERT( (pos<BOX_MAX_POS) );

  if( trayNum == BOXDAT_TRAYNUM_CURRENT )
  {
    trayNum = boxData->currentTrayNumber;
  }
  trayData = BOXTRAYDAT_GetTrayData(box,trayNum);

  return (POKEMON_PASO_PARAM*) &(trayData->ppp[pos]);
}









//==============================================================================================
// だいすきクラブ壁紙
//==============================================================================================

//------------------------------------------------------------------
/**
 * だいすきクラブ壁紙を１枚、有効にする
 *
 * @param   box     ボックスデータポインタ
 * @param   number    有効にする壁紙ナンバー
 *
 */
//------------------------------------------------------------------
void BOXDAT_SetDaisukiKabegamiFlag( BOX_MANAGER* box, u32 number )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  GF_ASSERT( number < BOX_EX_WALLPAPER_MAX );

  boxData->daisukiBitFlag |= (1 << number);
/// SaveData_RequestTotalSave();金銀で削除　ポケモン以外のデータはいかなる場合も書くのでこの関数でリクエストしなくてＯＫ
}

//------------------------------------------------------------------
/**
 * だいすきクラブ壁紙を取得しているかチェック
 *
 * @param   box     ボックスデータポインタ
 * @param   number    チェックする壁紙ナンバー
 *
 * @retval  BOOL    TRUEで取得している
 */
//------------------------------------------------------------------
BOOL BOXDAT_GetDaisukiKabegamiFlag( const BOX_MANAGER* box, u32 number )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  GF_ASSERT( number < BOX_EX_WALLPAPER_MAX );

  return (boxData->daisukiBitFlag & (1<<number)) != 0;
}

//------------------------------------------------------------------
/**
 * 取得しただいすきクラブ壁紙の総数を取得
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32
 */
//------------------------------------------------------------------
u32 BOXDAT_GetDaiukiKabegamiCount( const BOX_MANAGER* box )
{
  u32 i, cnt;

  for(i=0, cnt=0; i<BOX_EX_WALLPAPER_MAX; i++)
  {
    if( BOXDAT_GetDaisukiKabegamiFlag(box, i) )
    {
      cnt++;
    }
  }
  return cnt;
}

//------------------------------------------------------------------
/**
 * 編集を行ったトレーのビットを立てる
 *
 * @param   box   ボックスデータポインタ
 * @param inTrayIdx トレーインデックス
 *
 * @retval  none
 */
//------------------------------------------------------------------
/*
void BOXDAT_SetTrayUseBit(BOX_MANAGER* box, const u8 inTrayIdx)
{
  u8 bit;
  u32 data = 0;
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  if (inTrayIdx >= BOX_MAX_TRAY){
    GF_ASSERT_MSG(0,"ボックストレーインデックス不正\n");
    return;
  }

  bit = (0b1);
  data = bit << inTrayIdx;

  boxData->UseBoxBits |= data;
}
*/

//------------------------------------------------------------------
/**
 * トレーの全ビットを立てる
 *
 * @param   box   ボックスデータポインタ
 * @param inTrayIdx トレーインデックス
 *
 * @retval  none
 */
//------------------------------------------------------------------
/*
void BOXDAT_SetTrayUseBitAll(BOX_MANAGER* box)
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  boxData->UseBoxBits = TRAY_ALL_USE_BIT;
  OS_Printf("全ビットオン%x\n",boxData->UseBoxBits);
}
*/

//------------------------------------------------------------------
/**
 * トレーの全ビットを立てる
 *
 * @param   box   ボックスデータポインタ
 * @param inTrayIdx トレーインデックス
 *
 * @retval  none
 */
//------------------------------------------------------------------
/*
void BOXDAT_ClearTrayUseBits(BOX_MANAGER* box)
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  boxData->UseBoxBits = 0;
  OS_Printf("トレー編集ビット全クリア\n");
}
*/

//------------------------------------------------------------------
/**
 * トレーの編集ビット群を取得
 *
 * @param   box   ボックスデータポインタ
 *
 * @retval  u32   編集ビット群
 */
//------------------------------------------------------------------
/*
u32 BOXDAT_GetTrayUseBits(const BOX_MANAGER* box)
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  return boxData->UseBoxBits;
}
*/

//------------------------------------------------------------------
/**
 * 1ボックスのポケモンデータサイズを取得
 *
 *
 * @retval  u32   データサイズ
 */
//------------------------------------------------------------------
u32 BOXDAT_GetOneBoxDataSize(void)
{
  return  sizeof(BOX_TRAY_DATA);
}

//------------------------------------------------------------------
/**
 * 18ボックスのポケモンデータ以外のダミー１６バイトの中身をチェックする
 *
 *
 */
//------------------------------------------------------------------
void BOXDAT_CheckBoxDummyData(BOX_MANAGER* box)
{
  //ダミーは無くしました。
  /*
  u8 i,d;
  for(i = 0; i < BOX_MAX_TRAY; i++){
    BOX_TRAY_DATA *trayData = BOXTRAYDAT_GetTrayData(box,i);
    for(d = 0; d < TRAY_DUMMY_NUM; d++){
      GF_ASSERT( trayData->dummy[d] == 0 );
      trayData->dummy[d] = 0;
    }
  }
  */

}

BOX_MANAGER * BOX_DAT_InitManager( const HEAPID heapId , SAVE_CONTROL_WORK * sv)
{
  BOX_MANAGER* pData;
  pData = GFL_HEAP_AllocMemory( heapId , sizeof( BOX_MANAGER ) );
  pData->sv = sv;
  {
    BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(pData);
    BOXDAT_Init(boxData);
  }
  return pData;

}
void BOX_DAT_ExitManager( BOX_MANAGER *box )
{
  GFL_HEAP_FreeMemory( box );
}


//---------------------------------------------------------------------------
/**
 * @brief ボックスデータのセット(1ボックス単位(30匹))
 * @param trayIdx   ボックス番号
 * @param dataPtr   保存するデータのポインタ
 * @param boxData   ボックスセーブデータへのポインタ
 */
//---------------------------------------------------------------------------
void BOXDAT_SetPPPData_Tray( u8 trayIdx , void *dataPtr , BOX_MANAGER *box )
{
  BOX_SAVEDATA *boxData = BOXDAT_GetBoxSaveData(box);
  //通信用に切り分け、解読シーケンスをはさむか？
  // 将来作り直す必要がある
//  GFL_STD_MemCopy( dataPtr , (void*)boxData->ppp[trayIdx] , 0x88*30 );

}

//==============================================================================================
// ボックスデータ分割処理
//==============================================================================================

void BOXTRAYDAT_Init( BOX_TRAY_DATA* traydat )
{
  u32 p;
  for(p = 0; p < BOX_MAX_POS; p++)
  {
    PPP_Clear( &(traydat->ppp[p]) );
  }
}

u32 BOXTRAYDAT_GetTotalSize( void )
{
  return sizeof( BOX_TRAY_DATA );
}

static BOX_TRAY_DATA* BOXTRAYDAT_GetTrayData( const BOX_MANAGER *boxData , const u32 trayNum )
{
  return SaveControl_DataPtrGet(boxData->sv, GMDATA_ID_BOXTRAY_01+trayNum);
}
