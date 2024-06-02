#include <regex>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

class CalendaristicDate
{
private:
    int day;
    int month;
    int year;

    inline static bool isYearValid(int year) { return (0 <= year && year <= 2024); }
    inline static bool isMonthValid(int month) { return (1 <= month && month <= 12); }
    inline static bool isDayValid(int day, int month, int year)
    {
        if (!isYearValid(year))
            throw(logic_error("Invalid year, can not determine if day is valid!"));
        if (!isMonthValid(month))
            throw(logic_error("Invalid month, can not determine if day is valid!"));
        if (day < 1)
            return false;
        vector<int> monthsWithThirtyDays{1, 3, 5, 7, 8, 10, 12};
        vector<int> monthsWithThirtyOneDays{4, 6, 9, 11};
        if (month == 2)
            return (year % 4 == 0) ? (day <= 28) : (day <= 29);
        if (find(monthsWithThirtyDays.begin(), monthsWithThirtyDays.end(), month) != monthsWithThirtyDays.end())
            return (day <= 30);
        if (find(monthsWithThirtyOneDays.begin(), monthsWithThirtyOneDays.end(), month) != monthsWithThirtyOneDays.end())
            return (day <= 31);
        throw(runtime_error("Can not determine if day is valid!"));
    }

public:
    CalendaristicDate() : day(0), month(0), year(0) {}
    CalendaristicDate(int year) : day(0), month(0), year(year)
    {
        if (!isYearValid(year))
            throw(logic_error("Invalid year, can not create date!"));
    }
    CalendaristicDate(int month, int year) : day(0), month(month), year(year)
    {
        if (!isYearValid(year))
            throw(logic_error("Invalid year, can not create date!"));
        if (!isMonthValid(month))
            throw(logic_error("Invalid month, can not create date!"));
    }
    CalendaristicDate(int day, int month, int year) : day(day), month(month), year(year)
    {
        if (!isYearValid(year))
            throw(logic_error("Invalid year, can not create date!"));
        if (!isMonthValid(month))
            throw(logic_error("Invalid month, can not create date!"));
        if (!isDayValid(day, month, year))
            throw(logic_error("Invalid year, can not create date!"));
    }

    bool operator==(CalendaristicDate &calendaristicDate)
    {
        return (this->year == calendaristicDate.year) && (this->month == calendaristicDate.month) && (this->day == calendaristicDate.day);
    }

    friend istream &operator>>(istream &in, CalendaristicDate &calendaristicDate)
    {
        string dateString;
        getline(in, dateString);

        vector<int> values;
        int value = 0;
        for (int index = 0; index < dateString.size(); index++)
            if (dateString[index] == '.')
            {
                if (value)
                    values.push_back(value);
                value = 0;
                continue;
            }
            else if (isdigit(dateString[index]))
                value = value * 10 + (dateString[index] - '0');
        if (value)
            values.push_back(value);

        switch (values.size())
        {
        case 3:
            calendaristicDate = CalendaristicDate(values[0], values[1], values[2]);
            break;
        case 2:
            calendaristicDate = CalendaristicDate(values[0], values[1]);
            break;
        case 1:
            calendaristicDate = CalendaristicDate(values[0]);
            break;
        default:
            throw(logic_error("Invalid date input!"));
            break;
        }
        return in;
    }
    friend ostream &operator<<(ostream &out, const CalendaristicDate &calendaristicDate)
    {
        out << ((calendaristicDate.day == 0) ? "" : (to_string(calendaristicDate.day) + "."));
        out << ((calendaristicDate.month == 0) ? "" : (to_string(calendaristicDate.month) + "."));
        out << calendaristicDate.year;
        return out;
    }
};

class BibliographicMaterial
{
protected:
    vector<pair<string, string>> authors;
    string title;

public:
    BibliographicMaterial() {}
    BibliographicMaterial(vector<pair<string, string>> authors, string title) : authors(authors), title(title) {}
    ~BibliographicMaterial() = default;

    vector<pair<string, string>> getAuthors() { return authors; }
    string getAuthorsString()
    {
        string answer;
        for (const auto &author : authors)
            answer += author.first + " " + author.second[0] + ". ";
        answer.pop_back();
        return answer;
    }
    void setAuthors(vector<pair<string, string>> authors) { this->authors = authors; }

    string getTitle() { return title; }
    void setTitle(string title) { this->title = title; }

    bool hasAuthorWithFirstName(string firstName) const
    {
        for (const auto &author : authors)
            if (author.first == firstName)
                return true;

        return false;
    }

    friend istream &operator>>(istream &in, BibliographicMaterial &bibliographicMaterial)
    {
        string authorsString, title;
        getline(in, authorsString);
        getline(in, title);

        vector<pair<string, string>> authors;
        string firstName, lastName;
        bool isFirstName = true;
        for (int index = 0; index < authorsString.size(); index++)
            if (authorsString[index] == ' ')
                isFirstName = false;
            else
            {
                if (authorsString[index] == ',')
                {
                    if (firstName.size() || lastName.size())
                        authors.push_back(pair<string, string>(firstName, lastName));
                    firstName = "";
                    lastName = "";
                    isFirstName = true;
                    continue;
                }
                else
                {
                    if (isFirstName)
                        firstName += authorsString[index];
                    else
                        lastName += authorsString[index];
                }
            }
        if (firstName.size() || lastName.size())
            authors.push_back(pair<string, string>(firstName, lastName));

        bibliographicMaterial.setAuthors(authors);
        bibliographicMaterial.setTitle(title);
        return in;
    }
    friend ostream &operator<<(ostream &out, const BibliographicMaterial &bibliographicMaterial)
    {
        out << "Title: " + bibliographicMaterial.title << endl;
        out << "Author" << ((bibliographicMaterial.authors.size() > 1) ? "s: " : ": ");
        for (int index = 0; index < bibliographicMaterial.authors.size(); index++)
        {
            out << bibliographicMaterial.authors[index].first << " " << bibliographicMaterial.authors[index].second;
            if (index != bibliographicMaterial.authors.size() - 1)
                out << ", ";
        }
        return out;
    }
};

class PrintedBibliographicMaterial : public BibliographicMaterial
{
protected:
    CalendaristicDate datePublished;

public:
    PrintedBibliographicMaterial() {}
    PrintedBibliographicMaterial(vector<pair<string, string>> authors, string title, int yearPublished) : BibliographicMaterial(authors, title), datePublished(yearPublished) {}
    ~PrintedBibliographicMaterial() = default;

    CalendaristicDate getDatePublished() const { return datePublished; }
    void setYearPublished(CalendaristicDate &datePublished) { this->datePublished = datePublished; }
};

class Website : public BibliographicMaterial
{
private:
    inline static const regex urlRegex{
        R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
        regex::extended};
    inline static const bool isUrlValid(string url) { return regex_match(url.begin(), url.end(), urlRegex); }

    string url;
    CalendaristicDate dateLastAccess;

public:
    Website() {}
    using BibliographicMaterial::BibliographicMaterial;
    Website(vector<pair<string, string>> authors, string title, string url, CalendaristicDate &dateLastAccess) : BibliographicMaterial(authors, title), url(url), dateLastAccess(dateLastAccess)
    {
        if (!isUrlValid(url))
            throw(logic_error("Invalid website url!"));
    }
    ~Website() = default;

    string getUrl() const { return url; }
    void setUrl(string url)
    {
        if (!isUrlValid(url))
            throw(logic_error("Invalid website url!!!"));
        this->url = url;
    }

    CalendaristicDate getDateLastAccess() const { return dateLastAccess; }
    void setDateLastAccess(CalendaristicDate &dateLastAccess) { this->dateLastAccess = dateLastAccess; }

    friend istream &operator>>(istream &in, Website &website)
    {
        string url;
        CalendaristicDate dateLastAccess;

        in >> static_cast<BibliographicMaterial &>(website);
        getline(in, url);
        in >> dateLastAccess;

        website.authors[0].first += " " + website.authors[0].second;
        website.authors[0].second = "";

        website.setUrl(url);
        website.setDateLastAccess(dateLastAccess);
        return in;
    }
    friend ostream &operator<<(ostream &out, const Website &website)
    {
        out << static_cast<const BibliographicMaterial &>(website) << endl
            << "URL: " + website.getUrl() << endl
            << "Last access date: " << website.getDateLastAccess();
        return out;
    }
};

class Article : public PrintedBibliographicMaterial
{
private:
    string journal;
    string journalNumber;
    int startPage, endPage;

    inline static bool arePagesValid(int startPage, int endPage) { return (startPage > 0) && (endPage > 0) && (startPage <= endPage); }

public:
    Article() {}
    Article(vector<pair<string, string>> authors, string title, int yearPublished, string journal, string journalNumber, int startPage, int endPage) : PrintedBibliographicMaterial(authors, title, yearPublished), journal(journal), journalNumber(journalNumber), startPage(startPage), endPage(endPage)
    {
        if (!arePagesValid(startPage, endPage))
            throw(logic_error("Invalid article pages!"));
    }
    ~Article() = default;

    string getJournal() const { return journal; }
    void setJournal(string journal) { this->journal = journal; }

    string getJournalNumber() const { return journalNumber; }
    void setJournalNumber(string journalNumber) { this->journalNumber = journalNumber; }

    int getStartPage() { return startPage; }
    int getEndPage() { return endPage; }
    string getJournalPages() const { return to_string(startPage) + "-" + to_string(endPage); }
    void setJournalPages(int startPage, int endPage)
    {
        if (!arePagesValid(startPage, endPage))
            throw(logic_error("Invalid article pages!"));
        this->startPage = startPage;
        this->endPage = endPage;
    }

    friend istream &operator>>(istream &in, Article &article)
    {
        CalendaristicDate datePublished;
        string journalString;

        in >> static_cast<BibliographicMaterial &>(article);
        getline(in, journalString);
        in >> datePublished;

        vector<string> journalData;
        string data;
        for (int index = 0; index < journalString.size(); index++)
            if (journalString[index] == ',')
            {
                if (data.size())
                    journalData.push_back(data);
                data = "";
            }
            else
                data += journalString[index];
        if (data.size())
            journalData.push_back(data);

        if (journalData.size() != 3)
            throw(logic_error("Invalid journal data!"));

        vector<int> pageValues;
        int value = 0;
        for (int index = 0; index < journalData[2].size(); index++)
            if (journalData[2][index] == '-')
            {
                if (value)
                    pageValues.push_back(value);
                value = 0;
            }
            else if (isdigit(journalData[2][index]))
                value = value * 10 + (journalData[2][index] - '0');
        if (value)
            pageValues.push_back(value);

        if (pageValues.size() != 2)
            throw(logic_error("Invalid journal page data!"));

        article.setYearPublished(datePublished);
        article.setJournal(journalData[0]);
        article.setJournalNumber(journalData[1]);
        article.setJournalPages(pageValues[0], pageValues[1]);
        return in;
    }

    friend ostream &operator<<(ostream &out, const Article &article)
    {
        out << static_cast<const BibliographicMaterial &>(article) << endl;
        out << article.getJournal() << ", " << article.getJournalNumber() << ", " << article.getJournalPages() << endl;
        out << article.getDatePublished();
        return out;
    }
};

class Book : public PrintedBibliographicMaterial
{
private:
    string publisher;
    string publisherCity;

public:
    Book() {}
    Book(vector<pair<string, string>> authors, string title, int yearPublished, string publisher, string publisherCity) : PrintedBibliographicMaterial(authors, title, yearPublished), publisher(publisher), publisherCity(publisherCity) {}
    ~Book() = default;

    string getPublisher() const { return this->publisher; }
    void setPublisher(string publisher) { this->publisher = publisher; }

    string getPublisherCity() const { return this->publisherCity; }
    void setPublisherCity(string publisherCity) { this->publisherCity = publisherCity; }

    friend istream &operator>>(istream &in, Book &book)
    {
        CalendaristicDate datePublished;
        string publisher, publisherCity;

        in >> static_cast<BibliographicMaterial &>(book);
        getline(in, publisher);
        getline(in, publisherCity);
        in >> datePublished;

        book.setPublisher(publisher);
        book.setPublisherCity(publisherCity);
        book.setYearPublished(datePublished);
        return in;
    }
    friend ostream &operator<<(ostream &out, const Book &book)
    {
        out << static_cast<const BibliographicMaterial &>(book) << endl;
        out << book.getPublisher() << endl
            << book.getPublisherCity() << endl;
        out << book.getDatePublished();
        return out;
    }
};

class BibliographyManager
{
private:
    ostream &out;
    vector<shared_ptr<BibliographicMaterial>> websites, articles, books;

    template <class Material>
    vector<shared_ptr<BibliographicMaterial>> readMaterials(string material, string filePath)
    {
        ifstream in(filePath);
        vector<shared_ptr<BibliographicMaterial>> answer;

        while (!in.eof())
        {
            try
            {
                Material material;
                in >> material;

                answer.push_back(make_shared<Material>(material));
            }
            catch (const logic_error &error)
            {
                out << "Error reading " + material << ": " << error.what() << endl;
                break;
            }
        }
        if (in.eof())
            out << "Finished reading " + material + "s from file!" << endl;

        return answer;
    }

    template <class Material>
    vector<Material> getMaterials(vector<shared_ptr<BibliographicMaterial>> &materials)
    {
        vector<Material> answer;
        for (const auto &material : materials)
        {
            auto ptr = static_pointer_cast<Material>(material);
            answer.push_back(*ptr.get());
        }
        return answer;
    }

    template <class Material>
    void printMaterials(string materialMessage, string material, vector<Material> materials)
    {
        out << materialMessage << endl
            << endl;
        for (int index = 0; index < materials.size(); index++)
        {
            out << material << " #" << index + 1 << ":" << endl;
            out << materials[index] << endl
                << endl;
        }
    }

public:
    BibliographyManager(ostream &out, string dataFolderPath) : out(out)
    {
        this->websites = readMaterials<Website>("website", dataFolderPath + "/websites.txt");
        this->articles = readMaterials<Article>("article", dataFolderPath + "/articles.txt");
        this->books = readMaterials<Book>("book", dataFolderPath + "/books.txt");
    }
    ~BibliographyManager() = default;

    void printAllBibliographicMaterial()
    {
        auto allWebsites = getMaterials<Website>(websites);
        auto allArticles = getMaterials<Article>(articles);
        auto allBooks = getMaterials<Book>(books);

        printMaterials<Website>("All websites:", "Website", allWebsites);
        printMaterials<Article>("All articles:", "Article", allArticles);
        printMaterials<Book>("All books:", "Book", allBooks);
    }

    void printPrintedBibliographicMaterialForDate(CalendaristicDate &date)
    {
        auto allArticles = getMaterials<Article>(articles);
        auto allBooks = getMaterials<Book>(books);

        vector<Article> filteredArticles;
        for (const auto &article : allArticles)
            if (article.getDatePublished() == date)
                filteredArticles.push_back(article);

        vector<Book> filteredBooks;
        for (const auto &book : allBooks)
            if (book.getDatePublished() == date)
                filteredBooks.push_back(book);

        printMaterials<Article>("Articles matching criteria:", "Article", filteredArticles);
        printMaterials<Book>("Books matching criteria:", "Book", filteredBooks);
    }

    void printPrintedBibliographicMaterialForAuthor(string firstName)
    {
        auto allArticles = getMaterials<Article>(articles);
        auto allBooks = getMaterials<Book>(books);

        vector<Article> filteredArticles;
        for (const auto &article : allArticles)
            if (article.hasAuthorWithFirstName(firstName))
                filteredArticles.push_back(article);

        vector<Book> filteredBooks;
        for (const auto &book : allBooks)
            if (book.hasAuthorWithFirstName(firstName))
                filteredBooks.push_back(book);

        printMaterials<Article>("Articles matching criteria:", "Article", filteredArticles);
        printMaterials<Book>("Books matching criteria:", "Book", filteredBooks);
    }

    void printWebsitesForOwner(string name)
    {
        auto allWebsites = getMaterials<Website>(websites);

        vector<Website> filteredWebsites;
        for (const auto &website : allWebsites)
            if (website.hasAuthorWithFirstName(name))
                filteredWebsites.push_back(website);

        printMaterials<Website>("Websites matching criteria:", "Website", filteredWebsites);
    }
};

int main()
{
    BibliographyManager bibliographyManager(cout, "data");

    int request;
    cout << endl;
    cin >> request;

    switch (request)
    {
    case 1:
        bibliographyManager.printAllBibliographicMaterial();
        break;
    case 2:
    {
        CalendaristicDate date;
        cout << "Year: " << endl;
        while (true)
        {
            try
            {
                cin >> date;
                break;
            }
            catch (const logic_error &error)
            {
                cout << error.what() << endl;
            }
        }
        bibliographyManager.printPrintedBibliographicMaterialForDate(date);
        break;
    }
    case 3:
    {
        string firstName;
        cout << "Author first name: " << endl;
        getline(cin, firstName);
        getline(cin, firstName);
        bibliographyManager.printPrintedBibliographicMaterialForAuthor(firstName);
        break;
    }

    case 4:
    {
        string name;
        cout << "Owner first name: " << endl;
        getline(cin, name);
        getline(cin, name);
        bibliographyManager.printWebsitesForOwner(name);
        break;
    }

    default:
        break;
    }
}