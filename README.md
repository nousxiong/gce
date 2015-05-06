GCE v1.2
=======

GCE is an actor model implementation featuring lightweight & fast
actor implementations, type matching for messages,
network transparent messaging, and more.

Features Overview
---------------

* Lightweight, fast and efficient actor implementations
* Network transparent messaging
* Error handling based on Erlang’s failure model
* Type matching for messages
* Copy-on-write messages
* Thread-base and on-the-fly coroutine-base actors, "Write async code like sync"
* Seamless cooperate with Boost.Asio
* Lightweight cluster support
* Lua support, including lua5.1,5.2,5.3 and luajit2.0

Manual
---------------
manual.pdf

Get the Sources
---------------

* git clone git://github.com/nousxiong/gce.git

or

* svn checkout https://github.com/nousxiong/gce/trunk

Dependencies
------------

* CMake 3.0 and newer
* Boost 1.57.0 and newer

Need build sub librares:

* Boost.Atomic
* Boost.Coroutine
* Boost.Context
* Boost.System
* Boost.Regex
* Boost.DateTime
* Boost.Timer
* Boost.Chrono
* Boost.Thread

Please build boost with stage mode (for example: b2 ... stage)

Optional Dependencies
------------

* Lua (5.1, 5.2, 5.3 or luajit2.0)
* Openssl (>=1.0)

Supported Compilers
-------------------

* GCC >= 4.6
* VC >= 9.0 (sp1)

Build (Linux)
-----------

* (cd to your gce sources root dir)
* cd ..
* mkdir gce_build
* cd gce_build
* cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=your_boost_root_dir -DSUB_LIBRARYS="actor amsg adata asio log assert" ../gce
* make
* *Optional:* make install (if set CMAKE_INSTALL_PREFIX when run cmake, for example: -DCMAKE_INSTALL_PREFIX=../install)

Build (Windows)
-----------

* (on cmd console, cd to your gce sources root dir)
* cd ..
* mkdir gce_build
* cd gce_build
* cmake -G "Visual Studio 12 2013" -DBOOST_ROOT=your_boost_root_dir -DSUB_LIBRARYS="actor amsg adata asio log assert" ..\gce
* (open generated vc sln, and build ALL_BUILD project)
* *Optional:* (build INSTALL project)(if set CMAKE_INSTALL_PREFIX when run cmake, for example: -DCMAKE_INSTALL_PREFIX=..\install)

CMake Options
-----------

The main build options you will want to configure are as follows:

* SUB_LIBRARYS: GCE's sublibraries. default: "actor amsg adata asio log assert"
* GCE_LUA: Enable or disable lua support. default: OFF
* GCE_LUA_VERSION: Lua version string, options: "5.1" "5.2" "5.3". default: "5.1"
* GCE_PACKER: GCE serilization's way, options: "amsg" "adata". default: "adata"
* GCE_OPENSSL: Enable or disable openssl support. default: OFF
* LUA_INCLUDEDIR: Tell gce where to find lua's include headers. default: ""
* LUA_LIBRARYDIR: Tell gce where to find lua's lib. default: ""
* OPENSSL_INCLUDEDIR: Tell gce where to find openssl's include headers. default: ""
* OPENSSL_LIBRARYDIR: Tell gce where to find openssl's lib. default: ""
* GCE_[xxx]_BUILD_EXAMPLE: Enable or disable build xxx's example. default: OFF
* GCE_[xxx]_BUILD_TEST: Enable or disable build xxx's test. default: OFF

There is a example for using luajit2.0 and openssl under windows:

cmake -G "Visual Studio 12 2013" -DCMAKE_INSTALL_PREFIX=..\install -DBOOST_ROOT=h:\data\src\boost_1_57_0 -DSUB_LIBRARYS="actor amsg adata asio log assert lualib" -DGCE_LUA=ON -DGCE_LUA_VERSION="5.1" -DGCE_PACKER="adata" -DGCE_OPENSSL=ON -DOPENSSL_INCLUDEDIR=h:\data\src\openssl\include -DOPENSSL_LIBRARYDIR=h:\data\src\openssl\lib -DLUA_INCLUDEDIR=h:\data\src\lua\jit\include -DLUA_LIBRARYDIR=h:\data\src\lua\jit ..\gce

Hello world
-----------

```cpp
#include <gce/actor/all.hpp>
#include <iostream>
#include <string>

void mirror(gce::stackful_actor self)
{
  /// wait for messages
  gce::message msg;
  gce::aid_t sender = self.recv(msg);
  std::string what;
  msg >> what;

  /// prints "Hello World!"
  std::cout << what << std::endl;

  /// replies "!dlroW olleH"
  gce::message m;
  m << std::string(what.rbegin(), what.rend());
  self.reply(sender, m);
}

int main()
{
  try
  {
    /// everything begin here
    gce::context ctx;

    /// create a hello_world actor, using thread-base actor
    gce::threaded_actor hello_world = gce::spawn(ctx);

    /// create a new actor that calls ’mirror(gce::self_t)’, using coroutine-base actor
    gce::aid_t mirror_actor = gce::spawn(hello_world, boost::bind(&mirror, _1));

    /// send "Hello World!" to mirror
    gce::message m;
    m << std::string("Hello World!");
    gce::response_t res = hello_world.request(mirror_actor, m);

    /// ... and wait for a response
    gce::message msg;
    hello_world.respond(res, msg);
    std::string reply_str;
    msg >> reply_str;

    /// prints "!dlroW olleH"
    std::cout << reply_str << std::endl;
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
```

Type matching
-----------

```cpp
#include <gce/actor/all.hpp>
#include <iostream>

void echo(gce::stackful_actor self)
{
  /// wait for "start" message. 
  /// if and only if after fetch "start", then others
  self->recv("start");
  std::cout << "start!" << std::endl;

  /// handle other messages
  gce::message msg;
  gce::aid_t sender = self.recv(msg);
  std::cout << "recv message: " << gce::atom(msg.get_type()) << std::endl;

  /// reply
  self->send(sender);
}

int main()
{
  try
  {
    gce::context ctx;

    gce::threaded_actor base = gce::spawn(ctx);

    gce::aid_t echo_actor = gce::spawn(base, boost::bind(&echo, _1));

    /// send "hi" message to echo
    base->send(echo_actor, "hi");

    /// send "start" message to echo, after "hi" message
    base->send(echo_actor, "start");

    /// ... and wait for a response
    base->recv();
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
```

Actor link
-----------

```cpp
#include <gce/actor/all.hpp>
#include <iostream>

void quiter(gce::stackful_actor self)
{
  /// wait for gce::exit from link actor
  self->recv();
}

void link(gce::stackful_actor self)
{
  /// create 10 actor and link with them
  for (std::size_t i=0; i<10; ++i)
  {
    gce::spawn(self, boost::bind(&quiter, _1), gce::linked);
  }

  /// quit, will send 10 gce::exit to quiter actors 
  /// and 1 gce::exit to base actor(in main)
}

int main()
{
  try
  {
    gce::context ctx;

    gce::threaded_actor base = gce::spawn(ctx);

    /// create a link actor and monitor it.
    gce::spawn(base, boost::bind(&link, _1), gce::monitored);

    /// wait for gce::exit message
    base->recv();

    std::cout << "end" << std::endl;
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
```

Changelog
---------------
v1.2
* GCE.Actor change to header only
* Add addon system to cooperate with any async code
* Add GCE.Asio that using addon system to thin wrap Boost.Asio
* add lua, now user can use lua to write gce code
* gce::actor<gce::threaded> change to gce::threaded_actor
* gce::actor<gce::stackful> change to gce::stackful_actor
* gce::actor<gce::nonblocked> change to gce::nonblocked_actor
* gce::actor<gce::stackless> change to gce::stackless_actor
* global send/recv removed, move to gce::actor<T>, using operator ->
* none request message can relay now
* atom can optionally be omited, user can use enum, string(limit 13 bytes) or old-atom-style
* api recv (recv response) change to respond
* gce::connect and gce::bind remove "is_router_", move to net_option
* message can now serialize Boost.Shared_ptr, limit local process
* message max limit removed

v1.1 
* gce::mixin_t change to gce::actor<gce::threaded>
* gce::self_t change to gce::actor<gce::stackful>
* gce::slice_t change to gce::actor<gce::nonblocked>
* add new type actor: gce::actor<gce::stackless>
