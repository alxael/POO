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
                "user",
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