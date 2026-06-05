// Generated from SQL - do not edit

#ifndef QUERIES_H
#define QUERIES_H

#include "sqlite3.h"

#include "models.h"

// ============ Param Structs ============

typedef struct {
    sql_text FirstName;
    sql_text LastName;
    sql_text Email;
} CreateCompetitorParams;

typedef struct {
    sql_text Email;
    sql_int64 Id;
} UpdateCompetitorEmailParams;

typedef struct {
    sql_text Name;
    sql_text Registration;
} CreateBoatParams;

typedef struct {
    sql_int64 CompetitorId;
    sql_text Species;
    sql_int64 WeightGrams;
    sql_text CaughtAt;
} CreateCatchParams;

// ============ Query Functions ============

// select * from competitor where id = ?;
int get_competitor(sqlite3 *db, sql_int64 Id, void (*cb)(Competitor*, void*), void *ctx);
// select * from competitor;
int list_competitors(sqlite3 *db, void (*cb)(Competitor*, void*), void *ctx);
// insert into competitor ( first_name, last_name, email ) values (?, ?, ?) returning *;
int create_competitor(sqlite3 *db, CreateCompetitorParams *params, void (*cb)(Competitor*, void*), void *ctx);
// update competitor set email = ? where id = ? returning *;
int update_competitor_email(sqlite3 *db, UpdateCompetitorEmailParams *params, void (*cb)(Competitor*, void*), void *ctx);
// delete from competitor where id = ?;
int delete_competitor(sqlite3 *db, sql_int64 Id);

// select * from boat where id = ?;
int get_boat(sqlite3 *db, sql_int64 Id, void (*cb)(Boat*, void*), void *ctx);
// select * from boat;
int list_boats(sqlite3 *db, void (*cb)(Boat*, void*), void *ctx);
// insert into boat (name, registration) values (?, ?) returning *;
int create_boat(sqlite3 *db, CreateBoatParams *params, void (*cb)(Boat*, void*), void *ctx);
// select * from catch where id = ?;
int get_catch(sqlite3 *db, sql_int64 Id, void (*cb)(Catch*, void*), void *ctx);
// select * from catch where competitor_id = ?;
int list_catches_by_competitor(sqlite3 *db, sql_int64 CompetitorId, void (*cb)(Catch*, void*), void *ctx);
// insert into catch (competitor_id, species, weight_grams, caught_at) values (?, ?, ?, ?) returning *;
int create_catch(sqlite3 *db, CreateCatchParams *params, void (*cb)(Catch*, void*), void *ctx);

#endif // QUERIES_H
