  require "../label_make/label_make"
  require "../constant.rb"

=begin
  u8    basic_hp;       //基本ＨＰ
  u8    basic_pow;      //基本攻撃力
  u8    basic_def;      //基本防御力
  u8    basic_agi;      //基本素早さ

  u8    basic_spepow;   //基本特殊攻撃力
  u8    basic_spedef;   //基本特殊防御力
  u8    type1;          //属性１
  u8    type2;          //属性２

  u8    get_rate;       //捕獲率
  u8    give_exp;       //贈与経験値
  u16   pains_hp    :2; //贈与努力値ＨＰ
  u16   pains_pow   :2; //贈与努力値攻撃力
  u16   pains_def   :2; //贈与努力値防御力
  u16   pains_agi   :2; //贈与努力値素早さ
  u16   pains_spepow:2; //贈与努力値特殊攻撃力
  u16   pains_spedef:2; //贈与努力値特殊防御力
  u16               :4; //贈与努力値予備

  u16   item1;          //アイテム１
  u16   item2;          //アイテム２

  u16   item3;          //アイテム３
  u8    sex;            //性別ベクトル
  u8    egg_birth;      //タマゴの孵化歩数

  u8    friend;         //なつき度初期値
  u8    grow;           //成長曲線識別
  u8    egg_group1;     //こづくりグループ1
  u8    egg_group2;     //こづくりグループ2

  u8    speabi1;        //特殊能力１
  u8    speabi2;        //特殊能力２
  u8    speabi3;        //特殊能力３
  u8    escape;         //逃げる率

  u16   form_index;     //別フォルムパーソナルデータ開始位置
  u8    form_max;       //別フォルムMAX
  u8    color :7;       //色（図鑑で使用）
  u8    reverse :1;     //反転フラグ

  u32   machine1;       //技マシンフラグ１
  u32   machine2;       //技マシンフラグ２
  u32   machine3;       //技マシンフラグ２
  u32   machine4;       //技マシンフラグ２
=end

class PARA
  enum_const_set %w[
    POKENAME
    PAREFORM_POKENAME
    SHINKA_POKE1
    SHINKA_POKE2
    SHINKA_POKE3
    SHINKA_POKE4
    SHINKA_POKE5
    SHINKA_POKE6
    SHINKA_POKE7
    WAZA1
    WAZA2
    WAZA3
    WAZA4
    WAZA5
    WAZA6
    WAZA7
    WAZA8
    WAZA9
    WAZA10
    WAZA11
    WAZA12
    WAZA13
    WAZA14
    WAZA15
    WAZA16
    WAZA17
    WAZA18
    WAZA19
    WAZA20
    WAZA21
    WAZA22
    WAZA23
    WAZA24
    WAZA25
    ITEM1
    ITEM2
    ITEM3
    SPEABI1
    SPEABI2
    SPEABI3
    ZENKOKU_NO
    CHIHOU_NO
    SHINKA_LEVEL
    TYPE1
    TYPE2
    GROW
    EXP
    GET
    GENDER
    EGG_GROUP1
    EGG_GROUP2
    BIRTH_STEP
    FRIEND
    ESCAPE
    HP
    POW
    DEF
    AGI
    SPEPOW
    SPEDEF
    HP_EXP
    POW_EXP
    DEF_EXP
    AGI_EXP
    SPEPOW_EXP
    SPEDEF_EXP
    WAZA_LV1
    WAZA_LV2
    WAZA_LV3
    WAZA_LV4
    WAZA_LV5
    WAZA_LV6
    WAZA_LV7
    WAZA_LV8
    WAZA_LV9
    WAZA_LV10
    WAZA_LV11
    WAZA_LV12
    WAZA_LV13
    WAZA_LV14
    WAZA_LV15
    WAZA_LV16
    WAZA_LV17
    WAZA_LV18
    WAZA_LV19
    WAZA_LV20
    WAZA_LV21
    WAZA_LV22
    WAZA_LV23
    WAZA_LV24
    WAZA_LV25
    SHINKA_COND1
    SHINKA_COND2
    SHINKA_COND3
    SHINKA_COND4
    SHINKA_COND5
    SHINKA_COND6
    SHINKA_COND7
    MACHINE_001
    MACHINE_002
    MACHINE_003
    MACHINE_004
    MACHINE_005
    MACHINE_006
    MACHINE_007
    MACHINE_008
    MACHINE_009
    MACHINE_010
    MACHINE_011
    MACHINE_012
    MACHINE_013
    MACHINE_014
    MACHINE_015
    MACHINE_016
    MACHINE_017
    MACHINE_018
    MACHINE_019
    MACHINE_020
    MACHINE_021
    MACHINE_022
    MACHINE_023
    MACHINE_024
    MACHINE_025
    MACHINE_026
    MACHINE_027
    MACHINE_028
    MACHINE_029
    MACHINE_030
    MACHINE_031
    MACHINE_032
    MACHINE_033
    MACHINE_034
    MACHINE_035
    MACHINE_036
    MACHINE_037
    MACHINE_038
    MACHINE_039
    MACHINE_040
    MACHINE_041
    MACHINE_042
    MACHINE_043
    MACHINE_044
    MACHINE_045
    MACHINE_046
    MACHINE_047
    MACHINE_048
    MACHINE_049
    MACHINE_050
    MACHINE_051
    MACHINE_052
    MACHINE_053
    MACHINE_054
    MACHINE_055
    MACHINE_056
    MACHINE_057
    MACHINE_058
    MACHINE_059
    MACHINE_060
    MACHINE_061
    MACHINE_062
    MACHINE_063
    MACHINE_064
    MACHINE_065
    MACHINE_066
    MACHINE_067
    MACHINE_068
    MACHINE_069
    MACHINE_070
    MACHINE_071
    MACHINE_072
    MACHINE_073
    MACHINE_074
    MACHINE_075
    MACHINE_076
    MACHINE_077
    MACHINE_078
    MACHINE_079
    MACHINE_080
    MACHINE_081
    MACHINE_082
    MACHINE_083
    MACHINE_084
    MACHINE_085
    MACHINE_086
    MACHINE_087
    MACHINE_088
    MACHINE_089
    MACHINE_090
    MACHINE_091
    MACHINE_092
    MACHINE_093
    MACHINE_094
    MACHINE_095
    MACHINE_096
    MACHINE_097
    MACHINE_098
    MACHINE_099
    MACHINE_100
    GRA_NO
    FORM_NAME
    COLOR
    REVERSE
    MAX
  ]
end

class ARG
  enum_const_set %w[
    READ_FILE
  ]
end

  p PARA::MAX
  p ARG::READ_FILE
