-- Создание базы данных сервера системы демонстрации медиаконтента


DROP TABLE agents           CASCADE;
DROP TABLE agents_groups    CASCADE;
DROP TABLE media            CASCADE;
DROP TABLE blocks           CASCADE;
DROP TABLE schedule         CASCADE;
DROP TABLE blocks_media     CASCADE;
DROP TABLE schedule_blocks  CASCADE;

DROP TYPE IF EXISTS media_type       CASCADE;

CREATE TYPE media_type AS ENUM ('image', 'movie');

CREATE TABLE agents_groups  (   id              SERIAL PRIMARY KEY,
                                name            varchar(128) NOT NULL UNIQUE,
                                description     text 
                            );


CREATE TABLE agents (           id              SERIAL PRIMARY KEY,
                                group_id        integer REFERENCES agents_groups (id) NOT NULL,
                                agent_id        varchar(128) NOT NULL UNIQUE
                    );


CREATE TABLE media  (           id              BIGSERIAL PRIMARY KEY,
                                type            media_type NOT NULL,
                                hash            varchar(128) NOT NULL,
                                size            bigint NOT NULL,
                                add_timestamp   timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                last_access     timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                name            varchar(256),
                                description     text
                    );

CREATE UNIQUE INDEX media_hash_index ON media (type, hash);

CREATE TABLE blocks (           id              SERIAL PRIMARY KEY,
                                name            varchar(128) NOT NULL UNIQUE,
                                add_timestamp   timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                last_change     timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                version         integer NOT NULL DEFAULT 0,
                                description     text,
                                data            text
                    );

CREATE TABLE blocks_media   (   block_id        integer REFERENCES blocks (id) NOT NULL,
                                media_id        bigint REFERENCES media (id) NOT NULL
                            );

CREATE UNIQUE INDEX blocks_media_index  ON blocks_media (block_id, media_id);

CREATE TABLE schedule   (       id              SERIAL PRIMARY KEY,
                                group_id        integer REFERENCES agents_groups (id) NOT NULL UNIQUE,
                                add_timestamp   timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                last_change     timestamp with time zone NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                version         integer NOT NULL DEFAULT 1,
                                description     text,
                                data            text,
				CHECK(version > 0)
                        );

CREATE TABLE schedule_blocks (  schedule_id     integer REFERENCES schedule (id) NOT NULL,
                                block_id        integer REFERENCES blocks (id) NOT NULL
                             );

CREATE UNIQUE INDEX schedule_blocks_index ON schedule_blocks (schedule_id, block_id);
