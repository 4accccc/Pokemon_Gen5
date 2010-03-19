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

#==============================================
#コマンドパラメータ定義
#==============================================
$prm_none = [
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #
] 
$prm_se_play = [
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #SEラベル
  $_cmd_prm.new( 0, 127, $h_se_default ),       #volume
  $_cmd_prm.new( -128, 127, $h_se_default ),      #pan
  $_cmd_prm.new( -32768, 32768, $h_se_default ),  #pitch
  $_cmd_prm.new( 0, 3, $h_se_default ),  #プレイヤー                
]
$prm_se_stop = [
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #SEラベル
] 
$prm_se_volume_effect_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #SEラベル
  $_cmd_prm.new( 0, 127, nil ), #スタート
  $_cmd_prm.new( 0, 127, nil ), #エンド
  $_cmd_prm.new( 1, P_FREE, nil ), #フレーム
  $_cmd_prm.new( 0, 3, $h_se_default ), #プレイヤーNo
] 
$prm_se_pan_effect_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #SEラベル
  $_cmd_prm.new( -128, 127, nil ), #スタート
  $_cmd_prm.new( -128, 127, nil ), #エンド
  $_cmd_prm.new( 1, P_FREE, nil ), #フレーム
  $_cmd_prm.new( 0, 3, $h_se_default ), #プレイヤーNo
] 
$prm_se_pitch_effect_req = [
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #SEラベル
  $_cmd_prm.new( -32768, 32767, nil ), #スタート
  $_cmd_prm.new( -32768, 32767, nil ), #エンド
  $_cmd_prm.new( 1, P_FREE, nil ), #フレーム
  $_cmd_prm.new( 0, 3, $h_se_default ), #プレイヤーNo
] 
$prm_brightness_req = [
  $_cmd_prm.new( P_LIST, P_LIST, $h_brightness_disp ), #適用画面
  $_cmd_prm.new( P_FREE, P_FREE, nil ), #フレーム
  $_cmd_prm.new( -16, 16, nil ), #開始輝度
  $_cmd_prm.new( -16, 16, nil ), #終了輝度
]
$prm_flash_req = [
  $_cmd_prm.new( P_LIST, P_LIST, $h_brightness_disp ), #適用画面
  $_cmd_prm.new( -16, 16, nil ), #開始輝度
  $_cmd_prm.new( -16, 16, nil ), #最大輝度
  $_cmd_prm.new( -16, 16, nil ), #終了輝度
  $_cmd_prm.new( 1, P_FREE, nil ), #start～maxまでのフレーム
  $_cmd_prm.new( 1, P_FREE, nil ), #max～endまでのフレーム
]
$prm_motionbl_start = [
  $_cmd_prm.new( 0, 31, nil ), #新しくブレンドするα
  $_cmd_prm.new( 0, 31, nil ), #バッファリングされているα
] 

$cmd_tbl = [
  CDemo3DCmd::new( "SE_PLAY", 5, $prm_se_play),
  CDemo3DCmd::new( "SE_STOP", 1, $prm_se_stop),
  CDemo3DCmd::new( "SE_VOLUME_EFFECT_REQ", 5, $prm_se_volume_effect_req),
  CDemo3DCmd::new( "SE_PAN_EFFECT_REQ", 5, $prm_se_pan_effect_req),
  CDemo3DCmd::new( "SE_PITCH_EFFECT_REQ", 5, $prm_se_pitch_effect_req),
  CDemo3DCmd::new( "BRIGHTNESS_REQ", 4, $prm_brightness_req),
  CDemo3DCmd::new( "FLASH_REQ", 6, $prm_flash_req),
  CDemo3DCmd::new( "MOTIONBL_START", 2, $prm_motionbl_start),
  CDemo3DCmd::new( "MOTIONBL_END", 0, $prm_none),
  CDemo3DCmd::new( "END", 0, $prm_none),
]


