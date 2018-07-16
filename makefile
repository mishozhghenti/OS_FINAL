all: net_raid_server net_raid_client

net_raid_server: net_raid_server.c
	gcc net_raid_server.c -o net_raid_server

net_raid_client: net_raid_client.c
	 gcc -Wall net_raid_client.c `pkg-config fuse --cflags --libs` -o net_raid_client

clean:
	 rm net_raid_client net_raid_server
