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
    @gpower_id = "0"           #GパワーID
    @comment = "0"             #効果説明
    @type = "0"                #効果系統
    @data = "0"                #効果データ
    @enable_f = "0"            #有効無効フラグ
  end

  attr_accessor :no, :level_w, :level_b, :point, :time;
  attr_accessor :msg_id_title, :msg_id_explain, :type, :data, :gpower_id, :comment, :enable_f;
end


PowerData = [];

POWER_TYPE_ARRAY_MAX = 100;
PowerType = [];
$DummyStartNo = 0;
$DummyEndNo = -1;
$PalaceStartNo = 0;
$PalaceEndNo = -1;
$DistributionStartNo = 0;
$DistributionEndNo = -1;
$DistributionStartNoSw = 0;
$DistributionEndNoSw = -1;

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
    cell += 2;
    PowerData[s].gpower_id = line[cell];
    cell += 1;
    PowerData[s].comment = line[cell];
    cell += 1;
    PowerData[s].time = line[cell];
    cell += 1;
    PowerData[s].enable_f = line[cell];
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
  
  distribution_start = 0;
  distribution_start_sw = 0;
  palace_start = 0;
  dummy_start = 0;
  
  for i in 0..PowerData.size-1
    if(PowerData[i].type == "ENCOUNT_UP" || PowerData[i].type == "ENCOUNT_DOWN" \
        || PowerData[i].type == "HATCH_UP" || PowerData[i].type == "EXP_UP" \
        || PowerData[i].type == "EXP_DOWN" || PowerData[i].type == "MONEY_UP" \
        || PowerData[i].type == "SALE" || PowerData[i].type == "CAPTURE_UP")
      num = PowerData[i].data.to_f;
      num *= 100; #小数を整数に変換する(100倍の固定小数)
      #PowerData[i].data = (0x100 * num / 100).to_i;  #下位8bit固定小数にさらに変換
      PowerData[i].data = num.to_i;  #下位8bit固定小数にさらに変換
    end

    if(PowerData[i].enable_f == "×")
      if($DummyEndNo != -1)
        print "ダミーが連番になっていない"
        print i;
        print "\n"
        exit(200);
      end
      if(dummy_start == 0)
        $DummyStartNo = i
        dummy_start = 1
      end
    elsif(dummy_start == 1 && $DummyEndNo == -1)
      $DummyEndNo = i - 1;
    end

    if(PowerData[i].level_w != "配布" && PowerData[i].level_b != "配布" \
        && PowerData[i].level_w != "パレス" && PowerData[i].level_b != "パレス")
=begin
      if(palace_start != 0)
        print "パレスの後にノーマルのGパワーが設定されている"
        print i;
        print "\n"
        exit(200);
      end
=end
      if(distribution_start_sw != 0)
        print "配布の後にノーマルのGパワーが設定されている"
        print i;
        print "\n"
        exit(200);
      end
    end
    
    if(PowerData[i].level_w == "配布" || PowerData[i].level_b == "配布")
=begin
      if(palace_start != 0 )
        print "配布よりも先 or 途中で パレスが設定されている\n"
        exit(200);
      end
=end
      if( $DistributionEndNo != -1 )
        print "BW配布より後に配布が設定されている\n"
        exit(200);
      end

      if(distribution_start == 0)
        $DistributionStartNo = i;
        distribution_start = 1;
      elsif(distribution_start_sw == 0 && $DistributionEndNo != -1)
        $DistributionStartNoSw = i
        distribution_start_sw = 1
      end
      PowerData[i].level_w = "POWER_LEVEL_DISTRIBUTION";
      PowerData[i].level_b = "POWER_LEVEL_DISTRIBUTION";
    else
      if(distribution_start == 1 && $DistributionEndNo == -1)
        $DistributionEndNo = i - 1;
      end
      if(distribution_start_sw == 1 && $DistributionEndNoSw == -1 )
        $DistributionEndNoSw = i - 1;
      end
    end
    
    if(PowerData[i].level_w == "パレス" || PowerData[i].level_b == "パレス")
      if(palace_start == 0)
        $PalaceStartNo = i;
        palace_start = 1;
      end
      PowerData[i].level_w = "POWER_LEVEL_PALACE";
      PowerData[i].level_b = "POWER_LEVEL_PALACE";
    else
      if(palace_start == 1 && $PalaceEndNo == -1)
        $PalaceEndNo = i - 1;
      end
    end
  end
  if($PalaceEndNo == -1)
    $PalaceEndNo = PowerData.size - 1;
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
      file.printf("\t\tGPOWER_TYPE_%s,\t\t//type\n", PowerData[i].type);
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
    
    for i in 0..PowerData.size-1
      file.printf("\t%s,\t//%d : %s\n", PowerData[i].gpower_id, i, PowerData[i].comment);
    end
   
    if $DistributionEndNoSw == -1 then
      $DistributionEndNoSw = PowerData.size-1
    end
    file.printf("\n");
    file.printf("\tGPOWER_ID_DISTRIBUTION_START = %d,\t\t//配布Gパワー開始ID(このIDを含む)\n", $DistributionStartNo);
    file.printf("\tGPOWER_ID_DISTRIBUTION_END = %d,\t\t//配布Gパワー終了ID(このIDを含む)\n\n", $DistributionEndNo);
    if $DistributionStgartNoSw == 0 then
      file.printf("\tGPOWER_ID_DISTRIBUTION_START_SW = %d,\t\t//SWAN配布Gパワー開始ID(このIDを含む)\n", $DistributionStartNoSw);
      file.printf("\tGPOWER_ID_DISTRIBUTION_END_SW = %d,\t\t//SWAN配布Gパワー終了ID(このIDを含む)\n\n", $DistributionEndNoSw);
    end
#   file.printf("\tGPOWER_ID_PALACE_START = %d,\t\t//パレスGパワー開始ID(このIDを含む)\n", $PalaceStartNo); #この定義は未使用なので削除111128
#   file.printf("\tGPOWER_ID_PALACE_END = %d,\t\t//パレスGパワー終了ID(このIDを含む)\n\n", $PalaceEndNo); #この定義は未使用なので削除11128
    file.printf("\tGPOWER_ID_DUMMY_START = %d,\t\t//BWの未実装パワー開始ID(このIDを含む)\n", $DummyStartNo);
    file.printf("\tGPOWER_ID_DUMMY_END = %d,\t\t//BWの未実装パワー終了ID(このIDを含む)\n\n", $DummyEndNo);
    file.printf("\tGPOWER_ID_MAX = %d,\t\t//Gパワー最大数\n", PowerData.size);
    #file.printf("\tGPOWER_ID_NULL = GPOWER_ID_MAX,\t\t//Gパワーが発動していない\n");
    file.printf("}GPOWER_ID;\n\n");
    
    file.printf("\n\n//効果系統　　!!出力元データによってenumの並びが変わる場合があります!!\n");
    file.printf("typedef enum{\n");
    for i in 0..POWER_TYPE_ARRAY_MAX
      if(PowerType[i] == "NULL")
        break;
      end
      file.printf("\tGPOWER_TYPE_%s,\t\t//%d\n", PowerType[i], i);
    end
    file.printf("\n\tGPOWER_TYPE_MAX,\n");
    file.printf("\tGPOWER_TYPE_NULL = GPOWER_TYPE_MAX,\n");
    file.printf("}GPOWER_TYPE;\n\n");
  }
  
end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
