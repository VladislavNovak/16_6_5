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
    // assert(cha < storageSize);
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
void autoReverseStorageStatus (unsigned int &switchStorage, std::string const &userInputString) {
    for (char c : userInputString) {
        changeSwitchStatus(c, OpType::REVERSAL, switchStorage);
    }
}

void plumbingController (unsigned int &switchStorage, const int* externalData) {
    const char PLUMBING = '3';
    const int outsideTemperature = externalData[static_cast<int>(ExternalData::OUTSIDE_TEMPERATURE)];

    if (outsideTemperature < 0 && !getStorageItemStatus(PLUMBING, switchStorage)) {
        changeSwitchStatus(PLUMBING, OpType::ON, switchStorage);
    } else if (outsideTemperature > 5 && getStorageItemStatus(PLUMBING, switchStorage)) {
        changeSwitchStatus(PLUMBING, OpType::OFF, switchStorage);
    }
}

void heatingController (unsigned int &switchStorage, const int* externalData) {
    const char HEATING = '4';
    const int insideTemperature = externalData[static_cast<int>(ExternalData::INSIDE_TEMPERATURE)];

    if (insideTemperature < 22 && !getStorageItemStatus(HEATING, switchStorage)) {
        changeSwitchStatus(HEATING, OpType::ON, switchStorage);
    } else if (insideTemperature > 25 && getStorageItemStatus(HEATING, switchStorage)) {
        changeSwitchStatus(HEATING, OpType::OFF, switchStorage);
    }
}

void conditionerController (unsigned int &switchStorage, const int* externalData) {
    const char CONDITIONER = '5';
    const int insideTemperature = externalData[static_cast<int>(ExternalData::INSIDE_TEMPERATURE)];

    if (insideTemperature > 30 && !getStorageItemStatus(CONDITIONER, switchStorage)) {
        changeSwitchStatus(CONDITIONER, OpType::ON, switchStorage);
    } else if (insideTemperature < 25 && getStorageItemStatus(CONDITIONER, switchStorage)) {
        changeSwitchStatus(CONDITIONER, OpType::OFF, switchStorage);
    }
}

void gardenLightingController (unsigned int &switchStorage, const int* externalData) {
    const char GARDEN_LIGHTING = '6';
    const int isMotionDetection = externalData[static_cast<int>(ExternalData::MOTION_DETECTION)];
    const int time = externalData[static_cast<int>(ExternalData::TIME)];

    if (isMotionDetection && (time >= 16 || time <= 5) && !getStorageItemStatus(GARDEN_LIGHTING, switchStorage)) {
        changeSwitchStatus(GARDEN_LIGHTING, OpType::ON, switchStorage);
    } else if (getStorageItemStatus(GARDEN_LIGHTING, switchStorage)) {
        changeSwitchStatus(GARDEN_LIGHTING, OpType::OFF, switchStorage);
    }
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
    vector<string> switchNames = {
            "all",
            "inside light",
            "outside light",
            "plumbing",
            "heating",
            "conditioner",
            "garden lighting"
    };

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
        printf("%-*s: %i\n",
               30, externalDataNames[i].c_str(),
               externalData[i]);
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
    unsigned int storage{0};
    int externalData[static_cast<int>(ExternalData::STORAGE_SIZE)] = {0};
    string userChoice;

    srand(time(NULL));

    //------hour 00:00

    setCurrentExternalData(externalData);

    printReport(storage, externalData);

    if (!getStorageItemStatus(MAIN_SWITCH, storage)) {
        if (getUserYesNo("Smart home is off. Turn it on?")) {
            changeSwitchStatus(MAIN_SWITCH, OpType::ON, storage);
        }
    }

    if (getStorageItemStatus(MAIN_SWITCH, storage)) {
        if (getUserYesNo("Do you want to on/off devices?")) {
            userChoice = getUserChoiceFrom("12");
            autoReverseStorageStatus(storage, userChoice);

            plumbingController(storage, externalData);
            heatingController(storage, externalData);
            conditionerController(storage, externalData);
        }
    }

    if (getStorageItemStatus(INSIDE_LIGHT, storage)) {
        gardenLightingController(storage, externalData);
    }

    printReport(storage, externalData);
}