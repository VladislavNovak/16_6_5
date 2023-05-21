#include <iostream>
#include <algorithm> // any_of
#include <string>
#include <cassert>
#include <vector>
#include <cstdlib> // rand, srand
#include <ctime> // time

using std::cout;
using std::endl;
using std::string;
using std::vector;

enum class ExternalType { TIME, OUTSIDE_TEMPERATURE, INSIDE_TEMPERATURE, MOTION_DETECTION, COUNT };

enum class InternalType { BRIGHTNESS, COUNT };

// операции добавления/удаления/инверсии
enum class OpType { ON, OFF, REVERSAL };

enum class ToggleType {
    MAIN = 1,
    INSIDE_LIGHT = 2,
    OUTSIDE_LIGHT = 4,
    PLUMBING = 8,
    HEATING = 16,
    CONDITIONER = 32,
    GARDEN_LIGHTING = 64,
    COUNT = 7
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

std::string getJoinRange(const std::string &range) {
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

// Возвращает true при введении Y/y или false при N/n. Другие символы запрещены
bool hasDialogYesNo(const std::string &proposition) {
    printf("%s Press 'Y/y' to agree or 'N/n' to deny: ", proposition.c_str());
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
    while (curCount < maxCount) {
        char ch = getUserChar<char>(constraints);
        if (ch == TERMINATOR) break;
        if (!isRepeatAllowed && isIncludes(userInput, ch)) {
            printf("'%c' is already there in %s. Enter another or press the key '%c': ", ch, userInput.c_str(),
                   TERMINATOR);
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

// Возвращает занимаемую позицию в enum
int getTogglePosition(ToggleType const &item) {
    int position = 0;
    while (true) {
        if (((int)(item) >> position) == 1) return position;
        ++position;
    }
}

// Преобразовывает текущий enum в номер позиции. Возвращает в виде типа char
char getTogglePositionAsChar(ToggleType const &item) {
    int position = getTogglePosition((item));
    return static_cast<char>(position + '0');
}

// Поменять флаг указанного бита по указанной операции
void changeToggle(ToggleType item, OpType operation, unsigned int &store) {
    int choice = static_cast<int>(item);

    switch (operation) {
        case (OpType::ON):       store |= choice; break;
        case (OpType::OFF):      store &= ~choice; break;
        case (OpType::REVERSAL): store ^= choice; break;
    }
}

void resetAllToggles(unsigned int &store) {
    const auto COUNT = static_cast<size_t>(ToggleType::COUNT);
    for (int i = 0; i < COUNT; ++i) {
        changeToggle((ToggleType)(1 << i), OpType::OFF, store);
    }
}

// Получить состояние указанного бита
bool hasToggleFlag(ToggleType item, const unsigned int &store) {
    return bool(store & static_cast<int>(item));
}

// Получить отчет о бите в формате "status_bit : name_bit"
string getToggleInfo(ToggleType item, const unsigned int &store) {
    string switchInfo = hasToggleFlag(item, store) ? "ON:  " : ": OFF: ";
    switchInfo += switchNames[getTogglePosition(item)];
    return switchInfo;
}

bool plumbingController(unsigned int &store, const int* externalData) {
    bool isStatusChanges = false;
    ToggleType toggle = ToggleType::PLUMBING;
    const int outsideTemperature = externalData[static_cast<int>(ExternalType::OUTSIDE_TEMPERATURE)];

    if (outsideTemperature < 0 && !hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::ON, store);
        isStatusChanges = true;
    } else if (outsideTemperature > 5 && hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::OFF, store);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool heatingController(unsigned int &store, const int* externalData) {
    bool isStatusChanges = false;
    ToggleType toggle = ToggleType::HEATING;
    const int insideTemperature = externalData[static_cast<int>(ExternalType::INSIDE_TEMPERATURE)];

    if (insideTemperature < 22 && !hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::ON, store);
        isStatusChanges = true;
    } else if (insideTemperature > 25 && hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::OFF, store);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool conditionerController(unsigned int &store, const int* externalData) {
    bool isStatusChanges = false;
    ToggleType toggle = ToggleType::CONDITIONER;
    const int insideTemperature = externalData[static_cast<int>(ExternalType::INSIDE_TEMPERATURE)];

    if (insideTemperature > 30 && !hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::ON, store);
        isStatusChanges = true;
    } else if (insideTemperature < 25 && hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::OFF, store);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool gardenLightingController(unsigned int &store, const int* externalData) {
    bool isStatusChanges = false;
    ToggleType toggle = ToggleType::GARDEN_LIGHTING;
    const int isMotionDetection = externalData[static_cast<int>(ExternalType::MOTION_DETECTION)];
    const int time = externalData[static_cast<int>(ExternalType::TIME)];

    if (isMotionDetection && (time >= 16 || time <= 5) && !hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::ON, store);
        isStatusChanges = true;
    } else if (hasToggleFlag(toggle, store)) {
        changeToggle(toggle, OpType::OFF, store);
        isStatusChanges = true;
    }

    return isStatusChanges;
}

int getRandomIntInRange(int from, int to) {
    return (from + std::rand() % (to - from + 1)); // NOLINT(cert-msc50-cpp)
}

// Если start !=1, возвращает следующее значение по отношению к previous
int getCurrentTime(const int previous, int &start) {
    int current = start < 0 ? (previous + 1): start;

    // Сбрасываем start. Теперь будет действовать лишь previous
    if (start != -1) start = -1;
    return current > 23 ? 0 : current;
}

bool setBrightness(int* internalData, const int* externalData) {
    bool isStatusChanges = false;
    const int MAX_BRIGHT = 5000;
    const double REDUCTION = 11.5;
    unsigned int brightness;
    const int time = externalData[static_cast<int>(ExternalType::TIME)];

    if (time >= 0 && time <= 16) brightness = 0;
    else if (time == 17) brightness = 1;
    else if (time == 18) brightness = 2;
    else if (time == 19) brightness = 3;
    else brightness = 4;

    cout << "I: time:: " << time << endl;
    cout << "I: flag:: " << brightness << endl;
    int result = (int)(MAX_BRIGHT - (MAX_BRIGHT * REDUCTION / 100 * brightness));

    if (internalData[static_cast<int>(InternalType::BRIGHTNESS)] != result) {
        internalData[static_cast<int>(InternalType::BRIGHTNESS)] = result;
        isStatusChanges = true;
    }

    return isStatusChanges;
}

bool setBrightness2(unsigned int &currentBrightness, const int* externalData) {
    bool isStatusChanges = false;
    const int MAX_BRIGHT = 5000;
    const double REDUCTION = 11.5;
    unsigned int brightness;
    const int time = externalData[static_cast<int>(ExternalType::TIME)];

    if (time >= 0 && time <= 16) brightness = 0;
    else if (time == 17) brightness = 1;
    else if (time == 18) brightness = 2;
    else if (time == 19) brightness = 3;
    else brightness = 4;

    unsigned int result = (int)(MAX_BRIGHT - (MAX_BRIGHT * REDUCTION / 100 * brightness));

    if (currentBrightness != result) {
        currentBrightness = result;
        isStatusChanges = true;
    }

    return isStatusChanges;
}

void setExternalData(int *externalData, int &startTime) {
    externalData[static_cast<int>(ExternalType::TIME)] = getCurrentTime(externalData[static_cast<int>(ExternalType::TIME)], startTime);
    externalData[static_cast<int>(ExternalType::OUTSIDE_TEMPERATURE)] = getRandomIntInRange(-10, 40);
    externalData[static_cast<int>(ExternalType::INSIDE_TEMPERATURE)] = getRandomIntInRange(-10, 40);
    externalData[static_cast<int>(ExternalType::MOTION_DETECTION)] = getRandomIntInRange(0, 1);
}

void printSwitchStorage(const unsigned int &store) {
    const auto SIZE = static_cast<size_t>(ToggleType::COUNT);

    assert(SIZE == switchNames.size());

    cout << "------------------------------------\nToggleType status: \n";
    for (int currentToggleNumber = 0; currentToggleNumber < SIZE; ++currentToggleNumber) {
        // - выравнивание по левому краю, * даёт возможность указать длину поля
        printf("(%i) status of %-*s: %s\n",
               currentToggleNumber,
               16, switchNames[currentToggleNumber].c_str(),
               hasToggleFlag((ToggleType)(1 << currentToggleNumber), store) ? "ON " : "OFF");
    }
}

void printChangedTogglesInfo(vector<string> &changedTogglesInfo) {
    cout << "------------------------------------\nList of changed states: \n";
    for (auto &i: changedTogglesInfo) cout << i << endl;
}

void printExternalData(const int *externalData) {
    const auto SIZE = static_cast<size_t>(ExternalType::COUNT);
    vector<string> externalDataNames = {
            "Time",
            "Outside temperature",
            "Inside temperature ",
            "Is motion detection"
    };

    assert(SIZE == externalDataNames.size());

    cout << "------------------------------------\nData outside: \n";
    for (int i = 0; i < SIZE; ++i) {
        if (i == static_cast<int>(ExternalType::TIME)) {
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

void printReport(const unsigned int &store, const int *externalData) {
    printExternalData(externalData);
    printSwitchStorage(store);
    cout << "------------------------------------\n";
}

int main() {
    unsigned int togglesBox;
    resetAllToggles(togglesBox);
    int externalData[static_cast<int>(ExternalType::COUNT)] = {0};
    int internalData[static_cast<int>(InternalType::COUNT)] = {0};
    unsigned int currentBrightness = 0;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int startTime = getRandomIntInRange(0, 23);

    while (true) {
        vector<string> changedTogglesInfo;
        string userChoice;

        setExternalData(externalData, startTime);

        printReport(togglesBox, externalData);

        if (hasDialogYesNo(!hasToggleFlag(ToggleType::MAIN, togglesBox)
                         ? "Smart home is off. Turn it on?"
                         : "Smart home is on. Turn it off?")) {
            changeToggle(ToggleType::MAIN, OpType::REVERSAL, togglesBox);
            changedTogglesInfo.push_back(getToggleInfo(ToggleType::MAIN, togglesBox));
        }

        if (hasToggleFlag(ToggleType::MAIN, togglesBox)) {
            if (hasDialogYesNo("Do you want to on/off devices?")) {
                userChoice = getUserChoiceFrom("12");

                if (isIncludes(userChoice, getTogglePositionAsChar(ToggleType::INSIDE_LIGHT))) {
                    changeToggle(ToggleType::INSIDE_LIGHT, OpType::REVERSAL, togglesBox);
                    changedTogglesInfo.push_back(getToggleInfo(ToggleType::INSIDE_LIGHT, togglesBox));
                }
                if (isIncludes(userChoice, getTogglePositionAsChar(ToggleType::OUTSIDE_LIGHT))) {
                    changeToggle(ToggleType::OUTSIDE_LIGHT, OpType::REVERSAL, togglesBox);
                    changedTogglesInfo.push_back(getToggleInfo(ToggleType::OUTSIDE_LIGHT, togglesBox));
                }
            }

            bool isPlumbingChanges = plumbingController(togglesBox, externalData);
            if (isPlumbingChanges) changedTogglesInfo.push_back(getToggleInfo(ToggleType::PLUMBING, togglesBox));

            bool isHeatingChanges = heatingController(togglesBox, externalData);
            if (isHeatingChanges) changedTogglesInfo.push_back(getToggleInfo(ToggleType::HEATING, togglesBox));

            bool isConditionerChanges = conditionerController(togglesBox, externalData);
            if (isConditionerChanges) changedTogglesInfo.push_back(getToggleInfo(ToggleType::CONDITIONER, togglesBox));

            if (hasToggleFlag(ToggleType::INSIDE_LIGHT, togglesBox)) {
                setBrightness(internalData, externalData);
                // setBrightness(currentBrightness, externalData);
                cout << "BRI:: " << internalData[static_cast<int>(InternalType::BRIGHTNESS)] << endl;
                // cout << "BRI:: " << currentBrightness << endl;
            }

            if (hasToggleFlag(ToggleType::OUTSIDE_LIGHT, togglesBox)) {
                bool isGardenLightingChanges = gardenLightingController(togglesBox, externalData);
                if (isGardenLightingChanges) changedTogglesInfo.push_back(getToggleInfo(ToggleType::GARDEN_LIGHTING, togglesBox));
            }
        } else {
            resetAllToggles(togglesBox);
        }

        if (!changedTogglesInfo.empty()) {
            printChangedTogglesInfo(changedTogglesInfo);
            printReport(togglesBox, externalData);
        }
    }
}