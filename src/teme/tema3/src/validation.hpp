#include <regex>
#include <iostream>

using namespace std;

namespace validation
{
    class Validator
    {
    private:
        inline static const regex emailRegex{"^[a-zA-Z0-9][a-zA-Z0-9_.]+@[a-zA-Z0-9_]+.[a-zA-Z0-9_.]+$"};
        inline static const string specialCharacters = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

    public:
        inline static const bool isPasswordStrong(const string &password) noexcept
        {
            bool hasLower = false, hasUpper = false, hasNumber = false, hasSpecial = false;
            for (const auto &character : password)
            {
                if (islower(character))
                    hasLower = true;
                if (isupper(character))
                    hasUpper = true;
                if (isnumber(character))
                    hasNumber = true;
                if (specialCharacters.find(character) != string::npos)
                    hasSpecial = true;
            }
            return (hasLower || hasUpper || hasNumber || hasSpecial);
        }
        inline static const bool isEmail(const string &email) noexcept
        {
            return regex_search(email, emailRegex);
        }
    };
};