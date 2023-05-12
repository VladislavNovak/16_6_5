#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

enum Switches {
    MAIN = 1,
    INSIDE_LIGHT = 2,
    OUTSIDE_LIGHT = 4,
    PLUMBING = 8,
    HEATING = 16,
    CONDITIONER = 32,
    GARDEN_LIGHTING = 64,
};

int main() {
    string switchNames[] = {
            "all",
            "inside light",
            "outside light",
            "plumbing",
            "heating",
            "conditioner",
            "garden lighting"
    };

    unsigned int switchStates[5] = {0};

    // switchStates[0] &= ~Switches::LIGHTS_INSIDE;
    // switchStates[1] &= ~Switches::LIGHTS_OUTSIDE;
    // switchStates[2] &= ~Switches::HEATERS;
    // switchStates[3] &= ~Switches::WATER_PIPE_HEATING;
    // switchStates[4] &= ~Switches::CONDITIONER;


    // // Включить. Результат: 4
    // switchState = switchState | Switches::HEATERS;
    // cout << switchState << endl;
    //
    // // Выключить. Результат: 0
    // switchState = switchState & ~Switches::HEATERS;
    // cout << switchState << endl;

    for (auto i : switchStates) {
        cout << i << endl;
    }

}