CREATE TABLE IF NOT EXISTS Currencies (
    id int NOT NULL PRIMARY KEY,
    name varchar(255) NOT NULL,
    code varchar(255) NOT NULL UNIQUE
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
    "user" int references Users(id),
    iban varchar(255) NOT NULL UNIQUE,
    amount double precision NOT NULL,
    firstname varchar(255) NOT NULL,
    lastname varchar(255) NOT NULL
);