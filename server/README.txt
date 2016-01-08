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

Running the leaderboard
-----------------------

Multiplayer server relies on a Google AppEngine application, located in the
stats/ directory, called the leaderboard. This server is used for
authentication, profiles, and game stats. Before running the leaderboard,
choose a Google App Engine app name (the name is never visible):

https://console.developers.google.com

Change the application: line in stats/app.yaml with your app name:

application: <your Google App Engine app name>

Next edit stats/config.py, and search for REPLACEME_ and change these
'secrets' to unique complex strings. Edit server/secrets.cpp and use the
same secret strings as used in stats/config.py. Make sure these secrets
are never checked into a public repository.

Now run the leaderboard. If you plan to run locally only, run the app in
stats/, and use 8080 for the port. If you plan to run the leaderboard
remotely, push the app to Google app engine.

Preparing the game client
-------------------------

1. Modify game/serviceurls.cpp with the appropriate leaderboard urls
from above that you are using.

2. Modify game/serviceurls.cpp to point to an http server that serves
mission packs (see testdata.tar.gz for layout):

const char *kszIndexUrl = "http://<your_server.com>/wi/index";
const char *kszPackInfoUrl = "http://<your_server.com>/wi/info";
const char *kszPackUrl = "http://<your_server.com>/wi/pack";

Recompile the client.

Customizing server/docker/config.json
-------------------------------------

   If you intend to create public servers and manage them, you'll want
   to set up a proper config.json. This is a json formatted dictionary
   of the below keys. If you just want to run locally, this isn't required.
   Note you can set this up later and then rebuild everything, but it is
   easier to do up front. The config variables are:

   AUTH_PREFIX: some services (like Google Compute Engine), use 'gcloud'
        to set up docker registry authentication. If you're using
        GCE, set this to gcloud. It is ok to leave this empty if you're
        authenticating with your registry in a different way.

   PROJECT_NAME: this is the project name that will be used as part of
        the docker image name:
        <registry_url>/<project_name>/<image_name>:<image_tag>
        If this variable isn't set, the default value is 'server'.

   REGISTRY_URL: this is the url that refers to the registry this
        image will reside on. For example for Google Compute Engine,
        this might be 'us.gcr.io'. It's ok to keep this empty if there
        you have no intent of making public servers.

   WICONTENT_URL: The server must have access to the same mission packs
        that users have. This is the url of the mission pack tarball that
        the server will poll for changes every 5 minutes. The polling
        is done by using HTTP ETags for efficiency, which means a download
        only takes place if there is a change.

   LEADERBOARD_ADDRESS_AND_PORT: This is the domain name and port of the
        leaderboard server, in the format <domain_name>:<port>, typically
        <GAE APPNAME>.appspot.com:80. If running locally, that would be
        127.0.0.1:<port>, with port typically 8080.
        
   SENDCOMMAND_SECRET: this is the secret key used to sign requests to
        the leaderboard's /api/sendcommand request handler. Ths must match
        SENDCOMMAND_SECRET found in ../stats/config.py.

   Finally config.json shouldn't be checked into the project, so that it
   doesn't accidentally find its way onto github.com.

   If you don't provide a config.json, you can still run locally. Your
   project_name will default to 'server', your leaderboard will be assumed
   to be running at 127.0.0.1:8080, and you won't be able to push images
   or create/delete servers.

Quick game server overview
--------------------------

The server supports two modes of building and executing. By default the server
is built and run inside a docker container. You can also build and run without
containers if you wish (read more below). Containers are the default because
that is the format the WI team uses for public servers.

Docker containers can be run on most public hosting services including Amazon,
Google, Azure, and other providers. The WI team uses Google Compute Engine,
and provies utilities for creating and deleting public servers on Google
Compute Engine.

Building and Running Quick Overview
-----------------------------------

By default, 'make' will build from a container, and './run' will run
from a container. Read the sections below about using containers and
registry authentication. Native building and running is also supported by
specifying DOCKER=0. 

1. Build the server

   [REL=1] [DOCKER=0] make [clean] [all]

   DOCKER=0 means build without using docker containers (default is build
        with docker conainers).
   REL=1 means build release (default is build debug)
   clean removes all build products.
   all builds the server. This is the default target if no targets are
        specified.

2. Run the server

   [DEBUGGER=<gdb|cgdb>] [REL=1] [DOCKER=0] ./run

   If no mission pack data is installed an attempt will be made to
   install it from either testdata.tar.gz or from the WICONTENT_URL specified
   in docker/config.json.
  

Building and Running Using Containers
-------------------------------------

Install the gcloud tool from here:
https://cloud.google.com/sdk/

If you've installed some time ago, make sure it is up to date:

$ gcloud components update

Login:

$ gcloud auth login

Select the Google Compute Engine project you'll be using for this server:

$ gcloud config set project <GCE project name>

The latest image tags are stored in docker/image_versions.json and are updated
every time an image is built. When images are used for building or running,
the latest image tag is looked up in this file and used. You either have this
image locally already, or it is stored on the remote registry. Importantly,
on a multi-person team, this means if you check in image_versions.json, be
sure to first push the images with those versions to the registry (more on
this below).

Note using distinct image versions rather than 'latest' ensures that docker
doesn't use the wrong image for building or running.

1. Install docker:

   https://docs.docker.com/engine/installation/ 

   If on OSX or Windows, once installation is complete, run the
   "Docker Quickstart Terminal". This initializes the VirtualBox VM with
   docker daemon running, and sets up necessary environment variables.
   Once this is complete, in any other terminal instance you can type
   "eval $(docker env default)" to set environment variables and communicate
   with the docker daemon. Note none of this is necessary on Linux since
   docker daemon is running natively.

2. Create wi_base and wi_build images if necessary

   There are 3 images used:

   wi_base: basic image that the other two images are based on
   wi_build: wi_base + build tools for building the server + debuggers.
        Also used for running locally.
   wi_server: used for running public servers.

   If you haven't built wi_base or wi_build yet, you need to do that
   first, because wi_build is used to build the server. If you will be
   using wi_build image stored on your image registry to build the server,
   you can skip this step.

   $ make new_base_build_images

   If you're running a remote registry, be sure to push these new images
   to the registry.

   $ make push_base_build_images

4. Build the server

   If you will be using wi_build image that you built locally, you don't need
   to be authenticated to continue. If you are using a wi_build image stored
   on your image registry to build the server, first authenticate with your
   registry service. For Google Compute, if you already followed the
   authentication steps mentioned earlier, you're ready to go.

   Refer to build instructions detailed above.

5. Run the server locally

   Similar to the above, if you're using a local image you don't need
   authentication. If you're using a remote image, be sure to authenticate
   with your registry first.

   When running locally in a container, the entire source tree is mapped into
   the container. This makes it possible for source code debugging to work
   properly. Also, docker/entrypoint is mapped into the container. This way
   testing changes to entrypoint doesn't require building a new image.

   Refer to the run instructions detailed above.

6. Build and push a server image

   When you're ready to build a server image, first make sure you are
   authenticated as mentioned previously. To build the server image:

   $ make new_server_image

   This will make REL=1 clean all, then build a wi_server image. No source
   code is included in this image, only the bare essentials needed to run
   the serer.

   Next, you need to push this image to the remote registry, so that your
   public server can access it.

   $ make push_server_image
   
7. Create a public server

   This command assumes that Google Compute Engine is being used. Make sure
   you are authenticated as mentioned previously. To create a public server
   on Google Compute Engine:

   $ make create_gcloud_server

   You'll get asked 3 questions:

   1) The first question is about a server id. This should be a number between
      1-99, and be unique from other servers. If you have more than one server
      deployed, the servers will appear in game in a list sorted by this id.
   
   2) Server Name. This is for appearance only and appears in-game.

   3) Server Location. This is also for appearance only and appears in-game.

8. Drain and delete a public server

   This command assumes that Google Compute Engine is being used. Make sure
   you are authenticated as mentioned previously. To drain and delete a public
   server on Google Compute Engine:

   $ make drain_delete_gcloud_server

   This will lead you through a series of steps to drain and delete a server.
   The script will first will wait for the user count to get to a requested
   level, then put the server into drain mode. In drain mode, new users can't
   join the server. Then every minute a message is broadcasted to the
   remaining users that the server will be shut down. This is continued until
   the user count drops to zero, or 5 minutes, whichever comes first, then
   the server is deleted.

9. Rebuild your client with the changes that were made to serviceurls.cpp.

10. If you can connect to your public server and play, distribute your game client.

Operational overview
--------------------

When a user presses the multiplayer button, the client queries the leaderboard
for a list of current game servers (game servers publish information about
themselves to the leaderboard on a regular interval). This is an example of
what the leaderboard returns to the client for Hostile Takeover:

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

The server expects to find mission pack content (and other content) in this
directory layout:

wi/
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

It's necessary that the client and the server see the same mission packs.
Hostile Takeover supports user submitted missions packs submitted to
the forums. A cronjob pulls the mission packs out of the forums, checks them
for validity, and builds a tarball with the above layout. The game servers
will poll for changes to this tarball in an efficient way.

Please contact scottlu for a copy of the latest tarball.


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
