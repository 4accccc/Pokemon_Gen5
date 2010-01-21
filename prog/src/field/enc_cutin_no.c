//======================================================================
/**
 * @file	enc_cutin_no.c
 * @brief	エンカウントカットインナンバー
 * @author	Saito
 *
 */
//======================================================================

#include "enc_cutin_no.h"
#include "msg/msg_sptr_name.h"
#include "../../../resource/fld3d_ci/fldci_id_def.h"

static const ENC_CUTIN_DAT EncCutinDat[] = 
{
  {FLDCIID_RIVAL, 0, 0, SPTR_RIVAL, GRA_TRANS_ALL}, //ライバル
  {FLDCIID_RIVAL, 0, 0, SPTR_SUPPORT, GRA_TRANS_ALL}, //サポート
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM01A, GRA_TRANS_ALL}, //ジムリーダー01A
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM01B, GRA_TRANS_ALL}, //ジムリーダー01B
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM01C, GRA_TRANS_ALL}, //ジムリーダー01C
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM02, GRA_TRANS_ALL}, //ジムリーダー02
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM03, GRA_TRANS_ALL}, //ジムリーダー03
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM04, GRA_TRANS_ALL}, //ジムリーダー04
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM05, GRA_TRANS_ALL}, //ジムリーダー05
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM06, GRA_TRANS_ALL}, //ジムリーダー06
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM07, GRA_TRANS_ALL}, //ジムリーダー07
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM08A, GRA_TRANS_ALL}, //ジムリーダー08A
  {FLDCIID_RIVAL, 0, 0, SPTR_GYM08B, GRA_TRANS_ALL}, //ジムリーダー08B
  {FLDCIID_RIVAL, 0, 0, SPTR_BIGFOUR01, GRA_TRANS_ALL}, //四天王1
  {FLDCIID_RIVAL, 0, 0, SPTR_BIGFOUR02, GRA_TRANS_ALL}, //四天王2
  {FLDCIID_RIVAL, 0, 0, SPTR_BIGFOUR03, GRA_TRANS_ALL}, //四天王3
  {FLDCIID_RIVAL, 0, 0, SPTR_BIGFOUR04, GRA_TRANS_ALL}, //四天王4
  {FLDCIID_RIVAL, 0, 0, SPTR_CHAMP, GRA_TRANS_ALL}, //チャンプ
  {FLDCIID_RIVAL, 0, 0, SPTR_BOSS, GRA_TRANS_ALL}, //ボス
  {FLDCIID_RIVAL, 0, 0, SPTR_SAGE, GRA_TRANS_ALL}, //セージ
  {0, 0, 0, 0, GRA_TRANS_NONE}, //プラズマ団
};

const ENC_CUTIN_DAT *ENC_CUTIN_NO_GetDat(const inEncCutinNo)
{
  return &EncCutinDat[inEncCutinNo];
}
