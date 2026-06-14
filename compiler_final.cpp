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
// BUG FIX: depth-aware comma splitter for function args / params
// Splits a string on top-level commas only, respecting (), [], and quotes.
// Fixes nested call argument parsing, e.g. add(add(1,2), add(3,4))
// ==========================================
vector<string> splitArgs(const string& argsRaw) {
    vector<string> result;
    int depth = 0; bool inStr = false; char sc = 0;
    string cur = "";
    for (size_t i = 0; i < argsRaw.size(); i++) {
        char c = argsRaw[i];
        if (!inStr && (c == '"' || c == '\'')) { inStr = true; sc = c; cur += c; continue; }
        if (inStr) { cur += c; if (c == sc) inStr = false; continue; }
        if (c == '(' || c == '[') { depth++; cur += c; continue; }
        if (c == ')' || c == ']') { depth--; cur += c; continue; }
        if (c == ',' && depth == 0) { result.push_back(cleanString(cur)); cur = ""; continue; }
        cur += c;
    }
    // Push final token; only skip if everything was empty (no args at all)
    string trimmedFinal = cleanString(cur);
    if (!trimmedFinal.empty() || !result.empty()) result.push_back(trimmedFinal);
    return result;
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

// ==========================================
// ARRAY RUNTIME SUPPORT
// Arrays are represented in canonical "[el1, el2, el3]" string form so
// they round-trip through console.log and Variable.value naturally.
// ==========================================

// True if value (already-evaluated string) looks like an array literal "[...]"
bool isArrayValue(const string& v) {
    string s = cleanString(v);
    return s.size() >= 2 && s.front() == '[' && s.back() == ']';
}

// Parse a canonical array string "[1, 2, 3]" into element strings.
// Elements are stored already-evaluated (numbers as plain numbers,
// strings WITHOUT quotes is ambiguous with numbers, so we keep string
// elements quoted internally to disambiguate types on reverse/print).
vector<string> parseArrayElements(const string& v) {
    string s = cleanString(v);
    if (s.size() < 2 || s.front() != '[' || s.back() != ']') return {};
    string inner = s.substr(1, s.size() - 2);
    if (cleanString(inner).empty()) return {};
    return splitArgs(inner);
}

// Build canonical array string from element strings (elements should
// already be in their display form, e.g. "1", "\"hi\"", etc.)
string buildArrayValue(const vector<string>& elems) {
    string out = "[";
    for (size_t i = 0; i < elems.size(); i++) {
        out += elems[i];
        if (i + 1 < elems.size()) out += ", ";
    }
    out += "]";
    return out;
}

// Evaluate an array literal "[expr1, expr2, ...]" into canonical array value.
// Numeric/boolean/null elements are stored bare; string elements are stored
// with quotes so printing/reversal preserves type distinctions.
string evaluateArrayLiteral(const string& expr) {
    string inner = expr.substr(1, expr.size() - 2);
    vector<string> parts = splitArgs(inner);
    vector<string> elems;
    for (auto& p : parts) {
        if (p.empty()) continue;

        // BUG FIX: spread operator inside array literals, e.g. [...arr]
        // or [0, ...arr, 99]. Splice the spread array's elements in place
        // instead of evaluating "...arr" as a single bogus element.
        if (p.size() >= 3 && p[0] == '.' && p[1] == '.' && p[2] == '.') {
            string spreadExpr = cleanString(p.substr(3));
            string spreadVal = evaluateExpression(spreadExpr);
            if (isArrayValue(spreadVal)) {
                vector<string> spreadElems = parseArrayElements(spreadVal);
                for (auto& se : spreadElems) elems.push_back(se);
            } else {
                // Spreading a string spreads its characters
                for (char ch : spreadVal) elems.push_back(string("\"") + ch + "\"");
            }
            continue;
        }

        string val = evaluateExpression(p);
        // If the evaluated result isn't numeric/bool/null/array/object,
        // treat it as a string and re-quote for storage.
        if (isNumber(val) || val == "true" || val == "false" || val == "null" ||
            val == "undefined" || isArrayValue(val)) {
            elems.push_back(val);
        } else {
            elems.push_back("\"" + val + "\"");
        }
    }
    return buildArrayValue(elems);
}

// For console.log / display: convert canonical array storage form into
// JS-style printed form (strip the extra quotes added for storage).
string formatArrayForDisplay(const string& v) {
    vector<string> elems = parseArrayElements(v);
    vector<string> out;
    for (auto& e : elems) {
        if (e.size() >= 2 && e.front() == '"' && e.back() == '"')
            out.push_back(e.substr(1, e.size() - 2));
        else
            out.push_back(e);
    }
    return buildArrayValue(out);
}

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

    // Array literal: [el1, el2, ...]
    if (expr.front() == '[' && expr.back() == ']') {
        return evaluateArrayLiteral(expr);
    }

    // .length property: <something>.length  (works for strings, arrays, vars)
    if (expr.size() > 7 && expr.substr(expr.size()-7) == ".length") {
        string base = cleanString(expr.substr(0, expr.size()-7));
        string val = evaluateExpression(base);
        if (isArrayValue(val)) return to_string(parseArrayElements(val).size());
        return to_string(val.size());
    }

    // Array/string method calls: <base>.method(args)
    // e.g. arr.reverse(), arr.push(5), arr.join(""), str.toUpperCase()
    {
        // Find a top-level ".method(" pattern with matching trailing ')'
        if (!expr.empty() && expr.back() == ')') {
            int depth = 0;
            for (int i = (int)expr.size()-1; i >= 0; i--) {
                char c = expr[i];
                if (c == ')') depth++;
                else if (c == '(') { depth--; if (depth == 0) {
                    size_t openParen = (size_t)i;
                    // look backwards for a '.' at depth 0 before openParen
                    int d2 = 0;
                    for (int j = (int)openParen - 1; j >= 0; j--) {
                        char cj = expr[j];
                        if (cj == ')' || cj == ']') d2++;
                        else if (cj == '(' || cj == '[') d2--;
                        if (d2 == 0 && cj == '.') {
                            string base = expr.substr(0, j);
                            string method = expr.substr(j+1, openParen-(j+1));
                            string argsRaw = expr.substr(openParen+1, expr.size()-1-(openParen+1));
                            // Skip known global namespaces handled elsewhere (Math.*)
                            if (base == "Math") break;
                            string baseVal = evaluateExpression(base);

                            if (isArrayValue(baseVal)) {
                                vector<string> elems = parseArrayElements(baseVal);
                                if (method == "reverse") {
                                    reverse(elems.begin(), elems.end());
                                    string newVal = buildArrayValue(elems);
                                    // mutate original variable if base is a simple var name
                                    int vidx = findVariable(cleanString(base));
                                    if (vidx != -1) variables[vidx].value = newVal;
                                    return newVal;
                                }
                                if (method == "push") {
                                    vector<string> args = splitArgs(argsRaw);
                                    for (auto& a : args) {
                                        if (a.empty()) continue;
                                        string v = evaluateExpression(a);
                                        if (isNumber(v) || v=="true"||v=="false"||v=="null"||v=="undefined"||isArrayValue(v))
                                            elems.push_back(v);
                                        else
                                            elems.push_back("\"" + v + "\"");
                                    }
                                    string newVal = buildArrayValue(elems);
                                    int vidx = findVariable(cleanString(base));
                                    if (vidx != -1) variables[vidx].value = newVal;
                                    return to_string(elems.size());
                                }
                                if (method == "join") {
                                    string sep = ",";
                                    string a = cleanString(argsRaw);
                                    if (!a.empty()) sep = evaluateExpression(a);
                                    string out = "";
                                    for (size_t k = 0; k < elems.size(); k++) {
                                        string e = elems[k];
                                        if (e.size()>=2 && e.front()=='"' && e.back()=='"') e = e.substr(1,e.size()-2);
                                        out += e;
                                        if (k+1 < elems.size()) out += sep;
                                    }
                                    return out;
                                }
                                if (method == "pop") {
                                    if (elems.empty()) return "undefined";
                                    string last = elems.back();
                                    elems.pop_back();
                                    string newVal = buildArrayValue(elems);
                                    int vidx = findVariable(cleanString(base));
                                    if (vidx != -1) variables[vidx].value = newVal;
                                    if (last.size()>=2 && last.front()=='"' && last.back()=='"') return last.substr(1,last.size()-2);
                                    return last;
                                }
                            } else {
                                // string methods
                                if (method == "toUpperCase") {
                                    string out = baseVal;
                                    transform(out.begin(), out.end(), out.begin(), ::toupper);
                                    return out;
                                }
                                if (method == "toLowerCase") {
                                    string out = baseVal;
                                    transform(out.begin(), out.end(), out.begin(), ::tolower);
                                    return out;
                                }
                                if (method == "charAt") {
                                    int idx = (int)safeStod(evaluateExpression(cleanString(argsRaw)));
                                    if (idx >= 0 && idx < (int)baseVal.size()) return string(1, baseVal[idx]);
                                    return "";
                                }
                                if (method == "split") {
                                    string sep = "";
                                    string a = cleanString(argsRaw);
                                    if (!a.empty()) sep = evaluateExpression(a);
                                    vector<string> elems;
                                    if (sep.empty()) {
                                        for (char ch : baseVal) elems.push_back(string("\"") + ch + "\"");
                                    } else {
                                        size_t pos = 0, found;
                                        while ((found = baseVal.find(sep, pos)) != string::npos) {
                                            elems.push_back("\"" + baseVal.substr(pos, found-pos) + "\"");
                                            pos = found + sep.size();
                                        }
                                        elems.push_back("\"" + baseVal.substr(pos) + "\"");
                                    }
                                    return buildArrayValue(elems);
                                }
                            }
                            // Unknown method: fall through to generic call handling
                            break;
                        }
                    }
                    break;
                }}
            }
        }
    }

    // Indexing: name[expr] or "literal"[expr]  -- string/array element access
    if (!expr.empty() && expr.back() == ']') {
        int depth = 0;
        for (int i = (int)expr.size()-1; i >= 0; i--) {
            char c = expr[i];
            if (c == ']') depth++;
            else if (c == '[') { depth--; if (depth == 0) {
                if (i == 0) break; // "[1,2,3]" itself, not indexing - handled above
                string base = expr.substr(0, i);
                string idxExpr = expr.substr(i+1, expr.size()-1-(i+1));
                string baseVal = evaluateExpression(base);
                int idx = (int)safeStod(evaluateExpression(idxExpr));
                if (isArrayValue(baseVal)) {
                    vector<string> elems = parseArrayElements(baseVal);
                    if (idx < 0 || idx >= (int)elems.size()) return "undefined";
                    string e = elems[idx];
                    if (e.size()>=2 && e.front()=='"' && e.back()=='"') return e.substr(1,e.size()-2);
                    return e;
                } else {
                    // string indexing
                    if (idx < 0 || idx >= (int)baseVal.size()) return "undefined";
                    return string(1, baseVal[idx]);
                }
            }}
        }
    }

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
            // BUG FIX: += on a non-numeric current value (or non-numeric RHS)
            // means string concatenation in JS, e.g. row += "*".
            // Previously this always coerced both sides to numbers via
            // safeStod, turning string-building loops into "0".
            if (cop.type == 1 && (!isNumber(variables[idx].value) || !isNumber(right))) {
                variables[idx].value = variables[idx].value + right;
                return;
            }
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

    // BUG FIX / NEW: indexed assignment, e.g. arr[0] = 99;
    if (!name.empty() && name.back() == ']') {
        size_t lb = name.find('[');
        if (lb != string::npos) {
            string base = cleanString(name.substr(0, lb));
            string idxExpr = name.substr(lb+1, name.size()-1-(lb+1));
            int idx = (int)safeStod(evaluateExpression(idxExpr));
            int vidx = findVariable(base);
            if (vidx != -1 && isArrayValue(variables[vidx].value)) {
                vector<string> elems = parseArrayElements(variables[vidx].value);
                while ((int)elems.size() <= idx) elems.push_back("undefined");
                if (isNumber(right) || right=="true"||right=="false"||right=="null"||right=="undefined"||isArrayValue(right))
                    elems[idx] = right;
                else
                    elems[idx] = "\"" + right + "\"";
                variables[vidx].value = buildArrayValue(elems);
            }
            return;
        }
    }

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

    // BUG FIX: use splitArgs() instead of naive find(',') so nested calls
    // like Math.pow(2, add(1,2)) parse correctly.
    if (fname == "Math.pow") {
        vector<string> args = splitArgs(argsRaw);
        if (args.size() < 2) return "0";
        double a = safeStod(evaluateExpression(args[0]));
        double b = safeStod(evaluateExpression(args[1]));
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
        // BUG FIX: use splitArgs() instead of stringstream+getline(',')
        // so nested calls inside Math.max/min args parse correctly.
        vector<string> args = splitArgs(argsRaw);
        vector<double> vals;
        for (auto& a : args) vals.push_back(safeStod(evaluateExpression(a)));
        if (vals.empty()) return "0";
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

    // BUG FIX: bind params using depth-aware splitArgs() instead of
    // stringstream + getline(',') which broke on nested calls like
    // add(add(1,2), add(3,4)) -- the inner commas were treated as
    // argument separators, scrambling the argument list.
    vector<string> params = splitArgs(fn.params);
    vector<string> args   = splitArgs(argsStr);

    for (size_t i = 0; i < params.size(); i++) {
        string param = cleanString(params[i]);
        if (param.empty()) continue;
        string arg = (i < args.size() && !args[i].empty())
                        ? evaluateExpression(args[i])
                        : "undefined";
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
                    string val = evaluateExpression(cleanString(cur));
                    output += isArrayValue(val) ? formatArrayForDisplay(val) : val;
                    cur = "";
                } else cur += c;
            }
            if (!output.empty()) output += " ";
            {
                string val = evaluateExpression(cleanString(cur));
                output += isArrayValue(val) ? formatArrayForDisplay(val) : val;
            }
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

    // Standalone function call: name(args) or obj.method(args)
    // BUG FIX: previously called evaluateCall() directly, which only knows
    // about "name(args)" forms (built-ins / user functions) and does not
    // understand ".method(args)" syntax like arr.reverse() or arr.push(5).
    // Routing through evaluateExpression() lets the array/string method
    // handlers there run (and mutate the underlying variable).
    if (line.find('(') != string::npos) {
        evaluateExpression(line);
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
// 8. INPUT READING
// ==========================================
string readWholeInput() { string c, l; while (getline(cin, l)) c += l + '\n'; return c; }

// ==========================================
// 9. MAIN
// ==========================================
// NOTE: All hardcoded "hackathon test case" detectors/handlers (isTC1..isTC5,
// runTC1..runTC5) have been removed. The interpreter now ALWAYS executes the
// input as real JavaScript through executeAdvancedJS(), which evaluates
// variables, expressions, loops, conditionals, functions, and arrays
// dynamically -- so output automatically reflects whatever input values the
// program actually contains (e.g. changing `let num = 7` to `let num = 10`
// changes the printed result without any code changes here).
int main() {
    ios_base::sync_with_stdio(false); cin.tie(NULL);
    string code = readWholeInput();
    executeAdvancedJS(code);
    return 0;
}