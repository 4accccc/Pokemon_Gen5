#!/usr/bin/ruby
#
# プラチナのgmmをWBに強引変換するツールです  ここには GmmReadクラスしかありません
#
#
#

require "nkf"
require "rexml/document"
include REXML



### メイン処理クラス

class GmmRead
  
  ##置き換えたい物をここに書けばよい
  @@TagTbl=[
  ['数字１桁','[2:00:１桁'],
  ['数字２桁','[2:01:２桁'],
  ['数字３桁','[2:02:３桁'],
  ['数字４桁','[2:03:４桁'],
  ['数字５桁','[2:04:５桁'],
  ['数字６桁','[2:05:６桁'],
  ['数字７桁','[2:06:７桁'],
  ['数字８桁','[2:07:８桁'],
  ['数字９桁',''],                      ##WBなくなりました
  ['数字10桁',''],                      ##WBなくなりました
  ['ポケモン名','[1:01:ポケモン種族名'],
  ['ニックネーム','[1:02:ポケモンニックネーム'],
  ['ポケモン分類','[1:04:ポケモン分類'],
  ['トレーナー名','[1:00:トレーナー名'],
  ['マップ名','[1:05:マップ名'],
  ['とくせい名','[1:06:とくせい名'],
  ['わざ名','[1:07:わざ名'],
  ['せいかく','[1:08:せいかく'],
  ['どうぐ名','[1:09:どうぐ名'],
  ['ポロック名',''],                      ##WBなくなりました
  ['グッズ名','[1:10:グッズ名'],
  ['ボックス名','[1:11:ボックス名'],
  ['attackmsg',''],                      ##WBなくなりました
  ['ステータス名','[1:13:ステータス名'],
  ['トレーナー種別名','[1:14:トレーナー種別名'],
  ['ポケモンのタイプ','[1:03:ポケモンタイプ'],
  ['木の実の味（戦闘用）',''],            ##WBなくなりました
  ['特殊状態名','[1:17:特殊状態名'],
  ['バッグのポケット名','[1:18:バッグのポケット名'],
  ['欠番（どうぐ説明）',''],            ##WBなくなりました
  ['欠番（わざ説明）',''],            ##WBなくなりました
  ['欠番（とくせい説明）',''],            ##WBなくなりました
  ['欠番（ずかんテキスト）',''],            ##WBなくなりました
  ['欠番（きのみ説明）',''],            ##WBなくなりました
  ['ポケッチソフト名',''],            ##WBなくなりました
  ['地下のどうぐの名前',''],            ##WBなくなりました
  ['カップ名','[1:26:カップ名'],   ##対戦のレギュレーション名
  ['ポケット名（アイコンつき）',''],            ##WBなくなりました
  ['簡易会話単語','[1:28:簡易会話単語'],
  ['秘密会話質問項目',''] ,           ##WBなくなりました
  ['秘密会話質問の答え',''],            ##WBなくなりました
  ['アクセサリー名','[1:31:アクセサリー名'],
  ['ジム名','[1:32:ジム名'],
  ['時間帯','[1:33:時間帯'],
  ['コンテスト名称','[1:34:コンテスト名称'],
  ['コンテストランク','[1:35:コンテストランク'],
  ['国名','[1:36:国名'],
  ['地域名','[1:37:地域名'],
  ['風船の名前',''],            ##WBなくなりました
  ['リボンの名前','[1:39:リボンの名前'],
  ['イメージクリップ背景',''],            ##WBなくなりました
  ['GTC:ポケモン分類項目',''],            ##WBなくなりました
  ['GTC:ポケモン性別指定項目',''] ,           ##WBなくなりました
  ['GTC:レベル指定項目',''],            ##WBなくなりました
  ['縦倍角',''],            ##WBなくなりました
  ['操作画面ガイドアイコン',''],           ##WBなくなりました
  ['文字表示ウェイト','[190:02:ウェイト変更（単発）'],
  ['コールバック引数','[190:05:コールバック引数変更（単発）'],
  ['文字表示位置X',''],           ##WBなくなりました
  ['文字表示位置Y',''],            ##WBなくなりました
  ['広場ミニゲーム','[1:40:広場ミニゲーム'],
  ['広場イベント名','[1:41:広場イベント名'],
  ['広場アイテム名','[1:42:広場アイテム名'],
  ['日本の挨拶','[1:43:日本の挨拶'],
  ['英語の挨拶','[1:44:英語の挨拶'],
  ['フランス語の挨拶','[1:45:フランス語の挨拶'],
  ['ドイツ語の挨拶','[1:46:ドイツ語の挨拶'],
  ['イタリア語の挨拶','[1:47:イタリア語の挨拶'],
  ['スペイン語の挨拶','[1:48:スペイン語の挨拶'],
  ['センタリング','[BD:02:Ｘセンタリング'],
  ['右寄せ','[BD:03:Ｘ右寄せ'],
  ]

  
  def TagConv( )
    
    sjisline=""
    @ConvFileLine.each{ |line|
      sjisline = NKF.nkf("-e",line)
      @@TagTbl.each{ |tagbuf|
        if sjisline =~ /\[\d+:\d+:#{tagbuf[0]}:*\d*\]/
          if tagbuf[1] != ''
            p NKF.nkf("-e", sjisline)
            p NKF.nkf("-e", tagbuf[1])
            sjisline = sjisline.gsub(/\[\d+:\d+:#{tagbuf[0]}/,tagbuf[1])
          else
            p "Warring:Find not WBTag : " + @ConvFileLineTmp.length.to_s + " : " + sjisline
          end
        end
      }
      utfline = NKF.nkf("-w",sjisline)
      
      #utfline = line
      @ConvFileLineTmp.push(utfline)
    }
    @ConvFileLine = @ConvFileLineTmp
    @ConvFileLineTmp = Array.new
  end
  
  
  
  
  def initialize
    @TmplateFileLine = Array.new
    @ConvFileLine = Array.new
    @ConvFileLineTmp =  Array.new
  end
  
  
  
  def FileRead( gmmrenew )
    @ConvFileLine = Array.new
    p "conv:"+gmmrenew
    fpr = File.new(gmmrenew)
    fpr.each{ |line|
      @ConvFileLine.push(line)
    }
    fpr.close
  end
  
  
  def FileWrite( gmmrenew )
    fpr = File.new(gmmrenew,"w")
    @ConvFileLine.each{ |line|
      fpr.puts(line)
    }
    fpr.close
  end
  
  def TmplateRead( gmmrenew )
    @TmplateFileLine = Array.new
    fpr = File.new(gmmrenew)
    fpr.each{ |line|
      @TmplateFileLine.push(line)
    }
    fpr.close
  end
  
  
  def TmplateMake( )
    @TmplateFileLine.each{ |line|
      if line =~ /(<row)/
        break
      end
      @ConvFileLineTmp.push(line)
    }
    @TmplateFileLine = @ConvFileLineTmp
    @ConvFileLineTmp = Array.new
  end
  
  
  def TmplateFootMake( )
    writeflg = 0
    @TmplateFileLine.each{ |line|
      if line =~ /(<flowchart-group-list)/
        writeflg = 1
      end
      if writeflg == 1
        @ConvFileLine.push(line)
      end
    }
  end
  
  
  def BodyAdd( )
    @ConvFileLineTmp = @TmplateFileLine
    writeflg = 0
    @ConvFileLine.each{ |line|
      if line =~ /(<row)/
        writeflg = 1
      end
      if writeflg==1
        @ConvFileLineTmp.push(line)
      end
    }
    @ConvFileLine = @ConvFileLineTmp
    @ConvFileLineTmp = Array.new
  end
  
  
  
  
  def JpnConv( )
    
    @ConvFileLine.each{ |line|
      sjisline = NKF.nkf("-e",line)
      if sjisline =~ /(<language name="日本語")/
        sjisline = sjisline.sub('<language name="日本語"','<language name="JPN"')
      end
      utfline = NKF.nkf("-w",sjisline)
      #utfline = line
      @ConvFileLineTmp.push(utfline)
    }
    @ConvFileLine = @ConvFileLineTmp
    @ConvFileLineTmp = Array.new
  end
  
  
  def JpnDupe()
    dupearray = Array.new
    dupeflg = 0
    
    @ConvFileLine.each{ |line|
      sjisline = NKF.nkf("-e",line)
      if sjisline =~ /<language name/
        dupeflg = 1
      end
      if dupeflg == 1
        dupearray.push(sjisline)
      end
      if sjisline =~ /language>/
        dupeflg = 2
      end
      utfline = NKF.nkf("-w",sjisline)
      @ConvFileLineTmp.push(utfline)
      
      if dupeflg == 2
        dupearray.each{ |sjisline|
          if sjisline =~ /(<language name="JPN")/
            sjisline = sjisline.sub('<language name="JPN"','<language name="JPN_KANJI"')
          end
          utfline = NKF.nkf("-w",sjisline)
          @ConvFileLineTmp.push(utfline)
        }
        dupeflg = 0
        dupearray = Array.new
      end
    }
    @ConvFileLine = @ConvFileLineTmp
    @ConvFileLineTmp = Array.new
  end
  
  
  
end



