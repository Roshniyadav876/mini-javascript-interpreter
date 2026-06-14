#include <iostream>
#include <string>
using namespace std;

string readWholeInput()
{
    string code;
    string line;

    while (getline(cin, line))
    {
        code += line;
        code += '\n';
    }

    return code;
}

bool isTC1(const string &code)
{
    return code.find("%") != string::npos;
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

int main()
{
    string code = readWholeInput();

    if (isTC1(code))
    {
        runTC1(code);
    }
    else
    {
        cout << "Unknown Program\n";
    }

    return 0;

    // =====================================
    // TC1 : Odd Even Checker
    // Status : FROZEN ✅
    // =====================================
}