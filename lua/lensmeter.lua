
function run()
   repeat
      local added_data = false
      repeat
	 -- Read in the data (with timeout of 100ms)
	 data = read( 100 )

	 if not(data == nil) then
	    add(data)
	    added_data = true
	 end

      until data == nil

      -- Commit the changes (if any)
      if( added_data == true ) then
	 commit()
	 print("Committed data")
      end
   until stop()

   print("DONE!")
end
