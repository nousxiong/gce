///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "client.hpp"
#include <gce/actor/all.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 1)
    {
      std::cerr << "Usage: <gate_ep>" << std::endl;
      return 0;
    }

    client cln(argv[1]);
    cln.run();
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
