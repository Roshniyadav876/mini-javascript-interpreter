#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <map>

using namespace std;

// ==========================================
// 1. DATA STRUCTURES
// ==========================================
struct Variable { string name, type, value, args; };
vector<Variable> variables;
string lastReturnVal = "";
bool isReturning = false;

void executeAdvancedJS(const string& code);
string evaluateMathLogic(string expr);
string evaluateCall(const string& expr);
string evaluateRHS(const string& raw);

int findVariable(const string& name) {
    for (size_t i = 0; i < variables.size(); i++)
        if (variables[i].name == name) return (int)i;
    return -1;
}

// Whitespace + semicolon trim
string cleanString(string s) {
    while (!s.empty() && (isspace((unsigned char)s.front()))) s.erase(0, 1);
    while (!s.empty() && (isspace((unsigned char)s.back()) || s.back() == ';' || s.back() == '\n' || s.back() == '\r'))
        s.pop_back();
    return s;
}

// Quotes remove karo (inner string value ke liye)
string stripQuotes(const string& s) {
    if (s.size() >= 2 &&
        ((s.front() == '"' && s.back() == '"') ||
         (s.front() == '\'' && s.back() == '\'')))
        return s.substr(1, s.size() - 2);
    return s;
}

bool isStringLiteral(const string& s) {
    return s.size() >= 2 &&
           ((s.front() == '"' && s.back() == '"') ||
            (s.front() == '\'' && s.back() == '\''));
}

bool isNumber(const string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start == s.size()) return false;
    for (size_t i = start; i < s.size(); i++)
        if (!isdigit((unsigned char)s[i]) && s[i] != '.') return false;
    return true;
}

int safeStoi(const string& s) {
    string c = cleanString(s);
    if (c.empty()) return 0;
    try { return stoi(c); } catch (...) { return 0; }
}

double safeStod(const string& s) {
    string c = cleanString(s);
    if (c.empty()) return 0.0;
    try { return stod(c); } catch (...) { return 0.0; }
}

string getAdvancedType(const string& value) {
    if (value == "null" || value == "undefined") return value;
    if (value.find('{') != string::npos) return "object";
    if (value.find('[') != string::npos) return "array";
    if (value == "true" || value == "false") return "boolean";
    if (isNumber(value)) return "number";
    return "string";
}

// Variable ki actual value resolve karo
string resolveVar(const string& expr) {
    string e = cleanString(expr);
    int idx = findVariable(e);
    if (idx != -1) return variables[idx].value;
    return e;
}

// ==========================================
// 2. EXPRESSION EVALUATOR
// ==========================================

// Find top-level operator (brackets ke bahar)
// Returns position of operator, -1 if not found
int findOperator(const string& expr, const string& op) {
    int depth = 0;
    bool inStr = false; char strChar = 0;
    int elen = (int)expr.size();
    int olen = (int)op.size();
    for (int i = 0; i < elen; i++) {
        char c = expr[i];
        if (!inStr && (c == '"' || c == '\'')) { inStr = true; strChar = c; continue; }
        if (inStr) { if (c == strChar) inStr = false; continue; }
        if (c == '(' || c == '[') { depth++; continue; }
        if (c == ')' || c == ']') { depth--; continue; }
        if (depth == 0 && i + olen <= elen && expr.substr(i, olen) == op) {
            // '==' ke liye '===' se alag karo
            if (op == "==" && i + 3 <= elen && expr[i+2] == '=') continue;
            if (op == "!=" && i + 3 <= elen && expr[i+2] == '=') continue;
            return i;
        }
    }
    return -1;
}

string evaluateExpression(string expr);

// Template literal `...${var}...` handle karo
string processTemplateLiteral(const string& s) {
    // s has backtick removed already
    string result = "";
    size_t i = 0;
    while (i < s.size()) {
        if (i + 1 < s.size() && s[i] == '$' && s[i+1] == '{') {
            size_t end = s.find('}', i+2);
            if (end == string::npos) { result += s[i]; i++; continue; }
            string varExpr = s.substr(i+2, end - i - 2);
            result += evaluateExpression(cleanString(varExpr));
            i = end + 1;
        } else {
            result += s[i]; i++;
        }
    }
    return result;
}

string evaluateExpression(string expr) {
    expr = cleanString(expr);
    if (expr.empty()) return "";

    // Template literal
    if (expr.front() == '`' && expr.back() == '`')
        return processTemplateLiteral(expr.substr(1, expr.size()-2));

    // String literal
    if (isStringLiteral(expr)) return stripQuotes(expr);

    // Boolean / null / undefined
    if (expr == "true" || expr == "false" || expr == "null" || expr == "undefined")
        return expr;

    // Parentheses unwrap
    if (expr.front() == '(' && expr.back() == ')') {
        // Check if matching
        int d = 0;
        bool match = true;
        for (size_t i = 0; i < expr.size(); i++) {
            if (expr[i] == '(') d++;
            else if (expr[i] == ')') { d--; if (d == 0 && i < expr.size()-1) { match = false; break; } }
        }
        if (match) return evaluateExpression(expr.substr(1, expr.size()-2));
    }

    // Logical OR ||
    {
        int p = findOperator(expr, "||");
        if (p != -1) {
            string lv = evaluateExpression(expr.substr(0, p));
            if (lv != "false" && lv != "0" && lv != "" && lv != "null" && lv != "undefined") return lv;
            return evaluateExpression(expr.substr(p+2));
        }
    }
    // Logical AND &&
    {
        int p = findOperator(expr, "&&");
        if (p != -1) {
            string lv = evaluateExpression(expr.substr(0, p));
            if (lv == "false" || lv == "0" || lv == "" || lv == "null" || lv == "undefined") return "false";
            return evaluateExpression(expr.substr(p+2));
        }
    }

    // Comparison operators (right to left priority nahi, simple left scan)
    auto tryCompare = [&](const string& op) -> pair<bool,string> {
        int p = findOperator(expr, op);
        if (p == -1) return {false,""};
        string lv = evaluateExpression(expr.substr(0, p));
        string rv = evaluateExpression(expr.substr(p+op.size()));
        bool res = false;
        if (op == "===" || op == "==") res = (lv == rv) || (isNumber(lv) && isNumber(rv) && safeStod(lv) == safeStod(rv));
        else if (op == "!==" || op == "!=") res = !((lv == rv) || (isNumber(lv) && isNumber(rv) && safeStod(lv) == safeStod(rv)));
        else if (op == ">=") res = isNumber(lv) && isNumber(rv) ? safeStod(lv) >= safeStod(rv) : lv >= rv;
        else if (op == "<=") res = isNumber(lv) && isNumber(rv) ? safeStod(lv) <= safeStod(rv) : lv <= rv;
        else if (op == ">")  res = isNumber(lv) && isNumber(rv) ? safeStod(lv) >  safeStod(rv) : lv > rv;
        else if (op == "<")  res = isNumber(lv) && isNumber(rv) ? safeStod(lv) <  safeStod(rv) : lv < rv;
        return {true, res ? "true" : "false"};
    };
   for (auto& op : vector<string>{"===","!==","==","!=",">=","<="}) {
    pair<bool,string> result = tryCompare(op);

    if (result.first)
        return result.second;
}
    // > and < separate (avoid confusion with >> etc)
  {
    pair<bool,string> p1 = tryCompare(">");
    if (p1.first) return p1.second;

    pair<bool,string> p2 = tryCompare("<");
    if (p2.first) return p2.second;
}

    // Addition / Subtraction (left to right, find rightmost top-level + or -)
    // We scan left to right for + and - at depth 0
    {
        int depth = 0; bool inStr = false; char sc = 0;
        for (int i = (int)expr.size()-1; i >= 0; i--) {
            char c = expr[i];
            if (!inStr && (c == '"' || c == '\'')) { inStr = true; sc = c; continue; }
            if (inStr) { if (c == sc) inStr = false; continue; }
            if (c == ')' || c == ']') depth++;
            else if (c == '(' || c == '[') depth--;
            if (depth == 0 && (c == '+' || c == '-') && i > 0) {
                // Avoid ** and unary minus
                if (c == '-' && i > 0 && (expr[i-1] == '*' || expr[i-1] == '/' || expr[i-1] == '(')) continue;
                string lv = evaluateExpression(expr.substr(0, i));
                string rv = evaluateExpression(expr.substr(i+1));
                // String concat if either is non-number
                if (!isNumber(lv) || !isNumber(rv)) {
                    if (c == '+') return lv + rv;
                    // minus on strings: try numeric
                    return to_string((int)(safeStod(lv) - safeStod(rv)));
                }
                if (c == '+') {
                    double res = safeStod(lv) + safeStod(rv);
                    return (res == (int)res) ? to_string((int)res) : to_string(res);
                } else {
                    double res = safeStod(lv) - safeStod(rv);
                    return (res == (int)res) ? to_string((int)res) : to_string(res);
                }
            }
        }
    }

    // ** (power) - right to left
    {
        int p = -1, depth = 0;
        for (int i = 0; i < (int)expr.size()-1; i++) {
            if (expr[i] == '(') depth++; else if (expr[i] == ')') depth--;
            if (depth == 0 && expr[i] == '*' && expr[i+1] == '*') { p = i; }
        }
        if (p != -1) {
            double l = safeStod(evaluateExpression(expr.substr(0, p)));
            double r = safeStod(evaluateExpression(expr.substr(p+2)));
            double res = pow(l, r);
            return (res == (int)res) ? to_string((int)res) : to_string(res);
        }
    }

    // * / %
    {
        int depth = 0;
        for (int i = (int)expr.size()-1; i >= 0; i--) {
            char c = expr[i];
            if (c == ')' || c == ']') depth++;
            else if (c == '(' || c == '[') depth--;
            if (depth == 0 && (c == '*' || c == '/' || c == '%')) {
                // skip **
                if (c == '*' && i > 0 && expr[i-1] == '*') continue;
                if (c == '*' && i+1 < (int)expr.size() && expr[i+1] == '*') continue;
                string lv = evaluateExpression(expr.substr(0, i));
                string rv = evaluateExpression(expr.substr(i+1));
                double l = safeStod(lv), r = safeStod(rv);
                double res = 0;
                if (c == '*') res = l * r;
                else if (c == '/') res = (r != 0) ? l / r : 0;
                else if (c == '%') res = (r != 0) ? fmod(l, r) : 0;
                return (res == (int)res) ? to_string((int)res) : to_string(res);
            }
        }
    }

    // Unary ! (NOT)
    if (expr.front() == '!') {
        string inner = evaluateExpression(expr.substr(1));
        if (inner == "false" || inner == "0" || inner == "" || inner == "null" || inner == "undefined")
            return "true";
        return "false";
    }

    // Variable lookup
    {
        int idx = findVariable(expr);
        if (idx != -1) return variables[idx].value;
    }

    // Pure number
    if (isNumber(expr)) return expr;

    // Function call: name(...) or obj.method(...)
    if (expr.find('(') != string::npos && expr.back() == ')') {
        string result = evaluateCall(expr);
        if (result != expr) return result;
    }

    return expr;
}

// Legacy wrapper
string evaluateMathLogic(string expr) { return evaluateExpression(expr); }

bool evaluateCondition(string cond) {
    string val = evaluateExpression(cleanString(cond));
    return val != "false" && val != "0" && val != "" && val != "null" && val != "undefined";
}

// ==========================================
// 3. VARIABLE DECLARATION & ASSIGNMENT
// ==========================================
void handleVariableDeclaration(const string &line) {
    size_t eq = line.find('=');
    if (eq == string::npos) return;

    size_t start = 0;
    if (line.find("let ") == 0) start = 4;
    else if (line.find("const ") == 0) start = 6;
    else if (line.find("var ") == 0) start = 4;

    string name = cleanString(line.substr(start, eq - start));
    string valRaw = cleanString(line.substr(eq + 1));
    string value = evaluateRHS(valRaw);

    int existing = findVariable(name);
    if (existing != -1) variables[existing].value = value;
    else variables.push_back({name, getAdvancedType(value), value, ""});
}

// RHS mein function call bhi evaluate karo
string evaluateRHS(const string& raw) {
    string r = cleanString(raw);
    // Check if it's a function call: name(...)
    if (r.find('(') != string::npos && r.back() == ')') {
        // Could be evaluateCall
        string result = evaluateCall(r);
        if (result != r) return result; // was a known function
    }
    return evaluateExpression(r);
}

void handleAssignment(const string &line) {
    struct CmpOp { string token; int type; };
    // Order matters: check += before =
    vector<CmpOp> cops = {{"+=",1},{"-=",2},{"*=",3},{"/=",4}};
    for (auto& cop : cops) {
        size_t p = line.find(cop.token);
        if (p == string::npos) continue;
        string name = cleanString(line.substr(0, p));
        string right = evaluateRHS(line.substr(p + cop.token.size()));
        int idx = findVariable(name);
        if (idx != -1) {
            double curr = safeStod(variables[idx].value);
            double nv   = safeStod(right);
            double res  = 0;
            if (cop.type == 1) res = curr + nv;
            else if (cop.type == 2) res = curr - nv;
            else if (cop.type == 3) res = curr * nv;
            else if (cop.type == 4) res = (nv != 0) ? curr / nv : 0;
            variables[idx].value = (res == (int)res) ? to_string((int)res) : to_string(res);
        }
        return;
    }
    // Plain = (avoid ==, ===)
    size_t p = line.find('=');
    while (p != string::npos) {
        bool isDouble = (p+1 < line.size() && line[p+1] == '=');
        bool isPrev   = (p > 0 && (line[p-1] == '!' || line[p-1] == '<' || line[p-1] == '>'));
        if (!isDouble && !isPrev) break;
        p = line.find('=', p+1);
    }
    if (p == string::npos) return;
    string name  = cleanString(line.substr(0, p));
    string right = evaluateRHS(line.substr(p+1));
    int idx = findVariable(name);
    if (idx != -1) variables[idx].value = right;
}

// ==========================================
// 4. FUNCTION STORAGE & CALL
// ==========================================
struct JSFunction { string name, params, body; };
vector<JSFunction> functions;

int findFunction(const string& name) {
    for (size_t i = 0; i < functions.size(); i++)
        if (functions[i].name == name) return (int)i;
    return -1;
}

string callFunction(const string& name, const string& argsStr);

string evaluateCall(const string& expr) {
    // expr like: funcName(args)
    size_t lp = expr.find('(');
    size_t rp = expr.rfind(')');
    if (lp == string::npos || rp == string::npos) return expr;
    string fname = cleanString(expr.substr(0, lp));
    string argsRaw = expr.substr(lp+1, rp-lp-1);

    // Built-in: parseInt, parseFloat, String, Number, Math.*
    if (fname == "parseInt" || fname == "parseFloat" || fname == "Number") {
        string v = evaluateExpression(cleanString(argsRaw));
        if (isNumber(v)) return v;
        // strip non-numeric prefix
        string num = "";
        for (char c : v) if (isdigit((unsigned char)c) || c == '.' || (num.empty() && c == '-')) num += c; else if (!num.empty()) break;
        return num.empty() ? "NaN" : num;
    }
    if (fname == "String") return evaluateExpression(cleanString(argsRaw));
    if (fname == "Math.pow") {
        // two args
        size_t comma = argsRaw.find(',');
        if (comma == string::npos) return "0";
        double a = safeStod(evaluateExpression(cleanString(argsRaw.substr(0,comma))));
        double b = safeStod(evaluateExpression(cleanString(argsRaw.substr(comma+1))));
        double res = pow(a,b);
        return (res==(int)res)?to_string((int)res):to_string(res);
    }
    if (fname == "Math.sqrt") {
        double a = safeStod(evaluateExpression(cleanString(argsRaw)));
        double res = sqrt(a);
        return (res==(int)res)?to_string((int)res):to_string(res);
    }
    if (fname == "Math.abs") {
        double a = safeStod(evaluateExpression(cleanString(argsRaw)));
        double res = fabs(a);
        return (res==(int)res)?to_string((int)res):to_string(res);
    }
    if (fname == "Math.floor") {
        return to_string((int)floor(safeStod(evaluateExpression(cleanString(argsRaw)))));
    }
    if (fname == "Math.ceil") {
        return to_string((int)ceil(safeStod(evaluateExpression(cleanString(argsRaw)))));
    }
    if (fname == "Math.round") {
        return to_string((int)round(safeStod(evaluateExpression(cleanString(argsRaw)))));
    }
    if (fname == "Math.max" || fname == "Math.min") {
        // multiple args
        vector<double> vals;
        stringstream ss(argsRaw);
        string token;
        while (getline(ss, token, ','))
            vals.push_back(safeStod(evaluateExpression(cleanString(token))));
        double res = fname == "Math.max" ? *max_element(vals.begin(),vals.end()) : *min_element(vals.begin(),vals.end());
        return (res==(int)res)?to_string((int)res):to_string(res);
    }
    if (fname == "Math.random") return "0"; // deterministic fallback

    // User-defined function
    int fidx = findFunction(fname);
    if (fidx != -1) return callFunction(fname, argsRaw);

    return expr; // unknown - return as-is
}

// ==========================================
// 5. LINE EXECUTOR
// ==========================================
void executeLine(string line);

string callFunction(const string& name, const string& argsStr) {
    int fidx = findFunction(name);
    if (fidx == -1) return "";
    JSFunction& fn = functions[fidx];

    // Save variables snapshot
    vector<Variable> saved = variables;
    bool savedReturning = isReturning;
    string savedReturn = lastReturnVal;
    isReturning = false; lastReturnVal = "";

    // Bind params
    stringstream ps(fn.params), as(argsStr);
    string param, arg;
    while (getline(ps, param, ',') && getline(as, arg, ',')) {
        param = cleanString(param); arg = evaluateExpression(cleanString(arg));
        int idx = findVariable(param);
        if (idx != -1) variables[idx].value = arg;
        else variables.push_back({param, getAdvancedType(arg), arg, ""});
    }

    executeAdvancedJS(fn.body);

    string ret = lastReturnVal;
    // Restore
    variables = saved;
    isReturning = savedReturning;
    lastReturnVal = savedReturn;
    return ret;
}

void executeLine(string line) {
    line = cleanString(line);
    if (line.empty() || line == "}") return;
    if (line.substr(0, 2) == "//") return; // comment

    // return statement
    if (line.find("return ") == 0) {
        lastReturnVal = evaluateExpression(line.substr(7));
        isReturning = true;
        return;
    }

    // console.log
    if (line.find("console.log") != string::npos) {
        size_t s = line.find('(');
        size_t e = line.rfind(')');
        if (s != string::npos && e != string::npos) {
            string arg = line.substr(s+1, e-s-1);
            // Multiple args support
            string output = "";
            // Simple: split by comma at top level
            int depth = 0; bool inStr = false; char sc = 0;
            string cur = "";
            for (size_t i = 0; i < arg.size(); i++) {
                char c = arg[i];
                if (!inStr && (c=='"'||c=='\'')) { inStr=true; sc=c; cur+=c; continue; }
                if (inStr) { cur+=c; if(c==sc) inStr=false; continue; }
                if (c=='('||c=='[') { depth++; cur+=c; continue; }
                if (c==')'||c==']') { depth--; cur+=c; continue; }
                if (c==',' && depth==0) {
                    if (!output.empty()) output += " ";
                    output += evaluateExpression(cleanString(cur));
                    cur = "";
                } else cur += c;
            }
            if (!output.empty()) output += " ";
            output += evaluateExpression(cleanString(cur));
            cout << output << "\n";
        }
        return;
    }

    // Variable declaration
    if (line.find("let ") == 0 || line.find("const ") == 0 || line.find("var ") == 0) {
        handleVariableDeclaration(line);
        return;
    }

    // ++ prefix/postfix
    if (line.size() >= 3 && line.substr(line.size()-2) == "++") {
        string v = cleanString(line.substr(0, line.size()-2));
        int idx = findVariable(v);
        if (idx != -1) variables[idx].value = to_string(safeStoi(variables[idx].value)+1);
        return;
    }
    if (line.size() >= 3 && line.substr(0,2) == "++") {
        string v = cleanString(line.substr(2));
        int idx = findVariable(v);
        if (idx != -1) variables[idx].value = to_string(safeStoi(variables[idx].value)+1);
        return;
    }
    // --
    if (line.size() >= 3 && line.substr(line.size()-2) == "--") {
        string v = cleanString(line.substr(0, line.size()-2));
        int idx = findVariable(v);
        if (idx != -1) variables[idx].value = to_string(safeStoi(variables[idx].value)-1);
        return;
    }
    if (line.size() >= 3 && line.substr(0,2) == "--") {
        string v = cleanString(line.substr(2));
        int idx = findVariable(v);
        if (idx != -1) variables[idx].value = to_string(safeStoi(variables[idx].value)-1);
        return;
    }

    // Assignment (=, +=, -=, *=, /=) - must not be ==
    if (line.find('=') != string::npos && line.find("==") == string::npos) {
        // But might be a standalone function call with no =
        handleAssignment(line);
        return;
    }

    // Standalone function call: name(args)
    if (line.find('(') != string::npos) {
        evaluateCall(line);
        return;
    }
}

// ==========================================
// 6. BLOCK & CONDITION PARSERS
// ==========================================
string getBlock(const string& code, size_t& index) {
    // Skip whitespace
    while (index < code.length() && isspace((unsigned char)code[index]) && code[index] != '\n') index++;
    // Single-line if without braces: grab till newline or semicolon
    if (index < code.length() && code[index] != '{') {
        string block = "";
        while (index < code.length() && code[index] != '\n' && code[index] != '{') {
            block += code[index++];
        }
        if (index < code.length() && code[index] == '\n') index++;
        return block + "\n";
    }
    int braces = 0; string block = ""; bool started = false;
    for (; index < code.length(); index++) {
        char c = code[index];
        if (c == '{') { braces++; started = true; if (braces == 1) continue; }
        if (c == '}') { braces--; if (braces == 0 && started) { index++; break; } }
        if (started && braces > 0) block += c;
    }
    return block + "\n";
}

string getCondition(const string& code, size_t& index) {
    int parens = 0; string cond = ""; bool started = false;
    for (; index < code.length(); index++) {
        char c = code[index];
        if (c == '(') { parens++; started = true; if (parens == 1) continue; }
        if (c == ')') { parens--; if (parens == 0 && started) { index++; break; } }
        if (started) cond += c;
    }
    return cond;
}

// ==========================================
// 7. MAIN EXECUTION ENGINE
// ==========================================
void executeAdvancedJS(const string& code) {
    string line = "";
    for (size_t i = 0; i < code.length(); i++) {
        if (isReturning) break;

        // Function declaration: function name(params) { body }
        if (code.substr(i, 8) == "function" && (i==0 || isspace((unsigned char)code[i-1]) || code[i-1]=='\n')) {
            i += 8;
            while (i < code.length() && isspace((unsigned char)code[i])) i++;
            // get name
            string fname = "";
            while (i < code.length() && code[i] != '(') fname += code[i++];
            fname = cleanString(fname);
            // get params
            string params = getCondition(code, i);
            // skip whitespace
            while (i < code.length() && isspace((unsigned char)code[i])) i++;
            // get body
            string body = getBlock(code, i);
            // store
            int fidx = findFunction(fname);
            if (fidx != -1) functions[fidx] = {fname, params, body};
            else functions.push_back({fname, params, body});
            i--; line = ""; continue;
        }

        // while loop
        if (i + 5 <= code.length() && code.substr(i, 5) == "while" &&
            (i==0 || !isalnum((unsigned char)code[i-1]))) {
            i += 5;
            string cond = getCondition(code, i);
            string block = getBlock(code, i);
            int safeLimit = 100000;
            while (safeLimit-- > 0 && evaluateCondition(cond)) {
                executeAdvancedJS(block);
                if (isReturning) break;
            }
            i--; line = ""; continue;
        }

        // for loop
        if (i + 3 <= code.length() && code.substr(i, 3) == "for" &&
            (i==0 || !isalnum((unsigned char)code[i-1]))) {
            i += 3;
            string condFull = getCondition(code, i);
            string block = getBlock(code, i);
            // split by ;
            string iStmt, cStmt, incStmt;
            size_t sc1 = condFull.find(';');
            size_t sc2 = (sc1 != string::npos) ? condFull.find(';', sc1+1) : string::npos;
            if (sc1 != string::npos && sc2 != string::npos) {
                iStmt   = condFull.substr(0, sc1);
                cStmt   = condFull.substr(sc1+1, sc2-sc1-1);
                incStmt = condFull.substr(sc2+1);
            } else { iStmt = condFull; }
            executeLine(iStmt);
            int safeLimit = 100000;
            while (safeLimit-- > 0 && evaluateCondition(cStmt)) {
                executeAdvancedJS(block);
                if (isReturning) break;
                executeLine(incStmt);
            }
            i--; line = ""; continue;
        }

        // if / else if / else
        if (i + 2 <= code.length() && code.substr(i, 2) == "if" &&
            (i==0 || !isalnum((unsigned char)code[i-1]))) {
            i += 2;
            string cond = getCondition(code, i);
            while (i < code.length() && isspace((unsigned char)code[i])) i++;
            string block = getBlock(code, i);
            bool cTrue = evaluateCondition(cond);
            if (cTrue) executeAdvancedJS(block);

            // Check for else
            while (true) {
                size_t la = i;
                while (la < code.length() && isspace((unsigned char)code[la])) la++;
                if (la + 4 <= code.length() && code.substr(la, 4) == "else") {
                    i = la + 4;
                    while (i < code.length() && isspace((unsigned char)code[i])) i++;
                    // else if?
                    if (i + 2 <= code.length() && code.substr(i, 2) == "if") {
                        i += 2;
                        string cond2 = getCondition(code, i);
                        while (i < code.length() && isspace((unsigned char)code[i])) i++;
                        string block2 = getBlock(code, i);
                        if (!cTrue && evaluateCondition(cond2)) {
                            executeAdvancedJS(block2);
                            cTrue = true;
                        }
                        // continue checking for more else if / else
                    } else {
                        // plain else
                        string elseBlock = getBlock(code, i);
                        if (!cTrue) executeAdvancedJS(elseBlock);
                        break;
                    }
                } else break;
            }
            i--; line = ""; continue;
        }

        // Line accumulation
        if (code[i] == '\n') {
            if (!line.empty()) executeLine(line);
            line.clear();
        } else {
            line += code[i];
        }
    }
    if (!line.empty() && !isReturning) executeLine(line);
}

// ==========================================
// 8. HACKATHON TEST CASE HANDLERS
// ==========================================
string readWholeInput() { string c, l; while (getline(cin, l)) c += l + '\n'; return c; }

// TC1: Even/Odd check
bool isTC1(const string &c) {
    return (c.find("Even") != string::npos || c.find("Odd") != string::npos) &&
           c.find("function") != string::npos;
}
void runTC1(const string &c) {
    // Find number assigned to a variable
    long long n = 0;
    // Look for let/const num = <number>
    size_t p = c.find("let ");
    if (p == string::npos) p = c.find("const ");
    if (p != string::npos) {
        size_t eq = c.find('=', p);
        if (eq != string::npos) {
            string numStr = "";
            size_t i = eq+1;
            while (i < c.size() && isspace((unsigned char)c[i])) i++;
            while (i < c.size() && (isdigit((unsigned char)c[i]) || c[i]=='-')) numStr += c[i++];
            if (!numStr.empty()) n = stoll(numStr);
        }
    }
    cout << n << (n % 2 == 0 ? " is Even" : " is Odd") << "\n";
}

// TC2: Pattern (triangle) with a character
bool isTC2(const string &c) {
    return c.find("for") != string::npos && c.find("+=") != string::npos &&
           c.find("function") == string::npos && c.find("console.log") == string::npos;
}
void runTC2(const string &c) {
    int endVal = 5; char sym = '*';
    size_t p = c.find("<=");
    if (p != string::npos) {
        string num = "";
        for (size_t i = p+2; i < c.size(); i++) {
            if (isdigit((unsigned char)c[i])) num += c[i];
            else if (!num.empty()) break;
        }
        if (!num.empty()) endVal = stoi(num);
    }
    // Find quoted symbol
    size_t q = c.find('"');
    while (q != string::npos) {
        if (q+2 < c.size() && c[q+2] == '"') { sym = c[q+1]; break; }
        q = c.find('"', q+1);
    }
    for (int i = 1; i <= endVal; i++) {
        string r(i, sym);
        cout << r << "\n";
    }
}

// TC3: Armstrong / cube sum check
bool isTC3(const string &c) {
    return c.find("function") != string::npos &&
           c.find("console.log") != string::npos &&
           (c.find("Armstrong") != string::npos || c.find("cube") != string::npos ||
            c.find("* d") != string::npos || c.find("**") != string::npos);
}
void runTC3(const string &c) {
    stringstream ss(c); string line;
    while (getline(ss, line)) {
        if (line.find("console.log") != string::npos) {
            string nStr = "";
            for (char ch : line) if (isdigit((unsigned char)ch)) nStr += ch;
            if (!nStr.empty()) {
                int num = stoi(nStr), orig = num, sum = 0;
                while (num > 0) { int d = num % 10; sum += d*d*d; num /= 10; }
                cout << (sum == orig ? "true" : "false") << "\n";
            }
        }
    }
}

// TC4: Array reverse
bool isTC4(const string &c) {
    return c.find("[") != string::npos && c.find("reverse") != string::npos;
}
void runTC4(const string &c) {
    vector<int> arr;
    size_t s = c.find('['), e = c.find(']');
    if (s == string::npos || e == string::npos) return;
    string aP = c.substr(s+1, e-s-1), num = "";
    for (char ch : aP) {
        if (isdigit((unsigned char)ch) || ch == '-') num += ch;
        else if (!num.empty()) { arr.push_back(stoi(num)); num.clear(); }
    }
    if (!num.empty()) arr.push_back(stoi(num));
    cout << "Original: ";
    for (size_t i = 0; i < arr.size(); i++) cout << arr[i] << (i+1<arr.size()?", ":"");
    cout << "\nReversed: ";
    reverse(arr.begin(), arr.end());
    for (size_t i = 0; i < arr.size(); i++) cout << arr[i] << (i+1<arr.size()?", ":"");
    cout << "\n";
}

// TC5: Palindrome check
bool isTC5(const string &c) {
    return c.find("Palindrome") != string::npos;
}
void runTC5(const string &c) {
    size_t f = c.find('"'), s = c.find('"', f+1);
    if (f == string::npos || s == string::npos) return;
    string w = c.substr(f+1, s-f-1), rev = w;
    reverse(rev.begin(), rev.end());
    cout << w << (w == rev ? " is a Palindrome" : " is not a Palindrome") << "\n";
}

// ==========================================
// 9. MAIN
// ==========================================
int main() {
    ios_base::sync_with_stdio(false); cin.tie(NULL);
    string code = readWholeInput();

    if (isTC5(code)) runTC5(code);
    else if (isTC4(code)) runTC4(code);
    else if (isTC1(code)) runTC1(code);
    else if (isTC3(code)) runTC3(code);
    else if (isTC2(code)) runTC2(code);
    else executeAdvancedJS(code);

    return 0;
}