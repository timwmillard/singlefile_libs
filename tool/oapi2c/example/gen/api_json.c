// Generated from OpenAPI spec - do not edit

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"

// ============ JSON Helper Functions ============

// Allocator helpers - use allocator if provided, otherwise malloc/free
static void *json_alloc(api_Allocator *alloc, size_t size) {
    if (alloc && alloc->alloc) return alloc->alloc(alloc->ctx, size);
    return malloc(size);
}

static void *json_calloc(api_Allocator *alloc, size_t count, size_t size) {
    size_t total = count * size;
    void *ptr = json_alloc(alloc, total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

static void json_free(api_Allocator *alloc, void *ptr) {
    if (ptr == NULL) return;
    if (alloc && alloc->free) { alloc->free(alloc->ctx, ptr); return; }
    if (alloc && alloc->alloc) return; // Arena without free - don't call free()
    free(ptr);
}

static const char *skip_ws(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static const char *skip_value(const char *p) {
    p = skip_ws(p);
    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && *(p+1)) p++;
            p++;
        }
        if (*p == '"') p++;
    } else if (*p == '{') {
        int depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '{') depth++;
            else if (*p == '}') depth--;
            else if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p+1)) p++;
                    p++;
                }
            }
            if (*p) p++;
        }
    } else if (*p == '[') {
        int depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '[') depth++;
            else if (*p == ']') depth--;
            else if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p+1)) p++;
                    p++;
                }
            }
            if (*p) p++;
        }
    } else {
        while (*p && *p != ',' && *p != '}' && *p != ']') p++;
    }
    return p;
}

static const char *find_key(const char *json, const char *key) {
    const char *p = skip_ws(json);
    if (*p != '{') return NULL;
    p = skip_ws(p + 1);

    size_t keylen = strlen(key);
    while (*p && *p != '}') {
        if (*p != '"') return NULL;
        p++;
        const char *kstart = p;
        while (*p && *p != '"') p++;
        size_t klen = p - kstart;
        p++;
        p = skip_ws(p);
        if (*p != ':') return NULL;
        p = skip_ws(p + 1);

        if (klen == keylen && memcmp(kstart, key, keylen) == 0) {
            return p;
        }
        p = skip_value(p);
        p = skip_ws(p);
        if (*p == ',') p = skip_ws(p + 1);
    }
    return NULL;
}

static char *parse_string(api_Allocator *alloc, const char *p, const char **endp) {
    p = skip_ws(p);
    if (*p != '"') { *endp = p; return NULL; }
    p++;
    const char *start = p;
    size_t len = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && *(p+1)) { p += 2; len++; }
        else { p++; len++; }
    }
    char *str = json_alloc(alloc, len + 1);
    if (!str) { *endp = p; return NULL; }
    const char *s = start;
    char *d = str;
    while (s < p) {
        if (*s == '\\' && *(s+1)) {
            s++;
            switch (*s) {
                case 'n': *d++ = '\n'; break;
                case 't': *d++ = '\t'; break;
                case 'r': *d++ = '\r'; break;
                case '"': *d++ = '"'; break;
                case '\\': *d++ = '\\'; break;
                default: *d++ = *s; break;
            }
            s++;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
    *endp = (*p == '"') ? p + 1 : p;
    return str;
}

static int64_t parse_int(const char *p, const char **endp) {
    p = skip_ws(p);
    char *end;
    int64_t val = strtoll(p, &end, 10);
    *endp = end;
    return val;
}

static double parse_double(const char *p, const char **endp) {
    p = skip_ws(p);
    char *end;
    double val = strtod(p, &end);
    *endp = end;
    return val;
}

static bool parse_bool(const char *p, const char **endp) {
    p = skip_ws(p);
    if (strncmp(p, "true", 4) == 0) { *endp = p + 4; return true; }
    if (strncmp(p, "false", 5) == 0) { *endp = p + 5; return false; }
    *endp = p;
    return false;
}

static size_t count_array(const char *p) {
    p = skip_ws(p);
    if (*p != '[') return 0;
    p = skip_ws(p + 1);
    if (*p == ']') return 0;
    size_t count = 1;
    while (*p && *p != ']') {
        if (*p == ',' ) count++;
        else if (*p == '"') {
            p++;
            while (*p && *p != '"') {
                if (*p == '\\' && *(p+1)) p++;
                p++;
            }
        } else if (*p == '{' || *p == '[') {
            p = skip_value(p) - 1;
        }
        if (*p) p++;
    }
    return count;
}

static const char *array_first(const char *p) {
    p = skip_ws(p);
    if (*p != '[') return NULL;
    p = skip_ws(p + 1);
    if (*p == ']') return NULL;
    return p;
}

static const char *array_next(const char *p) {
    p = skip_value(p);
    p = skip_ws(p);
    if (*p == ',') return skip_ws(p + 1);
    return NULL;
}

static int write_escaped(char *buf, size_t size, const char *str) {
    if (!str) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"");
    for (const char *p = str; *p; p++) {
        switch (*p) {
            case '"':  len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\\""); break;
            case '\\': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\\\"); break;
            case '\n': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\n"); break;
            case '\r': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\r"); break;
            case '\t': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\t"); break;
            default:   len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "%c", *p); break;
        }
    }
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"");
    return len;
}

const char *api_catch_status_to_string(api_CatchStatus val) {
    switch (val) {
        case API_CATCH_STATUS_PENDING: return "pending";
        case API_CATCH_STATUS_VERIFIED: return "verified";
        case API_CATCH_STATUS_DISPUTED: return "disputed";
        case API_CATCH_STATUS_REJECTED: return "rejected";
        default: return NULL;
    }
}

api_CatchStatus api_catch_status_from_string(const char *str) {
    if (str == NULL) return 0;
    if (strcmp(str, "pending") == 0) return API_CATCH_STATUS_PENDING;
    if (strcmp(str, "verified") == 0) return API_CATCH_STATUS_VERIFIED;
    if (strcmp(str, "disputed") == 0) return API_CATCH_STATUS_DISPUTED;
    if (strcmp(str, "rejected") == 0) return API_CATCH_STATUS_REJECTED;
    return 0;
}

// Forward declarations
static const char *api_boat_parse(api_Allocator *alloc, const char *json, api_Boat *obj);
static int api_boat_write(api_Allocator *alloc, char *buf, size_t size, api_Boat *obj);
static const char *api_catch_parse(api_Allocator *alloc, const char *json, api_Catch *obj);
static int api_catch_write(api_Allocator *alloc, char *buf, size_t size, api_Catch *obj);
static const char *api_competitor_parse(api_Allocator *alloc, const char *json, api_Competitor *obj);
static int api_competitor_write(api_Allocator *alloc, char *buf, size_t size, api_Competitor *obj);
static const char *api_leaderboard_entry_parse(api_Allocator *alloc, const char *json, api_LeaderboardEntry *obj);
static int api_leaderboard_entry_write(api_Allocator *alloc, char *buf, size_t size, api_LeaderboardEntry *obj);
static const char *api_species_parse(api_Allocator *alloc, const char *json, api_Species *obj);
static int api_species_write(api_Allocator *alloc, char *buf, size_t size, api_Species *obj);

static int api_boat_write(api_Allocator *alloc, char *buf, size_t size, api_Boat *obj) {
    (void)alloc; // unused in write, but kept for consistent signature
    if (obj == NULL) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "{");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"crew\":[");
    for (size_t i = 0; i < obj->crew_count; i++) {
        if (i > 0) len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",");
        len += api_competitor_write(alloc, buf + len, size > (size_t)len ? size - len : 0, &obj->crew[i]);
    }
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "]");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"id\":%lld", (long long)obj->id);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"name\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->name);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"registration\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->registration);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "}");
    return len;
}

char *api_boat_to_json(api_Allocator *alloc, api_Boat *obj) {
    int len = api_boat_write(alloc, NULL, 0, obj);
    char *buf = json_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;
    api_boat_write(alloc, buf, len + 1, obj);
    return buf;
}

static const char *api_boat_parse(api_Allocator *alloc, const char *json, api_Boat *obj) {
    if (json == NULL || obj == NULL) return NULL;
    memset(obj, 0, sizeof(*obj));
    const char *p;
    (void)p; // may be unused

    p = find_key(json, "crew");
    if (p != NULL) {
        size_t count = count_array(p);
        obj->crew_count = count;
        if (count > 0) {
            obj->crew = json_calloc(alloc, count, sizeof(api_Competitor));
            const char *elem = array_first(p);
            for (size_t i = 0; i < count && elem; i++) {
                api_competitor_parse(alloc, elem, &obj->crew[i]);
                elem = array_next(elem);
            }
        }
    }

    p = find_key(json, "id");
    if (p != NULL) {
        obj->id = parse_int(p, &p);
    }

    p = find_key(json, "name");
    if (p != NULL) {
        obj->name = parse_string(alloc, p, &p);
    }

    p = find_key(json, "registration");
    if (p != NULL) {
        obj->registration = parse_string(alloc, p, &p);
    }

    return skip_value(json);
}

int api_boat_from_json(api_Allocator *alloc, const char *json, api_Boat *obj) {
    return api_boat_parse(alloc, json, obj) != NULL ? 0 : -1;
}

void api_boat_free(api_Allocator *alloc, api_Boat *obj) {
    if (obj == NULL) return;
    for (size_t i = 0; i < obj->crew_count; i++) api_competitor_free(alloc, &obj->crew[i]);
    json_free(alloc, obj->crew);
    json_free(alloc, obj->name);
    json_free(alloc, obj->registration);
}

static int api_catch_write(api_Allocator *alloc, char *buf, size_t size, api_Catch *obj) {
    (void)alloc; // unused in write, but kept for consistent signature
    if (obj == NULL) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "{");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"caught_at\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->caught_at);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"competitor_id\":%lld", (long long)obj->competitor_id);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"id\":%lld", (long long)obj->id);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"length_cm\":%g", (double)obj->length_cm);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"photos\":[");
    for (size_t i = 0; i < obj->photos_count; i++) {
        if (i > 0) len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",");
        len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->photos[i]);
    }
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "]");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"points\":%g", (double)obj->points);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"species\":");
    len += api_species_write(alloc, buf + len, size > (size_t)len ? size - len : 0, obj->species);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"status\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, api_catch_status_to_string(obj->status));
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"verified\":%s", obj->verified ? "true" : "false");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"weight_kg\":%g", (double)obj->weight_kg);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "}");
    return len;
}

char *api_catch_to_json(api_Allocator *alloc, api_Catch *obj) {
    int len = api_catch_write(alloc, NULL, 0, obj);
    char *buf = json_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;
    api_catch_write(alloc, buf, len + 1, obj);
    return buf;
}

static const char *api_catch_parse(api_Allocator *alloc, const char *json, api_Catch *obj) {
    if (json == NULL || obj == NULL) return NULL;
    memset(obj, 0, sizeof(*obj));
    const char *p;
    (void)p; // may be unused

    p = find_key(json, "caught_at");
    if (p != NULL) {
        obj->caught_at = parse_string(alloc, p, &p);
    }

    p = find_key(json, "competitor_id");
    if (p != NULL) {
        obj->competitor_id = parse_int(p, &p);
    }

    p = find_key(json, "id");
    if (p != NULL) {
        obj->id = parse_int(p, &p);
    }

    p = find_key(json, "length_cm");
    if (p != NULL) {
        obj->length_cm = (float)parse_double(p, &p);
    }

    p = find_key(json, "photos");
    if (p != NULL) {
        size_t count = count_array(p);
        obj->photos_count = count;
        if (count > 0) {
            obj->photos = json_calloc(alloc, count, sizeof(char*));
            const char *elem = array_first(p);
            for (size_t i = 0; i < count && elem; i++) {
                obj->photos[i] = parse_string(alloc, elem, &elem);
                elem = array_next(elem);
            }
        }
    }

    p = find_key(json, "points");
    if (p != NULL) {
        obj->points = parse_double(p, &p);
    }

    p = find_key(json, "species");
    if (p != NULL) {
        obj->species = json_calloc(alloc, 1, sizeof(api_Species));
        if (obj->species) api_species_parse(alloc, p, obj->species);
    }

    p = find_key(json, "status");
    if (p != NULL) {
        char *str = parse_string(alloc, p, &p);
        obj->status = api_catch_status_from_string(str);
        json_free(alloc, str);
    }

    p = find_key(json, "verified");
    if (p != NULL) {
        obj->verified = parse_bool(p, &p);
    }

    p = find_key(json, "weight_kg");
    if (p != NULL) {
        obj->weight_kg = parse_double(p, &p);
    }

    return skip_value(json);
}

int api_catch_from_json(api_Allocator *alloc, const char *json, api_Catch *obj) {
    return api_catch_parse(alloc, json, obj) != NULL ? 0 : -1;
}

void api_catch_free(api_Allocator *alloc, api_Catch *obj) {
    if (obj == NULL) return;
    json_free(alloc, obj->caught_at);
    for (size_t i = 0; i < obj->photos_count; i++) json_free(alloc, obj->photos[i]);
    json_free(alloc, obj->photos);
    if (obj->species) { api_species_free(alloc, obj->species); json_free(alloc, obj->species); }
}

static int api_competitor_write(api_Allocator *alloc, char *buf, size_t size, api_Competitor *obj) {
    (void)alloc; // unused in write, but kept for consistent signature
    if (obj == NULL) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "{");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"boat\":");
    len += api_boat_write(alloc, buf + len, size > (size_t)len ? size - len : 0, obj->boat);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"catches\":[");
    for (size_t i = 0; i < obj->catches_count; i++) {
        if (i > 0) len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",");
        len += api_catch_write(alloc, buf + len, size > (size_t)len ? size - len : 0, &obj->catches[i]);
    }
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "]");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"created_at\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->created_at);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"id\":%lld", (long long)obj->id);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"name\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->name);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"total_points\":%g", (double)obj->total_points);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "}");
    return len;
}

char *api_competitor_to_json(api_Allocator *alloc, api_Competitor *obj) {
    int len = api_competitor_write(alloc, NULL, 0, obj);
    char *buf = json_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;
    api_competitor_write(alloc, buf, len + 1, obj);
    return buf;
}

static const char *api_competitor_parse(api_Allocator *alloc, const char *json, api_Competitor *obj) {
    if (json == NULL || obj == NULL) return NULL;
    memset(obj, 0, sizeof(*obj));
    const char *p;
    (void)p; // may be unused

    p = find_key(json, "boat");
    if (p != NULL) {
        obj->boat = json_calloc(alloc, 1, sizeof(api_Boat));
        if (obj->boat) api_boat_parse(alloc, p, obj->boat);
    }

    p = find_key(json, "catches");
    if (p != NULL) {
        size_t count = count_array(p);
        obj->catches_count = count;
        if (count > 0) {
            obj->catches = json_calloc(alloc, count, sizeof(api_Catch));
            const char *elem = array_first(p);
            for (size_t i = 0; i < count && elem; i++) {
                api_catch_parse(alloc, elem, &obj->catches[i]);
                elem = array_next(elem);
            }
        }
    }

    p = find_key(json, "created_at");
    if (p != NULL) {
        obj->created_at = parse_string(alloc, p, &p);
    }

    p = find_key(json, "id");
    if (p != NULL) {
        obj->id = parse_int(p, &p);
    }

    p = find_key(json, "name");
    if (p != NULL) {
        obj->name = parse_string(alloc, p, &p);
    }

    p = find_key(json, "total_points");
    if (p != NULL) {
        obj->total_points = parse_double(p, &p);
    }

    return skip_value(json);
}

int api_competitor_from_json(api_Allocator *alloc, const char *json, api_Competitor *obj) {
    return api_competitor_parse(alloc, json, obj) != NULL ? 0 : -1;
}

void api_competitor_free(api_Allocator *alloc, api_Competitor *obj) {
    if (obj == NULL) return;
    if (obj->boat) { api_boat_free(alloc, obj->boat); json_free(alloc, obj->boat); }
    for (size_t i = 0; i < obj->catches_count; i++) api_catch_free(alloc, &obj->catches[i]);
    json_free(alloc, obj->catches);
    json_free(alloc, obj->created_at);
    json_free(alloc, obj->name);
}

static int api_leaderboard_entry_write(api_Allocator *alloc, char *buf, size_t size, api_LeaderboardEntry *obj) {
    (void)alloc; // unused in write, but kept for consistent signature
    if (obj == NULL) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "{");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"competitor\":");
    len += api_competitor_write(alloc, buf + len, size > (size_t)len ? size - len : 0, obj->competitor);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"rank\":%lld", (long long)obj->rank);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"total_catches\":%lld", (long long)obj->total_catches);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"total_points\":%g", (double)obj->total_points);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"total_weight_kg\":%g", (double)obj->total_weight_kg);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "}");
    return len;
}

char *api_leaderboard_entry_to_json(api_Allocator *alloc, api_LeaderboardEntry *obj) {
    int len = api_leaderboard_entry_write(alloc, NULL, 0, obj);
    char *buf = json_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;
    api_leaderboard_entry_write(alloc, buf, len + 1, obj);
    return buf;
}

static const char *api_leaderboard_entry_parse(api_Allocator *alloc, const char *json, api_LeaderboardEntry *obj) {
    if (json == NULL || obj == NULL) return NULL;
    memset(obj, 0, sizeof(*obj));
    const char *p;
    (void)p; // may be unused

    p = find_key(json, "competitor");
    if (p != NULL) {
        obj->competitor = json_calloc(alloc, 1, sizeof(api_Competitor));
        if (obj->competitor) api_competitor_parse(alloc, p, obj->competitor);
    }

    p = find_key(json, "rank");
    if (p != NULL) {
        obj->rank = (int32_t)parse_int(p, &p);
    }

    p = find_key(json, "total_catches");
    if (p != NULL) {
        obj->total_catches = (int32_t)parse_int(p, &p);
    }

    p = find_key(json, "total_points");
    if (p != NULL) {
        obj->total_points = parse_double(p, &p);
    }

    p = find_key(json, "total_weight_kg");
    if (p != NULL) {
        obj->total_weight_kg = parse_double(p, &p);
    }

    return skip_value(json);
}

int api_leaderboard_entry_from_json(api_Allocator *alloc, const char *json, api_LeaderboardEntry *obj) {
    return api_leaderboard_entry_parse(alloc, json, obj) != NULL ? 0 : -1;
}

void api_leaderboard_entry_free(api_Allocator *alloc, api_LeaderboardEntry *obj) {
    if (obj == NULL) return;
    if (obj->competitor) { api_competitor_free(alloc, obj->competitor); json_free(alloc, obj->competitor); }
}

static int api_species_write(api_Allocator *alloc, char *buf, size_t size, api_Species *obj) {
    (void)alloc; // unused in write, but kept for consistent signature
    if (obj == NULL) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "{");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"id\":%lld", (long long)obj->id);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"min_size_cm\":%g", (double)obj->min_size_cm);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"name\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->name);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"points_per_kg\":%g", (double)obj->points_per_kg);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"scientific_name\":");
    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->scientific_name);
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",\"tags\":[");
    for (size_t i = 0; i < obj->tags_count; i++) {
        if (i > 0) len += snprintf(buf + len, size > (size_t)len ? size - len : 0, ",");
        len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->tags[i]);
    }
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "]");
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "}");
    return len;
}

char *api_species_to_json(api_Allocator *alloc, api_Species *obj) {
    int len = api_species_write(alloc, NULL, 0, obj);
    char *buf = json_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;
    api_species_write(alloc, buf, len + 1, obj);
    return buf;
}

static const char *api_species_parse(api_Allocator *alloc, const char *json, api_Species *obj) {
    if (json == NULL || obj == NULL) return NULL;
    memset(obj, 0, sizeof(*obj));
    const char *p;
    (void)p; // may be unused

    p = find_key(json, "id");
    if (p != NULL) {
        obj->id = parse_int(p, &p);
    }

    p = find_key(json, "min_size_cm");
    if (p != NULL) {
        obj->min_size_cm = (float)parse_double(p, &p);
    }

    p = find_key(json, "name");
    if (p != NULL) {
        obj->name = parse_string(alloc, p, &p);
    }

    p = find_key(json, "points_per_kg");
    if (p != NULL) {
        obj->points_per_kg = parse_double(p, &p);
    }

    p = find_key(json, "scientific_name");
    if (p != NULL) {
        obj->scientific_name = parse_string(alloc, p, &p);
    }

    p = find_key(json, "tags");
    if (p != NULL) {
        size_t count = count_array(p);
        obj->tags_count = count;
        if (count > 0) {
            obj->tags = json_calloc(alloc, count, sizeof(char*));
            const char *elem = array_first(p);
            for (size_t i = 0; i < count && elem; i++) {
                obj->tags[i] = parse_string(alloc, elem, &elem);
                elem = array_next(elem);
            }
        }
    }

    return skip_value(json);
}

int api_species_from_json(api_Allocator *alloc, const char *json, api_Species *obj) {
    return api_species_parse(alloc, json, obj) != NULL ? 0 : -1;
}

void api_species_free(api_Allocator *alloc, api_Species *obj) {
    if (obj == NULL) return;
    json_free(alloc, obj->name);
    json_free(alloc, obj->scientific_name);
    for (size_t i = 0; i < obj->tags_count; i++) json_free(alloc, obj->tags[i]);
    json_free(alloc, obj->tags);
}

