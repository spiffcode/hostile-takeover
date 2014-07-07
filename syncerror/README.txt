These scripts are for debugging sync errors. Sync errors are caused by one
of the clients going out of sync with the other clients in an multiplayer game.
Since multiplayer is done as a synchronized simulation, all clients must have
exactly the same simulation state. Each client uploads a hash to the server of
the simulation state. For each game step, the server checks the hashes for all
clients and if there is a mismatch, tells the clients and they upload their
state to the sync error url. This state can be parsed with these tools to find
where the state divergence occured.
