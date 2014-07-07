Hostile Takeover Multiplayer Server
-----------------------------------

Hostile Takeover requires two servers to operate multiplayer:

1. A TCP based game server written in C++, located in the server directory.
This server builds and runs on OSX or Linux. This server does not build
and run on Windows at this time.

2. An HTTP based server located in the stats directory for accounts,
leaderboard, game history, and game stats. This is written in python and runs
on Google App Engine. This server is called the "leaderboard server", or just
"leaderboard" for short.

This document will refer to the game running on a mobile device as the "client".

Quick start: how to run locally
-------------------------------

The game server builds / runs on OSX or Linux. The game server does not
build or run on Windows at this time.

This is a simple, short instructions
for how to run the game server and leaderboard server locally, for testing
purposes:

1. From the server directory:

   # For release
   mkdir release
   make REL=1 clean all
   # For debug
   mkdir debug
   make clean all

2. From the server directory, tar xvf testdata.tar.gz
   This is test data for the server. For the full mission pack list, see below
   in this document.

3. Run the App Engine app in the stats directory locally using the GAE 
   launcher. Show the GAE log console for debug output

4. Modify start.sh with the appropriate port you're using for the locally
   running instance of the stats GAE app

5. To run the server, use start.sh locally for a sample set of command line
   flags: . start.sh

6. Build the game under Debug so the localhost service urls are used
   (see game/serviceurls.cpp).

7. Run the game in the simulator. Press Play -> Multiplayer.

Please continue reading for how to deploy on a public server.

How to run in production
------------------------

1. Create a Google App Engine application name here:

   https://appengine.google.com/

   It doesn't matter what the name is, the user will never see it.

2. Change the application: line in ../stats/app.yaml with your app name:

   application: <your Google App Engine app name>

3. Deploy this application to Google App Engine using the
   GoogleAppEngineLauncher app that comes with the SDK.

4. Modify ../game/serviceurls.cpp, and replace where you see <GAE APPNAME>
   with your GAE app name. These service urls are now permanent.

5. Rebuild the server on a Linux system

   $ cd server
   $ mkdir release
   $ make REL=1 clean all

6. Copy start.sh, release/hts, and htdata832.pdb to a publicly reachable
   Linux system

   $ scp start.sh you@your_server.com:
   $ scp release/hts you@your_server.com:
   $ scp ../game/htdata832.pdb you@your_server.com:

7. Download a copy of the 5000+ single and multiplayer maps, and copy this
   to your server as well

   $ wget http://www.warfareincorporated.com/~wicontent/wicontent.tar.gz
   $ scp wicontent.tar.gz you@your_server.com:

8. ssh to you@your_server.com and perform the following:

   $ <install apache>
   $ cd <to the apache document root>
   $ tar xvf wicontent.tar.gz (probably need to be root to do this)
   
   (you'll now have a wi subdirectory in your apache document root)

9. Back on your local system, modify ../game/serviceurls.cpp, kszIndexUrl,
   kszPackInfoUrl, and kszPackUrl as follows:

const char *kszIndexUrl = "http://<your_server.com>/wi/index";
const char *kszPackInfoUrl = "http://<your_server.com>/wi/info";
const char *kszPackUrl = "http://<your_server.com>/wi/pack";

   This tells the client how to download mission packs. It will also be needed
   by the server.

10. Rebuild your client with the changes that were made to serviceurls.cpp.

11. Back on your_server.com machine, cd into the directory containing
    start.sh, hts, and htdata832.pdb

12. Edit start.sh:

   a. Change --listen_address to be the ip of your server and the port you
      wish to use, in ip:port format.
   b. Change --missionpack_dir to the directory that holds the
      wicontent.tar.gz content that was untarred into the apache document
      root. For example: --missionpack_dir /var/www/wi
   c. Change --htdata to ./htdata832.pdb, since it is in the current directory.
   d. Change --stats_address to <GAE NAME>.appsport.com:80, using the Google
      App Engine name you choose previously.
   e. Change --server_name to the server name you wish to show up in-game.
   f. Change --server_location to the server location you want to show up
      in-game.
   g. Change --server_type to production. 
   h. --server_info_extra is for adding additional json to that shows up
      in the /api/serverinfo response from the leaderboard. It is optional but
      can be useful for deployment information such as instance ids, zones,
      versions, etc. Note the client looks for sort_key to indicate how this
      server will be sorted related to others in the server list.

13. Go ahead and run start.sh to start the server. Ultimately this can be
    done from a cronjob at system startup using the @reboot directive (just
    run start.sh from the @reboot crontab directive).

14. Wait 30 seconds for the game server to announce itself to the leaderboard.
    Now start the game client (with the serviceurls.cpp changes), and select
    Multiplayer. You will get an error if your client can't see the server.

15. You can start as many servers as you want. The client will present a list
    of all game servers to the user, who will then choose which server to
    connect to.

16. If this is all working, distribute your game client.


How to deploy a new game server
-------------------------------

Occasionally you'll want to deploy a new game server (with changes perhaps),
yet you want to minimally disrupt game play of players on the existing server.
Here are steps to do that:

1. Build the new server. Deploy it with a lower sort_key so it is first in
   the client's server list.

2. Go to the admin page of the leaderboard with your browser:

   https://<GAE NAME>.appspot.com/private/admin

3. Select Drain / Undrain. Select the checkbox next to the old server you will
   be stopping, select Drain. In approximately a minute, this will put the
   server into a mode where it will stop accepting users.

4. From the admin page, select Send Chat to Server. Select the checkbox next
   to the old server, and type a message that the server will be shutting down
   in 15 minutes, and all users should join the new server. Press submit.
   Within 30 seconds, all users on that server will see this message. Send the
   message every 2 minutes or so to remind users.

5. After 15 minutes or so, stop the old server.

This is the way to do it manually. There is a script that performs these steps
automatically, however if the player count less than 10, just stop the
old server without going through these steps.

Operational overview
--------------------

When a user presses the multiplayer button, the client queries the leaderboard
for a list of current game servers (game servers publish information about
themselves to the leaderboard on a regular interval). This is an example of
what the leaderboard returns to the client for Warfare Incorporated:

{
  "expires_utc": 1402207827,
  "updated_utc": 1402207767,
  "infos": [
    {
      "status": "ok",
      "expires_utc": 1402207827,
      "player_count": 3,
      "protocol": 21,
      "name": "Edge of Tomorrow",
      "zone": "us-east-1b",
      "sort_key": 60,
      "start_utc": 1346624757,
      "instance": "i-ee916831",
      "version": "wiuser-64",
      "location": "USA",
      "address": "50.18.179.243:15618",
      "type": "production"
    }
  ]
}

This lists just one server, although the leaderboard will report all game
servers that are reporting to it. If more than one is listed, the client
will bring up UI letting the user choose which game server to connect to.

After selecting the server, the user will then be able to login with a
user name / password. To authenticate the user, the client sends the
credentials to the leaderboard over HTTPS and gets a token back. The client
then uses this token to login to the game server. The game server knows how
to check to know that the token is valid.

From here, the user will be able to join existing rooms on this server, or
create new rooms, chat with other players, and play games with other users.
Once a game is complete, the game server posts the results to the leaderboard,
which saves them away, and adjusts player scores and stats as appropriate.

Note you can see the serverinfo from your own leaderboard this way:

$ curl http://<GAE APP NAME>.appspot.com/api/serverinfo


Mission Packs and other content
-------------------------------

The server expects to find mission pack content (and other content) at the
location pointed to by --missionpack_dir. This is a directory tree with
a required layout:

<missionpack_dir>
    info                directory containing mission pack descriptions
    pack                directory containing mission packs themselves
    index               an index of the mission packs available for
                        download.
    index-all           an index of all the mission packs, including ones
                        removed from the public list.
    modlist             a list of the game server admins and moderators
    badwords            a description of the swear words to filter out of chat.


Mission Packs
-------------

It's important that the client and the server see the same mission packs.
Warfare Incorporated supports user submitted missions packs submitted to
the forums. A cronjob pulls the mission packs out of the forums, checks them
for validity, and builds the new missionpack_dir content, and pushes these
changes out to all game servers. This happens every 15 minutes or so. Game
servers watch when the index file changes and reload.

Game servers could use the content from warfareincorporated.com directly by
updating from this url:

http://www.warfareincorporated.com/~wicontent/wicontent.tar.gz

NOTE: This file is currently a static snapshot. It can be made to
dynamically update in the future. Contact scottlu.


Modlist
-------

The game server supports administrator and moderator roles. Users that have
these roles have certain powers / abilities that regular users don't have.
This is present to help manage the community and game play environment for the
benefit of the game and players. Admins and mods can issue commands through
in-game or in-room chat. For an overview of commands, grant yourself
admin and/or mod powers, and type /help (commands to be documented).

The structure of the modlist file is very simple:

admin,<username>
mod,<username>

List additional admins and mods as needed, 1 per line.

Some of the commands available to admins aren't available to mods. The game
server will watch for this file to change every 5 minutes, and reload it.


Badwords
--------

Badwords is a description format for words that will get filtered. It isn't
perfect, but it can be made more perfect for anyone interested :). By default,
the server will filter words. Admins / mods can turn filtering off with the
/filter command.

<documentation for badwords to be provided>


Chat logging
------------

Chatting is logged into the log subdirectory of the game directory, using
a binary format. This can be useful to resolve disputes when mods / admins
aren't present, or when there is a dispute involving mods or admins. Only
the game administrator has access to this data.

<documentation for chat log format to be provided>
