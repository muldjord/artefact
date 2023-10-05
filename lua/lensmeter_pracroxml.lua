function header() 
   output("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
   output("<results>\n")
end

function outputeye(eye, sphere, cyl, axis)
   output("    <group name=\"" .. string.lower(eye) .. "\">\n")
   output("      <value name=\"sphere\" value=\"" .. sphere .. "\"/>\n")
   output("      <value name=\"cyl\" value=\"" .. cyl .. "\"/>\n")
   output("      <value name=\"axis\" value=\"" .. axis .. "\"/>\n")
   output("    </group>\n")
end

function parse()
   local filename = getFilename()

   local file = io.open(filename, "rb")

   if( file ) then

      -- Get the filesize.
      local size = file:seek("end", 0)
      file:seek("set", 0)

      -- Read in the entire content of the file.
      local data = file:read(size)
      
      output("  <result class=\"" .. getDeviceClass() .. "\" timestamp=\"" .. getDataTimestamp() .. "\">\n")
      
      -- Parse the mama
      for eye, spheresign, sphere, cylsign, cyl, axis
      in string.gmatch(data, "(%u+) +SPH: +([-%+]) (%d+%.%d+).\n" ..
		       "      CYL: +([-%+]) (%d+%.%d+) *X* *(%d+)")
      do
	 outputeye(eye, spheresign .. sphere, cylsign .. cyl, axis)
      end 
      
      output("  </result>\n")
   end

end

function footer()
   output("</results>\n")
end
