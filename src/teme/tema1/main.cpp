#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <curses.h>

#include "bank.h"
#include "database.h"

using namespace std;
using namespace spdlog;
using namespace bank;
using namespace database;

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

int main(int argc, char *argv[])
{
    // set up logging
    auto logger = basic_logger_mt("logger", "logs/main.log");
    set_default_logger(logger);
    flush_on(level::info);

    // get database name
    string databaseName = "poo";
    if (argc > 1)
        databaseName = string(argv[1]);

    // initialize database manager
    DatabaseManager manager("postgresql://postgres:postgres@localhost:5432/" + databaseName);

    // create tables in database if they do not exist
    manager.executeQuery("scripts/createTables.sql");

    /// initialize database with countries
    manager.executeQuery("scripts/initializeDatabase.sql");

    // load data
    manager.loadCountries();
    manager.loadUsers();

    // available commands
    string generalHeader = "General commands:";
    string exit = "exit";
    string exitMessage = " [" + exit + "] - close the application";
    string help = "help";
    string helpMessage = " [" + help + "] - view this prompt";
    string clear = "clear";
    string clearMessage = " [" + clear + "] - clear terminal";
    vector<string> generalCommands{generalHeader, helpMessage, clearMessage, exitMessage};

    string authenticationHeader = "Authentication commands:";
    string signup = "signup";
    string signupMessage = " [" + signup + "] - add an account";
    string login = "login";
    string loginMessage = " [" + login + "] - log into an existing account";
    string logout = "logout";
    string logoutMessage = "*[" + logout + "] - log out of your account";
    string deleteUser = "delete";
    string deleteUserMessage = "*[" + deleteUser + "*] - delete your account";
    string infoUser = "infouser";
    string infoUserMessage = "*[" + infoUser + "*] - get you account's information";
    vector<string> authenticationCommands{authenticationHeader, signupMessage, loginMessage, logoutMessage, deleteUserMessage, infoUserMessage};

    string accountsHeader = "Account-related commands:";

    vector<vector<string>> commandSections{generalCommands, authenticationCommands};

    // prompt user with welcome message
    cout << "Welcome to Useless Bank! You can view the available commands by typing 'help' below." << endl;

    // input management will be added soon via replxx
    string input;
    User authenticatedUser;
    while (1)
    {
        cout << "> ";
        getline(cin, input);

        bool isUserLoggedIn = !authenticatedUser.getEmail().empty();

        if (!input.compare(exit))
            break;
        if (!input.compare(help))
        {
            for (auto it = commandSections.begin(); it != commandSections.end(); it++)
            {
                for (auto itp = (*it).begin(); itp != (*it).end(); itp++)
                    cout << *itp << endl;
                cout << endl;
            }
            continue;
        }
        if (!input.compare(clear))
        {
            system(CLEAR_COMMAND);
            cout << "Enter your commands below." << endl;
            continue;
        }
        if (!input.compare(signup))
        {
            try
            {
                string email, firstName, lastName, password, countryCode;
                cout << "Please enter your email: ";
                getline(cin, email);
                cout << "Please enter your first name: ";
                getline(cin, firstName);
                cout << "Please enter your last name: ";
                getline(cin, lastName);
                cout << "Please enter your password: ";
                getline(cin, password);

                vector<pair<string, string>> countryDisplayData = manager.getCountryDisplayData();
                cout << "Please enter your country's code. The available countries are listed below:" << endl;
                for (auto it = countryDisplayData.begin(); it != countryDisplayData.end(); it++)
                    cout << (it->second) << ": " << (it->first) << endl;
                getline(cin, countryCode);

                User user = manager.createUser(countryCode, email, firstName, lastName, password);
                authenticatedUser = user;
                cout << "You have successfully created an account!" << endl;
            }
            catch (exception const &exception)
            {
                error(exception.what());
                cout << "Error: " << exception.what() << endl;
                cout << "Please try again." << endl;
            }
            continue;
        }

        if (!input.compare(login))
        {
            try
            {
                string email, password;
                cout << "Please enter your email: ";
                getline(cin, email);

                User user = manager.findUserByEmail(email);

                cout << "Please enter your password: ";
                getline(cin, password);

                if (user.authenticate(password))
                {
                    authenticatedUser = user;
                    cout << "You have successfully logged in!" << endl;
                }
                else
                    throw(runtime_error("Incorrect password!"));
            }
            catch (exception const &exception)
            {
                error(exception.what());
                cout << "Error: " << exception.what() << endl;
                cout << "Please try again." << endl;
            }
            continue;
        }
        if (!input.compare(logout) && isUserLoggedIn)
        {
            User emptyUser;
            authenticatedUser = emptyUser;
            cout << "You have successfully logged out!" << endl;
            continue;
        }
        if (!input.compare(deleteUser) && isUserLoggedIn)
        {
            try
            {
                manager.deleteUser(authenticatedUser);
                User emptyUser;
                authenticatedUser = emptyUser;
                cout << "Account deleted successfully!" << endl;
            }
            catch (exception const &exception)
            {
                error(exception.what());
                cout << "Error: " << exception.what() << endl;
                cout << "Please try again." << endl;
            }
            continue;
        }
        if (!input.compare(infoUser) && isUserLoggedIn)
        {
            cout << authenticatedUser;
            continue;
        }
   
        cout << "Unknown command or you do not have access to this command. To view all available commands, type 'help'." << endl;
    }
    return 0;
}