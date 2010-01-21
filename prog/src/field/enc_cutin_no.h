//======================================================================
/*
 * @file	enc_cutin_no.h
 * @brief	エンカウントカットインナンバー
 * @author saito
 */
//======================================================================

#pragma once

typedef enum{
  ENC_CUTIN_RIVAL,
  ENC_CUTIN_SUPPORT,
  ENC_CUTIN_GYM1A,
  ENC_CUTIN_GYM1B,
  ENC_CUTIN_GYM1C,
  ENC_CUTIN_GYM2,
  ENC_CUTIN_GYM3,
  ENC_CUTIN_GYM4,
  ENC_CUTIN_GYM5,
  ENC_CUTIN_GYM6,
  ENC_CUTIN_GYM7,
  ENC_CUTIN_GYM8A,
  ENC_CUTIN_GYM8B,
  ENC_CUTIN_BIGFOUR1,
  ENC_CUTIN_BIGFOUR2,
  ENC_CUTIN_BIGFOUR3,
  ENC_CUTIN_BIGFOUR4,
  ENC_CUTIN_CHAMP,
  ENC_CUTIN_BOSS,
  ENC_CUTIN_SAGE,
  ENC_CUTIN_PLASMA,
}ENC_CUTIN_NO;

typedef enum {
  GRA_TRANS_NONE,   //転送なし
  GRA_TRANS_ALL,    //自分、相手、ともに転送
  GRA_TRANS_ENEMY,  //相手のみ転送
}GRA_TRANS_TYPE;

typedef struct ENC_CUTIN_DAT_tag
{
  int CutinNo;      //カットインのナンバー
  int ChrArcIdx;   //相手側のキャラ転送データアーカイブインデックス
  int PltArcIdx;    //相手側のパレット転送データアーカイブインデックス
  int MsgIdx;       //相手側の名前データインデックス
  GRA_TRANS_TYPE TransType;    //転送データの種類
}ENC_CUTIN_DAT;

/**
 * ※カットイン用トレーナー名メッセージデータの並びと
 * 　転送画像の並びは合わせるようにする
 *
 * 　転送画像データアーカイブはキャラデータ、パレットデータの順番で一組にして、順番をつける
 *
*/

extern const ENC_CUTIN_DAT *ENC_CUTIN_NO_GetDat(const inEncCutinNo);

