#============================================================================
#
#
#   スクリプトフラグ/ワーク管理システム
#
#
#   2009.07.14  tamada GAMEFREAK inc.
#
#   2009.12.06  デバッグシンボル生成機能を追加
#
#============================================================================
$KCODE= "SJIS"

class FlagReadError < Exception; end

XLS2TAB = "ruby #{ENV["PROJECT_ROOT"]}tools/exceltool/xls2xml/tab_out.rb -c "

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def read_excel_file( filename )
  command = "#{XLS2TAB} #{filename}"
  tmpfile = `#{command}`.split("\n")
  return tmpfile
end

#============================================================================
#============================================================================
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
class PMFlagData

  COL_NUMBER = 0
  COL_NAME = 1
  COL_ACTIVE = 2
  COL_COMMENT = 3

  #----------------------------------------------------------------------------
  #----------------------------------------------------------------------------
  class PMFlag
    attr_reader :name, :comment
     def initialize(name, comment, access)
       @name = name
       @comment = comment
       @access = access
     end
  end

  attr_reader  :filename, :prefix, :start_no, :max, :count

  #----------------------------------------------------------------------------
  #----------------------------------------------------------------------------
  def initialize(filename, prefix, start_no, max)
    @filename = filename
    @prefix = prefix
    @start_no = start_no
    @max = max

    STDERR.puts "MAKE FLAGS #{filename}, #{prefix}, #{start_no} - #{max}"
    @flags = Array.new
    @count = 0

    if filename != nil then
      @tmpfile = read_excel_file( @filename )
      read(@tmpfile)
    else
      @max.times{|n|
        name = @prefix + sprintf("%03d", n)
        @flags << PMFlag.new(name, "領域確保による自動生成", 0)
      }
    end
  end

  def raiseError(string)
    raise FlagReadError, "#{@filename}:#{@count}:\n #{string}"
  end

  #----------------------------------------------------------------------------
  #メソッド：読み込み
  #----------------------------------------------------------------------------
  def read(lines)
    column = lines.shift.split(",")
    raiseError "フォーマット異常です.#{column[COL_NUMBER]}" if column[COL_NUMBER] != "number"
    raiseError "フォーマット異常です.#{column[COL_NAME]}" if column[COL_NAME] != "name"
    raiseError "フォーマット異常です.#{column[COL_ACTIVE]}" if column[COL_ACTIVE] != "active"
    raiseError "フォーマット異常です.#{column[COL_COMMENT]}" if column[COL_COMMENT] != "comment"

    lines.each_with_index{|line, index|
      column = line.split(",")
      if line =~ /^#END/ then
        break
      end
      begin
        number = Integer(column[COL_NUMBER])
      rescue
        puts "#{line}"
        raiseError("numberに数値でない値#{column[COL_NUMBER]}が指定されています")
      end
      name = column[COL_NAME]
      active = column[COL_ACTIVE]
      comment = column[COL_COMMENT]
      if number != @count then
        raiseError "No(#{number})が間違っています。正しくは#{@count}です。\n#{line}"
      end
      if @count >= @max then
        raiseError "要素数が多すぎます MAX=#{@max}\n"
      end
      @count += 1
      if active == "×" then
        @flags << PMFlag.new(nil, comment, 0)
      elsif active != "○" then
        raiseError "activeに○×以外の要素\"#{active}\"が入っています.\n#{line}"
      elsif name =~/^#{@prefix}/ then
        @flags << PMFlag.new(name, comment, 0)
      else
        raiseError "名称が間違っています。#{@prefix}が含まれていません。\n#{line}"
      end
    }
  end

  #----------------------------------------------------------------------------
  #メソッド：書き出し
  #----------------------------------------------------------------------------
  def write_define()
    output = ""
    output += "/* flags \"#{@filename}\" #{@start_no} ～ #{@start_no + @max - 1} */\n"
    output += "\n"
    output += sprintf("\#define %-32s %5d /* 0x%04x %s */\n", 
                      @prefix + "AREA_START", @start_no, @start_no, nil)
    output += "\n"
    @flags.each_with_index{ |flag, index|
      number = @start_no + index
      name = flag.name
      if name == nil then
        output += sprintf("\t\t/* not used %5d(0x%04x) */\n", number, number)
      else
        output += sprintf("\#define %-32s %5d /* 0x%04x %s */\n", flag.name, number, number, flag.comment)
      end
    }
    output += "\n"
    output += sprintf("\#define %-32s %5d /* 0x%04x %s */\n", 
                      @prefix + "AREA_END", @start_no + @max - 1, @start_no + @max - 1, nil)
    output += "\n\n"
    return output
  end

  #----------------------------------------------------------------------------
  #メソッド：バイナリ書き出し
  #----------------------------------------------------------------------------
  def write_binary()
    output = ""
    @flags.each_with_index{| flag, index |
      next if flag.name == nil
      raiseError "名称（#{flag.name}）が長すぎます\n" if flag.name.length > 29
      output += [@start_no + index].pack("S")
      output += [flag.name].pack("a29x")
    }
    return output
  end

end

#============================================================================
#============================================================================
class FLAGMANAGER

  COL_FILENAME = 0
  COL_STARTNO = 1
  COL_MAX = 2
  COL_PREFIX = 3
  COL_DEBUGNAME = 4

  attr_reader :manager_filename, :tmpfile, :flagdatas

  #----------------------------------------------------------------------------
  #----------------------------------------------------------------------------
  def initialize(filename)
    @manager_filename = filename
    @tmpfile = read_excel_file( @manager_filename )
    @flagdatas = Array.new
    @debugfiles = Hash.new
    read_list(@tmpfile)
    check()
  end

  #----------------------------------------------------------------------------
  #エラー処理
  #----------------------------------------------------------------------------
  def raiseError(string, count)
    raise FlagReadError, "#{@manager_filename}:#{count}:\n #{string}"
  end

  #----------------------------------------------------------------------------
  #読み込み処理
  #----------------------------------------------------------------------------
  def read_list(lines)
    linecount = 1
    column = lines.shift.split(",")
    raiseError("ファイルフォーマット異常：",linecount) if column[COL_FILENAME] != "管理ファイル名"
    raiseError("ファイルフォーマット異常：",linecount) if column[COL_STARTNO] != "開始ナンバー"
    raiseError("ファイルフォーマット異常：",linecount) if column[COL_MAX] != "最大数"
    raiseError("ファイルフォーマット異常：",linecount) if column[COL_PREFIX] != "プレフィックス指定"
    raiseError("ファイルフォーマット異常：",linecount) if column[COL_DEBUGNAME] != "デバッグリスト名"

    next_pos = 0
    lines.each{ |line|
      linecount += 1
      column = line.split(",")
      if column.size < 4 || column[0] == "\#END" then break end

      filename = column[COL_FILENAME]
      prefix = column[COL_PREFIX]
      if column[COL_STARTNO] == "auto" then
        start_no = next_pos
      else
        start_no = Integer(column[COL_STARTNO])
      end
      begin 
        max = Integer(column[COL_MAX])
      rescue
        raiseError("最大値設定異常", linecount)
      end
      next_pos = start_no + max

      if filename != "-" then
        @flagdatas << PMFlagData.new(filename, prefix, start_no, max)
      else
        @flagdatas << PMFlagData.new(nil, prefix, start_no, max)
      end
      debug_id = column[COL_DEBUGNAME]
      if debug_id != "-" then
        if @debugfiles.has_key?( debug_id ) then
          @debugfiles[ debug_id ] << @flagdatas.last
        else
          @debugfiles[ debug_id ] = [ @flagdatas.last ]
        end
      end
    }
  end

  #----------------------------------------------------------------------------
  #パラメータチェック
  #----------------------------------------------------------------------------
  def check()
    prefixes = Hash.new

    before_tail = -1
    @flagdatas.each_with_index{|flagdata, index|
      #プレフィックスの重複チェック（エラー）
      prefix = flagdata.prefix
      if prefixes.has_key?(prefix) then
        raiseError("プレフィックスが重なっています。#{prefix}", index)
      else
        prefixes[prefix] = nil
      end
      #使用領域の重複チェック（エラー）
      n_min = flagdata.start_no
      if before_tail < n_min then
        before_tail = n_min + flagdata.max - 1
      else
        raiseError("#{flagdata.filename}:使用するナンバーが前の領域に重なっています. \
        before tail = #{before_tail}, \
                   now start = #{n_min}", index)
      end
      #使用領域の定義チェック（警告）
      if flagdata.count < flagdata.max && flagdata.filename != nil then
        STDERR.puts "警告：#{flagdata.filename}"
        STDERR.puts "警告：要素最大まで定義されていません。max=#{flagdata.max}, count=#{flagdata.count}"
      end
    }

  end

  #----------------------------------------------------------------------------
  # 定義の書き出し
  #----------------------------------------------------------------------------
  def write_defines()
    sym = File.basename(@manager_filename,".*").upcase
    flag_min = -1
    flag_max = 0
    output_content = ""
    @flagdatas.each{|flagdata|
      output_content += flagdata.write_define()
      if flag_min < 0 then flag_min = flagdata.start_no end
      flag_max = flagdata.start_no + flagdata.max
    }
    output = ""
    output += "/* 自動生成フラグ定義ファイル 開始*/"
    output += "\#pragma once"
    output += "\n\n"
    output += sprintf("\#define %s_TOTAL_MIN  %5d /* 0x%04x */\n",sym, flag_min, flag_min)
    
    output += output_content

    output += "\n\n"
    output += sprintf("\#define %s_TOTAL_MAX  %5d /* 0x%04x */\n",sym, flag_max, flag_max)
    output += "\n\n"
    output += "/* 自動生成フラグ定義ファイル 終了*/"
    output += "\n\n"

    return output
  end
  #----------------------------------------------------------------------------
  # バイナリの書き出し
  #----------------------------------------------------------------------------
  def write_binary()
    files = Array.new
    @debugfiles.each{|key, value|
      filename = "wkdir/" + key
      files << filename
      output = ""
      value.each{|flagdata| output += flagdata.write_binary() }
      STDERR.puts "genarate #{filename}..."
      File.open(filename, "wb"){|file| file.write output }
    }
    return files
  end

end

#============================================================================
#
#   メイン処理
#
#============================================================================
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
man = FLAGMANAGER.new(ARGV[0])

defines = man.write_defines()

File.open(ARGV[1], "w"){|file|
  file.puts defines
}

files = man.write_binary()
File.open(ARGV[2], "w"){|file|
  files.each{|name| file.puts "\"#{name}\"" }
}


