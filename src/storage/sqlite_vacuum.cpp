#include "duckdb.hpp"

#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "sqlite_scanner.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/attached_database.hpp"
#include "storage/sqlite_catalog.hpp"
#include "storage/sqlite_transaction.hpp"
#include "sqlite_db.hpp"

namespace duckdb {

struct VacuumFunctionData : public TableFunctionData {
	VacuumFunctionData() {
	}

	bool finished = false;
	string db_name = "";
	string file_path = "";
};

static unique_ptr<FunctionData> VacuumBind(ClientContext &context, TableFunctionBindInput &input,
                                           vector<LogicalType> &return_types, vector<string> &names) {

	auto result = make_uniq<VacuumFunctionData>();

	// Validate input
	if (input.inputs[0].IsNull()) {
		throw BinderException("Database name cannot be NULL");
	}

	result->db_name = input.inputs[0].GetValue<string>();

	// Look up the database
	auto &db_manager = DatabaseManager::Get(context);
	auto db = db_manager.GetDatabase(context, result->db_name);
	if (!db) {
		throw BinderException("Database \"%s\" not found", result->db_name);
	}

	// Verify it's a SQLite database
	auto &catalog = db->GetCatalog();
	if (catalog.GetCatalogType() != "sqlite") {
		throw BinderException("Database \"%s\" is not a SQLite database", result->db_name);
	}

	// Get the file path from the catalog
	auto &sqlite_catalog = catalog.Cast<SQLiteCatalog>();
	result->file_path = sqlite_catalog.GetDBPath();

	// Set return type
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");

	return std::move(result);
}

static void VacuumExecute(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<VacuumFunctionData>();

	if (data.finished) {
		return;
	}

	// Open a new connection outside of any transaction
	// VACUUM cannot run inside a transaction in SQLite
	SQLiteOpenOptions options;
	options.access_mode = AccessMode::READ_WRITE;
	SQLiteDB db = SQLiteDB::Open(data.file_path, options);

	// Execute VACUUM
	db.Execute("VACUUM");

	// Return success
	auto &result_vector = output.data[0];
	auto result_data = FlatVector::GetData<bool>(result_vector);
	result_data[0] = true;
	output.SetCardinality(1);

	data.finished = true;
}

SQLiteVacuumFunction::SQLiteVacuumFunction()
    : TableFunction("sqlite_vacuum", {LogicalType::VARCHAR}, VacuumExecute, VacuumBind) {
}

} // namespace duckdb
