// Generated from SQL - do not edit

#ifndef SQL_MODEL_H
#define SQL_MODEL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef unsigned char sql_byte;

typedef double sql_double;

typedef struct {
    sql_double value;
    bool null;
} sql_nulldouble;

typedef int sql_int;

typedef struct {
    sql_int value;
    bool null;
} sql_nullint;

typedef int64_t sql_int64;

typedef struct {
    sql_int64 value;
    bool null;
} sql_nullint64;

typedef double sql_numeric;

typedef struct {
    sql_numeric value;
    bool null;
} sql_nullnumeric;

typedef bool sql_bool;

typedef struct {
    sql_bool value;
    bool null;
} sql_nullbool;

typedef struct {
    sql_byte *data;
    size_t len;
} sql_blob;

typedef struct {
    sql_byte *data;
    size_t len;
    bool null;
} sql_nullblob;

typedef struct {
    sql_byte *data;
    size_t len;
} sql_text;

typedef struct {
    char *data;
    size_t len;
    bool null;
} sql_nulltext;

// ============ Table Structs ============

typedef struct {
    sql_int64 Id;
    sql_text Name;
    sql_text Registration;
} Boat;

typedef struct {
    sql_int64 Id;
    sql_int64 CompetitorId;
    sql_text Species;
    sql_int64 WeightGrams;
    sql_text CaughtAt;
} Catch;

typedef struct {
    sql_int64 Id;
    sql_text FirstName;
    sql_text LastName;
    sql_text Email;
    sql_nullint64 BoatId;
} Competitor;

#endif // SQL_MODEL_H
