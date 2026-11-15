#ifndef COMMANDPROCESSING_H
#define COMMANDPROCESSING_H
#include "../Logger/LoggingObserver.h"

#include <fstream>
#include <iostream>
#include <list>
#include <string>

class GameEngine;

/**
 * A Command is one line typed by the user (or read from a file), for example
 * "loadmap europe.map". It stores the original text and, once the command has
 * been handled, a string describing its effect (or the reason it was rejected).
 */
class Command : public Subject, public ILoggable {
public:
    /** Creates a command from the text that was read; its effect starts empty. */
    explicit Command(const std::string& text);

    /** Copy constructor. */
    Command(const Command& other);

    /** Assignment operator. */
    Command& operator=(const Command& other);

    /** Destructor. */
    ~Command();

    /** Returns the full text of the command exactly as it was read. */
    std::string getText() const;

    /** Returns the command word (first word of the text) in lower case. */
    std::string getName() const;

    /** Returns everything after the command word (map file or player name), trimmed. */
    std::string getArgument() const;

    /** Returns the saved effect, or an empty string if none was saved yet. */
    std::string getEffect() const;

    /** Records the effect of this command (or the error that rejected it). */
    void saveEffect(const std::string& effectText);

    /** Returns the log line for this command's effect. */
    std::string stringToLog() const override;

    /** Stream insertion: prints the command text and its effect. */
    friend std::ostream& operator<<(std::ostream& out, const Command& command);

private:
    std::string text;   // Text of the command as it was read
    std::string effect; // Effect of the command, or the error message if it was rejected
};

/**
 * A CommandProcessor gets commands from the console, stores them in a list of
 * Command objects and checks whether they are valid in the current game state.
 * Game components must obtain their commands through getCommand().
 */
class CommandProcessor : public Subject, public ILoggable {
public:
    /** Creates a processor that validates commands against the state of 'engine' (not owned). */
    explicit CommandProcessor(GameEngine* engine);

    /** Copy constructor. Deep-copies the saved commands; the engine is shared. */
    CommandProcessor(const CommandProcessor& other);

    /** Assignment operator. Deep-copies the saved commands; the engine is shared. */
    CommandProcessor& operator=(const CommandProcessor& other);

    /** Virtual destructor. Deletes every saved command. */
    virtual ~CommandProcessor();

    /**
     * Reads the next command, saves it in the list and returns it.
     * Returns nullptr when there is no more input. The returned Command is owned
     * by this processor.
     */
    Command* getCommand();

    /**
     * Checks that 'command' is valid in the current game state. If it is not, an
     * error message is saved in the command's effect. Returns true when valid.
     */
    bool validate(Command* command);

    /** Returns all the commands saved so far, oldest first. */
    const std::list<Command*>& getCommands() const;

    /** Stream insertion: prints every saved command with its effect. */
    friend std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor);

    /** Returns the log line for the most recently saved command. */
    std::string stringToLog() const override;

protected:
    /**
     * Reads one non-empty line from the console into 'commandText'.
     * Returns false when the input has ended.
     */
    virtual bool readCommand(std::string& commandText);

    /** Creates a Command from 'commandText', stores it in the list and returns it. */
    Command* saveCommand(const std::string& commandText);

    /** Writes the description used by operator<<; subclasses extend it. */
    virtual void print(std::ostream& out) const;

private:
    GameEngine* engine;            // Game whose current state decides validity (not owned)
    std::list<Command*> commands;  // Commands read so far (owned)

    /** Deletes every saved command. */
    void clearCommands();

    /** Deep-copies the commands of 'other' into this (empty) processor. */
    void copyCommands(const CommandProcessor& other);
};

/**
 * A FileLineReader reads a text file one non-empty line at a time.
 * It is the "adaptee" of the FileCommandProcessorAdapter.
 */
class FileLineReader {
public:
    /** Opens 'fileName' for reading. Use isOpen() to find out whether it worked. */
    explicit FileLineReader(const std::string& fileName);

    /** Copy constructor. Reopens the file and skips the lines already read. */
    FileLineReader(const FileLineReader& other);

    /** Assignment operator. Reopens the file and skips the lines already read. */
    FileLineReader& operator=(const FileLineReader& other);

    /** Destructor. Closes the file. */
    ~FileLineReader();

    /** Returns true if the file was opened successfully. */
    bool isOpen() const;

    /** Reads the next non-empty line into 'line'. Returns false at the end of the file. */
    bool readLineFromFile(std::string& line);

    /** Stream insertion: prints the file name and the number of lines read. */
    friend std::ostream& operator<<(std::ostream& out, const FileLineReader& reader);

private:
    std::string fileName;  // Name of the file being read
    std::ifstream file;    // Open stream on the file
    int linesRead;         // Number of non-empty lines returned so far

    /** Opens the file and skips 'linesToSkip' non-empty lines. */
    void openAndSkip(int linesToSkip);
};

/**
 * Adapter that lets the game read its commands from a saved text file instead of
 * the console. It reuses everything in CommandProcessor and only replaces
 * readCommand() so that the text comes from a FileLineReader.
 */
class FileCommandProcessorAdapter : public CommandProcessor {
public:
    /** Creates an adapter that reads commands from 'fileName'. */
    FileCommandProcessorAdapter(GameEngine* engine, const std::string& fileName);

    /** Copy constructor. Deep-copies the file reader. */
    FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other);

    /** Assignment operator. Deep-copies the file reader. */
    FileCommandProcessorAdapter& operator=(const FileCommandProcessorAdapter& other);

    /** Destructor. Deletes the file reader. */
    ~FileCommandProcessorAdapter() override;

    /** Returns true if the command file was opened successfully. */
    bool isReady() const;

protected:
    /** Reads the next non-empty line of the file into 'commandText'. */
    bool readCommand(std::string& commandText) override;

    /** Extends the base description with the file reader. */
    void print(std::ostream& out) const override;

private:
    FileLineReader* fileReader; // Source of the commands (owned)
};

#endif
