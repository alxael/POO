#include <string>
#include <pqxx/pqxx>

using namespace std;
using namespace spdlog;

namespace exception
{
    class Exception
    {
    protected:
        string message;

    public:
        explicit Exception(const char *message) : message(message) {}
        explicit Exception(const string &message) : message(message) {}
        virtual ~Exception() noexcept {}

        virtual string what() const noexcept { return message; }
    };

    class EntryNotFoundException : public Exception
    {
    public:
        EntryNotFoundException() : Exception("Requested resource not found!") {}
        EntryNotFoundException(string message) : Exception(message) { warn(message); }
        EntryNotFoundException(string query, string message) : Exception(message)
        {
            error("Query " + query + " returned an empty result!");
            warn(message);
        }
    };

    class EntryDuplicateFoundException : public Exception
    {
    public:
        EntryDuplicateFoundException() : Exception("Unique resource duplicate found!") {}
        EntryDuplicateFoundException(string message) : Exception(message) { warn(message); }
        EntryDuplicateFoundException(string query, string message) : Exception(message)
        {
            error("Query " + query + " returned multiple entries, when identifier should be unique.");
            warn(message);
        }
    };

    class EntrySynchronizationException : public Exception
    {
    public:
        EntrySynchronizationException() : Exception("Local resource is not synchronized with remote resource!") {}
        EntrySynchronizationException(string message) : Exception(message) { warn(message); }
        EntrySynchronizationException(string query, string message) : Exception(message)
        {
            error("Query " + query + " result is not synchronized with local data.");
            warn(message);
        }
    };

    class KeyTypeUnsupportedException : public Exception
    {
    public:
        KeyTypeUnsupportedException() : Exception("Unsupported primary key type detected!") {}
        KeyTypeUnsupportedException(string type) : Exception("Unsupported primary key type " + type + " detected!") { warn(message); }
    };

    class InvalidBusinessLogicException : public Exception
    {
    public:
        InvalidBusinessLogicException() : Exception("Invalids business logic!") {}
        InvalidBusinessLogicException(string message) : Exception(message) {}
    };

    class ValidationException : public Exception
    {
    public:
        ValidationException() : Exception("Invalid input!") {}
        ValidationException(string message) : Exception(message) {}
    };

    class UserQuitException : public Exception
    {
    public:
        UserQuitException() : Exception("User quit!") {}
    };
};