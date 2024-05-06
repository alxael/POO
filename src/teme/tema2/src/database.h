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

        // SQL utility functions
        inline string queryString(string input) { return "'" + input + "'"; }
        void executeQuery(ifstream &file, char fileSeparator = ';')
        {
            try
            {
                vector<string> queries;
                for (string query; getline(file, query, fileSeparator);)
                    if (!query.empty())
                    {
                        try
                        {
                            string finalQuery = query + fileSeparator;
                            info(finalQuery);
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
        }
        pair<int, Currency> parseCurrency(pqxx::row row)
        {
            int id = row[0].as<int>();
            string name = row[1].as<string>();
            string code = row[2].as<string>();
            Currency currency(name, code);
            return pair<int, Currency>(id, currency);
        }
        pair<int, Exchange> parseExchange(pqxx::row row)
        {
            int id = row[0].as<int>();
            int sourceId = row[1].as<int>();
            int destinationId = row[2].as<int>();
            double rate = row[3].as<double>();

            auto its = currencies.find(sourceId);
            if (its == currencies.end())
                throw(runtime_error("Could not find currency with ID: " + to_string(sourceId)));

            auto itd = currencies.find(destinationId);
            if (itd == currencies.end())
                throw(runtime_error("Could not find currency with ID: " + to_string(destinationId)));

            Exchange exchange(&its->second, &itd->second, rate);

            return pair<int, Exchange>(id, exchange);
        }
        pair<int, Country> parseCountry(pqxx::row row)
        {
            int id = row[0].as<int>();
            string name = row[1].as<string>();
            string code = row[2].as<string>();
            string pattern = row[3].as<string>();
            Country country(name, code, pattern);
            return pair<int, Country>(id, country);
        }
        pair<int, User> parseUser(pqxx::row row)
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

            User user(&it->second, email, firstName, lastName, password);

            return (pair<int, User>(id, user));
        }
        pair<int, Account> parseAccount(pqxx::row row)
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

            Account account(&itc->second, &itu->second, IBAN, amount, firstName, lastName);
            return pair<int, Account>(id, account);
        }
        pair<int, Transaction> parseTransaction(pqxx::row row)
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

            Transaction transaction(&iti->second, &ito->second, amount);
            return pair<int, Transaction>(id, transaction);
        }

    public:
        DatabaseManager(string connectionString, string initializationFilePath) : connection{connectionString}, work{connection}
        {
            info("Database connection opened.");

            // initialize database
            ifstream initializationFile;
            initializationFile.open(initializationFilePath, ios_base::in);
            executeQuery(initializationFile);
            initializationFile.close();

            // load all data
            loadCurrencies();
            loadExchanges();
            loadCountries();
            loadUsers();
            loadAccounts();
            loadTransactions();
        }
        ~DatabaseManager()
        {
            connection.close();
            info("Database connection closed.");
        }

        // Currencies
        void loadCurrencies(string query = "SELECT * FROM Currencies;")
        {
            pqxx::result result = work.exec(query);

            map<int, Currency> queryResult;
            for (auto const &row : result)
                queryResult.insert(parseCurrency(row));

            currencies = queryResult;
        }
        map<int, Currency> getCurrencies() { return currencies; }
        pair<int, Currency> getCurrencyFromCode(string code)
        {
            string query = "SELECT * FROM Currencies WHERE code=" + queryString(code) + ";";
            pqxx::result result = work.exec(query);

            if (result.size() > 1)
                throw(runtime_error("Multiple currencies with code '" + code + "' found!"));
            if (result.empty())
                throw(runtime_error("No currency with code '" + code + "' was found!"));

            return parseCurrency(result[0]);
        }
        vector<pair<string, string>> getCurrencyDisplayData()
        {
            vector<pair<string, string>> result;
            for (auto it = currencies.begin(); it != currencies.end(); it++)
                result.emplace_back(pair<string, string>((it->second).getCode(), (it->second).getName()));
            return result;
        }

        // Exchanges
        void loadExchanges(string query = "SELECT * FROM Exchanges;")
        {
            pqxx::result result = work.exec(query);
            map<int, Exchange> queryResult;
            for (auto const &row : result)
                queryResult.insert(parseExchange(row));
            exchanges = queryResult;
        }
        map<int, Exchange> getExchanges() { return exchanges; }
        pair<int, Exchange> getExchangeFromCurrencyCodes(string sourceCode, string destinationCode)
        {
            pair<int, Currency> sourceCurrency = getCurrencyFromCode(sourceCode);
            pair<int, Currency> destinationCurrency = getCurrencyFromCode(destinationCode);

            string query = "SELECT * FROM Exchanges WHERE source=" + to_string(sourceCurrency.first) + " AND destination=" + to_string(destinationCurrency.first) + ";";
            pqxx::result result = work.exec(query);

            if (result.size() > 1)
                throw(runtime_error("Multiple exchanges with currencies '" + sourceCode + "' and '" + destinationCode + "' found!"));
            if (result.empty())
                throw(runtime_error("No exchange with currencies '" + sourceCode + "' and '" + destinationCode + "' was found!"));

            return parseExchange(result[0]);
        }
        vector<tuple<string, string, string>> getExchangeDisplayData()
        {
            vector<tuple<string, string, string>> result;
            for (auto it = exchanges.begin(); it != exchanges.end(); it++)
                if ((it->second).getSource() != (it->second).getDestination())
                    result.emplace_back(tuple<string, string, string>((it->second).getSource().getCode(), (it->second).getDestination().getCode(), to_string((it->second).getRate())));

            return result;
        }

        // Countries
        void loadCountries(string query = "SELECT * FROM Countries;")
        {
            pqxx::result result = work.exec(query);

            map<int, Country> queryResult;
            for (auto const &row : result)
                queryResult.insert(parseCountry(row));

            countries = queryResult;
        }
        map<int, Country> getCountries() { return countries; }
        vector<pair<string, string>> getCountryDisplayData()
        {
            vector<pair<string, string>> result;
            for (auto it = countries.begin(); it != countries.end(); it++)
                result.emplace_back(pair<string, string>((it->second).getCode(), (it->second).getName()));

            return result;
        }
        pair<int, Country> getCountryFromCode(string code)
        {
            string query = "SELECT * FROM Countries WHERE code=" + queryString(code) + ";";
            pqxx::result result = work.exec(query);

            if (result.size() > 1)
                throw(runtime_error("Multiple countries with code '" + code + "' found!"));
            if (result.empty())
                throw(runtime_error("No country with code '" + code + "' was found!"));

            return parseCountry(result[0]);
        }

        // Users
        void loadUsers(string query = "SELECT * FROM Users;")
        {
            pqxx::result result = work.exec(query);

            map<int, User> queryResult;
            for (auto const &row : result)
                queryResult.insert(parseUser(row));

            users = queryResult;
        }
        map<int, User> getUsers() { return users; }
        pair<int, User> getUserFromEmail(string email)
        {
            string query = "SELECT * FROM Users WHERE email=" + queryString(email) + ";";
            pqxx::result result = work.exec(query);

            if (result.size() > 1)
                throw(runtime_error("Multiple users with email '" + email + "' found!"));
            if (result.empty())
                throw(runtime_error("No user with email '" + email + "' was found!"));

            return parseUser(result[0]);
        }
        pair<int, User> createUser(string countryCode, string email, string firstName, string lastName, string password)
        {
            pair<int, Country> countryEntry = getCountryFromCode(countryCode);
            User user(email, firstName, lastName, password, &countryEntry.second);

            string query = "INSERT INTO Users VALUES (" +
                           to_string(users.size()) + "," +
                           to_string(countryEntry.first) + "," +
                           queryString(email) + "," +
                           queryString(firstName) + "," +
                           queryString(lastName) + "," +
                           queryString(user.getPassword()) + ");";
            work.exec(query);

            pair<int, User> entry(users.size(), user);
            users.insert(entry);
            return entry;
        }
        void deleteUser(pair<int, User> user)
        {
            vector<pair<int, Account>> userAccounts = getUserAccounts(user);
            for (auto it = userAccounts.begin(); it != userAccounts.end(); it++)
                deleteAccount((it->second).getIBAN(), user);

            string query = "DELETE FROM Users WHERE id=" + to_string(user.first) + ";";
            work.exec(query);

            users.erase(user.first);
        }

        // Accounts
        void loadAccounts(string query = "SELECT * FROM Accounts;")
        {
            pqxx::result result = work.exec(query);

            map<int, Account> queryResult;
            for (auto const &row : result)
                queryResult.insert(parseAccount(row));

            accounts = queryResult;
        }
        map<int, Account> getAccounts() { return accounts; }
        pair<int, Account> getAccountFromIBAN(string IBAN)
        {
            string query = "SELECT * FROM Accounts WHERE iban=" + queryString(IBAN) + ";";
            pqxx::result result = work.exec(query);

            if (result.size() > 1)
                throw(runtime_error("Multiple accounts with IBAN '" + IBAN + "' found!"));
            if (result.empty())
                throw(runtime_error("No account with IBAN '" + IBAN + "' was found!"));

            return parseAccount(result[0]);
        }
        vector<pair<int, Account>> getUserAccounts(pair<int, User> user)
        {
            string query = "SELECT * FROM Accounts WHERE associatedUser=" + to_string(user.first) + ";";
            pqxx::result result = work.exec(query);

            vector<pair<int, Account>> accounts;
            for (const auto &row : result)
                accounts.emplace_back(parseAccount(row));

            return accounts;
        }
        pair<int, Account> createAccount(string currencyCode, pair<int, User> user, string firstName, string lastName)
        {
            pair<int, Currency> currency = getCurrencyFromCode(currencyCode);
            Account account(&currency.second, &user.second, firstName, lastName);

            string query = "INSERT INTO Accounts VALUES (" +
                           to_string(accounts.size()) + "," +
                           to_string(currency.first) + "," +
                           to_string(user.first) + "," +
                           queryString(account.getIBAN()) + "," +
                           to_string(account.getAmount()) + "," +
                           queryString(firstName) + "," +
                           queryString(lastName) + ");";
            work.exec(query);

            pair<int, Account> entry(accounts.size(), account);
            accounts.insert(entry);
            return entry;
        }
        void updateAccountAmount(pair<int, Account> account, double newAmount)
        {
            string query = "UPDATE Accounts SET amount=" + to_string(newAmount) + " WHERE id=" + to_string(account.first) + ";";
            work.exec(query);

            accounts[account.first].setAmount(newAmount);
        }
        void deleteAccount(string IBAN, pair<int, User> authenticatedUser)
        {
            pair<int, Account> account = getAccountFromIBAN(IBAN);
            if (account.second.getUser() != authenticatedUser.second)
                throw(runtime_error("You may only delete your own account."));

            // delete all transactions associated with the account
            string transactionsQuery = "DELETE FROM Transactions WHERE inbound=" + to_string(account.first) + " OR " + "outbound=" + to_string(account.first) + ";";
            work.exec(transactionsQuery);
            loadTransactions();

            string accountsQuery = "DELETE FROM Accounts WHERE id=" + to_string(account.first) + ";";
            work.exec(accountsQuery);
            accounts.erase(account.first);
        }

        // Transactions
        void loadTransactions(string query = "SELECT * FROM Transactions;")
        {
            pqxx::result result = work.exec(query);

            map<int, Transaction> queryResult;
            for (auto const &row : result)
                queryResult.insert(parseTransaction(row));

            transactions = queryResult;
        }
        map<int, Transaction> getTransactions() { return transactions; }
        void createTransaction(pair<int, User> authenticatedUser, string inboundIBAN, string outboundIBAN, double amount)
        {
            pair<int, Account> inboundAccount = getAccountFromIBAN(inboundIBAN);
            pair<int, Account> outboundAccount = getAccountFromIBAN(outboundIBAN);

            vector<pair<int, Account>> userAccounts = getUserAccounts(authenticatedUser);
            if (find(userAccounts.begin(), userAccounts.end(), outboundAccount) == userAccounts.end())
                throw(runtime_error("You may only transfer money from your own account."));

            pair<int, Exchange> exchange = getExchangeFromCurrencyCodes(outboundAccount.second.getCurrency().getCode(), inboundAccount.second.getCurrency().getCode());

            // subtract money from outbound, add to inbound
            double rate = exchange.second.getRate();
            double newOutboundAmount = outboundAccount.second.getAmount() - amount;
            if (newOutboundAmount < 0)
                throw(runtime_error("Insufficient funds for transaction."));
            double newInboundAmount = inboundAccount.second.getAmount() + amount * rate;

            updateAccountAmount(inboundAccount, newInboundAmount);
            updateAccountAmount(outboundAccount, newOutboundAmount);

            string query = "INSERT INTO Transactions VALUES (" +
                           to_string(transactions.size()) + "," +
                           to_string(inboundAccount.first) + "," +
                           to_string(outboundAccount.first) + "," +
                           to_string(amount) + ");";
            work.exec(query);

            Transaction transaction(&inboundAccount.second, &outboundAccount.second, amount);
            transactions.insert(pair<int, Transaction>(transactions.size(), transaction));
        }
        vector<pair<int, Transaction>> getAccountTransactions(pair<int, Account> account)
        {
            string query = "SELECT * FROM Transactions WHERE inbound=" + to_string(account.first) + " OR outbound=" + to_string(account.first) + ";";
            pqxx::result result = work.exec(query);

            vector<pair<int, Transaction>> transactions;
            for (const auto &row : result)
                transactions.emplace_back(parseTransaction(row));

            return transactions;
        }
    };
}