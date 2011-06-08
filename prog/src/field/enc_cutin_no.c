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
  {FLDCIID_RIVAL, SPTR_RIVAL ,0x7fff ,0x0000}, //ライバル
  {FLDCIID_RIVAL, SPTR_SUPPORT,0x7fff,0x0000}, //サポート
  {FLDCIID_RIVAL, SPTR_GYM01A, 0x7fff,0x0000}, //ジムリーダー01A
  {FLDCIID_RIVAL, SPTR_GYM01B, 0x7fff,0x0000}, //ジムリーダー01B
  {FLDCIID_RIVAL, SPTR_GYM01C, 0x7fff,0x0000}, //ジムリーダー01C
  {FLDCIID_RIVAL, SPTR_GYM02, 0x7fff,0x0000}, //ジムリーダー02
  {FLDCIID_RIVAL, SPTR_GYM03, 0x7fff,0x0000}, //ジムリーダー03
  {FLDCIID_RIVAL, SPTR_GYM04, 0x7fff,0x5294}, //ジムリーダー04
  {FLDCIID_RIVAL, SPTR_GYM05, 0x7fff,0x0000}, //ジムリーダー05
  {FLDCIID_RIVAL, SPTR_GYM06, 0x7fff,0x0000}, //ジムリーダー06
  {FLDCIID_RIVAL, SPTR_GYM07, 0x7fff,0x0000}, //ジムリーダー07
  {FLDCIID_RIVAL, SPTR_GYM08A, 0x7fff,0x0000}, //ジムリーダー08A
  {FLDCIID_RIVAL, SPTR_GYM08B, 0x7fff,0x0000}, //ジムリーダー08B
  {FLDCIID_SITEN, SPTR_BIGFOUR01, 0x7fff,0x0000}, //四天王1
  {FLDCIID_SITEN, SPTR_BIGFOUR02, 0x7fff,0x0000}, //四天王2
  {FLDCIID_SITEN, SPTR_BIGFOUR03, 0x7fff,0x0000}, //四天王3
  {FLDCIID_SITEN, SPTR_BIGFOUR04, 0x7fff,0x0000}, //四天王4
  {FLDCIID_SITEN, SPTR_CHAMP ,0x7fff,0x0000}, //チャンプ
  {FLDCIID_PLASMA02, SPTR_BOSS, 0x7fff,0x0000}, //ボス
  {FLDCIID_PLASMA02, SPTR_SAGE, 0x7fff,0x0000}, //セージ
  {FLDCIID_PLASMA01, 0, 0x7fff,0x0000}, //プラズマ団
  {FLDCIID_SHIN_MU,  0, 0x7fff,0x0000}, //パッケージ
  {FLDCIID_IDOU, 0, 0x7fff,0x0000}, //移動ポケ
  {FLDCIID_3JUUSHI, 0, 0x7fff,0x0000}, //三銃士
  {FLDCIID_DOUKUTSU, 0, 0x7fff,0x0000}, //トレーナー洞窟
  {FLDCIID_SABAKU, 0, 0x7fff,0x0000}, //トレーナー砂漠
};

const ENC_CUTIN_DAT *ENC_CUTIN_NO_GetDat(const int inEncCutinNo)
{
  return &EncCutinDat[inEncCutinNo];
}
