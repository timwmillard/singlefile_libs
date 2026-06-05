-- Boat
create table if not exists boat (
    id integer primary key,
    name text not null default '',
    registration text not null default ''
);

-- Competitor
create table if not exists competitor (
    id integer primary key,
    first_name text not null default '',
    last_name text not null default '',
    email text not null default '',
    boat_id integer references boat(id)
);

-- Catch
create table if not exists catch (
    id integer primary key,
    competitor_id integer not null references competitor(id),
    species text not null,
    weight_grams integer not null check (weight_grams > 0),
    caught_at text not null default ''
);
