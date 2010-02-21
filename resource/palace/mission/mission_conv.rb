=begin
実行手順
ruby mission_conv.rb [CSVファイルへのパス]

出力ファイル
mission_data.cdat
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
class MISSION_DATA
  def initialize()
    @mission_no = "0"                  #ミッション番号
    @type = "0"                        #ミッション系統
    @level = "0"                       #発生レベル
    @odds = "0"                        #発生率
    @msg_id_contents = "0"            #ミッション内容gmmのmsg_id
    @msg_id_contents_monolith = "0"   #ミッション内容モノリスgmmのmsg_id
    
    @obj_id = [];
    for i in 0..OBJID_MAX
      @obj_id[i] = "BOY1";      #変化OBJ ID
    end

    @obj_sex = [];
    for i in 0..OBJID_MAX
      @obj_sex[i] = "男";       #変化OBJの性別
    end

    @talk_type = [];
    for i in 0..OBJID_MAX
      @talk_type[i] = "可愛い"  #変化OBJの会話タイプ
    end
    
    @data = [];
    for i in 0..DATA_MAX
      @data[i] = "0";        #データ(ミッション系統毎に扱いが変化)
    end
    
    @time = "0"           #時間(秒)
    
    @reward = [];
    for i in 0..REWARD_MAX
      @reward[i] = "0";      #報酬 0(1位) ～ 3(4位)
    end
    
    @confirm_type = "0"   #ミッション確認タイプ
    @limit_run = "0"       #制限走り　TRUE:走り禁止
    @limit_talk = "0"      #制限話　TRUE:話禁止
  end

  attr_accessor :mission_no, :type, :level, :odds, :msg_id_contents, :msg_id_contents_monolith;
  attr_accessor :obj_id, :obj_sex, :talk_type, :data, :time, :reward, :confirm_type;
  attr_accessor :limit_run, :limit_talk;
end


MissionData = [];


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
    MissionData[s] = MISSION_DATA.new();

    MissionData[s].mission_no = line[cell];
    cell += 1;
    MissionData[s].type = line[cell];
    cell += 1;
    MissionData[s].level = line[cell];
    cell += 1;
    MissionData[s].odds = line[cell];
    cell += 1;
    MissionData[s].msg_id_contents = line[cell];
    cell += 1;
    MissionData[s].confirm_type = line[cell];
    cell += 1;
    MissionData[s].msg_id_contents_monolith = line[cell];
    cell += 1;

    for g in 0..OBJID_MAX-1
      MissionData[s].obj_id[g] = line[cell];
      cell += 1;
      MissionData[s].obj_sex[g] = line[cell];
      cell += 1;
      MissionData[s].talk_type[g] = line[cell];
      cell += 1;
    end
    
    for g in 0..DATA_MAX-1
      MissionData[s].data[g] = line[cell];
      cell += 1;
    end
    
    MissionData[s].time = line[cell];
    cell += 1;

    for g in 0..REWARD_MAX-1
      MissionData[s].reward[g] = line[cell];
      cell += 1;
    end
    
    MissionData[s].limit_run = line[cell];
    cell += 1;
    MissionData[s].limit_talk = line[cell];
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
  for i in 0..MissionData.size-1
    
    MissionData[i].type = "MISSION_TYPE_" + MissionData[i].type;
    MissionData[i].confirm_type = "CONFIRM_" + MissionData[i].confirm_type;
    
    for s in 0..OBJID_MAX-1
      if(MissionData[i].obj_id[s] =~ /.*-.*/)
        MissionData[i].obj_id[s] = "0";
      end
    end

    for s in 0..OBJID_MAX-1
      if(MissionData[i].obj_sex[s] =~ /男/)
        MissionData[i].obj_sex[s] = "0";
      elsif(MissionData[i].obj_sex[s] =~ /女/)
        MissionData[i].obj_sex[s] = "1";
      else
        MissionData[i].obj_sex[s] = "0";
      end
    end

    for s in 0..OBJID_MAX-1
      if(MissionData[i].talk_type[s] =~ /可愛い/)
        MissionData[i].talk_type[s] = "TALK_TYPE_CUTE";
      elsif(MissionData[i].talk_type[s] =~ /怖い/)
        MissionData[i].talk_type[s] = "TALK_TYPE_FRIGHTENING";
      elsif(MissionData[i].talk_type[s] =~ /無口/)
        MissionData[i].talk_type[s] = "TALK_TYPE_RETICENCE";
      elsif(MissionData[i].talk_type[s] =~ /人間男/)
        MissionData[i].talk_type[s] = "TALK_TYPE_MAN";
      elsif(MissionData[i].talk_type[s] =~ /人間女/)
        MissionData[i].talk_type[s] = "TALK_TYPE_WOMAN";
      end
    end
    
    for s in 0..DATA_MAX-1
      if(MissionData[i].data[s] =~ /.*-.*/)
        MissionData[i].data[s] = "0";
      end
    end
    
    for s in 0..REWARD_MAX-1
      if(MissionData[i].reward[s] =~ /.*-.*/)
        MissionData[i].reward[s] = "0";
      end
    end
    
    if(MissionData[i].limit_run =~ /.*-.*/)
      MissionData[i].limit_run = "FALSE";
    elsif(MissionData[i].limit_run =~ /.*○.*/)
      MissionData[i].limit_run = "TRUE";
    end

    if(MissionData[i].limit_talk =~ /.*-.*/)
      MissionData[i].limit_talk = "FALSE";
    elsif(MissionData[i].limit_talk =~ /.*○.*/)
      MissionData[i].limit_talk = "TRUE";
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
  
  File.open("mission_data.cdat", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       mission_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("const MISSION_CONV_DATA MissionData[] = {\n");

    for i in 0..MissionData.size-1
      file.printf("\t{\t//%d\n", i);
      file.printf("\t\t%s,\t\t//mission_no\n", MissionData[i].mission_no);
      file.printf("\t\t%s,\t\t//type\n", MissionData[i].type);
      file.printf("\t\t%s,\t\t//level\n", MissionData[i].level);
      file.printf("\t\t%s,\t\t//odds\n", MissionData[i].odds);
      file.printf("\t\t%s,\t\t//msg_id_contents\n", MissionData[i].msg_id_contents);
      file.printf("\t\t%s,\t\t//msg_id_contents_monolith\n", MissionData[i].msg_id_contents_monolith);

      file.printf("\t\t{");
      for s in 0..OBJID_MAX-1
        file.printf("%s,", MissionData[i].obj_id[s]);
      end
      file.printf("},\t\t//obj_id\n");

      file.printf("\t\t{");
      for s in 0..OBJID_MAX-1
        file.printf("%s,", MissionData[i].obj_sex[s]);
      end
      file.printf("},\t\t//obj_sex\n");

      file.printf("\t\t{");
      for s in 0..OBJID_MAX-1
        file.printf("%s,", MissionData[i].talk_type[s]);
      end
      file.printf("},\t\t//talk_type\n");

      file.printf("\t\t{");
      for s in 0..DATA_MAX-1
        file.printf("%s,", MissionData[i].data[s]);
      end
      file.printf("},\t\t//data\n");
      
      file.printf("\t\t%s,\t\t//time\n", MissionData[i].time);

      file.printf("\t\t{");
      for s in 0..REWARD_MAX-1
        file.printf("%s,", MissionData[i].reward[s]);
      end
      file.printf("},\t\t//reward\n");

      file.printf("\t\t%s,\t\t//confirm_type\n", MissionData[i].confirm_type);
      file.printf("\t\t%s,\t\t//limit_run\n", MissionData[i].limit_run);
      file.printf("\t\t%s,\t\t//limit_talk\n", MissionData[i].limit_talk);
      file.printf("\t\t0,\t\t//padding\n");

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
