#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

#include "bank.h"
#include "database.h"

using namespace std;
using namespace spdlog;
using namespace bank;
using namespace database;

#ifdef WIN32
#include <windows.h>
#define CLEAR_COMMAND "cls"
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR_COMMAND "clear"
#endif

namespace interface
{

    class Command
    {
    private:
        string name;
        string description;
        bool authenticationRequired;

    public:
        Command(string name, string description, bool authenticationRequired) : name(name), description(description), authenticationRequired(authenticationRequired) {}
        Command() {}
        ~Command() {}

        bool operator<(const Command &command) const
        {
            return name < command.name;
        }

        string getName() const { return name; }
        string getDescription() const { return description; }
        string getFormattedDescription() const
        {
            string result;
            result += authenticationRequired ? "*" : " ";
            result += "[" + name + "] - ";
            result += description;
            return result;
        }

        bool getAuthenticationRequired() const { return authenticationRequired; }
    };

    class CLI
    {
    private:
        DatabaseManager manager;
        pair<int, User> authenticatedUser;
        map<Command, function<void()>> commandMapping;
        bool isRunning;

        void clearUtility() { system(CLEAR_COMMAND); }
        void setInputEcho(bool enable)
        {
#ifdef WIN32
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode;
            GetConsoleMode(hStdin, &mode);
            if (!enable)
                mode &= ~ENABLE_ECHO_INPUT;
            else
                mode |= ENABLE_ECHO_INPUT;
            SetConsoleMode(hStdin, mode);
#else
            struct termios tty;
            tcgetattr(STDIN_FILENO, &tty);
            if (!enable)
                tty.c_lflag &= ~ECHO;
            else
                tty.c_lflag |= ECHO;
            (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
        }
        string getInput(string message)
        {
            string input;
            cout << message;
            cin >> input;
            if (input == "exit")
                this->exit();
            // if(input == "quit" || input == "q")
            //     continue commands
            return input;
        }

        bool strongPasswordValidator(string password)
        {
            bool hasLower = false, hasUpper = false, hasNumber = false, hasSpecial = false;
            string special = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
            for (auto it = password.begin(); it != password.end(); it++)
            {
                if (islower(*it))
                    hasLower = true;
                if (isupper(*it))
                    hasUpper = true;
                if (isnumber(*it))
                    hasNumber = true;
                if (special.find(*it) != string::npos)
                    hasSpecial = true;
            }
            return (hasLower || hasUpper || hasNumber || hasSpecial);
        }
        bool emailValidator(string email)
        {
            regex emailRegex("^[a-zA-Z0-9][a-zA-Z0-9_.]+@[a-zA-Z0-9_]+.[a-zA-Z0-9_.]+$");
            return regex_search(email, emailRegex);
        }

        string emailUtility()
        {
            string email;
            while (true)
            {
                email = getInput("Email: ");
                if (!this->emailValidator(email))
                    cout << "The provided email is not valid." << endl;
                else
                    break;
            }
            return email;
        }
        string passwordUtility(bool validate = true)
        {
            string password;
            while (true)
            {
                setInputEcho(false);
                password = getInput("Password: ");
                setInputEcho(true);

                cout << endl;
                if (validate)
                {
                    if (!this->strongPasswordValidator(password))
                        cout << "The provided password does not match the strength criteria. "
                             << "Please make sure your password contains at least 8 characters, one uppercase letter, one lowercase letter, one number and one special character." << endl;
                    else
                        break;
                }
                else
                    break;
            }
            return password;
        }
        string countryCodeUtility()
        {
            cout << "Please enter your country code. The available countries are:" << endl;
            auto countryDisplayData = manager.getCountryDisplayData();
            for (const auto &displayData : countryDisplayData)
                cout << displayData.first << ": " << displayData.second << endl;
            cout << endl;

            set<string> countryCodes;
            for (const auto &displayData : countryDisplayData)
                countryCodes.insert(displayData.first);

            string countryCode;
            while (true)
            {
                countryCode = getInput("Country code: ");
                if (countryCodes.find(countryCode) == countryCodes.end())
                    cout << "Please enter a valid country code." << endl;
                else
                    break;
            }
            return countryCode;
        }
        string currencyCodeUtility()
        {
            cout << "Please enter the currency. The available currencies are:" << endl;
            auto currencyDisplayData = manager.getCurrencyDisplayData();
            for (const auto &displayData : currencyDisplayData)
                cout << displayData.first << ": " << displayData.second << endl;
            cout << endl;

            set<string> currencyCodes;
            for (const auto &displayData : currencyDisplayData)
                currencyCodes.insert(displayData.first);

            string currencyCode;
            while (true)
            {
                currencyCode = getInput("Currency code: ");
                if (currencyCodes.find(currencyCode) == currencyCodes.end())
                    cout << "Please enter a valid currency code." << endl;
                else
                    break;
            }
            return currencyCode;
        }

        void clear()
        {
            clearUtility();
            cout << "Please enter your commands below." << endl;
        }
        void exit()
        {
            clearUtility();
            isRunning = false;
        }
        void help()
        {
            vector<Command> noAuthenticationRequiredCommands;
            vector<Command> authenticationRequiredCommands;
            for (auto it = commandMapping.begin(); it != commandMapping.end(); it++)
            {
                Command command = it->first;
                if (command.getAuthenticationRequired())
                    authenticationRequiredCommands.emplace_back(command);
                else
                    noAuthenticationRequiredCommands.emplace_back(command);
            }
            this->clear();
            cout << endl;
            cout << "Commands that don't require authentication:" << endl;
            for (const auto &command : noAuthenticationRequiredCommands)
                cout << command.getFormattedDescription() << endl;
            cout << endl;
            cout << "Commands that require authentication:" << endl;
            for (const auto &command : authenticationRequiredCommands)
                cout << command.getFormattedDescription() << endl;
        }
        void signup()
        {
            if (authenticatedUser.first != -1)
                throw(runtime_error("An user is already logged in! Please log out first!"));
            string email, firstName, lastName, password, countryCode;
            firstName = getInput("First name: ");
            lastName = getInput("Last name: ");
            email = emailUtility();
            password = passwordUtility();
            countryCode = countryCodeUtility();
            auto user = manager.createUser(countryCode, email, firstName, lastName, password);
            authenticatedUser = user;
            cout << "You have successfully created an account!" << endl;
        }
        void login()
        {
            if (authenticatedUser.first != -1)
                throw(runtime_error("An user is already signed in! Please log out first!"));
            string email, password;
            email = emailUtility();
            password = passwordUtility(false);
            auto user = manager.getUserFromEmail(email);
            if (user.second.authenticate(password))
            {
                authenticatedUser = user;
                cout << "You have successfully logged in!" << endl;
            }
            else
                cout << "Incorrect password!" << endl;
        }
        void logout()
        {
            authenticatedUser.first = -1;
            cout << "You have successfully logged out!" << endl;
        }
        void userInfo()
        {
            cout << authenticatedUser.second;
        }
        void userDelete()
        {
            cout << "Are you sure you want to delete your account? [yes/no]" << endl;
            string confirmation = getInput("Confirm: ");
            if (confirmation == "yes" || confirmation == "y")
            {
                manager.deleteUser(authenticatedUser);
                authenticatedUser = make_pair(-1, User());
                cout << "Account deleted successfully!" << endl;
            }
        }
        void addAccount()
        {
            string firstName, lastName, currencyCode;
            firstName = getInput("Account holder's first name: ");
            lastName = getInput("Account holder's last name: ");
            currencyCode = currencyCodeUtility();

            pair<int, Account> account = manager.createAccount(currencyCode, authenticatedUser, firstName, lastName);
            cout << "You have successfully created a bank account!" << endl;
        }
        void deleteAccount()
        {
            this->viewAccounts();
            cout << endl;
            string IBAN = getInput("IBAN: ");
            manager.deleteAccount(IBAN, authenticatedUser);
            cout << "You have successfully deleted your bank account" << endl;
        }
        void viewAccounts()
        {
            auto accounts = manager.getUserAccounts(authenticatedUser);
            for (const auto &account : accounts)
            {
                cout << "Account " << account.second.getIBAN() << endl;
                cout << account.second << endl;
            }
        }
        void addTransaction()
        {
            cout << "Please select the account from which you want to make the transaction:" << endl;
            this->viewAccounts();
            cout << endl;

            string outboundIBAN, inboundIBAN, amountString;
            outboundIBAN = getInput("Your IBAN: ");
            inboundIBAN = getInput("Destination IBAN: ");
            amountString = getInput("Transaction amount: ");

            if (outboundIBAN == inboundIBAN)
                throw(runtime_error("You can not make a transfer from an account to the same account."));

            double amount = stod(amountString);
            manager.createTransaction(authenticatedUser, inboundIBAN, outboundIBAN, amount);
            cout << "Transaction successfully registered!" << endl;
        }
        void viewTransactions()
        {

            cout << "Please select the account from which you want to make the transaction:" << endl;
            this->viewAccounts();

            string IBAN = getInput("IBAN: ");

            bool matches = false;
            auto accounts = manager.getUserAccounts(authenticatedUser);
            pair<int, Account> userAccount;
            for (const auto &account : accounts)
                if (account.second.getIBAN() == IBAN)
                {
                    matches = true;
                    userAccount = account;
                    break;
                }
            if (!matches)
                throw(runtime_error("IBAN does not exist, or it is not associated with one of your accounts."));

            auto transactions = manager.getAccountTransactions(userAccount);
            vector<string> inboundTransactions, outboundTransactions;
            for (const auto &transaction : transactions)
                if (transaction.second.getInbound() == userAccount.second)
                {
                    inboundTransactions.emplace_back("From " + transaction.second.getOutbound().getIBAN() +
                                                     " recieved " + to_string(transaction.second.getAmount()) +
                                                     " " + transaction.second.getOutbound().getCurrency().getCode());
                }
                else
                {
                    outboundTransactions.emplace_back("To " + transaction.second.getInbound().getIBAN() +
                                                      " sent " + to_string(transaction.second.getAmount()) +
                                                      " " + transaction.second.getOutbound().getCurrency().getCode());
                }

            cout << endl;
            if (!inboundTransactions.empty())
            {
                cout << "Inbound transactions: " << endl;
                for (const auto &inboundTransaction : inboundTransactions)
                    cout << inboundTransaction << endl;
            }
            if (!outboundTransactions.empty())
            {
                cout << endl
                     << "Outbound transactions: " << endl;
                for (const auto &outboundTransaction : outboundTransactions)
                    cout << outboundTransaction;
            }
            if (inboundTransactions.empty() && outboundTransactions.empty())
                cout << "There are no transactions associated with this account." << endl;
        }
        void viewExchange()
        {
            auto exchanges = manager.getExchangeDisplayData();
            for (const auto &exchange : exchanges)
                cout << get<0>(exchange) << " -> " << get<1>(exchange) << " at " << get<2>(exchange) << endl;
        }

    public:
        CLI(string databaseConnectionString, string initializationFilePath) : manager{databaseConnectionString, initializationFilePath}
        {
            authenticatedUser.first = -1;
            commandMapping.insert(make_pair(Command("clear", "clear terminal", false), [this]()
                                            { this->clear(); }));
            commandMapping.insert(make_pair(Command("exit", "exit application", false), [this]()
                                            { this->exit(); }));
            commandMapping.insert(make_pair(Command("help", "list application commands", false), [this]()
                                            { this->help(); }));

            commandMapping.insert(make_pair(Command("signup", "create an account", false), [this]()
                                            { this->signup(); }));
            commandMapping.insert(make_pair(Command("login", "log into an existing account", false), [this]()
                                            { this->login(); }));
            commandMapping.insert(make_pair(Command("logout", "log out of your account", true), [this]()
                                            { this->logout(); }));
            commandMapping.insert(make_pair(Command("user-delete", "delete your account", true), [this]()
                                            { this->userDelete(); }));
            commandMapping.insert(make_pair(Command("user-info", "view your account's information", true), [this]()
                                            { this->userInfo(); }));

            commandMapping.insert(make_pair(Command("add-account", "create a new account", true), [this]()
                                            { this->addAccount(); }));
            commandMapping.insert(make_pair(Command("delete-account", "delete account", true), [this]()
                                            { this->deleteAccount(); }));
            commandMapping.insert(make_pair(Command("view-accounts", "view your accounts", true), [this]()
                                            { this->viewAccounts(); }));

            commandMapping.insert(make_pair(Command("add-transaction", "create a new transaction", true), [this]()
                                            { this->addTransaction(); }));
            commandMapping.insert(make_pair(Command("view-transactions", "view all transactions from an account", true), [this]()
                                            { this->viewTransactions(); }));
            commandMapping.insert(make_pair(Command("view-exchange", "view current exchange rates", false), [this]()
                                            { this->viewExchange(); }));
        }
        ~CLI() {}

        void start()
        {
            isRunning = true;

            clearUtility();
            cout << "Welcome to Useless Bank! You can view the available commands by typing 'help' below." << endl;
            cout << endl;

            while (isRunning)
            {
                string input;
                input = getInput("> ");

                bool commandFound = false;
                for (const auto &mapping : commandMapping)
                {
                    if (input == mapping.first.getName())
                    {
                        if (authenticatedUser.first == -1 && mapping.first.getAuthenticationRequired())
                        {
                            cout << "You must be authenticated to use this functionality!" << endl;
                            commandFound = true;
                            break;
                        }
                        cout << endl;
                        try
                        {
                            mapping.second();
                        }
                        catch (exception const &exception)
                        {
                            cout << exception.what() << endl;
                        }
                        cout << endl;
                        commandFound = true;
                        break;
                    }
                }
                if (!commandFound)
                    cout << "Command not found!" << endl;
            }
        }
    };
};