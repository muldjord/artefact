/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luatcpsocket.cc
 *
 *  Fri Mar  5 11:19:44 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Pentominos.
 *
 *  Pentominos is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Pentominos is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Pentominos; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "luatcpsocket.h"

#include <tcpsocket.h>

#include <hugin.hpp>

#include <vector>

#include <luautil.h>

#ifndef TEST_TEST_LUA
#include <sysconfig.h>
#endif

static std::vector<Pentominos::TCPSocket * > sockets;

static int tcp_connect(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_NUMBER,
                              Pentominos::T_END);

  std::string host = lua_tostring(L, lua_gettop(L) - 1);
  unsigned short int port = lua_tointeger(L, lua_gettop(L));

  DEBUG(luatcpsocket, "host [%s]\n", host.c_str());
  DEBUG(luatcpsocket, "port [%d]\n", port);

  Pentominos::TCPSocket *socket = new Pentominos::TCPSocket();
  socket->connect(host, port);

  sockets.push_back(socket);

  unsigned int idx = sockets.size() - 1;
  lua_pushinteger(L, idx);

  DEBUG(luatcpsocket, "TCP connect (%d)\n", idx);

  return 1;
}

static int tcp_close(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_END);
  
  unsigned int idx = lua_tointeger(L, lua_gettop(L));

  DEBUG(luatcpsocket, "TCP close (%d)\n", idx);

  delete sockets[idx];
  sockets[idx] = NULL;

  return 0;
}

static int tcp_read(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_END);

  unsigned int idx = lua_tointeger(L, lua_gettop(L));

  DEBUG(luatcpsocket, "TCP read (%d)\n", idx);

  Pentominos::TCPSocket *socket = sockets[idx];

  char buf[10000];
  socket->read(buf, sizeof(buf));

  lua_pushstring(L, buf);

  return 1;
}

static int tcp_write(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_STRING,
                              Pentominos::T_END);

  unsigned int idx = lua_tointeger(L, lua_gettop(L) - 1);
  std::string data = lua_tostring(L, lua_gettop(L));

  DEBUG(luatcpsocket, "TCP write (%d)\n", idx);

  Pentominos::TCPSocket *socket = sockets[idx];

  socket->write((char*)data.data(), data.length());

  return 0;
}

void register_tcpsocket(lua_State *L)
{
  lua_register(L, "tcp_connect", tcp_connect);
  lua_register(L, "tcp_close", tcp_close);
  lua_register(L, "tcp_read", tcp_read);
  lua_register(L, "tcp_write", tcp_write);
}

#ifdef TEST_LUATCPSOCKET
//deps: ../lib/tcpsocket.cc ../lib/debug.cc ../lib/tostring.cc ../lib/exception.cc ../lib/log.cc ../lib/luautil.cc
//cflags: -I../lib -I.. $(LUA_CFLAGS)
//libs: $(LUA_LIBS)
#include "test.h"

#define LUAPROG                                   \
""
/*
  "sock = tcp_connect('87.63.201.210', 80)    \n" \
  "tcp_write(sock, 'GET /index.html')         \n" \
  "print('GET')                               \n" \
  "data = tcp_read(sock)                      \n" \
  "print(data)                                \n" \
  "tcp_close(sock)                            \n"
*/

#include <string>

TEST_BEGIN;

debug_parse("+all");

//Pentominos::TCPSocket sock;
//sock.listen(10000);

lua_State *L = luaL_newstate();
luaL_openlibs(L);

register_tcpsocket(L);

if(luaL_loadstring(L, LUAPROG)) {
  std::string msg = lua_tostring(L, lua_gettop(L));
  printf("loadstring\n%s\n", msg.c_str());
}

if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
  std::string msg = lua_tostring(L, lua_gettop(L));
  printf("pcall\n%s\n", msg.c_str());
}

lua_close(L);

TEST_END;

#endif/*TEST_LUATCPSOCKET*/
