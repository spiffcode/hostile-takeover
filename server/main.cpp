#include "inc/basictypes.h"
#include "base/thread.h"
#include "base/socketaddress.h"
#include "base/tick.h"
#include "base/log.h"
#include "server/server.h"
#include "server/levelinfocache.h"
#include "server/ncpackfile.h"
#include "server/statsposter.h"
#include "server/serverinfoupdater.h"
#include "mpshared/constants.h"
#include "mpshared/packmanager.h"
#include "mpshared/xmsglog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

// Main server thread
base::Thread main_thread;

// These are defaults that can be overridden
const dword kcRoomsMax = 200;
const dword kcGamesPerRoomMax = 100;
const dword kcPlayersPerRoomMax = 200;
const dword kcConnectedPlayersMax = 1500;

std::vector<std::string> ParseArgs(int argc, char **argv, const char *fmt) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], fmt) == 0 && i <= argc - 2) {
            args.push_back(argv[i + 1]);
            i++;
        }
    }
    return args;
}

std::string ParseString(int argc, char **argv, const char *fmt) {
    std::vector<std::string> args = ParseArgs(argc, argv, fmt);
    if (!args.empty()) {
        return args[0];
    }
    return "";
}

int ParseInteger(int argc, char **argv, const char *fmt, int def = 0) {
    std::string s = ParseString(argc, argv, fmt);
    if (s.length() == 0) {
        return def;
    }
    int n = def;
    base::Format::ToInteger(s.c_str(), 10, &n);
    return n;
}

int ParseDword(int argc, char **argv, const char *fmt, dword def = 0) {
    std::string s = ParseString(argc, argv, fmt);
    if (s.length() == 0) {
        return def;
    }
    dword n = def;
    base::Format::ToDword(s.c_str(), 10, &n);
    return n;
}

bool FindArg(int argc, char **argv, const char *fmt) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], fmt) == 0) {
            return true;
        }
    }
    return false;
}

base::SocketAddress GetListenAddress(int argc, char **argv) {
    // Accept address arg or default address, port arg or default port
    // For default address, choose interface adapter.

    base::SocketAddress listen_address;
    std::string listen_address_arg = ParseString(argc, argv, "--listen_address");
    if (listen_address_arg.size() != 0) {
        listen_address = base::SocketAddress(listen_address_arg.c_str());
    } else {
        // Choose a network, rather than using ANY, since it will be sent in server info
        int n = 0;
        char sz[32];
        dword ip;
        while (base::SocketAddress::GetNetworkInfo(n, sz, sizeof(sz), &ip)) {
            if (strncmp(sz, "eth", 3) == 0) {
                listen_address.SetIP(ip);
                break;
            }
            if (strncmp(sz, "en", 2) == 0) {
                listen_address.SetIP(ip);
                break;
            }
            n++;
        }
    }
    int listen_port = ParseInteger(argc, argv, "--listen_port");
    if (listen_port != 0) {
        listen_address.SetPort(listen_port);
    }
    return listen_address;
}

int main(int argc, char **argv)
{
    // Log arguments
    std::string args;
    if (argc != 0) {
        for (int i = 0; i < argc; i++) {
            if (i != 0) {
                args += " ";
            }
            args += argv[i];
        }
    } else {
        args = "<none>";
    }
    RLOG() << "Arguments: " << args.c_str();

    // Seed random number generator
    srand((int)base::GetTickCount());

    // Root of the pack cache
    std::string rootdir = ParseString(argc, argv, "--missionpack_dir");
    std::string packdir = rootdir + "/pack";
    std::string indexfile = rootdir + "/index";
    std::string modfile = rootdir + "/modlist";
    std::string badwordsfile = rootdir + "/badwords";

    // Initialize the level info cache from the index so the server knows
    // the latest and greatest mission packs.
    wi::PackManager packm(packdir.c_str());
    packm.WatchIndex(indexfile.c_str());
    wi::LevelInfoCache cache(packm);

    // Submit main pack and packs from the index
    wi::NoCachePackFileReader pakr;
    if (pakr.Push(NULL, ParseString(argc, argv, "--htdata").c_str())) {
        wi::PackId packid;
        memset(&packid, 0, sizeof(packid));
        packid.id = PACKID_MAIN;
        cache.Submit(pakr, packid);
        pakr.Pop();
    }
    cache.SubmitIndex(indexfile);
    if (cache.empty()) {
        RLOG() << "cache is empty!";
        exit(1);
    }

    // Set up StatsPoster
    base::SocketAddress stats_address(ParseString(argc, argv,
            "--stats_address").c_str());
    std::string stats_path(ParseString(argc, argv, "--stats_path"));
    wi::StatsPoster stats(stats_address, stats_path);

    // Set up the server
    dword server_id = ParseDword(argc, argv, "--server_id");
    bool checksync = FindArg(argc, argv, "--checksync");
    RLOG() << "Sync checking turned " << (checksync ?  "ON." : "OFF.");

    bool accountsharing = FindArg(argc, argv, "--accountsharing");
    RLOG() << "Account sharing turned " << (accountsharing ?  "ON." : "OFF.");

    std::string log_file = ParseString(argc, argv, "--log_file");
    wi::XMsgLog *xmsglog = NULL;
    if (log_file.size() != 0) {
        xmsglog = new wi::XMsgLog(log_file);
    }

    int max_players = ParseInteger(argc, argv, "--max_players",
            kcConnectedPlayersMax);
    int max_rooms = ParseInteger(argc, argv, "--max_rooms", kcRoomsMax);
    int max_games_per_room = ParseInteger(argc, argv, "--max_games_per_room",
            kcGamesPerRoomMax);
    int max_players_per_room = ParseInteger(argc, argv,
            "--max_players_per_room", kcPlayersPerRoomMax);

    wi::Server server(stats, xmsglog, cache, server_id, checksync,
            max_rooms, max_games_per_room, max_players_per_room,
            max_players, modfile, badwordsfile, accountsharing);

    base::SocketAddress listen_address = GetListenAddress(argc, argv);
    if (!server.Listen(listen_address)) {
        printf("Server failed to start.\n");
        return 1;
    }
    listen_address = server.listen_address();

    // Create default rooms
    dword result;
    server.lobby().NewRoom(NULL, "Main", "", wi::kroomidMain,
            wi::kfRmPermanent | wi::kfRmLocked, &result);
    server.lobby().NewRoom(NULL, "Main / Registered", "", wi::kroomidRegistered,
            wi::kfRmPermanent | wi::kfRmRegisteredOnly | wi::kfRmLocked,
            &result);
    server.lobby().NewRoom(NULL, "Main / Unmoderated", "",
            wi::kroomidUnmoderated,
            wi::kfRmPermanent | wi::kfRmLocked | wi::kfRmUnmoderated | wi::kfRmRegisteredOnly,
            &result);

    // Get the public_address. This is the address that gets sent in
    // ServerInfo. For example in AWS, the internal "private" ip
    // is not the same as the public one. If this is not present,
    // use the listen_address.
    base::SocketAddress public_address;
    std::string public_address_str = ParseString(argc, argv, "--public_address");
    if (public_address_str.size() != 0) {
        public_address = base::SocketAddress(public_address_str.c_str());
    } else {
        public_address = listen_address;
    }

    // Start up ServerInfoUpdater
    std::string server_info_path(ParseString(argc, argv, "--server_info_path"));
    std::string server_name(ParseString(argc, argv, "--server_name"));
    std::string server_location(ParseString(argc, argv, "--server_location"));
    std::string server_type(ParseString(argc, argv, "--server_type"));
    std::string server_info_extra(ParseString(argc, argv, "--server_info_extra"));
    dword expires = ParseDword(argc, argv, "--server_info_expires", 60 * 5);
    wi::ServerInfoUpdater updater(server, stats_address, server_info_path,
            server_id, server_name, server_location, server_type,
            public_address, server_info_extra, expires);
    server.SetUpdater(&updater);

    // Pump messages and wait for network i/o
    base::Thread::RunLoop();
    return 0;
}
