-- // General functions
-- output
-- 
-- // Entry field retrieval functions.
-- getCPR
-- getBirthYear
-- getBirthMonth
-- getBirthDay
-- getGivenName
-- getSurName
-- getSrcAddr
-- getDstAddr
-- getEntryTimestamp
-- getUID
-- 
-- // Data field retrieval functions.
-- getDeviceType
-- getDeviceID
-- getDataTimestamp
-- getLocation
-- getFilename


-- // Lens example 1
--<80> HUMPHREY SYSTEMS     
--   LA 350    SEQ 01
--
--     PROG ADD LENS
--
--
--
--Ref Line E 546
--RIGHT SPH: - 5.00
--      CYL: - 0.25 X  63<B0>
--  PSM HOR:   0.00 IN
--      VER:   0.50 DN
--
--Ref Line E 546
--LEFT  SPH: - 5.00
--      CYL: - 0.25 X  63<B0>
--  PSM HOR:   0.00 OUT
--      VER:   0.50 DN
--
--NET PSM H:   0.00
--        V:   0.00
--
--PD: 95.5
--
--  (R 47.5 +  L 48.0)
--


-- // Lens example 2
--<80> HUMPHREY SYSTEMS     
--   LA 350    SEQ 07
--
--     PROG ADD LENS
--
--
--
--Ref Line E 546
--RIGHT SPH: - 4.75
--      CYL: - 0.25 X 133<B0>
--  PSM HOR:   1.50 OUT
--      VER:   0.75 DN
--
--Ref Line E 546
--LEFT  SPH: - 4.75
--      CYL: - 0.25 X 158<B0>
--  PSM HOR:   1.75 IN
--      VER:   1.00 DN
--
--NET PSM H:   0.25 IN
--        V:   0.25 UP OD
--
--PD: 92.0
--
--  (R 47.5 +  L 44.5)


function header() 
   output("<xml>\n")
end

function outputeye(eye, sphere, cyl, axis)
   output("    <value eye=\"" .. eye .. 
	  "\" sphere=\"" .. sphere .. 
	  "\" cyl=\""  .. cyl ..
	  "\" axis=\"" .. axis .. "\"/>\n")
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
      
      output("  <lensdata timestamp=\"" .. getDataTimestamp() .. "\">\n")
      
      -- Parse the mama
      for eye, spheresign, sphere, cylsign, cyl, axis
      in string.gmatch(data, "(%u+) +SPH: +([-%+]) (%d+%.%d+).\n" ..
		       "      CYL: +([-%+]) (%d+%.%d+) *X* *(%d+)")
      do
	 outputeye(eye, spheresign .. sphere, cylsign .. cyl, axis)
      end 
      
      output("  </lensdata>\n")
   end

end

function footer()
   output("</xml>\n")
end
