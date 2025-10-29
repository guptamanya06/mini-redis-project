#include <iostream>
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

deque<string> history_deque;
const size_t HISTORY_MAX = 100;

vector<string> split_by_space(const string &s)
{
    vector<string> parts;
    string cur;
    for (size_t i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        if (isspace((unsigned char)c))
        {
            if (!cur.empty())
            {
                parts.push_back(cur);
                cur.clear();
            }
        }
        else
        {
            cur.push_back(c);
        }
    }
    if (!cur.empty())
        parts.push_back(cur);
    return parts;
}

struct Entry
{
    string key;
    string value;
    Entry(const string &k, const string &v) : key(k), value(v) {}
};

class HashTable
{
public:
    HashTable(size_t initial_size = 16)
        : buckets(initial_size), count(0) {}

    void set(const string &key, const string &value)
    {
        size_t idx = index_for(key);
        for (auto &e : buckets[idx])
        {
            if (e.key == key)
            {
                e.value = value;
                return;
            }
        }
        buckets[idx].emplace_back(key, value);
        ++count;
        if ((double)count / buckets.size() > load_factor_threshold)
            rehash();
    }

    bool get(const string &key, string &out)
    {
        size_t idx = index_for(key);
        for (auto &e : buckets[idx])
        {
            if (e.key == key)
            {
                out = e.value;
                return true;
            }
        }
        return false;
    }

    bool del(const string &key)
    {
        size_t idx = index_for(key);
        auto &bucket = buckets[idx];
        for (size_t i = 0; i < bucket.size(); ++i)
        {
            if (bucket[i].key == key)
            {
                bucket.erase(bucket.begin() + i);
                --count;
                return true;
            }
        }
        return false;
    }

    vector<string> keys() const
    {
        vector<string> out;
        for (const auto &bucket : buckets)
            for (const auto &e : bucket)
                out.push_back(e.key);
        return out;
    }

private:
    vector<vector<Entry>> buckets;
    size_t count;
    const double load_factor_threshold = 0.75;

    unsigned long long str_hash(const string &s) const
    {
        const unsigned long long p = 1315423911ULL;
        unsigned long long h = 2166136261u;
        for (unsigned char c : s)
        {
            h = (h ^ c) * p;
        }
        return h;
    }

    size_t index_for(const string &key) const
    {
        return (size_t)(str_hash(key) % buckets.size());
    }

    void rehash()
    {
        size_t new_size = buckets.size() * 2;
        vector<vector<Entry>> new_buckets(new_size);
        for (auto &bucket : buckets)
        {
            for (auto &e : bucket)
            {
                size_t idx = (size_t)(str_hash(e.key) % new_size);
                new_buckets[idx].emplace_back(e.key, e.value);
            }
        }
        buckets.swap(new_buckets);
    }
};
int main()
{
    cout << "MiniRedis C++ CLI. Type HELP for commands. Type EXIT to quit." << endl;
    HashTable ht;
    string line;

    while (true)
    {
        cout << "> ";
        if (!getline(cin, line))
            break;
        size_t a = line.find_first_not_of(" \t\r\n");
        if (a == string::npos)
        {
            continue;
        }
        size_t b = line.find_last_not_of(" \t\r\n");
        string trimmed = line.substr(a, b - a + 1);

        history_deque.push_back(trimmed);
        if (history_deque.size() > HISTORY_MAX)
            history_deque.pop_front();

        vector<string> parts = split_by_space(trimmed);
        if (parts.empty())
            continue;
        string CMD = parts[0];
        for (auto &ch : CMD)
            ch = (char)toupper((unsigned char)ch);
        if (CMD == "EXIT")
        {
            cout << "Thank You" << endl;
            break;
        }
        else if (CMD == "HELP")
        {
            cout << "Commands:\n"
                 << "  SET [key] [value]       - Store a value (single-word value)\n"
                 << "  GET [key]             - Retrieve value\n"
                 << "  DEL [key]             - Delete key\n"
                 << "  KEYS                - List all keys\n"
                 << "  HISTORY [N]         - Show last N commands\n"
                 << "  HELP                - Shows Command List\n"
                 << "  EXIT                - Quit\n";
        }
        else if (CMD == "SET")
        {
            if (parts.size() < 3)
            {
                cout << "Usage: SET key value\n";
                continue;
            }
            string key = parts[1];
            string value = parts[2];
            ht.set(key, value);
            cout << "OK" << endl;
        }
        else if (CMD == "GET")
        {
            if (parts.size() != 2)
            {
                cout << "Usage: GET key\n";
                continue;
            }
            string key = parts[1], out;
            if (ht.get(key, out))
                cout << out << endl;
            else
                cout << "Key does not exist" << endl;
        }
        else if (CMD == "DEL")
        {
            if (parts.size() != 2)
            {
                cout << "Usage: DEL key\n";
                continue;
            }
            string key = parts[1];
            bool removed = ht.del(key);
            cout << (removed ? "Removed Successfully" : "Key does not exist") << endl;
        }
        else if (CMD == "KEYS")
        {
            vector<string> all = ht.keys();
            if (all.empty())
            {
                cout << "There are no existing keys\n";
            }
            else
            {
                for (size_t i = 0; i < all.size(); ++i)
                {
                    if (i)
                        cout << " ";
                    cout << all[i];
                }
                cout << "\n";
            }
        }
        else if (CMD == "HISTORY")
        {
            int n = (int)history_deque.size();
            if (parts.size() > 1)
            {
                try
                {
                    n = stoi(parts[1]);
                }
                catch (...)
                {
                    n = (int)history_deque.size();
                }
            }
            if (n <= 0 || history_deque.empty())
            {
                cout << "No history available\n";
                continue;
            }
            if (n > (int)history_deque.size())
                n = (int)history_deque.size();
            int start = (int)history_deque.size() - n;
            for (int i = start; i < (int)history_deque.size(); ++i)
            {
                cout << (i + 1) << ": " << history_deque[i] << "\n";
            }
        }
        else
        {
            cout << "Unknown command. Type HELP for commands.\n";
        }
    }

    return 0;
}
