function header() 
end

function parse()
   local filename = getFilename()

   local file = io.open(filename, "rb")
   if ( file ) then
      repeat
	 data = file:read(1024)
	 if not(data == nil) then
	    output(data)
	 end
      until data == nil
   end
end

function footer()
end
