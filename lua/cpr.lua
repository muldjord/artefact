--
-- Tabeller
--
-- cprnr
-- efternavn
-- fornavn
-- conavn
-- lokalitet
-- adresse
-- bynavn
-- postnr
-- postby
-- kommunenr
-- sikringsgruppe
-- laegenr
-- beskyttelse
-- reserveret
-- aktion
-- doedsdato
-- insert_at
-- moved_at

pg = pg_connect("server.dns.addr", "5432", "cprread", "", "cpr");
if( pg )
then
   res = pg_query(pg, "SELECT * FROM cprtab WHERE cprnr='" .. getCPR() .. "';");
   pg_close(pg);
end

cpr = getCPR(); --res[0];

if( res and res[1] and res[2] and res[5] and res[6] and res[8] )
then
   surname = res[1];
   givenname = res[2];
   address = res[5];
   postnr = res[7];
   city = res[8];
end

root = addGroup(0, 'cpr')

addValue(root, 'cpr', UTF8Encode(cpr))

if( cpr and givenname and surname and address and postnr and city )
then
--  addValue(root, 'name', UTF8Encode(givenname .. ' ' .. surname))
--  addValue(root, 'address', UTF8Encode(address))
--  addValue(root, 'city', UTF8Encode(postnr .. ' ' .. city))
  addValue(root, 'name', givenname .. ' ' .. surname)
  addValue(root, 'address', address)
  addValue(root, 'city', postnr .. ' ' .. city)
end
