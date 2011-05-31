//============================================================================================
/**
 * @file  pm_wb_voice.c
 * @brief ポケモンＷＢ用鳴き声再生コールバック関数
 * @author  
 * @date  
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"

#include "savedata/save_control.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "sound/swan_sound_data.sadl"   //サウンドラベルファイル

#include "sound/pm_wb_voice.h"
#include "arc_def.h"

//==============================================================================================
//
//  構造体定義
//
//==============================================================================================

typedef struct
{ 
  u16 frequency;
  u16 volume;
}POKEMON_VOICE_PARAM;

//==============================================================================================
//
//  サンプリングデータ用定義
//
//==============================================================================================
//通常
#define VOICE_VOLUME_DEFAULT  (120)     //seq設定(sound_data.sarc)内(PV_VOL)を参照
#define VOICE_RATE_DEFAULT    (13379)   //サンプリングレート

// ぺラップ
#define PERAP_SAMPLING_RATE   (8180)    //サンプリングレート
#define PERAP_SAMPLING_TIME   (1)       //サンプリング時間
#define PERAP_SAMPLING_SIZE   (PERAP_SAMPLING_RATE * PERAP_SAMPLING_TIME) //データ量

#define PERAP_WAVE_SPD        (32768)   //再生スピード(１倍速)
#define PERAP_WAVE_SPDRAND    (8192)    //再生スピードのランダム
extern PERAPVOICE * SaveData_GetPerapVoice(SAVE_CONTROL_WORK * sv);

//フォルム違いで鳴き声が違うもの
typedef struct PMVOICE_FORM_PARAM{
  u16 monsno;
  u16 form;
  u16 wave_idx;
}PMVOICE_FORM_PARAM;


//============================================================================================
/**
 *
 *
 * @brief 波形ＩＤＸ取得
 *
 *
 */
//============================================================================================
void PMV_GetVoiceWaveIdx( u32 pokeNo,     // [in]ポケモンナンバー
              u32 pokeFormNo,   // [in]ポケモンフォームナンバー
              u32* waveIdx )    // [out]波形IDX
{
  static const PMVOICE_FORM_PARAM DATA_PMVoiceFormTbl[] = {
    { MONSNO_SHEIMI,  FORMNO_SHEIMI_SKY, WAVE_ARC_PV492_SKY }, //シェイミスカイフォルム
    { MONSNO_KYUREMU, FORMNO_KYUREMU_WHITE, WAVE_ARC_PV653_WHITE }, //キュレム　レシラム合体フォルム
    { MONSNO_KYUREMU, FORMNO_KYUREMU_BLACK, WAVE_ARC_PV653_BLACK }, //キュレム　ゼクロム合体フォルム
  };
  int i;

  if(( pokeNo < MONSNO_HUSIGIDANE)||( pokeNo > MONSNO_END) ){
    //指定範囲外
//    *waveIdx = PMVOICE_POKE001;
    *waveIdx = MONSNO_ZIBAKOIRU; // 仮で範囲外の泣き声をジバコイルにした 2011.04.28 by mori
    return;
  }

  for( i = 0;i < NELEMS(DATA_PMVoiceFormTbl);i++){
    const PMVOICE_FORM_PARAM* prm = &DATA_PMVoiceFormTbl[i];

    if( prm->monsno == pokeNo && pokeFormNo == prm->form ){
      *waveIdx = prm->wave_idx;
      return;
    }
  }
  // 波形ＩＤＸ取得
  *waveIdx = (pokeNo-1) + PMVOICE_POKE001;  // pokeNo(1origin)を変換。PMVOICE_POKE001～
}

//============================================================================================
/**
 *
 *
 * @brief 波形データカスタマイズ
 *
 *
 */
//============================================================================================
BOOL PMV_CustomVoiceWave( u32     pokeNo,     // [in]ポケモンナンバー
                          u32     pokeFormNo, // [in]ポケモンフォームナンバー
                          u32     userParam,  // [in]ユーザーパラメーター
                          void**  wave,       // [out]波形データ
                          u32*    size,       // [out]波形サイズ(MAX 26000)
                          int*    rate,       // [out]波形再生レート
                          int*    speed,      // [out]波形再生スピード
                          s8*     volume)     // [out]再生ボリューム
{
  PMV_REF* pmvRef;
  POKEMON_VOICE_PARAM  pvp;

  if(( pokeNo < MONSNO_HUSIGIDANE)||( pokeNo > MONSNO_END) ){
    //指定範囲外
    OS_Printf("ジバコイルの泣き声\n");
    pokeNo = MONSNO_ZIBAKOIRU;
  }


  //ボリューム＆サンプリングレート設定
  //  *volume = VOICE_VOLUME_DEFAULT;
  //  *rate   = VOICE_RATE_DEFAULT;
  GFL_ARC_LoadData( &pvp, ARCID_POKEVOICE_PARAM, pokeNo );
  *volume = pvp.volume;
  //*rate   = pvp.frequency;

  if( userParam == NULL ){ return FALSE; }  // 既存waveのみ使用

  //参照ユーザーパラメータ設定
  pmvRef = (PMV_REF*)userParam;

  if(( pokeNo == MONSNO_PERAPPU )&&( pokeFormNo == 0 )){  //ぺラップ判定
    //録音データ存在判定
    if( PERAPVOICE_GetExistFlag(pmvRef->perapVoice) == TRUE ){
      //録音データ展開
      PERAPVOICE_ExpandVoiceData( *wave, PERAPVOICE_GetVoiceData(pmvRef->perapVoice) );
      *size = PERAP_SAMPLING_SIZE;
      *rate = PERAP_SAMPLING_RATE;
      *volume = VOICE_VOLUME_DEFAULT;
      //ランダムに音程を変える仕様らしい
      *speed = PERAP_WAVE_SPD + GFUser_GetPublicRand( PERAP_WAVE_SPDRAND );
      return TRUE;
    }
  }
  return FALSE;
}

//============================================================================================
/**
 *
 *
 * @brief 参照データ作成（自分のぺラップ用データより）
 *
 *
 */
//============================================================================================
void PMV_MakeRefDataMine( PMV_REF* pmvRef )
{
  //ぺラップの録音データステータスポインタをセーブデータより取得
  pmvRef->perapVoice = SaveData_GetPerapVoice(SaveControl_GetPointer());
}







