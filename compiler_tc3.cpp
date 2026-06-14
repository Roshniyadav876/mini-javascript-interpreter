#include <iostream>
#include <string>

using namespace std;

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

int main()
{
    string code = readWholeInput();

    if(isTC3(code))
    {
        runTC3(code);
    }
    else
    {
        cout << "Not TC3\n";
    }

    return 0;
}