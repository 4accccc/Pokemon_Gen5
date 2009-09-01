#! /usr/local/bin/ruby
class NCG_STRUCT
	def initialize
		temp_h = Struct.new("NCG_HEADER",:FormatID, :ByteOrderMark, :FormatVersion,
					  :FileSize, :HeaderSize, :DataBlocks)
		@Header = temp_h.new
		temp_c = Struct.new("CHAR_DATA",:BlockType, :BlockSize, :CharWidth,
					  :CharHeight, :BitMode, :CharData )
		@CharData = temp_c.new
		temp_l = Struct.new("LINK_DATA",:BlockType, :BlockSize, :PalFileName )
		@LinkData = temp_l.new
	end

	def Pars(inFileName)
		File.open(inFileName){ |file|
			file.binmode
			#ヘッダーを読む
			@Header.FormatID  = file.read(4)
			byteOrderMark = file.read(2)
			formatVersion	= file.read(2)
			fileSize = file.read(4)
			headerSize = file.read(2)
			dataBlocks = file.read(2)
			
			@Header.ByteOrderMark = byteOrderMark.unpack("S").shift
			@Header.FormatVersion	= formatVersion.unpack("S").shift
			@Header.FileSize = fileSize.unpack("I").shift
			@Header.HeaderSize = headerSize.unpack("S").shift
			@Header.DataBlocks = dataBlocks.unpack("S").shift
			#キャラデータを読む
			@CharData.BlockType = file.read(4)
			blockSize = file.read(4)
			charWidth = file.read(4)
			charHeight = file.read(4)
			bitMode = file.read(4)
			
			@CharData.BlockSize = blockSize.unpack("I").shift
			@CharData.CharWidth = charWidth.unpack("I").shift
			@CharData.CharHeight = charHeight.unpack("I").shift
			@CharData.BitMode = bitMode.unpack("I").shift

			size = @CharData.BlockSize-20 #実データ以外のサイズ(20バイト)を引く

			charData = file.read(size)
			
			@CharData.CharData = charData.unpack("C*")
			

			#アトリビュートを読む（飛ばす）
			blockType = file.read(4)
			blockSize = file.read(4)
			size = ( blockSize.unpack("I").shift ) - 8
			data = file.read(size)
			
			#リンク情報を読む
			@LinkData.BlockType = file.read(4)
			blockSize = file.read(4)
			@LinkData.BlockSize = blockSize.unpack("I").shift
			size = @LinkData.BlockSize - 8
			file_name = file.read(size)
			pal_file_name = file_name.delete("\000")
			array = pal_file_name.split("\\")
			
			@LinkData.PalFileName = array.pop
			
			p @Header.FormatID
			p @Header.ByteOrderMark
			p @Header.FormatVersion
			p @Header.FileSize
			p @Header.HeaderSize
			p @Header.DataBlocks
			p "--------------------"
			p @CharData.BlockType
			p @CharData.BlockSize
			p @CharData.CharWidth
			p @CharData.CharHeight
			p @CharData.BitMode
			p "--------------------"
			p @LinkData.BlockType
			p @LinkData.BlockSize
			p @LinkData.PalFileName
		}
	end

	def GetCharData
		@CharData.CharData
	end
	def GetWidth
		@CharData.CharWidth
	end
	def GetPalFileName
		@LinkData.PalFileName
	end
end
