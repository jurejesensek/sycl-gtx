#pragma once

#include "../../data_ref.h"

namespace cl {
namespace sycl {
namespace detail {

#define SYCL_POINT_OP_EQ(lhs, op)				\
	for(size_t i = 0; i < dimensions; ++i) {	\
		lhs values[i] op= rhs.values[i];		\
	}

template <size_t dimensions>
struct point_base {
protected:
	friend class data_ref;

	data_ref::type_t type = data_ref::type_t::general;
	size_t values[dimensions];

	void set(point_base& rhs) {
		SYCL_POINT_OP_EQ(this->, );
	}
};

template <size_t dimensions, bool is_numeric = true>
struct point;

// Numeric points
template <size_t dimensions>
struct point<dimensions, true>
	: public point_base<dimensions>
{
	point& operator+=(const point& rhs) {
		SYCL_POINT_OP_EQ(this->, +);
		return *this;
	}
	template <bool is_numeric>
	point<dimensions, is_numeric> operator+(const point<dimensions, is_numeric>& rhs) const {
		point<dimensions, is_numeric> lhs;
		lhs.set(*this);
		SYCL_POINT_OP_EQ(lhs., +);
		return lhs;
	}
};

// Non-numeric points
template <size_t dimensions>
struct point<dimensions, false>
	: public point_base<dimensions>
{
	point& operator+=(const point<dimensions, true>& rhs) {
		SYCL_POINT_OP_EQ(this->, +);
		return *this;
	}
	point operator+(const point<dimensions, true>& rhs) const {
		point lhs;
		lhs.set(*this);
		SYCL_POINT_OP_EQ(lhs., +);
		return lhs;
	}
	data_ref operator+(const point& rhs) const {
		point lhs;
		lhs.set(*this);
		SYCL_POINT_OP_EQ(lhs., +);
		return data_ref(lhs, type) + rhs;
	}
};

#undef SYCL_POINT_OP_EQ

} // namespace detail
} // namespace sycl
} // namespace cl