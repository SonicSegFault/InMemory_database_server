#pragma once

#include <unordered_map>
#include <string>
#include <optional>

namespace db {
    class KVStore {
        public:
            bool set(const std::string& key, const std::string& value);
            std::optional<std::string> get(const std::string&key);
            bool del(const std::string& key);
        private:
            std::unordered_map<std::string, std::string> store_;
    };
} // namespace db
