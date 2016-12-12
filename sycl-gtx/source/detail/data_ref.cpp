#include "SYCL/detail/data_ref.h"
#include "SYCL/ranges/id.h"
#include "SYCL/detail/src_handlers/kernel_source.h"

using namespace cl::sycl;
using namespace detail;

void detail::kernel_add(string_class line) {
  kernel_::source::add(line);
}

const string_class data_ref::open_parenthesis = "(";