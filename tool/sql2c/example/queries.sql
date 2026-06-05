-- name: GetCompetitor :one
select *
from competitor
where id = ?;

-- name: ListCompetitors :many
select *
from competitor;

-- name: CreateCompetitor :one
insert into competitor (
    first_name,
    last_name,
    email
) values (?, ?, ?)
returning *;

-- name: UpdateCompetitorEmail :one
update competitor
set email = ?
where id = ?
returning *;

-- name: DeleteCompetitor :exec
delete from competitor
where id = ?;

-- name: GetBoat :one
select *
from boat
where id = ?;

-- name: ListBoats :many
select *
from boat;

-- name: CreateBoat :one
insert into boat (name, registration)
values (?, ?)
returning *;

-- name: GetCatch :one
select *
from catch
where id = ?;

-- name: ListCatchesByCompetitor :many
select *
from catch
where competitor_id = ?;

-- name: CreateCatch :one
insert into catch (competitor_id, species, weight_grams, caught_at)
values (?, ?, ?, ?)
returning *;
