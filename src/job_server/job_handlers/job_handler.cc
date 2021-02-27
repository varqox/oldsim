#include "src/job_server/job_handlers/job_handler.hh"
#include "sim/constants.hh"
#include "src/job_server/main.hh"

using sim::JobStatus;

namespace job_server::job_handlers {

void JobHandler::job_canceled() {
    STACK_UNWINDING_MARK;

    mysql.prepare("UPDATE jobs SET status=?, data=? WHERE id=?")
        .bind_and_execute(EnumVal(JobStatus::CANCELED), get_log(), job_id_);
}

void JobHandler::job_done() {
    STACK_UNWINDING_MARK;

    mysql.prepare("UPDATE jobs SET status=?, data=? WHERE id=?")
        .bind_and_execute(EnumVal(JobStatus::DONE), get_log(), job_id_);
}

void JobHandler::job_done(StringView new_info) {
    STACK_UNWINDING_MARK;

    mysql.prepare("UPDATE jobs SET status=?, info=?, data=? WHERE id=?")
        .bind_and_execute(EnumVal(JobStatus::DONE), new_info, get_log(), job_id_);
}

} // namespace job_server::job_handlers
