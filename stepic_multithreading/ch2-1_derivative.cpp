#include <iostream>
#include <string>
#include <regex>

std::string derivative(std::string polynomial) {
    
    std::map<int,int> terms;
    std::ostringstream derivative;

    /* Remove whitespaces */
    polynomial = std::regex_replace(polynomial, std::regex("\\s+"), "");

    /* Find polynonial terms */
    auto regex = std::regex("(\\-)?([0-9]{0,})\\*?x(\\^([0-9]{0,}))?");
    auto it = std::sregex_iterator(polynomial.begin(), polynomial.end(), regex);
    for (;it != std::sregex_iterator(); ++it) {

        int sign = ((*it)[1].str()[0] != '-') ? 1 : -1;
        int fact = ((*it)[2].str()[0] == 0) ? 1 : std::stoi((*it)[2].str()); 
        int degr = ((*it)[4].str()[0] == 0) ? 1 : std::stoi((*it)[4].str());

        terms[degr] += sign * fact;
    }   

    /* Differentiate polynomial */
    for (auto it = terms.rbegin(); it != terms.rend(); ++it) {

        int fact = it->first * it->second;
        int degr = it->first - 1;

        derivative << ((it != terms.rbegin()) && (fact > 0) ? "+" : "") << fact;
        
        if (degr != 0) { 
            derivative << ((fact != 0) ? "*x" : "x");
            if (degr > 1) {
                derivative << "^" << degr;
            }
        }
    }
    
    return derivative.str();
}

int main(int argc, char** argv) {

    std::cout << derivative("-x^2-x^3") << std::endl;
    std::cout << derivative("1*x^3-2*x^2+x+1232") << std::endl;
    std::cout << derivative("2*x^100+100*x^2") << std::endl;
    std::cout << derivative("2*x^100+100*x^2") << std::endl;

    return 0;
}