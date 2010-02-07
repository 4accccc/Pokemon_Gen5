=begin
実行手順
ruby power_conv.rb [CSVファイルへのパス]

出力ファイル
power_data.cdat
=end


#--------------------------------------------------------------
#  定数定義
#--------------------------------------------------------------
OBJID_MAX = 4;
DATA_MAX = 3;
REWARD_MAX = 4;

#--------------------------------------------------------------
#  グローバル変数
#--------------------------------------------------------------
InputExcelFile = ARGV[0];


#--------------------------------------------------------------
#  ミッションデータ
#--------------------------------------------------------------
class POWER_DATA
  def initialize()
    @no = "0"                  #パワー番号
    @level_w = "0"             #必要Whiteレベル
    @level_b = "0"             #必要Blackレベル
    @point = "0"               #使用ポイント
    @time = "0"                #継続時間(分)
    @msg_id_title = "0"        #パワー項目msg_id
    @msg_id_explain = "0"      #パワー説明msg_id
    @type = "0"                #効果系統
    @data = "0"                #効果データ
  end

  attr_accessor :no, :level_w, :level_b, :point, :time;
  attr_accessor :msg_id_title, :msg_id_explain, :type, :data;
end


PowerData = [];

POWER_TYPE_ARRAY_MAX = 100;
PowerType = [];

=begin
//==================================================================
/**
 * Excel(カンマ区切り)ファイルを開き行単位でチェック
 *
 * @param   sp_array		
 *
 * @retval  def		
 */
//==================================================================
=end
def CsvConvFileCheck()
  
  s = 0;
  title = 0;
  
  require 'csv'
  csv = CSV.open(InputExcelFile, 'r');
  csv.each_with_index{|line, i|
    if ((/^#END/ =~ line[0]) != nil)
      break;
    end
    if ((/^no/ =~ line[0]) != nil)
      title += 1;
      next;
    end
    if(title == 0)
      next;
    end

    cell = 0;
    PowerData[s] = POWER_DATA.new();

    PowerData[s].no = line[cell];
    cell += 1;
    PowerData[s].level_w = line[cell];
    cell += 1;
    PowerData[s].level_b = line[cell];
    cell += 1;
    PowerData[s].point = line[cell];
    cell += 1;
    PowerData[s].msg_id_title = line[cell];
    cell += 1;
    PowerData[s].msg_id_explain = line[cell];
    cell += 1;
    PowerData[s].type = line[cell];
    cell += 1;
    PowerData[s].data = line[cell];
    cell += 3;
    PowerData[s].time = line[cell];
    cell += 1;

    s += 1;
  }
  
end


=begin
//==================================================================
/**
 * 読み込んだファイルを出力形式に合わせて変換
 */
//==================================================================
=end
def DataConv()
  for i in 0..PowerData.size-1
    if(PowerData[i].type == "1" || PowerData[i].type == "2" || PowerData[i].type == "3" \
        || PowerData[i].type == "9" || PowerData[i].type == "10" || PowerData[i].type == "11")
      num = PowerData[i].data.to_f;
      num *= 100; #小数を整数に変換する(100倍の固定小数)
      PowerData[i].data = (0x100 * num / 100).to_i;  #下位8bit固定小数にさらに変換
    end
  end
  
  #効果系統を抽出
  power_type_count = 0;
  for i in 0..POWER_TYPE_ARRAY_MAX 
    PowerType[i] = "NULL";
  end
  for i in 0..PowerData.size-1
    type = PowerData[i].type;
    for s in 0..POWER_TYPE_ARRAY_MAX 
      if(type == PowerType[s])
        break;
      end
      if(PowerType[s] == "NULL")
        PowerType[s] = type;
        power_type_count += 1;
        break;
      end
    end
  end
end


=begin
//==================================================================
/**
 * 読み込んだファイルをCデータとして出力
 */
//==================================================================
=end
def DataFileOutput()
  
  File.open("power_data.cdat", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       power_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("const POWER_CONV_DATA PowerData[] = {\n");

    for i in 0..PowerData.size-1
      file.printf("\t{\t//%d\n", i);
      file.printf("\t\t%s,\t\t//level_w\n", PowerData[i].level_w);
      file.printf("\t\t%s,\t\t//level_b\n", PowerData[i].level_b);
      file.printf("\t\t%s,\t\t//point\n", PowerData[i].point);
      file.printf("\t\t%s,\t\t//msg_id_title\n", PowerData[i].msg_id_title);
      file.printf("\t\t%s,\t\t//msg_id_explain\n", PowerData[i].msg_id_explain);
      file.printf("\t\t%s,\t\t//data\n", PowerData[i].data);
      file.printf("\t\t%s,\t\t//type\n", PowerData[i].type);
      file.printf("\t\t%s,\t\t//time\n", PowerData[i].time);
      file.printf("\t},\n");
    end
    
    file.printf("};\n");
  }

  File.open("gpower_id.h", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       power_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("#pragma once\n\n");
    
    file.printf("typedef enum{\n");
    file.printf("\tGPOWER_ID_MAX = %d,\t\t//Gパワー最大数\n", PowerData.size);
    file.printf("\tGPOWER_ID_NULL = GPOWER_ID_MAX,\t\t//Gパワーが発動していない\n");
    file.printf("}GPOWER_ID;\n\n");
    
    file.printf("\n\n//効果系統\n");
    file.printf("typedef enum{\n");
    file.printf("\tGPOWER_TYPE_NULL,\n");
    for i in 0..POWER_TYPE_ARRAY_MAX
      if(PowerType[i] == "NULL")
        break;
      end
      file.printf("\tGPOWER_TYPE_%s,\n", PowerType[i]);
    end
    file.printf("\tGPOWER_TYPE_MAX,\n");
    file.printf("}GPOWER_TYPE;\n\n");
  }
  
end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
