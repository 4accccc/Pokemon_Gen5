#! ruby -Ks

  require File.dirname(__FILE__) + '/../label_make/label_make'
  require File.dirname(__FILE__) + '/../gmm_make/gmm_make'
  require File.dirname(__FILE__) + '/../constant'
  require File.dirname(__FILE__) + '/../hash/wazano_hash.rb'
  require File.dirname(__FILE__) + '/../hash/item_hash.rb'
  require File.dirname(__FILE__) + '/../hash/tokusei_hash.rb'
  require 'date'

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
  u8    rank;           //ポケモンランク
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
  u16   form_gra_index; //別フォルムグラフィックデータ開始位置

  u8    form_max;       //別フォルムMAX
  u8    color     :6;   //色（図鑑で使用）
  u8    reverse   :1;   //反転フラグ
  u8    pltt_only :1;   //別フォルム時パレットのみ変化  
  u16   give_exp;       //贈与経験値

  u16   height;         //高さ
  u16   weight;         //重さ

  u32   machine1;       //技マシンフラグ１
  u32   machine2;       //技マシンフラグ２
  u32   machine3;       //技マシンフラグ２
  u32   machine4;       //技マシンフラグ２
=end

class PARA
  enum_const_set %w[
    POKENAME
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
    WAZA_MACHINE
    GRA_NO
    FORM_NAME
    COLOR
    REVERSE
    OTHERFORM
    RANK
    PLTT_ONLY
    HEIGHT
    WEIGHT
    MAX
  ]
end

class SHINKA
  enum_const_set %w[
	  NONE
	  FRIEND_HIGH
	  FRIEND_HIGH_NOON
	  FRIEND_HIGH_NIGHT
	  LEVELUP
	  TUUSHIN
	  TUUSHIN_ITEM
	  ITEM
	  SPECIAL_POW
	  SPECIAL_EVEN
	  SPECIAL_DEF
	  SPECIAL_RND_EVEN
	  SPECIAL_RND_ODD
	  SPECIAL_LEVELUP
	  SPECIAL_NUKENIN
	  SPECIAL_BEAUTIFUL
	  ITEM_MALE
	  ITEM_FEMALE
	  SOUBI_NOON
	  SOUBI_NIGHT
	  WAZA
	  POKEMON
	  MALE
	  FEMALE
	  PLACE_TENGANZAN
	  PLACE_KOKE
	  PLACE_ICE
  ]
end

class SHINKA_PARAM
  enum_const_set %w[
    NONE
    LEVEL
    ITEM
    WAZA
    POKEMON
  ]
end

class FORM
  def initialize
    @form_name = []
    @form_index = 0
  end

  def add_form_name( form_name )
    @form_name << form_name
  end

  def get_form_name( index )
    return @form_name[ index ]
  end

  def get_form_max
    return @form_name.size
  end

  def set_form_index( index )
    @form_index = index
  end

  def get_form_index
    return @form_index
  end
end

def write_lst_file( fp_lst, gra_no, form_name )
  case gra_no
  when 1..151
   fp_lst.printf( "kanto/%03d/pfwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pfwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pfwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pfwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pfwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pfwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pbwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pbwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pbwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pbwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pbwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pbwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pmwb_%03d%s_n.ncl\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "kanto/%03d/pmwb_%03d%s_r.ncl\n",   gra_no, gra_no, form_name )
  when 152..251
   fp_lst.printf( "jyoto/%03d/pfwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pfwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pfwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pfwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pfwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pfwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pbwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pbwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pbwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pbwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pbwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pbwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pmwb_%03d%s_n.ncl\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "jyoto/%03d/pmwb_%03d%s_r.ncl\n",   gra_no, gra_no, form_name )
  when 252..386
   fp_lst.printf( "houen/%03d/pfwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pfwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pfwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pfwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pfwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pfwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pbwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pbwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pbwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pbwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pbwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pbwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pmwb_%03d%s_n.ncl\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "houen/%03d/pmwb_%03d%s_r.ncl\n",   gra_no, gra_no, form_name )
  when 387..493
   fp_lst.printf( "shinou/%03d/pfwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pfwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pfwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pfwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pfwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pfwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pbwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pbwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pbwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pbwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pbwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pbwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pmwb_%03d%s_n.ncl\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "shinou/%03d/pmwb_%03d%s_r.ncl\n",   gra_no, gra_no, form_name )
  else
   fp_lst.printf( "issyu/%03d/pfwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pfwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pfwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pfwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pfwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pfwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pbwb_%03d%s_m.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pbwb_%03d%s_f.ncg\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pbwb_%03dc%s_m.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pbwb_%03dc%s_f.ncg\n",  gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pbwb_%03d%s.nce\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pbwb_%03d%s.nmc\n",     gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pmwb_%03d%s_n.ncl\n",   gra_no, gra_no, form_name )
   fp_lst.printf( "issyu/%03d/pmwb_%03d%s_r.ncl\n",   gra_no, gra_no, form_name )
  end
end

#データテーブル
  type_table = {
    "ノーマル"=>0,
    "ノ－マル"=>0,
    "格闘"=>1,
    "飛行"=>2,
    "毒"=>3,
    "地面"=>4,
    "岩"=>5,
    "虫"=>6,
    "ゴースト"=>7,
    "ゴ－スト"=>7,
    "鉄"=>8,
    "鋼"=>8,
    "メタル"=>8,
    "炎"=>9,
    "水"=>10,
    "草"=>11,
    "電気"=>12,
    "エスパー"=>13,
    "エスパ－"=>13,
    "氷"=>14,
    "ドラゴン"=>15,
    "悪"=>16,
  }

  color_table = {
    "赤"=>0,
    "青"=>1,
    "黄"=>2,
    "緑"=>3,
    "黒"=>4,
    "茶"=>5,
    "紫"=>6,
    "灰"=>7,
    "白"=>8,
    "桃"=>9,
  }

  shinka_table = {
    "なし"                      => [  0, "NONE",              SHINKA_PARAM::NONE ],
    "なつき度　高い"            => [  1, "NONE",              SHINKA_PARAM::NONE ],
    "なつき度　高い　朝昼"      => [  2, "NONE",              SHINKA_PARAM::NONE ],
    "なつき度　高い　夜"        => [  3, "NONE",              SHINKA_PARAM::NONE ],
    "レベルアップ"              => [  4, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "通信"                      => [  5, "NONE",              SHINKA_PARAM::NONE ],
    "通信　アップグレード"      => [  6, "アップグレード",    SHINKA_PARAM::ITEM ],
    "通信　おうじゃのしるし"    => [  6, "おうじゃのしるし",  SHINKA_PARAM::ITEM ],
    "通信　メタルコート"        => [  6, "メタルコート",      SHINKA_PARAM::ITEM ],
    "通信　りゅうのウロコ"      => [  6, "りゅうのウロコ",    SHINKA_PARAM::ITEM ],
    "通信　こだいのキバ"        => [  6, "こだいのキバ",      SHINKA_PARAM::ITEM ],
    "通信　こだいのウロコ"      => [  6, "こだいのウロコ",    SHINKA_PARAM::ITEM ],
    "通信　しんかいのキバ"      => [  6, "しんかいのキバ",    SHINKA_PARAM::ITEM ],
    "通信　しんかいのウロコ"    => [  6, "しんかいのウロコ",  SHINKA_PARAM::ITEM ],
    "通信　プロテクター"        => [  6, "プロテクター",      SHINKA_PARAM::ITEM ],
    "通信　エレキブースター"    => [  6, "エレキブースター",  SHINKA_PARAM::ITEM ],
    "通信　マグマブースター"    => [  6, "マグマブースター",  SHINKA_PARAM::ITEM ],
    "通信　あやしいパッチ"      => [  6, "あやしいパッチ",    SHINKA_PARAM::ITEM ],
    "通信　れいかいのぬの"      => [  6, "れいかいのぬの",    SHINKA_PARAM::ITEM ],
    "道具　かみなりのいし"      => [  7, "かみなりのいし",    SHINKA_PARAM::ITEM ],
    "道具　たいようのいし"      => [  7, "たいようのいし",    SHINKA_PARAM::ITEM ],
    "道具　つきのいし"          => [  7, "つきのいし",        SHINKA_PARAM::ITEM ],
    "道具　ほのおのいし"        => [  7, "ほのおのいし",      SHINKA_PARAM::ITEM ],
    "道具　みずのいし"          => [  7, "みずのいし",        SHINKA_PARAM::ITEM ],
    "道具　リーフのいし"        => [  7, "リーフのいし",      SHINKA_PARAM::ITEM ],
    "道具　ひかりのいし"        => [  7, "ひかりのいし",      SHINKA_PARAM::ITEM ],
    "道具　やみのいし"          => [  7, "やみのいし",        SHINKA_PARAM::ITEM ],
    "特殊　攻"                  => [  8, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　同"                  => [  9, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　防"                  => [ 10, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　個性乱数が偶数"      => [ 11, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　個性乱数が奇数"      => [ 12, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　レベルアップ"        => [ 13, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　手持ちに空きがある"  => [ 14, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "特殊　美しさ"              => [ 15, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "道具♂　めざめいし"        => [ 16, "めざめいし",        SHINKA_PARAM::ITEM ],
    "道具♀　めざめいし"        => [ 17, "めざめいし",        SHINKA_PARAM::ITEM ],
    "装備　朝昼　まんまるいし"  => [ 18, "まんまるいし",      SHINKA_PARAM::ITEM ],
    "装備　夜　するどいキバ"    => [ 19, "するどいキバ",      SHINKA_PARAM::ITEM ],
    "装備　夜　するどいツメ"    => [ 19, "するどいツメ",      SHINKA_PARAM::ITEM ],
    "技習得　ものまね"          => [ 20, "ものまね",          SHINKA_PARAM::WAZA ],
    "技習得　ダブルアタック"    => [ 20, "ダブルアタック",    SHINKA_PARAM::WAZA ],
    "技習得　ころがる"          => [ 20, "ころがる",          SHINKA_PARAM::WAZA ],
    "技習得　げんしのちから"    => [ 20, "げんしのちから",    SHINKA_PARAM::WAZA ],
    "ポケモン　テッポウオ"      => [ 21, "テッポウオ",        SHINKA_PARAM::POKEMON ],
    "性別♂"                    => [ 22, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "性別♀"                    => [ 23, "LEVEL",             SHINKA_PARAM::LEVEL ],
    "場所　てんがんざん"        => [ 24, "NONE",              SHINKA_PARAM::NONE ],
    "場所　苔生した岩"          => [ 25, "NONE",              SHINKA_PARAM::NONE ],
    "場所　凍結した岩"          => [ 26, "NONE",              SHINKA_PARAM::NONE ],
  }

  shinka_cond = [ 
  	"SHINKA_NONE",
  	"SHINKA_FRIEND_HIGH",
  	"SHINKA_FRIEND_HIGH_NOON",
  	"SHINKA_FRIEND_HIGH_NIGHT",
  	"SHINKA_LEVELUP",
  	"SHINKA_TUUSHIN",
  	"SHINKA_TUUSHIN_ITEM",
  	"SHINKA_ITEM",
  	"SHINKA_SPECIAL_POW",
  	"SHINKA_SPECIAL_EVEN",
  	"SHINKA_SPECIAL_DEF",
  	"SHINKA_SPECIAL_RND_EVEN",
  	"SHINKA_SPECIAL_RND_ODD",
  	"SHINKA_SPECIAL_LEVELUP",
  	"SHINKA_SPECIAL_NUKENIN",
  	"SHINKA_SPECIAL_BEAUTIFUL",
  	"SHINKA_ITEM_MALE",
  	"SHINKA_ITEM_FEMALE",
  	"SHINKA_SOUBI_NOON",
  	"SHINKA_SOUBI_NIGHT",
  	"SHINKA_WAZA",	
  	"SHINKA_POKEMON",
  	"SHINKA_MALE",
  	"SHINKA_FEMALE",
  	"SHINKA_PLACE_TENGANZAN",
  	"SHINKA_PLACE_KOKE",
  	"SHINKA_PLACE_ICE",
  ]

	if ARGV.size < 2
		print "error: ruby personal_conv.rb read_file gmm_file\n"
		print "read_file:読み込むファイル\n"
		print "gmm_file:ポケモン名のgmmファイルを書き出すための元になるファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_READ_GMM_FILE = 1
  EVO_MAX = 7 #進化のMAX数

  label = LabelMake.new
  read_data = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      str = str.tr( "\"\r\n", "" )
      split_data = str.split(/,/)
      next if split_data.size <= 1       #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
      read_data[ cnt ] = str
      cnt += 1
    end
  }

  monsno = Hash::new
  gra_hash = Hash::new
  monsname = []
  form = []
  machine = []
  pokelist = []
  
  form[ 0 ] = FORM::new
  read_data.size.times {|i|
    form[ i + 1 ] = FORM::new
  }

  #ポケモンラベル＆グラフィックデータ圧縮リスト＆gmmファイル生成
  print "ポケモンラベル＆グラフィックデータ圧縮リスト＆gmmファイル　生成中\n"
  fp_monsno = open( "monsno_def.h", "w" )
  fp_monsnum = open( "monsnum_def.h", "w" )
  fp_gra = open( "pokegra_wb.scr", "w" )
  fp_lst = open( "pokegra_wb.lst", "w" )
  fp_wave = open( "wb_sound_pv_wav.lst", "w" )
  fp_bank = open( "wb_sound_pv_bnk.lst", "w" )
  fp_num = open( "zukan2grano.txt", "w" )
  gmm = GMM::new
  gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "monsname.gmm" )

  fp_monsno.print( "//============================================================================================\n" )
  fp_monsno.print( "/**\n" )
  fp_monsno.print( " * @file	monsno_def.h\n" )
  fp_monsno.print( " * @bfief	ポケモンNoのDefine定義ファイル\n" )
  fp_monsno.print( " * @author	PersonalConverter\n" )
  fp_monsno.print( " * パーソナルコンバータから生成されました\n" )
  fp_monsno.print( "*/\n")
  fp_monsno.print( "//============================================================================================\n" )
  fp_monsno.print( "\n#pragma once\n" )
  fp_monsno.print( "#include  \"monsnum_def.h\"\n\n" )

  fp_monsnum.print( "//============================================================================================\n" )
  fp_monsnum.print( "/**\n" )
  fp_monsnum.print( " * @file	monsnum_def.h\n" )
  fp_monsnum.print( " * @bfief	ポケモンNoのDefine定義ファイル（ラベル名はグラフィックナンバー）\n" )
  fp_monsnum.print( " * @author	PersonalConverter\n" )
  fp_monsnum.print( " * パーソナルコンバータから生成されました\n" )
  fp_monsnum.print( "*/\n")
  fp_monsnum.print( "//============================================================================================\n" )
  fp_monsnum.print( "\n#pragma once\n\n" )

	fp_gra.printf( "\"pfwb_000_m.NCGR\"\n" )
	fp_gra.printf( "\"pfwb_000_f.NCGR\"\n" )
	fp_gra.printf( "\"pfwb_000c_m.NCBR\"\n" )
	fp_gra.printf( "\"pfwb_000c_f.NCBR\"\n" )
	fp_gra.printf( "\"pfwb_000.NCER\"\n" )
	fp_gra.printf( "\"pfwb_000.NANR\"\n" )
	fp_gra.printf( "\"pfwb_000.NMCR\"\n" )
	fp_gra.printf( "\"pfwb_000.NMAR\"\n" )
	fp_gra.printf( "\"pfwb_000.NCEC\"\n" )
	fp_gra.printf( "\"pbwb_000_m.NCGR\"\n" )
	fp_gra.printf( "\"pbwb_000_f.NCGR\"\n" )
	fp_gra.printf( "\"pbwb_000c_m.NCBR\"\n" )
	fp_gra.printf( "\"pbwb_000c_f.NCBR\"\n" )
	fp_gra.printf( "\"pbwb_000.NCER\"\n" )
	fp_gra.printf( "\"pbwb_000.NANR\"\n" )
	fp_gra.printf( "\"pbwb_000.NMCR\"\n" )
	fp_gra.printf( "\"pbwb_000.NMAR\"\n" )
	fp_gra.printf( "\"pbwb_000.NCEC\"\n" )
	fp_gra.printf( "\"pmwb_000_n.NCLR\"\n" )
	fp_gra.printf( "\"pmwb_000_r.NCLR\"\n" )

  fp_lst.printf( "kanto/000/pfwb_000_m.ncg\n" )
  fp_lst.printf( "kanto/000/pfwb_000_f.ncg\n" )
  fp_lst.printf( "kanto/000/pfwb_000c_m.ncg\n" )
  fp_lst.printf( "kanto/000/pfwb_000c_f.ncg\n" )
  fp_lst.printf( "kanto/000/pfwb_000.nce\n" )
  fp_lst.printf( "kanto/000/pfwb_000.nmc\n" )
  fp_lst.printf( "kanto/000/pbwb_000_m.ncg\n" )
  fp_lst.printf( "kanto/000/pbwb_000_f.ncg\n" )
  fp_lst.printf( "kanto/000/pbwb_000c_m.ncg\n" )
  fp_lst.printf( "kanto/000/pbwb_000c_f.ncg\n" )
  fp_lst.printf( "kanto/000/pbwb_000.nce\n" )
  fp_lst.printf( "kanto/000/pbwb_000.nmc\n" )
  fp_lst.printf( "kanto/000/pmwb_000_n.ncl\n" )
  fp_lst.printf( "kanto/000/pmwb_000_r.ncl\n" )

  gmm.make_row_index( "MONSNAME_", 0, "ーーーーー" )

  pokelist << "－－－－－\t999\tノーマル\tノーマル\t－\t－\n"

  cnt = 1
  other_form = 0
  monsno_max = 0
  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)
    if split_data[ PARA::POKENAME ] == ""
      other_form = 1
      if monsno_max == 0
        monsno_max = cnt
      end
    end
    if other_form == 1
      if split_data[ PARA::POKENAME ] == ""
      else
        if form[ monsno[ split_data[ PARA::POKENAME ] ] ].get_form_index == 0 && split_data[ PARA::OTHERFORM ] == "●"
          form[ monsno[ split_data[ PARA::POKENAME ] ] ].set_form_index( cnt )
        end
      end
    else
      monsno[ split_data[ PARA::POKENAME ] ] = cnt
      gra_hash[ split_data[ PARA::GRA_NO ] ] = cnt
      monsname[ cnt - 1 ] = split_data[ PARA::POKENAME ]
      fp_monsno.print( "#define\t\t" )
      label_str = label.make_label( "MONSNO_", split_data[ PARA::POKENAME ] )
      fp_monsno.print( label_str )
      tab_cnt = ( 19 - label_str.length ) / 2 + 1
      for j in 1..tab_cnt
        fp_monsno.print( "\t" )
      end
      fp_monsno.printf( "( %d )\n", cnt )

      fp_monsnum.printf( "#define\t\tMONSNO_%03d\t\t( %d )\t\t//%s\n", split_data[ PARA::GRA_NO ].to_i, cnt, split_data[ PARA::POKENAME ] )

      if split_data[ PARA::FORM_NAME ] == ""
        form_name = ""
      else
        form_name = "_"
        form_name << split_data[ PARA::FORM_NAME ]
      end

      gra_no = split_data[ PARA::GRA_NO ].to_i

	    fp_gra.printf( "\"pfwb_%03d%s_m.NCGR\"\n",   gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03d%s_f.NCGR\"\n",   gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03dc%s_m.NCBR\"\n",  gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03dc%s_f.NCBR\"\n",  gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03d%s.NCER\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03d%s.NANR\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03d%s.NMCR\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03d%s.NMAR\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pfwb_%03d%s.NCEC\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s_m.NCGR\"\n",   gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s_f.NCGR\"\n",   gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03dc%s_m.NCBR\"\n",  gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03dc%s_f.NCBR\"\n",  gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s.NCER\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s.NANR\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s.NMCR\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s.NMAR\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pbwb_%03d%s.NCEC\"\n",     gra_no, form_name )
	    fp_gra.printf( "\"pmwb_%03d%s_n.NCLR\"\n",   gra_no, form_name )
	    fp_gra.printf( "\"pmwb_%03d%s_r.NCLR\"\n",   gra_no, form_name )

      #SARC出力
      #WAVE&BANK
      if gra_no >= 501
	      fp_wave.printf( "WAVE_ARC_PMWB_%03d		: AUTO, \"PMWB_%03d.swls\"\n",   gra_no, gra_no )
	      fp_bank.printf( "BANK_PMWB_%03d		: TEXT, \"PMWB_%03d.bnk\",	WAVE_ARC_PMWB_%03d\n",   gra_no, gra_no, gra_no )
      end

      fp_num.printf( "%s\n", split_data[ PARA::GRA_NO ] )

      write_lst_file( fp_lst, gra_no, form_name )

      gmm.make_row_index( "MONSNAME_", cnt, split_data[ PARA::POKENAME ] )

      speabi1 = split_data[ PARA::SPEABI1 ]
      if speabi1 == ""
        speabi1 = "－"
      end
      speabi2 = split_data[ PARA::SPEABI2 ]
      if speabi2 == ""
        speabi2 = "－"
      end
      pokelist << "%s\t%s\t%s\t%s\t%s\t%s\n" % [ split_data[ PARA::POKENAME ], split_data[ PARA::GRA_NO ], split_data[ PARA::TYPE1 ], split_data[ PARA::TYPE2 ], speabi1, speabi2 ]
    end
    if split_data[ PARA::FORM_NAME ] != "" && split_data[ PARA::POKENAME ] != ""
      form[ monsno[ split_data[ PARA::POKENAME ] ] ].add_form_name( split_data[ PARA::FORM_NAME ] )
    end
    if other_form == 0 || ( other_form == 1 && split_data[ PARA::OTHERFORM ] == "●" )
      cnt += 1
    end
  }

  #他のスクリプトで使用するためのポケモンNoHashを生成
  fp_hash = open( "monsno_hash.rb", "w" )
  fp_hash.printf("#! ruby -Ks\n\n" )

  #モンスターナンバーハッシュテーブル
  fp_hash.printf("\t$monsno_hash = {\n" )
  monsno.size.times {|no|
    split_data = read_data[ no ].split(/,/)
    fp_hash.printf("\t\t\"%s\"=>%d,\n", split_data[ PARA::POKENAME ], monsno[ split_data[ PARA::POKENAME ] ] )
  }
  fp_hash.printf("\t}\n" )

  #フォルムナンバー最大値ハッシュテーブル
  fp_hash.printf("\t$formmax_hash = {\n" )
  monsno.size.times {|no|
    split_data = read_data[ no ].split(/,/)
    fp_hash.printf("\t\t\"%s\"=>%d,\n", split_data[ PARA::POKENAME ], form[ no ].get_form_max )
  }
  fp_hash.printf("\t}\n" )

  #GraNo2ZukanNoハッシュテーブル
  fp_hash.printf("\t$gra2zukan_hash = {\n" )
  gra_hash.size.times {|no|
    split_data = read_data[ no ].split(/,/)
    fp_hash.printf("\t\t\"%s\"=>%d,\n", split_data[ PARA::GRA_NO ], gra_hash[ split_data[ PARA::GRA_NO ] ] )
  }
  fp_hash.printf("\t}\n" )
  fp_hash.close

  #gmmファイルの後始末
  gmm.make_row_index( "MONSNAME_", monsno_max, "タマゴ" )
  gmm.make_row_index( "MONSNAME_", monsno_max + 1, "タマゴ" )
  gmm.close_gmm

  #タマゴは独自の持ち方をする
  fp_gra.print( "\"pfwb_egg_normal_m.NCGR\"\n" )
  fp_gra.print( "\"pfwb_egg_normal_f.NCGR\"\n" )
	fp_gra.print( "\"pfwb_eggc_normal_m.NCBR\"\n" )
	fp_gra.print( "\"pfwb_eggc_normal_f.NCBR\"\n" )
	fp_gra.print( "\"pfwb_egg_normal.NCER\"\n" )
	fp_gra.print( "\"pfwb_egg_normal.NANR\"\n" )
	fp_gra.print( "\"pfwb_egg_normal.NMCR\"\n" )
	fp_gra.print( "\"pfwb_egg_normal.NMAR\"\n" )
	fp_gra.print( "\"pfwb_egg_normal.NCEC\"\n" )
  fp_gra.print( "\"pbwb_egg_normal_m.NCGR\"\n" )
  fp_gra.print( "\"pbwb_egg_normal_f.NCGR\"\n" )
	fp_gra.print( "\"pbwb_eggc_normal_m.NCBR\"\n" )
	fp_gra.print( "\"pbwb_eggc_normal_f.NCBR\"\n" )
	fp_gra.print( "\"pbwb_egg_normal.NCER\"\n" )
	fp_gra.print( "\"pbwb_egg_normal.NANR\"\n" )
	fp_gra.print( "\"pbwb_egg_normal.NMCR\"\n" )
	fp_gra.print( "\"pbwb_egg_normal.NMAR\"\n" )
	fp_gra.print( "\"pbwb_egg_normal.NCEC\"\n" )
	fp_gra.print( "\"pmwb_egg_normal_n.NCLR\"\n" )
	fp_gra.print( "\"pmwb_egg_normal_r.NCLR\"\n" )

  fp_gra.print( "\"pfwb_egg_manafi_m.NCGR\"\n" )
  fp_gra.print( "\"pfwb_egg_manafi_f.NCGR\"\n" )
	fp_gra.print( "\"pfwb_eggc_manafi_m.NCBR\"\n" )
	fp_gra.print( "\"pfwb_eggc_manafi_f.NCBR\"\n" )
	fp_gra.print( "\"pfwb_egg_manafi.NCER\"\n" )
	fp_gra.print( "\"pfwb_egg_manafi.NANR\"\n" )
	fp_gra.print( "\"pfwb_egg_manafi.NMCR\"\n" )
	fp_gra.print( "\"pfwb_egg_manafi.NMAR\"\n" )
	fp_gra.print( "\"pfwb_egg_manafi.NCEC\"\n" )
  fp_gra.print( "\"pbwb_egg_manafi_m.NCGR\"\n" )
  fp_gra.print( "\"pbwb_egg_manafi_f.NCGR\"\n" )
	fp_gra.print( "\"pbwb_eggc_manafi_m.NCBR\"\n" )
	fp_gra.print( "\"pbwb_eggc_manafi_f.NCBR\"\n" )
	fp_gra.print( "\"pbwb_egg_manafi.NCER\"\n" )
	fp_gra.print( "\"pbwb_egg_manafi.NANR\"\n" )
	fp_gra.print( "\"pbwb_egg_manafi.NMCR\"\n" )
	fp_gra.print( "\"pbwb_egg_manafi.NMAR\"\n" )
	fp_gra.print( "\"pbwb_egg_manafi.NCEC\"\n" )
	fp_gra.print( "\"pmwb_egg_manafi_n.NCLR\"\n" )
	fp_gra.print( "\"pmwb_egg_manafi_r.NCLR\"\n" )

  fp_lst.print( "egg/pfwb_egg_normal_m.ncg\n" )
	fp_lst.print( "egg/pfwb_eggc_normal_m.ncg\n" )
	fp_lst.print( "egg/pfwb_egg_normal.nce\n" )
	fp_lst.print( "egg/pfwb_egg_normal.nmc\n" )
  fp_lst.print( "egg/pbwb_egg_normal_m.ncg\n" )
	fp_lst.print( "egg/pbwb_eggc_normal_m.ncg\n" )
	fp_lst.print( "egg/pbwb_egg_normal.nce\n" )
	fp_lst.print( "egg/pbwb_egg_normal.nmc\n" )
	fp_lst.print( "egg/pmwb_egg_normal_n.ncl\n" )
	fp_lst.print( "egg/pmwb_egg_normal_r.ncl\n" )
  fp_lst.print( "egg/pfwb_egg_manafi_m.ncg\n" )
	fp_lst.print( "egg/pfwb_eggc_manafi_m.ncg\n" )
	fp_lst.print( "egg/pfwb_egg_manafi.nce\n" )
	fp_lst.print( "egg/pfwb_egg_manafi.nmc\n" )
  fp_lst.print( "egg/pbwb_egg_manafi_m.ncg\n" )
	fp_lst.print( "egg/pbwb_eggc_manafi_m.ncg\n" )
	fp_lst.print( "egg/pbwb_egg_manafi.nce\n" )
	fp_lst.print( "egg/pbwb_egg_manafi.nmc\n" )
	fp_lst.print( "egg/pmwb_egg_manafi_n.ncl\n" )
	fp_lst.print( "egg/pmwb_egg_manafi_r.ncl\n" )

  fp_lst.print( "migawari/pfwb_migawari.ncg\n" )
	fp_lst.print( "migawari/pfwb_migawari.nce\n" )
	fp_lst.print( "migawari/pfwb_migawari.nmc\n" )
  fp_lst.print( "migawari/pbwb_migawari.ncg\n" )
	fp_lst.print( "migawari/pbwb_migawari.nce\n" )
	fp_lst.print( "migawari/pbwb_migawari.nmc\n" )
	fp_lst.print( "migawari/pmwb_migawari.ncl\n" )

  fp_gra.close
  fp_lst.close
  fp_num.close
	fp_wave.close
	fp_bank.close

  fp_pokelist = open( "pokelist.txt", "w" )
  pokelist.sort!
  pokelist.size.times {|i|
    fp_pokelist.printf( "%s", pokelist[ i ] )
  }
	fp_pokelist.close

  fp_monsno.printf( "#define\t\tMONSNO_TAMAGO\t\t\t\t( %d )\n", monsno_max )
  fp_monsno.printf( "#define\t\tMONSNO_DAMETAMAGO\t\t( %d )\n", monsno_max + 1 )
  fp_monsno.printf( "#define\t\tMONSNO_END\t\t\t\t\t( %d )\n",  monsno_max - 1 )
  fp_monsno.printf( "#define\t\tMONSNO_MAX\t\t\t\t\t( %d )\n",   monsno_max + 1 )
  fp_monsno.print( "\n" )

  fp_form = open( "otherform_wb.scr", "w" )
  fp_lst = open( "otherform_wb.lst", "w" )
  fp_pltt = open( "otherpltt_wb.scr", "w" )

  fp_monsno.print( "//別フォルム定義\n" )

  form.size.times {|i|
    if form[ i ].get_form_max != 0
      split_data = read_data[ i - 1 ].split(/,/)
      fp_monsno.printf( "//%s\n", split_data[ PARA::POKENAME ] )
      max = form[ i ].get_form_max
      max.times {|j|
        str = label.make_label( "FORMNO_", split_data[ PARA::POKENAME ] )
        str << "_"
        str << form[ i ].get_form_name( j ).upcase
        fp_monsno.printf( "#define\t\t%s\t\t\t\t( %d )\n", str, j )
        if j != 0
          if split_data[ PARA::PLTT_ONLY ] == "●"
  	        fp_form.printf( "\"pmwb_%s_%s_n.NCLR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pmwb_%s_%s_r.NCLR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
          else
  	        fp_form.printf( "\"pfwb_%s_%s_m.NCGR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%s_%s_f.NCGR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%sc_%s_m.NCBR\"\n",  split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%sc_%s_f.NCBR\"\n",  split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%s_%s.NCER\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%s_%s.NANR\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%s_%s.NMCR\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%s_%s.NMAR\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pfwb_%s_%s.NCEC\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s_m.NCGR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s_f.NCGR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%sc_%s_m.NCBR\"\n",  split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%sc_%s_f.NCBR\"\n",  split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s.NCER\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s.NANR\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s.NMCR\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s.NMAR\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pbwb_%s_%s.NCEC\"\n",     split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pmwb_%s_%s_n.NCLR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
  	        fp_form.printf( "\"pmwb_%s_%s_r.NCLR\"\n",   split_data[ PARA::GRA_NO ], form[ i ].get_form_name( j ) )
          end
          form_name = "_"
          form_name << form[ i ].get_form_name( j )

          write_lst_file( fp_lst, split_data[ PARA::GRA_NO ].to_i, form_name )
        end
      }
      fp_monsno.print( "\n" )
    end
  }

  #みがわり人形は最後に連結する
  fp_form.print( "\"pfwb_migawari.NCBR\"\n" )
	fp_form.print( "\"pfwb_migawari.NCER\"\n" )
	fp_form.print( "\"pfwb_migawari.NANR\"\n" )
	fp_form.print( "\"pfwb_migawari.NMCR\"\n" )
	fp_form.print( "\"pfwb_migawari.NMAR\"\n" )
	fp_form.print( "\"pfwb_migawari.NCEC\"\n" )
  fp_form.print( "\"pbwb_migawari.NCBR\"\n" )
	fp_form.print( "\"pbwb_migawari.NCER\"\n" )
	fp_form.print( "\"pbwb_migawari.NANR\"\n" )
	fp_form.print( "\"pbwb_migawari.NMCR\"\n" )
	fp_form.print( "\"pbwb_migawari.NMAR\"\n" )
	fp_form.print( "\"pbwb_migawari.NCEC\"\n" )
	fp_form.print( "\"pmwb_migawari.NCLR\"\n" )

  fp_monsno.close
  fp_monsnum.close
  fp_form.close
  fp_pltt.close

  print "ポケモンラベル＆グラフィックデータ圧縮リスト＆gmmファイル　生成終了\n"

  #技覚えテーブルハッシュの生成
  wothash = Hash::new

  print "パーソナルデータ生成中\n"
  #per_???.bin生成
  print "per_000.bin 生成中\n"
  fp_per = open( "per_000.bin", "wb" )

  data = [ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ].pack( "C10 S4 C10 S2 C2 S3 L4" )

	data.size.times{ |c|
		fp_per.printf("%c",data[ c ])
	}

  fp_per.close
  print "per_000.bin 生成終了\n"

  #wot_???.bin生成
  print "wot_000.bin 生成中\n"
  fp_wot = open( "wot_000.bin", "wb" )

  data = [ 0xffff, 0xffff ].pack( "S2" )

	data.size.times{ |c|
		fp_wot.printf("%c",data[ c ])
	}

  fp_wot.close
  print "wot_000.bin 生成終了\n"

  #evo_???.bin生成
  print "evo_000.bin 生成中\n"
  fp_evo = open( "evo_000.bin", "wb" )

  for evo_cnt in 1..EVO_MAX
    data = [ 0,0,0 ].pack( "S3" )
	  data.size.times{ |c|
		  fp_evo.printf("%c",data[ c ])
	  }
  end

  fp_evo.close
  print "evo_000.bin 生成終了\n"

  cnt = 1
  other_form = 0
  form_gra_index = 0
  seed = []
  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)
    if split_data[ PARA::POKENAME ] == ""
      other_form = 1
    end

    if wothash[ split_data[ PARA::POKENAME ] ] == nil
      wothash[ split_data[ PARA::POKENAME ] ] = []
    end

    if other_form == 0 || ( other_form == 1 && split_data[ PARA::OTHERFORM ] == "●" )
      printf( "per_%03d.bin 生成中\n", cnt )
      str = sprintf( "per_%03d.bin", cnt )
      fp_per = open( str, "wb" )

      hp      = split_data[ PARA::HP ].to_i
      pow     = split_data[ PARA::POW ].to_i
      defe    = split_data[ PARA::DEF ].to_i
      agi     = split_data[ PARA::AGI ].to_i
      spepow  = split_data[ PARA::SPEPOW ].to_i
      spedef  = split_data[ PARA::SPEDEF ].to_i
      if type_table[ split_data[ PARA::TYPE1 ] ] == nil
        printf( "定義されていないタイプです:%s\n", split_data[ PARA::TYPE1 ] )
        exit( 1 )
      end
      type1   = type_table[ split_data[ PARA::TYPE1 ] ]
      if type_table[ split_data[ PARA::TYPE2 ] ] == nil
        printf( "定義されていないタイプです:%s\n", split_data[ PARA::TYPE2 ] )
        exit( 1 )
      end
      type2   = type_table[ split_data[ PARA::TYPE2 ] ]

      get     = split_data[ PARA::GET ].to_i
      #@todo 今後はサウンド用の値になるはず
      rank    = split_data[ PARA::RANK ].to_i
      exp_value = ( split_data[ PARA::HP_EXP ].to_i     <<  0 ) |
                  ( split_data[ PARA::POW_EXP ].to_i    <<  2 ) |
                  ( split_data[ PARA::DEF_EXP ].to_i    <<  4 ) |
                  ( split_data[ PARA::AGI_EXP ].to_i    <<  6 ) |
                  ( split_data[ PARA::SPEPOW_EXP ].to_i <<  8 ) |
                  ( split_data[ PARA::SPEDEF_EXP ].to_i << 10 )

      if split_data[ PARA::ITEM1 ] == ""
        item1 = 0
      else
        item1 = $item_hash[ split_data[ PARA::ITEM1 ] ]
      end
      if split_data[ PARA::ITEM2 ] == ""
        item2 = 0
      else
        item2 = $item_hash[ split_data[ PARA::ITEM2 ] ]
      end
      if split_data[ PARA::ITEM3 ] == ""
        item3 = 0
      else
        item3 = $item_hash[ split_data[ PARA::ITEM3 ] ]
      end

      gender  = split_data[ PARA::GENDER ].to_i
      birth   = split_data[ PARA::BIRTH_STEP ].to_i
      friend  = split_data[ PARA::FRIEND ].to_i
      grow    = split_data[ PARA::GROW ].to_i
      egg1    = split_data[ PARA::EGG_GROUP1 ].to_i
      egg2    = split_data[ PARA::EGG_GROUP2 ].to_i

      if split_data[ PARA::SPEABI1 ] == ""
        speabi1 = 0
      else
        speabi1 = $tokusei_hash[ split_data[ PARA::SPEABI1 ] ]
      end
      if split_data[ PARA::SPEABI2 ] == ""
        speabi2 = 0
      else
        speabi2 = $tokusei_hash[ split_data[ PARA::SPEABI2 ] ]
      end
      if split_data[ PARA::SPEABI3 ] == ""
        speabi3 = 0
      else
        speabi3 = $tokusei_hash[ split_data[ PARA::SPEABI3 ] ]
      end

      escape  = split_data[ PARA::ESCAPE ].to_i
      
      form_per_start  = form[ cnt ].get_form_index

      if form[ cnt ].get_form_max == 0
        form_gra_start = 0
      else
        form_gra_start =  form_gra_index
        form_gra_index += ( form[ cnt ].get_form_max - 1 )
      end

      form_max  = form[ cnt ].get_form_max

      if color_table[ split_data[ PARA::COLOR ] ] ==nil
        printf( "定義されていないカラーです:%s\n", split_data[ PARA::COLOR ] )
        exit( 1 )
      end
      col_rev = color_table[ split_data[ PARA::COLOR ] ]
      if split_data[ PARA::REVERSE ] == "●"
        col_rev |= 0x40
      end
      if split_data[ PARA::PLTT_ONLY ] == "●"
        col_rev |= 0x80
      end

      exp = split_data[ PARA::EXP ].to_i

      if split_data[ PARA::HEIGHT ] == "" || split_data[ PARA::HEIGHT ] == nil
        height = 10
      else
        height = split_data[ PARA::HEIGHT ].to_i
      end

      if split_data[ PARA::WEIGHT ] == "" || split_data[ PARA::WEIGHT ] == nil
        weight = 10
      else
        weight = split_data[ PARA::WEIGHT ].to_i
      end

      waza_machine = split_data[ PARA::WAZA_MACHINE ].split(//)
      machine[ 0 ] = 0
      machine[ 1 ] = 0
      machine[ 2 ] = 0
      machine[ 3 ] = 0

      flag = 1
      flag_cnt = 32
      machine_index = 0

      waza_machine.size.times {|i|
        if waza_machine[ i ] == "●"
          machine[ machine_index ] |= flag
        end
        flag = flag << 1
        flag_cnt -= 1
        if flag_cnt == 0
          flag = 1
          flag_cnt = 32
          machine_index += 1
        end
      }

      data = [ hp,pow,defe,agi,spepow,spedef,type1,type2,get,rank,exp_value,item1,item2,item3,gender,birth,friend,grow,egg1,egg2,speabi1,speabi2,speabi3,escape,form_per_start,form_gra_start,form_max,col_rev,exp,height,weight,machine[0],machine[1],machine[2],machine[3] ].pack( "C10 S4 C10 S2 C2 S3 L4" )

	    data.size.times{ |c|
		    fp_per.printf("%c",data[ c ])
	    }

      fp_per.close
      printf( "per_%03d.bin 生成終了\n", cnt )

      printf( "wot_%03d.bin 生成中\n", cnt )
      str = sprintf( "wot_%03d.bin", cnt )
      fp_wot = open( str, "wb" )
      waza_cnt = 0
      while split_data[ PARA::WAZA1 + waza_cnt ] != ""
        waza_no = $wazano_hash[ split_data[ PARA::WAZA1 + waza_cnt ] ]
        waza_lv = split_data[ PARA::WAZA_LV1 + waza_cnt ].to_i

        data = [ waza_no, waza_lv ].pack("S2")
	      data.size.times{ |c|
		      fp_wot.printf("%c",data[ c ])
	      }
        #技覚えハッシュテーブル生成
        wothash[ split_data[ PARA::POKENAME ] ] << split_data[ PARA::WAZA1 + waza_cnt ]
        waza_cnt += 1
        if waza_cnt == 25
          break
        end
      end

      data = [ 0xffff, 0xffff ].pack( "S2" )

	    data.size.times{ |c|
		    fp_wot.printf("%c",data[ c ])
	    }
      fp_wot.close
      printf( "wot_%03d.bin 生成終了\n", cnt )

      printf( "evo_%03d.bin 生成中\n", cnt )
      str = sprintf( "evo_%03d.bin", cnt )
      fp_evo = open( str, "wb" )
      for evo_cnt in 0..EVO_MAX-1
        if split_data[ PARA::SHINKA_COND1 + evo_cnt ] == ""
          data = [ 0,0,0 ].pack( "S3" )
	        data.size.times{ |c|
		        fp_evo.printf("%c",data[ c ])
	        }
          next
        end
        if shinka_table[ split_data[ PARA::SHINKA_COND1 + evo_cnt ] ][ 0 ] == nil
          printf( "定義されていない進化条件です:%s\n", split_data[ PARA::SHINKA_COND1 + evo_cnt ] )
          exit( 1 )
        end
        evo_cond = shinka_table[ split_data[ PARA::SHINKA_COND1 + evo_cnt ] ][ 0 ]
        case shinka_table[ split_data[ PARA::SHINKA_COND1 + evo_cnt ] ][ 2 ]
        when SHINKA_PARAM::NONE
          evo_param = 0
        when SHINKA_PARAM::LEVEL
          evo_param = split_data[ PARA::SHINKA_LEVEL ].to_i
        when SHINKA_PARAM::ITEM
          evo_param = $item_hash[ shinka_table[ split_data[ PARA::SHINKA_COND1 + evo_cnt ] ][ 1 ] ]
        when SHINKA_PARAM::WAZA
          evo_param = $wazano_hash[ shinka_table[ split_data[ PARA::SHINKA_COND1 + evo_cnt ] ][ 1 ] ]
        when SHINKA_PARAM::POKEMON
          evo_param = monsno[ shinka_table[ split_data[ PARA::SHINKA_COND1 + evo_cnt ] ][ 1 ] ]
        end
        if split_data[ PARA::SHINKA_POKE1 + evo_cnt ] == ""
          evo_mons = 0
        else
          evo_mons = monsno[ split_data[ PARA::SHINKA_POKE1 + evo_cnt ] ]
        end
        if split_data[ PARA::SHINKA_POKE1 + evo_cnt ] != ""
          seed[ monsno[ split_data[ PARA::SHINKA_POKE1 + evo_cnt ] ] ] = cnt
        end
        data = [ evo_cond, evo_param, evo_mons ].pack( "S3" )
        data.size.times{ |c|
          fp_evo.printf("%c",data[ c ])
        }
      end
      fp_evo.close
      printf( "evo_%03d.bin 生成終了\n", cnt )
    end

    if other_form == 0 || ( other_form == 1 && split_data[ PARA::OTHERFORM ] == "●" )
      cnt += 1
    end
  }

  #技覚えハッシュテーブル生成
  fp_wothash = open( "wazaoboe_hash.rb", "w" )
  fp_wothash.printf("#! ruby -Ks\n\n" )
  fp_wothash.printf("\t$wazaoboe_hash = {\n" )

  monsname.size.times {|no|
    fp_wothash.printf( "\t\t\"%s\"=>[\n", monsname[ no ] )
    wothash[ monsname[ no ] ].size.times {|wot|
      fp_wothash.printf( "\t\t\t\t\"%s\",\n", wothash[ monsname[ no ] ][ wot ] )
    }
    fp_wothash.printf( "\t\t],\n", monsname[ no ] )
  }
  fp_wothash.printf("\t}\n" )
  fp_wothash.close

  print "パーソナルデータ生成終了\n"

  for i in 0..monsno_max - 1
    printf( "pms_%03d.bin 生成中\n", i )
    str = sprintf( "pms_%03d.bin", i )
    fp_pms = open( str, "wb" )
    pms = i
    pms_cnt = 0
    while seed[ pms ] != nil
      pms = seed[ pms ]
      pms_cnt += 1
      if pms_cnt > 7
        print "進化データに異常があります\n"
		    exit( 1 )
      end
    end
    fp_pms.write( [ pms ].pack("s") )
    fp_pms.close
    printf( "pms_%03d.bin 生成終了\n", i )
  end

  #タイムスタンプ比較用のダミーファイルを生成
  fp_w = open( "out_end", "w" )
  fp_w.close


