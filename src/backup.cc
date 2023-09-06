#include <chrono>
#include <sim/jobs/job.hh>
#include <sim/mysql/mysql.hh>
#include <simlib/concat_tostr.hh>
#include <simlib/file_info.hh>
#include <simlib/file_manip.hh>
#include <simlib/path.hh>
#include <simlib/process.hh>
#include <simlib/sim/problem_package.hh>
#include <simlib/spawner.hh>
#include <simlib/time.hh>
#include <simlib/working_directory.hh>

using sim::jobs::Job;
using std::string;
using std::vector;

/// @brief Displays help
static void help(const char* program_name) {
    if (program_name == nullptr) {
        program_name = "backup";
    }

    printf("Usage: %s [options]\n", program_name);
    puts("Make a backup of solutions and database contents");
}

int main2(int argc, char** argv) {
    if (argc != 1) {
        help(argc > 0 ? argv[0] : nullptr);
        return 1;
    }

    chdir_relative_to_executable_dirpath("..");

#define MYSQL_CNF ".mysql.cnf"
    FileRemover mysql_cnf_guard;

    // Get connection
    auto conn = sim::mysql::make_conn_with_credential_file(".db.config");

    FileDescriptor fd{MYSQL_CNF, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, S_0600};
    if (fd == -1) {
        errlog("Failed to open file `" MYSQL_CNF "`: open()", errmsg());
        return 1;
    }

    mysql_cnf_guard.reset(MYSQL_CNF);
    write_all_throw(
        fd,
        from_unsafe{concat(
            "[client]\nuser=\"", conn.impl()->user, "\"\npassword=\"", conn.impl()->passwd, "\"\n"
        )}
    );

    auto run_command = [](vector<string> args) {
        auto es = Spawner::run(args[0], args);
        if (es.si.code != CLD_EXITED or es.si.status != 0) {
            errlog(args[0], " failed: ", es.message);
            exit(1);
        }
    };

    // Remove temporary internal files that were not removed (e.g. a problem
    // adding job was canceled between the first and second stage while it was
    // pending)
    {
        using std::chrono::system_clock;
        using namespace std::chrono_literals;

        auto transaction = conn.start_transaction();
        auto stmt = conn.prepare("SELECT tmp_file_id FROM jobs "
                                 "WHERE tmp_file_id IS NOT NULL AND status IN (?,?,?)");
        stmt.bind_and_execute(
            EnumVal(Job::Status::DONE), EnumVal(Job::Status::FAILED), EnumVal(Job::Status::CANCELED)
        );
        uint64_t tmp_file_id = 0;
        stmt.res_bind_all(tmp_file_id);

        auto deleter = conn.prepare("DELETE FROM internal_files WHERE id=?");
        // Remove jobs temporary internal files
        while (stmt.next()) {
            auto file_path = sim::internal_files::path_of(tmp_file_id);
            if (access(file_path, F_OK) == 0 and
                system_clock::now() - get_modification_time(file_path) > 2h)
            {
                deleter.bind_and_execute(tmp_file_id);
                (void)unlink(file_path);
            }
        }

        // Remove internal files that do not have an entry in internal_files
        sim::PackageContents fc;
        fc.load_from_directory(sim::internal_files::dir);
        std::set<std::string, std::less<>> orphaned_files;
        fc.for_each_with_prefix("", [&](StringView file) {
            orphaned_files.emplace(file.to_string());
        });

        stmt = conn.prepare("SELECT id FROM internal_files");
        stmt.bind_and_execute();
        InplaceBuff<32> file_id;
        stmt.res_bind_all(file_id);
        while (stmt.next()) {
            orphaned_files.erase(orphaned_files.find(file_id));
        }

        // Remove orphaned files that are older than 2h (not to delete files
        // that are just created but not committed)
        for (const std::string& file : orphaned_files) {
            struct stat64 st = {};
            auto file_path = concat(sim::internal_files::dir, file);
            if (stat64(file_path.to_cstr().data(), &st)) {
                if (errno == ENOENT) {
                    break;
                }

                THROW("stat64", errmsg());
            }

            if (system_clock::now() - get_modification_time(st) > 2h) {
                stdlog("Deleting: ", file);
                (void)unlink(file_path);
            }
        }

        transaction.commit();
    }

    run_command({
        "mysqldump",
        concat_tostr("--defaults-file=", MYSQL_CNF),
        "--result-file=dump.sql",
        "--single-transaction",
        conn.impl()->db,
    });

    if (chmod("dump.sql", S_0600)) {
        THROW("chmod()", errmsg());
    }

    run_command({"git", "init", "--initial-branch", "main"});
    run_command({"git", "config", "user.name", "bin/backup"});
    run_command({"git", "config", "user.email", ""});
    // Optimize git for worktree with many files
    run_command({"git", "config", "feature.manyFiles", "true"});
    // Tell git not to delta compress too big files
    run_command({"git", "config", "core.bigFileThreshold", "16m"});
    // Tell git not to make internal files too large
    run_command({"git", "config", "pack.packSizeLimit", "1g"});
    // Tell git not to repack big packs
    run_command({"git", "config", "gc.bigPackThreshold", "500m"});
    // Tell git not to repack large number of big packs
    run_command({"git", "config", "gc.autoPackLimit", "0"});
    // Makes fetching from this repository faster
    run_command({"git", "config", "pack.window", "0"});
    // Prevent git from too excessive memory usage during git fetch (on remote) and git gc
    run_command({"git", "config", "pack.deltaCacheSize", "128m"});
    run_command({"git", "config", "pack.windowMemory", "128m"});
    run_command({"git", "config", "pack.threads", "1"});
    // Run automatic git gc more fequently
    run_command({"git", "config", "gc.auto", "500"});

    run_command({"git", "add", "--verbose", "dump.sql"});
    run_command({"git", "add", "--verbose", "bin/", "manage", "proot"});
    run_command({"git", "add", "--verbose", "internal_files/"});
    run_command({"git", "add", "--verbose", "logs/"});
    run_command({"git", "add", "--verbose", "sim.conf", ".db.config"});
    run_command({"git", "add", "--verbose", "static/"});
    run_command(
        {"git",
         "commit",
         "-q",
         "-m",
         concat_tostr("Backup ", mysql_localdate(), " (", mysql_date(), " UTC)")}
    );
    run_command({"git", "--no-pager", "show", "--stat"});

    return 0;
}

int main(int argc, char** argv) {
    try {
        return main2(argc, argv);

    } catch (const std::exception& e) {
        ERRLOG_CATCH(e);
        return 1;
    }
}
