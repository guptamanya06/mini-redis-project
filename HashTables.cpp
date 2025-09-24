#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Entry {
    string key;
    string value;
};

class HashTable {
    static const int SIZE = 997;
    vector<Entry> table[SIZE];

    int hash(const string &key) {
        int h = 0;
        for (char c : key) 
            h = (h * 7 + c) % SIZE;
        return h;
    }
public:
    void set(const string &value, const string &key) {
        int idx = hash(key);
        for (auto &entry : table[idx])
            if (entry.key == key) {
                entry.value = value; 
                return; 
            }
        table[idx].push_back({key, value});
    }

    bool get(string &value, const string &key) {
        int idx = hash(key);
        for (auto &entry : table[idx])
            if (entry.key == key) { 
                value = entry.value; 
                return true; 
            }
        return false;
    }

    void del(const string &key) {
        int idx = hash(key);
        auto &bucket = table[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                cout<<"Deleted: "<<it->value<<endl;
                bucket.erase(it); 
                break; 
            }
            else if(it == bucket.end() - 1)
                cout<<"Key not found"<<endl;
        }
    }
};
int main(){
    HashTable ht;
    string name,val;
    int choice;
    string nam, x;
    while(1){
        cout<<"\n****** MENU ******\n";
        cout<<"1. Input\n2. Search\n3. Delete\n4. Exit\n";
        cout<<"Enter your choice: ";
        cin>>choice;
        cin.ignore();
        switch(choice)
        {
            case 1:
                cout<<"Enter a name: ";
                getline(cin, name);
                cout<<"Enter a Key value: ";
                getline(cin, val);
                ht.set(name, val);
                break;
            case 2:
                cout<<"Enter the key: ";
                getline(cin, x);
                if (ht.get(nam, x)) 
                    cout << "Name: " << nam <<endl;
                else 
                    cout << "Name not found" << endl;
                break;
            case 3:
                cout<<"Enter the key to be removed: ";
                getline(cin, x);
                ht.del(x);
                break;
            case 4:
                exit(0);
            default:
                cout<<"\nInvalid Selection\n";
        }
    }
}

