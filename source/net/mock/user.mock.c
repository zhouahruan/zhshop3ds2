/**
 * Mock user data + login.
 */
#include "../api.h"
#include "../../core/utils.h"

#include <string.h>

int mock_login(const char* username, const char* password, User* out) {
    if (!username || !password || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->user_id  = 1;
    utils_strlcpy(out->username, username, sizeof(out->username));
    utils_strlcpy(out->token,     "mock-token-XYZ",  sizeof(out->token));
    out->is_login = 1;
    return 1;
}
