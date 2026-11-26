/*
 * BODMAS Calculator in C++
 * Author: Ayesha Aleem
 *
 * Type a full expression, for example:  2 + 3 * (4 - 1) ^ 2 / 3
 * The calculator follows BODMAS:
 *   B - Brackets          ( )
 *   O - Orders            ^  and sqrt()
 *   D/M - Division, Multiplication, Modulus   /  *  %
 *   A/S - Addition, Subtraction               +  -
 *
 * Also supported:
 *   - Negative numbers:  -5 + 2,  2 * -3
 *   - Factorial:         5!
 *   - Functions:         sqrt(16), abs(-4)
 *   - "ans" = previous result
 *   - "history" shows last calculations, "help" shows examples, "exit" quits
 *
 * Compile:  g++ -std=c++11 calculator.cpp -o calculator
 * Run:      ./calculator        (Windows: calculator.exe)
 */

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <iomanip>

using namespace std;

/*
 * Grammar (lowest priority first, so higher priority binds tighter):
 *
 *   expression := term   { ('+' | '-') term }
 *   term       := unary  { ('*' | '/' | '%') unary }
 *   unary      := '-' unary | '+' unary | power
 *   power      := postfix [ '^' unary ]        (right-associative)
 *   postfix    := primary { '!' }
 *   primary    := number | 'ans' | '(' expression ')' | function '(' expression ')'
 */
class Calculator {
private:
    string text;      // the expression typed by the user
    size_t pos;       // current position while reading
    double lastAnswer;

    void skipSpaces() {
        while (pos < text.size() && isspace(static_cast<unsigned char>(text[pos]))) {
            pos++;
        }
    }

    // Looks at the next character without consuming it
    char peek() {
        skipSpaces();
        return pos < text.size() ? text[pos] : '\0';
    }

    // Consumes the next character if it matches
    bool match(char c) {
        if (peek() == c) {
            pos++;
            return true;
        }
        return false;
    }

    double parseExpression() {
        double result = parseTerm();
        while (true) {
            if (match('+')) {
                result += parseTerm();
            } else if (match('-')) {
                result -= parseTerm();
            } else {
                break;
            }
        }
        return result;
    }

    double parseTerm() {
        double result = parseUnary();
        while (true) {
            if (match('*')) {
                result *= parseUnary();
            } else if (match('/')) {
                double divisor = parseUnary();
                if (divisor == 0) throw runtime_error("Division by zero");
                result /= divisor;
            } else if (match('%')) {
                double divisor = parseUnary();
                if (divisor == 0) throw runtime_error("Modulus by zero");
                result = fmod(result, divisor);
            } else {
                break;
            }
        }
        return result;
    }

    double parseUnary() {
        if (match('-')) return -parseUnary();
        if (match('+')) return parseUnary();
        return parsePower();
    }

    double parsePower() {
        double base = parsePostfix();
        if (match('^')) {
            double exponent = parseUnary();   // right-associative: 2^3^2 = 2^(3^2)
            double result = pow(base, exponent);
            if (isnan(result) || isinf(result)) {
                throw runtime_error("Power result is undefined or too large");
            }
            return result;
        }
        return base;
    }

    double parsePostfix() {
        double value = parsePrimary();
        while (match('!')) {
            value = factorial(value);
        }
        return value;
    }

    double parsePrimary() {
        char c = peek();

        // Brackets
        if (match('(')) {
            double value = parseExpression();
            if (!match(')')) throw runtime_error("Missing closing bracket ')'");
            return value;
        }

        // Number
        if (isdigit(static_cast<unsigned char>(c)) || c == '.') {
            return parseNumber();
        }

        // Name: function or "ans"
        if (isalpha(static_cast<unsigned char>(c))) {
            string name;
            while (pos < text.size() && isalpha(static_cast<unsigned char>(text[pos]))) {
                name += static_cast<char>(tolower(static_cast<unsigned char>(text[pos])));
                pos++;
            }
            if (name == "ans") return lastAnswer;

            if (!match('(')) throw runtime_error("Expected '(' after " + name);
            double arg = parseExpression();
            if (!match(')')) throw runtime_error("Missing closing bracket ')'");

            if (name == "sqrt") {
                if (arg < 0) throw runtime_error("Square root of a negative number");
                return sqrt(arg);
            }
            if (name == "abs") return fabs(arg);
            throw runtime_error("Unknown function: " + name);
        }

        if (c == '\0') throw runtime_error("Expression ended unexpectedly");
        throw runtime_error(string("Unexpected character '") + c + "'");
    }

    double parseNumber() {
        skipSpaces();
        size_t start = pos;
        bool seenDot = false;
        while (pos < text.size() &&
               (isdigit(static_cast<unsigned char>(text[pos])) || text[pos] == '.')) {
            if (text[pos] == '.') {
                if (seenDot) throw runtime_error("Number has two decimal points");
                seenDot = true;
            }
            pos++;
        }
        string numStr = text.substr(start, pos - start);
        if (numStr == ".") throw runtime_error("Invalid number");
        return stod(numStr);
    }

    static double factorial(double n) {
        if (n < 0 || n != floor(n)) {
            throw runtime_error("Factorial needs a non-negative whole number");
        }
        if (n > 20) throw runtime_error("Factorial is supported only up to 20");
        double result = 1;
        for (int i = 2; i <= static_cast<int>(n); i++) result *= i;
        return result;
    }

public:
    Calculator() : pos(0), lastAnswer(0) {}

    double evaluate(const string &expression) {
        text = expression;
        pos = 0;
        double result = parseExpression();
        if (peek() != '\0') {
            throw runtime_error(string("Unexpected character '") + peek() + "'");
        }
        lastAnswer = result;
        return result;
    }
};

void printHelp() {
    cout << "\nExamples:\n"
         << "  2 + 3 * 4            -> 14        (multiplication first)\n"
         << "  (2 + 3) * 4          -> 20        (brackets first)\n"
         << "  2 ^ 3 ^ 2            -> 512       (power is right-associative)\n"
         << "  -2 ^ 2               -> -4\n"
         << "  10 / 4 + sqrt(16)    -> 6.5\n"
         << "  5! + 3               -> 123\n"
         << "  ans * 2              -> uses the previous result\n"
         << "\nCommands: help, history, exit\n";
}

int main() {
    Calculator calc;
    vector<string> history;

    cout << "=== BODMAS Calculator (C++) ===\n";
    cout << "Type an expression, or 'help' for examples, 'exit' to quit.\n";

    string line;
    while (true) {
        cout << "\n> ";
        if (!getline(cin, line)) {
            cout << "\nGoodbye!\n";
            break;
        }

        // Trim spaces at both ends
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, last - first + 1);

        if (line == "exit" || line == "quit") {
            cout << "Goodbye!\n";
            break;
        }
        if (line == "help") {
            printHelp();
            continue;
        }
        if (line == "history") {
            if (history.empty()) {
                cout << "No calculations yet.\n";
            } else {
                for (size_t i = 0; i < history.size(); i++) {
                    cout << i + 1 << ". " << history[i] << "\n";
                }
            }
            continue;
        }

        try {
            double result = calc.evaluate(line);
            ostringstream out;
            out << setprecision(10) << result;
            cout << "= " << out.str() << "\n";

            history.push_back(line + " = " + out.str());
            if (history.size() > 10) history.erase(history.begin());
        } catch (const exception &e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
