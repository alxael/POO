#include <map>
#include <vector>
#include <iostream>

using namespace std;

enum ParticipantType {
    Student,
    Teacher,
    Other
};

class Participant {
private:
    string name;
    string cnp;
    int day, month, year;

public:
    Participant(string name, string cnp) : name(name) { setCnp(cnp); }

    string getName() const { return name; }

    void setName(string name) { this->name = name; }

    string getCnp() const { return cnp; }

    void setCnp(string cnp) {
        year = 1900 + stoi(cnp.substr(2, 2));
        if (year < 1924)
            year += 100;
        month = stoi(cnp.substr(4, 2));
        day = stoi(cnp.substr(6, 2));
        this->cnp = cnp;
    }

    ParticipantType getParticipantType() {
        int participantAge = 2024 - year;

    }

    bool operator<(const Participant &participant) const { return this->cnp < participant.cnp; }

    bool operator==(const Participant &participant) const { return this->cnp == participant.cnp; }

    friend istream &operator>>(istream &in, Participant &participant) {
        string inName, inCnp;
        char character;

        getline(in, inName, '-');
        for (int index = 0; index < 13; index++) {
            in >> character;
            if (!isdigit(character))
                throw (logic_error("Invalid CNP!"));
            inCnp += character;
        }

        participant.setName(inName);
        participant.setCnp(inCnp);
        return in;
    }

    friend ostream &operator<<(ostream &out, const Participant &participant) {
        out << participant.getName() << "-" << participant.getCnp();
        return out;
    }
};

class Activity {
protected:
    string name;
    string startDate;
public:
    const string &getName() const {
        return name;
    }

    void setName(const string &name) {
        Activity::name = name;
    }

    const string &getStartDate() const {
        return startDate;
    }

    void setStartDate(const string &startDate) {
        Activity::startDate = startDate;
    }

    int getDurationDays() const {
        return durationDays;
    }

    void setDurationDays(int durationDays) {
        Activity::durationDays = durationDays;
    }

    const map<Participant, int> &getParticipants() const {
        return participants;
    }

    void setParticipants(const map<Participant, int> &participants) {
        Activity::participants = participants;
    }

protected:
    int durationDays;
    map<Participant, int> participants;

public:
    Activity(string name, string startDate, int durationDays) : name(name), startDate(std::move(startDate)),
                                                                durationDays(durationDays) {}

    virtual void addParticipants(map<Participant, int> newParticipants) = 0;
};

class Course : public Activity {
private:
    int credits;

public:
    Course(int index, string startDate, int durationDays) : Activity("CP" + to_string(index), startDate,
                                                                     durationDays) {}

    void addParticipants(map<Participant, int> newParticipants) override {
        for (const auto &participant: newParticipants) {
            if (participant.first.)
        }
    }
};

class Contest : public Activity {
protected:
    vector<pair<Participant, int>> top;

public:
    using Activity::Activity;
};

class OnlineContest : public Contest {
private:
    string url;

public:
};

class OfflineContest : public Contest {
private:
public:
    OfflineContest(string name, string startDate) : Contest(name, startDate, 0) {}
};

int main() {

    return 0;
}