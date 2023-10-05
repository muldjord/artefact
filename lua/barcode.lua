-- VERSION:
--   0.5.10 Added more Morcher lens and changing naming of Morcher lenses.
--   0.5.9: Fixed naming of Staar ICL lens
--   0.5.8: Added enVista MX60A lens
--   0.5.7: Added amo_ac51l_old lens type and fixed a bug
--   0.5.6: Removed test code testing old AC51l and added new test for AC51l
--   0.5.5: Added Unknown barcode class 
--   0.5.4: Added new type of Amo AC51L barcode
--   0.5.3: Added Zeiss AT Torbi lens and Zeiss AT Lisa Lens
--   0.5.2: Fixed error in interpreter code for ZCB00
--   0.5.1: Changed model AC51l to AC51L

--=============================================================================
-- Read the data from <filename> and determ which classes it contains.
--
-- The following functions are available in the classify function:
--    - addClass(classname)
--          Mark data content as classname.
--    - invalidData(errormsg)
--          Report that the data is invalid.
-------------------------------------------------------------------------------
function classify(filename)
  _loadUtils()
  printClassifierMessage("Running Lens Barcode classifier...\n")
  local data = loadDataFromFile(filename)
  _classify(data)
end

lenses = {
  {"dummy_lens", "dummy"},

-- +H530SA60AT300P16
-- +H530SA60AT160P1A (VIRKER IKKE!!!!)
-- +H530SA30AL105P11
-- +H530MA60MA050P1_ (NOTE: '_' == ' ' which disappears when located last)
-- #define BARCODE_ALCON_BARCODE "\\+H530[A-Z]{2,2}[0-9A-Z]{4,4}[0-9]{3,3}.{2,3}"
  {"^%+H530%u%u....%d%d%d...?$", "alcon"},

-- +10181910819045AD (VIRKER IKKE!!!!)
-- +09181842586030S_ (NOTE: '_' == '' .. NOT ' ' as the others)
-- +789834598798734D
-- +06181652504081_/ (NOTE: '_' == ' ')
-- +11181996862070_9 (NOTE: '_' == ' ')
-- +07181717302012_X (NOTE: '_' == ' ')
-- #define REGEXP_ALCON_SERIAL_OLD "\\+[0-9]{14,14} {0,1}.{1,2} {0,1}"

--  {"^%+%d%d%d%d%d%d%d%d%d%d%d%d%d%d ?..? ?$", "alcon_serial_old"},

-- +113651060375009068
-- +1203120600109072B0
-- +1133410601176015B6
-- #define REGEXP_ALCON_SERIAL "\\+[0-9]{16,16}.{2,2}"

--  {"^%+%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d..$", "alcon_serial"},

-- 62290601X15504275X1208
-- 435126406X15504298X0506
-- 063625414X15504295X0208
-- #define REGEXP_AMO "[0-9]{8,10}X[0-9]{8,8}X[0-9]{4,4}"
  {"^%d%d%d%d%d%d%d%d%d?%d?X%d%d%d%d%d%d%d%dX%d%d%d%d$", "amo"},

-- AC51L0022070600705090810
-- #define REGEXP_AMO_AC51L "AC51L[0-9]{5,5}[0-9]{10,10}[0-9]{4,4}"
  {"^AC51L%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d$", "amo_ac51l_old"}, --<-- This one does not include serial

-- +H706AC51L001101/$7160150405G
-- #define BARCODE_AMO_AC51L "\\+H706AC51L[0-9]{6,6}/\\$[0-9]{10,10}[A-Z]"  
  {"^%+H706AC51L%d%d%d%d%d%d/%$%d%d%d%d%d%d%d%d%d%d%u", "amo_ac51l"},

-- ZA9003023554213706101011
-- #define REGEXP_AMO_ZA9003 "ZA9003[0-9]{4,4}[0-9]{10,10}[0-9]{4,4}"
  {"^ZA9003%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d$", "amo_za9003"},

-- ZCB000006550382808050510
-- #define REGEXP_AMO_ZCB00 "ZCB00[0-9]{19,19}"
  {"^ZCB00%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d$", "amo_zcb00"},

-- ZCT375016081374516010121
-- #define BARCODE_AMO_ZCT375 "ZCT375[0-9]{18,18}"
  {"^ZCT375%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d$", "amo_zct375"},

-- PCB000024055477016040419
-- #define BARCODE_AMO_PCB00 "PCB00[0-9]{19,19}"
  {"^PCB00%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d$", "amo_pcb00"},

-- 674122151058821ADMBHA311208
-- #define REGEXP_MORCHER "[0-9]{4,4}[0-9]{4,4}[0-9]{7,7}[A-Z]{6,6}[0-9]{6,6}"
  {"^%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%u%u%u%u%u%u%d%d%d%d%d%d$", "morcher"},

-- +H706ZCT30002401/$8282741306J
-- +H706ZCT10002351/$81191512019
-- #define BARCODE_TECNIS_ZCT "\\+H[0-9]{3,3}ZCT[0-9]{8,8}/\\$[0-9]{10,11}[A-Z]{0,1}"
  {"^%+H%d%d%dZCT%d%d%d%d%d%d%d%d/%$%d%d%d%d%d%d%d%d%d%d.?$", "tecnis_zct"},

-- +H706VRSA54030P1/$81303310125
-- #define BARCODE_ABBOTT_VERISYSE "\\+H706[A-Z]{4,4}[0-9]{2,2}[0-9]{3,3}.{2,2}/\\$[0-9]{11,11}"
  {"^%+H706%u%u%u%u%d%d%d%d%d../%$%d%d%d%d%d%d%d%d%d%d%d$", "abbott_verisyse"},

-- .709M07.0/03.0
-- "\.[0-9A-Z]{4}[0-9]{2,2}\.[0-9]/[0-9]{2,2}\.[0-9]"
  {"^%.709M%d%d%.%d/%d%d%.%d", "zeiss_at_torbi_709m"},

-- .939MP12.5/01.0
-- "\.[0-9A-Z]{5}[0-9]{2,2}\.[0-9]/[0-9]{2,2}\.[0-9]"
  {"^%.939MP%d%d.%d/%d%d%.%d", "zeiss_at_lisa_939mp"},

-- 076401458612221715123121A1897716
-- #define BARCODE_STAAR_ICL_VICM "076401458[0-9]{13,13}21A[0-9]{7,7}"
  {"^076401458%d%d%d%d%d%d%d%d%d%d%d%d%d21A%d%d%d%d%d%d%d", "staar_icl_vicm"},

-- 076401458636391716103121D182175
-- #define BARCODE_STAAR_ICL_VTICM "076401458[0-9]{13,13}21D[0-9]{6,6}"
  {"^076401458%d%d%d%d%d%d%d%d%d%d%d%d%d21D%d%d%d%d%d%d", "staar_icl_vticm"},

-- MX60A2250
-- #define BARCODE_ENVISTA_MX60A "MX60A[0-9]{4,4}"
  {"^MX60A%d%d%d%d$", "envista_mx60a"}
}

function _lensType(data)
  local barcode = cut(data, {"^"}, {"\n", "$"})
  for i, l in ipairs(lenses) do
    m = string.find(barcode, l[1])
    if m then return l[2] end
  end

  return "unknown" 
end

function _classify(data)
  printClassifierMessage("Data: '" .. data .. "'\n")
  lenstype = _lensType(data)
  if lenstype == "unknown" then
--  if not lenstype then
    invalidData("Lens barcode error\n");
    return false
  else
    printClassifierMessage("Found lens type '" .. lenstype .. "'\n")
  end

  addClass("barcode")
--  addClass(lenstype)
  return true 
end

--=============================================================================
-- Interpret data in <filename> creating a tree structure of groups containing
-- data matching the requested type <requestedType>.
--
-- The following functions are available in the interpret function:
--    - addGroup(parent, groupname) 
--          Creates a new group with a groupname refering to a parent group.
--          A value of 0 is allowed the group does not have any parents 
--          (ie. it is root).
--          The function returns a group descriptor refering to the group.
--    - addValue(groupdescriptor, valuename, value)
--          Adds a value with key valuename to a group refered to by 
--          groupdescriptor.
-- interpret() must return the root of the created tree structure.
-------------------------------------------------------------------------------
function interpret(filename, requestedType)
  _loadUtils()
  printInterpreterMessage("Running Lens Barcode Interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
  local data = loadDataFromFile(filename);
  _interpret(data, requestedType)
end

amoLenses = {
{"15506650", "TECNIS Z9000",  "5.0"},
{"15506651", "TECNIS Z9000",  "6.0"},
{"15506652", "TECNIS Z9000",  "7.0"},
{"15506653", "TECNIS Z9000",  "8.0"},
{"15506654", "TECNIS Z9000",  "9.0"},
{"15506655", "TECNIS Z9000", "10.0"},
{"15506656", "TECNIS Z9000", "10.5"},
{"15506657", "TECNIS Z9000", "11.0"},
{"15506658", "TECNIS Z9000", "11.5"},
{"15506659", "TECNIS Z9000", "12.0"},
{"15506660", "TECNIS Z9000", "12.5"},
{"15506661", "TECNIS Z9000", "13.0"},
{"15506662", "TECNIS Z9000", "13.5"},
{"15506663", "TECNIS Z9000", "14.0"},
{"15506664", "TECNIS Z9000", "14.5"},
{"15506665", "TECNIS Z9000", "15.0"},
{"15506666", "TECNIS Z9000", "15.5"},
{"15506667", "TECNIS Z9000", "16.0"},
{"15506668", "TECNIS Z9000", "16.5"},
{"15506669", "TECNIS Z9000", "17.0"},
{"15506670", "TECNIS Z9000", "17.5"},
{"15506671", "TECNIS Z9000", "18.0"},
{"15506672", "TECNIS Z9000", "18.5"},
{"15506673", "TECNIS Z9000", "19.0"},
{"15506674", "TECNIS Z9000", "19.5"},
{"15506675", "TECNIS Z9000", "20.0"},
{"15506676", "TECNIS Z9000", "20.5"},
{"15506677", "TECNIS Z9000", "21.0"},
{"15506678", "TECNIS Z9000", "21.5"},
{"15506679", "TECNIS Z9000", "22.0"},
{"15506680", "TECNIS Z9000", "22.5"},
{"15506681", "TECNIS Z9000", "23.0"},
{"15506682", "TECNIS Z9000", "23.5"},
{"15506683", "TECNIS Z9000", "24.0"},
{"15506684", "TECNIS Z9000", "24.5"},
{"15506685", "TECNIS Z9000", "25.0"},
{"15506686", "TECNIS Z9000", "25.5"},
{"15506687", "TECNIS Z9000", "26.0"},
{"15506688", "TECNIS Z9000", "26.5"},
{"15506689", "TECNIS Z9000", "27.0"},
{"15506690", "TECNIS Z9000", "27.5"},
{"15506691", "TECNIS Z9000", "28.0"},
{"15506692", "TECNIS Z9000", "28.5"},
{"15506693", "TECNIS Z9000", "29.0"},
{"15506694", "TECNIS Z9000", "29.5"},
{"15506695", "TECNIS Z9000", "30.0"},
{"15504247", "CeeOn 812C"  , "10.0"},
{"15504248", "CeeOn 812C"  , "09.0"},
{"15504249", "CeeOn 812C"  , "08.0"},
{"15504250", "CeeOn 812C"  , "07.0"},
{"15504251", "CeeOn 812C"  , "06.0"},
{"15504252", "CeeOn 812C"  , "05.0"},
{"15504253", "CeeOn 812C"  , "04.0"},
{"15504254", "CeeOn 812C"  , "03.0"},
{"15504255", "CeeOn 812C"  , "02.0"},
{"15504256", "CeeOn 812C"  , "01.0"},
{"15504257", "CeeOn 812C"  , "00.0"},
{"15504258", "CeeOn 812C"  , "01.0"},
{"15504259", "CeeOn 812C"  , "02.0"},
{"15504260", "CeeOn 812C"  , "03.0"},
{"15504261", "CeeOn 812C"  , "04.0"},
{"15504262", "CeeOn 812C"  , "05.0"},
{"15504263", "CeeOn 812C"  , "06.0"},
{"15504264", "CeeOn 812C"  , "07.0"},
{"15504265", "CeeOn 812C"  , "08.0"},
{"15504267", "CeeOn 812C"  , "09.0"},
{"15504269", "CeeOn 812C"  , "10.0"},
{"15504270", "CeeOn 812C"  , "10.5"},
{"15504271", "CeeOn 812C"  , "11.0"},
{"15504272", "CeeOn 812C"  , "11.5"},
{"15504273", "CeeOn 812C"  , "12.0"},
{"15504274", "CeeOn 812C"  , "12.5"},
{"15504275", "CeeOn 812C"  , "13.0"},
{"15504276", "CeeOn 812C"  , "13.5"},
{"15504277", "CeeOn 812C"  , "14.0"},
{"15504278", "CeeOn 812C"  , "14.5"},
{"15504279", "CeeOn 812C"  , "15.0"},
{"15504280", "CeeOn 812C"  , "15.5"},
{"15504281", "CeeOn 812C"  , "16.0"},
{"15504282", "CeeOn 812C"  , "16.5"},
{"15504283", "CeeOn 812C"  , "17.0"},
{"15504284", "CeeOn 812C"  , "17.5"},
{"15504285", "CeeOn 812C"  , "18.0"},
{"15504286", "CeeOn 812C"  , "18.5"},
{"15504287", "CeeOn 812C"  , "19.0"},
{"15504288", "CeeOn 812C"  , "19.5"},
{"15504289", "CeeOn 812C"  , "20.0"},
{"15504290", "CeeOn 812C"  , "20.5"},
{"15504291", "CeeOn 812C"  , "21.0"},
{"15504292", "CeeOn 812C"  , "21.5"},
{"15504293", "CeeOn 812C"  , "22.0"},
{"15504294", "CeeOn 812C"  , "22.5"},
{"15504295", "CeeOn 812C"  , "23.0"},
{"15504296", "CeeOn 812C"  , "23.5"},
{"15504297", "CeeOn 812C"  , "24.0"},
{"15504298", "CeeOn 812C"  , "24.5"},
{"15504299", "CeeOn 812C"  , "25.0"},
{"15504300", "CeeOn 812C"  , "25.5"},
{"15504301", "CeeOn 812C"  , "26.0"},
{"15504302", "CeeOn 812C"  , "26.5"},
{"15504303", "CeeOn 812C"  , "27.0"},
{"15504304", "CeeOn 812C"  , "27.5"},
{"15504305", "CeeOn 812C"  , "28.0"},
{"15504306", "CeeOn 812C"  , "28.5"},
{"15504307", "CeeOn 812C"  , "29.0"},
{"15504308", "CeeOn 812C"  , "29.5"},
{"15504309", "CeeOn 812C"  , "30.0"},
{"15504310", "CeeOn 812C"  , "31.0"},
{"15504311", "CeeOn 812C"  , "32.0"},
{"15504312", "CeeOn 812C"  , "33.0"},
{"15504313", "CeeOn 812C"  , "34.0"},
{"15504314", "CeeOn 812C"  , "35.0"},
{"15504315", "CeeOn 812C"  , "36.0"},
{"15504316", "CeeOn 812C"  , "37.0"},
{"15504317", "CeeOn 812C"  , "38.0"},
{"15504318", "CeeOn 812C"  , "39.0"},
{"15504319", "CeeOn 812C"  , "40.0"}
}

function _interpretAmo(data)
  if not data then return false end
 
  local firstX = string.find(data, "X")
  if not firstX then return false end

  local secondX = string.find(data, "X", firstX + 1)
  if not secondX then return false end

  local itemno = string.sub(data, firstX+1, secondX-1)
  printInterpreterMessage("Itemno: " .. itemno .. "\n")

  local lens = nil
  for i, l in ipairs(amoLenses) do
    if l[1] == itemno then
      lens = l
      break
    end
  end

  if not lens then return false end

  local model = lens[2]
  if not model then return false end
  
  local power = lens[3]
  if not power then return false end

  power = removePrefixZeros(power)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "power", power)
  addValue(root, "model", model)
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretAmoAc51l(data)
  if not data then return false end

  local serial = string.sub(data, 19, 28)

  local power = string.sub(data, 12, 13) .. "." .. string.sub(data, 14, 14)
  if not power then return false end
  power = removePrefixZeros(power)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "rawdata", data)
  addValue(root, "power", power)
  addValue(root, "model", "AC51L")
  addValue(root, "serial", serial)
  endGroup()

  return true
end 

function _interpretAmoAc51lOld(data)
  if not data then return false end

  local power = string.sub(data, 8, 9) .. "." .. string.sub(data, 10, 10)
  if not power then return false end
  power = removePrefixZeros(power)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "rawdata", data)
  addValue(root, "power", power)
  addValue(root, "model", "AC51L")
  endGroup()

  return true
end 

function _interpretAmoZa9003(data)
  if not data then return false end

  local power = string.sub(data, 8, 9) .. "." .. string.sub(data, 10, 10)
  if not power then return false end
  power = removePrefixZeros(power)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "barcode", data)
  addValue(root, "power", power)
  addValue(root, "model", "ZA9003")
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretAmoZcb00(data)
  if not data then return false end

  local power = string.sub(data, 8, 9) .. "." .. string.sub(data, 10, 10)
  if not power then return false end
  power = removePrefixZeros(power)
  
  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "power", power)
  addValue(root, "model", "ZCB00")
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretAmoZct375(data)
  if not data then return false end

  local power = string.sub(data, 8, 9) .. "." .. string.sub(data, 10, 10)
  if not power then return false end
  power = removePrefixZeros(power)
  
  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "power", power)
  addValue(root, "model", "ZCT375")
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretAmoPcb00(data)
  if not data then return false end

  local power = string.sub(data, 8, 9) .. "." .. string.sub(data, 10, 10)
  if not power then return false end
  power = removePrefixZeros(power)
  
  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "power", power)
  addValue(root, "model", "PCB00")
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretAlcon(data)
  if not data then return false end

  local barcode = cut(data, {"^"}, {"\n"})
  if not barcode then return false end
  
  local serial = cut(data, {"\n"}, {"$"})
  if not serial then return false end

  local power = string.sub(barcode, 12, 13) .. "." .. string.sub(barcode, 14, 14)
  if not power then return false end
  power = removePrefixZeros(power)

  local sign = string.sub(barcode, 15, 15)
  if not sign then return false end

  if sign == "N" then
    power = "-" .. power
  end

  local model = string.sub(barcode, 6, 11)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "alcon")
  addValue(root, "vendor", "Alcon")
  addValue(root, "serial", serial)
  addValue(root, "power", power)
  addValue(root, "model", model)
  addValue(root, "rawdata", data)
  endGroup()
  
  return true
end

--morcherLenses = {
--{"0721", "Anridia IOL 67B"}, 
--{"6604", "Retropupillar Sclera Tunnel IOL 66"},
--{"6741", "Aniridia IOL 67D"},
--{"6801", "Aniridia IOL 68"},
--{"8121", "Retropupillar IOL 81B"},
--{"9670", "Capsular Ring for Anridia 96G"}
--}
morcherLenses = {
{"0721", "Type 67B"}, 
{"6604", "Type 66"},
{"6741", "Type 67D"},
{"6801", "Type 68"},
{"8121", "Type 81B"},
{"9670", "Type 96G"}
}

-- Type code
-- | 
-- |    Strength (first character :=1 (-) :=2 (+) ) 
-- |    |  Serial
-- |    |  |      LOT number
-- |    |  |      |     Use before
-- [  ][  ][     ][    ][    ]
-- 674122151058821ADMBHA311208
-- 940122151188194AFECEA310510
function _interpretMorcher(data)
  if not data then return false end

  local power = nil
  if string.sub(data, 6, 6) == "1" then
    power = "-" .. string.sub(data, 6, 7) .. "." .. string.sub(data, 8, 8)
  else
    power = string.sub(data, 6, 7) .. "." .. string.sub(data, 8, 8)
  end
  if not power then return false end
  power = removePrefixZeros(power)
  
  local serial = string.sub(data, 9, 15)
  if not serial then return false end
  
  local typecode = string.sub(data, 1, 4)
  if not typecode then return false end
  
  local lens = nil
  for i, l in ipairs(morcherLenses) do
    if l[1] ==  typecode then
      lens = l
      break
    end
  end
  local model
  if not lens then 
    model = "Unknown"
  else
    model = lens[2]
  end

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "morcher")
  addValue(root, "vendor", "Morcher")
  addValue(root, "serial", serial)
  addValue(root, "power", power)
  addValue(root, "model", model)
  addValue(root, "rawdata", data)
  endGroup()
  
  return true
end

function _interpretTecnisZct(data)
  if not data then return false end

  local barcode = cut(data, {"^"}, {"/"})
  if not barcode then return false end
  
  local serial = cut(data, {"%$"}, {"$"})
  if not serial then return false end
  serial = string.sub(serial, 0, 10) 
  if not serial then return false end

  local power = string.sub(barcode, 13, 14) .. "." .. string.sub(barcode, 15, 15)
  if not power then return false end
  power = removePrefixZeros(power)

  local model = "ZCT" .. string.sub(barcode, 9, 11)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "amo")
  addValue(root, "vendor", "AMO")
  addValue(root, "serial", serial)
  addValue(root, "power", power)
  addValue(root, "model", model)
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretAbbottVerisyse(data)
  if not data then return false end

  local barcode = cut(data, {"^"}, {"/"})
  if not barcode then return false end
  
  local serial = cut(data, {"%$"}, {"$"})
  if not serial then return false end
  serial = string.sub(serial, 0, 10) 
  if not serial then return false end

  local power = string.sub(barcode, 12, 13) .. "." .. string.sub(barcode, 14, 14)
  if not power then return false end
  power = removePrefixZeros(power)

  local model = string.sub(barcode, 6, 11)

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "abbott")
  addValue(root, "vendor", "Abbott")
  addValue(root, "serial", serial)
  addValue(root, "power", power)
  addValue(root, "model", model)
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretZeissAtTorbi709m(data)
  if not data then return false end


  local barcode = cut(data, {"^"}, {"\n"})
  if not barcode then return false end

  local serial = cut(data, {"\n"}, {"$"})
  if not serial then return false end

  local sph = string.sub(barcode, 6, 9)
  if not sph then return false end

  local cyl = string.sub(barcode, 11, 14)
  if not cyl then return false end

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "zeiss")
  addValue(root, "vendor", "Zeiss")
  addValue(root, "type", "AT Torbi")
  addValue(root, "model", "939MP")
  addValue(root, "serial", serial)
--  addValue(vendor, "barcode", barcode)
  addValue(root, "rawdata", data)
  addValue(root, "cyl", cyl)
  addValue(root, "sph", sph)
  endGroup()

  return true
end

function _interpretZeissAtLisa939mp(data)
  if not data then return false end

  local barcode = cut(data, {"^"}, {"\n"})
  if not barcode then return false end

  local serial = cut(data, {"\n"}, {"$"})
  if not serial then return false end

  local sph = string.sub(barcode, 7, 10)
  if not sph then return false end

  local cyl = string.sub(barcode, 12, 15)
  if not cyl then return false end

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "zeiss")
  addValue(root, "vendor", "Zeiss")
  addValue(root, "type", "AT Lisa")
  addValue(root, "model", "709MP")
  addValue(root, "serial", serial)
--  addValue(vendor, "barcode", barcode)
  addValue(root, "rawdata", data)
  addValue(root, "cyl", cyl)
  addValue(root, "sph", sph)
  endGroup()

  return true
end

function _interpretStaarIclVicm(data)
  if not data then return false end

  local serial = string.sub(data, 25, 32) 
  if not serial then return false end


  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "staar")
  addValue(root, "vendor", "STAAR")
  addValue(root, "type", "Visian ICL")
  addValue(root, "serial", serial);
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretStaarIclVticm(data)
  if not data then return false end

  local serial = string.sub(data, 25, 31) 
  if not serial then return false end

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "staar")
  addValue(root, "vendor", "STAAR")
  addValue(root, "type", "Visian ICL")
  addValue(root, "model", "Toric")
  addValue(root, "serial", serial)
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpretEnvistaMx60a(data)
  if not data then return false end

  local power = string.sub(data, 6, 7) .. "." .. string.sub(data, 8, 8)
  if not power then return false end

  root = addGroup(0, "barcode")
  addValue(root, "vendor", "enVista")
  addValue(root, "model", "MX60A")
  addValue(root, "power", power)
  addValue(root, "rawdata", data)

  return true
end

function _interpretUnknownLens(data)
  if not data then return false end

  root = addGroup(0, "barcode")
--  vendor = addGroup(root, "unknown")
  addValue(root, "vendor", "Unknown")
  addValue(root, "rawdata", data)
  endGroup()

  return true
end

function _interpret(data, requestedType)
  if not requestedType == "barcode" then
    return false
  end

  lenstype = _lensType(data)

  if not lenstype then return false end

  printInterpreterMessage("Interpreting " .. lenstype .. " lens type\n")

  if lenstype == "alcon" then
    return _interpretAlcon(data)
  elseif lenstype == "amo" then
    return _interpretAmo(data)
  elseif lenstype == "amo_ac51l" then
    return _interpretAmoAc51l(data)
  elseif lenstype == "amo_ac51l_old" then
    return _interpretAmoAc51lOld(data)
  elseif lenstype == "amo_za9003" then
    return _interpretAmoZa9003(data)
  elseif lenstype == "amo_zcb00" then
    return _interpretAmoZcb00(data)
  elseif lenstype == "amo_zct375" then
    return _interpretAmoZct375(data)
  elseif lenstype == "amo_pcb00" then
    return _interpretAmoPcb00(data)
  elseif lenstype == "morcher" then
    return _interpretMorcher(data)
  elseif lenstype == "tecnis_zct" then
    return _interpretTecnisZct(data)
  elseif lenstype == "abbott_verisyse" then
    return _interpretAbbottVerisyse(data)
  elseif lenstype == "zeiss_at_torbi_709m" then
    return _interpretZeissAtTorbi709m(data)
  elseif lenstype == "zeiss_at_lisa_939mp" then
    return _interpretZeissAtLisa939mp(data)
  elseif lenstype == "staar_icl_vicm" then
    return _interpretStaarIclVicm(data)
  elseif lenstype == "staar_icl_vticm" then
    return _interpretStaarIclVticm(data)
  elseif lenstype == "envista_mx60a" then
    return _interpretEnvistaMx60a(data)
--  elseif lenstype == "unknown" then
--    return _interpretUnknownLens(data)
  end
  
  return false
end

--==============================================
-- Helper functions
--==============================================
function _loadUtils()
  local lua_base_dir = luaBaseDir()
  local utils = lua_base_dir .. "/" .. "utils.lua"
  dofile(utils)
end


--=============================================================================
--=============================================================================
--=============================================================================
--
-- Test function
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

function test()
  dofile 'test.lua'
  dofile 'utils.lua'
  TEST_EQUAL_STR("dummy", _lensType("dummy_lens"), "Testing dummy lens")

  TEST_EQUAL_STR("alcon", _lensType("+H530SA60AT300P16") , "Testing Alcon Barcode 1")
  TEST_EQUAL_STR("alcon", _lensType("+H530SA60AT160P1A") , "Testing Alcon Barcode 2")
  TEST_EQUAL_STR("alcon", _lensType("+H530SA30AL105P11") , "Testing Alcon Barcode 3")
  TEST_EQUAL_STR("alcon", _lensType("+H530MA60MA050P1 ") , "Testing Alcon Barcode 4")
  TEST_EQUAL_STR("alcon", _lensType("+H530MC50BD165P1"), "Testing Alcon Barcode 5")
  TEST_EQUAL_STR("unknown", _lensType("+H530MA60MA05AP1_"), "Testing wrong Alcon Barcode")

--  TEST_EQUAL_STR("alcon_serial_old", _lensType("+10181910819045AD"), "Testing Alcon Serial Old 1")
--  TEST_EQUAL_STR("alcon_serial_old", _lensType("+09181842586030S"), "Testing Alcon Serial Old 2")
--  TEST_EQUAL_STR("alcon_serial_old", _lensType("+789834598798734D"), "Testing Alcon Serial Old 3")
--  TEST_EQUAL_STR("alcon_serial_old", _lensType("+06181652504081 /"), "Testing Alcon Serial Old 4")
--  TEST_EQUAL_STR("alcon_serial_old", _lensType("+11181996862070 9"), "Testing Alcon Serial Old 5")
--  TEST_EQUAL_STR("alcon_serial_old", _lensType("+07181717302012 X"), "Testing Alcon Serial Old 6")
--  TEST_EQUAL_STR("unknown", _lensType("+071 1717302012_X"), "Testing wrong Alcon Serial Old")

--  TEST_EQUAL_STR("alcon_serial", _lensType("+113651060375009068"), "Testing Alcon Serial 1")
--  TEST_EQUAL_STR("alcon_serial", _lensType("+1203120600109072B0"), "Testing Alcon Serial 2")
--  TEST_EQUAL_STR("alcon_serial", _lensType("+1133410601176015B6"), "Testing Alcon Serial 3")
--  TEST_EQUAL_STR("unknown", _lensType("+113341060117A015B6"), "Testing wrong Alcon Serial") 

  TEST_EQUAL_STR("amo", _lensType("62290601X15504275X1208"), "Testing Amo 1")
  TEST_EQUAL_STR("amo", _lensType("435126406X15504298X0506"), "Testing Amo 2")
  TEST_EQUAL_STR("amo", _lensType("063625414X15504295X0208"), "Testing Amo 3")
  TEST_EQUAL_STR("unknown", _lensType("0636254X15504295X02081"), "Testing wrong Amo")

--  TEST_EQUAL_STR("amo_ac51l", _lensType("AC51L0022070600705090810"), "Testing Amo AC51L 1")
  TEST_EQUAL_STR("amo_ac51l", _lensType("+H706AC51L001101/$7160150405G"), "Testing Amo AC51L 1")
  TEST_EQUAL_STR("unknown", _lensType("AC51L00220706007050A0810"), "Testing wrong Amo AC51L")

  TEST_EQUAL_STR("amo_za9003", _lensType("ZA9003023554213706101011"), "Testing Amo ZA9003 1")
  TEST_EQUAL_STR("unknown", _lensType("ZA900302355421370610111"), "Testing wrong Amo ZA9003")

  TEST_EQUAL_STR("amo_zcb00", _lensType("ZCB000006550382808050510"), "Testing Amo ZCB00 1")
  TEST_EQUAL_STR("unknown", _lensType("ZCB0000065503828080505101"), "Testing wrong Amo ZCB00")

  TEST_EQUAL_STR("morcher", _lensType("674122151058821ADMBHA311208"), "Testing Morcher 1")
  TEST_EQUAL_STR("unknown", _lensType("674122151058821ADMB1A311208"), "Testing wrong Morcher")

  TEST_EQUAL_STR("tecnis_zct", _lensType("+H706ZCT30002401/$8282741306J"), "Testing Tecnis ZCT 1")
  TEST_EQUAL_STR("tecnis_zct", _lensType("+H706ZCT10002351/$81191512019"), "Testing Tecnis ZCT 2")
  TEST_EQUAL_STR("unknown", _lensType("+H503SN10002351/$81191512019"), "Testing wrong Tecnis ZCT")

  TEST_EQUAL_STR("zeiss_at_torbi_709m", _lensType(".709M07.0/03.0"), "Testing Zeiss AT Torbi 709M")
end
