#ifndef DATABASE_H
#define DATABASE_H

#include <exception>
#include <sqlite3.h>


class DatabaseError : std::exception {

private:
    const std::string m_Message;

public:
    DatabaseError(const std::string &msg);
    const char *what() const throw();

};


/**
 * The Database stores the URLs and the state of the generator.
 */
class Database {

private:
    sqlite3 *m_DB;

    sqlite3_stmt *m_stmtGetState;
    sqlite3_stmt *m_stmtSetState;
    sqlite3_stmt *m_stmtInsertURL;
    sqlite3_stmt *m_stmtGetURL;
    sqlite3_stmt *m_stmtIncrementURLViews;
    sqlite3_stmt *m_stmtGetViews;

protected:
    void incrementViews(const std::string &our_url) throw(DatabaseError);

public:
    Database(const char *filepath) throw(DatabaseError);
    ~Database();

    void storeURL(const std::string &our_url,
                  const std::string &their_url) throw(DatabaseError);
    std::string resolveURL(const std::string &our_url,
                           bool increment_views) throw(DatabaseError);

    unsigned int getViews(const std::string &our_url) throw(DatabaseError);

    sqlite3_int64 getState() throw(DatabaseError);
    void setState(sqlite3_int64 state) throw(DatabaseError);

};

#endif
