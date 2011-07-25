#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  フェスミッションデータ生成コンバーター
#
# @data   11.06.24 
# @author miyuki iwasawa
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定


$KCODE = 'SJIS'

require "../../tools/hash/item_hash.rb"
require "../../tools/hash/monsno_hash.rb"

require "tool/cdef_perser.rb"

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_FILENAME = "demo3d_demoid.h"  #出力ファイル名
DST_BRIEF_DAT = "各アニメテーブルを統合。resource/fldmmdl/anmlist.txtから自動生成"
DST_BRIEF_HEAD = "各アニメヘッダを統合。resource/fldmmdl/anmlist.txtから自動生成"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。"

DST_BRIEF_DATA_DEF_H  = "フェスミッションデータ　リテラル定義ファイル"
DST_NOTE_DATA_DEF_H = "このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。"

DST_BRIEF_MID_H  = "フェスミッションデータID定義ファイル"
DST_NOTE_MID_H = "このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。"

COLUMN_TYPE = 1
COLUMN_ID = 2
COLUMN_MEMO = 3
COLUMN_PARAM = 4
COLUMN_UNIT = 5
COLUMN_END_TYPE = 6
COLUMN_TBONUS = 7
COLUMN_PAY_BONUS = 8
COLUMN_FLAG = 9
COLUMN_LOCATION = 10 
COLUMN_HOST = 11
COLUMN_LV1_TIME = 12
COLUMN_LV1_GOAL = 13
COLUMN_LV1_NUM = 14
COLUMN_LV2_TIME = 15
COLUMN_LV2_GOAL = 16
COLUMN_LV2_NUM = 17
COLUMN_LV3_TIME = 18
COLUMN_LV3_GOAL = 19
COLUMN_LV3_NUM = 20
COLUMN_ITEM = 21
COLUMN_TITLE = 22
COLUMN_EXP = 23

arg = $*

$fpath_out_bin = arg[0]
$fpath_out_def = arg[1]
$fpath_out_mid = arg[2]
$fpath_in_dat_csv = arg[3]
$fpath_in_def_csv = arg[4]
$fpath_in_evflag_h = arg[5]
$fpath_in_msg_h = arg[6]

#構造体定義


$_size_prm = Struct.new("SizeParam",\
  :size, :char_size, :file )

$_list_prm = Struct.new("ListParam",\
	:idx, :f_name, :f_path, :name, :frm_num, :sizeTbl, :comment )

$_def_param = Struct.new("DefineParam",\
  :key, :value, :num, :comment )


#====================================================
# 定義値クラス
#====================================================
class CDefine

  attr  :name
  attr  :prefix
  attr  :type
  attr  :comment
  attr  :p_comment

  def initialize name,prefix,comment,p_comment, type
    @name = name
    @prefix = prefix
    @type = type
    
    @comment = comment
    @p_comment = p_comment
    @list  = Array.new()

    @ct = 0
  end #initialize

  def set key,value,comment
    @list << $_def_param.new
    cp = @list.last

    cp.key = key
    cp.comment = comment

    if value == "---" then
      cp.value = @ct.to_s
      cp.num = @ct
      @ct += 1
    else
      cp.value = value
      cp.num = nil 
    end
  end

  def get_num key
    for n in @list do
      if n.key == key then
        return n.num
      end
    end
    return nil
  end

  def print_enum  fp
    fp.puts("//=========================================")
    fp.puts("///#{@comment}")
    fp.puts("//=========================================")
    fp.puts("typedef enum{")

    for n in @list do
      if n.num != nil then
        fp.puts("  #{@prefix}#{n.key},  ///<#{n.comment}")
      else
        fp.puts("  #{@prefix}#{n.key} = #{n.value},  ///<#{n.comment}")
      end
    end
    fp.puts("}#{@name};\n\n")
  end

  def print_define  fp
    fp.puts("//=========================================")
    fp.puts("///#{@comment}")
    fp.puts("//=========================================")

    for n in @list do
      fp.puts("#define  #{@prefix}#{n.key}  ( #{n.value} )   ///<#{n.comment}")
    end
    fp.puts("\n\n")
  end
end #end of class CDefine


#====================================================
# 定義パース処理クラス
#====================================================
class CDefList
  def initialize f_path
    @list = Array.new()

    fp = File.open( f_path, "r" )
      #Enumリスト取得
      while work = line_get(fp)

        if work[0] == "#ENUM" then
          def_get( fp, "ENUM") 
        elsif work[0] == "#DEFINE" then
          def_get( fp, "DEFINE")
        end
      end 
    fp.close()
  end #initialize

  def line_get fp
    line = fp.gets
#    print(line)
    if line == nil then
      return nil
    end
    line.chomp!
    return line.split(",")
  end

  def def_get fp, type

    work = line_get( fp )
    @list << CDefine.new( work[1],work[2],work[3],work[4], type)
    cp = @list.last()
    end_key = "#"+type+"_END"

    while(1)
      work = line_get( fp )
      if work[0] != nil && work[0] == end_key then
        break
      end
#      print("#{work[1]} -> #{work[3]}\n")
      cp.set( work[1], work[2], work[3] ) 
    end
  end 

  def search key, err_msg
    for n in @list do
      num = n.get_num( key )
      if num != nil then
        return num
      end
    end
    print("Error! #{err_msg} -> #{key}")
    exit 1
  end

  def output_header f_path

    File::open( f_path, "w" ){  |file|
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{File.basename(f_path)}");
      file.puts(" * @brief\t" + DST_BRIEF_DATA_DEF_H);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("#ifndef __H_FMISSION_DATA_DEF_H__\n");
      file.puts("#define __H_FMISSION_DATA_DEF_H__\n\n");

      file.puts("#ifndef __ASM_NO_DEF_  //ここからアセンブラでは無視\n\n")

      prev = 0
      for n in @list do

        if n.type == "ENUM" then
          n.print_enum( file )
        else
          if prev == 0 then
            file.puts("#endif //__ASM_NO_DEF_\n\n")
            prev = 1
          end
          n.print_define( file )
        end
      end
      
      file.puts("\n\n#endif //__H_FMISSION_DATA_DEF_H__\n\n");
    }
  end
end #end of class CDefList

#====================================================
# 主処理 開始
#====================================================
class CMission

  attr  :id

  def print_mission_id fp, prefix, idx
    fp.puts("#define  #{prefix}_#{@type_str}_#{@id}  ( #{idx} )  ///<#{@memo}\n")
  end

  #ミッションデータバイナリ出力
  def print_bin fp
    buf = Array.new

    #u8*6 ミッションタイプ、クリアタイプ、ボーナスタイム、範囲、開催制限、表示単位
    buf[0] = @type
    buf[1] = @end_type
    buf[2] = @location
    buf[3] = @host
    buf[4] = @unit 
    buf[5] = 0  #パディング 
    fp.write( buf.pack("C6") )

    #u16*3 汎用パラメータ、参加条件フラグID,報酬アイテム
    buf[0] = @param
    buf[1] = @flag
    buf[2] = @item
    print("mission_param = #{@param}, #{@flag}, #{@item}\n")
    fp.write( buf.pack("S3"))

    #u8 * 4 報酬ボーナスタイプ、タイトルメッセージ、説明文、パディング
    buf[0] = @time_bonus
    buf[1] = @pay_bonus
    buf[2] = @title
    buf[3] = @exp
    fp.write( buf.pack("C4") )
    
    #(u16,u8,u8) * 3 レベルごとの制限時間と目標と報酬数
    fp.write( @level.pack("SCCSCCSCC") )

  end

  def line_get fp
    line = fp.gets
    line.chomp!
    return line.split(",")
  end

  def print_err err_str, word
    print("Error! #{err_str} -> #{word}\n")
    exit 1
  end

  def initialize work, c_def_list, c_evflag_list, c_msg_list
    @type = c_def_list.search( work[COLUMN_TYPE], "ミッションタイプ")
    @type_str = work[COLUMN_TYPE]
    @id = work[COLUMN_ID]
    print("missionid = #{@type_str}->#{@type}, #{@id}, #{work[COLUMN_PARAM]}\n")
    @memo = work[COLUMN_MEMO] 

    param = work[COLUMN_PARAM]
    case @type_str
    when "POKEBTL"
      @param = $monsno_hash.fetch( param, 0 )
      print("type POKEBTL\n")
    when "EVENT"
      @param = c_def_list.search( param, "パラメータ->イベントID")
      print("type EVENT\n")
    when "ITEM"
      @param = 0
      print("type ITEM\n")
    when "NPC"
      @param = 0
      print("type NPC\n")
    when "TRBTL"
      @param = 0
      print("type TRBTL\n")
    end
    @unit = c_def_list.search( work[COLUMN_UNIT], "単位系")
    @end_type = c_def_list.search( work[COLUMN_END_TYPE], "終了条件" ) 
    @time_bonus = work[COLUMN_TBONUS].to_i
    @pay_bonus = c_def_list.search( work[COLUMN_PAY_BONUS], "報酬ボーナスタイプ" )

    ev_flag = work[COLUMN_FLAG]
    if ev_flag.to_i == 0 then
      @flag = 0
    else
      @flag = c_evflag_list.searchID( ev_flag )
      if @flag == nil then
        print_err("参加条件イベントフラグが不正です", ev_flag )
      end
    end
    @location = c_def_list.search( work[COLUMN_LOCATION], "範囲" )
    @host = c_def_list.search( work[COLUMN_HOST], "開催制限" )

    @level = Array.new
    for i in 0..3 do
      @level[i*3+0] = work[COLUMN_LV1_TIME+(i*3)].to_i
      @level[i*3+1] = work[COLUMN_LV1_GOAL+(i*3)].to_i
      @level[i*3+2] = work[COLUMN_LV1_NUM+(i*3)].to_i
    end

    @item = $item_hash.fetch(work[COLUMN_ITEM],nil)
    if @item == nil then
      print_err("アイテム名が不正です",work[COLUMN_ITEM])
    end

    @title = c_msg_list.searchID( work[COLUMN_TITLE] )
    if @title == nil then
      print_err("タイトルラベルが不正です", work[COLUMN_TITLE])
    end
    @exp = c_msg_list.searchID( work[COLUMN_EXP] )
    if @exp == nil then
      print_err("説明文ラベルが不正です",work[COLUMN_EXP])
    end
  end
end #end of CMission

class CMissionList

  def initialize
  end

  def make_mission_list f_path, c_def_list, c_evflag_list, c_msg_list
    File::open( f_path ,"r"){ |file|
      @mission = Array.new
      #一行読み飛ばし
      file.gets()
      while line = file.gets()
        if line.slice(/#END/) != nil then
          break
        end
        line.chomp!()
        
        @mission << CMission.new( line.split(","), c_def_list, c_evflag_list, c_msg_list )
      end
    }
  end

  def output_mission_id f_path
    File::open( f_path, "w" ){  |file|
      
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{File.basename(f_path)}");
      file.puts(" * @brief\t" + DST_BRIEF_MID_H);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("#ifndef __H_FMISSION_DATA_ID_H__\n");
      file.puts("#define __H_FMISSION_DATA_ID_H__\n\n");
    
      idx = 0
      for m in @mission do
#        print("mid=#{m.id}\n")
        m.print_mission_id( file, "FMISSION_ID",idx )
        idx += 1 
      end
      file.puts("#define  FMISSION_ID_MAX  ( #{idx} )\n")
      file.puts("#define  FMISSION_ID_END  ( FMISSION_ID_MAX-1 )\n")
      file.puts("#define  FMISSION_ID_NULL ( 0xFF )\n\n")
      file.puts("#ifndef __ASM_NO_DEF_  //ここからアセンブラでは無視\n\n")
      file.puts("typedef u8 FEST_MISSION_ID;\n\n")
      file.puts("#endif //__ASM_NO_DEF_\n\n")
      file.puts("#endif //__H_FMISSION_DATA_ID_H__\n\n");
    }
  end

  def output_mission_bin f_path
    File::open( f_path, "wb"){  |file|
      for m in @mission do
        m.print_bin( file )
      end
    }
  end

end #end of CMissionList

def convert
  #エクセル#defineリスト取得
  c_def_list = CDefList.new( $fpath_in_def_csv )
  #イベントフラグ定義ヘッダ取得
  c_evflag_list = CDef_Perser.new( $fpath_in_evflag_h, "evflag_log.txt")
  #メッセージIDヘッダ取得
  c_msg_list = CDef_Perser.new( $fpath_in_msg_h, "msg_log.txt")

  # c_evflag_list.printMember()
  # c_msg_list.printMember()
  
  #ミッションデータ取得 
  c_mission = CMissionList.new()
  c_mission.make_mission_list( $fpath_in_dat_csv, c_def_list, c_evflag_list, c_msg_list )

  #出力
  c_mission.output_mission_id( $fpath_out_mid )
  c_mission.output_mission_bin( $fpath_out_bin )
 
  #定義出力
  c_def_list.output_header( $fpath_out_def )
end

convert()
