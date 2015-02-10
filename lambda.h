#ifndef LAMBDA_H
#define LAMBDA_H

#include <string>
#include <stdexcept>
#include <list>
#include <set>

namespace lambda
{

class term
{
public:
    // No copy, only move
    term(const term& other) = delete;
    term(term&& other);
    ~term();

    term duplicate() const;

    void operator=(term&& other);

    bool holds_application() const;
    bool holds_variable() const;
    bool holds_abstraction() const;

    // Get variable name (for variable)
    const std::string& get_var_name() const;

    // Get terms (for abstraction)
    const term& get_first_term() const;
    const term& get_second_term() const;

    // Get bound variable (for application)
    const std::string& get_bound_var() const;

    // Get content (for application)
    const term& get_content() const;

    std::string str() const;

    term(term&& term1, term&& term2);

protected:
    struct variable_container;
    struct abstraction_container;
    struct application_container;

    term(const variable_container* var)
        : m_variable(var) {}

    term(const abstraction_container* app)
        : m_abstraction(app) {}

    const variable_container *m_variable = nullptr;
    const abstraction_container *m_abstraction = nullptr;
    const application_container *m_application = nullptr;
};

inline bool term::holds_application() const
{
    return m_application != nullptr;
}

inline bool term::holds_variable() const
{
    return m_variable != nullptr;
}

inline bool term::holds_abstraction() const
{
    return m_abstraction != nullptr;
}

// Construct Terms conveniently
class var : public term
{
public:
    var(const std::string& name);
};

// Call abstraction 'func' to not conflict with abs() from cmath
class func : public term
{
public:
    func(const std::string& bound, term&& t);
};

// Combine two terms (application)
class app : public term
{
public:
    app(term&& t1, term&& t2);
};

// Same as app but using the plus operator
inline term operator+(term&& t1, term&& t2)
{
    return term(std::move(t1), std::move(t2));
}

// Construct cascaded abstractions from a list of variables
term vars(const std::list<std::string>& strs);

// Reduce to a simpler term as far as possible
term eval(const term& t);

std::set<std::string> free(const term& t);

std::string str(const std::set<std::string>& s);

}

#endif // LAMBDA_H
