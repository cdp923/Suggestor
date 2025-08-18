#include "fileProcessed.h"
#include <iostream>

bool createProcessedFileTable(sqlite3* db) {
    const char* table = "CREATE TABLE IF NOT EXISTS processed_files("
                        "filePath TEXT PRIMARY KEY);";
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, table, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating processed_files table: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}
bool checkIfFileIsProcessed(sqlite3* db, const std::string& filePath) {
    const char* sql = "SELECT 1 FROM processed_files WHERE filePath = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare check statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, filePath.c_str(), -1, SQLITE_TRANSIENT);

    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = true;
    }

    sqlite3_finalize(stmt);
    return result;
}
bool logProcessedFile(sqlite3* db, const std::string& filePath) {
    const char* sql = "INSERT OR IGNORE INTO processed_files (filePath) VALUES (?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare log statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, filePath.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to log processed file: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}