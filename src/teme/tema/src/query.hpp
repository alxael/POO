#include <string>
#include <type_traits>
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

using namespace std;
using namespace pqxx;
using namespace spdlog;

namespace database
{
    class Query
    {
    private:
        weak_ptr<pqxx::connection> connection;
        string query;

    public:
        Query(weak_ptr<pqxx::connection> connection) : connection(connection) {}
        Query(weak_ptr<pqxx::connection> connection, string query) : connection(connection), query(query) {}
        ~Query() = default;

        string getQuery() const noexcept { return query; }
        void setQuery(string query) noexcept { query = query; }

        template <typename ArgumentType>
        Query &setParameter(string identifier, ArgumentType argument, bool addQuotes = true)
        {
            string argumentString;
            if (is_same<ArgumentType, string>::value)
                argumentString = argument;
            if (is_same<ArgumentType, int>::value)
                argumentString = to_string(argument);
            if (is_same<ArgumentType, long long>::value)
                argumentString = to_string(argument);
            if (is_same<ArgumentType, double>::value)
                argumentString = to_string(argument);
            if (argumentString == "")
                throw(UnsupportedArgumentTypeException());

            string identifierString = ":" + identifier;
            string parameterString = addQuotes ? ('\'' + argumentString + '\'') : argumentString;
            auto position = query.find(identifierString);
            if (position == string::npos)
                throw(IdentifierNotFoundException());
            query.replace(position, identifierString.length(), parameterString);

            return *this;
        }
        Query &append(string value)
        {
            query += value;
            return *this;
        }

        result execute() const
        {
            shared_ptr<pqxx::connection> connectionPointer = connection.lock();
            work work(*connectionPointer.get());
            result result = work.exec(query);
            info("Executing query: " + query);
            work.commit();
            return result;
        }
    };

};