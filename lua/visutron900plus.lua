-- TODO: Define correct names for classes and values
--



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
  printClassifierMessage("Running VISUTRON 900+ Classifier...\n")
  local data = loadDataFromFile(filename)
  _classify(data)
end

function _checkOrigin(data)
  if not data then return false end

  return string.find(data, "VIS.-DATA.-VI") -- and string.find(data, "VI")
end

function _classify(data)
  if _checkOrigin(data) then
    printClassifierMessage("Found VISUTRON 900+ data.\n")
    printClassifierMessage("Adding visutron900plus class.\n")
    addClass("visutron900plus")
  else
    printClassifierMessage("Not VISUTRON 900+ data.\n")
    invalidData("Not VISUTRON 900+ data.\n")
  end
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
  printInterpreterMessage("Running VISUTRON 900+ Interpreter...\n")
  printInterpreterMessage("Loading file: " .. filename .. "\n")
  local data = loadDataFromFile(filename);

  data = string.gsub(data, "\013", "") -- Remove all Carriage return if any
  _interpret(data)
end 

function _cyl(data)
  if not data then return false end

--  print("DATA: ", data)
  local cyl = cut(data, {"CYL"}, {"AXIS", "$"})
--  print("CYL: ", cyl)

  if not cyl then return false end
  
  local _,_,sign,value = string.find(cyl, "([%-%+]*) -(%d+%.%d+)")

  if not value then return false end

  if sign then
    value = sign .. value
  end

  return value 
end

function _axis(data)
  if not data then return false end

--  print("DATA: ", data)
  local axis = cut(data, {"AXIS"}, {"PRISM", "$"})

--  print("AXIS: ", axis)

  if not axis then return false end
  
  local _,_,value = string.find(axis, "(%d+)")

  return value
end

--function _prism(data)
--  local prism = cut(data, {"PRISM"}, {"ACC", "$"})
--
--  if not prism then return false end
--
--  local _,_,value = string.find(prism, "(%d+%.%d+)")
--
--  return value
--end
--
--function _acc(data)
--  local acc = cut(data, {"ACC"}, {"L", "$"})
--
--  if not acc then return false end
--
--  local _,_,sign,value = string.find(acc, "([-%+]+).-(%d+%.%d+)")
--
--  if not value then return false end
--
--  if sign == "-" then
--    value = "-" .. value
--  end
--
--  return value
--end

function _sph_near(data)
  if not data then return false end

  local sph_near = cut(data, {"SPH_N"}, {"CYL", "$"})
  
  if not sph_near then return false end

  local _,_,sign,value = string.find(sph_near, "([%-%+]*) -(%d+%.%d+)")

  if not value then return false end

  if sign then
    value = sign .. value
  end

  return value
end

function _sph_far(data)
  if not data then return false end

  local sph_far = cut(data, {"SPH_F"}, {"SPH_N", "$"})
  
  if not sph_far then return false end

  local _,_,sign,value = string.find(sph_far, "([%-%+]*) -(%d+%.%d+)")

  if not value then return false end

  if sign  then
    value = sign .. value
  end

  return value
end

function _odxt(data)
  if not data then return false end

  local odxt_block = cut(data, {"R\010"}, {"L\010", "$"})
  
  if not odxt_block then return false end

  local odxt = {}
  odxt["sph_far"] = _sph_far(odxt_block)
  odxt["sph_near"] = _sph_near(odxt_block)
  odxt["cyl"] = _cyl(odxt_block)
  odxt["axis"] = _axis(odxt_block)

  return odxt
end

function _osin(data)
  if not data then return false end

  local osin_block = cut(data, {"L\010"}, {"$"})
  
  if not osin_block then return false end

  local osin = {}
  osin["sph_far"] = _sph_far(osin_block)
  osin["sph_near"] = _sph_near(osin_block)
  osin["cyl"] = _cyl(osin_block)
  osin["axis"] = _axis(osin_block)

  return osin
end

function _phoroptorAddGroup(group, group_name, parent)
  
  if not group or not group_name or not parent then return false end

  local group_node = beginGroup(parent, group_name)
  if group["sph_far"] then
    _addValue(group_node, "sph_far", group["sph_far"])
--  else
--    addValue(group_node, "sph_far", "0")
  end
  if group["sph_near"] then
    _addValue(group_node, "sph_near", group["sph_near"])
--  else
--    addValue(group_node, "sph_near", "0")
  end
  if group["cyl"] then
    _addValue(group_node, "cyl", group["cyl"])
--  else
--    addValue(group_node, "cyl", "0")
  end
  if group["axis"] then
    _addValue(group_node, "axis", group["axis"])
--  else
--    addValue(group_node, "axis", "0")
  end

  endGroup()

  return true
end

function _interpret(data)

  local odxt = _odxt(data)
  local osin = _osin(data)

  if not odxt and not osin then return false end
  
  local root = beginGroup(0, "visutron900plus")

  -- odxt
  if odxt then
    _phoroptorAddGroup(odxt, "odxt", root)
  end

  -- osin
  if osin then
    _phoroptorAddGroup(osin, "osin", root)
  end

  endGroup()

  return root
end

--=============================
-- Helper functions
--=============================
function _loadUtils()
  local lua_base_dir = luaBaseDir()
  local utils = lua_base_dir .. "/" .. "utils.lua"
  dofile(utils)
end

--=============================================================================
--=============================================================================
--=============================================================================
--
-- Test Functions
--
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function test()
  dofile 'utils.lua'
  dofile 'test.lua'

  local real_data = "\002\010VIS\013\010DATA\010VI\010R\010SPH_F: - 3.75\010SPH_N: + 4.50\010CYL  : - 3.50\010AXIS :     45\010PRISM:   5.50.IN\010ACC  : + 0.25\010L\010SPH_F: - 1.50\010SPH_N: - 0.50\010CYL  : - 2.50\010AXIS :    145\010PRISM:   3.50.DOWN\010ACC  : + 0.25\010HSA  :  16\010PD   :     64\010BLUR : + 1.50\010\003"


  -- Testing classifier
  TEST_FALSE( _checkOrigin("WDSSSVIS 900+ASD"), "Testing bad VISUTRON 900+ data.\n")
  TEST_TRUE( _checkOrigin("WDSSSVIS\013\010DATABLABLA VI\013\010 900+ASD"), "Testing good VISUTRON 900+ data.\n")
  testReset()
  _classify("WSDSADWDSDADSAVIS\013\010DATADADAADATTVI\013\010\b\basdasd")
  TEST_EQUAL_INT(1, testAddClassCount_, "Testing that a class has been added on good data.\n")
  TEST_EQUAL_INT(0, testInvalidDataCount_, "Testing that no invalid data has been added on good data.\n")
  testReset()
  TEST_FALSE(_checkOrigin("WADASDGGDSVI\010\013ASDASDASDASDASDVIS\013\010"), "Testing classification on bad data.\n")
  _classify("WADASDGGDSVIASDASDASDASDASDVI")
  TEST_EQUAL_INT(0, testAddClassCount_, "Testing that no class has been added on bad data.\n")
  TEST_EQUAL_INT(1, testInvalidDataCount_, "Testing that invalid Data is reported on bad data.\n")

  testReset()
  _classify(real_data)
  TEST_EQUAL_INT(1, testAddClassCount_, "Testing classifier on real data.\n")
  TEST_EQUAL_INT(0, testInvalidDataCount_, "Testing classifier on real data.\n")

  -- Testing interpreter
  TEST_FALSE(_cyl("ASDASD WAE +111 CYL 000"), "Testing _cyl on bad data.")
  TEST_EQUAL_STR("-22.0", _cyl("aTRGASASRCYL: 22 AAaaCYL: - 22.0 CYL:"), "Testing _cyl on good data - 1.\n")
  TEST_EQUAL_STR("+22.0", _cyl("aTRGASASRCYL: 23 AAaaCYL: + 22.0 CYL:"), "Testing _cyl on good data - 2.\n")

  TEST_FALSE(_axis("AAAAAERRR"), "Testing _axis on bad data.\n")
  TEST_EQUAL_STR("89", _axis("AXIS:    89"), "Testing _axis on good data.\n")

  TEST_FALSE(_sph_near("SPH_F: + 22.0\nSPH_N: 22010\n"), "Testing _sph_near on bad data.\n")
  TEST_EQUAL_STR("-22.00", _sph_near("SPH_F: + 22.0\nSPH_N: - 22.00\n"), "Testing _sph_near on good data - 1.\n")
  TEST_EQUAL_STR("+2.2", _sph_near("SPH_F: + TT 22.0\nSPH_N: +  2.2\n"), "Testing _sph_near on good data - 2.\n")

  TEST_FALSE(_sph_far("SPH_F: + FF 220\nSPH_N: 22.00\n"), "Testing _sph_far on bad data.\n")
  TEST_EQUAL_STR("-22.0", _sph_far("SPH_F: - 22.0\nSPH_N: - 22.00\n"), "Testing _sph_far on good data - 1.\n")
  TEST_EQUAL_STR("+2.2", _sph_far("SPH_F: +  2.2\nSPH_N: +  2.3\n"), "Testing _sph_far on good data - 2.\n")

--  TEST_FALSE(_prism("ASADADDDDPRISM  : 4."), "Testing _prism on bad data.")
--  TEST_NOTEQUAL_STR("4.44", _prism("ASADADDDDPRISM  : 4.4\nACC:4.44"), "Testing _prism on good data - 2.")
--  TEST_EQUAL_STR("4.44", _prism("ASADADDDDPRISM  : 4.44"), "Testing _prism on good data - 1.")

--  TEST_FALSE(_acc("ACC: 44 TI2 4.44\n"), "Testing _acc on bad data.")
--
  testReset()
  local data = "FFJFJFJF\nR\010\nSPH_F: + 22.0\nSPH_N: - 88.0\nCYL: +  2.0\nAXIS:   80\nL\010\n"
  TEST_TRUE(_odxt(real_data), "Testing _odxt on real data.\n")
  local odxt = _odxt(data)
  TEST_TRUE(odxt, "Testing _odxt on good constructed data.\n")
  if odxt then
    TEST_EQUAL_STR("+22.0", odxt["sph_far"], "Testing _odxt sph_far value.\n")
    TEST_EQUAL_STR("-88.0", odxt["sph_near"], "Testing _odxt sph_near value.\n")
    TEST_EQUAL_STR("+2.0", odxt["cyl"], "Testing _odxt cyl value.\n")
    TEST_EQUAL_STR("80", odxt["axis"], "Testing _odxt axis value.\n")
  end
  
  local data = "FFFJFJFJFR\010SPH_F: + 25.0\010SPH_N: - 18.0\nCYL: + 22.0\010AXIS:   81FJFJFJF\nL\010SPH_F: + 22.0\010SPH_N: - 88.0\010CYL: +  2.0\010AXIS:   80"
  TEST_TRUE(_osin(real_data), "Testing _osin on real data.\n")
  local osin = _osin(data)
  TEST_TRUE(osin, "Testing _osin on good constructed data.\n")
  if osin then
    TEST_EQUAL_STR("+22.0", osin["sph_far"], "Testing _osin sph_far value.\n")
    TEST_EQUAL_STR("-88.0", osin["sph_near"], "Testing _osin sph_near value.\n")
    TEST_EQUAL_STR("+2.0", osin["cyl"], "Testing _osin cyl value.\n")
    TEST_EQUAL_STR("80", osin["axis"], "Testing _osin axis value.\n")
  end


  local root = addGroup(0, "phoroptor")
  local odxt = {}
  odxt["sph_far"] = "22.0"
  odxt["sph_near"] = "-3.0"
  odxt["cyl"] = "8.00"
  odxt["axis"] = "79"
  testReset()
  TEST_TRUE(_phoroptorAddGroup(odxt, "odxt", root), "Testing phoroptor creation of groups.\n")
  TEST_EQUAL_INT(4, testAddValueCount_, "Testing addValue counter - 1.\n")
  TEST_EQUAL_INT(1, testAddGroupCount_, "Testing addGroup counter - 1.\n")

  testReset()
  TEST_TRUE(_interpret(real_data), "Testing _interpret on real data.\n")
  TEST_EQUAL_INT(3, testAddGroupCount_, "Testing that addGroup counter.\n")
  TEST_EQUAL_INT(8, testAddValueCount_, "Testing that addValue counter.\n")
end

--interpret(arg[1], "phoroptor")
--classify(arg[1])
--function _interpret(data)
--
--  -- Parse data
--  _, _, odxt, odxt_sph_f_sign, odxt_sph_f, odxt_sph_n_sign, odxt_sph_n, odxt_cyl_sign, odxt_cyl, odxt_axis, odxt_prism, odxt_prism_dir, odxt_acc_sign, odxt_acc = string.find(data, "(R).-"
--                                                                  .. "SPH_F: ([-%+]?).-(%d+%.%d+).-"
--                                                                  .. "SPH_N: ([-%+]?).-(%d+%.%d+).-"
--                                                                  .. "CYL  : ([-%+]?).-(%d+%.%d+).-"
--                                                                  .. "AXIS : .-(%d+).-"
--                                                                  .. "PRISM: .-(%d+%.%d+).-"
--                                                                  .. "ACC  : ([-%+]?).-(%d+%.%d+).-")
--  _, _, osin, osin_sph_f_sign, osin_sph_f, osin_sph_n_sign, osin_sph_n, osin_cyl_sign, osin_cyl, osin_axis, osin_prism, osin_prism_dir, osin_acc_sign, osin_acc = string.find(data, "(L).-"
--                                                                  .. "SPH_F: ([-%+]?).-(%d+%.%d+).-"
--                                                                  .. "SPH_N: ([-%+]?).-(%d+%.%d+).-"
--                                                                  .. "CYL  : ([-%+]?).-(%d+%.%d+).-"
--                                                                  .. "AXIS : .-(%d+).-"
--                                                                  .. "PRISM: .-(%d+%.%d+).-"
--                                                                  .. "ACC  : ([-%+]?).-(%d+%.%d+).-")
--
--
--  printInterpreterMessage("Constructing data tree...\n");
--
---- TODO: What is root supposed to be?
--  root = addGroup(0, "phoroptor")
--
--  -- Build data tree
--  if(odxt) then
--    printInterpreterMessage("odxt\n");
--    odxt = addGroup(root, "odxt")
--      if(odxt_sph_f) then
--        if(odxt_sph_f_sign == "-" and odxt_sph_f ~= "0.00") then 
--          odxt_sph_f = "-" .. odxt_sph_f
--        end
--        addValue(odxt, "sph_f", odxt_sph_f)
--        printInterpreterMessage("\tsph_f added\n");
--      end
--      if(odxt_sph_n) then
--        if(odxt_sph_n_sign == "-" and doxt_sph_n ~= "0.00") then 
--          doxt_sph_n = "-" .. odxt_sph_n
--        end
--        addValue(odxt, "sph_n", odxt_sph_n)
--        printInterpreterMessage("\tsph_n added\n");
--      end
--      if(odxt_cyl) then
--        if(odxt_cyl_sign == "-" and odxt_cyl ~= "0.00") then
--        odxt_cyl = "-" .. odxt_cyl
--        end
--        addValue(odxt, "cyl", odxt_cyl)
--        printInterpreterMessage("\tcyl added\n");
--      end
--      if(odxt_axis) then
--        addValue(odxt, "axis", odxt_axis)
--        printInterpreterMessage("\taxis added\n");
--      end
--      if(odxt_acc) then
--        if(odxt_acc_sign == "-" and odxt_acc ~= "0.00") then
--          odxt_acc = "-" .. odxt_acc
--        end
--        addValue(odxt, "acc", odxt_acc)
--        printInterpreterMessage("\tacc added\n");
--      end
--  end
--  if(osin) then
--    printInterpreterMessage("osin\n");
--    osin = addGroup(root, "osin")
--      if(osin_sph_f) then
--        if(osin_sph_f_sign == "-" and osin_sph_f ~= "0.00") then 
--          osin_sph_f = "-" .. osin_sph_f
--        end
--        addValue(osin, "sph_f", osin_sph_f)
--        printInterpreterMessage("\tsph_f added\n");
--      end
--      if(osin_sph_n) then
--        if(osin_sph_n_sign == "-" and osin_sph_n ~= "0.00") then
--          osin_sph_n = "-" .. osin_sph_n
--        end
--        addValue(osin, "sph_n", osin_sph_n)
--        printInterpreterMessage("\tsph_n added\n");
--      end
--      if(osin_cyl) then
--        if(osin_cyl_sign == "-" and osin_cyl ~= "0.00") then
--          osin_cyl = "-" .. osin_cyl
--        end
--        addValue(osin, "cyl", osin_cyl)
--        printInterpreterMessage("\tcyl added\n");
--      end
--      if(osin_axis) then
--        addValue(osin, "axis", osin_axis)
--        printInterpreterMessage("\taxis added\n");
--      end
----      if(osin_prism) then
----        if(osin_prism_dir) then
----          addValue(osin, "prism_dir", osin_prism_dir)
----        end
----        addValue(osin, "prism", osin_prism)
----        printInterpreterMessage("\tprism added\n");
----      end
--      if(osin_acc) then
--        if(osin_acc_sign == "-" and osin_acc ~= "0.00") then
--          osin_acc = "-" .. osin_acc
--        end
--        addValue(osin, "acc", osin_acc)
--        printInterpreterMessage("\tacc added\n");
--      end
--  end
--  printInterpreterMessage("Returning data...\n")
--  return root
--end
--
