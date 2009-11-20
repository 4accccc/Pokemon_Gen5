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
    PowerData[s].time = line[cell];
    cell += 1;
    PowerData[s].msg_id_title = line[cell];
    cell += 1;
    PowerData[s].msg_id_explain = line[cell];
    cell += 1;
    PowerData[s].type = line[cell];
    cell += 1;
    PowerData[s].data = line[cell];
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
      file.printf("\t\t%s,\t\t//no\n", PowerData[i].no);
      file.printf("\t\t%s,\t\t//level_w\n", PowerData[i].level_w);
      file.printf("\t\t%s,\t\t//level_b\n", PowerData[i].level_b);
      file.printf("\t\t%s,\t\t//time\n", PowerData[i].time);
      file.printf("\t\t%s,\t\t//point\n", PowerData[i].point);
      file.printf("\t\t%s,\t\t//msg_id_title\n", PowerData[i].msg_id_title);
      file.printf("\t\t%s,\t\t//msg_id_explain\n", PowerData[i].msg_id_explain);
      file.printf("\t\t%s,\t\t//type\n", PowerData[i].type);
      file.printf("\t\t%s,\t\t//data\n", PowerData[i].data);
      file.printf("\t},\n");
    end
    
    file.printf("};\n");
  }

end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
