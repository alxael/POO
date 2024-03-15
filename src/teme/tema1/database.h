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

        map<int, Country> countries;
        map<int, User> users;

        string queryString(string input) { return "'" + input + "'"; }

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

        void loadCountries(string query = "SELECT * FROM Countries;")
        {
            pqxx::result result = work.exec(query);

            map<int, Country> queryResult;
            for (auto const &row : result)
            {
                int id = stoi(row[0].c_str());
                string name(row[1].c_str());
                string code(row[2].c_str());
                string pattern(row[3].c_str());
                Country country(name, code, pattern);
                queryResult.insert(pair<int, Country>(id, country));
            }

            countries = queryResult;
        }
        map<int, Country> getCountries() { return countries; }
        pair<int, Country> getCountryFromCode(string code)
        {
            for (auto it = countries.begin(); it != countries.end(); it++)
                if ((it->second).getCode().compare(code) == 0)
                    return *it;
            throw(runtime_error("Country with code '" + code + "' was not found."));
        }
        vector<pair<string, string>> getCountryDisplayData()
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
                int id = stoi(row[0].c_str());
                int countryId = stoi(row[1].c_str());
                string email(row[2].c_str());
                string firstName(row[3].c_str());
                string lastName(row[4].c_str());
                string password(row[5].c_str());

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
            pair<int, Country> countryEntry = getCountryFromCode(countryCode);
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
        User findUserByEmail(string email)
        {
            for (auto it = users.begin(); it != users.end(); it++)
                if (!(it->second).getEmail().compare(email))
                    return (it->second);
            throw(runtime_error("Could not find user with provided email."));
        }
        void deleteUser(User user)  
        {
            string query = "DELETE FROM Users WHERE email=" + queryString(user.getEmail()) + ";";
            work.exec(query);
            loadUsers();
        }
    };
};