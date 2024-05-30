#include <regex>
#include <chrono>
#include <random>
#include <format>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>

using namespace std;
using namespace CryptoPP;
using namespace exception;
using namespace validation;
using namespace std::chrono;

namespace bank
{
    class Currency
    {
    private:
        string name;
        string code;

    public:
        Currency(string name, string code) : name(name), code(code) {}
        Currency() {}
        ~Currency() {}

        string getName() const { return name; }
        void setName(string name) { name = name; }

        string getCode() const { return code; }
        void setCode(string code) { code = code; }

        bool operator!=(const Currency &other) { return (other.code != code && other.name != name); }

        friend ostream &operator<<(ostream &out, const Currency &currency)
        {
            string nameOutput = "Currency name: " + currency.name;
            string codeOutput = "Currency code: " + currency.code;
            out << nameOutput << endl
                << codeOutput << endl;
            return out;
        }
    };

    class Exchange
    {
    private:
        Currency source, destination;
        double rate;

    public:
        Exchange(Currency &source, Currency &destination, double rate) : source(source), destination(destination), rate(rate) {}
        Exchange() {}
        ~Exchange() {}

        Currency getSource() const { return source; }
        void setSource(Currency newSource) { source = newSource; }

        Currency getDestination() const { return destination; }
        void setDestination(Currency newDestination) { destination = newDestination; }

        double getRate() const { return rate; }
        void setRate(double rate) { rate = rate; }

        friend ostream &operator<<(ostream &out, const Exchange &exchange)
        {
            string exchangeOutput = "Exchange: " + exchange.source.getCode() + " -> " + exchange.destination.getCode();
            string rateOutput = "Exchange rate: " + to_string(exchange.rate);
            out << exchangeOutput << endl
                << rateOutput << endl;
            return out;
        }
    };

    class Country
    {
    private:
        string name;
        string code;
        string IBANPattern;

        bool IBANMatchesPattern(string IBAN) const
        {
            // check country code
            string countryCode(IBAN.substr(0, 2));
            if (countryCode.compare(code))
                return false;
            // check checksum digits
            if (!isdigit(IBAN[2]) || !isdigit(IBAN[3]))
                return false;

            // truncate and check pattern
            IBAN.assign(IBAN.substr(4));
            if (IBAN.length() != IBANPattern.length())
                return false;
            for (int index = 0; index < IBAN.length(); index++)
            {
                if (IBANPattern[index] == 'a' && !isupper(IBAN[index]))
                    return false;
                if (IBANPattern[index] == 'n' && !isdigit(IBAN[index]))
                    return false;
                if (IBANPattern[index] == 'c' && !isalnum(IBAN[index]))
                    return false;
            }

            return true;
        }
        long long getChecksum(string truncatedIBAN) const
        {
            string numberString;
            for (int index = 0; index < truncatedIBAN.length(); index++)
            {
                if (isdigit(truncatedIBAN[index]))
                    numberString += truncatedIBAN[index];
                if (isupper(truncatedIBAN[index]))
                    numberString += to_string(static_cast<int>(truncatedIBAN[index]) - 55);
                if (islower(truncatedIBAN[index]))
                    numberString += to_string(static_cast<int>(truncatedIBAN[index]) - 87);
            }

            int segmentStart = 0;
            int step = 9;
            string prepended;
            long long number = 0;
            while (segmentStart <= numberString.length() - step)
            {
                number = stoll(prepended + numberString.substr(segmentStart, step));
                long long remainder = number % 97;
                prepended = to_string(remainder);
                if (remainder < 10)
                    prepended = "0" + prepended;
                segmentStart += step;
                step = 7;
            }
            number = stoll(prepended + numberString.substr(segmentStart));
            return number;
        }

    public:
        Country(string name, string code, string pattern) : name(name), code(code), IBANPattern(pattern)
        {
            if (name.empty() || code.empty() || pattern.empty())
                throw(ValidationException("All constructor parameters must be non-empty."));
            if (pattern.length() < 15 || pattern.length() > 34)
                throw(ValidationException("IBAN pattern must have a length between 15 and 34 characters."));
        }
        Country(const Country &other)
        {
            name = other.name;
            code = other.code;
            IBANPattern = other.IBANPattern;
        }
        Country() {}
        ~Country() {}

        string getName() const { return name; }
        void setName(string name) { name = name; }

        string getCode() const { return code; }
        void setCode(string code) { code = code; }

        string getIBANPattern() const { return IBANPattern; }
        void setIBANPattern(string IBANPattern) { IBANPattern = IBANPattern; }

        friend ostream &operator<<(ostream &out, const Country &country)
        {
            string header = "Country information:";
            string nameOutput = "Name: " + country.name;
            string codeOutput = "Code: " + country.code;
            string IBANPatternOutput = "IBANPattern: " + country.IBANPattern;
            out << header << endl
                << nameOutput << endl
                << codeOutput << endl
                << IBANPatternOutput << endl;
            return out;
        }
        void operator=(const Country &other)
        {
            name = other.name;
            code = other.code;
            IBANPattern = other.IBANPattern;
        }

        bool isIBANValid(string IBAN)
        {
            if (!IBANMatchesPattern(IBAN))
                return false;

            IBAN = IBAN.append(IBAN.substr(0, 4));
            IBAN.assign(IBAN.substr(4));

            long checksum = getChecksum(IBAN);

            return (checksum % 97 == 1);
        }
        string generateIBAN()
        {
            string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            string number = "0123456789";
            string lowercase = "abcdefghijklmnopqrstuvwxyz";
            string alphanumeric = number + lowercase + uppercase;

            random_device randomDevice;
            mt19937 mersenneTwister(randomDevice());

            uniform_int_distribution<int> uppercaseDistribution(0, uppercase.length() - 1);
            uniform_int_distribution<int> numberDistribution(0, number.length() - 1);
            uniform_int_distribution<int> alphanumericDistribution(0, alphanumeric.length() - 1);

            string partialIBAN;
            for (int index = 0; index < IBANPattern.length(); index++)
            {
                switch (IBANPattern[index])
                {
                case 'a':
                    partialIBAN += uppercase[uppercaseDistribution(mersenneTwister)];
                    break;
                case 'n':
                    partialIBAN += number[numberDistribution(mersenneTwister)];
                    break;
                case 'c':
                    partialIBAN += alphanumeric[alphanumericDistribution(mersenneTwister)];
                    break;
                default:
                    break;
                }
            }

            string prefixAddedIBAN = partialIBAN + code + "00";
            string checkDigits = to_string(98 - (getChecksum(prefixAddedIBAN) % 97));
            if (checkDigits.length() < 2)
                checkDigits = "0" + checkDigits;
            string generatedIBAN = code + checkDigits + partialIBAN;
            return generatedIBAN;
        }
    };

    class User
    {
    private:
        Country country;
        string email;
        string firstName, lastName;
        string password;

        string hashPassword(string password)
        {
            // simple SHA256 hash, should be replaced by bcrpyt/scrypt/pbkdf2
            string passwordHashed;
            SHA256 hash;
            StringSource stringSource(password, true, new HashFilter(hash, new Base64Encoder(new StringSink(passwordHashed))));
            passwordHashed.pop_back(); // eliminate /n from the end
            return passwordHashed;
        }

    public:
        // used for instantiating an existing account
        User(Country &country, string email, string firstName, string lastName, string password) : country(country), email(email), firstName(firstName), lastName(lastName), password(password) {}
        // used for registering new account
        User(string emailProvided, string firstName, string lastName, string passwordNotHashed, Country &country) : country(country), email(emailProvided), firstName(firstName), lastName(lastName)
        {
            if (!Validator::isEmail(emailProvided))
                throw(ValidationException("The provided email is not valid."));

            if (!Validator::isPasswordStrong(passwordNotHashed))
                throw(ValidationException("The provided password does not match the strength criteria. Please make sure your password contains at least 8 characters, one uppercase letter, one lowercase letter, one number and one special character."));

            password = hashPassword(passwordNotHashed);

            if (firstName.empty() || lastName.empty())
                throw(ValidationException("First name and last name must be non-empty."));
        }
        User(const User &other)
        {
            country = other.country;
            email = other.email;
            firstName = other.firstName;
            lastName = other.lastName;
            password = other.password;
        }
        User() {}
        ~User() {}

        string getPassword() const { return password; }
        void setPassword(string password) { password = password; }

        string getEmail() const { return email; }
        void setEmail(string email) { email = email; }

        string getFullName() const { return firstName + " " + lastName; }
        void setFirstName(string firstName) { firstName = firstName; }
        void setFullName(string lastName) { lastName = lastName; }

        Country getCountry() const { return country; }
        void setCountry(Country country) { country = country; }

        friend ostream &operator<<(ostream &out, const User &user)
        {
            string emailOutput = "Email: " + user.email;
            string nameOutput = "Full name: " + user.getFullName();
            string countryOutput = "Country: " + user.country.getName();
            out << emailOutput << endl
                << nameOutput << endl
                << countryOutput << endl;
            return out;
        }
        void operator=(const User &other)
        {
            country = other.country;
            email = other.email;
            firstName = other.firstName;
            lastName = other.lastName;
            password = other.password;
        }
        bool operator==(const User &other) { return (other.email == email); }
        bool operator!=(const User &other) { return (other.email != email); }

        bool authenticate(string passwordNotHashed)
        {
            string passwordHashed = hashPassword(passwordNotHashed);
            return (password == passwordHashed);
        }
    };

    class Account
    {
    private:
        Currency currency;
        User user;
        string IBAN;
        double amount;
        // holder firstName and lastName
        string firstName, lastName;

    public:
        // used for instantiating an existing account
        Account(Currency &currency, User &user, string IBAN, double amount, string firstName, string lastName) : currency(currency), user(user), IBAN(IBAN), amount(amount), firstName(firstName), lastName(lastName) {}
        // used for creating a new account
        Account(Currency &currency, User &user, string firstName, string lastName) : currency(currency), user(user), firstName(firstName), lastName(lastName)
        {
            IBAN = user.getCountry().generateIBAN();
            amount = 0;
        }
        Account() { amount = 0; }
        ~Account() {}

        double getAmount() const { return amount; }
        void setAmount(double amount) { amount = amount; }

        string getIBAN() const { return IBAN; }
        void setIBAN(string IBAN) { IBAN = IBAN; }

        User getUser() const { return user; }
        void setUser(User user) { user = user; }

        Currency getCurrency() const { return currency; }
        void setCurrency(Currency currency) { currency = currency; }

        string getFullName() const { return firstName + " " + lastName; }
        void setFirstName(string firstName) { firstName = firstName; }
        void setLastName(string lastName) { lastName = lastName; }

        friend ostream &operator<<(ostream &out, const Account &account)
        {
            string IBANOutput = "IBAN: " + account.IBAN;
            string amountOutput = "Amount: " + to_string(account.amount);
            string currencyOutput = "Currency: " + account.currency.getCode();
            string nameOutput = "Holder full name: " + account.getFullName();
            cout << IBANOutput << endl
                 << amountOutput << endl
                 << currencyOutput << endl
                 << nameOutput << endl;
            return out;
        }
        bool operator==(const Account &other) const { return (IBAN == other.IBAN); }
    };

    class Transaction
    {
    private:
        Account inbound;
        Account outbound;
        double amount;
        time_point<system_clock> date;

    public:
        Transaction(Account &inbound, Account &outbound, double amount, time_point<system_clock> date) : inbound(inbound), outbound(outbound), amount(amount), date(date) {}
        ~Transaction() {}

        double getAmount() const { return amount; }
        void setAmount(double amount) { amount = amount; }

        Account getInbound() const { return inbound; }
        void setInbound(Account newInbound) { inbound = newInbound; }

        Account getOutbound() const { return outbound; }
        void setOutbound(Account newOutbound) { outbound = newOutbound; }

        time_point<system_clock> getDate() const { return date; }
        string getDateString() const { return std::format("{:%F %T}", date); }
        void setDate(time_point<system_clock> date) { date = date; }

        friend ostream &operator<<(ostream &out, const Transaction &transaction)
        {
            string transactionOutput = "Transaction: " + transaction.outbound.getIBAN() + " -> " + transaction.inbound.getIBAN();
            string amountOutput = "Transaction amount: " + to_string(transaction.amount);
            string dateOutput = "Transaction date: " + transaction.getDateString();
            out << transactionOutput << endl
                << amountOutput << endl
                << dateOutput << endl;
            return out;
        }
    };
};