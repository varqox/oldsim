#include "../main.hh"
#include "delete_problem.hh"

#include <sim/jobs/job.hh>
#include <simlib/time.hh>

using sim::jobs::Job;

namespace job_server::job_handlers {

void DeleteProblem::run() {
    STACK_UNWINDING_MARK;
    auto transaction = mysql.start_transaction();

    // Check whether the problem is not used as a contest problem
    {
        auto stmt = mysql.prepare("SELECT 1 FROM contest_problems"
                                  " WHERE problem_id=? LIMIT 1");
        stmt.bind_and_execute(problem_id_);
        if (stmt.next()) {
            return set_failure("There exists a contest problem that uses (attaches) this "
                               "problem. You have to delete all of them to be able to delete "
                               "this problem.");
        }
    }

    // Assure that problem exist and log its Simfile
    {
        auto stmt = mysql.prepare("SELECT simfile FROM problems WHERE id=?");
        stmt.bind_and_execute(problem_id_);
        InplaceBuff<0> simfile;
        stmt.res_bind_all(simfile);
        if (not stmt.next()) {
            return set_failure("Problem does not exist");
        }

        job_log("Deleted problem Simfile:\n", simfile);
    }

    // Add job to delete problem file
    mysql
        .prepare("INSERT INTO jobs(file_id, creator, type, priority, status,"
                 " created_at, aux_id, info, data) "
                 "SELECT file_id, NULL, ?, ?, ?, ?, NULL, '', ''"
                 " FROM problems WHERE id=?")
        .bind_and_execute(
            EnumVal(Job::Type::DELETE_FILE),
            default_priority(Job::Type::DELETE_FILE),
            EnumVal(Job::Status::PENDING),
            mysql_date(),
            problem_id_
        );

    // Add jobs to delete problem submissions' files
    mysql
        .prepare("INSERT INTO jobs(file_id, creator, type, priority, status,"
                 " created_at, aux_id, info, data) "
                 "SELECT file_id, NULL, ?, ?, ?, ?, NULL, '', ''"
                 " FROM submissions WHERE problem_id=?")
        .bind_and_execute(
            EnumVal(Job::Type::DELETE_FILE),
            default_priority(Job::Type::DELETE_FILE),
            EnumVal(Job::Status::PENDING),
            mysql_date(),
            problem_id_
        );

    // Delete problem (all necessary actions will take plate thanks to foreign
    // key constrains)
    mysql.prepare("DELETE FROM problems WHERE id=?").bind_and_execute(problem_id_);

    job_done();

    transaction.commit();
}

} // namespace job_server::job_handlers
