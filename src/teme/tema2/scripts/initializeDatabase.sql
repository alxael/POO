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

CREATE TABLE IF NOT EXISTS Accounts (
        id int NOT NULL PRIMARY KEY,
        currency int references Currencies(id),
        associatedUser int references Users(id),
        iban varchar(255) NOT NULL UNIQUE,
        amount double precision NOT NULL,
        firstname varchar(255) NOT NULL,
        lastname varchar(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS Transactions (
        id int NOT NULL PRIMARY KEY,
        inbound int references Accounts(id),
        outbound int references Accounts(id),
        amount double precision NOT NULL
);

INSERT INTO
        currencies (id, name, code)
VALUES
        (0, 'Euro', 'EUR'),
        (1, 'Romanian new leu', 'RON'),
        (2, 'British pound sterling', 'GBP');

INSERT INTO
        exchanges (id, source, destination, rate)
VALUES
        (0, 0, 0, 1),
        (1, 0, 1, 4.9750),
        (2, 0, 2, 0.8558),
        (3, 1, 0, 0.2010),
        (4, 1, 1, 1),
        (5, 1, 2, 0.1720),
        (6, 2, 0, 1.1683),
        (7, 2, 1, 5.8126),
        (8, 2, 2, 1);

INSERT INTO
        countries (id, name, code, ibanpattern)
VALUES
        (
                0,
                'Romania',
                'RO',
                'aaaacccccccccccccccc'
        ),
        (
                1,
                'France',
                'FR',
                'nnnnnnnnnncccccccccccnn'
        ),
        (2, 'Germany', 'GR', 'nnnnnnnnnnnnnnnnnn'),
        (
                3,
                'Italy',
                'IT',
                'annnnnnnnnncccccccccccc'
        ),
        (
                4,
                'United Kingdom',
                'GB',
                'aaaannnnnnnnnnnnnn'
        );

INSERT INTO
        users (
                id,
                country,
                email,
                firstname,
                lastname,
                password
        )
VALUES
        (
                0,
                0,
                'admin@admin.com',
                'admin',
                'admin',
                'nPQYuMQ86pZZ7D7hQfgOWwcvpFehsPbM4BTzP2aMMf8='
        ),
        (
                1,
                1,
                'test@test.com',
                'test',
                'test',
                'nPQYuMQ86pZZ7D7hQfgOWwcvpFehsPbM4BTzP2aMMf8='
        );

INSERT INTO
        accounts (
                id,
                currency,
                associatedUser,
                iban,
                amount,
                firstname,
                lastname
        )
VALUES
        (
                0,
                0,
                0,
                'RO83OPPCo1JNAQ8eEheih5zI',
                1000000,
                'admin',
                'admin'
        );