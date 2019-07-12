#pragma once

#include "job_handler.h"

namespace job_handlers {

class DeleteContest final : public JobHandler {
	uint64_t contest_id;

public:
	DeleteContest(uint64_t job_id, uint64_t contest_id)
	   : JobHandler(job_id), contest_id(contest_id) {}

	void run() override final;
};

} // namespace job_handlers
