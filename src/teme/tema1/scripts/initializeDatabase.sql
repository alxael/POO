INSERT INTO
        currencies (id, name, code)
VALUES
        (1, 'Euro', 'EUR'),
        (2, 'Romanian new leu', 'RON'),
        (3, 'British pound sterling', 'GBP');

INSERT INTO
        countries (id, name, code, ibanpattern)
VALUES
        (
                1,
                'Romania',
                'RO',
                'aaaacccccccccccccccc'
        ),
        (
                2,
                'France',
                'FR',
                'nnnnnnnnnncccccccccccnn'
        ),
        (3, 'Germany', 'GR', 'nnnnnnnnnnnnnnnnnn'),
        (
                4,
                'Italy',
                'IT',
                'annnnnnnnnncccccccccccc'
        ),
        (
                5,
                'United Kingdom',
                'GB',
                'aaaannnnnnnnnnnnnn'
        );

INSERT INTO
        users(
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
                1,
                'alex.aelenei04@gmail.com',
                'Alex',
                'Aelenei',
                'nPQYuMQ86pZZ7D7hQfgOWwcvpFehsPbM4BTzP2aMMf8='
        );