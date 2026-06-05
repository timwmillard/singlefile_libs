// Generated from OpenAPI spec - do not edit

#ifndef API_H
#define API_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct Req Req;
typedef struct Res Res;

// ============ Allocator ============

// Allocator interface - pass NULL to use malloc/free
typedef struct {
    void* (*alloc)(void *ctx, size_t size);
    void  (*free)(void *ctx, void *ptr);  // Can be NULL for arenas
    void *ctx;
} api_Allocator;

// ============ Enums ============

// Status of a catch verification
typedef enum {
    API_CATCH_STATUS_PENDING,
    API_CATCH_STATUS_VERIFIED,
    API_CATCH_STATUS_DISPUTED,
    API_CATCH_STATUS_REJECTED
} api_CatchStatus;

// Struct forward declarations
typedef struct api_Boat api_Boat;
typedef struct api_Catch api_Catch;
typedef struct api_Competitor api_Competitor;
typedef struct api_LeaderboardEntry api_LeaderboardEntry;
typedef struct api_Species api_Species;

// ============ Structs ============

// A registered fishing boat
struct api_Boat {
    // List of crew members
    api_Competitor *crew;
    size_t crew_count;
    // Unique identifier for the boat
    int64_t id;
    // Name of the boat
    char* name;
    // Official registration number
    char* registration;
};

// A recorded catch in the competition
struct api_Catch {
    // Timestamp when the fish was caught
    char* caught_at;
    // ID of the competitor who made the catch
    int64_t competitor_id;
    // Unique identifier for the catch
    int64_t id;
    // Length of the catch in centimeters
    float length_cm;
    // URLs of photos of the catch
    char* *photos;
    size_t photos_count;
    // Points awarded for this catch
    double points;
    // A fish species that can be caught
    api_Species *species;
    // Status of a catch verification
    api_CatchStatus status;
    // Whether the catch has been verified by officials
    bool verified;
    // Weight of the catch in kilograms
    double weight_kg;
};

// A competitor in the fishing competition
struct api_Competitor {
    // A registered fishing boat
    api_Boat *boat;
    // List of catches made by this competitor
    api_Catch *catches;
    size_t catches_count;
    // Timestamp when the competitor was registered
    char* created_at;
    // Unique identifier for the competitor
    int64_t id;
    // Full name of the competitor
    char* name;
    // Total points accumulated
    double total_points;
};

// An entry in the competition leaderboard
struct api_LeaderboardEntry {
    // A competitor in the fishing competition
    api_Competitor *competitor;
    // Current ranking position
    int32_t rank;
    // Total number of catches
    int32_t total_catches;
    // Total points accumulated
    double total_points;
    // Total weight of all catches
    double total_weight_kg;
};

// A fish species that can be caught
struct api_Species {
    // Unique identifier for the species
    int64_t id;
    // Minimum legal size in centimeters
    float min_size_cm;
    // Common name of the species
    char* name;
    // Points awarded per kilogram
    double points_per_kg;
    // Scientific name of the species
    char* scientific_name;
    // Classification tags for this species
    char* *tags;
    size_t tags_count;
};

// ============ JSON Functions ============

// String conversion for api_CatchStatus
const char *api_catch_status_to_string(api_CatchStatus val);
api_CatchStatus api_catch_status_from_string(const char *str);

// JSON serialization for api_Boat
char *api_boat_to_json(api_Allocator *alloc, api_Boat *obj);
int api_boat_from_json(api_Allocator *alloc, const char *json, api_Boat *obj);
void api_boat_free(api_Allocator *alloc, api_Boat *obj);

// JSON serialization for api_Catch
char *api_catch_to_json(api_Allocator *alloc, api_Catch *obj);
int api_catch_from_json(api_Allocator *alloc, const char *json, api_Catch *obj);
void api_catch_free(api_Allocator *alloc, api_Catch *obj);

// JSON serialization for api_Competitor
char *api_competitor_to_json(api_Allocator *alloc, api_Competitor *obj);
int api_competitor_from_json(api_Allocator *alloc, const char *json, api_Competitor *obj);
void api_competitor_free(api_Allocator *alloc, api_Competitor *obj);

// JSON serialization for api_LeaderboardEntry
char *api_leaderboard_entry_to_json(api_Allocator *alloc, api_LeaderboardEntry *obj);
int api_leaderboard_entry_from_json(api_Allocator *alloc, const char *json, api_LeaderboardEntry *obj);
void api_leaderboard_entry_free(api_Allocator *alloc, api_LeaderboardEntry *obj);

// JSON serialization for api_Species
char *api_species_to_json(api_Allocator *alloc, api_Species *obj);
int api_species_from_json(api_Allocator *alloc, const char *json, api_Species *obj);
void api_species_free(api_Allocator *alloc, api_Species *obj);

// ============ Handlers ============

// GET /boats - List all boats
void api_get_boats(Req *req, Res *res);

// POST /boats - Create a boat
void api_create_boat(Req *req, Res *res);

// GET /boats/{id} - Get a boat by ID
void api_get_boat(Req *req, Res *res);

// GET /catches - List all catches
void api_get_catches(Req *req, Res *res);

// POST /catches - Record a catch
void api_create_catch(Req *req, Res *res);

// GET /competitors - List all competitors
void api_get_competitors(Req *req, Res *res);

// POST /competitors - Create a competitor
void api_create_competitor(Req *req, Res *res);

// GET /competitors/{id} - Get a competitor by ID
void api_get_competitor(Req *req, Res *res);

// PUT /competitors/{id} - Update a competitor
void api_update_competitor(Req *req, Res *res);

// DELETE /competitors/{id} - Delete a competitor
void api_delete_competitor(Req *req, Res *res);

// GET /leaderboard - Get competition leaderboard
void api_get_leaderboard(Req *req, Res *res);

// GET /species - List all species
void api_get_species(Req *req, Res *res);


#endif // API_H
