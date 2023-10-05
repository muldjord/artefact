-- ===============
-- Name mapping
-- ---------------

local CLASSNAME = "lensbarcode"

local LENSTYPE_NOT_FOUND = 0 

-- C regexp: "\\+H530[A-Z]{2,2}[0-9]{2,2}[A-Z]{2,2}[0-9]{3,3}.{2,3}"
local REGEXP_ALCON_BARCODE = "%+H530%u%u%d%d%u%u%d%d%d...?"
local ALCON_BARCODE_ID = 1 

-- C regexp: "\\+[0-9]{14,14} {0,1}.{1,2} {0,1}"
local REGEXP_ALCON_SERIAL_OLD = "%+%d%d%d%d%d%d%d%d%d%d%d%d%d%d ?..? ?"
local ALCON_SERIAL_OLD_ID = 2

-- C regexp: "\\+[0-9]{16,16}.{2,2}"
local REGEXP_ALCON_SERIAL = "%+%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d.."
local ALCON_SERIAL_ID = 3

-- C regexp: [0-9]{8,10}X[0-9]{8,8}X[0-9]{4,4}"
local REGEXP_AMO = "%d%d%d%d%d%d%d%d%d?%d?X%d%d%d%d%d%d%d%dX%d%d%d%d"
local AMO_ID = 4

-- C regexp: "AC51L[0-9]{5,5}[0-9]{10,10}[0-9]{4,4}"
local REGEXP_AMO_AC51L = "AC51L%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
local AMO_AC51L_ID = 5

-- C regexp: "ZA9003[0-9]{4,4}[0-9]{10,10}[0-9]{4,4}"
local REGEXP_AMO_ZA9003 = "ZA9003%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
local AMO_ZA9003_ID = 6

-- C regexp: "ZCB00[0-9]{19,19}"
local REGEXP_AMO_ZCB00 = "ZCB00%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d"
local AMO_ZCB00_ID = 7

-- C regexp: "[0-9]{4,4}[0-9]{4,4}[0-9]{7,7}[A-Z]{6,6}[0-9]{6,6}"
local REGEXP_MORCHER = "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%a%a%a%a%a%a%d%d%d%d%d%d"
local MORCHER_ID = 8

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
  return _classify(data) 
end

function _classify(data)
  local info = _getLensInfo(data)
  if not info[0] then
    invalidData("Serial not found")
    return false
  end
  if not info[1] then
    invalidData("Barcode not found");
    return false;
  end

  addClass(CLASSNAME)
  
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
  printInterpreterMessage("Running Lens Barcode  interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
 
  local data = loadDataFromFile(filename)
  _interpret(data, requestedType)
end

function _alconBarcode(parent, serial, barcode)

  local model = string.sub(barcode, 6, 11)
  if not model then return false end

  local integer = string.sub(barcode, 12, 13)
  if not integer then return false end

  local decimal = string.sub(barcode, 14, 14)
  if not integer then return false end

  _addValue(parent, "vendor", "Alcon")
  _addValue(parent, "dioptre", integer .. "." .. decimal)
  _addValue(parent, "model", model)
  _addValue(parent, "barcode", barcode)
  _addValue(parent, "serial", serial)

  return true
end

function _morcherBarcode(parent, barcode) 

  local integer = string.sub(barcode, 6, 7)
  if not integer then return false end

  local decimal = string.sub(barcode, 8, 8)
  if not integer then return false end

  local dioptre
  if string.sub(barcode, 5, 6) == "1" then
    dioptre = "-" .. integer .. "." .. decimal
  else 
    dioptre = integer .. "." .. decimal
  end

  local typecode = string.sub(barcode, 0, 4)
  if not typecode then return false end

  local model
  if typecode == "6741" then
    model = "Aniridia 67D"
  end

  _addValue(parent, "vendor", "Morcher")
  if model then _addValue(parent, "model", model) end
  _addValue(parent, "serial", string.sub(barcode, 10, 16))
  _addValue(parent, "barcode", barcode)
  _addValue(parent, "dioptre", dioptre)

  return true
end

amolenses = {
  ["15506650"] = {"TECNIS Z9000",  "5.0"},
  ["15506651"] = {"TECNIS Z9000",  "6.0"},
  ["15506652"] = {"TECNIS Z9000",  "7.0"},
  ["15506653"] = {"TECNIS Z9000",  "8.0"},
  ["15506654"] = {"TECNIS Z9000",  "9.0"},
  ["15506655"] = {"TECNIS Z9000", "10.0"},
  ["15506656"] = {"TECNIS Z9000", "10.5"},
  ["15506657"] = {"TECNIS Z9000", "11.0"},
  ["15506658"] = {"TECNIS Z9000", "11.5"},
  ["15506659"] = {"TECNIS Z9000", "12.0"},
  ["15506660"] = {"TECNIS Z9000", "12.5"},
  ["15506661"] = {"TECNIS Z9000", "13.0"},
  ["15506662"] = {"TECNIS Z9000", "13.5"},
  ["15506663"] = {"TECNIS Z9000", "14.0"},
  ["15506664"] = {"TECNIS Z9000", "14.5"},
  ["15506665"] = {"TECNIS Z9000", "15.0"},
  ["15506666"] = {"TECNIS Z9000", "15.5"},
  ["15506667"] = {"TECNIS Z9000", "16.0"},
  ["15506668"] = {"TECNIS Z9000", "16.5"},
  ["15506669"] = {"TECNIS Z9000", "17.0"},
  ["15506670"] = {"TECNIS Z9000", "17.5"},
  ["15506671"] = {"TECNIS Z9000", "18.0"},
  ["15506672"] = {"TECNIS Z9000", "18.5"},
  ["15506673"] = {"TECNIS Z9000", "19.0"},
  ["15506674"] = {"TECNIS Z9000", "19.5"},
  ["15506675"] = {"TECNIS Z9000", "20.0"},
  ["15506676"] = {"TECNIS Z9000", "20.5"},
  ["15506677"] = {"TECNIS Z9000", "21.0"},
  ["15506678"] = {"TECNIS Z9000", "21.5"},
  ["15506679"] = {"TECNIS Z9000", "22.0"},
  ["15506680"] = {"TECNIS Z9000", "22.5"},
  ["15506681"] = {"TECNIS Z9000", "23.0"},
  ["15506682"] = {"TECNIS Z9000", "23.5"},
  ["15506683"] = {"TECNIS Z9000", "24.0"},
  ["15506684"] = {"TECNIS Z9000", "24.5"},
  ["15506685"] = {"TECNIS Z9000", "25.0"},
  ["15506686"] = {"TECNIS Z9000", "25.5"},
  ["15506687"] = {"TECNIS Z9000", "26.0"},
  ["15506688"] = {"TECNIS Z9000", "26.5"},
  ["15506689"] = {"TECNIS Z9000", "27.0"},
  ["15506690"] = {"TECNIS Z9000", "27.5"},
  ["15506691"] = {"TECNIS Z9000", "28.0"},
  ["15506692"] = {"TECNIS Z9000", "28.5"},
  ["15506693"] = {"TECNIS Z9000", "29.0"},
  ["15506694"] = {"TECNIS Z9000", "29.5"},
  ["15506695"] = {"TECNIS Z9000", "30.0"},
  ["15504247"] = {"CeeOn 812C"  , "10.0"},
  ["15504248"] = {"CeeOn 812C"  , "09.0"},
  ["15504249"] = {"CeeOn 812C"  , "08.0"},
  ["15504250"] = {"CeeOn 812C"  , "07.0"},
  ["15504251"] = {"CeeOn 812C"  , "06.0"},
  ["15504252"] = {"CeeOn 812C"  , "05.0"},
  ["15504253"] = {"CeeOn 812C"  , "04.0"},
  ["15504254"] = {"CeeOn 812C"  , "03.0"},
  ["15504255"] = {"CeeOn 812C"  , "02.0"},
  ["15504256"] = {"CeeOn 812C"  , "01.0"},
  ["15504257"] = {"CeeOn 812C"  , "00.0"},
  ["15504258"] = {"CeeOn 812C"  , "01.0"},
  ["15504259"] = {"CeeOn 812C"  , "02.0"},
  ["15504260"] = {"CeeOn 812C"  , "03.0"},
  ["15504261"] = {"CeeOn 812C"  , "04.0"},
  ["15504262"] = {"CeeOn 812C"  , "05.0"},
  ["15504263"] = {"CeeOn 812C"  , "06.0"},
  ["15504264"] = {"CeeOn 812C"  , "07.0"},
  ["15504265"] = {"CeeOn 812C"  , "08.0"},
  ["15504267"] = {"CeeOn 812C"  , "09.0"},
  ["15504269"] = {"CeeOn 812C"  , "10.0"},
  ["15504270"] = {"CeeOn 812C"  , "10.5"},
  ["15504271"] = {"CeeOn 812C"  , "11.0"},
  ["15504272"] = {"CeeOn 812C"  , "11.5"},
  ["15504273"] = {"CeeOn 812C"  , "12.0"},
  ["15504274"] = {"CeeOn 812C"  , "12.5"},
  ["15504275"] = {"CeeOn 812C"  , "13.0"},
  ["15504276"] = {"CeeOn 812C"  , "13.5"},
  ["15504277"] = {"CeeOn 812C"  , "14.0"},
  ["15504278"] = {"CeeOn 812C"  , "14.5"},
  ["15504279"] = {"CeeOn 812C"  , "15.0"},
  ["15504280"] = {"CeeOn 812C"  , "15.5"},
  ["15504281"] = {"CeeOn 812C"  , "16.0"},
  ["15504282"] = {"CeeOn 812C"  , "16.5"},
  ["15504283"] = {"CeeOn 812C"  , "17.0"},
  ["15504284"] = {"CeeOn 812C"  , "17.5"},
  ["15504285"] = {"CeeOn 812C"  , "18.0"},
  ["15504286"] = {"CeeOn 812C"  , "18.5"},
  ["15504287"] = {"CeeOn 812C"  , "19.0"},
  ["15504288"] = {"CeeOn 812C"  , "19.5"},
  ["15504289"] = {"CeeOn 812C"  , "20.0"},
  ["15504290"] = {"CeeOn 812C"  , "20.5"},
  ["15504291"] = {"CeeOn 812C"  , "21.0"},
  ["15504292"] = {"CeeOn 812C"  , "21.5"},
  ["15504293"] = {"CeeOn 812C"  , "22.0"},
  ["15504294"] = {"CeeOn 812C"  , "22.5"},
  ["15504295"] = {"CeeOn 812C"  , "23.0"},
  ["15504296"] = {"CeeOn 812C"  , "23.5"},
  ["15504297"] = {"CeeOn 812C"  , "24.0"},
  ["15504298"] = {"CeeOn 812C"  , "24.5"},
  ["15504299"] = {"CeeOn 812C"  , "25.0"},
  ["15504300"] = {"CeeOn 812C"  , "25.5"},
  ["15504301"] = {"CeeOn 812C"  , "26.0"},
  ["15504302"] = {"CeeOn 812C"  , "26.5"},
  ["15504303"] = {"CeeOn 812C"  , "27.0"},
  ["15504304"] = {"CeeOn 812C"  , "27.5"},
  ["15504305"] = {"CeeOn 812C"  , "28.0"},
  ["15504306"] = {"CeeOn 812C"  , "28.5"},
  ["15504307"] = {"CeeOn 812C"  , "29.0"},
  ["15504308"] = {"CeeOn 812C"  , "29.5"},
  ["15504309"] = {"CeeOn 812C"  , "30.0"},
  ["15504310"] = {"CeeOn 812C"  , "31.0"},
  ["15504311"] = {"CeeOn 812C"  , "32.0"},
  ["15504312"] = {"CeeOn 812C"  , "33.0"},
  ["15504313"] = {"CeeOn 812C"  , "34.0"},
  ["15504314"] = {"CeeOn 812C"  , "35.0"},
  ["15504315"] = {"CeeOn 812C"  , "36.0"},
  ["15504316"] = {"CeeOn 812C"  , "37.0"},
  ["15504317"] = {"CeeOn 812C"  , "38.0"},
  ["15504318"] = {"CeeOn 812C"  , "39.0"},
  ["15504319"] = {"CeeOn 812C"  , "40.0"}
}

function _amoBarcode(parent, barcode)

  local firstX = string.find(barcode, "X")
  if not firstX then return false end

  local secondX = string.find(barcode, "X", firstX + 1)
  if not secondX then return false end

  local varenr = string.sub(barcode, firstX+1, secondX-1)
  if not varenr then return false end

  local lensinfo = amolenses[varenr]
  if not lensinfo then return false end

  local model = lensinfo[1]
  if not model then return false end

  local dioptre = lensinfo[2]
  if not dioptre then return false end

  _addValue(parent, "vendor", "AMO")
  _addValue(parent, "model", model)
  _addValue(parent, "dioptre", dioptre)
  _addValue(parent, "barcode", barcode)
--  _addValue(parent, "partnumber", varenr)

  return true
end

function _amoAC51LBarcode(parent, barcode)
 local integer = string.sub(barcode, 8, 9)
 if not integer then return false end

 local decimal = string.sub(barcode, 10, 10)
 if not decimal then return false end

 _addValue(parent, "vendor", "AMO")
 _addValue(parent, "model", "AC51L")
 _addValue(parent, "dioptre", integer .. "." .. decimal)
 _addValue(parent, "barcode", barcode)

  return true
end

function _amoZA9003Barcode(parent, barcode)
  local integer = string.sub(barcode, 8, 9)
  if not integer then return false end

  local decimal = string.sub(barcode, 10, 10)
  if not decimal then return false end

 _addValue(parent, "vendor", "AMO")
 _addValue(parent, "model", "ZA9003")
 _addValue(parent, "dioptre", integer .. "." .. decimal)
 _addValue(parent, "barcode", barcode)
-- _addValue(parent, "serial", serial)

  return true
end

function _amoZCB00Barcode(parent, barcode)
  local integer = string.sub(barcode, 8, 9)
  if not integer then return false end

  local decimal = string.sub(barcode, 10, 10)
  if not decimal then return false end
  
  local dioptre = integer .. "." .. decimal

  if integer[0] == '0' then
    dioptre = string.sub(dioptre, 1, 3)
    if not dioptre then return false end
  end

 _addValue(parent, "vendor", "AMO")
 _addValue(parent, "model", "ZCB00")
 _addValue(parent, "dioptre", integer .. "." .. decimal)
 _addValue(parent, "barcode", barcode)
-- _addValue(parent, "serial", serial) 

  return true
end

function _interpretLensBarcode(data) 
 -- detect lens vendor and type
  
  local info = _getLensInfo(data)
  
  if not info[0] then
    invalidData("Serial not found")
    return false
  end
  if not info[1] then
    invalidData("Barcode not found");
    return false;
  end

  local serial = info[0]
  local barcode = info[1]

  local lensType = _lensType(barcode)
  
  if lensType == LENSTYPE_NOT_FOUND then
    invalidData("Type not recognized")
    return false
  end

  root = beginGroup(0, "lensbarcode")

  if lensType == ALCON_BARCODE_ID then
    if not _alconBarcode(root, serial, barcode) then return false end
  elseif lensType == ALCON_SERIAL_OLD_ID then

  elseif lensType == ALCON_SERIAL_ID then

  elseif lensType == AMO_ID then

  elseif lensType == AMO_AC51L_ID then

  elseif lensType == AMO_ZA9003_ID then

  elseif lensType == AMO_ZCB00_ID then

  elseif lensType == MORCHER_ID then

  endGroup()

  end
end

function _interpret(data, requestedType)

  if requestedType == "lensbarcode" then
    _interpretLensBarcode(data) 
  else
    printInterpreterMessage("Invalid requested class")
    return false
  end

  setResume(resume_text)
end


--=============================================================================
--
-- Local Functions
-------------------------------------------------------------------------------
function _loadUtils()
  local lua_base_dir = luaBaseDir()
  local utils = lua_base_dir .. "/" .. "utils.lua"
  dofile(utils)
  local base64 = lua_base_dir .. "/" .. "base64.lua"
  dofile(base64)
end

function _getLensInfo(data) 
  info = {}

  local splitpos = string.find(data, " ")
  if not splitpos then
    invalidData("Invalid format")
    return info
  end

  local serial = b64dec(string.sub(data, 0, splitpos-2))
  if not serial then
    invalidData("Serial not found")
    return info
  end

  local barcode = b64dec(string.sub(data, splitpos+1, string.len(data)-1))
  if not barcode then
    invalidData("Barcode not found")
    return info
  end

  info[0] = serial;
  info[1] = barcode;

  return info
end

-- detect lens type from barcode
function _lensType(barcode)
  if string.find(barcode, REGEXP_ALCON_BARCODE) then
    return ALCON_BARCODE_ID
  -- its important that ALCON_SERIAL is before ALCON_SERIAL_OLD as
  -- the latter matches the first.
  elseif string.find(barcode, REGEXP_ALCON_SERIAL) then
    return ALCON_SERIAL_ID
  elseif string.find(barcode, REGEXP_ALCON_SERIAL_OLD) then
    return ALCON_SERIAL_OLD_ID
  elseif string.find(barcode, REGEXP_AMO) then
    return AMO_ID
  elseif string.find(barcode, REGEXP_AMO_AC51L) then
    return AMO_AC51L_ID
  elseif string.find(barcode, REGEXP_AMO_ZA9003) then
    return AMO_ZA9003_ID
  elseif string.find(barcode, REGEXP_AMO_ZCB00) then
    return AMO_ZCB00_ID
  elseif string.find(barcode, REGEXP_MORCHER) then
    return MORCHER_ID
  else
    return LENSTYPE_NOT_FOUND 
  end
end
--=============================================================================
--
-- Test function
--
-------------------------------------------------------------------------------

function test()
  dofile 'test.lua'
  dofile 'utils.lua'
  dofile 'base64.lua'

  -- Testing classifier
  local data_serial = "+080917671720091D"
  local data_barcode = "+H530CZ70BD300P11"

  local data_serialb64 = "KzA4MDkxNzY3MTcyMDA5MUQK"
  local data_barcodeb64 = "K0g1MzBDWjcwQkQzMDBQMTEK"

  local data_b64 = data_serialb64 .. ' ' .. data_barcodeb64
  local data_invalid_format = data_serialb64 .. 'x' .. data_barcodeb64

  local info = _getLensInfo(data_b64)
  TEST_EQUAL_STR(data_serial, info[0], "Testing decoding serial")
  TEST_EQUAL_STR(data_barcode, info[1], "Testing decoding barcode")

  testReset()
  _classify(data_b64)
  TEST_EQUAL_INT(testAddClassCount_, 1, "Adding class from valid data")
  
  testReset()
  _classify(data_invalid_format)
  TEST_EQUAL_INT(testAddClassCount_, 0, "Class form invalid formated data added")

 -- Testing interpreter
  local data_alcon_barcode_1 = "+H530CZ70BD300P11"
  local data_alcon_barcode_2 = "+H530MA60MA050P1_"

  local data_alcon_serial_old_1 = "+789834598798734D"
  local data_alcon_serial_old_2 = "+09181842586030S_"
  local data_alcon_serial_old_3 = "+11181996862070_9"
  local data_alcon_serial_old_4 = "+07181717302012_X"

  local data_alcon_serial_1 = "+113651060375009068"
  local data_alcon_serial_2 = "+1203120600109072B0"
  local data_alcon_serial_3 = "+1133410601176015B6"

  local data_amo_barcode = "669324256X15506671X1105"
  local data_amo_ac51l = "AC51L0022070600705090810"
  local data_amo_za9003 = "ZA9003023554213706101011"
  local data_amo_zcb00 = "ZCB000006550382808050510"

  local data_morcher = "674122151058821ADMBHA311208"

  -- Test identifying functions
  TEST_EQUAL_INT(ALCON_BARCODE_ID, _lensType(data_alcon_barcode_1), "Identifying valid ALCON_BARCODE lens type 1")
  TEST_EQUAL_INT(ALCON_BARCODE_ID, _lensType(data_alcon_barcode_2), "Identifying valid ALCON_BARCODE lens type 2")
 
  TEST_EQUAL_INT(ALCON_SERIAL_OLD_ID, _lensType(data_alcon_serial_old_1), "Identifying valid ALCON_SERIAL_OLD lens type 1")
  TEST_EQUAL_INT(ALCON_SERIAL_OLD_ID, _lensType(data_alcon_serial_old_2), "Identifying valid ALCON_SERIAL_OLD lens type 2")
  TEST_EQUAL_INT(ALCON_SERIAL_OLD_ID, _lensType(data_alcon_serial_old_3), "Identifying valid ALCON_SERIAL_OLD lens type 3")
  TEST_EQUAL_INT(ALCON_SERIAL_OLD_ID, _lensType(data_alcon_serial_old_4), "Identifying valid ALCON_SERIAL_OLD lens type 4")
 

  TEST_EQUAL_INT(ALCON_SERIAL_ID, _lensType(data_alcon_serial_1), "Identifying valid ALCON_SERIALlens type 1")
  TEST_EQUAL_INT(ALCON_SERIAL_ID, _lensType(data_alcon_serial_2), "Identifying valid ALCON_SERIALlens type 2")
  TEST_EQUAL_INT(ALCON_SERIAL_ID, _lensType(data_alcon_serial_3), "Identifying valid ALCON_SERIALlens type 3")

  TEST_EQUAL_INT(AMO_ID, _lensType(data_amo_barcode), "Identifying valid AMO lens type")
  
  TEST_EQUAL_INT(AMO_AC51L_ID, _lensType(data_amo_ac51l), "Identifying valid AMO_AC51L lens type")
  TEST_EQUAL_INT(AMO_ZA9003_ID, _lensType(data_amo_za9003), "Identifying valid AMO_ZA9003 lens type")
  TEST_EQUAL_INT(AMO_ZCB00_ID, _lensType(data_amo_zcb00), "Identifying valid AMO_ZCB00 lens type")
  TEST_EQUAL_INT(MORCHER_ID, _lensType(data_morcher), "Identifying valid MORCHER lens type")

  -- Test class handling
  testReset()
  local alconSerial = "+080917671720091D"
  local alconBarcode = "+H530CZ70BD300P11"
  TEST_TRUE(_alconBarcode("alcontest", alconSerial, alconBarcode), "Testing return value of alconBarcode")
  TEST_EQUAL_INT(5, testAddValueCount_, "Testing number of fields add in alconBarcode")  
  TEST_EQUAL_STR("Alcon", testValueList_["alcontest"]["vendor"], "Testing vendor field in alcon barcode")
  TEST_EQUAL_STR("+080917671720091D", testValueList_["alcontest"]["serial"], "Testing serial field in alcon barcode")
  TEST_EQUAL_STR("+H530CZ70BD300P11", testValueList_["alcontest"]["barcode"], "Testing serial field in alcon barcode")
  TEST_EQUAL_STR("CZ70BD", testValueList_["alcontest"]["model"], "Testing serial field in alcon barcode")
  TEST_EQUAL_STR("30.0", testValueList_["alcontest"]["dioptre"], "Testing serial field in alcon barcode")

  testReset()
  local morcherBarcode = "940122151188194AFECEA310510";
  TEST_TRUE(_morcherBarcode("morchertest", morcherBarcode), "Testing return value of morcher barcode")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing number of fields add in morcher barcode")  
  TEST_EQUAL_STR("Morcher", testValueList_["morchertest"]["vendor"], "Testing vendor field in morcher barcode")
  TEST_FALSE(testValueList_["morchertest"]["model"], "Testing model field in morcher barcode")
  TEST_EQUAL_STR("188194A", testValueList_["morchertest"]["serial"], "Testing serial field in morcher barcode")
  TEST_EQUAL_STR("21.5", testValueList_["morchertest"]["dioptre"], "Testing dioptre field in morcher barcocde")

  testReset()
  local amoceeonBarcode = "435137096X15504294X0506"
  TEST_EQUAL_INT(AMO_ID, _lensType(amoceeonBarcode), "Identifying amo cee on barcode")
  TEST_TRUE(_amoBarcode("amoceeontest", amoceeonBarcode), "Testing return value of amo cee on barcode")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing number of fields add in amo cee on barcode")  
  TEST_EQUAL_STR("AMO", testValueList_["amoceeontest"]["vendor"], "Testing vendor field in amo cee on barcode")
  TEST_EQUAL_STR("CeeOn 812C", testValueList_["amoceeontest"]["model"], "Testing serial field in amo cee on barcode")
  TEST_FALSE(testValueList_["amoceeontest"]["serial"], "Testing serial field in amo cee on barcode")
  TEST_EQUAL_STR("22.5", testValueList_["amoceeontest"]["dioptre"], "Testing dioptre field in amo cee on barcocde")

  testReset()
  local amotecnisBarcode = "669324256X15506671X1105"
  TEST_EQUAL_INT(AMO_ID, _lensType(amotecnisBarcode), "Identifying amo tecnis barcode")
  TEST_TRUE(_amoBarcode("amotecnistest", amotecnisBarcode), "Testing return value of amo tecnis barcode")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing number of fields add in amo tecnis barcode")  
  TEST_EQUAL_STR("AMO", testValueList_["amotecnistest"]["vendor"], "Testing vendor field in amo tecnis barcode")
  TEST_EQUAL_STR("TECNIS Z9000", testValueList_["amotecnistest"]["model"], "Testing serial field in amo tecnis barcode")
  TEST_FALSE(testValueList_["amotecnistest"]["serial"], "Testing serial field in amo tecnis barcode")
  TEST_EQUAL_STR("18.0", testValueList_["amotecnistest"]["dioptre"], "Testing dioptre field in amo tecnis barcocde") 

  testReset()
  local amozcb00Barcode = "ZCB000011047820710010112"
  TEST_EQUAL_INT(AMO_ZCB00_ID, _lensType(amozcb00Barcode), "Identifying amo zcb00 barcode")
  TEST_TRUE(_amoZCB00Barcode("amozcb00test", amozcb00Barcode), "Testing return value of amo zcb00 barcode")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing number of fields add in amo zcb00 barcode")
  TEST_EQUAL_STR("AMO", testValueList_["amozcb00test"]["vendor"], "Testing vendor field in amo zcb00 barcode")
  TEST_EQUAL_STR("ZCB00", testValueList_["amozcb00test"]["model"], "Testing model field in amo zcb00 barcode")
  TEST_EQUAL_STR(amozcb00Barcode, testValueList_["amozcb00test"]["barcode"], "Testing barcode field in amo zcb00 barcode")
  TEST_EQUAL_STR("11.0", testValueList_["amozcb00test"]["dioptre"], "Testing diptre field in amo zcb00 barcode")
  TEST_FALSE(testValueList_["amozcb00test"]["serial"], "Testing serial field in amo zcb00 barcode")

--  _amoZA9003Barcode
  testReset()
  local amoza9003Barcode = "ZA9003025056958706101011"
  TEST_EQUAL_INT(AMO_ZA9003_ID, _lensType(amoza9003Barcode), "Identifying amo za9003 barcode")
  TEST_TRUE(_amoZA9003Barcode("amoza9003test", amoza9003Barcode), "Testing return value of amo za9003 barcode")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing number of fields add in amo za9003 barcode")
  TEST_EQUAL_STR("AMO", testValueList_["amoza9003test"]["vendor"], "Testing vendor field in amo za9003 barcode")
  TEST_EQUAL_STR("ZA9003", testValueList_["amoza9003test"]["model"], "Testing model field in amo za9003 barcode")
  TEST_EQUAL_STR(amoza9003Barcode, testValueList_["amoza9003test"]["barcode"], "Testing barcode field in amo za9003 barcode")
  TEST_EQUAL_STR("25.0", testValueList_["amoza9003test"]["dioptre"], "Testing diptre field in amo za9003 barcode")
  TEST_FALSE(testValueList_["amoza9003test"]["serial"], "Testing serial field in amo za9003 barcode")

-- _amoAC51LBarcode
  testReset()
  local amoac51lBarcode = "AC51L0022070600705090810"
  TEST_EQUAL_INT(AMO_AC51L_ID, _lensType(amoac51lBarcode), "Identifying amo ac51l barcode")
  TEST_TRUE(_amoAC51LBarcode("amoac51ltest", amoac51lBarcode), "Testing return value of amo ac51l barcode")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing number of fields add in amo ac51l barcode")
  TEST_EQUAL_STR("AMO", testValueList_["amoac51ltest"]["vendor"], "Testing vendor field in amo ac51l barcode")
  TEST_EQUAL_STR("AC51L", testValueList_["amoac51ltest"]["model"], "Testing model field in amo ac51l barcode")
  TEST_EQUAL_STR(amoac51lBarcode, testValueList_["amoac51ltest"]["barcode"], "Testing barcode field in amo ac51l barcode")
  TEST_EQUAL_STR("22.0", testValueList_["amoac51ltest"]["dioptre"], "Testing diptre field in amo ac51l barcode")
  TEST_FALSE(testValueList_["amoac51ltest"]["serial"], "Testing serial field in amo ac51l barcode")

end
