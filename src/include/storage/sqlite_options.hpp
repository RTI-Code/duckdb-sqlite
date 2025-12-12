//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/sqlite_options.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/enums/access_mode.hpp"

namespace duckdb {

struct SQLiteOpenOptions {
	// access mode
	AccessMode access_mode = AccessMode::READ_WRITE;
	// busy time-out in ms
	idx_t busy_timeout = 5000;
	// journal mode
	string journal_mode = "WAL";
	// synchronous mode (OFF, NORMAL, FULL, EXTRA)
	string synchronous = "NORMAL";
	// wal autocheckpoint threshold (-1 = don't set, 0 = disable, >0 = page count)
	int wal_autocheckpoint = 1000;
};

} // namespace duckdb
