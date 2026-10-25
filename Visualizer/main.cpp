#include "arduino_stubs.h"
#include "code.cpp"  // your Arduino sketch file

int main() {
    // Initialize logging
    // ArduinoSim::initLog();

    // Run Arduino lifecycle
    setup();

    // Simulate one pass of the Arduino loop (or more if desired)
    loop();

    return 0;
}
