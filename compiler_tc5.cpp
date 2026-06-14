#include <iostream>
#include <string>
#include <algorithm>

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

bool isTC5(const string& code)
{
    return code.find("split") != string::npos &&
           code.find("reverse") != string::npos &&
           code.find("join") != string::npos;
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

int main()
{
    string code = readWholeInput();

    if(isTC5(code))
    {
        runTC5(code);
    }
    else
    {
        cout << "Not TC5\n";
    }

    return 0;
}