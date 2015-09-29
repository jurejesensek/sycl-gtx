#include "kernel.h"
#include "program.h"
#include "queue.h"

using namespace cl::sycl;

kernel::kernel(bool)
	:	prog(ctx) {}

kernel::kernel(cl_kernel k)
	:	kern(k),
		ctx(get_info<info::kernel::context>()),
		prog(ctx, get_info<info::kernel::program>())
{}

void kernel::enqueue_task(queue* q) const {
	auto error_code = clEnqueueTask(
		q->get(), kern.get(),
		// TODO: Events
		0, nullptr, nullptr
	);
	detail::error::report(error_code);
}

void kernel::set(cl_kernel openclKernelObject) {
	kern = openclKernelObject;
	clReleaseKernel(openclKernelObject);
}

void kernel::set(const context& context, cl_program validProgram) {
	ctx = context;
	prog = program(context, validProgram);
}
