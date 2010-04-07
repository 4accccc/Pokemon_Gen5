#-----------------------------------------------
# demo3d コマンドパラメータ定義
# コマンドが増えたら$cmd_table[]を追加する必要があります
#
# c_demo3d_cmd_check.rbにrequireして使われます
#-----------------------------------------------
#==============================================
#リスト値ハッシュ定義
#==============================================
$h_se_default = {"---"=>"DEMO3D_SE_PARAM_DEFAULT"}
$h_brightness_disp = {"main"=>"MASK_MAIN_DISPLAY","sub"=>"MASK_SUB_DISPLAY","double"=>"MASK_DOUBLE_DISPLAY"}
$h_bgm_chg_type = {"play"=>"DEMO3D_BGM_PLAY","stop"=>"DEMO3D_BGM_STOP","change"=>"DEMO3D_BGM_CHANGE","push"=>"DEMO3D_BGM_PUSH","pop"=>"DEMO3D_BGM_POP"}

#==============================================
#プレフィックス・サフィックス定義
#==============================================
$psfix_fx16 = $_psfix_prm.new( "FX16_CONST( ", " )")
$psfix_msg_arc = $_psfix_prm.new( "NARC_message_", "_dat")

#==============================================
#コマンドパラメータ定義
#==============================================
$prm_none = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #
] 
$prm_se_play = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #SEラベル
  $_cmd_prm.new( 0, 127, $h_se_default, nil ),       #volume
  $_cmd_prm.new( -128, 127, $h_se_default, nil ),      #pan
  $_cmd_prm.new( -32768, 32768, $h_se_default, nil ),  #pitch
  $_cmd_prm.new( 0, 3, $h_se_default, nil ),  #プレイヤー                
]
$prm_se_stop = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil  ), #SEラベル
  $_cmd_prm.new( 0, 3, $h_se_default, nil ),  #プレイヤー                
] 
$prm_se_volume_effect_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #SEラベル
  $_cmd_prm.new( 0, 127, nil, nil ), #スタート
  $_cmd_prm.new( 0, 127, nil, nil ), #エンド
  $_cmd_prm.new( 1, P_FREE, nil, nil ), #フレーム
  $_cmd_prm.new( 0, 3, $h_se_default, nil ), #プレイヤーNo
] 
$prm_se_pan_effect_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #SEラベル
  $_cmd_prm.new( -128, 127, nil, nil ), #スタート
  $_cmd_prm.new( -128, 127, nil, nil ), #エンド
  $_cmd_prm.new( 1, P_FREE, nil, nil ), #フレーム
  $_cmd_prm.new( 0, 3, $h_se_default, nil ), #プレイヤーNo
] 
$prm_se_pitch_effect_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #SEラベル
  $_cmd_prm.new( -32768, 32767, nil, nil ), #スタート
  $_cmd_prm.new( -32768, 32767, nil, nil ), #エンド
  $_cmd_prm.new( 1, P_FREE, nil, nil ), #フレーム
  $_cmd_prm.new( 0, 3, $h_se_default, nil ), #プレイヤーNo
] 
$prm_bgm_change_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #BGMラベル
  $_cmd_prm.new( 0, 65535, nil, nil ), #fadeout_frame
  $_cmd_prm.new( 0, 65535, nil, nil ), #fadein_frame
  $_cmd_prm.new( P_LIST, P_LIST, $h_bgm_chg_type, nil ), #type
] 
$prm_brightness_req = [
  $_cmd_prm.new( P_LIST, P_LIST, $h_brightness_disp, nil ), #適用画面
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #フレーム
  $_cmd_prm.new( -16, 16, nil, nil ), #開始輝度
  $_cmd_prm.new( -16, 16, nil, nil ), #終了輝度
]
$prm_flash_req = [
  $_cmd_prm.new( P_LIST, P_LIST, $h_brightness_disp ), #適用画面
  $_cmd_prm.new( -16, 16, nil, nil ), #開始輝度
  $_cmd_prm.new( -16, 16, nil, nil ), #最大輝度
  $_cmd_prm.new( -16, 16, nil, nil ), #終了輝度
  $_cmd_prm.new( 1, P_FREE, nil, nil ), #start～maxまでのフレーム
  $_cmd_prm.new( 1, P_FREE, nil, nil ), #max～endまでのフレーム
]
$prm_light_color_set = [
  $_cmd_prm.new( 0, 3, nil, nil ), #ライトNo
  $_cmd_prm.new( 0, 31, nil, nil ), #color R
  $_cmd_prm.new( 0, 31, nil, nil ), #color G
  $_cmd_prm.new( 0, 31, nil, nil ), #color B
]
$prm_talkwin_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil, $psfix_msg_arc ), #メッセージアーカイブID
  $_cmd_prm.new( P_FREE, P_FREE, nil, nil ), #メッセージラベル
  $_cmd_prm.new( 0, P_FREE, nil, nil ), #メッセージセット数
  $_cmd_prm.new( 1, P_FREE, nil, nil ), #メッセージを表示するフレーム数
  $_cmd_prm.new( 1, 30, nil, nil ), #ウィンドウ左上X
  $_cmd_prm.new( 1, 21, nil, nil ), #ウィンドウ左上Y
  $_cmd_prm.new( 2, 28, nil, nil ), #ウィンドウサイズX
  $_cmd_prm.new( 2, 22, nil, nil ), #ウィンドウサイズY
] 
$prm_light_vector_set = [
  $_cmd_prm.new( 0, 3, nil, nil ), #ライトNo
  $_cmd_prm.new( -7.9, 7.9, nil, $psfix_fx16 ), #vector X
  $_cmd_prm.new( -7.9, 7.9, nil, $psfix_fx16 ), #vector Y
  $_cmd_prm.new( -7.9, 7.9, nil, $psfix_fx16 ), #vector Z
]
$prm_motionbl_start = [
  $_cmd_prm.new( 0, 31, nil, nil ), #新しくブレンドするα
  $_cmd_prm.new( 0, 31, nil, nil ), #バッファリングされているα
] 

$cmd_tbl = [
  CDemo3DCmd::new( "SE_PLAY", $prm_se_play),
  CDemo3DCmd::new( "SE_STOP", $prm_se_stop),
  CDemo3DCmd::new( "SE_VOLUME_EFFECT_REQ", $prm_se_volume_effect_req),
  CDemo3DCmd::new( "SE_PAN_EFFECT_REQ", $prm_se_pan_effect_req),
  CDemo3DCmd::new( "SE_PITCH_EFFECT_REQ", $prm_se_pitch_effect_req),
  CDemo3DCmd::new( "BGM_CHANGE_REQ", $prm_bgm_change_req),
  CDemo3DCmd::new( "BRIGHTNESS_REQ", $prm_brightness_req),
  CDemo3DCmd::new( "FLASH_REQ", $prm_flash_req),
  CDemo3DCmd::new( "LIGHT_COLOR_SET", $prm_light_color_set),
  CDemo3DCmd::new( "LIGHT_VECTOR_SET", $prm_light_vector_set),
  CDemo3DCmd::new( "TALKWIN_REQ", $prm_talkwin_req),
  CDemo3DCmd::new( "MOTIONBL_START", $prm_motionbl_start),
  CDemo3DCmd::new( "MOTIONBL_END", nil ),
  CDemo3DCmd::new( "END", nil),
]


