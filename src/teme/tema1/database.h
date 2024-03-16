#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
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
        map<int, Exchange> exchanges;
        map<int, Country> countries;
        map<int, User> users;
        map<int, Account> accounts;
        map<int, Transaction> transactions;

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
        pair<int, Currency> getCurrencyEntryFromCode(string code)
        {
            for (auto it = currencies.begin(); it != currencies.end(); it++)
                if (!(it->second).getCode().compare(code))
                    return *it;
            throw(runtime_error("Currency with code '" + code + "' was not found."));
        }
        pair<int, Exchange> getExchangeEntryFromCurrencyCodes(string sourceCode, string destinationCode)
        {
            for (auto it = exchanges.begin(); it != exchanges.end(); it++)
                if (!(it->second).getSource().getCode().compare(sourceCode) && !(it->second).getDestination().getCode().compare(destinationCode))
                    return *it;
            throw(runtime_error("Exchange from " + sourceCode + " to " + destinationCode + " not found."));
        }
        pair<int, Account> getAccountEntryFromIBAN(string IBAN)
        {
            for (auto it = accounts.begin(); it != accounts.end(); it++)
                if (!(it->second).getIBAN().compare(IBAN))
                    return *it;
            throw(runtime_error("Could not find account with provided IBAN."));
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
        vector<pair<string, string>> getCurrencyData()
        {
            vector<pair<string, string>> result;
            for (auto it = currencies.begin(); it != currencies.end(); it++)
                result.emplace_back(pair<string, string>((it->second).getCode(), (it->second).getName()));
            return result;
        }

        void loadExchanges(string query = "SELECT * FROM Exchanges;")
        {
            pqxx::result result = work.exec(query);

            map<int, Exchange> queryResult;
            for (auto const &row : result)
            {
                int id = row[0].as<int>();
                int sourceId = row[1].as<int>();
                int destinationId = row[2].as<int>();
                double rate = row[3].as<double>();

                auto its = currencies.find(sourceId);
                if (its == currencies.end())
                    throw(runtime_error("Could not find currency with ID: " + to_string(sourceId)));
                Currency source = its->second;

                auto itd = currencies.find(destinationId);
                if (itd == currencies.end())
                    throw(runtime_error("Could not find currency with ID: " + to_string(destinationId)));
                Currency destination = itd->second;

                Exchange exchange(its->second, itd->second, rate);
                queryResult.insert(pair<int, Exchange>(id, exchange));
            }

            exchanges = queryResult;
        }
        map<int, Exchange> getExchanges() { return exchanges; }

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

                User user(it->second, email, firstName, lastName, password);
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

        void loadAccounts(string query = "SELECT * FROM Accounts;")
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

                auto itu = users.find(userId);
                if (itu == users.end())
                    throw(runtime_error("Could not find user with ID: " + to_string(userId)));

                Account account((itc->second), (itu->second), IBAN, amount, firstName, lastName);
                queryResult.insert(pair<int, Account>(id, account));
            }

            accounts = queryResult;
        }
        map<int, Account> getAccounts() { return accounts; }
        Account createAccount(string currencyCode, User user, string firstName, string lastName)
        {
            pair<int, Currency> currencyEntry = getCurrencyEntryFromCode(currencyCode);
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
        Account getAccountByIBAN(string IBAN)
        {
            for (auto it = accounts.begin(); it != accounts.end(); it++)
                if (!(it->second).getIBAN().compare(IBAN))
                    return (it->second);
            throw(runtime_error("Could not find account with provided IBAN."));
        }
        void updateAccount(string IBAN, double newAmount)
        {
            pair<int, Account> accountEntry = getAccountEntryFromIBAN(IBAN);

            string query = "UPDATE Accounts SET amount=" + to_string(newAmount) + " WHERE iban=" + queryString(IBAN) + ";";
            work.exec(query);

            accounts[accountEntry.first].setAmount(newAmount);
        }
        vector<Account> getUserAccounts(User user)
        {
            loadAccounts();

            vector<Account> result;
            for (auto it = accounts.begin(); it != accounts.end(); it++)
                if ((it->second).getUser() == user)
                    result.emplace_back(it->second);

            return result;
        }
        void deleteAccount(string IBAN, User user)
        {
            Account account = getAccountByIBAN(IBAN);
            if (account.getUser() != user)
                throw(runtime_error("You may only delete your own account."));
            string query = "DELETE FROM Accounts WHERE iban=" + queryString(IBAN) + ";";
            work.exec(query);
            loadAccounts();
        }

        void loadTransactions(string query = "SELECT * FROM Transactions;")
        {
            pqxx::result result = work.exec(query);

            map<int, Transaction> queryResult;
            for (auto const &row : result)
            {
                int id = row[0].as<int>();
                int inboundId = row[1].as<int>();
                int outboundId = row[2].as<int>();
                double amount = row[3].as<double>();

                auto iti = accounts.find(inboundId);
                if (iti == accounts.end())
                    throw(runtime_error("Could not find inbound account with ID: " + to_string(inboundId)));

                auto ito = accounts.find(outboundId);
                if (iti == accounts.end())
                    throw(runtime_error("Could not find outbound account with ID: " + to_string(outboundId)));

                Transaction transaction(iti->second, ito->second, amount);
                queryResult.insert(pair<int, Transaction>(id, transaction));
            }

            transactions = queryResult;
        }
        map<int, Transaction> getTransactions() { return transactions; }
        void createTransaction(User user, string inboundIBAN, string outboundIBAN, double amount)
        {
            pair<int, Account> inboundEntry = getAccountEntryFromIBAN(inboundIBAN);
            pair<int, Account> outboundEntry = getAccountEntryFromIBAN(outboundIBAN);

            vector<Account> userAccounts = getUserAccounts(user);
            if (find(userAccounts.begin(), userAccounts.end(), outboundEntry.second) == userAccounts.end())
                throw(runtime_error("You may only transfer money from your own account."));

            // outbound = source, inbound = destination
            pair<int, Exchange> exchangeEntry = getExchangeEntryFromCurrencyCodes(outboundEntry.second.getCurrency().getCode(), inboundEntry.second.getCurrency().getCode());

            double rate = exchangeEntry.second.getRate();

            double newOutboundAmount = outboundEntry.second.getAmount() - amount;
            if (newOutboundAmount < 0)
                throw(runtime_error("Insufficient funds for transaction."));
            double newInboundAmount = inboundEntry.second.getAmount() + amount * rate;

            updateAccount(inboundIBAN, newInboundAmount);
            updateAccount(outboundIBAN, newOutboundAmount);

            string query = "INSERT INTO Transactions VALUES (" +
                           to_string(transactions.size()) + "," +
                           to_string(inboundEntry.first) + "," +
                           to_string(outboundEntry.first) + "," +
                           to_string(amount) + ");";

            work.exec(query);
            loadTransactions();
        }
        vector<Transaction> getAccountTransactions(Account account)
        {
            vector<Transaction> result;
            for (auto it = transactions.begin(); it != transactions.end(); it++)
                if ((it->second).getInbound() == account || (it->second).getOutbound() == account)
                    result.emplace_back((it->second));
            return result;
        }
    };
};