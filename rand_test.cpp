#include "al/core.hpp"
#include "al/util/al_Random.hpp"

#include <iostream>

using namespace al;
using namespace std;

int main()
{
    cout << rnd::uniform() << endl;

    float arr[10];
    for (int i = 0; i < 10; i += 1) {
        arr[i] = float(i);
    }

    StdRandom rnd;
    rnd.shuffle(arr, 10);

    for (int i = 0; i < 10; i += 1) {
        cout << arr[i] << endl;
    }

    return 0;
}