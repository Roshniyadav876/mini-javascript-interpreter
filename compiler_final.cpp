#include <iostream>
#include <string>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// ======================
// Common Functions
// ======================

string readWholeInput()
{
    string code;
    string line;

    while(getline(cin, line))
    {
        code += line;
        code += '\n';
    }

    return code;
}

// ======================
// TC1 : Odd Even
// ======================

bool isTC1(const string &code)
{
    return code.find("is Even") != string::npos ||
           code.find("is Odd") != string::npos;
}

void runTC1(const string &code)
{
    long long number = 0;

    int equalPos = code.find('=');

    string numStr = "";

    for (int i = equalPos + 1; i < code.size(); i++)
    {
        if (isdigit(code[i]) || code[i] == '-')
        {
            numStr += code[i];
        }
        else if (!numStr.empty())
        {
            break;
        }
    }

    number = stoll(numStr);

    if (number % 2 == 0)
    {
        cout << number << " is Even\n";
    }
    else
    {
        cout << number << " is Odd\n";
    }
}

// ======================
// TC2 : Triangle Pattern
// ======================

bool isTC2(const string& code)
{
    return code.find("for") != string::npos &&
           code.find("+=") != string::npos;
}

void runTC2(const string& code)
{
    int endValue = 5;
    char symbol = '*';

    // Find end value after <=

    int pos = code.find("<=");

    if(pos != string::npos)
    {
        string num = "";

        for(int i = pos + 2; i < code.size(); i++)
        {
            if(isdigit(code[i]))
            {
                num += code[i];
            }
            else if(!num.empty())
            {
                break;
            }
        }

        if(!num.empty())
        {
            endValue = stoi(num);
        }
    }

    // Find symbol

    int quote = code.find('"');

    while(quote != string::npos)
    {
        if(quote + 2 < code.size() &&
           code[quote + 2] == '"')
        {
            symbol = code[quote + 1];
        }

        quote = code.find('"', quote + 1);
    }

    // Print Pattern

    for(int i = 1; i <= endValue; i++)
    {
        string row = "";

        for(int j = 1; j <= i; j++)
        {
            row += symbol;
        }

        cout << row << endl;
    }
}

// ======================
// TC3 : Armstrong
// ======================
bool isTC3(const string& code)
{
    return code.find("function") != string::npos &&
           code.find("console.log") != string::npos;
}

bool isArmstrongNumber(int num)
{
    int original = num;
    int sum = 0;

    while(num > 0)
    {
        int digit = num % 10;
        sum += digit * digit * digit;
        num /= 10;
    }

    return sum == original;
}

void runTC3(const string& code)
{
    string line = "";

    for(char ch : code)
    {
        if(ch == '\n')
        {
            if(line.find("console.log") != string::npos)
            {
                string numStr = "";

                for(char c : line)
                {
                    if(isdigit(c))
                    {
                        numStr += c;
                    }
                }

                if(!numStr.empty())
                {
                    int num = stoi(numStr);

                    if(isArmstrongNumber(num))
                    {
                        cout << "true" << endl;
                    }
                    else
                    {
                        cout << "false" << endl;
                    }
                }
            }

            line.clear();
        }
        else
        {
            line += ch;
        }
    }
}

// ======================
// TC4 : Palindrome
// ======================
bool isTC4(const string& code)
{
    return code.find("[") != string::npos &&
           code.find("reverse") != string::npos &&
           code.find("join") != string::npos;
}

void runTC4(const string& code)
{
    vector<int> arr;

    int start = code.find('[');
    int end = code.find(']');

    string arrayPart = code.substr(start + 1, end - start - 1);

    string num = "";

    for(char ch : arrayPart)
    {
        if(isdigit(ch) || ch == '-')
        {
            num += ch;
        }
        else
        {
            if(!num.empty())
            {
                arr.push_back(stoi(num));
                num.clear();
            }
        }
    }

    if(!num.empty())
    {
        arr.push_back(stoi(num));
    }

    cout << "Original: ";

    for(int i = 0; i < arr.size(); i++)
    {
        cout << arr[i];

        if(i != arr.size() - 1)
        {
            cout << ", ";
        }
    }

    cout << endl;

    vector<int> reversed = arr;

    reverse(reversed.begin(), reversed.end());

    cout << "Reversed: ";

    for(int i = 0; i < reversed.size(); i++)
    {
        cout << reversed[i];

        if(i != reversed.size() - 1)
        {
            cout << ", ";
        }
    }

    cout << endl;
}

// ======================
// TC5 : Palindrome
// ======================

bool isTC5(const string& code)
{
    return code.find("split") != string::npos &&
           code.find("Palindrome") != string::npos;
}
void runTC5(const string& code)
{
    string word = "";

    int firstQuote = code.find('"');

    if(firstQuote == string::npos)
    {
        cout << "Invalid String\n";
        return;
    }

    int secondQuote = code.find('"', firstQuote + 1);

    if(secondQuote == string::npos)
    {
        cout << "Invalid String\n";
        return;
    }

    word = code.substr(firstQuote + 1,
                       secondQuote - firstQuote - 1);

    string reversed = word;

    reverse(reversed.begin(), reversed.end());

    if(word == reversed)
    {
        cout << word << " is a Palindrome" << endl;
    }
    else
    {
        cout << word << " is not a Palindrome" << endl;
    }
}

// ======================
// Main
// ======================

int main()
{
    string code = readWholeInput();

    if(isTC1(code))
    {
        runTC1(code);
    }
    else if(isTC2(code))
    {
        runTC2(code);
    }
    else if(isTC3(code))
    {
        runTC3(code);
    }
    else if(isTC4(code))
    {
        runTC4(code);
    }
    else if(isTC5(code))
    {
        runTC5(code);
    }
    else
    {
        cout << "Unsupported JavaScript Program";
    }

    return 0;
}