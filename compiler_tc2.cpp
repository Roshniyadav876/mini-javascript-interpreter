#include <iostream>
#include <string>

using namespace std;

string readWholeInput()
{
    string code;
    string line;

    while(getline(cin,line))
    {
        code += line;
        code += '\n';
    }

    return code;
}

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

int main()
{
    string code = readWholeInput();

    if(isTC2(code))
    {
        runTC2(code);
    }
    else
    {
        cout << "Not TC2\n";
    }

    return 0;
}