#============================================================================================
#
#	トレーナータイプ個別データコンバート用Rubyスクリプト
#
#	2011.05.23  tamada@gamefreak.co.jp
#
#============================================================================================
require ENV["PROJECT_ROOT"] + 'tools/gmm_make/gmm_make'
require ENV["PROJECT_ROOT"] + 'tools/constant'

#============================================================================================
#============================================================================================

class PARA
  enum_const_set %w[
    NO
    ID
    NAME
    SEX
    GRF
    GRB
    BGM
    HASH
    COMMENT
  ]
end

#------------------------------------------------------------------------------
#
# トレーナータイプデータ保持用クラス
#
#------------------------------------------------------------------------------
class TRTYPEDATA

  attr_reader( :no, :id, :name, :sex, :grf, :bgm )

  def initialize( line )
    @no = line[PARA::NO]
    @id = line[PARA::ID]
    @name = line[PARA::NAME]
    @sex = line[PARA::SEX]
    @grf = line[PARA::GRF]
    @grb = line[PARA::GRB]
    @bgm = line[PARA::BGM]
    @hash = line[PARA::HASH]
    @comment = line[PARA::COMMENT]
  end

  def trtype
    return "TRTYPE_" + @id.upcase
  end

  def trhash
    return @id.upcase + @sex + @name
  end

  def trsex
    if @sex == "♂" then
      return "PTL_SEX_MALE"
    elsif @sex == "♀" then
      return "PTL_SEX_FEMALE"
    end
  end

  def trbgm
    if @bgm == "" || !(@bgm =~ /^SEQ_BGM_/) then
      return "SEQ_BGM_EYE_01"
    end
    return @bgm
  end

end

#------------------------------------------------------------------------------
# トレーナーデータの重複チェック
#------------------------------------------------------------------------------
def check_ttdata( ttdata )
  ttdata.each_with_index{|ttd, idx|
    search = ttdata.select{|check_item| check_item != ttd && check_item.no == ttd.no }
    if search.size > 0 then
      puts "ナンバーの重複があります:#{ttd.no}\n"
      exit 1
    end
    search = ttdata.select{|check_item| check_item != ttd && check_item.id == ttd.id }
    if search.size > 0 then
      puts "IDの重複があります:#{ttd.id}\n"
      exit 1
    end

  }
end

#------------------------------------------------------------------------------
# 重複を除いたグラフィック名の配列を作成
#------------------------------------------------------------------------------
def make_gra_list( ttdata )
  gra_list = []
  ttdata.each{|ttd|
    index = gra_list.index(ttd.grf)
    if index == nil then
      gra_list << ttd.grf
    end
  }
  return gra_list
end

#============================================================================================
#============================================================================================
#------------------------------------------------------------------------------
# トレーナータイプ定義ファイル生成
#   trtype_def.h
#------------------------------------------------------------------------------
def make_trtype_header( filename, ttdata )
  str = ""
  str += "//============================================================================================\n" 
  str += "/**\n" 
  str += " * @file	#{filename}\n" 
  str += " * @bfief	トレーナータイプ定義\n";
  str += " * @author	#{__FILE__}\n" 
	str += " * @author	このファイルは、コンバータが吐き出したファイルです\n" 
  str += "*/\n"
  str += "//============================================================================================\n" 
  str += "\n#pragma once\n\n" 
	str +="\n"
  
  ttdata.each{|ttd|
    str += sprintf("#define	%s    ( %d )    //%s\n", ttd.trtype, ttd.no, ttd.trhash)
  }

	str += sprintf("#define	TRTYPE_MAX	( %d )\n", ttdata.size )

	fp_trtype = File.open( filename, "w" )
  fp_trtype.puts str
	fp_trtype.close
  return str
end


#------------------------------------------------------------------------------
# トレーナーグラフィックlstファイル生成
#   trfgra_wb.lst
#------------------------------------------------------------------------------
def make_trf_list( filename,  ttdata, gra_list )
  str = ""
  #こっちの方が正解のような…
  #gra_list.each{|graname|
  #  example = ttdata.detect{|ttd| ttd.grf == graname }
  #  str += sprintf( "trwb_%s,%s\n", graname, example.trsex )
  #}
  ttdata.each{|ttd|
    str += sprintf( "trwb_%s,%s\n", ttd.grf, ttd.trsex )
  }
  fp_trgra = File.open(filename,"w")
  fp_trgra.puts str
	fp_trgra.close
end

#------------------------------------------------------------------------------
# トレーナーグラフィックscrファイル生成
#   trfgra_wb.scr
#------------------------------------------------------------------------------

def put_gra_set( graname )
  str = ""
  str += "\"#{graname}.NCGR\"\n"
  str += "\"#{graname}.NCBR\"\n"
  str += "\"#{graname}.NCER\"\n"
  str += "\"#{graname}.NANR\"\n"
  str += "\"#{graname}.NMCR\"\n"
  str += "\"#{graname}.NMAR\"\n"
  str += "\"#{graname}.NCEC\"\n"
  str += "\"#{graname}.NCLR\"\n"
  return str
end

def make_trfgra( filename, gra_list )
  str = ""
  gra_list.each{|graname|
    str += put_gra_set( "trwb_" + graname )
  }
  File.open(filename,"w"){|file|
    file.puts str
  }
end

#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
def makeGMM( filename, defaultGMM, ttdata )
  gmm_type = GMM::new
  gmm_type.open_gmm( defaultGMM, filename )
  ttdata.each{|ttd|
    str = "MSG_" + ttd.trtype
    gmm_type.make_row_kanji( str, ttd.name, ttd.name )
  }
  gmm_type.close_gmm
end

#------------------------------------------------------------------------------
#トレーナータイプHash生成
# swan/tools/hash/trtype_hash.rb
#------------------------------------------------------------------------------
def make_hash( filename, ttdata )
  str = ""
  str += sprintf("#! ruby -Ks\n\n" )
  str += sprintf("\t$trtype_hash = {\n" )
  ttdata.each{|ttd|
    str += sprintf("\t\t\"%s\"=>%d,\n", ttd.trhash, ttd.no)
  }
  str += sprintf("\t}\n" )

  str += sprintf("#\n")
  str += sprintf("\t$trtype_id_hash = {\n")
  ttdata.each{|ttd|
    str += sprintf("\t\t\"%s\" =>%3d,\n", ttd.trtype, ttd.no)
  }
  str += sprintf("\t}\n")

  fp_hash = open( filename, "w" )
  fp_hash.puts str
  fp_hash.close
end


#------------------------------------------------------------------------------
# トレーナータイプ別プログラム参照テーブルの生成
#------------------------------------------------------------------------------
def make_cdat( filename, ttdata, gra_list )
  str = ""
  str += "//============================================================================================\n"
  str += "/**\n"
  str += " * @file	#{filename}\n"
	str += " * @bfief	トレーナータイプ別テーブル\n"
  str += " * @author	#{__FILE__}\n" 
	str += " * @author	このファイルは、コンバータが吐き出したファイルです\n"
  str += "*/\n"
  str += "//============================================================================================\n"
  str += "const TRAINER_TYPE_DATA DATA_TrainerType[] = {\n"


  ttdata.each{|ttd|
    str += sprintf("\t{ // No.%03d %s\n", ttd.no, ttd.trhash )
    str += sprintf("\t\t%3d,%s,\n", gra_list.index(ttd.grf), ttd.trsex )
    str += sprintf("\t\t\%s\n", ttd.trbgm )
    str += "\t},\n"
  }

  str += "};\n\n"

  fp_cdat = File.open( filename, "w" )
  fp_cdat.puts str
  fp_cdat.close

end

#============================================================================================
#============================================================================================
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

ttdata = []

File.open(ARGV[0]).each{|line|
  column = line.chomp.split(",")
  if column[PARA::NO] =~/\d+/ then
    ttdata << TRTYPEDATA.new( column )
  end
}

#エラーチェック
check_ttdata( ttdata )

#重複を除いたグラフィック名の配列を作成
gra_list = make_gra_list( ttdata )

#gmm
makeGMM( "trtype.gmm", ARGV[1], ttdata )

#trtype_def.h
make_trtype_header( "trtype_def.h", ttdata )

#trfgra_wb.scr
make_trfgra( "trfgra_wb.scr", gra_list )

#trfgra_wb.lst
make_trf_list( "trfgra_wb.lst", ttdata, gra_list )

# swan/tools/hash/trtype_hash.rb
make_hash( "trtype_hash.rb", ttdata )

# trtype_data.cdat
make_cdat( "trtype_data.cdat", ttdata, gra_list )


  #タイムスタンプ比較用のダミーファイルを生成
  fp_w = open( "out_end", "w" )
  fp_w.close

