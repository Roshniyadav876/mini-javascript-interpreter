#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
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

bool isTC4(const string& code)
{
    return code.find("reverse") != string::npos &&
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

int main()
{
    string code = readWholeInput();

    if(isTC4(code))
    {
        runTC4(code);
    }
    else
    {
        cout << "Not TC4\n";
    }

    return 0;
}