#include "CommandProcessing.h"

#include "../GameEngine/GameEngine.h"

#include <algorithm>
#include <cctype>

namespace {

/** Removes whitespace at both ends of 'text'. */
std::string trim(const std::string& text) {
    size_t first = 0;
    size_t last = text.size();
    while (first < last && std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }
    return text.substr(first, last - first);
}

/** Returns true if 'name' is a command the user is allowed to enter. */
bool isUserCommand(const std::string& name) {
    return name == "loadmap" || name == "validatemap" || name == "addplayer" ||
           name == "gamestart" || name == "replay" || name == "quit";
}

/** Returns true if 'name' is a command that only the game itself triggers. */
bool isGameCommand(const std::string& name) {
    return name == "issueorder" || name == "endissueorders" || name == "execorder" ||
           name == "endexecorders" || name == "win";
}

/** Returns true if the user command 'name' must be followed by an argument. */
bool needsArgument(const std::string& name) {
    return name == "loadmap" || name == "addplayer";
}

} // namespace

// ------------------------------------------------------------------- Command

Command::Command(const std::string& text) : text(text), effect() {}

Command::Command(const Command& other) : text(other.text), effect(other.effect) {}

Command& Command::operator=(const Command& other) {
    if (this != &other) {
        text = other.text;
        effect = other.effect;
    }
    return *this;
}

Command::~Command() {}

std::string Command::getText() const {
    return text;
}

std::string Command::getName() const {
    std::string cleaned = trim(text);
    size_t end = cleaned.find_first_of(" \t");
    std::string name = cleaned.substr(0, end);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return name;
}

std::string Command::getArgument() const {
    std::string cleaned = trim(text);
    size_t end = cleaned.find_first_of(" \t");
    if (end == std::string::npos) {
        return "";
    }
    return trim(cleaned.substr(end));
}

std::string Command::getEffect() const {
    return effect;
}

void Command::saveEffect(const std::string& effectText) {
    effect = effectText;
}

std::ostream& operator<<(std::ostream& out, const Command& command) {
    out << "Command: " << command.text;
    if (!command.effect.empty()) {
        out << " | Effect: " << command.effect;
    }
    return out;
}

// ----------------------------------------------------------- CommandProcessor

CommandProcessor::CommandProcessor(GameEngine* engine) : engine(engine) {}

CommandProcessor::CommandProcessor(const CommandProcessor& other) : engine(other.engine) {
    copyCommands(other);
}

CommandProcessor& CommandProcessor::operator=(const CommandProcessor& other) {
    if (this != &other) {
        clearCommands();
        engine = other.engine;
        copyCommands(other);
    }
    return *this;
}

CommandProcessor::~CommandProcessor() {
    clearCommands();
}

Command* CommandProcessor::getCommand() {
    std::string commandText;
    if (!readCommand(commandText)) {
        return nullptr;
    }
    return saveCommand(commandText);
}

bool CommandProcessor::readCommand(std::string& commandText) {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::string cleaned = trim(line);
        if (!cleaned.empty()) {
            commandText = cleaned;
            return true;
        }
    }
    return false;
}

Command* CommandProcessor::saveCommand(const std::string& commandText) {
    Command* command = new Command(commandText);
    commands.push_back(command);
    return command;
}

bool CommandProcessor::validate(Command* command) {
    if (command == nullptr) {
        return false;
    }
    if (engine == nullptr) {
        command->saveEffect("Error: no game is attached to validate this command.");
        return false;
    }

    std::string name = command->getName();
    std::string argument = command->getArgument();
    std::string stateName = engine->getCurrentStateName();

    if (isGameCommand(name)) {
        command->saveEffect("Error: '" + name + "' is triggered by the game and cannot be entered by the user.");
        return false;
    }
    if (!isUserCommand(name)) {
        command->saveEffect("Error: '" + name + "' is not a known command.");
        return false;
    }
    if (needsArgument(name) && argument.empty()) {
        command->saveEffect("Error: '" + name + "' needs an argument (" +
                            (name == "loadmap" ? "a map file" : "a player name") + ").");
        return false;
    }
    if (!needsArgument(name) && !argument.empty()) {
        command->saveEffect("Error: '" + name + "' does not take an argument.");
        return false;
    }

    std::vector<std::string> allowed = engine->getValidCommands();
    if (std::find(allowed.begin(), allowed.end(), name) == allowed.end()) {
        command->saveEffect("Error: '" + name + "' is not valid in state [" + stateName + "].");
        return false;
    }
    return true;
}

const std::list<Command*>& CommandProcessor::getCommands() const {
    return commands;
}

void CommandProcessor::print(std::ostream& out) const {
    out << "CommandProcessor with " << commands.size() << " saved command(s)";
    for (const Command* command : commands) {
        out << "\n  " << *command;
    }
}

void CommandProcessor::clearCommands() {
    for (Command* command : commands) {
        delete command;
    }
    commands.clear();
}

void CommandProcessor::copyCommands(const CommandProcessor& other) {
    for (const Command* command : other.commands) {
        commands.push_back(new Command(*command));
    }
}

std::ostream& operator<<(std::ostream& out, const CommandProcessor& processor) {
    processor.print(out);
    return out;
}

// ------------------------------------------------------------- FileLineReader

FileLineReader::FileLineReader(const std::string& fileName) : fileName(fileName), linesRead(0) {
    openAndSkip(0);
}

FileLineReader::FileLineReader(const FileLineReader& other)
    : fileName(other.fileName), linesRead(0) {
    openAndSkip(other.linesRead);
}

FileLineReader& FileLineReader::operator=(const FileLineReader& other) {
    if (this != &other) {
        file.close();
        file.clear();
        fileName = other.fileName;
        linesRead = 0;
        openAndSkip(other.linesRead);
    }
    return *this;
}

FileLineReader::~FileLineReader() {
    file.close();
}

bool FileLineReader::isOpen() const {
    return file.is_open();
}

bool FileLineReader::readLineFromFile(std::string& line) {
    std::string raw;
    while (file.is_open() && std::getline(file, raw)) {
        std::string cleaned = trim(raw);
        if (!cleaned.empty()) {
            line = cleaned;
            ++linesRead;
            return true;
        }
    }
    return false;
}

void FileLineReader::openAndSkip(int linesToSkip) {
    file.open(fileName);
    std::string skipped;
    for (int i = 0; i < linesToSkip; ++i) {
        readLineFromFile(skipped);
    }
}

std::ostream& operator<<(std::ostream& out, const FileLineReader& reader) {
    out << "FileLineReader on \"" << reader.fileName << "\" ("
        << (reader.file.is_open() ? "open" : "not open") << ", " << reader.linesRead
        << " line(s) read)";
    return out;
}

// -------------------------------------------------- FileCommandProcessorAdapter

FileCommandProcessorAdapter::FileCommandProcessorAdapter(GameEngine* engine,
                                                         const std::string& fileName)
    : CommandProcessor(engine), fileReader(new FileLineReader(fileName)) {}

FileCommandProcessorAdapter::FileCommandProcessorAdapter(const FileCommandProcessorAdapter& other)
    : CommandProcessor(other), fileReader(new FileLineReader(*other.fileReader)) {}

FileCommandProcessorAdapter& FileCommandProcessorAdapter::operator=(
    const FileCommandProcessorAdapter& other) {
    if (this != &other) {
        CommandProcessor::operator=(other);
        FileLineReader* newReader = new FileLineReader(*other.fileReader);
        delete fileReader;
        fileReader = newReader;
    }
    return *this;
}

FileCommandProcessorAdapter::~FileCommandProcessorAdapter() {
    delete fileReader;
}

bool FileCommandProcessorAdapter::isReady() const {
    return fileReader->isOpen();
}

bool FileCommandProcessorAdapter::readCommand(std::string& commandText) {
    return fileReader->readLineFromFile(commandText);
}

void FileCommandProcessorAdapter::print(std::ostream& out) const {
    CommandProcessor::print(out);
    out << "\n  Reading from: " << *fileReader;
}
