#pragma once
#include "BlockedOperationBase.h"
#include <experimental/coroutine>

template <typename TR, typename... TArgs>
struct std::experimental::coroutine_traits <std::unique_ptr<AO::TypedTask<TR>>, TArgs... >
{
	using promise_type = AO::TaskPromiseType<TR>;
};
