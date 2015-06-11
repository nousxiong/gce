///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "application.hpp"

int main(int argc, char* argv[])
{
  gce::log::asio_logger lgr;
  gce::log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");
  try
  {
    if (argc != 3)
    {
      GCE_INFO(lg) << "usage: exe [ctxid] [script].lua";
      return 1;
    }

    usr::application app(argv[1], lg);
    app.run(argv[2]);
  }
  catch (std::exception& ex)
  {
    GCE_ERROR(lg) << ex.what();
  }
  return 0;
}
