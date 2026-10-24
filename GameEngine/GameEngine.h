#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <iostream>
#include <string>
#include <vector>

class State;

/**
 * A Transition is a directed edge of the game's state graph.
 * It is labelled with the console command that triggers it and
 * points to the State that is reached when the command is accepted.
 * The target State is not owned by the Transition (it is owned by the GameEngine).
 */
class Transition {
public:
    /** Creates a transition labelled with 'command' that leads to 'target'. */
    Transition(const std::string& command, State* target);

    /** Copy constructor. The copy points to the same (non-owned) target state. */
    Transition(const Transition& other);

    /** Assignment operator. The target state is shared, not duplicated. */
    Transition& operator=(const Transition& other);

    /** Destructor. Does not delete the target state since it is not owned. */
    ~Transition();

    /** Returns the command that triggers this transition. */
    std::string getCommand() const;

    /** Returns the state reached by this transition. */
    State* getTarget() const;

    /** Redirects this transition to another target state. */
    void setTarget(State* newTarget);

    /** Stream insertion: prints the command and the name of the target state. */
    friend std::ostream& operator<<(std::ostream& out, const Transition& transition);

private:
    std::string command; // Command typed by the user to take this edge
    State* target;       // Destination state (not owned)
};

/**
 * A State represents one phase of the game. It owns the list of
 * outgoing transitions, which defines the commands that are valid in this phase.
 */
class State {
public:
    /** Creates a state with the given name and no outgoing transitions. */
    explicit State(const std::string& name);

    /**
     * Copy constructor. Transitions are deep copied, but they keep pointing
     * at the original target states (GameEngine re-links them when it is copied).
     */
    State(const State& other);

    /** Assignment operator. Deep copies the outgoing transitions. */
    State& operator=(const State& other);

    /** Destructor. Deletes the outgoing transitions owned by this state. */
    ~State();

    /** Returns the name of the state. */
    std::string getName() const;

    /** Adds an outgoing transition labelled 'command' leading to 'target'. */
    void addTransition(const std::string& command, State* target);

    /** Returns the transition labelled 'command', or nullptr if there is none. */
    Transition* findTransition(const std::string& command) const;

    /** Returns all outgoing transitions of this state. */
    const std::vector<Transition*>& getTransitions() const;

    /** Returns true if the state has no outgoing transition (end of the game). */
    bool isTerminal() const;

    /** Stream insertion: prints the state name and its outgoing transitions. */
    friend std::ostream& operator<<(std::ostream& out, const State& state);

private:
    std::string name;                      // Name of the game phase
    std::vector<Transition*> transitions;  // Outgoing edges (owned)

    /** Deletes every owned transition. */
    void clearTransitions();
};

/**
 * The GameEngine controls the flow of the game. It owns the state graph
 * and keeps track of the current state.
 * Commands that match an outgoing transition of the current state move the
 * engine to the next state; any other command is rejected.
 */
class GameEngine {
public:
    /** Builds the state graph and places the engine in the "start" state. */
    GameEngine();

    /** Copy constructor. Deep copies the whole state graph and the current state. */
    GameEngine(const GameEngine& other);

    /** Assignment operator. Deep copies the whole state graph and the current state. */
    GameEngine& operator=(const GameEngine& other);

    /** Destructor. Deletes every state (and, through them, every transition). */
    ~GameEngine();

    /**
     * Processes a command typed by the user. Surrounding whitespace and letter
     * case are ignored. Returns true and changes state if the command is valid
     * for the current state; otherwise prints an error and returns false.
     */
    bool processCommand(const std::string& command);

    /** Returns the current state. */
    const State* getCurrentState() const;

    /** Returns the name of the current state. */
    std::string getCurrentStateName() const;

    /** Returns the commands that are valid in the current state. */
    std::vector<std::string> getValidCommands() const;

    /** Returns true once the game has reached its final state. */
    bool isFinished() const;

    /** Stream insertion: prints the current state and the whole state graph. */
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& engine);

private:
    std::vector<State*> states; // Every state of the game (owned)
    State* currentState;        // State the game is currently in (not owned separately)

    /** Creates the states and transitions of the assignment's state diagram. */
    void buildStateGraph();

    /** Creates a state, stores it in the engine and returns it. */
    State* addState(const std::string& name);

    /** Deep copies the graph of 'other' into this (empty) engine. */
    void copyFrom(const GameEngine& other);

    /** Deletes all states and resets the current state. */
    void clear();
};

#endif
