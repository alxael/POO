/*
 * Aelenei Alex-Ioan 152
 * CLion MacOS, Apple clang compiler
 * Gabriel Majeri
 * */
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

class Item {
private:
    inline static int currentId = 0;

protected:
    const int id;

public:
    Item() : id(++Item::currentId) {}

    const int getId() const { return id; }

    virtual int getAddCost() { return 0; }

    virtual int getUpgradeCost() const = 0;

    virtual void upgrade() = 0;
};

class Wall : public Item {
private:
    // sizes in meters
    double height, length, width;
public:
    Wall() : Item(), height(2), length(1), width(0.5) {}

    Wall(double height, double length, double width)
            : Item(), height(height), length(length), width(width) {}

    double getHeight() const {
        return height;
    }

    void setHeight(double height) {
        Wall::height = height;
    }

    double getLength() const {
        return length;
    }

    void setLength(double length) {
        Wall::length = length;
    }

    double getWidth() const {
        return width;
    }

    void setWidth(double width) {
        Wall::width = width;
    }

    int getUpgradeCost() const override {
        return (int) (100 * length * width * height);
    }

    void upgrade() override {
        length += 1;
        width += 1;
        height += 1;
    }

    friend ostream &operator<<(ostream &out, const Wall &wall) {
        out << "Wall ID-" << wall.getId() << endl;
        out << "Length: " << wall.getLength() << endl;
        out << "Height: " << wall.getHeight() << endl;
        out << "Width: " << wall.getWidth() << endl;
        return out;
    }
};

class Tower : public Item {
private:
    // power in kws
    double power;
public:
    Tower() : Item(), power(1000) {}

    Tower(double power) : Item(), power(power) {}

    double getPower() const {
        return power;
    }

    void setPower(double power) {
        Tower::power = power;
    }

    int getUpgradeCost() const override {
        return (int) (500 * power);
    }

    void upgrade() override {
        power += 500;
    }

    friend ostream &operator<<(ostream &out, const Tower &tower) {
        out << "Tower ID-" << tower.getId() << endl;
        out << "Power: " << tower.getPower() << endl;
        return out;
    }
};

class Robot : public Item {
protected:
    double damage, health;
    int level;

public:
    Robot() : Item(), level(1), damage(100), health(100) {}

    Robot(int level, double damage, double health)
            : Item(), level(level), damage(damage), health(health) {}

    double getDamage() const {
        return damage;
    }

    void setDamage(double damage) {
        Robot::damage = damage;
    }

    double getHealth() const {
        return health;
    }

    void setHealth(double health) {
        Robot::health = health;
    }

    int getLevel() const {
        return level;
    }

    void setLevel(int level) {
        Robot::level = level;
    }

    friend ostream &operator<<(ostream &out, const Robot &robot) {
        out << "Level: " << robot.getLevel() << endl;
        out << "Health: " << robot.getHealth() << endl;
        out << "Damage: " << robot.getDamage() << endl;
        return out;
    }
};

class AerialRobot : public Robot {
private:
    // autonomy in hours
    int autonomy;
public:
    AerialRobot() : Robot(), autonomy(10) {}

    AerialRobot(int level, double damage, double health, int autonomy) : Robot(level, damage, health),
                                                                         autonomy(autonomy) {}

    int getAutonomy() const {
        return autonomy;
    }

    void setAutonomy(int autonomy) {
        AerialRobot::autonomy = autonomy;
    }

    int getUpgradeCost() const override {
        return (int) (50 * autonomy);
    }

    void upgrade() override {
        autonomy += 1;
        level += 1;
        damage += 25;
    }

    friend ostream &operator<<(ostream &out, const AerialRobot &robot) {
        out << "Aerial Robot ID-" << robot.getId() << endl;
        out << static_cast<const Robot &>(robot);
        out << "Autonomy: " << robot.getAutonomy() << endl;
        return out;
    }
};

class LandRobot : public Robot {
private:
    int bulletCount;
    bool hasShield;

public:
    LandRobot() : Robot(), bulletCount(500), hasShield(false) {}

    LandRobot(int level, double damage, double health, int bulletCount, bool hasShield) : Robot(level, damage, health),
                                                                                          bulletCount(bulletCount),
                                                                                          hasShield(hasShield) {}

    int getBulletCount() const {
        return bulletCount;
    }

    void setBulletCount(int bulletCount) {
        LandRobot::bulletCount = bulletCount;
    }

    bool isHasShield() const {
        return hasShield;
    }

    void setHasShield(bool hasShield) {
        LandRobot::hasShield = hasShield;
    }

    int getUpgradeCost() const override {
        return (int) (10 * bulletCount);
    }

    void upgrade() override {
        bulletCount += 100;
        level += 1;
        damage += 50;
        if (level >= 5) {
            hasShield = true;
            health += 50;
        }
    }

    friend ostream &operator<<(ostream &out, const LandRobot &robot) {
        out << "Land Robot ID-" << robot.getId() << endl;
        out << static_cast<const Robot &>(robot);
        out << "Bullets: " << robot.getBulletCount() << endl;
        out << "Has shield: " << (robot.isHasShield() ? "yes" : "no") << endl;
        return out;
    }
};

enum ItemType {
    Wall,
    Tower,
    AerialRobot,
    LandRobot
};

class Inventory {
private :
    map<int, ItemType> itemMapping;
    vector<shared_ptr<Item>> items;
    ostream &out;
    istream &in;
    int credits;

    inline static const map<ItemType, pair<int, string>> itemData{{Wall,        {300, "Wall"}},
                                                                  {Tower,       {500, "Tower"}},
                                                                  {AerialRobot, {100, "Aerial robot"}},
                                                                  {LandRobot,   {50,  "Land robot"}}};

    void addItemOfType(ItemType itemType) {
        auto entry = itemData.find(itemType);
        if (entry == itemData.end())
            throw (logic_error("Could not find item type!"));
        auto cost = entry->second.first;
        if (credits - cost < 0)
            throw (logic_error("Insufficient credits!"));
        switch (itemType) {
            case Wall: {
                auto item = std::make_shared<class Wall>();
                items.push_back(item);
                itemMapping.insert(make_pair(item->getId(), Wall));
                break;
            }
            case Tower: {
                auto item = std::make_shared<class Tower>();
                items.push_back(item);
                itemMapping.insert(make_pair(item->getId(), Tower));
                break;
            }
            case AerialRobot: {
                auto item = std::make_shared<class AerialRobot>();
                items.push_back(item);
                itemMapping.insert(make_pair(item->getId(), AerialRobot));
                break;
            }
            case LandRobot: {
                auto item = std::make_shared<class LandRobot>();
                items.push_back(item);
                itemMapping.insert(make_pair(item->getId(), LandRobot));
                break;
            }
            default:
                throw (logic_error("Could not find item type!"));
        }
    }

public:
    Inventory(istream &in, ostream &out) : credits(50000), in(in), out(out) {}

    void addItem() {
        for (const auto &item: itemData)
            out << item.first << " - " << item.second.second << " " << item.second.first << " credits" << endl;

        int item;
        in >> item;
        if (itemData.find(ItemType(item)) == itemData.end())
            throw (logic_error("Could not find item type!"));
        addItemOfType(ItemType(item));
    }

    void sellItem() {
        out << "Item ID: ";
        int itemId;
        in >> itemId;
        if (itemMapping.find(itemId) == itemMapping.end())
            throw (logic_error("Could not find item with ID " + to_string(itemId)));
        for (auto it = items.begin(); it != items.end(); it++)
            if ((*it)->getId() == itemId) {
                items.erase(it);
                credits += 500;
                break;
            }
    }

    void upgradeItem() {
        out << "Item ID: ";
        int itemId;
        in >> itemId;
        if (itemMapping.find(itemId) == itemMapping.end())
            throw (logic_error("Could not find item with ID " + to_string(itemId)));

        vector<shared_ptr<Item>>::iterator itemIt;
        for (auto it = items.begin(); it != items.end(); it++)
            if ((*it)->getId() == itemId) {
                itemIt = it;
                break;
            }

        int upgradeCost = (*itemIt)->getUpgradeCost();
        if (credits - upgradeCost < 0)
            throw (logic_error("Insufficient credits for equipment upgrade!"));
        out << "It costs " << upgradeCost << " credits to upgrade this equipment. Do you wish to procceed? [0/1]";
        bool operationType;
        in >> operationType;
        if (!operationType)
            return;
        else
            (*itemIt)->upgrade();
    }

    void printAllRobots() {
        for (const auto &item: items) {
            ItemType itemType = itemMapping[item->getId()];
            switch (itemType) {
                case Wall:
                    break;
                case Tower:
                    break;
                case AerialRobot: {
                    auto castItem = static_pointer_cast<class AerialRobot>(item);
                    out << *castItem << endl;
                    break;
                }
                case LandRobot: {
                    auto castItem = static_pointer_cast<class LandRobot>(item);
                    out << *castItem << endl;
                    break;
                }
                default:
                    throw (logic_error("Could not find item type!"));
            }
        }
    }

    static bool compareFunction(const shared_ptr<Item> itemOne, const shared_ptr<Item> itemTwo) {
        return itemOne->getUpgradeCost() < itemTwo->getUpgradeCost();
    }

    void printAllItemsByUpgradeCost() {
        sort(items.begin(), items.end(), compareFunction);
        for (const auto &item: items) {
            ItemType itemType = itemMapping[item->getId()];
            switch (itemType) {
                case Wall: {
                    auto castItem = static_pointer_cast<class Wall>(item);
                    out << *castItem << "Upgrade cost: " << (*item).getUpgradeCost() << endl << endl;
                    break;
                }
                case Tower: {
                    auto castItem = static_pointer_cast<class Tower>(item);
                    out << *castItem << "Upgrade cost: " << (*item).getUpgradeCost() << endl << endl;
                    break;
                }
                case AerialRobot: {
                    auto castItem = static_pointer_cast<class AerialRobot>(item);
                    out << *castItem << "Upgrade cost: " << (*item).getUpgradeCost() << endl << endl;
                    break;
                }
                case LandRobot: {
                    auto castItem = static_pointer_cast<class LandRobot>(item);
                    out << *castItem << "Upgrade cost: " << (*item).getUpgradeCost() << endl << endl;
                    break;
                }
                default:
                    throw (logic_error("Could not find item type!"));
            }
        }
    }
};

int main() {
    Inventory inventory(cin, cout);
    while (true) {
        cout << "1 - add item" << endl;
        cout << "2 - upgrade item" << endl;
        cout << "3 - print items in inventory by upgrade cost" << endl;
        cout << "4 - print robots in inventory" << endl;
        cout << "5 - sell item" << endl;

        int operationType;
        cin >> operationType;
        try {
            switch (operationType) {
                case 1:
                    inventory.addItem();
                    break;
                case 2:
                    inventory.upgradeItem();
                    break;
                case 3:
                    inventory.printAllItemsByUpgradeCost();
                    break;
                case 4:
                    inventory.printAllRobots();
                    break;
                case 5:
                    inventory.sellItem();
                    break;
                default:
                    throw (logic_error("Invalid operation type!"));
            }
        } catch (const logic_error &logicError) {
            cout << "Error! - " << logicError.what() << endl;
        }

    }
    return 0;
}