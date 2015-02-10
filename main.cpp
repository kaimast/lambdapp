#include <iostream>
#include "lambda.h"

using lambda::term;
using lambda::app;
using lambda::var;
using lambda::vars;
using lambda::func;

void simple()
{
    std::cout << "Simple term with no capture problem: " << std::endl;
    term t1 = app(func("y", vars({"x", "y", "z"})), var("z"));
    std::cout << "Initial Term:" << t1.str() << std::endl;
    std::cout << "Free Variables: " << lambda::str(lambda::free(t1)) << std::endl;

    term t2 = lambda::eval(t1);
    std::cout << "Reduced Term:" << t2.str() << std::endl;
}

void complex1()
{
    std::cout << "Complex term 1 with naming conflicts: " << std::endl;
    term t1 = app(func("x", func("y", app(var("b"), (vars({"x", "y"}))))), app(var("a"), var("y")));
    std::cout << "Initial Term:" << t1.str() << std::endl;
    std::cout << "Free Variables: " << lambda::str(lambda::free(t1)) << std::endl;

    term t2 = lambda::eval(t1);
    std::cout << "Reduced Term:" << t2.str() << std::endl;
}

void complex2()
{
    std::cout << "Complex term 2 with naming conflicts: " << std::endl;
    term t1 = app(func("y", app(func("x", func("y", app(var("b"), (vars({"x", "y"}))))), app(var("a"), var("y")))), var("c"));
    std::cout << "Initial Term:" << t1.str() << std::endl;
    std::cout << "Free Variables: " << lambda::str(lambda::free(t1)) << std::endl;

    term t2 = lambda::eval(t1);
    std::cout << "Reduced Term:" << t2.str() << std::endl;
}

int main()
{
    simple();
    complex1();
    complex2();
    return 0;
}

