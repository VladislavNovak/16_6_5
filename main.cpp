#include <iostream>
#include <algorithm> // any_of
#include <string>
#include <cassert>
#include <cctype>
#include <vector>
#include <cstdlib> // rand, srand
#include <ctime> // time

using std::cout;
using std::endl;
using std::string;
using std::vector;

enum class ExternalData { TIME, OUTSIDE_TEMPERATURE, INSIDE_TEMPERATURE, MOTION_DETECTION, STORAGE_SIZE };

// операции добавления/удаления/инверсии
enum class OpType { ON, OFF, REVERSAL };

enum class Switches {
    MAIN = 1,
    INSIDE_LIGHT = 2,
    OUTSIDE_LIGHT = 4,
    PLUMBING = 8,
    HEATING = 16,
    CONDITIONER = 32,
    GARDEN_LIGHTING = 64,
    STORAGE_SIZE = 7
};

vector<string> switchNames = {
        "all",
        "inside light",
        "outside light",
        "plumbing",
        "heating",
        "conditioner",
        "garden lighting"
};

// Получаем true если элемент `item` хоть раз встречается в диапазоне `range`
bool isIncludes(const string &range, const char &item) {
    return std::any_of(range.begin(),
                       range.end(),
                       [&item](const char &c) { return c == item; });
}

std::string getJoinRange (const std::string &range) {
    char const TERMINATOR = '.';
    char const JOIN = ',';

    std::string joinRange;
    for (int i = 0; i < range.size(); ++i) {
        joinRange += range[i];
        joinRange += (i != range.size() - 1) ? JOIN : TERMINATOR;
    }

    return joinRange;
}

// Получаем char-символ в обозначенном диапазоне `range`
template<typename T>
T getUserChar(string const &range) {
    T input;

    while (true) {
        std::cin >> input;
        // Если введено значение из диапазона - сбрасываем
        if (isIncludes(range, input)) break;
        printf("Error. Symbol %c not in range: %s Repeat: ", input, getJoinRange(range).c_str());
        // Сбрасываем коматозное состояние cin
        std::cin.clear();
        // Очищаем поток ввода
        fflush(stdin);
    }

    return input;
}

bool getUserYesNo (const std::string &proposition) {
    printf("%s Press 'Y' to agree or 'N' to skip: ", proposition.c_str());
    return isIncludes("Yy", getUserChar<char>("YynNn"));
}

// Получаем слово с параметрами:
//  - в указанном диапазоне `range`
//  - разрешено ли повторное введение символов. По умолчанию - не разрешено
//  - не более определенной длины (`maxCount`),
//  - с окончанием для ввода (`INPUT_ENDING`),
string getUserInput(const string &range, bool isRepeatAllowed = false, size_t maxCount = 128) {
    char const TERMINATOR = '.';
    string userInput;
    size_t curCount = 0;

    string constraints = range;
    constraints.append(1, TERMINATOR);

    // to format: zu - size_t, s - string.c_str, c - char
    printf("Enter%s symbols in range: %s ", (isRepeatAllowed ? "" : " unique"), getJoinRange(range).c_str());
    printf("To complete - press the key '%c': ", TERMINATOR);
    while(curCount < maxCount) {
        char ch = getUserChar<char>(constraints);
        if (ch == TERMINATOR) break;
        if (!isRepeatAllowed && isIncludes(userInput, ch)) {
            printf("'%c' is already there in %s. Enter another or press the key '%c': ", ch, userInput.c_str(), TERMINATOR);
            continue;
        }
        userInput += ch;
        ++curCount;
    }

    return userInput;
}

// Возвращает пользовательское слово с отсортированными уникальными символами, указанными в диапазоне range
string getUserChoiceFrom(const std::string &range) {
    string userInput = getUserInput(range, false, range.size());
    std::sort(userInput.begin(), userInput.end());

    return userInput;
}

// Меняет в переменной типа unsigned int указанный в changeItem бит
void changeStorageStatus (char item, OpType operation, unsigned int &store) {
    assert(std::isdigit(item));
    unsigned int cha = (item - '0');
    unsigned int choice = (1 << cha);

    switch (operation) {
        case (OpType::ON):       store |= choice; break;
        case (OpType::OFF):      store &= ~ choice; break;
        case (OpType::REVERSAL): store ^= choice; break;
    }
}

bool getStorageItemStatus (char item, const unsigned int &switchStorage) {
    assert(std::isdigit(item));
    unsigned int cha = (item - '0');
    unsigned int choice = (1 << cha);
    return bool(switchStorage & choice);
}

string getStorageItemInfo (char item, unsigned int &switchStorage) {
    unsigned int cha = (item - '0');
    string switchInfo = switchNames[cha];
    switchInfo += getStorageItemStatus(item, switchStorage) ? ": ON" : ": OFF";
    return switchInfo;
}

// вариант changeStorageStatus с использованием перечисления
void changeSwitchStatus (char item, OpType operation, unsigned int &switchStorage) {
    assert(std::isdigit(item));
    Switches choice;

    switch (item) {
        case ('0'): choice = Switches::MAIN; break;
        case ('1'): choice = Switches::INSIDE_LIGHT; break;
        case ('2'): choice = Switches::OUTSIDE_LIGHT; break;
        case ('3'): choice = Switches::PLUMBING; break;
        case ('4'): choice = Switches::HEATING; break;
        case ('5'): choice = Switches::CONDITIONER; break;
        case ('6'): choice = Switches::GARDEN_LIGHTING; break;
        default: choice = Switches::MAIN;
    }

    switch (operation) {
        case (OpType::ON):       switchStorage |= static_cast<unsigned int>(choice); break;
        case (OpType::OFF):      switchStorage &= ~ static_cast<unsigned int>(choice); break;
        case (OpType::REVERSAL): switchStorage ^= static_cast<unsigned int>(choice); break;
    }
}

// Меняет в store указанные в userInputString позиции на противоположные
void autoReverseStorageStatus (std::string const &items, unsigned int &switchStorage) {
    for (char c : items) {
        changeSwitchStatus(c, OpType::REVERSAL, switchStorage);
    }
}

bool plumbingController (unsigned int &switchStorage, const int* externalData) {
    const char PLUMBING = '3';
    bool isStatusChanges = false;
    const int outsideTemperature = externalData[static_cast<int>(ExternalData::OUTSIDE_TEMPERATURE)];

    if (outsideTemperature < 0 && !getStorageItemStatus(PLUMBING, switchStorage)) {
        changeSwitchStatus(PLUMBING, OpType::ON, switchStorage);
        isStatusChanges = true;
    } else if (outsideTemperature > 5 && getStorageItemStatus(PLUMBING, switchStorage)) {
        changeSwitchStatus(PLUMBING, OpType::OFF, switchStorage);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool heatingController (unsigned int &switchStorage, const int* externalData) {
    const char HEATING = '4';
    bool isStatusChanges = false;
    const int insideTemperature = externalData[static_cast<int>(ExternalData::INSIDE_TEMPERATURE)];

    if (insideTemperature < 22 && !getStorageItemStatus(HEATING, switchStorage)) {
        changeSwitchStatus(HEATING, OpType::ON, switchStorage);
        isStatusChanges = true;
    } else if (insideTemperature > 25 && getStorageItemStatus(HEATING, switchStorage)) {
        changeSwitchStatus(HEATING, OpType::OFF, switchStorage);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool conditionerController (unsigned int &switchStorage, const int* externalData) {
    const char CONDITIONER = '5';
    bool isStatusChanges = false;
    const int insideTemperature = externalData[static_cast<int>(ExternalData::INSIDE_TEMPERATURE)];

    if (insideTemperature > 30 && !getStorageItemStatus(CONDITIONER, switchStorage)) {
        changeSwitchStatus(CONDITIONER, OpType::ON, switchStorage);
        isStatusChanges = true;
    } else if (insideTemperature < 25 && getStorageItemStatus(CONDITIONER, switchStorage)) {
        changeSwitchStatus(CONDITIONER, OpType::OFF, switchStorage);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool gardenLightingController (unsigned int &switchStorage, const int* externalData) {
    const char GARDEN_LIGHTING = '6';
    bool isStatusChanges = false;
    const int isMotionDetection = externalData[static_cast<int>(ExternalData::MOTION_DETECTION)];
    const int time = externalData[static_cast<int>(ExternalData::TIME)];

    if (isMotionDetection && (time >= 16 || time <= 5) && !getStorageItemStatus(GARDEN_LIGHTING, switchStorage)) {
        changeSwitchStatus(GARDEN_LIGHTING, OpType::ON, switchStorage);
        isStatusChanges = true;
    } else if (getStorageItemStatus(GARDEN_LIGHTING, switchStorage)) {
        changeSwitchStatus(GARDEN_LIGHTING, OpType::OFF, switchStorage);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

int getRandomData (int from, int to) {
    return (from + std::rand() % (to - from + 1)); // NOLINT(cert-msc50-cpp)
}

void setCurrentExternalData (int* externalData) {
    ++externalData[static_cast<int>(ExternalData::TIME)];
    externalData[static_cast<int>(ExternalData::OUTSIDE_TEMPERATURE)] = getRandomData(-10, 40);
    externalData[static_cast<int>(ExternalData::INSIDE_TEMPERATURE)] = getRandomData(-10, 40);
    externalData[static_cast<int>(ExternalData::MOTION_DETECTION)] = getRandomData(0, 1);
}

void printSwitchStorage (const unsigned int &switchStorage) {
    const auto SIZE = static_cast<size_t>(Switches::STORAGE_SIZE);

    assert(SIZE == switchNames.size());

    cout << "------------------------------------\nSwitches status: \n";
    for (int currentSwitch = 0; currentSwitch < SIZE; ++currentSwitch) {
        // - выравнивание по левому краю, * даёт возможность указать длину поля
        printf("(%i) status of %-*s: %s\n",
               currentSwitch,
               16, switchNames[currentSwitch].c_str(),
               getStorageItemStatus(static_cast<char>(currentSwitch + '0'), switchStorage) ? "ON " : "OFF");
    }
}

void printChangedSwitches (vector<string> &changedSwitchesInfo) {
    cout << "List of changed states: " << endl;
    for (auto &i : changedSwitchesInfo) cout << i << endl;
}

void printExternalData (const int* externalData) {
    const auto SIZE = static_cast<size_t>(ExternalData::STORAGE_SIZE);
    vector<string> externalDataNames = {
            "Time",
            "Outside temperature",
            "Inside temperature ",
            "Is motion detection"
    };

    assert(SIZE == externalDataNames.size());

    cout << "------------------------------------\nData outside: \n";
    for (int i = 0; i < SIZE; ++i) {
        if (i == static_cast<int>(ExternalData::TIME)) {
            printf("%-*s: %02i:00\n",
                   30, externalDataNames[i].c_str(),
                   externalData[i]);
        } else {
            printf("%-*s: %i\n",
                   30, externalDataNames[i].c_str(),
                   externalData[i]);
        }
    }
}

void printReport(const unsigned int &switchStorage, const int* externalData) {
    printExternalData(externalData);
    printSwitchStorage(switchStorage);
    cout << "------------------------------------\n";
}

int main() {
    const char MAIN_SWITCH = '0';
    const char INSIDE_LIGHT = '1';
    const char OUTSIDE_LIGHT = '2';
    const char PLUMBING = '3';
    const char HEATING = '4';
    const char CONDITIONER = '5';
    const char GARDEN_LIGHTING = '6';
    unsigned int storage{0};
    int externalData[static_cast<int>(ExternalData::STORAGE_SIZE)] = {0};

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    while (true) {
        vector<string> changedSwitchesInfo;
        string userChoice;

        setCurrentExternalData(externalData);
        printReport(storage, externalData);

       bool isMainSwitchOn = getStorageItemStatus(MAIN_SWITCH, storage);
        if (getUserYesNo(!isMainSwitchOn ? "Smart home is off. Turn it on?" : "Smart home is on. Turn it off?")) {
            changeSwitchStatus(MAIN_SWITCH, OpType::REVERSAL, storage);
            changedSwitchesInfo.push_back(getStorageItemInfo(MAIN_SWITCH, storage));
        }

        if (getStorageItemStatus(MAIN_SWITCH, storage)) {
            if (getUserYesNo("Do you want to on/off devices?")) {
                userChoice = getUserChoiceFrom("12");
                autoReverseStorageStatus(userChoice, storage);
            }

            bool isPlumbingChanges = plumbingController(storage, externalData);
            if (isPlumbingChanges) changedSwitchesInfo.push_back(getStorageItemInfo(PLUMBING, storage));

            bool isHeatingChanges = heatingController(storage, externalData);
            if (isHeatingChanges) changedSwitchesInfo.push_back(getStorageItemInfo(HEATING, storage));

            bool isConditionerChanges = conditionerController(storage, externalData);
            if (isConditionerChanges) changedSwitchesInfo.push_back(getStorageItemInfo(CONDITIONER, storage));
        }

        if (getStorageItemStatus(INSIDE_LIGHT, storage)) {
            bool isGardenLightingChanges = gardenLightingController(storage, externalData);
            if (isGardenLightingChanges) changedSwitchesInfo.push_back(getStorageItemInfo(GARDEN_LIGHTING, storage));
        }

        if (!changedSwitchesInfo.empty()) {
            printChangedSwitches(changedSwitchesInfo);
            printReport(storage, externalData);
        }
    }
}