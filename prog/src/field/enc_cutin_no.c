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

//TODO エンカウントカットインで使用する名前を整理する（sptr_name.gmmにメッセージデータが用意されている)
static const ENC_CUTIN_DAT EncCutinDat[] = 
{
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_rival_01_LZ_bin, NARC_enc_ci_face_enc_rival_02_LZ_bin , NARC_enc_ci_face_enc_rival_nclr, SPTR_RIVAL, GRA_TRANS_ALL ,CI_PANEL_4,0x7fff,0x0000}, //ライバル
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_support_01_LZ_bin, NARC_enc_ci_face_enc_support_02_LZ_bin, NARC_enc_ci_face_enc_support_nclr, SPTR_SUPPORT, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //サポート
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym01a_01_LZ_bin, NARC_enc_ci_face_enc_gym01a_02_LZ_bin , NARC_enc_ci_face_enc_gym01a_nclr, SPTR_GYM01A, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー01A
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym01b_01_LZ_bin, NARC_enc_ci_face_enc_gym01b_02_LZ_bin, NARC_enc_ci_face_enc_gym01b_nclr, SPTR_GYM01B, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー01B
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym01c_01_LZ_bin, NARC_enc_ci_face_enc_gym01c_02_LZ_bin, NARC_enc_ci_face_enc_gym01c_nclr, SPTR_GYM01C, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー01C
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym02_01_LZ_bin, NARC_enc_ci_face_enc_gym02_02_LZ_bin, NARC_enc_ci_face_enc_gym02_nclr, SPTR_GYM02, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー02
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym03_01_LZ_bin, NARC_enc_ci_face_enc_gym03_02_LZ_bin, NARC_enc_ci_face_enc_gym03_nclr, SPTR_GYM03, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー03
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym04_01_LZ_bin, NARC_enc_ci_face_enc_gym04_02_LZ_bin, NARC_enc_ci_face_enc_gym04_nclr, SPTR_GYM04, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x5294}, //ジムリーダー04
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym05_01_LZ_bin, NARC_enc_ci_face_enc_gym05_02_LZ_bin, NARC_enc_ci_face_enc_gym05_nclr, SPTR_GYM05, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー05
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym06_01_LZ_bin, NARC_enc_ci_face_enc_gym06_02_LZ_bin, NARC_enc_ci_face_enc_gym06_nclr, SPTR_GYM06, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー06
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym07_01_LZ_bin, NARC_enc_ci_face_enc_gym07_02_LZ_bin, NARC_enc_ci_face_enc_gym07_nclr, SPTR_GYM07, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー07
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym08a_01_LZ_bin, NARC_enc_ci_face_enc_gym08a_02_LZ_bin, NARC_enc_ci_face_enc_gym08a_nclr, SPTR_GYM08A, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー08A
  {FLDCIID_RIVAL, NARC_enc_ci_face_enc_gym08b_01_LZ_bin, NARC_enc_ci_face_enc_gym08b_02_LZ_bin, NARC_enc_ci_face_enc_gym08b_nclr, SPTR_GYM08B, GRA_TRANS_ALL,CI_PANEL_4,0x7fff,0x0000}, //ジムリーダー08B
  {FLDCIID_SITEN, NARC_enc_ci_face_enc_bigfour01_01_LZ_bin, 0 , NARC_enc_ci_face_enc_bigfour01_nclr, SPTR_BIGFOUR01, GRA_TRANS_ALL,CI_PANEL_2,0x7fff,0x0000}, //四天王1
  {FLDCIID_SITEN, NARC_enc_ci_face_enc_bigfour02_01_LZ_bin, 0, NARC_enc_ci_face_enc_bigfour02_nclr, SPTR_BIGFOUR02, GRA_TRANS_ALL,CI_PANEL_2,0x7fff,0x0000}, //四天王2
  {FLDCIID_SITEN, NARC_enc_ci_face_enc_bigfour03_01_LZ_bin, 0, NARC_enc_ci_face_enc_bigfour03_nclr, SPTR_BIGFOUR03, GRA_TRANS_ALL,CI_PANEL_2,0x7fff,0x0000}, //四天王3
  {FLDCIID_SITEN, NARC_enc_ci_face_enc_bigfour04_01_LZ_bin, 0, NARC_enc_ci_face_enc_bigfour04_nclr, SPTR_BIGFOUR04, GRA_TRANS_ALL,CI_PANEL_2,0x7fff,0x0000}, //四天王4
  {FLDCIID_SITEN, NARC_enc_ci_face_enc_champ_01_LZ_bin, 0, NARC_enc_ci_face_enc_champ_nclr, SPTR_CHAMP, GRA_TRANS_ALL,CI_PANEL_2,0x7fff,0x0000}, //チャンプ
  {FLDCIID_PLASMA02, NARC_enc_ci_face_enc_boss_01_LZ_bin, 0, NARC_enc_ci_face_enc_boss_nclr, SPTR_BOSS, GRA_TRANS_ENEMY,CI_PANEL_2,0x7fff,0x0000}, //ボス
  {FLDCIID_PLASMA02, NARC_enc_ci_face_enc_sage_01_LZ_bin, 0, NARC_enc_ci_face_enc_sage_nclr, SPTR_SAGE, GRA_TRANS_ENEMY,CI_PANEL_2,0x7fff,0x0000}, //セージ
  {FLDCIID_PLASMA01, 0, 0, 0, 0, GRA_TRANS_NONE,CI_PANEL_2,0x7fff,0x0000}, //プラズマ団
  {FLDCIID_SHIN_MU, 0, 0, 0, 0, GRA_TRANS_NONE,CI_PANEL_2,0x7fff,0x0000}, //パッケージ
  {FLDCIID_IDOU, 0, 0, 0, 0, GRA_TRANS_NONE,CI_PANEL_2,0x7fff,0x0000}, //移動ポケ
  {FLDCIID_3JUUSHI, 0, 0, 0, 0, GRA_TRANS_NONE,CI_PANEL_2,0x7fff,0x0000}, //三銃士
  {FLDCIID_DOUKUTSU, 0, 0, 0, 0, GRA_TRANS_NONE,CI_PANEL_2,0x7fff,0x0000}, //トレーナー洞窟
  {FLDCIID_SABAKU, 0, 0, 0, 0, GRA_TRANS_NONE,CI_PANEL_2,0x7fff,0x0000}, //トレーナー砂漠
};

const ENC_CUTIN_DAT *ENC_CUTIN_NO_GetDat(const int inEncCutinNo)
{
  return &EncCutinDat[inEncCutinNo];
}
