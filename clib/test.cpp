#include <set>
#include <map>
#include "my_hash_map.h"
#include "uSTL.h"
using namespace std;

void test1() {
    cout << "test1" << endl;
    map<int, int> maps;

    uSTL node;
    maps[1] = 10;
    maps[2] = 12;
    maps[3] = 13;
    printf("%d: %d\n", 1, node.get(maps, 1, 100));
    printf("%d: %d\n", 2, node.get(maps, 2, 100));
    printf("%d: %d\n", 3, node.get(maps, 3, 100));
    printf("%d: %d\n", 5, node.get(maps, 5, 100));
}

int main(int argc, char* argv[]) {
    cout << "ok" << endl;
    test1();
    cout << "end" << endl;
    return 0;
    set<int> sets;
    sets.insert(1);
    sets.insert(2);
    sets.insert(3);

    uSTL node;
    cout << node.contain(sets, 1) << endl;
    cout << node.contain(sets, 2) << endl;
    cout << node.contain(sets, 4) << endl;
    cout << node.contain(sets, 100) << endl;
    
}
