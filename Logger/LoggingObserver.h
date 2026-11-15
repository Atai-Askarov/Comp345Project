#ifndef LOGGINGOBSERVER_H
#define LOGGINGOBSERVER_H

#include <iostream>
#include <list>
#include <string>

/**
 * Interface for every class whose activity can be written to the game log.
 */
class ILoggable {
public:
    /** Virtual destructor so derived objects are destroyed correctly. */
    virtual ~ILoggable();

    /** Builds and returns the line to write in the log file. */
    virtual std::string stringToLog() const = 0;
};

/**
 * An Observer is told by a Subject when something worth logging happened.
 */
class Observer {
public:
    /** Default constructor. */
    Observer();

    /** Copy constructor. */
    Observer(const Observer& other);

    /** Assignment operator. */
    Observer& operator=(const Observer& other);

    /** Virtual destructor. */
    virtual ~Observer();

    /** Called by a Subject; 'loggable' is the object that changed. */
    virtual void update(ILoggable* loggable) = 0;

    /** Stream insertion operator. */
    friend std::ostream& operator<<(std::ostream& out, const Observer& observer);
};

/**
 * A Subject keeps a list of observers and notifies them when it changes.
 */
class Subject {
public:
    /** Creates a subject with no observers. */
    Subject();

    /** Copy constructor. The copy is watched by the same observers. */
    Subject(const Subject& other);

    /** Assignment operator. Copies the list of observers. */
    Subject& operator=(const Subject& other);

    /** Destructor. Deletes the list (not the observers, which are not owned). */
    virtual ~Subject();

    /** Adds an observer to the list. */
    void attach(Observer* observer);

    /** Removes an observer from the list. */
    void detach(Observer* observer);

    /** Calls update() on every attached observer. */
    void notify(ILoggable* loggable);

    /** Returns the list of attached observers. */
    std::list<Observer*>* getObservers() const;

    /** Stream insertion operator: prints the number of observers. */
    friend std::ostream& operator<<(std::ostream& out, const Subject& subject);

private:
    std::list<Observer*>* observers; // Observers to notify (not owned)
};

/**
 * Observer that writes every log entry into "gamelog.txt".
 */
class LogObserver : public Observer {
public:
    /** Default constructor. */
    LogObserver();

    /** Copy constructor. */
    LogObserver(const LogObserver& other);

    /** Assignment operator. */
    LogObserver& operator=(const LogObserver& other);

    /** Destructor. */
    ~LogObserver();

    /** Appends loggable->stringToLog() to gamelog.txt. */
    void update(ILoggable* loggable) override;

    /** Stream insertion operator. */
    friend std::ostream& operator<<(std::ostream& out, const LogObserver& observer);
};

#endif
