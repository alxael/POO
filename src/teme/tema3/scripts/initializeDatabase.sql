CREATE TABLE IF NOT EXISTS Currencies (
        id int NOT NULL PRIMARY KEY,
        name varchar(255) NOT NULL,
        code varchar(255) NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS Exchanges (
        id int NOT NULL PRIMARY KEY,
        source int references Currencies(id),
        destination int references Currencies(id),
        rate double precision NOT NULL
);

CREATE TABLE IF NOT EXISTS Countries (
        id int NOT NULL PRIMARY KEY,
        name varchar(255) NOT NULL,
        code varchar(255) NOT NULL UNIQUE,
        ibanpattern varchar(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS Users (
        id int NOT NULL PRIMARY KEY,
        country int references Countries(id),
        email varchar(255) UNIQUE,
        firstname varchar(255),
        lastname varchar(255),
        password varchar(255) NOT NULL
);
CREATE SEQUENCE users_seq INCREMENT 1 START 2 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1;
ALTER SEQUENCE public.users_seq OWNER TO postgres;
ALTER TABLE users ALTER COLUMN id SET DEFAULT nextval('users_seq');

CREATE TABLE IF NOT EXISTS Accounts (
        id int NOT NULL PRIMARY KEY,
        currency int references Currencies(id),
        associatedUser int references Users(id),
        iban varchar(255) NOT NULL UNIQUE,
        amount double precision NOT NULL,
        firstname varchar(255) NOT NULL,
        lastname varchar(255) NOT NULL
);
CREATE SEQUENCE accounts_seq INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1;
ALTER SEQUENCE public.accounts_seq OWNER TO postgres;
ALTER TABLE accounts ALTER COLUMN id SET DEFAULT nextval('accounts_seq');

CREATE TABLE IF NOT EXISTS Transactions (
        id int NOT NULL PRIMARY KEY,
        inbound int references Accounts(id),
        outbound int references Accounts(id),
        amount double precision NOT NULL,
        date timestamp NOT NULL
);
CREATE SEQUENCE transactions_seq INCREMENT 1 START 1 MINVALUE 1 MAXVALUE 9223372036854775807 CACHE 1;
ALTER SEQUENCE public.transactions_seq OWNER TO postgres;
ALTER TABLE transactions ALTER COLUMN id SET DEFAULT nextval('transactions_seq');

INSERT INTO
        currencies (id, name, code)
VALUES
        (1, 'Euro', 'EUR'),
        (2, 'Romanian new leu', 'RON'),
        (3, 'British pound sterling', 'GBP');

INSERT INTO
        exchanges (id, source, destination, rate)
VALUES
        (1, 1, 1, 1),
        (2, 1, 2, 4.9750),
        (3, 1, 3, 0.8558),
        (4, 2, 1, 0.2010),
        (5, 2, 2, 1),
        (6, 2, 3, 0.1720),
        (7, 3, 1, 1.1683),
        (8, 3, 2, 5.8126),
        (9, 3, 3, 1);

INSERT INTO
        countries (id, name, code, ibanpattern)
VALUES
        (1, 'Romania', 'RO', 'aaaacccccccccccccccc'),
        (2, 'France', 'FR', 'nnnnnnnnnncccccccccccnn'),
        (3, 'Germany', 'GR', 'nnnnnnnnnnnnnnnnnn'),
        (4, 'Italy', 'IT', 'annnnnnnnnncccccccccccc'),
        (5, 'United Kingdom', 'GB', 'aaaannnnnnnnnnnnnn');

INSERT INTO
        users (country, email, firstname, lastname, password)
VALUES
        (1, 'admin@admin.com', 'admin', 'admin', 'nPQYuMQ86pZZ7D7hQfgOWwcvpFehsPbM4BTzP2aMMf8='),
        (2, 'test@test.com', 'test', 'test', 'nPQYuMQ86pZZ7D7hQfgOWwcvpFehsPbM4BTzP2aMMf8=');

INSERT INTO
        accounts (currency, associatedUser, iban, amount, firstname, lastname)
VALUES
        (1, 1, 'RO83OPPCo1JNAQ8eEheih5zI', 1000000, 'admin', 'admin');