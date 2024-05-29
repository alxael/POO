#include <string>
#include <pqxx/pqxx>

using namespace std;
using namespace spdlog;

namespace exception
{
    class UnsupportedArgumentTypeException : public runtime_error
    {
    public:
        UnsupportedArgumentTypeException() : runtime_error("Unsupported argument type!") {}
        UnsupportedArgumentTypeException(string message) : runtime_error(message) {}
    };

    class EntryNotFoundException : public logic_error
    {
    public:
        EntryNotFoundException() : logic_error("Requested resource not found!") {}
        EntryNotFoundException(string message) : logic_error(message) { warn(message); }
        EntryNotFoundException(string query, string message) : logic_error(message)
        {
            error("Query " + query + " returned an empty result!");
            warn(message);
        }
    };

    class EntryDuplicateFoundException : public logic_error
    {
    public:
        EntryDuplicateFoundException() : logic_error("Unique resource duplicate found!") {}
        EntryDuplicateFoundException(string message) : logic_error(message) { warn(message); }
        EntryDuplicateFoundException(string query, string message) : logic_error(message)
        {
            error("Query " + query + " returned multiple entries, when identifier should be unique.");
            warn(message);
        }
    };

    class EntrySynchronizationException : public logic_error
    {
    public:
        EntrySynchronizationException() : logic_error("Local resource is not synchronized with remote resource!") {}
        EntrySynchronizationException(string message) : logic_error(message) { warn(message); }
        EntrySynchronizationException(string query, string message) : logic_error(message)
        {
            error("Query " + query + " result is not synchronized with local data.");
            warn(message);
        }
    };

    class KeyTypeUnsupportedException : public runtime_error
    {
    public:
        KeyTypeUnsupportedException() : runtime_error("Unsupported primary key type detected!") {}
        KeyTypeUnsupportedException(string type) : runtime_error("Unsupported primary key type " + type + " detected!") { warn(type); }
    };

    class InvalidBusinessLogicException : public logic_error
    {
    public:
        InvalidBusinessLogicException() : logic_error("Invalids business logic!") {}
        InvalidBusinessLogicException(string message) : logic_error(message) {}
    };

    class ValidationException : public logic_error
    {
    public:
        ValidationException() : logic_error("Invalid input!") {}
        ValidationException(string message) : logic_error(message) {}
    };

    class UserQuitException : public logic_error
    {
    public:
        UserQuitException() : logic_error("User quit!") {}
    };

    class IdentifierNotFoundException : public logic_error
    {
    public:
        using logic_error::logic_error;
        IdentifierNotFoundException() : logic_error("Could not find identifier in query string!") {}
    };
};