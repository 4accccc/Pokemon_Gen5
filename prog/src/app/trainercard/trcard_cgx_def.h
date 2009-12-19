#ifndef __TRCARD_CGX_DEF_H__
#define __TRCARD_CGX_DEF_H__

#define TR_BG_BASE    (1) //プラス方向に使っていく
//トレーナー画像サイズ  (男女主人公のキャラが32x4、ユニオントレーナーは10*11)
#define TR_BG_SIZE    (32*4)  

//メッセージ表示領域(マイナス方向に使っていく)
#define MSG_BG_BASE (1023)

//サイン
#define SIGN_CGX  (TR_BG_BASE)
#define SIGN_PX (4)
#define SIGN_PY (11)
#define SIGN_SX (24)
#define SIGN_SY (8)
#define SIGN_BYTE_SIZE  (SIGN_SX*SIGN_SY*64)
#define TR_SIGN_SIZE  (SIGN_SX*SIGN_SY) //サインサイズ

//リーダーアイコン
#define LEADER_ICON_X (7)
#define LEADER_ICON_JY  (7)
#define LEADER_ICON_KY  (14)
#define LEADER_ICON_OX  (6)
#define LEADER_ICON_OY  (5)
#define LEADER_ICON_SX  (5)
#define LEADER_ICON_SY  (4)
#define LEADER_ICON_PY  (9)

#endif  //__TRCARD_CGX_DEF_H__
