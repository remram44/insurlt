#include <iostream>
#include <string>

#include "database.h"


#define check(r) if((r) != SQLITE_OK) { goto sqlerror; }


DatabaseError::DatabaseError(const std::string &msg)
  : m_Message(msg)
{
}

DatabaseError::~DatabaseError() throw()
{
}

const char *DatabaseError::what() const throw()
{
    return m_Message.c_str();
}


Database::Database(const char *filepath, Generator &generator)
        throw(DatabaseError)
  : m_Generator(generator)
{
    if(sqlite3_open(filepath, &m_DB) != SQLITE_OK)
        throw DatabaseError(std::string("Can't open ") + filepath);
    sqlite3_busy_timeout(m_DB, 50);

    // Checks whether the database is already setup
    bool database_setup = false;
    if(sqlite3_prepare_v2(m_DB, "SELECT state FROM generator_state;",
                          -1, &m_stmtGetState, NULL) == SQLITE_OK)
    {
        int ret = sqlite3_step(m_stmtGetState);
        if(ret == SQLITE_ROW)
        {
            sqlite3_int64 state = sqlite3_column_int(m_stmtGetState, 0);
            database_setup = true;
            std::cerr << "Database initialized, generator state: " << state
                      << std::endl;
            ret = sqlite3_step(m_stmtGetState);
        }
        if(ret != SQLITE_DONE)
            goto sqlerror;
        sqlite3_reset(m_stmtGetState);
    }

    if(!database_setup)
    {
        // Creates the tables
        const char *sql[] = {
            "CREATE TABLE generator_state("
            "    state INTEGER NOT NULL"
            "    );",
            "CREATE TABLE urls("
            "    id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
            "    our_url TEXT NOT NULL,"
            "    target_url TEXT NOT NULL,"
            "    views INTEGER NOT NULL"
            "    );",
            "CREATE INDEX urls_our_url_idx ON urls(our_url);",
            "INSERT INTO generator_state(state) VALUES(0);"
        };
        for(const char *sql_line : sql)
            check(sqlite3_exec(m_DB, sql_line, NULL, NULL, NULL));
        std::cerr << "Database created, generator state: 0" << std::endl;
    }

    if(!database_setup)
        check(sqlite3_prepare_v2(m_DB, "SELECT state FROM generator_state;",
                                 -1, &m_stmtGetState, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "UPDATE generator_state SET state=?;",
            -1, &m_stmtSetState, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "INSERT INTO urls(our_url, target_url, views) "
            "VALUES(?, ?, 0);",
            -1, &m_stmtInsertURL, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "SELECT target_url FROM urls WHERE our_url=?;",
            -1, &m_stmtGetURL, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "UPDATE urls SET views=views+1 WHERE our_url=?;",
            -1, &m_stmtIncrementURLViews, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "SELECT views FROM urls WHERE our_url=?;",
            -1, &m_stmtGetViews, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "BEGIN TRANSACTION;",
            -1, &m_stmtBegin, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "COMMIT;",
            -1, &m_stmtCommit, NULL));
    check(sqlite3_prepare_v2(
            m_DB,
            "ROLLBACK;",
            -1, &m_stmtRollback, NULL));

    return;

sqlerror:
    throw DatabaseError("Couldn't initialize database");
}

Database::~Database()
{
    sqlite3_finalize(m_stmtGetState);
    sqlite3_finalize(m_stmtSetState);
    sqlite3_finalize(m_stmtInsertURL);
    sqlite3_finalize(m_stmtGetURL);
    sqlite3_finalize(m_stmtIncrementURLViews);
    sqlite3_finalize(m_stmtGetViews);
    sqlite3_close(m_DB);
}

void Database::storeURL(const std::string &our_url,
                        const std::string &their_url) throw(DatabaseError)
{
    check(sqlite3_bind_text(m_stmtInsertURL, 1,
                            our_url.c_str(), our_url.size(),
                            SQLITE_TRANSIENT));
    check(sqlite3_bind_text(m_stmtInsertURL, 2,
                            their_url.c_str(), their_url.size(),
                            SQLITE_TRANSIENT));

    if(sqlite3_step(m_stmtInsertURL) != SQLITE_DONE)
        goto sqlerror;
    sqlite3_reset(m_stmtInsertURL);
    return;

sqlerror:
    throw DatabaseError("Couldn't insert a URL");
}

void Database::incrementViews(const std::string &our_url) throw(DatabaseError)
{
    check(sqlite3_bind_text(m_stmtIncrementURLViews, 1,
                            our_url.c_str(), our_url.size(),
                            SQLITE_TRANSIENT));

    if(sqlite3_step(m_stmtIncrementURLViews) == SQLITE_DONE)
    {
        sqlite3_reset(m_stmtIncrementURLViews);
        return;
    }

sqlerror:
    throw DatabaseError("Couldn't increment a URL's view count");
}

std::string Database::resolveURL(const std::string &our_url,
                                 bool increment_views) throw(DatabaseError)
{
    int ret;
    check(sqlite3_bind_text(m_stmtGetURL, 1,
                            our_url.c_str(), our_url.size(),
                            SQLITE_TRANSIENT));

    ret = sqlite3_step(m_stmtGetURL);
    if(ret == SQLITE_DONE)
    {
        sqlite3_reset(m_stmtGetURL);
        return "";
    }
    else if(ret == SQLITE_ROW)
    {
        std::string result((const char*)sqlite3_column_text(m_stmtGetURL, 0),
                           sqlite3_column_bytes(m_stmtGetURL, 0));
        sqlite3_reset(m_stmtGetURL);
        if(increment_views)
            incrementViews(our_url);
        return result;
    }

sqlerror:
    throw DatabaseError("Couldn't resolve a URL");
}

unsigned int Database::getViews(const std::string &our_url)
        throw(DatabaseError)
{
    int ret;
    check(sqlite3_bind_text(m_stmtGetViews, 1,
                            our_url.c_str(), our_url.size(),
                            SQLITE_TRANSIENT));

    ret = sqlite3_step(m_stmtGetViews);
    if(ret == SQLITE_DONE)
    {
        sqlite3_reset(m_stmtGetViews);
        return 0;
    }
    else if(ret == SQLITE_ROW)
    {
        unsigned int result = sqlite3_column_int(m_stmtGetViews, 0);
        sqlite3_reset(m_stmtGetViews);
        return result;
    }

sqlerror:
    throw DatabaseError("Couldn't get a URL's' view count");
}

sqlite3_int64 Database::getState() throw(DatabaseError)
{
    if(sqlite3_step(m_stmtGetState) == SQLITE_ROW)
    {
        sqlite3_int64 result = sqlite3_column_int64(m_stmtGetState, 0);
        if(sqlite3_step(m_stmtGetState) != SQLITE_DONE)
            goto sqlerror;
        sqlite3_reset(m_stmtGetState);
        return result;
    }

sqlerror:
    throw DatabaseError("Couldn't get the generator state");
}

void Database::setState(sqlite3_int64 state) throw(DatabaseError)
{
    check(sqlite3_bind_int64(m_stmtSetState, 1, state));
    if(sqlite3_step(m_stmtSetState) != SQLITE_DONE)
        goto sqlerror;
    sqlite3_reset(m_stmtSetState);
    return;

sqlerror:
    throw DatabaseError("Couldn't set the generator state");
}

Key Database::nextState() throw(DatabaseError)
{
    while(true)
    {
        if(sqlite3_step(m_stmtBegin) != SQLITE_DONE)
            goto sqlerror;
        check(sqlite3_reset(m_stmtBegin));

        Key state = getState();
        state = m_Generator.generate(state);

        check(sqlite3_bind_int64(m_stmtSetState, 1, state));
        int ret = sqlite3_step(m_stmtSetState);
        check(sqlite3_reset(m_stmtSetState));
        if(ret == SQLITE_DONE)
        {
            ret = sqlite3_step(m_stmtCommit);
            check(sqlite3_reset(m_stmtCommit));
        }
        switch(ret)
        {
        case SQLITE_DONE:
            return state;
        case SQLITE_BUSY:
            if(sqlite3_step(m_stmtRollback) != SQLITE_DONE)
                goto sqlerror;
            check(sqlite3_reset(m_stmtRollback));
            continue;
        default:
            goto sqlerror;
        }
    }

sqlerror:
    throw DatabaseError("Couldn't advance the generator state");
}
