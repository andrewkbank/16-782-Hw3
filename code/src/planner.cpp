#include <iostream>
#include <fstream>
// #include <boost/functional/hash.hpp>
#include <regex>
#include <unordered_set>
#include <set>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <queue>
#include <cstring>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <limits>
#include <vector>
#include <cctype>
#include <memory>

#define SYMBOLS 0
#define INITIAL 1
#define GOAL 2
#define ACTIONS 3
#define ACTION_DEFINITION 4
#define ACTION_PRECONDITION 5
#define ACTION_EFFECT 6
#ifndef ENVS_DIR
#define ENVS_DIR "../envs"
#endif
class GroundedCondition;
class Condition;
class GroundedAction;
class Action;
class Env;

using namespace std;

bool print_status = true;

class GroundedCondition
{
    string predicate;
    list<string> arg_values;
    bool truth = true;

public:
    GroundedCondition(const string &predicate, const list<string> &arg_values, bool truth = true)
    {
        this->predicate = predicate;
        this->truth = truth; // fixed
        for (const string &l : arg_values)
        {
            this->arg_values.push_back(l);
        }
    }

    GroundedCondition(const GroundedCondition &gc)
    {
        this->predicate = gc.predicate;
        this->truth = gc.truth; // fixed
        for (const string &l : gc.arg_values)
        {
            this->arg_values.push_back(l);
        }
    }

    string get_predicate() const
    {
        return this->predicate;
    }
    list<string> get_arg_values() const
    {
        return this->arg_values;
    }

    bool get_truth() const
    {
        return this->truth;
    }

    friend ostream &operator<<(ostream &os, const GroundedCondition &pred)
    {
        os << pred.toString() << " ";
        return os;
    }

    bool operator==(const GroundedCondition &rhs) const
    {
        if (this->predicate != rhs.predicate || this->arg_values.size() != rhs.arg_values.size())
            return false;

        auto lhs_it = this->arg_values.begin();
        auto rhs_it = rhs.arg_values.begin();

        while (lhs_it != this->arg_values.end() && rhs_it != rhs.arg_values.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }

        if (this->truth != rhs.get_truth()) // fixed
            return false;

        return true;
    }

    string toString() const
    {
        string temp;
        temp += this->predicate;
        temp += "(";
        for (const string &l : this->arg_values)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct GroundedConditionComparator
{
    bool operator()(const GroundedCondition &lhs, const GroundedCondition &rhs) const
    {
        return lhs == rhs;
    }
};

struct GroundedConditionHasher
{
    size_t operator()(const GroundedCondition &gcond) const
    {
        return hash<string>{}(gcond.toString());
    }
};

class Condition
{
    string predicate;
    list<string> args;
    bool truth;

public:
    Condition(const string &pred, const list<string> &args, const bool truth)
    {
        this->predicate = pred;
        this->truth = truth;
        for (const string &ar : args)
        {
            this->args.push_back(ar);
        }
    }

    string get_predicate() const
    {
        return this->predicate;
    }

    list<string> get_args() const
    {
        return this->args;
    }

    bool get_truth() const
    {
        return this->truth;
    }

    friend ostream &operator<<(ostream &os, const Condition &cond)
    {
        os << cond.toString() << " ";
        return os;
    }

    bool operator==(const Condition &rhs) const // fixed
    {

        if (this->predicate != rhs.predicate || this->args.size() != rhs.args.size())
            return false;

        auto lhs_it = this->args.begin();
        auto rhs_it = rhs.args.begin();

        while (lhs_it != this->args.end() && rhs_it != rhs.args.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }

        if (this->truth != rhs.get_truth())
            return false;

        return true;
    }

    string toString() const
    {
        string temp;
        if (!this->truth)
            temp += "!";
        temp += this->predicate;
        temp += "(";
        for (const string &l : this->args)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct ConditionComparator
{
    bool operator()(const Condition &lhs, const Condition &rhs) const
    {
        return lhs == rhs;
    }
};

struct ConditionHasher
{
    size_t operator()(const Condition &cond) const
    {
        return hash<string>{}(cond.toString());
    }
};

class Action
{
    string name;
    list<string> args;
    unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions;
    unordered_set<Condition, ConditionHasher, ConditionComparator> effects;

public:
    Action(const string &name, const list<string> &args,
           const unordered_set<Condition, ConditionHasher, ConditionComparator> &preconditions,
           const unordered_set<Condition, ConditionHasher, ConditionComparator> &effects)
    {
        this->name = name;
        for (const string &l : args)
        {
            this->args.push_back(l);
        }
        for (const Condition &pc : preconditions)
        {
            this->preconditions.insert(pc);
        }
        for (const Condition &pc : effects)
        {
            this->effects.insert(pc);
        }
    }
    string get_name() const
    {
        return this->name;
    }
    list<string> get_args() const
    {
        return this->args;
    }
    unordered_set<Condition, ConditionHasher, ConditionComparator> get_preconditions() const
    {
        return this->preconditions;
    }
    unordered_set<Condition, ConditionHasher, ConditionComparator> get_effects() const
    {
        return this->effects;
    }

    bool operator==(const Action &rhs) const
    {
        if (this->get_name() != rhs.get_name() || this->get_args().size() != rhs.get_args().size())
            return false;

        return true;
    }

    friend ostream &operator<<(ostream &os, const Action &ac)
    {
        os << ac.toString() << endl;
        os << "Precondition: ";
        for (const Condition &precond : ac.get_preconditions())
            os << precond;
        os << endl;
        os << "Effect: ";
        for (const Condition &effect : ac.get_effects())
            os << effect;
        os << endl;
        return os;
    }

    string toString() const
    {
        string temp;
        temp += this->get_name();
        temp += "(";
        for (const string &l : this->get_args())
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

struct ActionComparator
{
    bool operator()(const Action &lhs, const Action &rhs) const
    {
        return lhs == rhs;
    }
};

struct ActionHasher
{
    size_t operator()(const Action &ac) const
    {
        return hash<string>{}(ac.get_name());
    }
};

class Env
{
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> initial_conditions;
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator> goal_conditions;
    unordered_set<Action, ActionHasher, ActionComparator> actions;
    unordered_set<string> symbols;

public:
    void remove_initial_condition(const GroundedCondition &gc)
    {
        this->initial_conditions.erase(gc);
    }
    void add_initial_condition(const GroundedCondition &gc)
    {
        this->initial_conditions.insert(gc);
    }
    void add_goal_condition(const GroundedCondition &gc)
    {
        this->goal_conditions.insert(gc);
    }
    void remove_goal_condition(const GroundedCondition &gc)
    {
        this->goal_conditions.erase(gc);
    }
    void add_symbol(const string &symbol)
    {
        symbols.insert(symbol);
    }
    void add_symbols(const list<string> &symbols)
    {
        for (const string &l : symbols)
            this->symbols.insert(l);
    }
    void add_action(const Action &action)
    {
        this->actions.insert(action);
    }

    Action get_action(const string &name) const
    {
        for (Action a : this->actions)
        {
            if (a.get_name() == name)
                return a;
        }
        throw runtime_error("Action " + name + " not found!");
    }

    unordered_set<string> get_symbols() const
    {
        return this->symbols;
    }

    friend ostream &operator<<(ostream &os, const Env &w)
    {
        os << "***** Environment *****" << endl
           << endl;
        os << "Symbols: ";
        for (const string &s : w.get_symbols())
            os << s + ",";
        os << endl;
        os << "Initial conditions: ";
        for (const GroundedCondition &s : w.initial_conditions)
            os << s;
        os << endl;
        os << "Goal conditions: ";
        for (const GroundedCondition &g : w.goal_conditions)
            os << g;
        os << endl;
        os << "Actions:" << endl;
        for (const Action &g : w.actions)
            os << g << endl;
        cout << "***** Environment Created! *****" << endl;
        return os;
    }

    // add getters
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>
    get_initial_conditions() const
    {
        return this->initial_conditions;
    }
    unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>
    get_goal_conditions() const
    {
        return this->goal_conditions;
    }
    unordered_set<Action, ActionHasher, ActionComparator> get_actions() const
    {
        return this->actions;
    }
};

class GroundedAction
{
    string name;
    list<string> arg_values;

public:
    GroundedAction(const string &name, const list<string> &arg_values)
    {
        this->name = name;
        for (const string &ar : arg_values)
        {
            this->arg_values.push_back(ar);
        }
    }

    string get_name() const
    {
        return this->name;
    }

    list<string> get_arg_values() const
    {
        return this->arg_values;
    }

    bool operator==(const GroundedAction &rhs) const
    {
        if (this->name != rhs.name || this->arg_values.size() != rhs.arg_values.size())
            return false;

        auto lhs_it = this->arg_values.begin();
        auto rhs_it = rhs.arg_values.begin();

        while (lhs_it != this->arg_values.end() && rhs_it != rhs.arg_values.end())
        {
            if (*lhs_it != *rhs_it)
                return false;
            ++lhs_it;
            ++rhs_it;
        }
        return true;
    }

    friend ostream &operator<<(ostream &os, const GroundedAction &gac)
    {
        os << gac.toString() << " ";
        return os;
    }

    string toString() const
    {
        string temp;
        temp += this->name;
        temp += "(";
        for (const string &l : this->arg_values)
        {
            temp += l + ",";
        }
        temp = temp.substr(0, temp.length() - 1);
        temp += ")";
        return temp;
    }
};

list<string> parse_symbols(string symbols_str)
{
    list<string> symbols;
    size_t pos = 0;
    string delimiter = ",";
    while ((pos = symbols_str.find(delimiter)) != string::npos)
    {
        string symbol = symbols_str.substr(0, pos);
        symbols_str.erase(0, pos + delimiter.length());
        symbols.push_back(symbol);
    }
    symbols.push_back(symbols_str);
    return symbols;
}

Env *create_env(char *filename)
{
    ifstream input_file(filename);
    Env *env = new Env();
    regex symbolStateRegex("symbols:", regex::icase);
    regex symbolRegex("([a-zA-Z0-9_, ]+) *");
    regex initialConditionRegex("initialconditions:(.*)", regex::icase);
    regex conditionRegex("(!?[A-Z][a-zA-Z_]*) *\\( *([a-zA-Z0-9_, ]+) *\\)");
    regex goalConditionRegex("goalconditions:(.*)", regex::icase);
    regex actionRegex("actions:", regex::icase);
    regex precondRegex("preconditions:(.*)", regex::icase);
    regex effectRegex("effects:(.*)", regex::icase);
    int parser = SYMBOLS;

    unordered_set<Condition, ConditionHasher, ConditionComparator> preconditions;
    unordered_set<Condition, ConditionHasher, ConditionComparator> effects;
    string action_name;
    string action_args;

    string line;
    if (input_file.is_open())
    {
        while (getline(input_file, line))
        {
            string::iterator end_pos = remove(line.begin(), line.end(), ' ');
            line.erase(end_pos, line.end());

            if (line.empty())
                continue;

            if (parser == SYMBOLS)
            {
                smatch results;
                if (regex_search(line, results, symbolStateRegex))
                {
                    line = line.substr(8);
                    sregex_token_iterator iter(line.begin(), line.end(), symbolRegex, 0);
                    sregex_token_iterator end;

                    env->add_symbols(parse_symbols(iter->str())); // fixed

                    parser = INITIAL;
                }
                else
                {
                    cout << "Symbols are not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == INITIAL)
            {
                const char *line_c = line.c_str();
                if (regex_match(line_c, initialConditionRegex))
                {
                    const std::vector<int> submatches = {1, 2};
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        if (predicate[0] == '!')
                        {
                            env->remove_initial_condition(
                                GroundedCondition(predicate.substr(1), parse_symbols(args)));
                        }
                        else
                        {
                            env->add_initial_condition(
                                GroundedCondition(predicate, parse_symbols(args)));
                        }
                    }

                    parser = GOAL;
                }
                else
                {
                    cout << "Initial conditions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == GOAL)
            {
                const char *line_c = line.c_str();
                if (regex_match(line_c, goalConditionRegex))
                {
                    const std::vector<int> submatches = {1, 2};
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        if (predicate[0] == '!')
                        {
                            env->remove_goal_condition(
                                GroundedCondition(predicate.substr(1), parse_symbols(args)));
                        }
                        else
                        {
                            env->add_goal_condition(
                                GroundedCondition(predicate, parse_symbols(args)));
                        }
                    }

                    parser = ACTIONS;
                }
                else
                {
                    cout << "Goal conditions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTIONS)
            {
                const char *line_c = line.c_str();
                if (regex_match(line_c, actionRegex))
                {
                    parser = ACTION_DEFINITION;
                }
                else
                {
                    cout << "Actions not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_DEFINITION)
            {
                const char *line_c = line.c_str();
                if (regex_match(line_c, conditionRegex))
                {
                    const std::vector<int> submatches = {1, 2};
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;
                    // name
                    action_name = iter->str();
                    iter++;
                    // args
                    action_args = iter->str();
                    iter++;

                    parser = ACTION_PRECONDITION;
                }
                else
                {
                    cout << "Action not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_PRECONDITION)
            {
                const char *line_c = line.c_str();
                if (regex_match(line_c, precondRegex))
                {
                    const std::vector<int> submatches = {1, 2};
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        bool truth;

                        if (predicate[0] == '!')
                        {
                            predicate = predicate.substr(1);
                            truth = false;
                        }
                        else
                        {
                            truth = true;
                        }

                        Condition precond(predicate, parse_symbols(args), truth);
                        preconditions.insert(precond);
                    }

                    parser = ACTION_EFFECT;
                }
                else
                {
                    cout << "Precondition not specified correctly." << endl;
                    throw;
                }
            }
            else if (parser == ACTION_EFFECT)
            {
                const char *line_c = line.c_str();
                if (regex_match(line_c, effectRegex))
                {
                    const std::vector<int> submatches = {1, 2};
                    sregex_token_iterator iter(
                        line.begin(), line.end(), conditionRegex, submatches);
                    sregex_token_iterator end;

                    while (iter != end)
                    {
                        // name
                        string predicate = iter->str();
                        iter++;
                        // args
                        string args = iter->str();
                        iter++;

                        bool truth;

                        if (predicate[0] == '!')
                        {
                            predicate = predicate.substr(1);
                            truth = false;
                        }
                        else
                        {
                            truth = true;
                        }

                        Condition effect(predicate, parse_symbols(args), truth);
                        effects.insert(effect);
                    }

                    env->add_action(
                        Action(action_name, parse_symbols(action_args), preconditions, effects));

                    preconditions.clear();
                    effects.clear();
                    parser = ACTION_DEFINITION;
                }
                else
                {
                    cout << "Effects not specified correctly." << endl;
                    throw;
                }
            }
        }
        input_file.close();
    }

    else
        cout << "Unable to open file";

    return env;
}

using State = unordered_set<GroundedCondition, GroundedConditionHasher, GroundedConditionComparator>;

struct GroundedActionInstance
{
    GroundedAction action;
    vector<GroundedCondition> positive_preconditions;
    vector<GroundedCondition> negative_preconditions;
    vector<GroundedCondition> add_effects;
    vector<GroundedCondition> delete_effects;

    GroundedActionInstance(const GroundedAction &action) : action(action) {}
};

struct SearchStats
{
    size_t expanded = 0;
    size_t generated = 0;
    size_t duplicates = 0;
    size_t max_frontier = 0;
    double grounding_ms = 0.0;
    double elapsed_ms = 0.0;
    double total_ms = 0.0;
};

struct SearchNode
{
    State state;
    string key;
    int g;
    int parent;
    int action_index;

    SearchNode(const State &state, const string &key, int g, int parent, int action_index)
        : state(state), key(key), g(g), parent(parent), action_index(action_index) {}
};

struct SearchResult
{
    bool found = false;
    list<GroundedAction> plan;
    SearchStats stats;
};

string state_key(const State &state)
{
    vector<string> atoms;
    atoms.reserve(state.size());
    for (const GroundedCondition &condition : state)
        atoms.push_back(condition.toString());
    sort(atoms.begin(), atoms.end());

    string key;
    for (const string &atom : atoms)
        key += to_string(atom.size()) + ":" + atom;
    return key;
}

GroundedCondition ground_condition(
    const Condition &condition,
    const unordered_map<string, string> &substitution)
{
    list<string> grounded_arguments;
    for (const string &argument : condition.get_args())
    {
        auto value = substitution.find(argument);
        grounded_arguments.push_back(value == substitution.end() ? argument : value->second);
    }
    // States contain positive atoms only. The caller uses Condition::get_truth()
    // to place this atom in a positive/negative or add/delete list.
    return GroundedCondition(condition.get_predicate(), grounded_arguments, true);
}

vector<GroundedActionInstance> ground_actions(const Env *env)
{
    const auto symbol_set = env->get_symbols();
    vector<string> symbols(symbol_set.begin(), symbol_set.end());
    sort(symbols.begin(), symbols.end());

    const auto action_set = env->get_actions();
    vector<Action> schemas(action_set.begin(), action_set.end());
    sort(schemas.begin(), schemas.end(), [](const Action &lhs, const Action &rhs) {
        return lhs.toString() < rhs.toString();
    });

    // A predicate is static if no action can change it. Static preconditions can
    // be checked once while grounding; impossible instances are discarded and
    // satisfied static conditions need not be checked during search.
    unordered_set<string> dynamic_predicates;
    for (const Action &schema : schemas)
        for (const Condition &effect : schema.get_effects())
            dynamic_predicates.insert(effect.get_predicate());
    const State initial = env->get_initial_conditions();

    vector<GroundedActionInstance> grounded_actions;
    for (const Action &schema : schemas)
    {
        const auto parameter_list = schema.get_args();
        vector<string> parameters(parameter_list.begin(), parameter_list.end());
        vector<string> assignment(parameters.size());

        function<void(size_t)> enumerate = [&](size_t index) {
            if (index < parameters.size())
            {
                for (const string &symbol : symbols)
                {
                    assignment[index] = symbol;
                    enumerate(index + 1);
                }
                return;
            }

            unordered_map<string, string> substitution;
            list<string> argument_values;
            for (size_t i = 0; i < parameters.size(); ++i)
            {
                substitution[parameters[i]] = assignment[i];
                argument_values.push_back(assignment[i]);
            }

            GroundedActionInstance instance(
                GroundedAction(schema.get_name(), argument_values));

            bool static_preconditions_hold = true;
            for (const Condition &condition : schema.get_preconditions())
            {
                GroundedCondition grounded = ground_condition(condition, substitution);
                if (dynamic_predicates.find(condition.get_predicate()) ==
                    dynamic_predicates.end())
                {
                    const bool present = initial.find(grounded) != initial.end();
                    if (present != condition.get_truth())
                    {
                        static_preconditions_hold = false;
                        break;
                    }
                    continue;
                }

                if (condition.get_truth())
                    instance.positive_preconditions.push_back(grounded);
                else
                    instance.negative_preconditions.push_back(grounded);
            }

            if (!static_preconditions_hold)
                return;

            for (const Condition &condition : schema.get_effects())
            {
                GroundedCondition grounded = ground_condition(condition, substitution);
                if (condition.get_truth())
                    instance.add_effects.push_back(grounded);
                else
                    instance.delete_effects.push_back(grounded);
            }

            grounded_actions.push_back(instance);
        };

        enumerate(0);
    }

    sort(grounded_actions.begin(), grounded_actions.end(),
         [](const GroundedActionInstance &lhs, const GroundedActionInstance &rhs) {
             return lhs.action.toString() < rhs.action.toString();
         });
    return grounded_actions;
}

bool is_applicable(const State &state, const GroundedActionInstance &action)
{
    for (const GroundedCondition &condition : action.positive_preconditions)
        if (state.find(condition) == state.end())
            return false;

    for (const GroundedCondition &condition : action.negative_preconditions)
        if (state.find(condition) != state.end())
            return false;

    return true;
}

State apply_action(const State &state, const GroundedActionInstance &action)
{
    State successor = state;
    for (const GroundedCondition &condition : action.delete_effects)
        successor.erase(condition);
    for (const GroundedCondition &condition : action.add_effects)
        successor.insert(condition);
    return successor;
}

bool is_goal(const State &state, const vector<GroundedCondition> &goals)
{
    for (const GroundedCondition &goal : goals)
        if (state.find(goal) == state.end())
            return false;
    return true;
}

int goal_count_heuristic(const State &state, const vector<GroundedCondition> &goals)
{
    int unsatisfied = 0;
    for (const GroundedCondition &goal : goals)
        if (state.find(goal) == state.end())
            ++unsatisfied;
    return unsatisfied;
}

class EmptyDeleteListHeuristic
{
    struct RelaxedAction
    {
        vector<int> preconditions;
        vector<int> effects;
    };

    unordered_map<string, int> fact_ids;
    vector<RelaxedAction> actions;
    vector<vector<int>> consumers;
    vector<int> goal_ids;
    vector<GroundedCondition> goal_conditions;
    vector<GroundedActionInstance> grounded_actions;
    unordered_map<string, int> cached_costs;

    int fact_id(const GroundedCondition &condition)
    {
        const string key = condition.toString();
        auto found = fact_ids.find(key);
        if (found != fact_ids.end())
            return found->second;
        const int id = static_cast<int>(fact_ids.size());
        fact_ids[key] = id;
        return id;
    }

    int h_max(const State &state) const
    {
        const int infinity = numeric_limits<int>::max() / 4;
        vector<int> fact_cost(fact_ids.size(), infinity);
        vector<int> remaining_preconditions(actions.size());
        vector<int> maximum_precondition_cost(actions.size(), 0);
        vector<bool> finalized(fact_ids.size(), false);
        priority_queue<pair<int, int>, vector<pair<int, int>>,
                       greater<pair<int, int>>>
            reachable_facts;

        auto relax_fact = [&](int fact, int cost) {
            if (cost < fact_cost[fact])
            {
                fact_cost[fact] = cost;
                reachable_facts.push({cost, fact});
            }
        };

        for (const GroundedCondition &condition : state)
        {
            auto found = fact_ids.find(condition.toString());
            if (found != fact_ids.end())
                relax_fact(found->second, 0);
        }

        for (size_t action_index = 0; action_index < actions.size(); ++action_index)
        {
            remaining_preconditions[action_index] =
                static_cast<int>(actions[action_index].preconditions.size());
            if (remaining_preconditions[action_index] == 0)
                for (int effect : actions[action_index].effects)
                    relax_fact(effect, 1);
        }

        while (!reachable_facts.empty())
        {
            const int cost = reachable_facts.top().first;
            const int fact = reachable_facts.top().second;
            reachable_facts.pop();
            if (finalized[fact] || cost != fact_cost[fact])
                continue;
            finalized[fact] = true;

            for (int action_index : consumers[fact])
            {
                maximum_precondition_cost[action_index] =
                    max(maximum_precondition_cost[action_index], cost);
                --remaining_preconditions[action_index];
                if (remaining_preconditions[action_index] == 0)
                {
                    const int effect_cost =
                        maximum_precondition_cost[action_index] + 1;
                    for (int effect : actions[action_index].effects)
                        relax_fact(effect, effect_cost);
                }
            }
        }

        int heuristic = 0;
        for (int goal : goal_ids)
        {
            if (fact_cost[goal] == infinity)
                return infinity;
            heuristic = max(heuristic, fact_cost[goal]);
        }
        return heuristic;
    }

    bool relaxed_goal(const State &state) const
    {
        for (const GroundedCondition &goal : goal_conditions)
            if (state.find(goal) == state.end())
                return false;
        return true;
    }

    bool relaxed_applicable(
        const State &state,
        const GroundedActionInstance &action) const
    {
        for (const GroundedCondition &condition : action.positive_preconditions)
            if (state.find(condition) == state.end())
                return false;
        // The course STRIPS domains use positive preconditions. Retaining any
        // parsed negative preconditions is the literal empty-delete-list
        // transformation: only negative effects are removed.
        for (const GroundedCondition &condition : action.negative_preconditions)
            if (state.find(condition) != state.end())
                return false;
        return true;
    }

public:
    EmptyDeleteListHeuristic(
        const State &initial,
        const vector<GroundedCondition> &goals,
        const vector<GroundedActionInstance> &grounded_action_instances)
        : goal_conditions(goals), grounded_actions(grounded_action_instances)
    {
        for (const GroundedCondition &condition : initial)
            fact_id(condition);
        for (const GroundedCondition &goal : goals)
            goal_ids.push_back(fact_id(goal));

        actions.reserve(grounded_actions.size());
        for (const GroundedActionInstance &grounded_action : grounded_actions)
        {
            RelaxedAction relaxed_action;
            for (const GroundedCondition &precondition :
                 grounded_action.positive_preconditions)
                relaxed_action.preconditions.push_back(fact_id(precondition));
            for (const GroundedCondition &effect : grounded_action.add_effects)
                relaxed_action.effects.push_back(fact_id(effect));
            actions.push_back(relaxed_action);
        }

        consumers.resize(fact_ids.size());
        for (size_t action_index = 0; action_index < actions.size(); ++action_index)
            for (int precondition : actions[action_index].preconditions)
                consumers[precondition].push_back(static_cast<int>(action_index));
    }

    int evaluate(const State &state)
    {
        // h+ is the exact shortest-plan cost in the problem obtained by making
        // every delete list empty. We solve that monotone relaxed problem with
        // an inner A* search, using h_max only as an admissible lower bound.
        const int infinity = numeric_limits<int>::max() / 4;
        const string initial_key = state_key(state);
        auto cached = cached_costs.find(initial_key);
        if (cached != cached_costs.end())
            return cached->second;

        if (relaxed_goal(state))
        {
            cached_costs[initial_key] = 0;
            return 0;
        }

        struct RelaxedNode
        {
            State state;
            string key;
            int g;
        };
        struct RelaxedOpenEntry
        {
            int f;
            int h;
            size_t order;
            int node_index;
        };
        struct RelaxedOpenComparator
        {
            bool operator()(
                const RelaxedOpenEntry &lhs,
                const RelaxedOpenEntry &rhs) const
            {
                if (lhs.f != rhs.f)
                    return lhs.f > rhs.f;
                if (lhs.h != rhs.h)
                    return lhs.h > rhs.h;
                return lhs.order > rhs.order;
            }
        };

        vector<RelaxedNode> nodes;
        priority_queue<RelaxedOpenEntry,
                       vector<RelaxedOpenEntry>,
                       RelaxedOpenComparator>
            frontier;
        unordered_map<string, int> best_g;
        size_t insertion_order = 0;

        const int initial_h = h_max(state);
        if (initial_h == infinity)
        {
            cached_costs[initial_key] = infinity;
            return infinity;
        }
        nodes.push_back({state, initial_key, 0});
        frontier.push({initial_h, initial_h, insertion_order++, 0});
        best_g[initial_key] = 0;

        while (!frontier.empty())
        {
            const RelaxedOpenEntry entry = frontier.top();
            frontier.pop();
            const RelaxedNode node = nodes[entry.node_index];
            auto best = best_g.find(node.key);
            if (best == best_g.end() || node.g != best->second)
                continue;

            if (relaxed_goal(node.state))
            {
                cached_costs[initial_key] = node.g;
                return node.g;
            }

            for (const GroundedActionInstance &action : grounded_actions)
            {
                if (!relaxed_applicable(node.state, action))
                    continue;

                State successor = node.state;
                bool added_fact = false;
                for (const GroundedCondition &effect : action.add_effects)
                    added_fact = successor.insert(effect).second || added_fact;
                if (!added_fact)
                    continue;

                string successor_key = state_key(successor);
                const int successor_g = node.g + 1;
                auto old_g = best_g.find(successor_key);
                if (old_g != best_g.end() && successor_g >= old_g->second)
                    continue;

                const int successor_h = h_max(successor);
                if (successor_h == infinity)
                    continue;
                best_g[successor_key] = successor_g;
                nodes.push_back({successor, successor_key, successor_g});
                frontier.push({successor_g + successor_h, successor_h,
                               insertion_order++,
                               static_cast<int>(nodes.size() - 1)});
            }
        }

        cached_costs[initial_key] = infinity;
        return infinity;
    }
};

list<GroundedAction> reconstruct_plan(
    const vector<SearchNode> &nodes,
    int goal_index,
    const vector<GroundedActionInstance> &actions)
{
    list<GroundedAction> plan;
    for (int index = goal_index; nodes[index].parent >= 0; index = nodes[index].parent)
        plan.push_front(actions[nodes[index].action_index].action);
    return plan;
}

SearchResult breadth_first_search(
    const State &initial,
    const vector<GroundedCondition> &goals,
    const vector<GroundedActionInstance> &actions)
{
    SearchResult result;
    const auto start_time = chrono::steady_clock::now();
    vector<SearchNode> nodes;
    queue<int> frontier;
    unordered_set<string> visited;

    const string initial_key = state_key(initial);
    nodes.emplace_back(initial, initial_key, 0, -1, -1);
    frontier.push(0);
    visited.insert(initial_key);
    result.stats.max_frontier = 1;

    while (!frontier.empty())
    {
        const int node_index = frontier.front();
        frontier.pop();
        // Copy the node because appending successors may reallocate nodes.
        const SearchNode node = nodes[node_index];

        if (is_goal(node.state, goals))
        {
            result.found = true;
            result.plan = reconstruct_plan(nodes, node_index, actions);
            break;
        }

        ++result.stats.expanded;
        for (size_t action_index = 0; action_index < actions.size(); ++action_index)
        {
            if (!is_applicable(node.state, actions[action_index]))
                continue;

            ++result.stats.generated;
            State successor = apply_action(node.state, actions[action_index]);
            string successor_key = state_key(successor);
            if (!visited.insert(successor_key).second)
            {
                ++result.stats.duplicates;
                continue;
            }

            nodes.emplace_back(successor, successor_key, node.g + 1,
                               node_index, static_cast<int>(action_index));
            frontier.push(static_cast<int>(nodes.size() - 1));
        }
        result.stats.max_frontier = max(result.stats.max_frontier, frontier.size());
    }

    result.stats.elapsed_ms = chrono::duration<double, milli>(
                                  chrono::steady_clock::now() - start_time)
                                  .count();
    return result;
}

enum class HeuristicMode
{
    GOAL_COUNT,
    EMPTY_DELETE_LIST
};

struct OpenEntry
{
    int f;
    int h;
    size_t order;
    int node_index;
};

struct OpenEntryComparator
{
    bool operator()(const OpenEntry &lhs, const OpenEntry &rhs) const
    {
        if (lhs.f != rhs.f)
            return lhs.f > rhs.f;
        if (lhs.h != rhs.h)
            return lhs.h > rhs.h;
        return lhs.order > rhs.order;
    }
};

SearchResult a_star_search(
    const State &initial,
    const vector<GroundedCondition> &goals,
    const vector<GroundedActionInstance> &actions,
    HeuristicMode heuristic_mode)
{
    SearchResult result;
    const auto start_time = chrono::steady_clock::now();
    vector<SearchNode> nodes;
    priority_queue<OpenEntry, vector<OpenEntry>, OpenEntryComparator> frontier;
    unordered_map<string, int> best_g;
    size_t insertion_order = 0;

    unique_ptr<EmptyDeleteListHeuristic> empty_delete_list;
    if (heuristic_mode == HeuristicMode::EMPTY_DELETE_LIST)
        empty_delete_list.reset(new EmptyDeleteListHeuristic(initial, goals, actions));

    auto heuristic = [&](const State &state) {
        if (heuristic_mode == HeuristicMode::GOAL_COUNT)
            return goal_count_heuristic(state, goals);
        return empty_delete_list->evaluate(state);
    };

    const string initial_key = state_key(initial);
    const int initial_h = heuristic(initial);
    nodes.emplace_back(initial, initial_key, 0, -1, -1);
    frontier.push({initial_h, initial_h, insertion_order++, 0});
    best_g[initial_key] = 0;
    result.stats.max_frontier = 1;

    while (!frontier.empty())
    {
        const OpenEntry entry = frontier.top();
        frontier.pop();
        // Copy the node because appending successors may reallocate nodes.
        const SearchNode node = nodes[entry.node_index];

        auto best = best_g.find(node.key);
        if (best == best_g.end() || node.g != best->second)
            continue;

        if (is_goal(node.state, goals))
        {
            result.found = true;
            result.plan = reconstruct_plan(nodes, entry.node_index, actions);
            break;
        }

        ++result.stats.expanded;
        for (size_t action_index = 0; action_index < actions.size(); ++action_index)
        {
            if (!is_applicable(node.state, actions[action_index]))
                continue;

            ++result.stats.generated;
            State successor = apply_action(node.state, actions[action_index]);
            string successor_key = state_key(successor);
            const int tentative_g = node.g + 1;
            auto old_g = best_g.find(successor_key);
            if (old_g != best_g.end() && tentative_g >= old_g->second)
            {
                ++result.stats.duplicates;
                continue;
            }

            const int successor_h = heuristic(successor);
            best_g[successor_key] = tentative_g;
            nodes.emplace_back(successor, successor_key, tentative_g,
                               entry.node_index, static_cast<int>(action_index));
            const int successor_index = static_cast<int>(nodes.size() - 1);
            frontier.push({tentative_g + successor_h, successor_h,
                           insertion_order++, successor_index});
        }
        result.stats.max_frontier = max(result.stats.max_frontier, frontier.size());
    }

    result.stats.elapsed_ms = chrono::duration<double, milli>(
                                  chrono::steady_clock::now() - start_time)
                                  .count();
    return result;
}

string selected_search_mode()
{
    const char *configured_mode = getenv("PLANNER_MODE");
    string mode = configured_mode == nullptr ? "astar-edl" : configured_mode;
    transform(mode.begin(), mode.end(), mode.begin(),
              [](unsigned char character) { return static_cast<char>(tolower(character)); });
    return mode;
}

void print_search_stats(
    const string &mode,
    size_t grounded_action_count,
    const SearchResult &result)
{
    cout << "\nSearch statistics:" << endl;
    cout << "  mode: " << mode << endl;
    cout << "  grounded actions: " << grounded_action_count << endl;
    cout << "  solved: " << (result.found ? "yes" : "no") << endl;
    cout << "  plan length: " << result.plan.size() << endl;
    cout << "  expanded states: " << result.stats.expanded << endl;
    cout << "  generated successors: " << result.stats.generated << endl;
    cout << "  duplicate successors: " << result.stats.duplicates << endl;
    cout << "  maximum frontier: " << result.stats.max_frontier << endl;
    cout << "  grounding time (ms): " << result.stats.grounding_ms << endl;
    cout << "  search time (ms): " << result.stats.elapsed_ms << endl;
    cout << "  planning time (ms): " << result.stats.total_ms << endl;
}

list<GroundedAction> planner(Env *env)
{
    const auto planner_start = chrono::steady_clock::now();
    State initial = env->get_initial_conditions();
    const auto goal_set = env->get_goal_conditions();
    vector<GroundedCondition> goals(goal_set.begin(), goal_set.end());
    const auto grounding_start = chrono::steady_clock::now();
    vector<GroundedActionInstance> actions = ground_actions(env);
    const double grounding_ms = chrono::duration<double, milli>(
                                    chrono::steady_clock::now() - grounding_start)
                                    .count();
    const string mode = selected_search_mode();

    SearchResult result;
    if (mode == "bfs")
        result = breadth_first_search(initial, goals, actions);
    else if (mode == "astar" || mode == "astar-goal" || mode == "goal-count")
        result = a_star_search(initial, goals, actions, HeuristicMode::GOAL_COUNT);
    else if (mode == "astar-edl" || mode == "edl")
        result = a_star_search(initial, goals, actions, HeuristicMode::EMPTY_DELETE_LIST);
    else
        throw invalid_argument(
            "Unknown PLANNER_MODE '" + mode +
            "'. Use bfs, astar-goal, or astar-edl.");

    result.stats.grounding_ms = grounding_ms;
    result.stats.total_ms = chrono::duration<double, milli>(
                                chrono::steady_clock::now() - planner_start)
                                .count();
    print_search_stats(mode, actions.size(), result);
    if (!result.found)
        cerr << "No plan found." << endl;
    return result.plan;
}

int main(int argc, char *argv[])
{
    // DO NOT CHANGE THIS FUNCTION
    // char *env_file = static_cast<char *>("example.txt");
    const char *env_file = "example.txt";
    if (argc > 1)
        env_file = argv[1];
    std::string envsDirPath = ENVS_DIR;
    char *filename = new char[envsDirPath.length() + strlen(env_file) + 2];
    strcpy(filename, envsDirPath.c_str());
    strcat(filename, "/");
    strcat(filename, env_file);

    cout << "Environment: " << filename << endl;
    Env *env = create_env(filename);
    if (print_status)
    {
        cout << *env;
    }

    list<GroundedAction> actions = planner(env);

    cout << "\nPlan: " << endl;
    for (const GroundedAction &gac : actions)
    {
        cout << gac << endl;
    }

    delete env;
    return 0;
}
