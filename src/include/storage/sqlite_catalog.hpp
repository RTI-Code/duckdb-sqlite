//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/sqlite_catalog.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/catalog.hpp"
#include "sqlite_options.hpp"
#include "sqlite_db.hpp"
#include <atomic>
#include <mutex>
#include <vector>

namespace duckdb {
class SQLiteSchemaEntry;

class SQLiteCatalog : public Catalog {
public:
	explicit SQLiteCatalog(AttachedDatabase &db_p, const string &path, SQLiteOpenOptions options);
	~SQLiteCatalog();

	string path;
	SQLiteOpenOptions options;

public:
	void Initialize(bool load_builtin) override;
	string GetCatalogType() override {
		return "sqlite";
	}

	optional_ptr<CatalogEntry> CreateSchema(CatalogTransaction transaction, CreateSchemaInfo &info) override;

	void ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) override;

	optional_ptr<SchemaCatalogEntry> LookupSchema(CatalogTransaction transaction, const EntryLookupInfo &schema_lookup,
	                                              OnEntryNotFound if_not_found) override;

	SQLiteSchemaEntry &GetMainSchema() {
		return *main_schema;
	}

	PhysicalOperator &PlanCreateTableAs(ClientContext &context, PhysicalPlanGenerator &planner, LogicalCreateTable &op,
	                                    PhysicalOperator &plan) override;
	PhysicalOperator &PlanInsert(ClientContext &context, PhysicalPlanGenerator &planner, LogicalInsert &op,
	                             optional_ptr<PhysicalOperator> plan) override;
	PhysicalOperator &PlanDelete(ClientContext &context, PhysicalPlanGenerator &planner, LogicalDelete &op,
	                             PhysicalOperator &plan) override;
	PhysicalOperator &PlanUpdate(ClientContext &context, PhysicalPlanGenerator &planner, LogicalUpdate &op,
	                             PhysicalOperator &plan) override;

	unique_ptr<LogicalOperator> BindCreateIndex(Binder &binder, CreateStatement &stmt, TableCatalogEntry &table,
	                                            unique_ptr<LogicalOperator> plan) override;

	DatabaseSize GetDatabaseSize(ClientContext &context) override;

	//! Whether or not this is an in-memory SQLite database
	bool InMemory() override;
	string GetDBPath() override;

	//! Returns a reference to the persistent database connection
	SQLiteDB &GetPersistentDatabase();

	//! Transaction state management for the shared connection
	//! Returns true if this call started the transaction (caller should track this)
	bool TryBeginTransaction();
	//! Commits if this was the last active transaction
	void EndTransaction(bool commit);

	//! Acquire a read-only connection from the pool (or open a new one)
	SQLiteDB AcquireReadConnection();
	//! Return a read-only connection to the pool
	void ReleaseReadConnection(SQLiteDB db);

private:
	void DropSchema(ClientContext &context, DropInfo &info) override;

private:
	unique_ptr<SQLiteSchemaEntry> main_schema;
	//! Whether or not the database is in-memory
	bool in_memory;
	//! Single persistent database connection (kept open to avoid WAL checkpoints)
	SQLiteDB persistent_db;
	//! Transaction state tracking for the writer connection
	std::mutex transaction_mutex;
	int transaction_depth = 0;
	//! Read-only connection pool
	static constexpr idx_t MAX_READER_POOL_SIZE = 64;
	std::vector<SQLiteDB> reader_pool;
	std::mutex reader_pool_mutex;
};

} // namespace duckdb
