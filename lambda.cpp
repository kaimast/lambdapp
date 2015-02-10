#include "lambda.h"
#include <iostream>

namespace lambda
{

template<typename T> inline
std::set<T> operator+(std::set<T>&& s1, std::set<T>&& s2)
{
    std::set<T> result;

    for(auto it: s1)
    {
        result.emplace(it);
    }

    for(auto it: s2)
    {
        result.emplace(it);
    }

    return result;
}

struct term::variable_container
{
    variable_container(const std::string& name_)
        : name(name_) {}

    const std::string name;
};

struct term::application_container
{
    application_container(term&& t1_, term&& t2_)
        : t1(std::move(t1_)), t2(std::move(t2_))
    {}

    term t1, t2;
};

struct term::abstraction_container
{
    abstraction_container(const std::string& bound_variable_, term&& t_)
        : bound_variable(bound_variable_), t(std::move(t_))
    {
    }

    const std::string bound_variable;
    term t;
};

term::term(term&& other)
    : m_variable(other.m_variable), m_abstraction(other.m_abstraction), m_application(other.m_application)
{
    other.m_variable = nullptr;
    other.m_abstraction = nullptr;
    other.m_application = nullptr;
}

void term::operator=(term&& other)
{
    delete m_variable;
    delete m_abstraction;
    delete m_application;

    m_variable = other.m_variable;
    m_abstraction = other.m_abstraction;
    m_application = other.m_application;

    other.m_variable = nullptr;
    other.m_abstraction = nullptr;
    other.m_application = nullptr;
}

term::~term()
{
    delete m_variable;
    delete m_abstraction;
    delete m_application;
}

term::term(term &&term1, term &&term2)
    : m_application(new application_container(std::move(term1), std::move(term2)))
{
}

term term::duplicate() const
{
    if(holds_variable())
    {
        auto var = new variable_container(m_variable->name);
        return term(var);
    }
    else if(holds_abstraction())
    {
        auto abs = new abstraction_container(m_abstraction->bound_variable, m_abstraction->t.duplicate());
        return term(abs);
    }
    else if(holds_application())
    {
        return term(m_application->t1.duplicate(), m_application->t2.duplicate());
    }
    else
    {
        throw std::runtime_error("Invalid lambda term!");
    }
}

std::string term::str() const
{
    if(holds_variable())
    {
        return m_variable->name;
    }
    else if(holds_abstraction())
    {
        return "(\u03BB" + m_abstraction->bound_variable + "." + m_abstraction->t.str() + ")";
    }
    else if(holds_application())
    {
        return m_application->t1.str() + "(" + m_application->t2.str() + ")";
    }
    else
    {
        throw std::runtime_error("Invalid lambda term!");
    }
}

const std::string& term::get_var_name() const
{
    if(!holds_variable())
    {
        throw std::runtime_error("Can only call this if term holds a variable");
    }

    return m_variable->name;
}

// Get terms (for application)
const term& term::get_first_term() const
{
    if(!holds_application())
    {
        throw std::runtime_error("Can only call this if term holds an application");
    }

    return m_application->t1;
}

const term& term::get_second_term() const
{
    if(!holds_application())
    {
        throw std::runtime_error("Can only call this if term holds an application");
    }

    return m_application->t2;
}

// Get bound variable (for application)
const std::string& term::get_bound_var() const
{
    if(!holds_abstraction())
    {
        throw std::runtime_error("Can only call this if term holds an abstraction");
    }

    return m_abstraction->bound_variable;
}

// Get content (for abstraction)
const term& term::get_content() const
{
    if(!holds_abstraction())
    {
        throw std::runtime_error("Can only call this if term holds an abstraction");
    }

    return m_abstraction->t;
}

var::var(const std::string& name)
    : term(new variable_container(name))
{
}

func::func(const std::string& bound, term&& t)
    : term(new abstraction_container(bound, std::move(t)))
{
}

app::app(term&& t1, term&& t2)
    : term(std::move(t1), std::move(t2))
{
}

term vars(const std::list<std::string>& strs)
{
    // use tail recursion :)
    if(strs.size() == 0)
    {
        throw std::runtime_error("Cannot handle empty string");
    }
    else if(strs.size() == 1)
    {
        return var(*strs.begin());
    }
    else
    {
        auto head = strs.begin();

        auto tstart = strs.begin();
        tstart++;
        std::list<std::string> tail(tstart, strs.end());

        return var(*head) + vars(tail);
    }
}

std::string fresh(const term& t, const std::string& var)
{
    auto f = free(t);

    std::string fresh_var = var;

    do
    {
        fresh_var = fresh_var + "'";
    } while(f.find(fresh_var) != f.end());

    return fresh_var;
}

term substitute(const term& t, const std::string& var, const term& replacement)
{
    if(t.holds_abstraction())
    {
        if(t.get_bound_var() == var)
        {
            return t.duplicate();
        }
        else
        {
            auto f = free(replacement);

            if(f.find(t.get_bound_var()) == f.end())
            {
                return func(t.get_bound_var(), substitute(t.get_content(), var, replacement));
            }
            else
            {
                std::string new_bound = fresh(t.get_content(), t.get_bound_var());
                return func(new_bound, substitute(substitute(t.get_content(), t.get_bound_var(), lambda::var(new_bound)), var, replacement.duplicate()));
            }
        }
    }
    else if(t.holds_application())
    {
        return substitute(t.get_first_term(), var, replacement)
               + substitute(t.get_second_term(), var, replacement);
    }
    else if(t.holds_variable())
    {
        if(t.get_var_name() == var)
        {
            return replacement.duplicate();
        }
        else
        {
            return t.duplicate();
        }
    }
    else
    {
        throw std::runtime_error("Not a closed term!");
    }
}

term reduce(const term &t)
{
    if(t.holds_abstraction())
    {
        return t.duplicate();
    }
    else if(t.holds_application())
    {
        const term t1 = eval(t.get_first_term());
        const term& t2 = t.get_second_term();

        if(t.get_first_term().holds_abstraction())
        {
            return substitute(t1.get_content(), t1.get_bound_var(), t2);
        }
        else
        {
            // Cannot reduce further
            return t.duplicate();
        }
    }
    else if(t.holds_variable())
    {
        return t.duplicate();
    }
    else
    {
        throw std::runtime_error("Not a closed term!");
    }
}

term eval(const term &t)
{
    term n = t.duplicate();
    std::string prev, cur;

    // Repeat until no further changes occur
    do
    {
        prev = n.str();
        n = reduce(n);
        cur = n.str();
    } while(prev != cur);

    return n;
}

std::set<std::string> free(const term &t)
{
    if(t.holds_abstraction())
    {
        auto result = free(t.get_content());
        result.erase(t.get_bound_var());
        return result;
    }
    else if(t.holds_variable())
    {
        return {t.get_var_name()};
    }
    else if(t.holds_application())
    {
         return free(t.get_first_term()) + free(t.get_second_term());
    }
    else
    {
        throw std::runtime_error("Not a closed term!");
    }
}

std::string str(const std::set<std::string>& s)
{
    std::string result = "{";
    bool first = true;

    for(auto it: s)
    {
        if(!first)
        {
            result += ",";
        }

        first = false;
        result += it;
    }

    return result + "}";
}

}
