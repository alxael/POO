#include <map>
#include <vector>
#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace bank;
using namespace spdlog;

namespace database
{
    class DatabaseManager
    {
    private:
        pqxx::connection connection;
        pqxx::nontransaction work;

        map<int, Currency> currencies;
        map<int, Country> countries;
        map<int, User> users;
        map<int, Account> accounts;

        string queryString(string input) { return "'" + input + "'"; }

        pair<int, User> getUserEntryFromEmail(string email)
        {
            for (auto it = users.begin(); it != users.end(); it++)
                if (!(it->second).getEmail().compare(email))
                    return *it;
            throw(runtime_error("User with email '" + email + "' was not found."));
        }
        pair<int, Country> getCountryEntryFromCode(string code)
        {
            for (auto it = countries.begin(); it != countries.end(); it++)
                if (!(it->second).getCode().compare(code))
                    return *it;
            throw(runtime_error("Country with code '" + code + "' was not found."));
        }
        pair<int, Currency> getCurrencyFromCode(string code)
        {
            for (auto it = currencies.begin(); it != currencies.end(); it++)
                if (!(it->second).getCode().compare(code))
                    return *it;
            throw(runtime_error("Currency with code '" + code + "' was not found."));
        }

    public:
        DatabaseManager(string connectionString) : connection{connectionString}, work{connection}
        {
            info("Database connection opened.");
        }
        ~DatabaseManager()
        {
            connection.close();
            info("Database connection closed.");
        }

        void executeQuery(string fileName, char fileSeparator = ';')
        {
            ifstream in;
            in.open(fileName, ios_base::in);
            try
            {
                vector<string> queries;
                for (string query; getline(in, query, fileSeparator);)
                    if (!query.empty())
                    {
                        try
                        {
                            string finalQuery = query + fileSeparator;
                            work.exec(finalQuery);
                        }
                        catch (pqxx::sql_error const &sqlError)
                        {
                            error("SQL error: " + string(sqlError.what()));
                        }
                    }
            }

            catch (exception const &exception)
            {
                error("Unexpected exception: " + string(exception.what()));
            }
            in.close();
        }

        void loadCurrencies(string query = "SELECT * FROM Currencies;")
        {
            pqxx::result result = work.exec(query);

            map<int, Currency> queryResult;
            for (auto const &row : result)
            {
                int id = row[0].as<int>();
                string name = row[1].as<string>();
                string code = row[2].as<string>();
                Currency currency(name, code);

                queryResult.insert(pair<int, Currency>(id, currency));
            }

            currencies = queryResult;
        }
        map<int, Currency> getCurrencies() { return currencies; }
        vector<pair<string,string>> getCurrencyData()
        {
            vector<pair<string,string>> result;
            for (auto it = currencies.begin(); it != currencies.end(); it++)
                result.emplace_back(pair<string,string>((it->second).getCode(), (it->second).getName()));
            return result;
        }

        void loadCountries(string query = "SELECT * FROM Countries;")
        {
            pqxx::result result = work.exec(query);

            map<int, Country> queryResult;
            for (auto const &row : result)
            {
                int id = row[0].as<int>();
                string name = row[1].as<string>();
                string code = row[2].as<string>();
                string pattern = row[3].as<string>();
                Country country(name, code, pattern);
                queryResult.insert(pair<int, Country>(id, country));
            }

            countries = queryResult;
        }
        map<int, Country> getCountries() { return countries; }
        vector<pair<string, string>> getCountryCodeData()
        {
            vector<pair<string, string>> result;
            for (auto it = countries.begin(); it != countries.end(); it++)
                result.emplace_back(pair<string, string>((it->second).getCode(), (it->second).getName()));
            return result;
        }

        void loadUsers(string query = "SELECT * FROM Users;")
        {
            pqxx::result result = work.exec(query);

            map<int, User> queryResult;
            for (auto const &row : result)
            {
                int id = row[0].as<int>();
                int countryId = row[1].as<int>();
                string email = row[2].as<string>();
                string firstName = row[3].as<string>();
                string lastName = row[4].as<string>();
                string password = row[5].as<string>();

                auto it = countries.find(countryId);
                if (it == countries.end())
                    throw(runtime_error("Could not find country with ID: " + to_string(countryId)));
                Country country = (it->second);

                User user(country, email, firstName, lastName, password);
                queryResult.insert(pair<int, User>(id, user));
            }

            users = queryResult;
        }
        map<int, User> getUsers() { return users; }
        User createUser(string countryCode, string email, string firstName, string lastName, string password)
        {
            pair<int, Country> countryEntry = getCountryEntryFromCode(countryCode);
            User user(email, firstName, lastName, password, countryEntry.second);

            string query = "INSERT INTO Users VALUES (" +
                           to_string(users.size()) + "," +
                           to_string(countryEntry.first) + "," +
                           queryString(email) + "," +
                           queryString(firstName) + "," +
                           queryString(lastName) + "," +
                           queryString(user.getPassword()) + ");";

            work.exec(query);
            loadUsers();
            return user;
        }
        User getUserByEmail(string email)
        {
            pair<int, User> userEntry = getUserEntryFromEmail(email);
            return userEntry.second;
        }
        void deleteUser(User user)
        {
            vector<Account> userAccounts = getUserAccounts(user);
            for (auto it = userAccounts.begin(); it != userAccounts.end(); it++)
                deleteAccount((*it).getIBAN(), user);
            string query = "DELETE FROM Users WHERE email=" + queryString(user.getEmail()) + ";";
            work.exec(query);
            loadUsers();
        }

        void loadAccounts(string query = "SELECT * from Accounts;")
        {
            pqxx::result result = work.exec(query);

            map<int, Account> queryResult;
            for (auto const &row : result)
            {
                int id = row[0].as<int>();
                int currencyId = row[1].as<int>();
                int userId = row[2].as<int>();
                string IBAN = row[3].as<string>();
                double amount = row[4].as<double>();
                string firstName = row[5].as<string>();
                string lastName = row[6].as<string>();

                auto itc = currencies.find(currencyId);
                if (itc == currencies.end())
                    throw(runtime_error("Could not find currency with ID: " + to_string(currencyId)));
                Currency currency = (itc->second);

                auto itu = users.find(userId);
                if (itu == users.end())
                    throw(runtime_error("Could not find user with ID: " + to_string(userId)));
                User user = (itu->second);

                Account account(currency, user, IBAN, amount, firstName, lastName);
                queryResult.insert(pair<int, Account>(id, account));
            }

            accounts = queryResult;
        }
        map<int, Account> getAccounts() { return accounts; }
        Account createAccount(string currencyCode, User user, string firstName, string lastName)
        {
            pair<int, Currency> currencyEntry = getCurrencyFromCode(currencyCode);
            Account account(currencyEntry.second, user, firstName, lastName);
            pair<int, User> userEntry = getUserEntryFromEmail(user.getEmail());

            string query = "INSERT INTO Accounts VALUES (" +
                           to_string(accounts.size()) + "," +
                           to_string(currencyEntry.first) + "," +
                           to_string(userEntry.first) + "," +
                           queryString(account.getIBAN()) + "," +
                           to_string(account.getAmount()) + "," +
                           queryString(firstName) + "," +
                           queryString(lastName) + ");";

            work.exec(query);
            loadAccounts();
            return account;
        }
        Account findAccountByIBAN(string IBAN)
        {
            for (auto it = accounts.begin(); it != accounts.end(); it++)
                if (!(it->second).getIBAN().compare(IBAN))
                    return (it->second);
            throw(runtime_error("Could not find account with provided IBAN."));
        }
        vector<Account> getUserAccounts(User user)
        {
            vector<Account> result;
            for (auto it = accounts.begin(); it != accounts.end(); it++)
                if ((it->second).getUser() == user)
                    result.emplace_back(it->second);

            return result;
        }
        void deleteAccount(string IBAN, User user)
        {
            Account account = findAccountByIBAN(IBAN);
            if (account.getUser() != user)
                throw(runtime_error("You may only delete your own account."));
            string query = "DELETE FROM Accounts WHERE iban=" + queryString(IBAN) + ";";
            work.exec(query);
            loadAccounts();
        }
    };
};