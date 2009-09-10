class NCL_STRUCT
	def initialize(type)
		temp_h = Struct.new("NCL_HEADER",:FormatID, :ByteOrderMark, :FormatVersion,
					  :FileSize, :HeaderSize, :DataBlocks)
		@Header = temp_h.new
		temp_p = Struct.new("PAL_DATA",:BlockType, :BlockSize, :PalColorNum,
					  :PaletteNumber, :PaletteData )
		@PalData = temp_p.new
		@PalType = type
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
			#パレットデータを読む
			@PalData.BlockType = file.read(4)
			blockSize = file.read(4)
			palColorNum = file.read(4)
			paletteNumber = file.read(4)

			size = 0
			if @PalType == 1 then
				size = 512
			else
				size = 8192
			end
			paletteData = file.read(size)

			@PalData.PaletteData = paletteData.unpack("S*")

			@PalData.BlockSize = blockSize.unpack("I").shift
			@PalData.PalColorNum = palColorNum.unpack("I").shift
			@PalData.PaletteNumber = paletteNumber.unpack("I").shift

#			p @Header.FormatID
#			p @Header.ByteOrderMark
#			p @Header.FormatVersion
#			p @Header.FileSize
#			p @Header.HeaderSize
#			p @Header.DataBlocks
#			p "--------------------"
#			p @PalData.BlockType
#			p @PalData.BlockSize
#			p @PalData.PalColorNum
#			p @PalData.PaletteNumber

		}
	end

	def GetPaletteData
		@PalData.PaletteData
	end
end
