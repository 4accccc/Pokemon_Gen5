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
#include "../../arc/enc_ci_face.naix"

static const ENC_CUTIN_DAT EncCutinDat[] = 
{
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_rival_lz_bin, NARC_enc_ci_face_enc_rival_nclr, SPTR_RIVAL, GRA_TRANS_ALL}, //ライバル
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_support_lz_bin, NARC_enc_ci_face_enc_support_nclr, SPTR_SUPPORT, GRA_TRANS_ALL}, //サポート
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym01a_lz_bin, NARC_enc_ci_face_enc_gym01a_nclr, SPTR_GYM01A, GRA_TRANS_ALL}, //ジムリーダー01A
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym01b_lz_bin, NARC_enc_ci_face_enc_gym01b_nclr, SPTR_GYM01B, GRA_TRANS_ALL}, //ジムリーダー01B
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym01c_lz_bin, NARC_enc_ci_face_enc_gym01c_nclr, SPTR_GYM01C, GRA_TRANS_ALL}, //ジムリーダー01C
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym02_lz_bin, NARC_enc_ci_face_enc_gym02_nclr, SPTR_GYM02, GRA_TRANS_ALL}, //ジムリーダー02
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym03_lz_bin, NARC_enc_ci_face_enc_gym03_nclr, SPTR_GYM03, GRA_TRANS_ALL}, //ジムリーダー03
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym04_lz_bin, NARC_enc_ci_face_enc_gym04_nclr, SPTR_GYM04, GRA_TRANS_ALL}, //ジムリーダー04
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym05_lz_bin, NARC_enc_ci_face_enc_gym05_nclr, SPTR_GYM05, GRA_TRANS_ALL}, //ジムリーダー05
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym06_lz_bin, NARC_enc_ci_face_enc_gym06_nclr, SPTR_GYM06, GRA_TRANS_ALL}, //ジムリーダー06
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym07_lz_bin, NARC_enc_ci_face_enc_gym07_nclr, SPTR_GYM07, GRA_TRANS_ALL}, //ジムリーダー07
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym08a_lz_bin, NARC_enc_ci_face_enc_gym08a_nclr, SPTR_GYM08A, GRA_TRANS_ALL}, //ジムリーダー08A
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym08b_lz_bin, NARC_enc_ci_face_enc_gym08b_nclr, SPTR_GYM08B, GRA_TRANS_ALL}, //ジムリーダー08B
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_bigfour01_lz_bin, NARC_enc_ci_face_enc_bigfour01_nclr, SPTR_BIGFOUR01, GRA_TRANS_ALL}, //四天王1
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_bigfour02_lz_bin, NARC_enc_ci_face_enc_bigfour02_nclr, SPTR_BIGFOUR02, GRA_TRANS_ALL}, //四天王2
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_bigfour03_lz_bin, NARC_enc_ci_face_enc_bigfour03_nclr, SPTR_BIGFOUR03, GRA_TRANS_ALL}, //四天王3
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_bigfour04_lz_bin, NARC_enc_ci_face_enc_bigfour04_nclr, SPTR_BIGFOUR04, GRA_TRANS_ALL}, //四天王4
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_champ_lz_bin, NARC_enc_ci_face_enc_champ_nclr, SPTR_CHAMP, GRA_TRANS_ALL}, //チャンプ
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_boss_lz_bin, NARC_enc_ci_face_enc_boss_nclr, SPTR_BOSS, GRA_TRANS_ALL}, //ボス
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_sage_lz_bin, NARC_enc_ci_face_enc_sage_nclr, SPTR_SAGE, GRA_TRANS_ALL}, //セージ
  {0, 0, 0, 0, GRA_TRANS_NONE}, //プラズマ団
};

const ENC_CUTIN_DAT *ENC_CUTIN_NO_GetDat(const inEncCutinNo)
{
  return &EncCutinDat[inEncCutinNo];
}
