#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		  move_code_check.rb
#   @brief    モデルのmove_codeが正しいパターンで設定されているか調べる
#   @author		Masayuki Onoue
#   @data		2010.11.19
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"
load "./move_code/anm_type_table.rb"
require "mev_analyzer.rb"
require "mev_parser.rb"
require "obj_event_section_parser.rb"
require "obj_event_parser.rb"

outfile = File.open( ARGV[0] , "w" )
spflg = ARGV[1] == "/sp"

ALL_MOVE =  "ANMTYPE_MOVE"
DIR_MOVE =  "ANMTYPE_DIR"
STAY_MOVE=  "ANMTYPE_SP"
SP_MOVE=    "ANMTYPE_NO_USE"

if spflg == false
header =<<EOS
=========================================================

  move_code調査結果(特殊アニメは除外しています)

=========================================================
EOS
outfile.puts header

else
header =<<EOS
=========================================================

  move_code調査結果(特殊アニメを抽出)
  %sが設定されているもの全てを出力しています

=========================================================
EOS
outfile.printf(header,SP_MOVE);

end



mev_files = Array.new

anmtype_dir =["MV_DMY", "MV_LEFT", "MV_RIGHT", "MV_UP", "MV_DOWN", 
  "MV_DIR_RND", "MV_RND_UDL", "MV_RND_UDR", "MV_RND_ULR", "MV_RND_DLR",
  "MV_RND_V", "MV_RND_H", "MV_RND_UL","MV_RND_UR", "MV_RND_DL",
  "MV_RND_DR","MV_SPIN_R" ,"MV_SPIN_L"]

anmtype_stay =["MV_DMY",  "MV_DOWN"]

#EVファイル列挙
all_mev = Dir.glob("../data/*.mev")
all_mev.each{|filename|
 
  # MEVファイル解析
  mev_data = MEVFile.new
  mev_data.load( filename )
  mev_files << mev_data
}


# MOVE_CODEを調べる
mev_files.each{|mev_data|
  
  log = Array.new

  mev_data.obj_section.event.each do |event|


    if spflg == false
      # 全方向移動可能
      if $anm_type[event.OBJ_CODE_Number] == ALL_MOVE
        # 全てのmove_codeが使えます。
      end

      # 4方向のみ(移動できない)
      if $anm_type[event.OBJ_CODE_Number] == DIR_MOVE
        error = true
        anmtype_dir.each do |code|
          if event.MOVE_CODE_Number == code
            error = false
            break
          end
        end
        if error == true
          log << "evname  :"+event.ID_name+"\n"
          log << "対象OBJ :"+event.OBJ_CODE_Number+"\n"
          log << $anm_type[event.OBJ_CODE_Number]+"に"+event.MOVE_CODE_Number+"が設定されています。\n"
          log << "---------------------------------------------------\n"
        end
      end

      # その場のみ
      if $anm_type[event.OBJ_CODE_Number] == STAY_MOVE
        error = true
        dir_error = false
        anmtype_stay.each do |code|
          if event.MOVE_CODE_Number == code

            # ダミーの時はDIR_DOWNが設定されていなければいけない
            if event.MOVE_CODE_Number == "MV_DMY"
              if event.Direction_Type_Number == "DIR_DOWN"
                error = false
              else
                error = false
                dir_error = true
              end
            else
              # それ以外はOK
              error = false
            end
            break
          end
        end
        if error == true
          log << "evname  :"+event.ID_name+"\n"
          log << "対象OBJ :"+event.OBJ_CODE_Number+"\n"
          log << $anm_type[event.OBJ_CODE_Number]+"に"+event.MOVE_CODE_Number+"が設定されています。\n"
          log << "---------------------------------------------------\n"
        end
        if dir_error == true
          log << "evname  :"+event.ID_name+"\n"
          log << "対象OBJ :"+event.OBJ_CODE_Number+"\n"
          log << "その場のみOBJにMV_DMY+"+event.Direction_Type_Number+"が設定されています。\n"
          log << "---------------------------------------------------\n"
        end
      end

    else

      # 特殊アニメ
      if $anm_type[event.OBJ_CODE_Number] == SP_MOVE
        log << "evname    :"+event.ID_name+"\n"
        log << "対象OBJ   :"+event.OBJ_CODE_Number+"\n"
        log << "MOVE_CODE :"+event.MOVE_CODE_Number+"\n"
        log << "DIR       :"+event.Direction_Type_Number+"\n"
        log << "---------------------------------------------------\n"
      end

    end
  end

  unless log.size == 0
    outfile.printf("\n===================================================\n")
    outfile.puts mev_data.filename
    outfile.printf("---------------------------------------------------\n")
    outfile.puts log
  end
}
outfile.close

