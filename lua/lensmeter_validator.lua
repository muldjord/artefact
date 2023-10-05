function validate()
   -- If data chunk is less than 10 bytes its probably broken...
   -- Lensmeter sends out 1 byte when turned on/off
   if( getFileSize() < 10 ) then
      setValid(false)
   else
      setValid(true)
   end
end