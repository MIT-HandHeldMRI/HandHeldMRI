// arduino_stubs.h
#pragma once
#include <cstdint>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

// --- Empty macros ---
#define PI 3.1415
#define LOW 0
#define HIGH 1
#define GR_Y 0
#define GR_Z 1
#define GR_OFF 0
#define STARTSEQ0 0
#define STARTSEQ1 1
#define READOUT 2
#define ZZ
#define TR
#define CLAMP
#define RELEASE
#define GR_POS 1
#define GR_NEG -1
#define A10 10
#define A11 11

#define SIGN(x) ((x) > 0 ? 1 : -1)

#define ARM_DWT_CYCCNT (ArduinoSim::current_time_us * 600)

// --- Internal state tracking ---
namespace ArduinoSim {
    inline double current_time_us = 0.0;
    inline double grad_amp = 0.0;
    inline double rf_amp = 0.0;
    inline double grad_y = 0.0;
    inline double grad_z = 0.0;
    inline int numiters = 0;
    inline std::ofstream logfile;
    inline bool matlab_trigger = false;

    inline void initLog(int iter) {
        if (!logfile.is_open()) {
            logfile.open("outputs/log_" + std::to_string(iter) + ".csv");
            logfile << "time_us,grad_y,grad_z,rf_amp\n";
            logfile << "0,0,0,0\n";
        }
    }

    inline void logState(int grady, int gradz, int rfamp) {
        // initLog();
        logfile << current_time_us << "," << grady << "," << gradz << "," << rfamp << "\n";
    }

    struct Cleanup {
        ~Cleanup() {
            if (logfile.is_open()) {
                logfile.flush();
                logfile.close();
                // std::cout << "[ArduinoSim] Log written to waveform_log.csv\n";
            }
        }
    };
    inline Cleanup cleanup;
}

// --- Dummy hardware functions ---
inline void pinMode(int, int) {}
inline void digitalWriteFast(int pin, int output) {
    using namespace ArduinoSim;
    if (pin == READOUT && output == HIGH && matlab_trigger == false) {
        if (numiters < 80) {
            // First encounter: simulate waiting, do not yet trigger
            numiters ++;
            ArduinoSim::Cleanup();
            initLog(numiters);
            current_time_us = 0.0;
        } else {
            // Second encounter: trigger event
            ArduinoSim::Cleanup();   // flush logs, close files, etc.
            std::cout << "[ArduinoSim] Simulation done — exiting.\n";
            std::exit(0);            // ← terminate immediately
        }
    }
}

inline bool digitalRead(int pin) {
    using namespace ArduinoSim;
    if (pin == STARTSEQ1) {
        matlab_trigger = true;
        if (numiters < 80) {
            // First encounter: simulate waiting, do not yet trigger
            numiters ++;
            ArduinoSim::Cleanup();
            initLog(numiters);
            current_time_us = 0.0;
        } else {
            // Second encounter: trigger event
            ArduinoSim::Cleanup();   // flush logs, close files, etc.
            std::cout << "[ArduinoSim] Simulation done — exiting.\n";
            std::exit(0);            // ← terminate immediately
        }
    }
    return true;
}

inline uint16_t analogRead(int pin) {
    return 1023;
}

inline void delay(unsigned long ms) {
    using namespace ArduinoSim;
    if(ms < 500){
        current_time_us += ms * 1000.0;
    }
}

inline void delayMicroseconds(unsigned long us) {
    using namespace ArduinoSim;
    current_time_us += us;
}

inline void Serial_begin(int) {}
inline void Serial_println(const char* s) { std::cout << s << std::endl; }
inline void Serial_print(const char* s) { std::cout << s; }

// --- Simulated functions ---
inline void AFE_setup() {}
inline void GPIO_setup() {}
inline void set_arm_clock(int) {}
inline void initCycleCounter() {}

inline void set_gradient_amplitude(uint8_t amp) {
    using namespace ArduinoSim;
    grad_amp = amp;
}

inline void set_rf_amplitude(uint8_t amp) {
    using namespace ArduinoSim;
    rf_amp = amp;
}

inline void generatePulses(double /*freq*/, uint8_t pulse_time_us) {
    using namespace ArduinoSim;
    logState(grad_y, grad_z, rf_amp);
    delayMicroseconds(pulse_time_us);
    logState(grad_y, grad_z, 0);
}

inline void gradient(int gradient, int polarity) {
    using namespace ArduinoSim;
    if (gradient == GR_Y) grad_y = grad_amp * polarity;
        else if (gradient == GR_Z) grad_z = rf_amp * polarity;

    logState(grad_y, grad_z, 0);
}
