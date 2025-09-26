#include "engine.hpp"
#include <chrono>
#include <utility>

Engine::Engine(std::function<void(const std::string&)> expiry_callback)
: expiry_callback_(std::move(expiry_callback)) {}

std::int64_t Engine::now_seconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

void Engine::set(const std::string& key, const std::string& val, int ttl_seconds) {
    std::lock_guard<std::mutex> lg(mu_);
    ValueEntry e;
    e.value = val;
    if (ttl_seconds > 0) {
        e.expires_at = now_seconds() + ttl_seconds;
     
        expiry_pq_.push(ExpireItem{e.expires_at, key});
    } else {
        e.expires_at = 0;
    }
    store_[key] = std::move(e);
}

std::optional<std::string> Engine::get(const std::string& key) {
    std::lock_guard<std::mutex> lg(mu_);
    auto it = store_.find(key);
    if (it == store_.end()) return std::nullopt;
    if (it->second.expires_at != 0 && it->second.expires_at <= now_seconds()) {
       
        std::string expired_key = it->first;
        store_.erase(it);
        if (expiry_callback_) {
            
        }
       
        return std::nullopt;
    }
    return it->second.value;
}

bool Engine::del(const std::string& key) {
    std::lock_guard<std::mutex> lg(mu_);
    return store_.erase(key) > 0;
}

bool Engine::exists(const std::string& key) {
    std::lock_guard<std::mutex> lg(mu_);
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    if (it->second.expires_at != 0 && it->second.expires_at <= now_seconds()) {
        store_.erase(it);
        return false;
    }
    return true;
}

long Engine::ttl(const std::string& key) {
    std::lock_guard<std::mutex> lg(mu_);
    auto it = store_.find(key);
    if (it == store_.end()) return -1;
    if (it->second.expires_at == 0) return -1;
    auto rem = it->second.expires_at - now_seconds();
    return rem > 0 ? rem : -1;
}

void Engine::sweep_expired() {
    
    std::vector<std::string> to_notify;
    while (true) {
        std::lock_guard<std::mutex> lg(mu_);
        if (expiry_pq_.empty()) break;
        auto top = expiry_pq_.top();
        auto now = now_seconds();
        if (top.expires_at > now) break; 
      
        auto it = store_.find(top.key);
        if (it == store_.end()) {
            expiry_pq_.pop();
            continue;
        }
        if (it->second.expires_at != top.expires_at) {
          
            expiry_pq_.pop();
            continue;
        }
        store_.erase(it);
        expiry_pq_.pop();
        to_notify.push_back(top.key);
    }
    for (const auto& k : to_notify) {
        if (expiry_callback_) expiry_callback_(k);
    }
}
