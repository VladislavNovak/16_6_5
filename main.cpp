#include <iostream>
#include <algorithm> // any_of
#include <string>

using std::cout;
using std::endl;
using std::string;

enum ExternalData { OUTSIDE_TEMPERATURE, INSIDE_TEMPERATURE, MOTION_DETECTION, MAX_ITEMS };

enum DeviceStatus {
    MAIN = 1,
    INSIDE_LIGHT = 2,
    OUTSIDE_LIGHT = 4,
    PLUMBING = 8,
    HEATING = 16,
    CONDITIONER = 32,
    GARDEN_LIGHTING = 64,
};

// Получаем true если элемент `item` хоть раз встречается в диапазоне `range`
bool isIncludes(const string &range, const char &item) {
    return std::any_of(range.begin(),
                       range.end(),
                       [&item](const char &c) { return c == item; });
}

// Получаем char-символ в обозначенном диапазоне `range`
template<typename T>
T getUserChar(string const &range) {
    T input;

    while (true) {
        std::cin >> input;
        // Если введено значение из диапазона - сбрасываем
        if (isIncludes(range, input)) break;
        printf("Error. Symbol %c not in range: %s. Repeat: ", input, range.c_str());
        // Сбрасываем коматозное состояние cin
        std::cin.clear();
        // Очищаем поток ввода
        fflush(stdin);
    }

    return input;
}

// Получаем слово с параметрами:
//  - в указанном диапазоне `range`
//  - разрешено ли повторное введение символов. По умолчанию - не разрешено
//  - не более определенной длины (`maxCount`),
//  - с окончанием для ввода (`INPUT_ENDING`),
string getUserInput(const string &range, bool isRepeatAllowed = false, size_t maxCount = 128) {
    char const INPUT_ENDING = '.';
    string constraints = range;
    string userInput;
    size_t curCount = 0;

    constraints.append(1, INPUT_ENDING);

    // to format: zu - size_t, s - string.c_str, c - char
    printf("Enter one or more%s characters in the range: %s\n", (isRepeatAllowed ? "" : " unique"), constraints.c_str());
    printf("To complete, enter %zu characters or press the key '%c': ", maxCount, INPUT_ENDING);
    while(curCount < maxCount) {
        char ch = getUserChar<char>(constraints);
        if (ch == INPUT_ENDING) break;
        if (!isRepeatAllowed && isIncludes(userInput, ch)) {
            printf("Selected: %s. Symbol '%c' is already there. Enter another or press the key '%c': ", userInput.c_str(), ch, INPUT_ENDING);
            continue;
        }
        userInput += ch;
        ++curCount;
    }

    return userInput;
}

// Возвращает пользовательское слово с отсортированными уникальными символами, указанными в диапазоне constraint
string getUserChoice(size_t const devicesCount) {
    string constraint;
    // Набираем лишь символы от 0 до devicesCount
    for (int i = 0; i < devicesCount; ++i) constraint += static_cast<char>(i + '0');

    string userInput = getUserInput(constraint, false, devicesCount);
    std::sort(userInput.begin(), userInput.end());

    return userInput;
}

void printDeviceStatus (const unsigned int &store, size_t DEVICES_COUNT) {
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
               bool(store & (1 << currentDevice)) ? "ON " : "OFF");
    }
}

void printReportForHour(int hour, const unsigned int* externalDataStates, const unsigned int &switchesState, const size_t DEVICES_COUNT) {

    string externalDataNames[] = { "Outside temperature", "Inside temperature ", "Is motion detection" };

    cout << "-----------" << hour << "--------------------\n";
    cout << "Data outside: \n";
    for (int i = 0; i < ExternalData::MAX_ITEMS; ++i) {
        printf("%s: %14i\n", externalDataNames[i].c_str(), externalDataStates[i]);
    }

    cout << "-------------------------------\nCurrent switches status: \n";
    printDeviceStatus(switchesState, DEVICES_COUNT);
    cout << "-------------------------------\n";
}

int main() {
    const int DEVICES_COUNT = 7;
    unsigned int externalDataStates[ExternalData::MAX_ITEMS] = {0};
    unsigned int switchesState{0};
    string userChoice;

    //------hour 00:00

    externalDataStates[ExternalData::OUTSIDE_TEMPERATURE] = 12;
    externalDataStates[ExternalData::INSIDE_TEMPERATURE] = 23;
    externalDataStates[ExternalData::MOTION_DETECTION] = 0;

    printReportForHour(0, externalDataStates, switchesState, DEVICES_COUNT);

    cout << "Do you want to enable/disable devices?\n";
    cout << "Enter 'Y' to on/off devices or 'N' to skip until next hour: ";
    char YES_NO = getUserChar<char>("YyNn");
    if (YES_NO == 'Y' || YES_NO == 'y') {
        userChoice = getUserChoice(DEVICES_COUNT);

        cout << userChoice << endl;
    }
}