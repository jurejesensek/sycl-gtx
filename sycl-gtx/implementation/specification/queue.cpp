#include "queue.h"

using namespace cl::sycl;

error_handler& queue::default_error = helper::error::handler::default;

device queue::select_best_device(device_selector& selector, context& ctx) {
	auto device_pointers = ctx.get_info<CL_CONTEXT_DEVICES>();
	auto devices = helper::transform_vector<device>(device_pointers);
	auto index = helper::select_best_device(selector, devices);
	return devices[index];
}

// Master constructor
queue::queue(context ctx, device dev, cl_command_queue_properties properties, error_handler& sync_handler, bool host_fallback) {}

queue::queue(cl_command_queue cmd_queue, error_handler& sync_handler)
	:	command_q(refc::allocate<cl_command_queue>(cmd_queue, clReleaseCommandQueue)),
		dev(get_info<CL_QUEUE_DEVICE>(), sync_handler),
		ctx(get_info<CL_QUEUE_CONTEXT>(), sync_handler),
		handler(sync_handler) {
	auto error_code = clRetainCommandQueue(cmd_queue);
	handler.report(this, error_code);
}

queue::queue(context ctx, device dev, cl_command_queue_properties properties, error_handler& sync_handler)
	: queue(ctx, dev, properties, sync_handler, false) {}

// TODO
queue::queue(device_selector& selector, cl_command_queue_properties properties, error_handler& sync_handler) {}
//	: queue(nullptr, select_best_device(selector, ctx), properties, sync_handler, false) {}

queue::queue(context ctx, device_selector& selector, cl_command_queue_properties properties, error_handler& sync_handler)
	: queue(ctx, select_best_device(selector, ctx), properties, sync_handler, true) {}
queue::queue(device dev, cl_command_queue_properties properties, error_handler& sync_handler)
	: queue(context(nullptr, dev, sync_handler), dev, properties, sync_handler, false) {}

queue::~queue() {
	throw_asynchronous();

	// TODO
}

cl_command_queue queue::get() {
	return command_q.get();
}

context queue::get_context() {
	return ctx;
}

device queue::get_device() {
	return dev;
}

// TODO: Which error?
cl_int queue::get_error() {
	return 0;
}

void queue::disable_exceptions() {
	exceptions_enabled = false;
}

// TODO: This function checks to see if any asynchronous errors have been thrown in the queue
// and if so reports them via exceptions, or via the supplied async_handler
void queue::throw_asynchronous() {
	handler.apply();
}

// TODO: This a blocking wait for all enqueued tasks in the queue to complete.
void queue::wait() {
}

void queue::wait_and_throw() {
	wait();
	throw_asynchronous();
}