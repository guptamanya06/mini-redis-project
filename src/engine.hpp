#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <optional>
#include <cstdint>
#include <queue>
#include <functional>

struct ValueEntry {
    std::string value;
    std::int64_t expires_at = 0; 
};

struct ExpireItem {
    std::int64_t expires_at;
    std::string key;
    bool operator>(ExpireItem const& o) const {
        return expires_at > o.expires_at;
    }
};

class Engine {
public:
    explicit Engine(std::function<void(const std::string&)> expiry_callback = nullptr);
    ~Engine() = default;

    void set(const std::string& key, const std::string& val, int ttl_seconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    void sweep_expired(); 
    long ttl(const std::string& key);

private:
    std::unordered_map<std::string, ValueEntry> store_;
    std::mutex mu_;

    std::priority_queue<ExpireItem, std::vector<ExpireItem>, std::greater<ExpireItem>> expiry_pq_;

    std::function<void(const std::string&)> expiry_callback_;

    static std::int64_t now_seconds();
};
