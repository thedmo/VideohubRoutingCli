# VideohubRoutingCli

is a small Cli Program to remotely control BMD Videohub. This is a revamp of my former project VideohubRouterCli.

Goal is to make it so that routings from known devices can be saved and loaded with one command instead of having to set every single connection.

At the Location of the program a sqlite database with all the device information gets saved. If it is not apparent, it will automatically get created. If you want to migrate the database to another machine, you can just copy the database to the new location along with the binary of the tool.

Please test this tool in a test environment, before implementing it into production.
This tool is made by me in my freetime and a do not take any responsibility for any harm that occured because of it.

## Usage

- -h,   --help
Print Usage
- -a,   --add_router \<ip address\>
Add new device by ipv4 address to database. Only works, if router is available in network.
- -rm,  --remove_router
removes selected device from database
- -s,   --select_router \<ip address\>
select router by ipv4 address
- -LD,  --list_devices
lists all devices from database
- -rs,  --rename_source \<source number\> \<new name\>
rename source from selected device in database. Only works, when device is available in network
- -Ls,  --list_sources
list all sources from selected device
- -rd,  --rename_destination \<destination number\> \<new name\>
renames destination of selected device in database. onl works, when device is available in network.
- -Ld,  --list_destinations
list all detinations from selected device
- -nr,  --new_route \<destintaion number\> \<source number\>
prepare new route to take on selected device
- -t,   --take
take all prepared routes from selected device
- -l,   --lock \<destination number\>
lock route on selected device
- -Lr,  --list_routes
list routes of selected device
- -m,   --mark \<destination number\>
mark route to save
- -sr,  --save_routes \<routing name\>
save marked routes for selected device with a name
- -lsr, --list_saved_routings
list all saved routings for selected device
- -lr,  --load_routes \<routing name\>
load routes for selected device by name

## Todo

- list sources
- rename destination
- list destinations
- lock route
- list routes
- list saved routings

## Future features

- GUI with a matrix like interface (e.g. Dante interface; top: Sources, left: Destinations)
- Cross Compatibility (at the moment, because of telnet implementation with winsock, this tool can only operate in a windows environment)
- Implement mulicast discovery, so that videohub devices can be found even if ip address is not known
