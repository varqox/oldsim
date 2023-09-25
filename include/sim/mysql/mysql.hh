#pragma once

#include <simlib/file_path.hh>
#include <simlib/mysql/mysql.hh>

namespace sim::mysql {

template <class T>
using Optional = ::mysql::Optional<T>;

using Result = ::mysql::Result;
using Statement = ::mysql::Statement;
using Transaction = ::mysql::Transaction;
using Connection = ::mysql::Connection;

/**
 * @brief Creates Connection using file @p filename
 * @details File format as ConfigFile with variables:
 *   - user: user to log in as
 *   - password: user's password
 *   - db: database to use
 *   - host: MySQL's host
 *
 * @param filename file to load credentials from
 *
 * @return Connection object
 *
 * @errors If any error occurs std::runtime_error is thrown
 */
Connection make_conn_with_credential_file(FilePath filename);

} // namespace sim::mysql
