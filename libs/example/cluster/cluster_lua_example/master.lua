--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local gce = require('gce')

local config = {}

------------------------------------------------------------
------------------- cluster architecture -------------------
------------------------------------------------------------

-- glossary:

-- C: client
--  * mobile, pc, console
--  * c2s, b2s, p2p
--  * stateless, short/long-conn
-- R: router server
--  * forward or proxy, e.g. nginx
--  * guide, tell client which server to conn currently
--  * stateless, short/long-conn
-- B: base server
--  * client mainly interact server, all self and basic function done here
--  * stateless, short/long-conn
-- S: set server
--  * real time interact, e.g. dungeon, chat(im)
--  * group or area, e.g. lobby, stage
--  * stateless (mostly, if not stage), short/long-conn (mostly long-conn)
-- G: grid server
--  * dynamic load balancing, among grids may transfer some data relay on current grid's load
--  * maintain some object data that need convenient find and interact
--  * stateful, long-conn (not directly conn with client)
-- D: data server
--  * cache server, e.g. redis, memcache
--  * database (relational, nosql), e.g. mysql, mongodb
--  * stateful, long-conn (not directly conn with client)

-- X1 or Xn: X server/client with its no.footer
-- stateful, stateless: if client each session or request must conn to one specific server, or if the server must maintain some continuous state (like world events, npc or monster state), then this server is stateful; otherwise, is stateless
-- short-conn, long-conn: short-conn means short connection e.g. http/https, long-conn means long connection e.g. tcp/ssl


-- structure chart (abstract model):
-- 
--                       C1   C2 ... Cz
--        _______________|____|______|_____________
--       |                    |                    |
--   ____|______              |                    |
--  |    |      |             |                    |
--  R1   R2 ... Rk (optional) |                ____|______
--  |____|______|         ____|______         |    |      |
--       |               |    |      |        S1   S2 ... Sn (optional)
--       |               B1   B2 ... Bp       |____|______|
--       |         ______|____|______|             |
--       |        |           |                    |
--       |        |       ____|______              |
--       |        |      |    |      |             |
--       |        |      G1   G2 ... Gm (optional) |
--       |        |      |____|______|             |
--       |        |           |                    |
--       |________|______ ____|______ _____________|
--                       |    |      |
--                       D1   D2 ... Dy


-------------------------------------------------------------
-- a typical short-conn mobile online game structure chart --
-------------------------------------------------------------

--         C1   C2 ... Cz
--         |____|______|
--              |
--          ____|______
--         |    |      |
--         R1   R2 ... Rk
--         |____|______|
--              |
--        ______|_________
--       |                |
--   ____|______      ____|______
--  |    |      |    |    |      |
--  B1   B2 ... Bp   S1   S2 ... Sn (optional)
--  |____|______|    |____|______|
--       |                |
--       |_ ____ ______ __|
--         |    |      |
--         D1   D2 ... Dy

-- * C: mobile phone, e.g. iphone, c2s
-- * R: nginx or some http proxy, generally using dns polling to balance load, short-conn
-- * S: game server, for multi-player real time game (2~4), mostly long-conn, may not be use
-- * B: game server, all personal game logic here, e.g. play some single game, add friend and rank, short-conn
-- * D: cache and db, e.g. redis + mysql, a lot of interact work done by redis, long-conn


gce.actor(
  function ()
    local ec, sender, args, msg

    ec, sender, args = gce.match('init').recv('')
    local ctxid = args[1]

    gce.info('master<', ctxid, '> running...')

    gce.info('master<', ctxid, '> end.')
  end)
