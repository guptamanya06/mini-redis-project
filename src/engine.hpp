#pragma once
#include <string>
#include <optional>
#include <functional>

class EngineSimple {
public:

    explicit EngineSimple(std::function<void(const std::string&)> expiry_callback = nullptr);

    void set(const std::string& key, const std::string& value, int ttl_seconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    void sweep_expired();            
    long ttl(const std::string& key); 
    ~EngineSimple();

private:
    struct Impl;
    Impl* impl_;
};
