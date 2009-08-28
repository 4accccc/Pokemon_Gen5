#4ƒoƒCƒg‹«ŠE‚É‡‚í‚¹‚Ä‚O‚ğ‚Â‚ß‚é
rname = ARGV[0]
wname = ARGV[1]

File.open(wname, "wb") {|wf|
	File.open(rname,"rb") {|f|
		wf.write(f.read)
		padding = File.size(ARGV[0]) % 4
		if padding != 0 then
			padding.times{|dmy| wf.write(0)}
		end
	}
}
