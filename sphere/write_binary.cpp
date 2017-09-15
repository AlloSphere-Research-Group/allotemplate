#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main()
{
    auto myfile = ofstream("file.binary", ios::binary);
    for (int i = 0; i < 10; i += 1) {
        float f = i;
        myfile.write(reinterpret_cast<char const*>(&f), sizeof(f));
    }
    myfile.close();

    auto readFile = ifstream {"file.binary", ios::binary};
    vector<float> vec;
    vec.resize(10);
    readFile.read(reinterpret_cast<char*>(vec.data()), sizeof(float) * 10);

    for (int i = 0; i < 10; i += 1) {
        // float f;
        // readFile.read(reinterpret_cast<char*>(&f), sizeof(f));
        // cout << f << endl;
        cout << vec[i] << endl;
    }
}