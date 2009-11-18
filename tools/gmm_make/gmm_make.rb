#! ruby -Ks

  require 'kconv'

#=====================================================================================
#
# gmm生成スクリプト
#
# ex)
# gmm = GMM::new
# gmm.open_gmm( "template.gmm", "monsname.gmm" )
# gmm.make_row_index( "MONSNAME_", 1, "フシギダネ" )
# gmm.close_gmm
#
#=====================================================================================

class GMM
  #===========================================================================
  # 初期化
  #===========================================================================
  def initialize( width = 216 )
    @fp_gmm_r
    @fp_gmm_w
    @file_open_flag = 0
    @width_value = width
  end

  #===========================================================================
  #
  # ファイルのオープン
  #
  # @param[in]  read_file   gmmのヘッダやフッタを生成する元になるgmmファイル
  #                         gmmファイルであるか確認はしてません
  # @param[in]  write_file  書き出すgmmファイル
  #
  #
  #===========================================================================
  def open_gmm( read_file, write_file )
    if @file_open_flag == 1
      printf("FileOpenAlready!\n")
      exit( 1 )
    end
    begin
      @fp_gmm_r = open( read_file )
    rescue
      printf( "FileOpenError:%s\n",read_file )
      exit( 1 )
    end
    @fp_gmm_w = open( write_file, "w" )
    @file_open_flag = 1
    make_header
  end

  #===========================================================================
  #
  # gmmのヘッダを作成
  #
  #===========================================================================
  def make_header
    @fp_gmm_r.rewind
    while str = @fp_gmm_r.gets
      @fp_gmm_w.print str
      if str.index("</lock>") != nil
        @fp_gmm_w.print "\r\n"
        @fp_gmm_w.print "\r\n"
        break
      end
    end
  end

  #===========================================================================
  #
  # rowデータを作成
  #
  # @param[in]  label ラベル名
  # @param[in]  str   rowデータとなる文字列
  #
  #===========================================================================
  def make_row( label, str )
    @fp_gmm_w.printf( "\t<row id=\"%s\">\r\n", label )
    @fp_gmm_w.printf( "\t\t<language name=\"JPN\" width=\"%d\">%s</language>\r\n", @width_value, str.toutf8 )
    @fp_gmm_w.print "\t</row>\r\n\r\n"
  end

  #===========================================================================
  #
  # rowデータを作成
  #
  # @param[in]  label ラベル名
  # @param[in]  str_jpn   rowデータとなる文字列
  # @param[in]  str_kanji rowデータとなる文字列（漢字文字列）
  #
  #===========================================================================
  def make_row_kanji( label, str_jpn, str_kanji )
    @fp_gmm_w.printf( "\t<row id=\"%s\">\r\n", label )
    @fp_gmm_w.printf( "\t\t<language name=\"JPN\" width=\"%d\">%s</language>\r\n", @width_value, str_jpn.toutf8 )
    @fp_gmm_w.printf( "\t\t<language name=\"JPN_KANJI\" width=\"%d\">%s</language>\r\n", @width_value, str_kanji.toutf8 )
    @fp_gmm_w.print "\t</row>\r\n\r\n"
  end

  #===========================================================================
  #
  # rowデータを作成
  # ポケモン名とかトレーナー名とかIDによる連番の生成
  #
  # @param[in]  label IDの元になるラベル名
  # @param[in]  cnt   labelにくっつける連番
  # @param[in]  str   rowデータとなる文字列
  #
  # ex)こんな形になることを想定
  #   ポケモン名なら
  #   mons_no = 1
  #   row_make( "MONSNAME_", mons_no, "フシギダネ" )
  #
  #                   ↓
  #
  #   <row id="MONSNAME_001">
  #     <language name="JPN">フシギダネ</language>
  #   </row>
  #
  #===========================================================================
  def make_row_index( label, cnt, str )
    label = sprintf( "%s%03d", label, cnt )
    make_row( label, str )
  end

  #===========================================================================
  #
  # rowデータを作成
  # ポケモン名とかトレーナー名とかIDによる連番の生成（漢字あり）
  #
  # @param[in]  label     IDの元になるラベル名
  # @param[in]  cnt       labelにくっつける連番
  # @param[in]  str_jpn   rowデータとなる文字列
  # @param[in]  str_kanji rowデータとなる文字列
  #
  #===========================================================================
  def make_row_index_kanji( label, cnt, str_jpn, str_kanji )
    label = sprintf( "%s%03d", label, cnt )
    make_row_kanji( label, str_jpn, str_kanji )
  end

  #===========================================================================
  #
  # gmmのフッタを作成
  #
  #===========================================================================
  def make_footer
    find = 0
    @fp_gmm_r.rewind
    while str = @fp_gmm_r.gets
      if str.index("<flowchart-group-list>") != nil
        find = 1
      end
      if find == 1
        @fp_gmm_w.print str
      end
    end
  end

  #===========================================================================
  #
  # ファイルクローズ
  #
  #===========================================================================
  def close_gmm
    make_footer
    @fp_gmm_r.close
    @fp_gmm_w.close
    @file_open_flag = 0
  end
end
