#include "LoggingObserver.h"

#include <fstream>

// ------------------------------------------------------------- ILoggable

ILoggable::~ILoggable() {}

// ------------------------------------------------------------- Observer

Observer::Observer() {}

Observer::Observer(const Observer& other) {}

Observer& Observer::operator=(const Observer& other) {
    return *this;
}

Observer::~Observer() {}

std::ostream& operator<<(std::ostream& out, const Observer& observer) {
    out << "Observer";
    return out;
}

// ------------------------------------------------------------- Subject

Subject::Subject() {
    observers = new std::list<Observer*>();
}

Subject::Subject(const Subject& other) {
    observers = new std::list<Observer*>(*other.observers);
}

Subject& Subject::operator=(const Subject& other) {
    if (this != &other) {
        *observers = *other.observers;
    }
    return *this;
}

Subject::~Subject() {
    delete observers;
}

void Subject::attach(Observer* observer) {
    observers->push_back(observer);
}

void Subject::detach(Observer* observer) {
    observers->remove(observer);
}

void Subject::notify(ILoggable* loggable) {
    for (Observer* observer : *observers) {
        observer->update(loggable);
    }
}

std::list<Observer*>* Subject::getObservers() const {
    return observers;
}

std::ostream& operator<<(std::ostream& out, const Subject& subject) {
    out << "Subject with " << subject.observers->size() << " observer(s)";
    return out;
}

// ------------------------------------------------------------- LogObserver

LogObserver::LogObserver() {}

LogObserver::LogObserver(const LogObserver& other) : Observer(other) {}

LogObserver& LogObserver::operator=(const LogObserver& other) {
    Observer::operator=(other);
    return *this;
}

LogObserver::~LogObserver() {}

void LogObserver::update(ILoggable* loggable) {
    std::ofstream file("gamelog.txt", std::ios::app);
    file << loggable->stringToLog() << std::endl;
}

std::ostream& operator<<(std::ostream& out, const LogObserver& observer) {
    out << "LogObserver writing to gamelog.txt";
    return out;
}
