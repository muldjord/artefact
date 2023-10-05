function header()

   filename = "/tmp/cpr.tmp." .. os.time()
   os.execute("echo " .. getCPR() .. " | nc server.dns.addr 10301 > " .. filename)

   local file = io.open(filename, "rb")

   cpr = ""
   givenname = ""
   surname = ""
   address = ""
   postnr = ""
   city = ""

   if( file ) then
      -- Get the filesize.
      local size = file:seek("end", 0)
      file:seek("set", 0)

      -- Read in the entire content of the file.
      local data = file:read(size)
      
      -- Parse the mama
      for code1, code2, value
      in string.gmatch(data, "00(%d)(%d)([^\n.]+)\n")
      do
	 --print(code1..code2..": " ..  value)
	 if code1 == "0" then
	    if code2 == "0" then cpr = value end
	    if code2 == "1" then surname = value end
	    if code2 == "2" then givenname = value end
	    if code2 == "5" then address = value end
	    if code2 == "7" then postnr = value end
	    if code2 == "8" then city = value end
	 end
      end 
   end

   output("<?xml version='1.0' encoding='utf-8'?>\n")
   output("<results>\n")
   output("  <result class=\"cpr\" timestamp=\"" .. os.time() .. "\">\n")
   output("    <value name=\"cpr\" value=\"" .. UTF8Encode(cpr) .. "\"/>\n")
   output("    <value name=\"name\" value=\"" .. UTF8Encode(givenname .. " " .. surname) .. "\"/>\n")
   output("    <value name=\"address\" value=\"" .. UTF8Encode(address) .. "\"/>\n")
   output("    <value name=\"city\" value=\"" .. UTF8Encode(postnr .. " " .. city) .. "\"/>\n")
   output("  </result>\n")
   output("</results>\n")

   os.remove(filename)

end

function parse()
end

function footer()
end
