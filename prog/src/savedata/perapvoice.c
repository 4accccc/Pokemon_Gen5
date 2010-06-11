//============================================================================================
/**
 * @file  perapvoice.c
 * @brief ペラップ録音データ保存情報
 * @author AkitoMori
 * @date  2010.04.13
 *
 * @note ペラップデータ圧縮 ( WB版 2010.04.13 by mori )<br>
 * 
 * WBでは 符号付き8bit 8180hz,1秒で録音するため8180バイトの録音データが生成される。<br>
 * ただセーブデータで押さえてあるのは昔と同じ1000バイトのため、<br>
 * 1/8に縮める必要がある<br>
 * 
 * 第１段階<br>
 * 8180のデータを全部参照するわけにはいかないので、8個につき２個だけ参照するようにする。<br>
 * 正確には８個の中の0番目と4番目のデータだけ取り出す。これで４分の１<br>
 * 
 * 第２段階<br>
 * 符号付き8bitのデータを縮める。符号付き8bitは-128～127の値を取るのでこのデータをまずは<br>
 * 16で割る(-8～7になる)。その後このデータに８を足して0-15のデータに変換する。<br>
 * 4bitでデータを表現できるようになったので、1btyeに２つずつ詰め込む<br>
 * これで２分の１<br>
 * 
 * 180バイト分だけは無視することにする。<br>
 * 1/4と1/2のデータを格納して8180→1000バイト圧縮の出来上がり。<br>
 * 
 * 展開は逆で。

 */
//============================================================================================

#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/perapvoice.h"
#include "perapvoice_local.h"
#include "poke_tool/monsno_def.h"

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief オウムポケモンの声データセーブワークの定義
 */
//----------------------------------------------------------
struct PERAPVOICE {
  BOOL exist_flag;                  ///< 「おしゃべり」で録音をしたので声データが存在する( TRUE or FALSE )
  u8 voicedata[PERAPVOICE_LENGTH];  ///< 録音した声データ
};

//============================================================================================
//
//  セーブデータシステムが依存する関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief ぺラップの声ワークのサイズ取得
 * @return  int   サイズ（バイト単位）
 */
//----------------------------------------------------------
int PERAPVOICE_GetWorkSize(void)
{
  return sizeof(PERAPVOICE);
}

//----------------------------------------------------------
/**
 * @brief ぺラップの声ワークの初期化
 * @param pv    PERAPVOICEデータへのポインタ
 */
//----------------------------------------------------------
void PERAPVOICE_Init(PERAPVOICE * pv)
{
  MI_CpuClear32(pv, sizeof(PERAPVOICE));
  pv->exist_flag = FALSE;
}

//----------------------------------------------------------
/**
 * @brief ぺラップの声ワークの初期化
 * @param heapID    メモリ確保をおこなうヒープ指定
 * @return  PERAPVOICE  取得したワークへのポインタ
 */
//----------------------------------------------------------
PERAPVOICE * PERAPVOICE_AllocWork(HEAPID heapID)
{
  PERAPVOICE *pv = GFL_HEAP_AllocClearMemory(heapID, sizeof(PERAPVOICE));
  PERAPVOICE_Init(pv);
  return pv;
}

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief ぺラップの声セーブデータの取得
 * @param sv    セーブデータへのポインタ
 * @return  PERAPVOICEデータへのポインタ
 */
//----------------------------------------------------------
PERAPVOICE * SaveData_GetPerapVoice(SAVE_CONTROL_WORK * sv)
{
//  OS_Printf("1:perapvoice adr=%08x\n",(u32)SaveControl_DataPtrGet(sv, GMDATA_ID_PERAPVOICE));
  return (PERAPVOICE*)SaveControl_DataPtrGet(sv, GMDATA_ID_PERAPVOICE);
}

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief 声データが存在するかどうかのチェック
 * @param pv    声データへのポインタ
 * @return  BOOL  TRUE：存在する、FALSE：存在しない
 */
//----------------------------------------------------------
BOOL PERAPVOICE_GetExistFlag(const PERAPVOICE * pv)
{
  return pv->exist_flag;
}

//==============================================================================
/**
 * @brief   声データの存在フラグを落とす
 *
 * @param   pv    声データへのポインタ
 *
 * @retval  none    
 */
//==============================================================================
void PERAPVOICE_ClearExistFlag( PERAPVOICE * pv )
{
  pv->exist_flag = FALSE;
}

//==============================================================================
/**
 * @brief   声データの存在フラグを立てる
 *
 * @param   pv    声データへのポインタ
 *
 * @retval  none    
 */
//==============================================================================
void PERAPVOICE_SetExistFlag( PERAPVOICE * pv )
{
  pv->exist_flag = TRUE;
}

//----------------------------------------------------------
/**
 * @brief 声データの取得
 * @param pv    声データへのポインタ
 * @return  声データへのポインタ
 */
//----------------------------------------------------------
const void * PERAPVOICE_GetVoiceData(const PERAPVOICE * pv)
{
  return pv->voicedata;
}


#define VOICE_REST_START  ( PERAPVOICE_LENGTH*8 )
#define VOICE_REST_END    ( 8180 )    // TWLでできるサンプリングレートと同じ
//==============================================================================
/**
 * @brief   声データの展開
 *
 * @param   des   展開先
 * @param   src   ペラップボイスのポインタ
 *
 * @retval  none    
 *
 * 06/04/15時点ではペラップボイスは4bit,2khz,1秒サンプリング=1k<br>
 * このデータを8bitに伸ばして2kにして格納する<br>
 * <br>
 * 10/04/11時点ではペラップボイスは4bit,8khz/4,1秒サンプリング=1k<br>
 * このデータを8bitに伸ばして2khz→8180バイト分格納する<br>
 */
//==============================================================================
void PERAPVOICE_ExpandVoiceData( s8 *des, const s8 *src )
{
  int i,j,count=0;
  s8 tmp;
  u8 dat;

//  OS_Printf("src=%08x, des=%08x\n", src, des);

  // 4bit1kbyteを8bit8kbyteに伸ばす
  for(i=0;i<PERAPVOICE_LENGTH;i++){
    dat = src[i]&0x0f;        // 下位データを取り出して
    tmp = dat-8;              // 8引いて-8～7のデータにして
    tmp = tmp*16;             // 16掛けて-128～127のデータにする
    for(j=0;j<4;j++){
      des[count+j]   = tmp;  // 4回同じデータを格納
    }

    dat = src[i]>>4;          // 上位のデータを取り出して
    tmp = dat-8;              // 上と同じ計算を繰り返す
    tmp = tmp*16;
    for(j=4;j<8;j++){
      des[count+j]   = tmp;   // 4回同じデータを格納
    }
    count += 8;
  }

  // 1000バイトを8000バイトに展開しても180余りがでてしまうので、
  // 無音で書き潰す
  for(i=VOICE_REST_START;i<VOICE_REST_END;i++){
    des[i] = 0;
  }
}

//----------------------------------------------------------
/**
 * @brief 声データのセット
 * @param pv    声データへのポインタ
 * @param src   セットするデータへのポインタ
 *
 * 06/04/15時点ではペラップボイスは8bit,2khz,1秒サンプリング=2k<br>
 * このデータを4bitに縮めて1kにして格納する<br>
 * 10/04/10時点ではペラップボイスは8bit,8180hz,1秒サンプリング=8180byte<br>
 * このデータを4bitかつ8回に2個取り出すように縮めて1kにして格納する<br>
 *
 */
//----------------------------------------------------------
void PERAPVOICE_SetVoiceData(PERAPVOICE * pv, const s8 * src)
{
  int i,count;
  s8 tmp;
  u8 dat;

  // 格納フラグを立てる
  pv->exist_flag = TRUE;

  // 8180hz8bitを4bit1000バイトにして格納
  count = 0;
  for(i=0;i<PERAPVOICE_LENGTH*8;i+=8){
    tmp = (src[i]/16);                // 8バイトの中の0バイト目を取り出して16で割る
    dat = tmp+8;                      // -8～7のデータに8足して0～15にする
    pv->voicedata[count]   = dat;     // 下位データとして格納 

    tmp = (src[i+4]/16);              // 8バイトの中の4バイト目を取り出して１６で割る
    dat = tmp+8;                      // -8～7のデータに8足して0～15にする
    pv->voicedata[count] |= (dat<<4); // 下位データとして格納 
    count++;
  }

}

//==============================================================================
/**
 * @brief   声データのコピー
 *
 * @param   des   コピー先声データのポインタ
 * @param   src   コピー元声データのポインタ
 *
 * @retval  none  
 */
//==============================================================================
void PERAPVOICE_CopyData(PERAPVOICE * des, const PERAPVOICE * src)
{
  GFL_STD_MemCopy(src, des, sizeof(PERAPVOICE));
}


//=============================================================================================
/**
 * @brief ペラップの声データを削除するかPOKEPARTYをチェックして削除
 *
 * @param   pv      ペラップボイスのセーブデータポインタ
 * @param   party   POKEPARTYのポインタ（手持ちのみでOK、バトルボックスはチェックの必要なし)
 */
//=============================================================================================
void PERAPVOICE_CheckPerapInPokeParty( PERAPVOICE *pv, POKEPARTY *party )
{
  int i,num;
  GF_ASSERT( pv );
  GF_ASSERT( party );

  // 手持ちパーティにペラップがいるかチェックする
  num = PokeParty_GetPokeCount(party);
  for(i=0;i<num;i++){
    if(PP_Get( PokeParty_GetMemberPointer(party,i), ID_PARA_monsno, NULL)==MONSNO_PERAPPU){
      return;
    }
  }

  // ペラップが手持ちにいなかったら声データ削除
  PERAPVOICE_ClearExistFlag( pv );
}

// 「おしゃべり」のこんらん確率を取得するための配列参照位置
// この添字を参照している根拠は何も無い
#define PERAPVOICE_WAZA_PARAM0_INDEX ( 100 )
#define PERAPVOICE_WAZA_PARAM1_INDEX ( 500 )
#define PERAPVOICE_WAZA_PARAM2_INDEX ( 700 )
//--------------------------------------------------------------
/**
 * @brief 技のパラメータを取得
 *
 * @param perap   ぺラップデータのポインタ
 *
 * @retval  パラメータ= 0～2 録音してなかったら必ず0
 */
//--------------------------------------------------------------
int PERAPVOICE_GetWazaParam( PERAPVOICE* pv )
{
  u8 num;

  //声データが存在するかどうかのチェック
  if( PERAPVOICE_GetExistFlag(pv) == TRUE ){
    //要素[100][500][700]を見てパラメータを決定する
    num = pv->voicedata[PERAPVOICE_WAZA_PARAM0_INDEX]
        ^ pv->voicedata[PERAPVOICE_WAZA_PARAM1_INDEX]
        ^ pv->voicedata[PERAPVOICE_WAZA_PARAM2_INDEX];
//    OS_Printf( "voicedata[100] = %d\n", num1 );
//    OS_Printf( "voicedata[500] = %d\n", num2 );
//    OS_Printf( "voicedata[700] = %d\n", num3 );

    if(num>=0 && num<100){
      return 1;
    }else if(num>=150 && num<256){
      return 2;
    }
  }
  return 0;
}
