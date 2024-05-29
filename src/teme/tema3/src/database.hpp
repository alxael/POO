#include <map>
#include <any>
#include <ctime>
#include <chrono>
#include <vector>
#include <format>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <type_traits>

using namespace std;
using namespace bank;
using namespace spdlog;
using namespace exception;

namespace database
{
    template <typename KeyType, class Data>
    class Entity
    {
    protected:
        weak_ptr<pqxx::connection> connection;
        map<KeyType, Data> data;
        string table;

        inline static const string keyToString(KeyType key)
        {
            if (is_same<KeyType, long long>::value)
                return to_string(key);
            throw(KeyTypeUnsupportedException(typeid(KeyType).name()));
        }
        inline static const time_point<system_clock> stringToTimePoint(const string date, const string format = "%F %H:%M:%S") noexcept
        {
            tm timeStruct = {};
            istringstream ss(date);
            ss >> get_time(&timeStruct, format.c_str());
            return system_clock::from_time_t(mktime(&timeStruct));
        }

        virtual pair<KeyType, Data> parseData(pqxx::row row) const = 0;
        map<KeyType, Data> getRecords(Query &query) const
        {
            auto result = query.execute();
            map<KeyType, Data> dataResult;
            for (auto const &row : result)
                dataResult.insert(parseData(row));
            return dataResult;
        }

        Entity(weak_ptr<pqxx::connection> &connection, const string table) : connection(connection), table(table) {}
        ~Entity() = default;

        map<KeyType, Data> getAllRecords() const
        {
            Query query(connection, "SELECT * FROM :table;");
            query.setParameter<string>("table", table, false);
            return getRecords(query);
        }
        map<KeyType, Data> getRecordsByProperty(string property, string value) const
        {
            Query query(connection, "SELECT * FROM :table WHERE :property=:value;");
            query.setParameter<string>("table", table, false)
                .template setParameter<string>("property", property, false)
                .template setParameter<string>("value", value);
            return getRecords(query);
        }
        map<KeyType, Data> getRecordsByProperty(map<string, string> properties) const
        {
            Query query(connection, "SELECT * FROM :table WHERE ");
            query.setParameter<string>("table", table, false);
            long long index = 0;
            for (const auto &property : properties)
            {
                query.append(":property=:value ")
                    .setParameter<string>("property", property.first, false)
                    .setParameter<string>("value", property.second);
                index++;
                if (index < properties.size())
                    query.append(" AND ");
            }
            query.append(";");
            return getRecords(query);
        }
        pair<KeyType, Data> getRecordByProperty(string property, string value) const
        {
            auto result = getRecordsByProperty(property, value);
            if (result.size() > 1)
                throw(EntryDuplicateFoundException("Multiple entries for unique property " + property + " found in table " + table + "!"));
            if (result.empty())
                throw(EntryNotFoundException("Could not find entry with property " + property + " in table " + table + "!"));
            return *result.begin();
        }
        pair<KeyType, Data> getRecordByProperty(map<string, string> properties) const
        {
            auto result = getRecordsByProperty(properties);
            string propertiesDescription;
            for (auto const &property : properties)
                propertiesDescription += "(" + property.first + ", " + property.second + ")";
            if (result.size() > 1)
                throw(EntryDuplicateFoundException("Multiple entries found for " + propertiesDescription + "found in table " + table + "!"));
            if (result.empty())
                throw(EntryNotFoundException("Could not find entry for " + propertiesDescription + "in table " + table + "!"));
            return *result.begin();
        }
        void deleteRecordsByProperty(string property, string value)
        {
            Query query(connection, "DELETE FROM :table WHERE :property=:value;");
            query.setParameter<string>("table", table, false)
                .template setParameter<string>("property", property, false)
                .template setParameter<string>("value", value);
            query.execute();
            loadData();
        }

    public:
        void loadData() { data = getAllRecords(); }
        map<KeyType, Data> getData() const { return data; }
        virtual pair<KeyType, Data> getRecordById(KeyType id, bool cached = false) const
        {
            if (cached)
            {
                auto entry = data.find(id);
                if (entry == data.end())
                    throw(EntrySynchronizationException("Entry with ID " + keyToString(id) + " in table " + table + " is not synchronized!"));
                return *entry;
            }
            else
            {
                auto entry = getRecordByProperty("id", keyToString(id));
                return entry;
            }
        }
        virtual void deleteRecordById(KeyType id) { deleteRecordsByProperty("id", keyToString(id)); }
    };

    class CurrencyEntity : public Entity<long long, Currency>
    {
    private:
        pair<long long, Currency> parseData(pqxx::row row) const override
        {
            auto id = row[0].as<long long>();
            auto name = row[1].as<string>();
            auto code = row[2].as<string>();
            Currency currency(name, code);
            return pair<long long, Currency>(id, currency);
        }

    public:
        CurrencyEntity(weak_ptr<pqxx::connection> connection) : Entity::Entity(connection, "currencies") {}
        ~CurrencyEntity() = default;

        pair<long long, Currency> getRecordById(long long id, bool cached = true) const override { return Entity::getRecordById(id, cached); }
        pair<long long, Currency> getCurrencyFromCode(string code) const
        {
            auto result = getRecordByProperty("code", code);
            auto dataResult = data.find(result.first);
            return *dataResult;
        }
        vector<pair<string, string>> getCurrencyDisplayData() const
        {
            vector<pair<string, string>> result;
            for (const auto &currencyEntry : data)
                result.emplace_back(pair<string, string>(currencyEntry.second.getCode(), currencyEntry.second.getName()));
            return result;
        }
    };

    class ExchangeEntity : public Entity<long long, Exchange>
    {
    private:
        const CurrencyEntity &currencyEntity;

        pair<long long, Exchange> parseData(pqxx::row row) const override
        {
            auto id = row[0].as<long long>();
            auto sourceId = row[1].as<long long>();
            auto destinationId = row[2].as<long long>();
            auto rate = row[3].as<double>();

            auto source = currencyEntity.getRecordById(sourceId, true);
            auto destination = currencyEntity.getRecordById(destinationId, true);
            Exchange exchange(source.second, destination.second, rate);
            return pair<long long, Exchange>(id, exchange);
        }

    public:
        ExchangeEntity(weak_ptr<pqxx::connection> connection, CurrencyEntity &currencyEntity)
            : Entity::Entity(connection, "exchanges"),
              currencyEntity(currencyEntity) {}
        ~ExchangeEntity() = default;

        pair<long long, Exchange> getRecordById(long long id, bool cached = true) const override { return Entity::getRecordById(id, cached); }
        pair<long long, Exchange> getExchangeFromCurrencyCodes(string sourceCode, string destinationCode) const
        {
            auto source = currencyEntity.getCurrencyFromCode(sourceCode);
            auto destination = currencyEntity.getCurrencyFromCode(destinationCode);
            map<string, string> properties{{"source", to_string(source.first)}, {"destination", to_string(destination.first)}};
            return getRecordByProperty(properties);
        }
        vector<tuple<string, string, string>> getExchangeDisplayData() const
        {
            vector<tuple<string, string, string>> result;
            for (const auto &entry : data)
                if (entry.second.getSource() != entry.second.getDestination())
                    result.emplace_back(tuple<string, string, string>(entry.second.getSource().getCode(), entry.second.getDestination().getCode(), to_string(entry.second.getRate())));
            return result;
        }
    };

    class CountryEntity : public Entity<long long, Country>
    {
    private:
        pair<long long, Country> parseData(pqxx::row row) const override
        {
            auto id = row[0].as<long long>();
            auto name = row[1].as<string>();
            auto code = row[2].as<string>();
            auto pattern = row[3].as<string>();
            Country country(name, code, pattern);
            return pair<long long, Country>(id, country);
        }

    public:
        CountryEntity(weak_ptr<pqxx::connection> connection) : Entity::Entity(connection, "countries") {}
        ~CountryEntity() = default;

        vector<pair<string, string>> getCountryDisplayData() const
        {
            vector<pair<string, string>> result;
            for (const auto &entry : data)
                result.emplace_back(pair<string, string>(entry.second.getCode(), entry.second.getName()));
            return result;
        }
        pair<long long, Country> getCountryFromCode(string code) const { return getRecordByProperty("code", code); }
    };

    class UserEntity : public Entity<long long, User>
    {
    private:
        const CountryEntity &countryEntity;

        pair<long long, User> parseData(pqxx::row row) const override
        {
            auto id = row[0].as<long long>();
            auto countryId = row[1].as<long long>();
            auto email = row[2].as<string>();
            auto firstName = row[3].as<string>();
            auto lastName = row[4].as<string>();
            auto password = row[5].as<string>();

            auto country = countryEntity.getRecordById(countryId, true);
            User user(country.second, email, firstName, lastName, password);
            return (pair<long long, User>(id, user));
        }

    public:
        UserEntity(weak_ptr<pqxx::connection> connection, CountryEntity &countryEntity)
            : Entity::Entity(connection, "users"),
              countryEntity(countryEntity) {}
        ~UserEntity() = default;

        pair<long long, User> getUserFromEmail(string email) const { return getRecordByProperty("email", email); }
        pair<long long, User> createUser(string countryCode, string email, string firstName, string lastName, string password)
        {
            auto country = countryEntity.getCountryFromCode(countryCode);
            User user(email, firstName, lastName, password, country.second);

            Query query(connection, "INSERT INTO :table VALUES (DEFAULT, :country, :email, :firstName, :lastName, :password);");
            query.setParameter<string>("table", table, false)
                .setParameter<long long>("country", country.first)
                .setParameter<string>("email", email)
                .setParameter<string>("firstName", firstName)
                .setParameter<string>("lastName", lastName)
                .setParameter<string>("password", user.getPassword());
            query.execute();

            auto entry = getUserFromEmail(email);
            data.insert(entry);
            return entry;
        }
        void deleteRecordById(long long userId) override { Entity::deleteRecordById(userId); }
    };

    class TransactionEntity;

    class AccountEntity : public Entity<long long, Account>
    {
    private:
        TransactionEntity &transactionEntity;
        const CurrencyEntity &currencyEntity;
        const UserEntity &userEntity;

        pair<long long, Account> parseData(pqxx::row row) const override
        {
            auto id = row[0].as<long long>();
            auto currencyId = row[1].as<long long>();
            auto userId = row[2].as<long long>();
            auto IBAN = row[3].as<string>();
            auto amount = row[4].as<double>();
            auto firstName = row[5].as<string>();
            auto lastName = row[6].as<string>();

            auto currency = currencyEntity.getRecordById(currencyId, true);
            auto user = userEntity.getRecordById(userId, true);
            Account account(currency.second, user.second, IBAN, amount, firstName, lastName);
            return pair<long long, Account>(id, account);
        }

    public:
        AccountEntity(weak_ptr<pqxx::connection> connection, CurrencyEntity &currencyEntity, UserEntity &userEntity, TransactionEntity &transactionEntity)
            : Entity::Entity(connection, "accounts"),
              currencyEntity(currencyEntity), userEntity(userEntity), transactionEntity(transactionEntity) {}
        ~AccountEntity() = default;

        pair<long long, Account> getAccountFromIBAN(string IBAN) const { return getRecordByProperty("iban", IBAN); }
        map<long long, Account> getUserAccounts(long long userId) const { return getRecordsByProperty("associatedUser", Entity::keyToString(userId)); }
        pair<long long, Account> createAccount(string currencyCode, long long userId, string firstName, string lastName)
        {
            auto currency = currencyEntity.getCurrencyFromCode(currencyCode);
            auto user = userEntity.getRecordById(userId);
            Account account(currency.second, user.second, firstName, lastName);

            Query query(connection, "INSERT INTO :table VALUES (DEFAULT, :currency, :user, :iban, :amount, :firstName, :lastName);");
            query.setParameter<string>("table", table, false)
                .setParameter<long long>("currency", currency.first)
                .setParameter<long long>("user", user.first)
                .setParameter<string>("iban", account.getIBAN())
                .setParameter<double>("amount", account.getAmount())
                .setParameter<string>("firstName", firstName)
                .setParameter<string>("lastName", lastName);
            query.execute();

            auto entry = getAccountFromIBAN(account.getIBAN());
            data.insert(entry);
            return entry;
        }
        void updateAccountAmount(long long accountId, double newAmount)
        {
            Query query(connection, "UPDATE :table SET amount=:amount WHERE id=:id;");
            query.setParameter<string>("table", table, false)
                .setParameter<double>("amount", newAmount)
                .setParameter<long long>("id", accountId);
            query.execute();
            auto newAccount = getRecordById(accountId);
            data.erase(accountId);
            data.insert(newAccount);
        }
    };

    class TransactionEntity : public Entity<long long, Transaction>
    {
    private:
        AccountEntity &accountEntity;
        const ExchangeEntity &exchangeEntity;

        pair<long long, Transaction> parseData(pqxx::row row) const override
        {
            auto id = row[0].as<long long>();
            auto inboundId = row[1].as<long long>();
            auto outboundId = row[2].as<long long>();
            auto amount = row[3].as<double>();
            auto dateString = row[4].as<string>();

            auto date = stringToTimePoint(dateString);
            auto inbound = accountEntity.getRecordById(inboundId, true);
            auto outbound = accountEntity.getRecordById(outboundId, true);
            Transaction transaction(inbound.second, outbound.second, amount, date);
            return pair<long long, Transaction>(id, transaction);
        }

    public:
        TransactionEntity(weak_ptr<pqxx::connection> connection, AccountEntity &accountEntity, ExchangeEntity &exchangeEntity)
            : Entity::Entity(connection, "transactions"),
              accountEntity(accountEntity), exchangeEntity(exchangeEntity) {}
        ~TransactionEntity() = default;

        pair<long long, Transaction> createTransaction(long long userId, string inboundIBAN, string outboundIBAN, double amount)
        {
            auto inbound = accountEntity.getAccountFromIBAN(inboundIBAN);
            auto outbound = accountEntity.getAccountFromIBAN(outboundIBAN);

            auto userAccounts = accountEntity.getUserAccounts(userId);
            if (userAccounts.find(outbound.first) == userAccounts.end())
                throw(InvalidBusinessLogicException("You may only transfer money from your own account!"));

            auto inboundCurrency = inbound.second.getCurrency();
            auto outboundCurrency = outbound.second.getCurrency();
            auto exchange = exchangeEntity.getExchangeFromCurrencyCodes(outboundCurrency.getCode(), inboundCurrency.getCode());

            auto rate = exchange.second.getRate();
            auto newOutboundAmount = outbound.second.getAmount() - amount;
            if (newOutboundAmount < 0)
                throw(InvalidBusinessLogicException("Insufficient funds for transaction!"));
            auto newInboundAmount = inbound.second.getAmount() + amount * rate;

            accountEntity.updateAccountAmount(inbound.first, newInboundAmount);
            accountEntity.updateAccountAmount(outbound.first, newOutboundAmount);

            auto now = system_clock::now();
            string nowString = std::format("{:%F %T}", now);

            Query query(connection, "INSERT INTO :table VALUES (DEFAULT, :inbound, :outbound, :amount, :date);");
            query.setParameter<string>("table", table, false)
                .setParameter<long long>("inbound", inbound.first)
                .setParameter<long long>("outbound", outbound.first)
                .setParameter<double>("amount", amount)
                .setParameter<string>("date", nowString);
            query.execute();

            auto entry = getRecordByProperty("date", nowString);
            data.insert(entry);
            return entry;
        }
        pair<map<long long, Transaction>, map<long long, Transaction>> getAccountTransactions(long long accountId)
        {
            auto inboundTransactions = getRecordsByProperty("inbound", keyToString(accountId));
            auto outboundTransactions = getRecordsByProperty("outbound", keyToString(accountId));
            return make_pair(inboundTransactions, outboundTransactions);
        }
    };

    class AccountTransactionEntity : public AccountEntity, public TransactionEntity
    {
    public:
        AccountTransactionEntity(AccountEntity &accountEntity, TransactionEntity &transactionEntity) : AccountEntity(accountEntity), TransactionEntity(transactionEntity) {}

        void deleteRecordById(long long accountId) override
        {
            TransactionEntity::deleteRecordsByProperty("inbound", TransactionEntity::keyToString(accountId));
            TransactionEntity::deleteRecordsByProperty("outbound", TransactionEntity::keyToString(accountId));
            AccountEntity::deleteRecordById(accountId);
        }
    };

    class DatabaseManager
    {
    private:
        static shared_ptr<DatabaseManager> instance;
        static once_flag only_one;

        shared_ptr<pqxx::connection> connection;

        CurrencyEntity currencyEntity;
        ExchangeEntity exchangeEntity;
        CountryEntity countryEntity;
        UserEntity userEntity;
        AccountEntity accountEntity;
        TransactionEntity transactionEntity;
        AccountTransactionEntity accountTransactionEntity;

        // SQL utility functions
        void initializeDatabase(ifstream &file, char fileSeparator = ';')
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
                            Query query(connection, finalQuery);
                            query.execute();
                        }
                        catch (pqxx::sql_error const &sqlError)
                        {
                            error("SQL error: " + string(sqlError.what()));
                        }
                    }
            }
            catch (std::exception const &exception)
            {
                error("Unexpected exception: " + string(exception.what()));
            }
        }

    public:
        DatabaseManager(shared_ptr<pqxx::connection> connection, string initializationFilePath) : connection(connection),
                                                                                                  currencyEntity(connection), countryEntity(connection),
                                                                                                  exchangeEntity(connection, currencyEntity), userEntity(connection, countryEntity),
                                                                                                  accountEntity(connection, currencyEntity, userEntity, transactionEntity), transactionEntity(connection, accountEntity, exchangeEntity),
                                                                                                  accountTransactionEntity(accountEntity, transactionEntity)
        {
            info("Database connection opened.");

            // initialize database
            ifstream initializationFile;
            initializationFile.open(initializationFilePath, ios_base::in);
            initializeDatabase(initializationFile);
            initializationFile.close();

            currencyEntity.loadData();
            countryEntity.loadData();
            exchangeEntity.loadData();
            userEntity.loadData();
            accountEntity.loadData();
            transactionEntity.loadData();
        }
        ~DatabaseManager()
        {
            connection.get()->close();
            info("Database connection closed.");
        }

        DatabaseManager &operator=(const DatabaseManager &databaseManager)
        {
            if (this != &databaseManager)
                instance = databaseManager.instance;
            return *this;
        }

        static DatabaseManager &getInstance(shared_ptr<pqxx::connection> connection, string initializationFilePath)
        {
            call_once(
                DatabaseManager::only_one,
                [](shared_ptr<pqxx::connection> connection, string initializationFilePath)
                {
                    DatabaseManager::instance.reset(new DatabaseManager(connection, initializationFilePath));
                },
                connection, initializationFilePath);
            return *DatabaseManager::instance;
        }

        CurrencyEntity getCurrencyEntity() { return currencyEntity; }
        ExchangeEntity getExchangeEntity() { return exchangeEntity; }
        CountryEntity getCountryEntity() { return countryEntity; }
        UserEntity getUserEntity() { return userEntity; }
        AccountEntity getAccountEntity() { return accountEntity; }
        TransactionEntity getTransactionEntity() { return transactionEntity; }
        AccountTransactionEntity getAccountTransactionEntity() { return accountTransactionEntity; }
    };
}