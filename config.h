#pragma once

#define PLAYER "mpv"

typedef struct { const char *name, *url, *desc; } Station;

static const Station STATIONS[] = {
    { "nts-1",       "https://stream-relay-geo.ntslive.net/stream",          "NTS Radio — Channel 1" },
    { "nts-2",       "https://stream-relay-geo.ntslive.net/stream2",         "NTS Radio — Channel 2" },
    { "fip",         "https://icecast.radiofrance.fr/fip-hifi.aac",          "FIP"                   },
};

#define N_STATIONS (int)(sizeof(STATIONS) / sizeof(STATIONS[0]))
