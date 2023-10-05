function header()
   --telnet jboss 18101
   --
   --3:f:f:2908753693
   --
   --giver entry 3
   --
   --1 er nyeste
   --0 giver antal
   --========================
   --[laegenavn]
   --[unixtime]
   --[dato]
   --[dato - igen]
   --[cpr]
   --[patientnavn]
   --[diagnose (ref/kat/amd)]
   --[visus odext]
   --[visus osin]
   --<Registrering>registrerings data ()</Registrering>
   --<Visitering>visiterings data (laege)</Visitering>
   --<praktiserende>praktiserende laeges kommentarer
   --========================
   --bemaerk ingen sluttag!

   f = io.popen("echo 0:f:f:" .. getCPR() .. " | nc jboss 18101")
   num = f:read()
   f:close()

   print(num)

   if num == '0'
   then
      output("<?xml version='1.0' encoding='utf-8'?>\n")
      output("<results/>\n")
      return
   end

   f = io.popen("echo " .. num .. ":f:f:" .. getCPR() .. " | nc jboss 18101")
   doctor = f:read('*l')
   timestamp = f:read('*l')
   date1 = f:read('*l')
   date2 = f:read('*l')
   cpr = f:read('*l')
   patientname = f:read('*l')
   diagnose_code = f:read('*l')
   visus_odext = f:read('*l')
   visus_osin = f:read('*l')

   diagnose = 'empty'
   data = f:read('*l')
   while data
   do
      print(data)
      if string.find(data, '<praktiserende>')
      then
	 diagnose = string.sub(data, 16, string.len(data))
      end

      data = f:read('*l')
   end
   f:close()

   doctor = string.sub(doctor, 0, string.len(doctor) - 1)
   timestamp = string.sub(timestamp, 0, string.len(timestamp) - 1)
   diagnose = string.sub(diagnose, 0, string.len(diagnose) - 1)
   diagnose_code = string.sub(diagnose_code, 0, string.len(diagnose_code) - 1)

   output("<?xml version='1.0' encoding='utf-8'?>\n")
   output("<results>\n")
   output("  <result class=\"referral\" timestamp=\"" .. timestamp .. "\">\n")
   output("    <value name=\"doctor\" value=\"" .. UTF8Encode(doctor) .. "\"/>\n")
   output("    <value name=\"diagnose\" value=\"" .. UTF8Encode(diagnose) .. "\"/>\n")
   output("    <value name=\"diagnosecode\" value=\"" .. UTF8Encode(diagnose_code) .. "\"/>\n")
   output("  </result>\n")
   output("</results>\n")
end

function parse()
end

function footer()
end
