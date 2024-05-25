#include <ctime>
#include <chrono>
#include <format>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <type_traits>

using namespace std;
using namespace chrono;

enum RiskType
{
    Low,
    Medium,
    High
};

template <typename ValueType>
class RiskFactor
{
protected:
    ValueType value;
    string unit;

    inline static const string valueToString(ValueType value)
    {
        if (is_same<ValueType, string>::value)
            return value;
        if (is_same<ValueType, long long>::value)
            return to_string(value);
        if (is_same<ValueType, double>::value)
            return to_string(value);
        if (is_same<ValueType, bool>::value)
            return value ? "Da" : "Nu";
        if (is_same<ValueType, RiskType>::value)
        {
            switch (value)
            {
            case Low:
                return "Scazut";
            case Medium:
                return "Moderat";
            case High:
                return "Crescut";
            }
        }
        throw(runtime_error("Unsupported ValueType!!!"));
    }
    inline const string valueToString() const
    {
        return RiskFactor::valueToString(value);
    }

public:
    RiskFactor(){};
    RiskFactor(string value, string unit) : value(value), unit(unit){};
    ~RiskFactor() = default;

    virtual bool isAtRisk() = 0;

    friend ostream &operator<<(ostream &out, const RiskFactor<ValueType> riskFactor)
    {
        out << riskFactor.valueToString() << " " << riskFactor.unit;
    }
};

template <typename ValueType>
class DateRiskFactor : public RiskFactor<ValueType>
{
protected:
    time_point<system_clock> date;

public:
    DateRiskFactor() : RiskFactor<ValueType>() {}
    DateRiskFactor(ValueType value, string unit, string date) : RiskFactor<ValueType>(value, unit)
    {
        tm timeStruct = {};
        istringstream ss(date);
        ss >> get_time(&timeStruct, "%d.%m.%y");
        this->date = system_clock::from_time_t(mktime(&timeStruct));
    }
    ~DateRiskFactor() = default;

    friend ostream &operator<<(ostream &out, const DateRiskFactor<ValueType> dateRiskFactor)
    {
        RiskFactor<ValueType> riskFactor = dynamic_cast<RiskFactor<ValueType>>(dateRiskFactor);
        out << "(" + format("{%d.%m.%y}", dateRiskFactor.date) + ") " << riskFactor;
    }
};

class Cholesterol : public DateRiskFactor<long long>
{
};
class Pacient
{
    string nume;
    string prenume;
    long long varsta;
    string adresa;
};

int main()
{
    return 0;
}