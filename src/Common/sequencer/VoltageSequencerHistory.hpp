#include <vector>
#include <stack>

struct Action {
    int index;      // Sequencer step index
    float oldValue; // Value before the change
    float newValue; // Value after the change

    Action(int idx, float oldVal, float newVal)
        : index(idx), oldValue(oldVal), newValue(newVal) {}
};

class Session {
    std::vector<Action> actions;

public:
    // Add an action to the session
    void addAction(int index, float oldValue, float newValue) 
    {
        actions.emplace_back(index, oldValue, newValue);
    }

    // Get all actions in the session (for undo/redo purposes)
    const std::vector<Action>& getActions() const 
    {
        return actions;
    }

    // Check if the session is empty
    bool isEmpty() const 
    {
        return actions.empty();
    }

    // Clear the session
    void clear() 
    {
        actions.clear();
    }
};

class HistoryManager 
{
    public:

    std::stack<Session> undo_stack;
    std::stack<Session> redo_stack;
    Session current_session;
    bool session_open = false;


    // Start a new session
    void startSession() 
    {
        if (session_open) {
            endSession(); // Close any existing session first
        }
        current_session.clear();
        session_open = true;
    }

    // End the current session and add it to the undo stack
    void endSession() 
    {
        if (!current_session.isEmpty()) 
        {
            undo_stack.push(current_session);
            current_session.clear();
        }
        session_open = false;
    }

    // Record an action in the current session
    void recordAction(int index, float old_value, float new_value) 
    {
        if (session_open) 
        {
            current_session.addAction(index, old_value, new_value);
        }
    }

    // Undo the last session
    void undo() 
    {
        if (!undo_stack.empty()) 
        {
            Session lastSession = undo_stack.top();
            undo_stack.pop();

            // Here, you should apply the inverse of each action in lastSession
            // to your VoltageSequencer instance

            redo_stack.push(lastSession);
        }
    }

    // Redo the last undone session
    // NOTE AND WARNING: This function is not called yet.  The undo is being
    // done within the VoltageSequencer instead.
    void redo() 
    {
        /*
        if (!redo_stack.empty()) {
            Session sessionToRedo = redo_stack.top();
            redo_stack.pop();

            // Here, reapply each action in sessionToRedo
            // to your VoltageSequencer instance

            undo_stack.push(sessionToRedo);
        }
        */
    }
};
