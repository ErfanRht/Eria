#include <iostream>
#include <bits/stdc++.h>
using namespace std;

int tabeBazgashti(int mahdi, int ali, int i) {
    if (i == -1){
        return 0;
    }
    if (mahdi == 0 || ali == 0)
    {
        return 1;
    }
    else{
        return tabeBazgashti(mahdi - 1, ali, i + 1) + tabeBazgashti(mahdi, ali - 1, i - 1);
    }
}

int main()
{
    int mahdi, ali;
    cin >> mahdi >> ali;

    int maxHalat=0;
    int i=0;
    maxHalat = tabeBazgashti(mahdi, ali, i);

    cout << maxHalat;

    return 0;
}
