#include <string>
#include <sstream>
#include <vector>
#include <cctype>

#ifndef INNERSTAT_COMMON_JSON_UTIL_H_
#define INNERSTAT_COMMON_JSON_UTIL_H_

// ---- JSON utilities (simple, not a full JSON parser) ----
namespace innerstat_json_util {
    inline std::string escape(const std::string &in) {
        std::ostringstream oss;
        for (char c : in) {
            switch (c) {
                case '\\': oss << "\\\\"; break;
                case '"': oss << "\\\""; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        oss << "\\u";
                        const char *hex = "0123456789abcdef";
                        oss << hex[(c >> 12) & 0xF] << hex[(c >> 8) & 0xF] << hex[(c >> 4) & 0xF] << hex[c & 0xF];
                    } else {
                        oss << c;
                    }
            }
        }
        return oss.str();
    }

    inline std::string unescape(const std::string &in) {
        std::ostringstream oss;
        for (size_t i = 0; i < in.size(); ++i) {
            char c = in[i];
            if (c == '\\' && i + 1 < in.size()) {
                char n = in[++i];
                switch (n) {
                    case 'n': oss << '\n'; break;
                    case 'r': oss << '\r'; break;
                    case 't': oss << '\t'; break;
                    case '"': oss << '"'; break;
                    case '\\': oss << '\\'; break;
                    case 'u': {
                        // Skip simplistic unicode handling (assume ASCII)
                        // Read next 4 hex digits if available
                        if (i + 4 < in.size()) {
                            std::string hex = in.substr(i + 1, 4);
                            // We won't convert - just ignore and advance
                            i += 4;
                        }
                        break;
                    }
                    default: oss << n; break;
                }
            } else {
                oss << c;
            }
        }
        return oss.str();
    }

    // Extract value for a key in a flat JSON object {"key":"value", ...}
    inline bool extractString(const std::string &obj, const std::string &key, std::string &out) {
        std::string pattern = '"' + key + '"';
        size_t pos = obj.find(pattern);
        if (pos == std::string::npos) return false;
        pos = obj.find(':', pos);
        if (pos == std::string::npos) return false;
        // skip spaces
        while (pos < obj.size() && obj[pos] != '"' && obj[pos] != '-' && !std::isdigit(obj[pos])) ++pos;
        if (pos >= obj.size()) return false;
        if (obj[pos] == '"') {
            size_t start = ++pos;
            std::ostringstream val;
            bool escapeFlag = false;
            for (; pos < obj.size(); ++pos) {
                char c = obj[pos];
                if (!escapeFlag && c == '\\') { escapeFlag = true; continue; }
                if (!escapeFlag && c == '"') break;
                if (escapeFlag) { val << '\\' << c; escapeFlag = false; } else { val << c; }
            }
            out = unescape(val.str());
            return true;
        } else {
            // numeric without quotes
            size_t start = pos;
            while (pos < obj.size() && (std::isdigit(obj[pos]) || obj[pos]=='.' || obj[pos]=='-' || obj[pos]=='e' || obj[pos]=='E')) ++pos;
            out = obj.substr(start, pos-start);
            return true;
        }
    }
}

#endif // INNERSTAT_COMMON_JSON_UTIL_H_