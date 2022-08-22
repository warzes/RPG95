#pragma once

#include <iostream>

class timer {

private:
    std::chrono::steady_clock::time_point begin;
    bool verbose;
    std::stringstream logfile;

public:
    timer();
    timer(bool b);
    void start();
    void toc();
    void toc(std::string s);
    void stop();
    void stop(std::string s);
    void dump();
};

timer::timer() {
    verbose = true;
    this->start();
}

timer::timer(bool b) {
    verbose = b;
    this->start();
}

void timer::start() {
    begin = std::chrono::steady_clock::now();
}

void timer::stop(std::string s) {

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    if (verbose) {
        std::cout << s.c_str() << " Time elapsed = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1e6 << std::endl;
    }

    logfile << s << "," << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1e6 << std::endl;
}

void timer::stop() {
    this->stop("");
}

void timer::toc() {
    this->toc("");
}

void timer::toc(std::string s) {
    this->stop(s);
    this->start();
}

void timer::dump() {
    //ofstream myfile;
    //myfile.open("/tmp/timer.csv");
    //myfile << logfile.rdbuf();
    //myfile.close();
}