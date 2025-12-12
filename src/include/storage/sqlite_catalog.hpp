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

	//! Returns a reference to the in-memory database (if any)
	SQLiteDB *GetInMemoryDatabase();
	//! Release the in-memory database (if there is any)
	void ReleaseInMemoryDatabase();

	//! Returns a reference to the persistent database connection
	SQLiteDB *GetDatabase();
	//! Release the database connection
	void ReleaseDatabase(SQLiteDB *db);

private:
	void DropSchema(ClientContext &context, DropInfo &info) override;

private:
	unique_ptr<SQLiteSchemaEntry> main_schema;
	//! Whether or not the database is in-memory
	bool in_memory;
	//! In-memory database - if any
	SQLiteDB in_memory_db;
	//! The lock maintaing access to the in-memory database
	mutex in_memory_lock;
	//! Whether or not there is any active transaction on the in-memory database
	bool active_in_memory;
	//! Connection pool for on-disk databases
	vector<unique_ptr<SQLiteDB>> connection_pool;
	vector<SQLiteDB*> available_connections;
	//! The lock maintaining access to the connection pool
	mutex pool_lock;
};

} // namespace duckdb
