#include <iostream>

using namespace std;

class Complex
{
private:
    double re, im;
    static const double eps = 1e-14;

public:
    Complex(double re = 0, double im = 0) : re(re), im(im) {}
    static Complex from_polar(double modulus, double angle)
    {
        return Complex(modulus * cos(angle), modulus * sin(angle));
    }
    double getModulus() const
    {
        return sqrt(re * re + im * im);
    }
    friend ostream &operator<<(ostream &out, const Complex &a)
    {
        out << a.re << " + " << a.im << "i";
        return out;
    }
    friend istream &operator>>(istream &in, Complex &a)
    {
        double re, im;
        in >> a.re >> a.im;
        return in;
    }
    void operator=(const Complex &a)
    {
        re = a.re;
        im = a.im;
    }
    Complex operator+(const Complex &a)
    {
        Complex res;
        res.re = re + a.re;
        res.im = im + a.im;
        return res;
    }
    Complex operator-(const Complex &a)
    {
        Complex res;
        res.re = re - a.re;
        res.im = im - a.im;
        return res;
    }
    Complex operator*(const Complex &a)
    {
        Complex res;
        res.re = re * a.re - im * a.im;
        res.im = re * a.im + im * a.re;
        return res;
    }
    Complex operator/(const Complex &a)
    {
        Complex res;
        double length = a.getModulus();
        res.re = (re * a.re + im * a.im) / sqrt(a.re * a.re + a.im * a.im);
        res.im = (re * a.im - im * a.re) / sqrt(a.re * a.re + a.im * a.im);
        return res;
    }
    bool operator==(const Complex &a)
    {
        return fabs(re - a.re) < eps && fabs(im - a.im) < eps;
    }
    bool operator!=(const Complex &a)
    {
        return !(*this == a);
    }
};

int main()
{
    Complex x;
    cin >> x;
    cout << x << '\n';

    Complex y = Complex::from_polar(2, 0.5);
    cout << y << '\n';

    cout << x + y << '\n';
    cout << x - y << '\n';
    cout << x * y << '\n';
    cout << x / y << '\n';

    cout << (x == y) << ' ' << (x != y) << '\n';
    return 0;
}