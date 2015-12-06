#include "command_group.h"

#include "accessor.h"
#include "buffer.h"
#include "queue.h"
#include <map>
#include <unordered_set>

using namespace cl::sycl;
using namespace detail;

void command_group::enter() {
	detail::command::group_::last = this;
}
void command_group::exit() {
	detail::command::group_::last = nullptr;
}

// TODO: Reschedules commands to achieve better performance
void command_group::optimize() {
	DSELF();

	auto size_to_keep = commands.size();
	std::map<command_t*, bool> keep;
	//keep.reserve(size_to_keep);
	std::map<detail::buffer_base*, command_t*> last_read;
	std::unordered_set<detail::buffer_base*> was_written;

	using detail::command::type_t;

	for(auto& command : commands) {
		keep[&command] = true;

		if(command.type == type_t::get_accessor) {
			// User accesses the buffer

			auto ptr = command.data.buf_acc.data;

			{ // Reset reads
				auto it = last_read.find(ptr);
				if(it != last_read.end()) {
					last_read.erase(it);
				}
			}

			{ // Reset writes
				auto it = was_written.find(ptr);
				if(it != was_written.end()) {
					was_written.erase(it);
				}
			}
		}
		else if(command.type == type_t::copy_data) {
			auto ptr = command.data.buf_copy.buf.data;

			if(command.data.buf_copy.mode == access::read) {
				auto it = last_read.find(ptr);

				// Keep only the last read
				if(it != last_read.end()) {
					keep[it->second] = false;
					--size_to_keep;
				}

				last_read[ptr] = &command;
			}
			else if(command.data.buf_copy.mode == access::write) {
				auto it = was_written.find(ptr);

				// Keep only the first write
				if(it == was_written.end()) {
					was_written.insert(ptr);
				}
				else {
					keep[&command] = false;
					--size_to_keep;
				}
			}
		}
	}

	decltype(commands) saveResults;
	saveResults.reserve(saveResults.size() + size_to_keep);

	for(auto& command : commands) {
		if(keep[&command]) {
			saveResults.push_back(std::move(command));
		}
	}
	commands = std::move(saveResults);
}

// Executes all commands in queue and removes them
void command_group::flush(vector_class<cl_event> wait_events) {
	DSELF() << q << q->get();

	using detail::command::type_t;

	for(auto& command : commands) {
		if(command.type == type_t::get_accessor) {
			auto& acc = command.data.buf_acc;
			auto d = debug();
			d << command.type << acc.data << acc.mode << acc.target;
		}
		else if(command.type == type_t::copy_data) {
			auto& copy = command.data.buf_copy;
			auto d = debug();
			d << command.type << copy.buf.data << copy.buf.mode << copy.buf.target << copy.mode;
		}
		else {
			debug() << "command:" << command.name;
		}
		command.function(q, wait_events);
	}
	commands.clear();

	auto error = clFlush(q->get());
	detail::error::report(error);
}


using namespace detail;

command_group* command::group_::last = nullptr;

bool command::group_::in_scope() {
	return last != nullptr;
}

void command::group_::check_scope() {
	if(!in_scope()) {
		detail::error::report(error::code::NOT_IN_COMMAND_GROUP_SCOPE);
	}
}
