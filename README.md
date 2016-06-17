GCE v1.1
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

* CMake 2.8 and newer
* Boost 1.55.0 and newer

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
* cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=your_boost_root_dir -DSUB_LIBRARYS="actor amsg" ../gce
* make
* *Optional:* make install (if set CMAKE_INSTALL_PREFIX when run cmake, for example: -DCMAKE_INSTALL_PREFIX=../install)

Build (Windows)
-----------

* (on cmd console, cd to your gce sources root dir)
* cd ..
* mkdir gce_build
* cd gce_build
* cmake -G "Visual Studio 9 2008" -DBOOST_ROOT=your_boost_root_dir -DSUB_LIBRARYS="actor amsg" ..\gce
* (open generated vc sln, and build ALL_BUILD project)
* *Optional:* (build INSTALL project)(if set CMAKE_INSTALL_PREFIX when run cmake, for example: -DCMAKE_INSTALL_PREFIX=..\install)

Hello world
-----------

```cpp
#include <gce/actor/all.hpp>
#include <iostream>
#include <string>

void mirror(gce::actor<gce::stackful>& self)
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
  /// everything begin here
  gce::context ctx;

  /// create a hello_world actor, using thread-base actor
  gce::actor<gce::threaded> hello_world = gce::spawn(ctx);

  /// create a new actor that calls ’mirror(gce::self_t)’, using coroutine-base actor
  gce::aid_t mirror_actor = gce::spawn(hello_world, boost::bind(&mirror, _1));

  /// send "Hello World!" to mirror
  gce::message m;
  m << std::string("Hello World!");
  gce::response_t res = hello_world.request(mirror_actor, m);

  /// ... and wait for a response
  gce::message msg;
  hello_world.recv(res, msg);
  std::string reply_str;
  msg >> reply_str;

  /// prints "!dlroW olleH"
  std::cout << reply_str << std::endl;

  return 0;
}
```

Type matching
-----------

```cpp
#include <gce/actor/all.hpp>
#include <iostream>

void echo(gce::actor<gce::stackful>& self)
{
  /// wait for "start" message. 
  /// if and only if after fetch "start", then others
  gce::recv(self, gce::atom("start"));
  std::cout << "start!" << std::endl;

  /// handle other messages
  gce::message msg;
  gce::aid_t sender = self.recv(msg);
  std::cout << "recv message: " << gce::atom(msg.get_type()) << std::endl;

  /// reply
  gce::send(self, sender);
}

int main()
{
  gce::context ctx;

  gce::actor<gce::threaded> base = gce::spawn(ctx);

  gce::aid_t echo_actor = gce::spawn(base, boost::bind(&echo, _1));

  /// send "hi" message to echo
  gce::send(base, echo_actor, gce::atom("hi"));

  /// send "start" message to echo, after "hi" message
  gce::send(base, echo_actor, gce::atom("start"));

  /// ... and wait for a response
  gce::recv(base);

  return 0;
}
```

Actor link
-----------

```cpp
#include <gce/actor/all.hpp>
#include <iostream>

void quiter(gce::actor<gce::stackful>& self)
{
  /// wait for gce::exit from link actor
  gce::recv(self);
}

void link(gce::actor<gce::stackful>& self)
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
  gce::context ctx;

  gce::actor<gce::threaded> base = gce::spawn(ctx);

  /// create a link actor and monitor it.
  gce::spawn(base, boost::bind(&link, _1), gce::monitored);

  /// wait for gce::exit message
  gce::recv(base);

  std::cout << "end" << std::endl;

  return 0;
}
```

### Amazing  open source projects:

**NoahGameFrame**
* Auther: ketoo
* GitHub: https://github.com/ketoo/NoahGameFrame
* Description: NF is a lightweight, fast, scalable, distributed plugin framework.Greatly inspired by OGRE and Bigworld.

**breeze**
* Auther: zsummer
* Github: https://github.com/zsummer/breeze
* Description:A fast, scalable, distributed game server framework for C++

Changelog
---------------
v1.1 
* gce::mixin_t change to gce::actor<gce::threaded>
* gce::self_t change to gce::actor<gce::stackful>
* gce::slice_t change to gce::actor<gce::nonblocked>
* add new type actor: gce::actor<gce::stackless>
