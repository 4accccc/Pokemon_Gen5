#----------------------------------------------------------------------------
#  overlaytool.rb
#  afterを複数指定できるようにパッチ修正  2009.01.16 k.ohno
#  TWL専用に改造                          2009.04.15 k.ohno
#  拡張メモリ対応の為にソース整理         2010.01.13 tamada
#----------------------------------------------------------------------------


#----------------------------------------------------------------------------
#
#       定数定義
#
#----------------------------------------------------------------------------

#DEFAULT_LSFFILE		= "default.lsf"
OVERLAY_DIR			= "overlay"
MAKE_PROG_FILE  	= "make_prog_files"
OUTPUT_LSFFILE  	= OVERLAY_DIR + "/main.lsf"
OUTPUT_TWL_LSFFILE  	= OVERLAY_DIR + "/main.TWL.HYB.lsf"
OUTPUT_OVERLAYFILES	= OVERLAY_DIR + "/overlay_files"
OUTPUT_OVERLAYTEXT  = OVERLAY_DIR + "/overlaymap.txt"

MATCH_KEYWORD	 = /\A#===>/			# 行頭が「#===>」
MATCH_LIB        = /^#LIB=>/            # ライブラリを含める場合
MATCH_OVERLAYSRC = /^SRCS_OVERLAY_.*/	# 行頭から「SRCS_OVERLAY_...」
MATCH_ENDSRC	 = /[\s\t].*\\\z/				# 行末が「\」
MATCH_ERROR      = /\\\s/

DEFAULT_SECTION_NAME		= ["main", "ITCM", "EX_MEM_START"]

#デフォルトで読み込むファイル
#default_lsf = File.read(DEFAULT_LSFFILE)
#
default_lsf = <<DEFAULT_LSFFILE

#----------------------------------------------------------------------------
#
#  Nitro LCF SPEC FILE
#
#----------------------------------------------------------------------------

Static main
{
  Address		$(ADDRESS_STATIC)
  Object		$(OBJS_STATIC)
  Library		$(LLIBS) $(GLIBS) $(CW_LIBS)

  #	スタックサイズ　IRQスタックサイズ
  #	(省略時にはスタックサイズは0(最大限度まで確保)、IRQ スタックサイズは 0x800 バイト)
  StackSize	0x1800 0x800
}

Autoload ITCM
{
  Address		0x01ff8000
  Object		* (.itcm)
}

Autoload DTCM
{
  Address		$(ADDRESS_DTCM)
  Object		* (.dtcm)
}

Autoload EX_MEM_START
{
  Address		$EX_MEM_START_ADRS
}

#----------------------------------------------------------------------------
#  オーバーレイ設定
#----------------------------------------------------------------------------

DEFAULT_LSFFILE


##
## TWL定義
##
bottom_lsf1 = <<BOTTOM_LSFFILE1


Ltdautoload LTDMAIN
{
	# NITRO/TWL 共有のオーバーレイが在る場合は、さらにその後ろに配置する必要があります。
  
BOTTOM_LSFFILE1
  
bottom_lsf2 = <<BOTTOM_LSFFILE2
  Object		* (.ltdmain)
	Object		$(OBJS_LTDAUTOLOAD)
	Library		$(LLIBS_EX) $(GLIBS_EX)
}

BOTTOM_LSFFILE2


#----------------------------------------------------------------------------
#
#
#
#----------------------------------------------------------------------------
class OverlayInfo

  attr_reader :name, :put_name, :afterList, :source_name, :lib_name

  def initialize( name, afterList, source_name, lib_name )
    @name = name
    @put_name = @name.downcase.sub(/srcs_overlay_/,"")
    @afterList = afterList
    @source_name = source_name
    @lib_name = lib_name
  end

  def putOverlaySection()
    
    output = sprintf("Overlay %s\n{\n", @put_name )

    # 「SRCS_OVERLAY_???」からSRCS_OVERLAY_を削って小文字化した名前をオーバーレイ領域の
    # ターゲットにする。ただし、予約語として「main,ITCM,DTCM」という文字列が定義されているので
    # これらは避けるようにする
    @afterList.each { |afline|
      if DEFAULT_SECTION_NAME.include?( afline ) == false then
        afline = afline.downcase.sub(/srcs_overlay_/,"")
      end
      output += sprintf( "\tAfter\t%s\n", afline )
    }
    output += sprintf("\tObject")
    @source_name.each_with_index{ | name, index |
      filename = name.slice(/[a-zA-Z_0-9]+\./)
      if index != 0 then
        output += sprintf("\t")
      end
      output += sprintf("\t$(OBJDIR)/%so",filename)
      if name.match(/\\\z/) then
        output += sprintf(" \\")
      end
      output += sprintf("\n")
    }

    if @lib_name.length > 0 then
      output += sprintf("\tLibrary")
      @lib_name.each_with_index{ |name, index|
        filename = name
        output += sprintf("\t%s",filename)
        if index + 1 != @lib_name.length  then
          output += sprintf(" \\")
        end
        output += sprintf("\n")
      }
    end
    output += sprintf("}\n\n")
    return output
  end

end




#----------------------------------------------------------------------------
#
#ターゲットになるソースコードの記述ファイルを１行ずつ処理
#
#----------------------------------------------------------------------------
sections = Array.new

section_name = DEFAULT_SECTION_NAME.dup
source_name          = nil
lib_name          = Array.new
afterList = []

USERNAME_SKIP = 1     #解釈しない
USERNAME_INSERT = 0   #解釈する
USERNAME_NONE = -1     #ユーザーネーム定義はまだない
## ユーザー名定義があった場合に1 記憶しておく
usernamestate = USERNAME_NONE



File.readlines(MAKE_PROG_FILE).each_with_index{ |line, line_count|
  line.chomp!

  #行末の「\」の後ろにスペースが入っていると誤動作するのでチェックする
  if line.match(MATCH_ERROR) then
    printf "%d行目： \\マークの後ろにスペースがはいっています。\n",line_count + 1
    exit 1
  end
  
  #プリプロセッサ解釈部分
  if usernamestate == USERNAME_NONE
    if line =~ /^#if\s+(.*)$/
      $1.split('|').each{|usernameline|
        if usernameline == ENV["USERNAME"]  #
          usernamestate = USERNAME_INSERT
        end
      }
      if usernamestate == USERNAME_NONE
        usernamestate = USERNAME_SKIP
      end
    end
  else
    if line =~ /^#else$/
      usernamestate = 1 - usernamestate  # 反転
    elsif line =~ /^#endif$/
      usernamestate = USERNAME_NONE
    end
  end
  if usernamestate == USERNAME_SKIP  ## この定義の場合処理しない
    next
  end
  
  #オーバーレイ定義の取得
  if line.match(MATCH_KEYWORD) then
    m = MATCH_KEYWORD.match(line)
    _str = m.post_match.split.first.sub(/[\s\t]/,"")
    if section_name.include?(_str) == true then
      afterList << m.post_match.split.first.sub(/[\s\t]/,"")
    else
      printf "「%s」 というオーバーレイソ\ースは定義されていません\n",_str
      p section_name
      exit 1
    end
    next
  end

  #オーバーレイ配置ライブラリ名の取得
  if line.match(MATCH_LIB) then
    line =~ /(\S+)\s+(\S+)/
    lib_name << $2
    next
  end

  #上記意外の＃から始まる行はコメントとみなす
  if line =~/^#/ then
    next
  end

  #オーバーレイセクション名の取得
  if line.match(MATCH_OVERLAYSRC) then

    if afterList.length == 0 then
      printf "Error!!\n"
      printf "#===>でオーバーレイアドレスを指定していないのに、\n"
      printf "オーバーレイ用のソ\ースコードが記述されています\n"
      exit 1
    end

    section_name << line.split.first
    source_name = [ line ]
    next
  end

  #「SRCS_OVERLAY」の後はソースコード行なので「\」がなくなるまで
  # 保存し続ける
  if source_name != nil then
    if line.match(/[a-zA-Z_0-9]+\.[cs]/) then
      source_name << line
    end

    # 「\」が無い行がでてきたら終わりなので、取得終了
    if line.match(MATCH_ENDSRC)==nil || line=="" then
      sections << OverlayInfo.new( section_name.last, afterList, source_name, lib_name )
      source_name = nil
      lib_name = []
      afterList = []
    end
  end

}


#----------------------------------------------------------------------------
#
#  読み込みデータを解釈して出力
#
#----------------------------------------------------------------------------
total_output = ""
total_output += default_lsf
total_output += "#ここから下はoverlaytool.rbで自動生成されています\n\n"
sections.each{|overlay_info|
  total_output += overlay_info.putOverlaySection()
}
total_output.gsub!("$EX_MEM_START_ADRS","0x02400000")

#NITRO用ファイルを作成する部分
File.open(OUTPUT_LSFFILE, "w" ){|file|
  #NITROのLSFのファイルの$LIBを読み替える
  file.puts total_output.gsub(".$HYB","")
}

##TWL用ファイルを作成する部分
File.open(OUTPUT_TWL_LSFFILE, "w" ) {|file|
  #TWLのLSFファイルの$LIBを読み替える
  file.puts total_output.gsub("$HYB","TWL.HYB")
  file.puts(bottom_lsf1)
  sections.each{|section|
    file.printf("\tAfter\t%s\n",section.put_name )
  }
  file.puts(bottom_lsf2)
}

#オーバーレイに該当するファイルがコンパイルターゲットになるようにする
File.open(OUTPUT_OVERLAYFILES,"w"){|file|
  file.printf("SRCS_OVERLAY\t=\t")
  sections.each{|section|
    file.printf( "\t\t\t$(%s)",section.name )
    if section != sections.last then
      file.printf("\t\\\n")
    else
      file.printf("\n")
    end

  }
}

#オーバーレイのターゲットネームとその番号をテキストに吐き出す
File.open(OUTPUT_OVERLAYTEXT,"w"){|file|
  sections.each_with_index{|section, index|
    file.printf( " ID 0x%x = %s\n",index + 3, section.put_name )
  }
}

