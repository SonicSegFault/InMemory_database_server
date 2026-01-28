#include "database.hpp"

bool db::KVStore::set(const std::string& key, const std::string& value) {
    store_[key] = value; return true;
}

std::optional<std::string>db::KVStore::get(const std::string& key) {
    auto it = store_.find(key);
    if(it != store_.end()){
        return it->second;
    } return std::nullopt;
}   

bool db::KVStore::del(const std::string& key) {
    return store_.erase(key) > 0;
}