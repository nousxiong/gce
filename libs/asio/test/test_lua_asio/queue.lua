--
-- Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
--
-- Distributed under the Boost Software License, Version 1.0. (See accompanying
-- file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
--
-- See https://github.com/nousxiong/gce for latest version.
--

local queue = {}

function queue.new()
  return {first=0, last=-1}
end

function queue.push(que, value)
  que.last = que.last + 1
  que[que.last] = value
end

function queue.pop(que)
  local first = que.first
  if first > que.last then
    error("queue is empty!")
  end

  local value = que[first]
  que[first] = nil
  que.first = first + 1
  return value
end

function queue.empty(que)
  return que.first > que.last
end

return queue
