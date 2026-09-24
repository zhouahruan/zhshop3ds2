/**
 * API configuration.
 *
 * Set USE_MOCK to 1 to ship with built-in mock data so the app runs
 * without a backend. Set to 0 once you have a real API and fill in
 * API_BASE_URL. The ApiClient in api.h reads these values.
 */
#ifndef NET_API_CONFIG_H
#define NET_API_CONFIG_H

/* Keep USE_MOCK=1 for the bundled CIA so the app works offline on
 * real hardware. Set to 0 for live API testing. */
#define USE_MOCK          1

/* Zone base: 3DS endpoints live under the /3ds path. The global endpoints
 * (visitors/pulse/announcements/search) use API_GLOBAL_BASE_URL. */
#define API_BASE_URL      "https://backend.appmiaoda.com/projects/supabase317616740710264832/functions/v1/zhshop-api/3ds"
#define API_GLOBAL_BASE_URL \
    "https://backend.appmiaoda.com/projects/supabase317616740710264832/functions/v1/zhshop-api"
#define API_TIMEOUT_MS    15000

/* Optional bearer token for authenticated endpoints (POST /apps, etc.). */
#define USER_TOKEN         ""

/* soc / httpc buffer sizes. */
#define SOC_BUFFER_SIZE   (0x100000)
#define SOC_BUFFER_ALIGN  (0x1000)

#endif /* NET_API_CONFIG_H */
