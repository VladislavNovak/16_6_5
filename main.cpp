#include <iostream>
#include <algorithm> // any_of
#include <string>
#include <cassert>
#include <cctype>

using std::cout;
using std::endl;
using std::string;

enum class ExternalData { OUTSIDE_TEMPERATURE, INSIDE_TEMPERATURE, MOTION_DETECTION, AMOUNT };

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
    AMOUNT = 7
};

// LIB
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

// LIB
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

// SNIPPET
bool getUserYesNo (const std::string &proposition) {
    printf("%s Press 'Y' to agree or 'N' to skip: ", proposition.c_str());
    return isIncludes("Yy", getUserChar<char>("YynNn"));
}

// LIB
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

// SNIPPET
// Возвращает пользовательское слово с отсортированными уникальными символами, указанными в диапазоне range
string getUserChoiceFrom(const std::string &range) {
    string userInput = getUserInput(range, false, range.size());
    std::sort(userInput.begin(), userInput.end());

    return userInput;
}

// LIB
// Меняет в переменной типа unsigned int указанный в changeItem бит
void changeStorageStatus (unsigned int &store, OpType operation, char numberItem) {
    assert(std::isdigit(numberItem));
    unsigned int cha = (numberItem - '0');
    unsigned int choice = (1 << cha);

    switch (operation) {
        case (OpType::ON):       store |= choice; break;
        case (OpType::OFF):      store &= ~ choice; break;
        case (OpType::REVERSAL): store ^= choice; break;
    }
}

// LIB candidate (нужно добавить для безопасности try/catch)
bool getStorageItemStatus (const unsigned int &store, char numberItem) {
    assert(std::isdigit(numberItem));
    unsigned int cha = (numberItem - '0');
    unsigned int choice = (1 << cha);
    return bool(store & choice);
}

// SNIPPET вариант changeStorageStatus с использованием перечисления
void changeSwitchStatus (unsigned int &store, OpType operation, char device) {
    assert(std::isdigit(device));
    Switches choice;

    switch (device) {
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
        case (OpType::ON):       store |= static_cast<unsigned int>(choice); break;
        case (OpType::OFF):      store &= ~ static_cast<unsigned int>(choice); break;
        case (OpType::REVERSAL): store ^= static_cast<unsigned int>(choice); break;
    }
}

// Меняет в store указанные в userInputString позиции на противоположные
void autoReverseStorageStatus (unsigned int &store, std::string const &userInputString) {
    for (char c : userInputString) {
        changeSwitchStatus(store, OpType::REVERSAL, c);
    }
}

void offAllStorageStatus (unsigned int &store) {

}

void printDeviceStatus (const unsigned int &switchesState) {
    const auto DEVICES_COUNT = static_cast<size_t>(Switches::AMOUNT);
    string deviceNames[] = {
            "all",
            "inside light",
            "outside light",
            "plumbing",
            "heating",
            "conditioner",
            "garden lighting"
    };

    for (int currentDevice = 0; currentDevice < DEVICES_COUNT; ++currentDevice) {
        printf("(%i) status of %-*s: %s\n",
               currentDevice,
               16, deviceNames[currentDevice].c_str(),
               bool(switchesState & (1 << currentDevice)) ? "ON " : "OFF");
    }
}

void printExternalStatus (const unsigned int* externalDataStates) {
    string externalDataNames[] = { "Outside temperature", "Inside temperature ", "Is motion detection" };

    cout << "Data outside: \n";
    for (int i = 0; i < static_cast<int>(ExternalData::AMOUNT); ++i) {
        printf("%s: %14i\n", externalDataNames[i].c_str(), externalDataStates[i]);
    }
}

void printReportForHour(int hour, const unsigned int* externalDataStates, const unsigned int &switchesState) {
    cout << "-----------" << hour << "--------------------\n";
    printExternalStatus(externalDataStates);
    cout << "-------------------------------\nCurrent switches status: \n";
    printDeviceStatus(switchesState);
    cout << "-------------------------------\n";
}

int main() {
    const char MAIN_SWITCH = '0';
    const char INSIDE_LIGHT = '1';
    const char OUTSIDE_LIGHT = '2';
    unsigned int storage{0};
    unsigned int externalDataStates[static_cast<int>(ExternalData::AMOUNT)] = {0};
    string userChoice;

    //------hour 00:00

    externalDataStates[static_cast<int>(ExternalData::OUTSIDE_TEMPERATURE)] = 12;
    externalDataStates[static_cast<int>(ExternalData::INSIDE_TEMPERATURE)] = 23;
    externalDataStates[static_cast<int>(ExternalData::MOTION_DETECTION)] = 0;

    printReportForHour(0, externalDataStates, storage);

    if (!getStorageItemStatus(storage, MAIN_SWITCH)) {
        if (getUserYesNo("Smart home is off. Turn it on?")) {
            changeSwitchStatus(storage, OpType::ON, MAIN_SWITCH);
        };
    }

    if (getStorageItemStatus(storage, MAIN_SWITCH)) {
        if (getUserYesNo("Do you want to on/off devices?")) {
            userChoice = getUserChoiceFrom("12");
            autoReverseStorageStatus(storage, userChoice);
        }
    }

    bool isInsideLightOn = getStorageItemStatus(storage, INSIDE_LIGHT);
    bool isOutsideLightOn = getStorageItemStatus(storage, OUTSIDE_LIGHT);
    if (isInsideLightOn || isOutsideLightOn) {

    }

    cout << "-------------------------------\nNew switches status: \n";
    printDeviceStatus(storage);
}