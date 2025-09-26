#include "engine_simple.hpp"

#include <unordered_map>
#include <mutex>
#include <queue>
#include <chrono>
#include <vector>
#include <memory>

using namespace std::chrono;

static inline std::int64_t now_seconds() {
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

struct ValueEntry {
    std::string value;
    std::int64_t expires_at = 0; 
};

struct ExpireItem {
    std::int64_t expires_at;
    std::string key;
    bool operator>(const ExpireItem& o) const {
        return expires_at > o.expires_at;
    }
};

struct EngineSimple::Impl {
    std::unordered_map<std::string, ValueEntry> store;
    std::priority_queue<ExpireItem, std::vector<ExpireItem>, std::greater<ExpireItem>> pq;
    std::mutex mu;
    std::function<void(const std::string&)> expiry_callback;
    explicit Impl(std::function<void(const std::string&)> cb) : expiry_callback(std::move(cb)) {}
};

EngineSimple::EngineSimple(std::function<void(const std::string&)> expiry_callback)
: impl_(new Impl(std::move(expiry_callback))) {}

EngineSimple::~EngineSimple() { delete impl_; }

void EngineSimple::set(const std::string& key, const std::string& value, int ttl_seconds) {
    std::lock_guard<std::mutex> lg(impl_->mu);
    ValueEntry e;
    e.value = value;
    if (ttl_seconds > 0) {
        e.expires_at = now_seconds() + ttl_seconds;
        impl_->pq.push(ExpireItem{e.expires_at, key});
    } else {
        e.expires_at = 0;
    }
    impl_->store[key] = std::move(e);
}

std::optional<std::string> EngineSimple::get(const std::string& key) {
    std::lock_guard<std::mutex> lg(impl_->mu);
    auto it = impl_->store.find(key);
    if (it == impl_->store.end()) return std::nullopt;
    if (it->second.expires_at != 0 && it->second.expires_at <= now_seconds()) {
        impl_->store.erase(it);
        return std::nullopt;
    }
    return it->second.value;
}

bool EngineSimple::del(const std::string& key) {
    std::lock_guard<std::mutex> lg(impl_->mu);
    return impl_->store.erase(key) > 0;
}

bool EngineSimple::exists(const std::string& key) {
    std::lock_guard<std::mutex> lg(impl_->mu);
    auto it = impl_->store.find(key);
    if (it == impl_->store.end()) return false;
    if (it->second.expires_at != 0 && it->second.expires_at <= now_seconds()) {
        impl_->store.erase(it);
        return false;
    }
    return true;
}

long EngineSimple::ttl(const std::string& key) {
    std::lock_guard<std::mutex> lg(impl_->mu);
    auto it = impl_->store.find(key);
    if (it == impl_->store.end()) return -1;
    if (it->second.expires_at == 0) return -1;
    auto rem = it->second.expires_at - now_seconds();
    return rem > 0 ? rem : -1;
}

void EngineSimple::sweep_expired() {
    std::vector<std::string> expired_keys;
    while (true) {
        std::lock_guard<std::mutex> lg(impl_->mu);
        if (impl_->pq.empty()) break;
        auto top = impl_->pq.top();
        auto now = now_seconds();
        if (top.expires_at > now) break; 
        impl_->pq.pop();
        auto it = impl_->store.find(top.key);
        if (it == impl_->store.end()) {
            continue; 
        }
        if (it->second.expires_at != top.expires_at) {
            continue; 
        }
        
        impl_->store.erase(it);
        expired_keys.push_back(top.key);
    }
    for (const auto& k : expired_keys) {
        if (impl_->expiry_callback) impl_->expiry_callback(k);
    }
}
